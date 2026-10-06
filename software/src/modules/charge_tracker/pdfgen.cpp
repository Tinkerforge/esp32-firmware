/**
 * Simple engine for creating PDF files.
 * It supports text, shapes, images etc...
 * Capable of handling millions of objects without too much performance
 * penalty.
 * Public domain license - no warrenty implied; use at your own risk.
 */

/**
 * PDF HINTS & TIPS
 * The specification can be found at
 * https://www.adobe.com/content/dam/acom/en/devnet/pdf/pdfs/pdf_reference_archives/PDFReference.pdf
 * The following sites have various bits & pieces about PDF document
 * generation
 * http://www.mactech.com/articles/mactech/Vol.15/15.09/PDFIntro/index.html
 * http://gnupdf.org/Introduction_to_PDF
 * http://www.planetpdf.com/mainpage.asp?WebPageID=63
 * http://archive.vector.org.uk/art10008970
 * http://www.adobe.com/devnet/acrobat/pdfs/pdf_reference_1-7.pdf
 * https://blog.idrsolutions.com/2013/01/understanding-the-pdf-file-format-overview/
 *
 * To validate the PDF output, there are several online validators:
 * http://www.validatepdfa.com/online.htm
 * http://www.datalogics.com/products/callas/callaspdfA-onlinedemo.asp
 * http://www.pdf-tools.com/pdf/validate-pdfa-online.aspx
 *
 * In addition the 'pdftk' server can be used to analyse the output:
 * https://www.pdflabs.com/docs/pdftk-cli-examples/
 *
 * PDF page markup operators:
 * b    closepath, fill,and stroke path.
 * B    fill and stroke path.
 * b*   closepath, eofill,and stroke path.
 * B*   eofill and stroke path.
 * BI   begin image.
 * BMC  begin marked content.
 * BT   begin text object.
 * BX   begin section allowing undefined operators.
 * c    curveto.
 * cm   concat. Concatenates the matrix to the current transform.
 * cs   setcolorspace for fill.
 * CS   setcolorspace for stroke.
 * d    setdash.
 * Do   execute the named XObject.
 * DP   mark a place in the content stream, with a dictionary.
 * EI   end image.
 * EMC  end marked content.
 * ET   end text object.
 * EX   end section that allows undefined operators.
 * f    fill path.
 * f*   eofill Even/odd fill path.
 * g    setgray (fill).
 * G    setgray (stroke).
 * gs   set parameters in the extended graphics state.
 * h    closepath.
 * i    setflat.
 * ID   begin image data.
 * j    setlinejoin.
 * J    setlinecap.
 * k    setcmykcolor (fill).
 * K    setcmykcolor (stroke).
 * l    lineto.
 * m    moveto.
 * M    setmiterlimit.
 * n    end path without fill or stroke.
 * q    save graphics state.
 * Q    restore graphics state.
 * re   rectangle.
 * rg   setrgbcolor (fill).
 * RG   setrgbcolor (stroke).
 * s    closepath and stroke path.
 * S    stroke path.
 * sc   setcolor (fill).
 * SC   setcolor (stroke).
 * sh   shfill (shaded fill).
 * Tc   set character spacing.
 * Td   move text current point.
 * TD   move text current point and set leading.
 * Tf   set font name and size.
 * Tj   show text.
 * TJ   show text, allowing individual character positioning.
 * TL   set leading.
 * Tm   set text matrix.
 * Tr   set text rendering mode.
 * Ts   set super/subscripting text rise.
 * Tw   set word spacing.
 * Tz   set horizontal scaling.
 * T*   move to start of next line.
 * v    curveto.
 * w    setlinewidth.
 * W    clip.
 * y    curveto.
 */

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS 1 // Drop the MSVC complaints about snprintf
#define _USE_MATH_DEFINES
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE /* For localtime_r */
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600 /* for M_SQRT2 */
#endif

#include <sys/types.h> /* for ssize_t */
#endif

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "pdfgen.h"
#include "options.h"

#if OPTIONS_PRODUCT_ID_IS_WARP()
#define PDF_MAX_OBJECTS 2400
#else
// Maximum tracked charges: 130 files × 256 records = 33,280 charges
// Pages needed: 1 (at least 8 charges) + ceil((33280 - 8) / 32) + 1 (possible split for the summary) = 1042 pages
//             + 15 summary pages (256 users + 256 chargers in subtotal tables, ~50 rows per page)
// Objects: 4 header (none, info, 2 fonts)
//        + 1057 pages × 4 (page, frame stream, image stream, image)
//        + ceil(33280 / 8) + 1 = 4161 table content streams
//        + (256 + 256) / 8 + 2 = 66 subtotal row streams
//        + 2 footer (pages, catalog)
//        = 8461
// WARP1 (7680 charges, 256 users): 4 + (243 + 6) × 4 + 961 + 34 + 2 = 1997 < 2400
#define PDF_MAX_OBJECTS 10100
#endif
#define PDF_MAX_OBJECTS_PER_PAGE 100

#define RGB_R(c) (((c) >> 16) & 0xff)
#define RGB_G(c) (((c) >> 8) & 0xff)
#define RGB_B(c) (((c) >> 0) & 0xff)

#define PDF_RGB_R(c) (float)((((c) >> 16) & 0xff) / 255.0)
#define PDF_RGB_G(c) (float)((((c) >> 8) & 0xff) / 255.0)
#define PDF_RGB_B(c) (float)((((c) >> 0) & 0xff) / 255.0)

#if defined(_MSC_VER)
#define inline __inline
#define snprintf _snprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#ifdef stat
#undef stat
#endif
#define stat _stat
#define SKIP_ATTRIBUTE
#else
#include <strings.h> // strcasecmp
#endif

#include <arpa/inet.h>

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


// Limits on image sizes for sanity checking & to avoid plausible overflow
// issues
#define MAX_IMAGE_WIDTH (16 * 1024)
#define MAX_IMAGE_HEIGHT (16 * 1024)

// Signatures for various image formats
static const uint8_t png_signature[] = {0x89, 0x50, 0x4E, 0x47,
                                        0x0D, 0x0A, 0x1A, 0x0A};

// Special signatures for PNG chunks
static const char png_chunk_header[] = "IHDR";
static const char png_chunk_palette[] = "PLTE";
static const char png_chunk_transparency[] = "tRNS";
static const char png_chunk_data[] = "IDAT";
static const char png_chunk_end[] = "IEND";

typedef struct pdf_object pdf_object;

enum {
    OBJ_none, /* skipped */
    OBJ_info,
    OBJ_stream,
    OBJ_font,
    OBJ_page,
    OBJ_catalog,
    OBJ_pages,
    OBJ_image,
    OBJ_imagestream,

    OBJ_count,
};

/**
 * Simple dynamic string object. Tries to store a reasonable amount on the
 * stack before falling back to malloc once things get large
 */
struct dstr {
    char static_data[128];
    char *data;
    size_t alloc_len;
    size_t used_len;
};

struct image_stream_t {
    float width;
    float height;
};

struct image_t {
    // Used for image stream, but this "balances" the union member size.
    float x;
    float y;
};

struct page_t {
    uint16_t stream_count;
    uint16_t image_count;
    int page_number;
};

struct font_t {
    const char *name;
    int index;
};

struct pdf_object {
    int type:8;                /* See OBJ_xxxx */
    int index:24;               /* PDF output index */
    int page_id;
    union {
        struct image_stream_t image_stream;
        struct image_t image;
        struct page_t page;
        struct pdf_info *info;
        struct font_t font;
    };
};

struct pdf_doc {
    char errstr[128];
    int errval;
    std::unique_ptr<struct pdf_object[]> objects;
    std::unique_ptr<uint16_t[]> offsets;
    std::vector<int> page_indices;
    size_t objects_in_use = 0;
    size_t offsets_in_use = 0;
    int current_page_id = 0;
    int pages_index = 0;
    int page_count = 0;
    int first_object_index = 0;
    bool write_error_occurred;

    int page_number = 0;

    float width;
    float height;

    struct pdf_object *current_font;
    int font_obj_index[PDF_FONT_COUNT];

    std::function<ssize_t(const void *buf, size_t len)> write_fn;
    std::function<int(struct pdf_doc *pdf, uint32_t page_num, uint32_t stream_num)> stream_fn;
    std::function<int(struct pdf_doc *pdf, uint32_t page_num, uint32_t image_num)> image_fn;
    std::function<int(struct pdf_doc *pdf, uint32_t page_num)> page_fn;

    std::unique_ptr<char[]> write_buf;
    size_t write_buf_size;
    size_t write_buf_used;
    size_t write_buf_written = 0;
    size_t last_write_buf_written = 0;

    struct dstr scratch_str;

    struct {
        int current_obj_index;
        bool is_image;
    } callback_context;

    struct {
        bool in_text;
        int32_t line_x; // Start of the current text line in 1/100 pt (positions are written with 2 decimals)
        int32_t line_y;
        int font;
        float font_size;
        uint32_t fill_colour;
        uint32_t stroke_colour;
        float line_width;
    } sb;
};

/**
 * Since we're casting random areas of memory to these, make sure
 * they're packed properly to match the image format requirements
 */
#pragma pack(push, 1)
struct png_chunk {
    uint32_t length;
    // chunk type, see png_chunk_header, png_chunk_data, png_chunk_end
    char type[4];
};

#pragma pack(pop)

/**
 * Simple dynamic string object. Tries to store a reasonable amount on the
 * stack before falling back to malloc once things get large
 */

#define INIT_DSTR                                                            \
    (struct dstr)                                                            \
    {                                                                        \
        .static_data = {0}, .data = nullptr, .alloc_len = 0, .used_len = 0      \
    }

static char *dstr_data(struct dstr *str)
{
    return str->data ? str->data : str->static_data;
}

static size_t dstr_len(const struct dstr *str)
{
    return str->used_len;
}

static ssize_t dstr_ensure(struct dstr *str, size_t len)
{
    if (len <= str->alloc_len)
        return 0;
    if (!str->data && len <= sizeof(str->static_data))
        str->alloc_len = len;
    else if (str->alloc_len < len) {
        size_t new_len;

        new_len = std::max(len + 1024, (size_t)2048); // generating a "full" pdf was observed to require up to 2029 bytes

        if (str->data) {
            char *new_data = (char *)realloc((void *)str->data, new_len);
            if (!new_data)
                return -ENOMEM;
            str->data = new_data;
        } else {
            str->data = (char *)malloc(new_len);
            if (!str->data)
                return -ENOMEM;
            if (str->used_len)
                memcpy(str->data, str->static_data, str->used_len + 1);
        }

        str->alloc_len = new_len;
    }
    return 0;
}

// Locales can replace the decimal character with a ','.
// This breaks the PDF output, so we force a 'safe' locale.
static void force_locale(char *buf, int len)
{
    char *saved_locale = setlocale(LC_ALL, nullptr);

    if (!saved_locale) {
        *buf = '\0';
    } else {
        strncpy(buf, saved_locale, len - 1);
        buf[len - 1] = '\0';
    }

    setlocale(LC_NUMERIC, "POSIX");
}

static void restore_locale(char *buf)
{
    setlocale(LC_ALL, buf);
}

#ifndef SKIP_ATTRIBUTE
[[gnu::format(printf, 2, 3)]]
static int dstr_printf(struct dstr *str, const char *fmt, ...);
#endif
static int dstr_printf(struct dstr *str, const char *fmt, ...)
{
    va_list ap, aq;
    int len;
    char saved_locale[32];

    force_locale(saved_locale, sizeof(saved_locale));

    va_start(ap, fmt);
    va_copy(aq, ap);
    len = vsnprintf(nullptr, 0, fmt, ap);
    if (dstr_ensure(str, str->used_len + len + 1) < 0) {
        va_end(ap);
        va_end(aq);
        restore_locale(saved_locale);
        return -ENOMEM;
    }
    vsprintf(dstr_data(str) + str->used_len, fmt, aq);
    str->used_len += len;
    va_end(ap);
    va_end(aq);
    restore_locale(saved_locale);

    return len;
}

static ssize_t dstr_append_data(struct dstr *str, const void *extend,
                                size_t len)
{
    if (dstr_ensure(str, str->used_len + len + 1) < 0)
        return -ENOMEM;
    memcpy(dstr_data(str) + str->used_len, extend, len);
    str->used_len += len;
    dstr_data(str)[str->used_len] = '\0';
    return len;
}

static ssize_t dstr_append(struct dstr *str, const char *extend)
{
    return dstr_append_data(str, extend, strlen(extend));
}

static void dstr_free(struct dstr *str)
{
    if (str->data)
        free(str->data);
    *str = INIT_DSTR;
}

/**
 * PDF Implementation
 */

#ifndef SKIP_ATTRIBUTE
[[gnu::format(printf, 3, 4)]]
static int pdf_set_err(struct pdf_doc *doc, int errval, const char *buffer,
                       ...);
#endif
static int pdf_set_err(struct pdf_doc *doc, int errval, const char *buffer,
                       ...)
{
    va_list ap;
    int len;

    va_start(ap, buffer);
    len = vsnprintf(doc->errstr, sizeof(doc->errstr) - 1, buffer, ap);
    va_end(ap);

    if (len < 0) {
        doc->errstr[0] = '\0';
        return errval;
    }

    if (len >= (int)(sizeof(doc->errstr) - 1))
        len = (int)(sizeof(doc->errstr) - 1);

    doc->errstr[len] = '\0';
    doc->errval = errval;

    return errval;
}

const char *pdf_get_err(const struct pdf_doc *pdf, int *errval)
{
    if (!pdf)
        return nullptr;
    if (pdf->errstr[0] == '\0')
        return nullptr;
    if (errval)
        *errval = pdf->errval;
    return pdf->errstr;
}

void pdf_clear_err(struct pdf_doc *pdf)
{
    if (!pdf)
        return;
    pdf->errstr[0] = '\0';
    pdf->errval = 0;
}

static struct pdf_object *pdf_get_object(const struct pdf_doc *pdf, int index)
{
    if (index < pdf->first_object_index)
        printf("!!!\n");
    return (struct pdf_object *)&pdf->objects[index - pdf->first_object_index];
}

static struct pdf_object *pdf_append_object(struct pdf_doc *pdf, struct pdf_object *obj)
{
    if (pdf->objects_in_use >= PDF_MAX_OBJECTS_PER_PAGE)
        return nullptr;

    obj->index = pdf->objects_in_use + pdf->first_object_index;
    pdf->objects[pdf->objects_in_use++] = *obj;
    return &pdf->objects[pdf->objects_in_use - 1];
}

static void pdf_object_destroy(struct pdf_object *object)
{
    switch (object->type) {
        case OBJ_info:
            free(object->info);
            object->info = nullptr;
            break;
        default:
            break;
    }
}

static struct pdf_object *pdf_add_object(struct pdf_doc *pdf, int type)
{
    struct pdf_object obj;

    if (!pdf)
        return nullptr;

    obj.page_id = -1;
    obj.type = type;

    switch (obj.type) {
        case OBJ_info:
            obj.info = (struct pdf_info *)calloc(1, sizeof(*obj.info));
            break;
    }

    return pdf_append_object(pdf, &obj);
}

struct pdf_doc *pdf_create(float width, float height, const struct pdf_info *info, size_t write_buf_size)
{
    struct pdf_doc *pdf;
    struct pdf_object *obj;

    pdf = new struct pdf_doc();
    if (!pdf)
        return nullptr;
    pdf->scratch_str = INIT_DSTR;

    pdf->width = width;
    pdf->height = height;
    pdf->objects = std::unique_ptr<struct pdf_object[]>(new struct pdf_object[PDF_MAX_OBJECTS_PER_PAGE]());
    pdf->offsets = std::unique_ptr<uint16_t[]>(new uint16_t[PDF_MAX_OBJECTS]());
    pdf->write_buf = std::unique_ptr<char[]>(new char[write_buf_size]());
    pdf->write_buf_size = write_buf_size;

    /* We don't want to use ID 0 */
    pdf_add_object(pdf, OBJ_none);

    /* Create the 'info' object */
    obj = pdf_add_object(pdf, OBJ_info);
    if (!obj) {
        pdf_destroy(pdf);
        return nullptr;
    }

    *obj->info = *info;
    obj->info->creator[sizeof(obj->info->creator) - 1] = '\0';
    obj->info->producer[sizeof(obj->info->producer) - 1] = '\0';
    obj->info->title[sizeof(obj->info->title) - 1] = '\0';
    obj->info->author[sizeof(obj->info->author) - 1] = '\0';
    obj->info->subject[sizeof(obj->info->subject) - 1] = '\0';
    obj->info->date[sizeof(obj->info->date) - 1] = '\0';

    /* FIXME: Should be quoting PDF strings? */
    if (!obj->info->date[0]) {
        time_t now = time(nullptr);
        struct tm tm;
        localtime_r(&now, &tm);

        // Only set the creation date if the time is synced.
        if (tm.tm_year + 1900 >= 2024) {
            // PDF date format: D:YYYYMMDDHHmmSSOHH'mm
            char offset[8] = "";
            strftime(offset, sizeof(offset), "%z", &tm); // +hhmm
            size_t len = strftime(obj->info->date, sizeof(obj->info->date), "%Y%m%d%H%M%S", &tm);
            if ((strlen(offset) == 5) && ((len + 8) < sizeof(obj->info->date))) {
                snprintf(obj->info->date + len, sizeof(obj->info->date) - len, "%c%c%c'%c%c'", offset[0], offset[1], offset[2], offset[3], offset[4]);
            }
        }
    }

    // The largest content stream of the charge log is about 3.2 KiB.
    if (dstr_ensure(&pdf->scratch_str, 3 * 1024) < 0) {
        pdf_destroy(pdf);
        return nullptr;
    }

    // Register all fonts up front. They are header objects and therefore have fixed indices.
    static const char *const font_names[PDF_FONT_COUNT] = {PDF_FONT_NAME_REGULAR, PDF_FONT_NAME_BOLD};
    for (int i = PDF_FONT_COUNT - 1; i >= 0; --i) {
        if (pdf_set_font(pdf, font_names[i]) < 0) {
            pdf_destroy(pdf);
            return nullptr;
        }
        pdf->font_obj_index[i] = pdf->current_font->index;
    }

    return pdf;
}

float pdf_width(const struct pdf_doc *pdf)
{
    return pdf->width;
}

float pdf_height(const struct pdf_doc *pdf)
{
    return pdf->height;
}

void pdf_destroy(struct pdf_doc *pdf)
{
    if (pdf) {
        for (ssize_t i = 0; i < pdf->objects_in_use; ++i)
            pdf_object_destroy(&pdf->objects[i]);

        dstr_free(&pdf->scratch_str);
        delete pdf;
    }
}

static struct pdf_object *pdf_find_first_object(struct pdf_doc *pdf,
                                                int type)
{
    if (!pdf)
        return nullptr;

    for (ssize_t i = 0; i < pdf->objects_in_use; ++i)
        if (pdf->objects[i].type == type)
            return &pdf->objects[i];

    return nullptr;
}

static struct pdf_object *pdf_find_next_object(struct pdf_doc *pdf, struct pdf_object *last, int type)
{
    if (!pdf)
        return nullptr;

    ssize_t start_offset = last - pdf->objects.get() + 1;

    for (ssize_t i = start_offset; i < pdf->objects_in_use; ++i)
        if (pdf->objects[i].type == type)
            return &pdf->objects[i];

    return nullptr;
}

int pdf_set_font(struct pdf_doc *pdf, const char *font)
{
    struct pdf_object *obj;
    int last_index = 0;

    /* See if we've used this font before */
    for (obj = pdf_find_first_object(pdf, OBJ_font); obj; obj = pdf_find_next_object(pdf, obj, OBJ_font)) {
        if (strcmp(obj->font.name, font) == 0)
            break;
        last_index = obj->font.index;
    }

    /* Create a new font object if we need it */
    if (!obj) {
        obj = pdf_add_object(pdf, OBJ_font);
        if (!obj)
            return pdf->errval;
        obj->font.name = font;
        obj->font.index = last_index + 1;
    }

    pdf->current_font = obj;

    return 0;
}

void pdf_notify_page(struct pdf_doc *pdf, uint32_t stream_count, uint32_t image_count) {
    if (pdf->pages_index == 0) {
        pdf->pages_index += pdf->objects_in_use;
    }

    pdf->pages_index += 1 // page
                       + stream_count // streams
                       + image_count * 2;  // images and imagestreams

    ++pdf->page_count;
}

struct pdf_object *pdf_append_page(struct pdf_doc *pdf, uint32_t stream_count, uint32_t image_count)
{
    struct pdf_object *page;

    page = pdf_add_object(pdf, OBJ_page);

    if (!page)
        return nullptr;

    pdf->page_indices.push_back(page->index);
    page->page.stream_count = stream_count;
    page->page.image_count = image_count;
    page->page.page_number = pdf->page_number++;

    return page;
}

static void pdf_add_page(struct pdf_doc *pdf, struct pdf_object *page) {
    for (size_t i = 0; i < page->page.stream_count; ++i) {
        struct pdf_object *obj = pdf_add_object(pdf, OBJ_stream);
        obj->page_id = page->index;
    }

    for (size_t i = 0; i < page->page.image_count; ++i) {
        struct pdf_object *obj = pdf_add_object(pdf, OBJ_imagestream);
        obj->page_id = page->index;
    }

    for (size_t i = 0; i < page->page.image_count; ++i) {
        struct pdf_object *obj = pdf_add_object(pdf, OBJ_image);
        obj->page_id = page->index;
    }
}

static void pdf_flush_write_buf(struct pdf_doc *pdf, int target_free_space) {
    auto *head = pdf->write_buf.get();
    target_free_space = std::max(target_free_space, 128);

    if (target_free_space > pdf->write_buf_size)
        target_free_space = pdf->write_buf_size;

    while (target_free_space > (pdf->write_buf_size - pdf->write_buf_used)) {
        ssize_t written = pdf->write_fn(head, pdf->write_buf_used);
        if (written <= 0) {
            pdf->write_error_occurred = true;
            return;
        }
        pdf->write_buf_used -= written;
        head += written;
    }

    if (pdf->write_buf_used != 0 && head != pdf->write_buf.get()) {
        memmove(pdf->write_buf.get(), head, pdf->write_buf_used);
    }
}

static int pdf_printf(struct pdf_doc *pdf, const char *fmt, ...)
{
    if (pdf->write_error_occurred)
        return 0;

    va_list ap, aq;
    int len;
    char saved_locale[32];

    force_locale(saved_locale, sizeof(saved_locale));

    const size_t write_buf_remaining = pdf->write_buf_size - pdf->write_buf_used;

    va_start(ap, fmt);
    va_copy(aq, ap);
    len = vsnprintf(pdf->write_buf.get() + pdf->write_buf_used, write_buf_remaining, fmt, ap);
    if (len > pdf->write_buf_size) {
        printf("write buf too small! %u, but required are %d.\n", pdf->write_buf_size, len);
        pdf->write_error_occurred = true;
        return 0;
    }

    if (len >= write_buf_remaining) {
        pdf_flush_write_buf(pdf, len + 1); // Include termination

        // Must check for write errors again because the flag might have been set by pdf_flush_write_buf,
        // which means that there probably isn't enough room inside the output buffer.
        if (pdf->write_error_occurred)
            return 0;

        vsprintf(pdf->write_buf.get() + pdf->write_buf_used, fmt, aq);
    }
    pdf->write_buf_used += len;
    pdf->write_buf_written += len;
    va_end(ap);
    va_end(aq);
    restore_locale(saved_locale);

    return len;
}

static void pdf_write(struct pdf_doc *pdf, const char *buf, size_t count) {
    if (pdf->write_error_occurred)
        return;

    while (count > 0) {
        pdf_flush_write_buf(pdf, count);

        // Must check for write errors inside the loop because the flag can also be set by pdf_flush_write_buf.
        if (pdf->write_error_occurred)
            return;

        size_t to_write = min(count, pdf->write_buf_size - pdf->write_buf_used);
        memcpy(pdf->write_buf.get() + pdf->write_buf_used, buf, to_write);
        pdf->write_buf_used += to_write;
        pdf->write_buf_written += to_write;
        count -= to_write;
        buf += to_write;
    }
}

static struct pdf_object *pdf_get_page(struct pdf_doc *pdf, struct pdf_object *object) {
    if (object->page_id <= 0) {
        printf("can't get page: object not a child of a page!\n");
        exit(1);
    }
    return pdf_get_object(pdf, object->page_id);
}

static int pdf_add_image(struct pdf_doc *pdf, struct pdf_object *page,
                         struct pdf_object *image, struct pdf_object *image_stream, float x, float y,
                         float width, float height);

// Writes a UTF-8 string as PDFDocEncoding text string. PDFDocEncoding matches Latin-1 for the printable characters >= 0xA0.
static void pdf_write_info_string(struct pdf_doc *pdf, const char *key, const char *utf8)
{
    char buf[2 * 64 + 1];
    size_t used = 0;
    const int len = (int)strlen(utf8);

    for (int i = 0; (i < len) && ((used + 2) < sizeof(buf));) {
        uint32_t code = 0;
        int code_len = pdf_utf8_to_utf32(&utf8[i], len - i, &code);
        if (code_len <= 0) {
            ++i;
            continue;
        }
        i += code_len;

        char c;
        if ((code >= 0x20) && (code < 0x7F)) {
            c = (char)code;
        } else if ((code >= 0xA0) && (code <= 0xFF)) {
            c = (char)code;
        } else {
            c = '?';
        }

        if ((c == '(') || (c == ')') || (c == '\\')) {
            buf[used++] = '\\';
        }
        buf[used++] = c;
    }
    buf[used] = '\0';

    pdf_printf(pdf, "  /%s (%s)\r\n", key, buf);
}

static int pdf_save_object(struct pdf_doc *pdf, int index)
{
    struct pdf_object *object = pdf_get_object(pdf, index);
    if (!object)
        return -ENOENT;

    if (object->type == OBJ_none)
        return -ENOENT;

    // printf("objects ptr: %p, offsets ptr: %p\n", (void *)pdf->objects.get(), (void *)pdf->offsets.get());

    if (pdf->offsets_in_use >= PDF_MAX_OBJECTS) {
        printf("ERROR: offsets array overflow! offsets_in_use=%zu, max=%d\n", pdf->offsets_in_use, PDF_MAX_OBJECTS);
        return -ENOMEM;
    }

    pdf->offsets[pdf->offsets_in_use++] = pdf->write_buf_written - pdf->last_write_buf_written;
    pdf->last_write_buf_written = pdf->write_buf_written;

    pdf_printf(pdf, "%d 0 obj\r\n", index);

    //struct pdf_object *page = pdf_get_page(pdf, object);

    switch (object->type) {
    case OBJ_stream:{
        struct pdf_object *page = pdf_get_page(pdf, object);

        pdf->callback_context.current_obj_index = object->index;
        pdf->callback_context.is_image = false;
        pdf->stream_fn(pdf, page->page.page_number, object->index - page->index - 1);
        pdf->callback_context.current_obj_index = -1;

        break;
    }
    case OBJ_imagestream: {
        struct pdf_object *page = pdf_get_page(pdf, object);

        pdf->callback_context.current_obj_index = object->index;
        pdf->callback_context.is_image = true;
        pdf->image_fn(pdf, page->page.page_number, object->index - page->index - page->page.stream_count - 1);
        pdf->callback_context.current_obj_index = -1;
        break;
    }
    case OBJ_image: {
        struct pdf_object *page = pdf_get_page(pdf, object);
        struct pdf_object *image = pdf_get_object(pdf, object->index - page->page.image_count);
        pdf_add_image(pdf, nullptr, image, object, image->image.x, image->image.y, object->image_stream.width, object->image_stream.height);
        break;
    }
    case OBJ_info: {
        struct pdf_info *info = object->info;

        pdf_printf(pdf, "<<\r\n");
        if (info->creator[0])
            pdf_write_info_string(pdf, "Creator", info->creator);
        if (info->producer[0])
            pdf_write_info_string(pdf, "Producer", info->producer);
        if (info->title[0])
            pdf_write_info_string(pdf, "Title", info->title);
        if (info->author[0])
            pdf_write_info_string(pdf, "Author", info->author);
        if (info->subject[0])
            pdf_write_info_string(pdf, "Subject", info->subject);
        if (info->date[0])
            pdf_printf(pdf, "  /CreationDate (D:%s)\r\n", info->date);
        pdf_printf(pdf, ">>\r\n");
        break;
    }

    case OBJ_page: {
        pdf_printf(pdf,
                "<<\r\n"
                "  /Type /Page\r\n"
                "  /Parent %d 0 R\r\n",
                pdf->pages_index);
        pdf_printf(pdf, "  /MediaBox [0 0 %f %f]\r\n", pdf->width, pdf->height);
        pdf_printf(pdf, "  /Resources <<\r\n");
        pdf_printf(pdf, "    /Font <<\r\n");
        for (int i = 0; i < PDF_FONT_COUNT; ++i) {
            pdf_printf(pdf, "      /F%d %d 0 R\r\n", i + 1, pdf->font_obj_index[i]);
        }
        pdf_printf(pdf, "    >>\r\n");
        // We trim transparency to just 4-bits
        pdf_printf(pdf, "    /ExtGState <<\r\n");
        for (int i = 0; i < 16; i++) {
            pdf_printf(pdf, "      /GS%d <</ca %f>>\r\n", i,
                    (float)(15 - i) / 15);
        }
        pdf_printf(pdf, "    >>\r\n");

        if (object->page.image_count > 0) {
            pdf_printf(pdf, "    /XObject <<");
            for (size_t i = 0; i < object->page.image_count; ++i)
                pdf_printf(pdf, "      /Image%d %d 0 R ", object->index + 1 + object->page.stream_count + i, object->index + 1 + object->page.stream_count + i);
            pdf_printf(pdf, "    >>\r\n");
        }
        pdf_printf(pdf, "  >>\r\n");

        pdf_printf(pdf, "  /Contents [\r\n");

        for (size_t i = 0; i < object->page.stream_count; ++i) {
            pdf_printf(pdf, "%d 0 R\r\n", object->index + 1 + i);
        }

         for (size_t i = 0; i < object->page.image_count; ++i) {
            pdf_printf(pdf, "%d 0 R\r\n", object->index + 1 + object->page.stream_count + object->page.image_count + i);
        }

        pdf_printf(pdf, "]\r\n");

        pdf_printf(pdf, ">>\r\n");
        break;
    }

    case OBJ_font:
        pdf_printf(pdf,
                "<<\r\n"
                "  /Type /Font\r\n"
                "  /Subtype /Type1\r\n"
                "  /BaseFont /%s\r\n"
                "  /Encoding /WinAnsiEncoding\r\n"
                ">>\r\n",
                object->font.name);
        break;

    case OBJ_pages: {
        int npages = 0;

        pdf_printf(pdf, "<<\r\n"
                    "  /Type /Pages\r\n"
                    "  /Kids [ ");
        for(int i : pdf->page_indices) {
            pdf_printf(pdf, "%d 0 R ", i);
            npages++;
        }
        /*for (struct pdf_object *page = pdf_find_first_object(pdf, OBJ_page); page; page = pdf_find_next_object(pdf, page, OBJ_page)) {
            npages++;
            pdf_printf(pdf, "%d 0 R ", page->index);
        }*/

        pdf_printf(pdf, "]\r\n");
        pdf_printf(pdf, "  /Count %d\r\n", npages);
        pdf_printf(pdf, ">>\r\n");
        break;
    }

    case OBJ_catalog: {
        struct pdf_object *pages = pdf_find_first_object(pdf, OBJ_pages);

        pdf_printf(pdf, "<<\r\n"
                    "  /Type /Catalog\r\n");
        pdf_printf(pdf,
                "  /Pages %d 0 R\r\n"
                ">>\r\n",
                pages->index);
        break;
    }

    default:
        return pdf_set_err(pdf, -EINVAL, "Invalid PDF object type %d",
                           object->type);
    }

    pdf_printf(pdf, "endobj\r\n");

    return 0;
}

// Slightly modified djb2 hash algorithm to get pseudo-random ID
static uint64_t hash(uint64_t hash, const void *data, size_t len)
{
    const uint8_t *d8 = (const uint8_t *)data;
    for (; len; len--) {
        hash = (((hash & 0x03ffffffffffffff) << 5) +
                (hash & 0x7fffffffffffffff)) +
               *d8++;
    }
    return hash;
}

int pdf_save_file(struct pdf_doc *pdf)
{
    struct pdf_object *obj;
    int xref_offset;
    int xref_count = 0;
    uint64_t id1, id2;
    time_t now = time(nullptr);
    char saved_locale[32];

    force_locale(saved_locale, sizeof(saved_locale));

    pdf_printf(pdf, "%%PDF-1.3\r\n");
    /* Hibit bytes */
    pdf_printf(pdf, "%c%c%c%c%c\r\n", 0x25, 0xc7, 0xec, 0x8f, 0xa2);

    id1 = hash(5381, &pdf->objects[1], sizeof(struct pdf_info));

    int i = 0;
    // dump header objects (OBJ_none, OBJ_info, OBJ_font)
    for (; i < pdf->objects_in_use; i++) {
        if (pdf_save_object(pdf, i) >= 0)
            xref_count++;
    }

    pdf->page_indices.reserve(pdf->page_count);
    for (int p = 0; p < pdf->page_count; ++p) {
        if (pdf->write_error_occurred)
            return -1;

        pdf->page_fn(pdf, p);

        // dump page objects
        for (; i < pdf->objects_in_use + pdf->first_object_index; i++) {
            obj = pdf_get_object(pdf, i);

            if (obj->type == OBJ_page) {
                pdf_add_page(pdf, obj);
            }

            if (pdf_save_object(pdf, i) >= 0)
                xref_count++;


            if (obj->type == OBJ_page) {
                auto to_delete = obj->index - pdf->current_page_id;
                for (int j = 0; j < to_delete; ++j) {
                    pdf_object_destroy(pdf_get_object(pdf, j + pdf->current_page_id));
                }
                auto page_idx = obj->index;
                memmove(pdf->objects.get(), pdf->objects.get() + to_delete, sizeof(struct pdf_object) * (PDF_MAX_OBJECTS_PER_PAGE - to_delete));
                pdf->current_page_id = page_idx;
                pdf->first_object_index = page_idx;
                pdf->objects_in_use -= to_delete;
            }
        }
    }

/*
    //Dump all the objects & get their file offsets
    for (int i = 0; i < pdf->objects_in_use; i++) {
        obj = pdf_get_object(pdf, i);
        printf("!!!%d\n", obj->type);
        if (obj->type == OBJ_page) {
            pdf_add_page(pdf, obj);
        }

        if (pdf_save_object(pdf, i) >= 0)
            xref_count++;

        if (obj->type == OBJ_page) {
            if (pdf->current_page_id != 0) {
                for(int j = pdf->current_page_id; j < obj->index; ++j) {
                    pdf_object_destroy(pdf_get_object(pdf, j));
                }
            }
            pdf->current_page_id = obj->index;
        }
    }
*/

    // Insert the pages and catalog objects last, we will know all page offsets by now.
    auto *pages = pdf_add_object(pdf, OBJ_pages);
    if (!pages) {
        return -1;
    }
    if (pdf_save_object(pdf, pages->index) >= 0)
        xref_count++;

    auto *catalog = pdf_add_object(pdf, OBJ_catalog);
    if (!catalog) {
        return -1;
    }
    if (pdf_save_object(pdf, catalog->index) >= 0)
        xref_count++;

    /* xref */
    xref_offset = pdf->write_buf_written;
    pdf_printf(pdf, "xref\r\n");
    pdf_printf(pdf, "0 %d\r\n", xref_count + 1);
    pdf_printf(pdf, "0000000000 65535 f\r\n");
    int offset = 0;
    for (int o = 0; o < pdf->offsets_in_use; o++) {
        offset += pdf->offsets[o];
        pdf_printf(pdf, "%10.10d 00000 n\r\n", offset);
    }

    pdf_printf(pdf,
            "trailer\r\n"
            "<<\r\n"
            "/Size %d\r\n",
            xref_count + 1);
    obj = pdf_find_first_object(pdf, OBJ_catalog);
    pdf_printf(pdf, "/Root %d 0 R\r\n", obj->index);
    //obj = pdf_find_first_object(pdf, OBJ_info);
    pdf_printf(pdf, "/Info %d 0 R\r\n", 1 /*obj->index*/);
    /* Generate document unique IDs */

    id1 = hash(id1, &xref_count, sizeof(xref_count));
    id2 = hash(5381, &now, sizeof(now));
    pdf_printf(pdf, "/ID [<%16.16" PRIx64 "> <%16.16" PRIx64 ">]\r\n", id1, id2);
    pdf_printf(pdf, ">>\r\n"
                "startxref\r\n");
    pdf_printf(pdf, "%d\r\n", xref_offset);
    pdf_printf(pdf, "%%%%EOF\r\n");

    restore_locale(saved_locale);

    pdf_flush_write_buf(pdf, pdf->write_buf_size);

    return pdf->write_error_occurred ? -1 : 0;
}

static int pdf_add_stream(struct pdf_doc *pdf, const char *buffer)
{
    size_t len;

    len = strlen(buffer);
    /* We don't want any trailing whitespace in the stream */
    while (len >= 1 && (buffer[len - 1] == '\r' || buffer[len - 1] == '\n'))
        len--;

    pdf_printf(pdf, "<< /Length %zu >>stream\r\n", len);
    pdf_write(pdf, buffer, len);
    pdf_printf(pdf, "\r\nendstream\r\n");

    return 0;
}

int pdf_utf8_to_utf32(const char *utf8, int len, uint32_t *utf32)
{
    uint32_t ch;
    uint8_t mask;

    if (len <= 0 || !utf8 || !utf32)
        return -EINVAL;

    ch = *(uint8_t *)utf8;
    if ((ch & 0x80) == 0) {
        len = 1;
        mask = 0x7f;
    } else if ((ch & 0xe0) == 0xc0 && len >= 2) {
        len = 2;
        mask = 0x1f;
    } else if ((ch & 0xf0) == 0xe0 && len >= 3) {
        len = 3;
        mask = 0xf;
    } else if ((ch & 0xf8) == 0xf0 && len >= 4) {
        len = 4;
        mask = 0x7;
    } else
        return -EINVAL;

    ch = 0;
    for (int i = 0; i < len; i++) {
        int shift = (len - i - 1) * 6;
        if (!*utf8)
            return -EINVAL;
        if (i == 0)
            ch |= ((uint32_t)(*utf8++) & mask) << shift;
        else
            ch |= ((uint32_t)(*utf8++) & 0x3f) << shift;
    }

    *utf32 = ch;

    return len;
}

int pdf_utf8_to_pdfencoding(const char *utf8, int len, uint8_t *res)
{
    uint32_t code;
    int code_len;

    *res = 0;

    code_len = pdf_utf8_to_utf32(utf8, len, &code);
    if (code_len < 0) {
        return -EINVAL;
    }

    if (code > 255) {
        /* We support *some* minimal UTF-8 characters */
        // See Appendix D of
        // https://opensource.adobe.com/dc-acrobat-sdk-docs/pdfstandards/pdfreference1.7old.pdf
        // These are all in WinAnsiEncoding
        switch (code) {
        case 0x152: // Latin Capital Ligature OE
            *res = 0214;
            break;
        case 0x153: // Latin Small Ligature oe
            *res = 0234;
            break;
        case 0x160: // Latin Capital Letter S with caron
            *res = 0212;
            break;
        case 0x161: // Latin Small Letter S with caron
            *res = 0232;
            break;
        case 0x178: // Latin Capital Letter y with diaeresis
            *res = 0237;
            break;
        case 0x17d: // Latin Capital Letter Z with caron
            *res = 0216;
            break;
        case 0x17e: // Latin Small Letter Z with caron
            *res = 0236;
            break;
        case 0x192: // Latin Small Letter F with hook
            *res = 0203;
            break;
        case 0x2c6: // Modifier Letter Circumflex Accent
            *res = 0210;
            break;
        case 0x2dc: // Small Tilde
            *res = 0230;
            break;
        case 0x2013: // Endash
            *res = 0226;
            break;
        case 0x2014: // Emdash
            *res = 0227;
            break;
        case 0x2018: // Left Single Quote
            *res = 0221;
            break;
        case 0x2019: // Right Single Quote
            *res = 0222;
            break;
        case 0x201a: // Single low-9 Quotation Mark
            *res = 0202;
            break;
        case 0x201c: // Left Double Quote
            *res = 0223;
            break;
        case 0x201d: // Right Double Quote
            *res = 0224;
            break;
        case 0x201e: // Double low-9 Quotation Mark
            *res = 0204;
            break;
        case 0x2020: // Dagger
            *res = 0206;
            break;
        case 0x2021: // Double Dagger
            *res = 0207;
            break;
        case 0x2022: // Bullet
            *res = 0225;
            break;
        case 0x2026: // Horizontal Ellipsis
            *res = 0205;
            break;
        case 0x2030: // Per Mille Sign
            *res = 0211;
            break;
        case 0x2039: // Single Left-pointing Angle Quotation Mark
            *res = 0213;
            break;
        case 0x203a: // Single Right-pointing Angle Quotation Mark
            *res = 0233;
            break;
        case 0x20ac: // Euro
            *res = 0200;
            break;
        case 0x2122: // Trade Mark Sign
            *res = 0231;
            break;
        default:
            // Replace unknown unicode code points with '•' as per Appendix D:
            /*
            In WinAnsiEncoding, all unused codes greater than 40 map to the bullet character.
            However, only code 225 is specifically assigned to the bullet character; other codes are
            subject to future reassignment
            */
            *res = 0225;
            break;
        }
    } else if (code >= 0x80 && code <= 0x9F) {
        // Replace unicode control characters that are not in cp1252 with '•'.
        // See above and https://en.wikipedia.org/wiki/ISO/IEC_8859-1#Code_page_layout
        // Other control characters (code >= 0 && code <= 0x1F) are passed through.
        *res = 0225;
    } else {
        *res = code;
    }
    return code_len;
}

static int pdf_add_text_spacing(struct pdf_doc *pdf, struct pdf_object *page,
                                const char *text, float size, float xoff,
                                float yoff, uint32_t colour, float spacing)
{
    int ret;
    size_t len = text ? strlen(text) : 0;
    struct dstr str = INIT_DSTR;
    int alpha = (colour >> 24) >> 4;

    /* Don't bother adding empty/null strings */
    if (!len)
        return 0;

    dstr_append(&str, "BT ");
    dstr_printf(&str, "/GS%d gs ", alpha);
    dstr_printf(&str, "%f %f TD ", xoff, yoff);
    dstr_printf(&str, "/F%d %f Tf ", 1/*pdf->current_font->font.index*/, size);
    dstr_printf(&str, "%f %f %f rg ", PDF_RGB_R(colour), PDF_RGB_G(colour),
                PDF_RGB_B(colour));
    dstr_printf(&str, "%f Tc ", spacing);
    dstr_append(&str, "(");

    /* Escape magic characters properly */
    for (size_t i = 0; i < len;) {
        int code_len;
        uint8_t pdf_char;
        code_len = pdf_utf8_to_pdfencoding(&text[i], len - i, &pdf_char);
        if (code_len < 0) {
            dstr_free(&str);
            return pdf_set_err(pdf, code_len, "Invalid UTF-8 encoding");
        }

        if (strchr("()\\", pdf_char)) {
            char buf[3];
            /* Escape some characters */
            buf[0] = '\\';
            buf[1] = pdf_char;
            buf[2] = '\0';
            dstr_append(&str, buf);
        } else if (strrchr("\n\r\t\b\f", pdf_char)) {
            /* Skip over these characters */
            ;
        } else {
            dstr_append_data(&str, &pdf_char, 1);
        }

        i += code_len;
    }
    dstr_append(&str, ") Tj ");
    dstr_append(&str, "ET");

    ret = pdf_add_stream(pdf, dstr_data(&str));
    dstr_free(&str);
    return ret;
}

static const uint16_t *find_font_widths(const char *font_name);

static int pdf_get_font_encoded_text_truncation_point(struct pdf_doc *pdf, const char *font_name,
                            const char *text, float size, float max_width) {
    if (!font_name)
        font_name = pdf->current_font->font.name;

    const uint16_t *widths = find_font_widths(font_name);

    if (!widths)
        return pdf_set_err(pdf, -EINVAL,
                           "Unable to determine width for font '%s'",
                           pdf->current_font->font.name);

    uint32_t len = 0;
    auto text_len = strlen(text);

    for (int i = 0; i < (int)text_len; ++i) {
        uint8_t pdf_char = text[i];

        if (pdf_char != '\n' && pdf_char != '\r')
            len += widths[pdf_char];

        /* Our widths arrays are for 14pt fonts */
        if (len * size / (14.0f * 72.0f) > max_width) {
            return i - 1;
        }
    }
    return -1;
}

int pdf_add_multiple_text_spacing(struct pdf_doc *pdf, struct pdf_object *page,
                                const char * text, size_t text_lines, size_t text_cols, float size, float xoff,
                                float yoff, uint32_t colour, float spacing, float leading, const float *col_offsets, bool truncate_cells)
{
    int ret;

    int alpha = (colour >> 24) >> 4;

    pdf->scratch_str.used_len = 0;
    dstr_append(&pdf->scratch_str, "BT ");
    dstr_printf(&pdf->scratch_str, "/GS%d gs ", alpha);
    dstr_printf(&pdf->scratch_str, "/F%d %f Tf ", 1/*pdf->current_font->font.index*/, size);
    dstr_printf(&pdf->scratch_str, "%f %f %f rg ", PDF_RGB_R(colour), PDF_RGB_G(colour),
                PDF_RGB_B(colour));
    dstr_printf(&pdf->scratch_str, "%f Tc ", spacing);

    dstr_printf(&pdf->scratch_str, "%f %f Td ", xoff, yoff);
    dstr_printf(&pdf->scratch_str, "%f TL ", -leading);

    const char *text_head = text;

    for (size_t line = 0; line < text_lines; ++line) {
        for (size_t col = 0; col < text_cols; ++col) {
            dstr_printf(&pdf->scratch_str, "%f %f Td ", (col == 0) ? (line == 0 ? 0 : -col_offsets[text_cols - 1]) : col_offsets[col] - col_offsets[col - 1], col == 0 ? -leading : 0.);
            dstr_append(&pdf->scratch_str, "(");
            size_t len = strlen(text_head);

            struct dstr inner = INIT_DSTR;
            /* Escape magic characters properly */
            for (size_t i = 0; i < len;) {
                int code_len;
                uint8_t pdf_char;
                code_len = pdf_utf8_to_pdfencoding(&text_head[i], len - i, &pdf_char);
                if (code_len < 0) {
                    dstr_free(&inner);
                    return pdf_set_err(pdf, code_len, "Invalid UTF-8 encoding");
                }

                if (strchr("()\\", pdf_char)) {
                    char buf[3];
                    /* Escape some characters */
                    buf[0] = '\\';
                    buf[1] = pdf_char;
                    buf[2] = '\0';
                    dstr_append(&inner, buf);
                } else if (strrchr("\n\r\t\b\f", pdf_char)) {
                    /* Skip over these characters */
                    ;
                } else {
                    dstr_append_data(&inner, &pdf_char, 1);
                }

                i += code_len;
            }

            if (truncate_cells) {
            // Truncate text in case it does not fit into the table column
            int trunc_idx = pdf_get_font_encoded_text_truncation_point(pdf, DEFAULT_FONT, dstr_data(&inner), size, col_offsets[col + 1] - col_offsets[col]);
            if (trunc_idx > 0) {
                dstr_data(&inner)[trunc_idx] = '\0';
                inner.used_len = trunc_idx;
                dstr_data(&inner)[trunc_idx - 1] = 0x85;
            }
            }

            dstr_append(&pdf->scratch_str, dstr_data(&inner));
            dstr_free(&inner);
            dstr_append(&pdf->scratch_str, ") Tj ");
            text_head += len + 1;
        }
    }
    dstr_append(&pdf->scratch_str, "ET");

    ret = pdf_add_stream(pdf, dstr_data(&pdf->scratch_str));
    return ret;
}

int pdf_add_text(struct pdf_doc *pdf, struct pdf_object *page,
                 const char *text, float size, float xoff, float yoff,
                 uint32_t colour)
{
    return pdf_add_text_spacing(pdf, page, text, size, xoff, yoff, colour, 0);
}

/* How wide is each character, in points, at size 14 */
static const uint16_t helvetica_widths[256] = {
    280, 280, 280, 280,  280, 280, 280, 280,  280,  280, 280,  280, 280,
    280, 280, 280, 280,  280, 280, 280, 280,  280,  280, 280,  280, 280,
    280, 280, 280, 280,  280, 280, 280, 280,  357,  560, 560,  896, 672,
    192, 335, 335, 392,  588, 280, 335, 280,  280,  560, 560,  560, 560,
    560, 560, 560, 560,  560, 560, 280, 280,  588,  588, 588,  560, 1023,
    672, 672, 727, 727,  672, 615, 784, 727,  280,  504, 672,  560, 839,
    727, 784, 672, 784,  727, 672, 615, 727,  672,  951, 672,  672, 615,
    280, 280, 280, 472,  560, 335, 560, 560,  504,  560, 560,  280, 560,
    560, 223, 223, 504,  223, 839, 560, 560,  560,  560, 335,  504, 280,
    560, 504, 727, 504,  504, 504, 336, 262,  336,  588, 352,  560, 352,
    223, 560, 335, 1008, 560, 560, 335, 1008, 672,  335, 1008, 352, 615,
    352, 352, 223, 223,  335, 335, 352, 560,  1008, 335, 1008, 504, 335,
    951, 352, 504, 672,  280, 335, 560, 560,  560,  560, 262,  560, 335,
    742, 372, 560, 588,  335, 742, 335, 403,  588,  335, 335,  335, 560,
    541, 280, 335, 335,  367, 560, 840, 840,  840,  615, 672,  672, 672,
    672, 672, 672, 1008, 727, 672, 672, 672,  672,  280, 280,  280, 280,
    727, 727, 784, 784,  784, 784, 784, 588,  784,  727, 727,  727, 727,
    672, 672, 615, 560,  560, 560, 560, 560,  560,  896, 504,  560, 560,
    560, 560, 280, 280,  280, 280, 560, 560,  560,  560, 560,  560, 560,
    588, 615, 560, 560,  560, 560, 504, 560,  504,
};

static const uint16_t helvetica_bold_widths[256] = {
    280,  280, 280,  280, 280, 280, 280, 280,  280, 280, 280, 280,  280, 280,
    280,  280, 280,  280, 280, 280, 280, 280,  280, 280, 280, 280,  280, 280,
    280,  280, 280,  280, 280, 335, 477, 560,  560, 896, 727, 239,  335, 335,
    392,  588, 280,  335, 280, 280, 560, 560,  560, 560, 560, 560,  560, 560,
    560,  560, 335,  335, 588, 588, 588, 615,  982, 727, 727, 727,  727, 672,
    615,  784, 727,  280, 560, 727, 615, 839,  727, 784, 672, 784,  727, 672,
    615,  727, 672,  951, 672, 672, 615, 335,  280, 335, 588, 560,  335, 560,
    615,  560, 615,  560, 335, 615, 615, 280,  280, 560, 280, 896,  615, 615,
    615,  615, 392,  560, 335, 615, 560, 784,  560, 560, 504, 392,  282, 392,
    588,  352, 560,  352, 280, 560, 504, 1008, 560, 560, 335, 1008, 672, 335,
    1008, 352, 615,  352, 352, 280, 280, 504,  504, 352, 560, 1008, 335, 1008,
    560,  335, 951,  352, 504, 672, 280, 335,  560, 560, 560, 560,  282, 560,
    335,  742, 372,  560, 588, 335, 742, 335,  403, 588, 335, 335,  335, 615,
    560,  280, 335,  335, 367, 560, 840, 840,  840, 615, 727, 727,  727, 727,
    727,  727, 1008, 727, 672, 672, 672, 672,  280, 280, 280, 280,  727, 727,
    784,  784, 784,  784, 784, 588, 784, 727,  727, 727, 727, 672,  672, 615,
    560,  560, 560,  560, 560, 560, 896, 560,  560, 560, 560, 560,  280, 280,
    280,  280, 615,  615, 615, 615, 615, 615,  615, 588, 615, 615,  615, 615,
    615,  560, 615,  560,
};

#if 0 // Unused width tables, see find_font_widths
static const uint16_t helvetica_bold_oblique_widths[256] = {
    280,  280, 280,  280, 280, 280, 280, 280,  280, 280, 280, 280,  280, 280,
    280,  280, 280,  280, 280, 280, 280, 280,  280, 280, 280, 280,  280, 280,
    280,  280, 280,  280, 280, 335, 477, 560,  560, 896, 727, 239,  335, 335,
    392,  588, 280,  335, 280, 280, 560, 560,  560, 560, 560, 560,  560, 560,
    560,  560, 335,  335, 588, 588, 588, 615,  982, 727, 727, 727,  727, 672,
    615,  784, 727,  280, 560, 727, 615, 839,  727, 784, 672, 784,  727, 672,
    615,  727, 672,  951, 672, 672, 615, 335,  280, 335, 588, 560,  335, 560,
    615,  560, 615,  560, 335, 615, 615, 280,  280, 560, 280, 896,  615, 615,
    615,  615, 392,  560, 335, 615, 560, 784,  560, 560, 504, 392,  282, 392,
    588,  352, 560,  352, 280, 560, 504, 1008, 560, 560, 335, 1008, 672, 335,
    1008, 352, 615,  352, 352, 280, 280, 504,  504, 352, 560, 1008, 335, 1008,
    560,  335, 951,  352, 504, 672, 280, 335,  560, 560, 560, 560,  282, 560,
    335,  742, 372,  560, 588, 335, 742, 335,  403, 588, 335, 335,  335, 615,
    560,  280, 335,  335, 367, 560, 840, 840,  840, 615, 727, 727,  727, 727,
    727,  727, 1008, 727, 672, 672, 672, 672,  280, 280, 280, 280,  727, 727,
    784,  784, 784,  784, 784, 588, 784, 727,  727, 727, 727, 672,  672, 615,
    560,  560, 560,  560, 560, 560, 896, 560,  560, 560, 560, 560,  280, 280,
    280,  280, 615,  615, 615, 615, 615, 615,  615, 588, 615, 615,  615, 615,
    615,  560, 615,  560,
};

static const uint16_t helvetica_oblique_widths[256] = {
    280, 280, 280, 280,  280, 280, 280, 280,  280,  280, 280,  280, 280,
    280, 280, 280, 280,  280, 280, 280, 280,  280,  280, 280,  280, 280,
    280, 280, 280, 280,  280, 280, 280, 280,  357,  560, 560,  896, 672,
    192, 335, 335, 392,  588, 280, 335, 280,  280,  560, 560,  560, 560,
    560, 560, 560, 560,  560, 560, 280, 280,  588,  588, 588,  560, 1023,
    672, 672, 727, 727,  672, 615, 784, 727,  280,  504, 672,  560, 839,
    727, 784, 672, 784,  727, 672, 615, 727,  672,  951, 672,  672, 615,
    280, 280, 280, 472,  560, 335, 560, 560,  504,  560, 560,  280, 560,
    560, 223, 223, 504,  223, 839, 560, 560,  560,  560, 335,  504, 280,
    560, 504, 727, 504,  504, 504, 336, 262,  336,  588, 352,  560, 352,
    223, 560, 335, 1008, 560, 560, 335, 1008, 672,  335, 1008, 352, 615,
    352, 352, 223, 223,  335, 335, 352, 560,  1008, 335, 1008, 504, 335,
    951, 352, 504, 672,  280, 335, 560, 560,  560,  560, 262,  560, 335,
    742, 372, 560, 588,  335, 742, 335, 403,  588,  335, 335,  335, 560,
    541, 280, 335, 335,  367, 560, 840, 840,  840,  615, 672,  672, 672,
    672, 672, 672, 1008, 727, 672, 672, 672,  672,  280, 280,  280, 280,
    727, 727, 784, 784,  784, 784, 784, 588,  784,  727, 727,  727, 727,
    672, 672, 615, 560,  560, 560, 560, 560,  560,  896, 504,  560, 560,
    560, 560, 280, 280,  280, 280, 560, 560,  560,  560, 560,  560, 560,
    588, 615, 560, 560,  560, 560, 504, 560,  504,
};

static const uint16_t symbol_widths[256] = {
    252, 252, 252, 252,  252, 252, 252,  252, 252,  252,  252, 252, 252, 252,
    252, 252, 252, 252,  252, 252, 252,  252, 252,  252,  252, 252, 252, 252,
    252, 252, 252, 252,  252, 335, 718,  504, 553,  839,  784, 442, 335, 335,
    504, 553, 252, 553,  252, 280, 504,  504, 504,  504,  504, 504, 504, 504,
    504, 504, 280, 280,  553, 553, 553,  447, 553,  727,  672, 727, 616, 615,
    769, 607, 727, 335,  636, 727, 691,  896, 727,  727,  774, 746, 560, 596,
    615, 695, 442, 774,  650, 801, 615,  335, 869,  335,  663, 504, 504, 636,
    553, 553, 497, 442,  525, 414, 607,  331, 607,  553,  553, 580, 525, 553,
    553, 525, 553, 607,  442, 580, 718,  691, 496,  691,  497, 483, 201, 483,
    553, 0,   0,   0,    0,   0,   0,    0,   0,    0,    0,   0,   0,   0,
    0,   0,   0,   0,    0,   0,   0,    0,   0,    0,    0,   0,   0,   0,
    0,   0,   0,   0,    0,   0,   756,  624, 248,  553,  168, 718, 504, 759,
    759, 759, 759, 1050, 994, 607, 994,  607, 403,  553,  414, 553, 553, 718,
    497, 463, 553, 553,  553, 553, 1008, 607, 1008, 663,  829, 691, 801, 994,
    774, 774, 829, 774,  774, 718, 718,  718, 718,  718,  718, 718, 774, 718,
    796, 796, 897, 829,  553, 252, 718,  607, 607,  1050, 994, 607, 994, 607,
    497, 331, 796, 796,  792, 718, 387,  387, 387,  387,  387, 387, 497, 497,
    497, 497, 0,   331,  276, 691, 691,  691, 387,  387,  387, 387, 387, 387,
    497, 497, 497, 0,
};

static const uint16_t times_widths[256] = {
    252, 252, 252, 252, 252, 252, 252, 252,  252, 252, 252, 252,  252, 252,
    252, 252, 252, 252, 252, 252, 252, 252,  252, 252, 252, 252,  252, 252,
    252, 252, 252, 252, 252, 335, 411, 504,  504, 839, 784, 181,  335, 335,
    504, 568, 252, 335, 252, 280, 504, 504,  504, 504, 504, 504,  504, 504,
    504, 504, 280, 280, 568, 568, 568, 447,  928, 727, 672, 672,  727, 615,
    560, 727, 727, 335, 392, 727, 615, 896,  727, 727, 560, 727,  672, 560,
    615, 727, 727, 951, 727, 727, 615, 335,  280, 335, 472, 504,  335, 447,
    504, 447, 504, 447, 335, 504, 504, 280,  280, 504, 280, 784,  504, 504,
    504, 504, 335, 392, 280, 504, 504, 727,  504, 504, 447, 483,  201, 483,
    545, 352, 504, 352, 335, 504, 447, 1008, 504, 504, 335, 1008, 560, 335,
    896, 352, 615, 352, 352, 335, 335, 447,  447, 352, 504, 1008, 335, 987,
    392, 335, 727, 352, 447, 727, 252, 335,  504, 504, 504, 504,  201, 504,
    335, 766, 278, 504, 568, 335, 766, 335,  403, 568, 302, 302,  335, 504,
    456, 252, 335, 302, 312, 504, 756, 756,  756, 447, 727, 727,  727, 727,
    727, 727, 896, 672, 615, 615, 615, 615,  335, 335, 335, 335,  727, 727,
    727, 727, 727, 727, 727, 568, 727, 727,  727, 727, 727, 727,  560, 504,
    447, 447, 447, 447, 447, 447, 672, 447,  447, 447, 447, 447,  280, 280,
    280, 280, 504, 504, 504, 504, 504, 504,  504, 568, 504, 504,  504, 504,
    504, 504, 504, 504,
};

static const uint16_t times_bold_widths[256] = {
    252, 252, 252, 252,  252, 252, 252, 252,  252,  252,  252,  252,  252,
    252, 252, 252, 252,  252, 252, 252, 252,  252,  252,  252,  252,  252,
    252, 252, 252, 252,  252, 252, 252, 335,  559,  504,  504,  1008, 839,
    280, 335, 335, 504,  574, 252, 335, 252,  280,  504,  504,  504,  504,
    504, 504, 504, 504,  504, 504, 335, 335,  574,  574,  574,  504,  937,
    727, 672, 727, 727,  672, 615, 784, 784,  392,  504,  784,  672,  951,
    727, 784, 615, 784,  727, 560, 672, 727,  727,  1008, 727,  727,  672,
    335, 280, 335, 585,  504, 335, 504, 560,  447,  560,  447,  335,  504,
    560, 280, 335, 560,  280, 839, 560, 504,  560,  560,  447,  392,  335,
    560, 504, 727, 504,  504, 447, 397, 221,  397,  524,  352,  504,  352,
    335, 504, 504, 1008, 504, 504, 335, 1008, 560,  335,  1008, 352,  672,
    352, 352, 335, 335,  504, 504, 352, 504,  1008, 335,  1008, 392,  335,
    727, 352, 447, 727,  252, 335, 504, 504,  504,  504,  221,  504,  335,
    752, 302, 504, 574,  335, 752, 335, 403,  574,  302,  302,  335,  560,
    544, 252, 335, 302,  332, 504, 756, 756,  756,  504,  727,  727,  727,
    727, 727, 727, 1008, 727, 672, 672, 672,  672,  392,  392,  392,  392,
    727, 727, 784, 784,  784, 784, 784, 574,  784,  727,  727,  727,  727,
    727, 615, 560, 504,  504, 504, 504, 504,  504,  727,  447,  447,  447,
    447, 447, 280, 280,  280, 280, 504, 560,  504,  504,  504,  504,  504,
    574, 504, 560, 560,  560, 560, 504, 560,  504,
};

static const uint16_t times_bold_italic_widths[256] = {
    252, 252, 252, 252, 252, 252, 252, 252,  252, 252, 252, 252,  252, 252,
    252, 252, 252, 252, 252, 252, 252, 252,  252, 252, 252, 252,  252, 252,
    252, 252, 252, 252, 252, 392, 559, 504,  504, 839, 784, 280,  335, 335,
    504, 574, 252, 335, 252, 280, 504, 504,  504, 504, 504, 504,  504, 504,
    504, 504, 335, 335, 574, 574, 574, 504,  838, 672, 672, 672,  727, 672,
    672, 727, 784, 392, 504, 672, 615, 896,  727, 727, 615, 727,  672, 560,
    615, 727, 672, 896, 672, 615, 615, 335,  280, 335, 574, 504,  335, 504,
    504, 447, 504, 447, 335, 504, 560, 280,  280, 504, 280, 784,  560, 504,
    504, 504, 392, 392, 280, 560, 447, 672,  504, 447, 392, 350,  221, 350,
    574, 352, 504, 352, 335, 504, 504, 1008, 504, 504, 335, 1008, 560, 335,
    951, 352, 615, 352, 352, 335, 335, 504,  504, 352, 504, 1008, 335, 1008,
    392, 335, 727, 352, 392, 615, 252, 392,  504, 504, 504, 504,  221, 504,
    335, 752, 268, 504, 610, 335, 752, 335,  403, 574, 302, 302,  335, 580,
    504, 252, 335, 302, 302, 504, 756, 756,  756, 504, 672, 672,  672, 672,
    672, 672, 951, 672, 672, 672, 672, 672,  392, 392, 392, 392,  727, 727,
    727, 727, 727, 727, 727, 574, 727, 727,  727, 727, 727, 615,  615, 504,
    504, 504, 504, 504, 504, 504, 727, 447,  447, 447, 447, 447,  280, 280,
    280, 280, 504, 560, 504, 504, 504, 504,  504, 574, 504, 560,  560, 560,
    560, 447, 504, 447,
};

static const uint16_t times_italic_widths[256] = {
    252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252,  252, 252,
    252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252,  252, 252,
    252, 252, 252, 252, 252, 335, 423, 504, 504, 839, 784, 215,  335, 335,
    504, 680, 252, 335, 252, 280, 504, 504, 504, 504, 504, 504,  504, 504,
    504, 504, 335, 335, 680, 680, 680, 504, 927, 615, 615, 672,  727, 615,
    615, 727, 727, 335, 447, 672, 560, 839, 672, 727, 615, 727,  615, 504,
    560, 727, 615, 839, 615, 560, 560, 392, 280, 392, 425, 504,  335, 504,
    504, 447, 504, 447, 280, 504, 504, 280, 280, 447, 280, 727,  504, 504,
    504, 504, 392, 392, 280, 504, 447, 672, 447, 447, 392, 403,  277, 403,
    545, 352, 504, 352, 335, 504, 560, 896, 504, 504, 335, 1008, 504, 335,
    951, 352, 560, 352, 352, 335, 335, 560, 560, 352, 504, 896,  335, 987,
    392, 335, 672, 352, 392, 560, 252, 392, 504, 504, 504, 504,  277, 504,
    335, 766, 278, 504, 680, 335, 766, 335, 403, 680, 302, 302,  335, 504,
    527, 252, 335, 302, 312, 504, 756, 756, 756, 504, 615, 615,  615, 615,
    615, 615, 896, 672, 615, 615, 615, 615, 335, 335, 335, 335,  727, 672,
    727, 727, 727, 727, 727, 680, 727, 727, 727, 727, 727, 560,  615, 504,
    504, 504, 504, 504, 504, 504, 672, 447, 447, 447, 447, 447,  280, 280,
    280, 280, 504, 504, 504, 504, 504, 504, 504, 680, 504, 504,  504, 504,
    504, 447, 504, 447,
};

static const uint16_t zapfdingbats_widths[256] = {
    0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   280,  981, 968, 981, 987, 724, 795, 796, 797, 695,
    967, 946, 553, 861, 918,  940, 918, 952, 981, 761, 852, 768, 767, 575,
    682, 769, 766, 765, 760,  497, 556, 541, 581, 697, 792, 794, 794, 796,
    799, 800, 822, 829, 795,  847, 829, 839, 822, 837, 930, 749, 728, 754,
    796, 798, 700, 782, 774,  798, 765, 712, 713, 687, 706, 832, 821, 795,
    795, 712, 692, 701, 694,  792, 793, 718, 797, 791, 797, 879, 767, 768,
    768, 765, 765, 899, 899,  794, 790, 441, 139, 279, 418, 395, 395, 673,
    673, 0,   393, 393, 319,  319, 278, 278, 513, 513, 413, 413, 235, 235,
    336, 336, 0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,    0,   0,   737, 548, 548, 917, 672, 766, 766,
    782, 599, 699, 631, 794,  794, 794, 794, 794, 794, 794, 794, 794, 794,
    794, 794, 794, 794, 794,  794, 794, 794, 794, 794, 794, 794, 794, 794,
    794, 794, 794, 794, 794,  794, 794, 794, 794, 794, 794, 794, 794, 794,
    794, 794, 901, 844, 1024, 461, 753, 931, 753, 925, 934, 935, 935, 840,
    879, 834, 931, 931, 924,  937, 938, 466, 890, 842, 842, 873, 873, 701,
    701, 880, 0,   880, 766,  953, 777, 871, 777, 895, 974, 895, 837, 879,
    934, 977, 925, 0,
};

static const uint16_t courier_widths[256] = {
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
    604,
};

#endif

static int pdf_text_point_width(struct pdf_doc *pdf, const char *text,
                                ptrdiff_t text_len, float size,
                                const uint16_t *widths, float *point_width)
{
    uint32_t len = 0;
    if (text_len < 0)
        text_len = strlen(text);
    *point_width = 0.0f;

    for (int i = 0; i < (int)text_len;) {
        uint8_t pdf_char = 0;
        int code_len;
        code_len =
            pdf_utf8_to_pdfencoding(&text[i], text_len - i, &pdf_char);
        if (code_len < 0)
            return pdf_set_err(pdf, code_len,
                               "Invalid unicode string at position %d in %s",
                               i, text);
        i += code_len;

        if (pdf_char != '\n' && pdf_char != '\r')
            len += widths[pdf_char];
    }

    /* Our widths arrays are for 14pt fonts */
    *point_width = len * size / (14.0f * 72.0f);

    return 0;
}

static const uint16_t *find_font_widths(const char *font_name)
{
    if (strcasecmp(font_name, "Helvetica") == 0)
        return helvetica_widths;
    if (strcasecmp(font_name, "Helvetica-Bold") == 0)
        return helvetica_bold_widths;
#if 0 // Only the Helvetica fonts are used. Don't spend flash on the other width tables.
    if (strcasecmp(font_name, "Helvetica-BoldOblique") == 0)
        return helvetica_bold_oblique_widths;
    if (strcasecmp(font_name, "Helvetica-Oblique") == 0)
        return helvetica_oblique_widths;
    if (strcasecmp(font_name, "Courier") == 0 ||
        strcasecmp(font_name, "Courier-Bold") == 0 ||
        strcasecmp(font_name, "Courier-BoldOblique") == 0 ||
        strcasecmp(font_name, "Courier-Oblique") == 0)
        return courier_widths;
    if (strcasecmp(font_name, "Times-Roman") == 0)
        return times_widths;
    if (strcasecmp(font_name, "Times-Bold") == 0)
        return times_bold_widths;
    if (strcasecmp(font_name, "Times-Italic") == 0)
        return times_italic_widths;
    if (strcasecmp(font_name, "Times-BoldItalic") == 0)
        return times_bold_italic_widths;
    if (strcasecmp(font_name, "Symbol") == 0)
        return symbol_widths;
    if (strcasecmp(font_name, "ZapfDingbats") == 0)
        return zapfdingbats_widths;
#endif

    return nullptr;
}

int pdf_get_font_text_width(struct pdf_doc *pdf, const char *font_name,
                            const char *text, float size, float *text_width)
{
    if (!font_name)
        font_name = pdf->current_font->font.name;
    const uint16_t *widths = find_font_widths(font_name);

    if (!widths)
        return pdf_set_err(pdf, -EINVAL,
                           "Unable to determine width for font '%s'",
                           pdf->current_font->font.name);
    return pdf_text_point_width(pdf, text, -1, size, widths, text_width);
}

/*
 * Content stream builder
 *
 * Collects multiple drawing operations into pdf->scratch_str and writes them as one content stream.
 * Avoids printf for the numbers. Table pages contain a lot of them.
 */

#define PDF_WIDTH_SCALE (1.0f / (14.0f * 72.0f)) // Our widths arrays are for 14pt fonts

/*
 * Left and right side bearings (distance between the glyph's advance box and its ink) of Helvetica and
 * Helvetica-Bold in WinAnsiEncoding, in 1/1000 of the font size. Used to align the ink of text exactly
 * to margins and column edges.
 */
static const int8_t helvetica_lsb[256] = {
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,  124,   52,   14,   40,   29,   52,   48,   73,   38,   40,   50,   87,   46,   87,   -8,
      43,  102,   34,   32,   28,   35,   43,   46,   37,   38,  110,  110,   45,   50,   50,   77,
      34,   17,   79,   48,   89,   90,   90,   44,   83,  100,   17,   79,   80,   75,   76,   38,
      91,   38,   93,   48,   21,   85,   30,   22,   22,   13,   28,   64,   -8,   23,   44,  -22,
      22,   42,   54,   31,   26,   40,   18,   35,   70,   66,  -18,   58,   68,   70,   70,   36,
      54,   26,   69,   34,   14,   65,   10,    6,   17,   20,   31,   43,  100,   29,   75,    0,
       2,    0,   64,   11,   47,  115,   38,   38,   20,    9,   48,   91,   43,    0,   28,    0,
       0,   65,   65,   48,   49,   50,   -5,   -9,    5,   63,   34,   85,   40,    0,   31,   13,
       0,  121,   52,   26,   67,   11,  100,   44,   30,  -13,   37,   98,   40,   46,  -13,   28,
      48,   50,   10,    7,   92,   65,   48,  110,   39,   52,   40,   98,   26,   25,   25,   95,
      17,   17,   17,   17,   17,   17,   11,   48,   90,   90,   90,   90,    1,   71,   -1,    9,
      20,   76,   38,   38,   38,   38,   38,   95,   30,   85,   85,   85,   85,   13,   91,   70,
      42,   42,   42,   42,   42,   42,   34,   31,   40,   40,   40,   40,   -5,   65,   -7,    3,
      36,   70,   36,   36,   36,   36,   36,   50,   18,   65,   65,   65,   65,   20,   54,   20,
};

static const int8_t helvetica_rsb[256] = {
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,   70,   50,   14,   31,   30,   30,   49,   42,   77,   46,   50,   86,   49,   87,   -6,
      49,  127,   45,   50,   36,   43,   43,   36,   43,   47,   64,   63,   50,   50,   45,   47,
      64,   14,   44,   45,   55,   54,   32,   69,   78,   84,   74,    9,   23,   72,   76,   36,
      50,   36,   43,   46,   18,   77,   22,   15,   18,    6,   28,   28,   -6,   69,   44,  -22,
     102,   21,   33,   23,   61,   43,   20,   75,   70,   72,   69,   -2,   70,   71,   69,   46,
      33,   61,   12,   41,   24,   74,   14,   14,   27,   22,   43,   58,  100,   72,   76,    0,
      13,    0,   64,   14,   33,  115,   43,   43,   26,    7,   46,   90,   41,    0,   28,    0,
       0,   64,   64,   34,   31,   50,   -5,   -1,   14,   62,   41,   94,   45,    0,   43,    6,
       0,  127,   46,   21,   67,   11,  100,   50,   37,  -14,   37,  101,   40,   49,  -14,   31,
      49,   50,   16,   20,   32,   12,   15,   64,   46,  120,   41,  105,   20,   26,   22,   83,
      14,   14,   14,   14,   14,   14,   50,   45,   54,   54,   54,   54,   68,   -2,   -8,    3,
      55,   76,   36,   36,   36,   36,   36,   96,   34,   77,   77,   77,   77,    7,   51,   56,
      21,   21,   21,   21,   21,   21,   44,   23,   43,   43,   43,   43,   74,    4,   -2,    9,
      46,   69,   46,   46,   46,   46,   46,   50,   82,   74,   74,   74,   74,   22,   34,   22,
};

static const int8_t helvetica_bold_lsb[256] = {
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,  112,   50,    3,   22,   22,   55,   50,   40,   22,   23,   50,   64,   26,   64,    2,
      29,   68,   30,   29,   24,   27,   32,   29,   22,   28,  113,  113,   40,   50,   40,   64,
      27,   26,   82,   44,   77,   79,   74,   42,   68,   63,   24,   74,   80,   66,   68,   40,
      76,   43,   80,   32,   14,   76,   24,   13,   22,   27,   30,   66,  -12,   18,   61,  -22,
      17,   28,   59,   34,   29,   22,   14,   41,   67,   67,    4,   59,   67,   60,   63,   35,
      58,   28,   63,   29,   14,   58,   14,    5,   16,    9,   21,   37,  100,   72,   60,    0,
       6,    0,   66,   21,   72,   92,   31,   28,    8,   11,   32,   83,   28,    0,   30,    0,
       0,   71,   66,   75,   73,   50,   -9,   -7,   -9,   71,   29,   80,   23,    0,   21,   27,
       0,   66,   37,   31,   26,    5,  100,   33,   18,  -14,   30,   88,   40,   26,  -14,   16,
      48,   56,    7,    6,  121,   58,   20,   64,   27,   31,   23,   88,   23,   23,   18,   51,
      26,   26,   26,   26,   26,   26,    1,   44,   79,   79,   79,   79,  -10,   63,  -19,   -9,
       0,   68,   40,   40,   40,   40,   40,   79,   31,   76,   76,   76,   76,   27,   76,   67,
      28,   28,   28,   28,   28,   28,   27,   34,   22,   22,   22,   22,  -10,   67,  -19,   -9,
      35,   63,   35,   35,   35,   35,   35,   50,   11,   58,   58,   58,   58,    9,   58,    9,
};

static const int8_t helvetica_bold_rsb[256] = {
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,   71,   50,    3,   29,   26,   28,   50,   30,   48,   33,   51,   64,   35,   64,    3,
      39,  127,   41,   40,   34,   39,   37,   28,   31,   40,   70,   70,   55,   50,   55,   55,
      28,   19,   56,   37,   41,   43,   25,   67,   65,   65,   70,    5,   32,   57,   61,   36,
      34,   33,   45,   34,   13,   68,   20,   12,   14,   17,   33,   25,  -11,   73,   62,  -22,
     120,   32,   36,   34,   66,   31,   20,   78,   70,   71,   68,    8,   71,   65,   65,   42,
      37,   67,   19,   36,   32,   70,   20,   12,   21,   18,   32,   72,  100,   37,   65,    0,
      10,    0,   77,   21,   68,   92,   33,   36,    7,   10,   34,   83,   30,    0,   33,    0,
       0,   72,   77,   63,   60,   50,   -1,   -3,  -12,   71,   36,   86,   24,    0,   32,   17,
       0,  117,   34,   19,   26,    4,  100,   38,   19,  -14,   41,   88,   40,   35,  -14,   18,
      49,   57,   14,   13,   16,   38,   27,   90,   39,  100,   22,   94,   28,    5,   21,   67,
      19,   19,   19,   19,   19,   19,   34,   37,   43,   43,   43,   43,   65,  -12,  -21,   -9,
      41,   61,   36,   36,   36,   36,   36,   79,   23,   68,   68,   68,   68,   17,   34,   36,
      32,   32,   32,   32,   32,   32,   32,   34,   31,   31,   31,   31,   71,  -12,  -21,   -9,
      42,   65,   42,   42,   42,   42,   42,   50,   13,   70,   70,   70,   70,   18,   37,   18,
};

static int8_t font_side_bearing(int font, uint8_t c, bool right)
{
    // All digits have the same advance width, but different bearings. Use the bearings of '0' for all of them, so that numbers in a column line up digit by digit instead of by the ink of their first or last digit.
    if ((c >= '0') && (c <= '9')) {
        c = '0';
    }

    if (font == PDF_FONT_BOLD) {
        return right ? helvetica_bold_rsb[c] : helvetica_bold_lsb[c];
    }

    return right ? helvetica_rsb[c] : helvetica_lsb[c];
}

static const uint16_t *font_widths(int font)
{
    return font == PDF_FONT_BOLD ? helvetica_bold_widths : helvetica_widths;
}

// Appends v with up to 'decimals' decimal places, followed by a space.
static void sb_append_num(struct dstr *str, float v, int decimals = 2)
{
    char buf[24];
    char *const end = buf + sizeof(buf);
    char *p = end;

    *--p = ' ';

    bool neg = v < 0;
    if (neg) {
        v = -v;
    }

    const uint32_t scale = decimals == 3 ? 1000 : 100;
    const uint32_t fixed = (uint32_t)(v * scale + 0.5f);
    uint32_t integer = fixed / scale;
    uint32_t frac = fixed % scale;

    if (frac != 0) {
        int digits = decimals;
        while (frac % 10 == 0) {
            frac /= 10;
            --digits;
        }
        for (int i = 0; i < digits; ++i) {
            *--p = (char)('0' + frac % 10);
            frac /= 10;
        }
        *--p = '.';
    }

    do {
        *--p = (char)('0' + integer % 10);
        integer /= 10;
    } while (integer != 0);

    if (neg && (fixed != 0)) {
        *--p = '-';
    }

    dstr_append_data(str, p, (size_t)(end - p));
}

static void sb_end_text(struct pdf_doc *pdf)
{
    if (pdf->sb.in_text) {
        dstr_append(&pdf->scratch_str, "ET\n");
        pdf->sb.in_text = false;
    }
}

static void sb_set_fill_colour(struct pdf_doc *pdf, uint32_t colour)
{
    colour &= 0xFFFFFF;
    if (colour == pdf->sb.fill_colour) {
        return;
    }

    pdf->sb.fill_colour = colour;
    sb_append_num(&pdf->scratch_str, PDF_RGB_R(colour), 3);
    sb_append_num(&pdf->scratch_str, PDF_RGB_G(colour), 3);
    sb_append_num(&pdf->scratch_str, PDF_RGB_B(colour), 3);
    dstr_append(&pdf->scratch_str, "rg\n");
}

static void sb_set_stroke(struct pdf_doc *pdf, uint32_t colour, float width)
{
    colour &= 0xFFFFFF;
    if (colour != pdf->sb.stroke_colour) {
        pdf->sb.stroke_colour = colour;
        sb_append_num(&pdf->scratch_str, PDF_RGB_R(colour), 3);
        sb_append_num(&pdf->scratch_str, PDF_RGB_G(colour), 3);
        sb_append_num(&pdf->scratch_str, PDF_RGB_B(colour), 3);
        dstr_append(&pdf->scratch_str, "RG\n");
    }

    if (width != pdf->sb.line_width) {
        pdf->sb.line_width = width;
        sb_append_num(&pdf->scratch_str, width);
        dstr_append(&pdf->scratch_str, "w\n");
    }
}

// Converts UTF-8 to the WinAnsi encoding used by the standard fonts.
// Control characters are dropped, except '\n' if keep_newlines is set. Invalid UTF-8 bytes are skipped.
static size_t sb_encode_text(const char *text, size_t len, uint8_t *out, size_t out_cap, bool keep_newlines)
{
    size_t n = 0;

    for (size_t i = 0; (i < len) && (n < out_cap);) {
        uint8_t c = 0;
        int code_len = pdf_utf8_to_pdfencoding(&text[i], (int)(len - i), &c);
        if (code_len <= 0) {
            ++i;
            continue;
        }
        i += (size_t)code_len;

        if ((c == '\n') && keep_newlines) {
            out[n++] = c;
            continue;
        }

        if (c < 0x20) {
            continue;
        }

        out[n++] = c;
    }

    return n;
}

static float sb_encoded_width(const uint16_t *widths, const uint8_t *text, size_t len, float size)
{
    uint32_t sum = 0;
    for (size_t i = 0; i < len; ++i) {
        sum += widths[text[i]];
    }

    return (float)sum * size * PDF_WIDTH_SCALE;
}

static void sb_emit_text(struct pdf_doc *pdf, int font, const uint8_t *text, size_t len, float size, float x, float y, uint32_t colour)
{
    if (len == 0) {
        return;
    }

    struct dstr *str = &pdf->scratch_str;

    if (!pdf->sb.in_text) {
        dstr_append(str, "BT\n");
        pdf->sb.in_text = true;
        pdf->sb.line_x = 0;
        pdf->sb.line_y = 0;
    }

    if ((font != pdf->sb.font) || (size != pdf->sb.font_size)) {
        pdf->sb.font = font;
        pdf->sb.font_size = size;
        dstr_append(str, font == PDF_FONT_BOLD ? "/F2 " : "/F1 ");
        sb_append_num(str, size);
        dstr_append(str, "Tf\n");
    }

    sb_set_fill_colour(pdf, colour);

    // Move relative to the previous text: Td is shorter than a complete text matrix (Tm).
    // The positions are rounded to 1/100 pt before calculating the offset, so that rounding errors don't add up.
    const int32_t new_x = static_cast<int32_t>(lroundf(x * 100));
    const int32_t new_y = static_cast<int32_t>(lroundf(y * 100));
    sb_append_num(str, (new_x - pdf->sb.line_x) / 100.0f);
    sb_append_num(str, (new_y - pdf->sb.line_y) / 100.0f);
    dstr_append(str, "Td (");
    pdf->sb.line_x = new_x;
    pdf->sb.line_y = new_y;

    // Escape magic characters
    size_t run_start = 0;
    for (size_t i = 0; i < len; ++i) {
        uint8_t c = text[i];
        if ((c == '(') || (c == ')') || (c == '\\')) {
            dstr_append_data(str, text + run_start, i - run_start);
            dstr_append_data(str, "\\", 1);
            run_start = i;
        }
    }
    dstr_append_data(str, text + run_start, len - run_start);

    dstr_append(str, ") Tj\n");
}

void pdf_stream_begin(struct pdf_doc *pdf)
{
    pdf->scratch_str.used_len = 0;
    dstr_data(&pdf->scratch_str)[0] = '\0';

    pdf->sb.in_text = false;
    pdf->sb.font = -1;
    pdf->sb.font_size = -1;
    pdf->sb.fill_colour = UINT32_MAX;
    pdf->sb.stroke_colour = UINT32_MAX;
    pdf->sb.line_width = -1;
}

int pdf_stream_end(struct pdf_doc *pdf)
{
    sb_end_text(pdf);
    return pdf_add_stream(pdf, dstr_data(&pdf->scratch_str));
}

void pdf_stream_fill_rect(struct pdf_doc *pdf, float x, float y, float width, float height, uint32_t colour)
{
    sb_end_text(pdf);
    sb_set_fill_colour(pdf, colour);

    sb_append_num(&pdf->scratch_str, x);
    sb_append_num(&pdf->scratch_str, y);
    sb_append_num(&pdf->scratch_str, width);
    sb_append_num(&pdf->scratch_str, height);
    dstr_append(&pdf->scratch_str, "re f\n");
}

void pdf_stream_line(struct pdf_doc *pdf, float x1, float y1, float x2, float y2, float width, uint32_t colour)
{
    sb_end_text(pdf);
    sb_set_stroke(pdf, colour, width);

    sb_append_num(&pdf->scratch_str, x1);
    sb_append_num(&pdf->scratch_str, y1);
    dstr_append(&pdf->scratch_str, "m ");
    sb_append_num(&pdf->scratch_str, x2);
    sb_append_num(&pdf->scratch_str, y2);
    dstr_append(&pdf->scratch_str, "l S\n");
}

float pdf_text_width(struct pdf_doc *pdf, int font, const char *text, float size)
{
    float width = 0;
    pdf_get_font_text_width(pdf, font == PDF_FONT_BOLD ? PDF_FONT_NAME_BOLD : PDF_FONT_NAME_REGULAR, text, size, &width);
    return width;
}

void pdf_stream_text(struct pdf_doc *pdf, int font, const char *text, float size, float x, float y, uint32_t colour, int align, float max_width)
{
    uint8_t buf[192];
    // Leave room for the ellipsis.
    size_t len = sb_encode_text(text, strlen(text), buf, sizeof(buf) - 1, false);

    const uint16_t *widths = font_widths(font);
    float width = sb_encoded_width(widths, buf, len, size);

    // Tolerate rounding errors
    if ((max_width > 0) && (width > (max_width + 0.01f))) {
        const float ellipsis_width = widths[0x85] * size * PDF_WIDTH_SCALE;

        while ((len > 0) && (((width + ellipsis_width) > max_width) || (buf[len - 1] == ' '))) {
            --len;
            width -= widths[buf[len]] * size * PDF_WIDTH_SCALE;
        }

        buf[len++] = 0x85; // WinAnsi "…"
        width += ellipsis_width;
    }

    // Align the ink, not the advance box, to x.
    if (align == PDF_ALIGN_RIGHT) {
        x -= width - (len > 0 ? font_side_bearing(font, buf[len - 1], true) * size / 1000.0f : 0);
    } else if (align == PDF_ALIGN_CENTER) {
        x -= width / 2;
    } else if (len > 0) {
        x -= font_side_bearing(font, buf[0], false) * size / 1000.0f;
    }

    sb_emit_text(pdf, font, buf, len, size, x, y, colour);
}

int pdf_stream_text_wrap(struct pdf_doc *pdf, int font, const char *text, float size, float x, float y, float width, float leading, uint32_t colour, bool draw)
{
    size_t text_len = strlen(text);
    if (text_len == 0) {
        return 0;
    }

    uint8_t *buf = (uint8_t *)malloc(text_len);
    if (buf == nullptr) {
        return pdf_set_err(pdf, -ENOMEM, "Failed to allocate text wrap buffer");
    }

    const size_t len = sb_encode_text(text, text_len, buf, text_len, true);
    const uint16_t *widths = font_widths(font);

    int lines = 0;
    size_t start = 0;

    while (start < len) {
        // Skip leading spaces of wrapped lines.
        while ((start < len) && (buf[start] == ' ')) {
            ++start;
        }

        float line_width = 0;
        size_t last_space = SIZE_MAX;
        size_t i = start;

        for (; (i < len) && (buf[i] != '\n'); ++i) {
            float char_width = widths[buf[i]] * size * PDF_WIDTH_SCALE;
            if (((line_width + char_width) > width) && (i > start)) {
                break;
            }
            if (buf[i] == ' ') {
                last_space = i;
            }
            line_width += char_width;
        }

        size_t end;
        size_t next;
        if ((i >= len) || (buf[i] == '\n')) {
            end = i;
            next = i + 1;
        } else if (last_space != SIZE_MAX) {
            end = last_space;
            next = last_space + 1;
        } else {
            // A single word that is too long for the line
            end = i;
            next = i;
        }

        if (draw && (end > start)) {
            sb_emit_text(pdf, font, buf + start, end - start, size, x - font_side_bearing(font, buf[start], false) * size / 1000.0f, y - lines * leading, colour);
        }

        ++lines;
        start = next;
    }

    free(buf);
    return lines;
}

int pdf_add_line(struct pdf_doc *pdf, struct pdf_object *page, float x1, float y1, float x2, float y2, float width, uint32_t colour)
{
    int ret;
    struct dstr str = INIT_DSTR;

    dstr_printf(&str, "%f w\r\n", width);
    dstr_printf(&str, "%f %f m\r\n", x1, y1);
    dstr_printf(&str, "/DeviceRGB CS\r\n");
    dstr_printf(&str, "%f %f %f RG\r\n", PDF_RGB_R(colour), PDF_RGB_G(colour),
                PDF_RGB_B(colour));
    dstr_printf(&str, "%f %f l S\r\n", x2, y2);

    ret = pdf_add_stream(pdf, dstr_data(&str));
    dstr_free(&str);

    return ret;
}

int pdf_add_horizontal_lines(struct pdf_doc *pdf, struct pdf_object *page, float x1, float y1, float x2, float y2, float width, uint32_t colour, float spacing, int count, bool first_line_double_wide)
{
    int ret;
    //struct dstr str = INIT_DSTR;
    pdf->scratch_str.used_len = 0;

    dstr_printf(&pdf->scratch_str, "%f w ", width);
    dstr_printf(&pdf->scratch_str, "/DeviceRGB CS ");
    dstr_printf(&pdf->scratch_str, "%f %f %f RG ", PDF_RGB_R(colour), PDF_RGB_G(colour), PDF_RGB_B(colour));

    if (first_line_double_wide) {
        dstr_printf(&pdf->scratch_str, "%f %f m ", x1, y1 + width / 2);
        dstr_printf(&pdf->scratch_str, "%f %f l ", x2, y2 + width / 2);
        dstr_printf(&pdf->scratch_str, "%f %f m ", x1, y1 - width / 2);
        dstr_printf(&pdf->scratch_str, "%f %f l ", x2, y2 - width / 2);
    }

    for (int i = 0; i < count; ++i) {
        dstr_printf(&pdf->scratch_str, "%f %f m ", x1, y1 - (spacing * i));
        dstr_printf(&pdf->scratch_str, "%f %f l ", x2, y2 - (spacing * i));
    }
    dstr_printf(&pdf->scratch_str, "S");

    ret = pdf_add_stream(pdf, dstr_data(&pdf->scratch_str));

    return ret;
}

#if 0
int pdf_add_rectangle(struct pdf_doc *pdf, struct pdf_object *page, float x,
                      float y, float width, float height, float border_width,
                      uint32_t colour)
{
    int ret;
    struct dstr str = INIT_DSTR;

    dstr_printf(&str, "%f %f %f RG ", PDF_RGB_R(colour), PDF_RGB_G(colour),
                PDF_RGB_B(colour));
    dstr_printf(&str, "%f w ", border_width);
    dstr_printf(&str, "%f %f %f %f re S ", x, y, width, height);

    ret = pdf_add_stream(pdf, dstr_data(&str));
    dstr_free(&str);

    return ret;
}
#endif

int pdf_add_filled_rectangle(struct pdf_doc *pdf, struct pdf_object *page,
                             float x, float y, float width, float height,
                             float border_width, uint32_t colour_fill,
                             uint32_t colour_border)
{
    int ret;
    struct dstr str = INIT_DSTR;

    dstr_printf(&str, "%f %f %f rg ", PDF_RGB_R(colour_fill),
                PDF_RGB_G(colour_fill), PDF_RGB_B(colour_fill));
    if (border_width > 0) {
        dstr_printf(&str, "%f %f %f RG ", PDF_RGB_R(colour_border),
                    PDF_RGB_G(colour_border), PDF_RGB_B(colour_border));
        dstr_printf(&str, "%f w ", border_width);
        dstr_printf(&str, "%f %f %f %f re B ", x, y, width, height);
    } else {
        dstr_printf(&str, "%f %f %f %f re f ", x, y, width, height);
    }

    ret = pdf_add_stream(pdf, dstr_data(&str));
    dstr_free(&str);

    return ret;
}

/**
 * Get the display dimensions of an image, respecting the images aspect ratio
 * if only one desired display dimension is defined.
 * The pdf parameter is only used for setting the error value.
 */
static int get_img_display_dimensions(struct pdf_doc *pdf, uint32_t img_width,
                                      uint32_t img_height,
                                      float *display_width,
                                      float *display_height)
{
    if (!display_height || !display_width) {
        return pdf_set_err(
            pdf, -EINVAL,
            "display_width and display_height may not be null pointers");
    }

    const float display_width_in = *display_width;
    const float display_height_in = *display_height;

    if (display_width_in < 0 && display_height_in < 0) {
        return pdf_set_err(pdf, -EINVAL,
                           "Unable to determine image display dimensions, "
                           "display_width and display_height are both < 0");
    }
    if (img_width == 0 || img_height == 0) {
        return pdf_set_err(pdf, -EINVAL,
                           "Invalid image dimensions received, the loaded "
                           "image appears to be empty.");
    }

    if (display_width_in < 0) {
        // Set width, keeping aspect ratio
        *display_width = display_height_in * ((float)img_width / img_height);
    } else if (display_height_in < 0) {
        // Set height, keeping aspect ratio
        *display_height = display_width_in * ((float)img_height / img_width);
    }
    return 0;
}

static int pdf_add_image(struct pdf_doc *pdf, struct pdf_object *page,
                         struct pdf_object *image, struct pdf_object *image_stream, float x, float y,
                         float width, float height)
{
    int ret;
    struct dstr str = INIT_DSTR;

    dstr_append(&str, "q ");
    dstr_printf(&str, "%f 0 0 %f %f %f cm ", width, height, x, y);
    dstr_printf(&str, "/Image%d Do ", image->index);
    dstr_append(&str, "Q");

    ret = pdf_add_stream(pdf, dstr_data(&str));
    dstr_free(&str);
    return ret;
}

static int parse_png_header(struct pdf_img_info *info, const uint8_t *data,
                            size_t length, char *err_msg,
                            size_t err_msg_length)
{
    if (length <= sizeof(png_signature)) {
        snprintf(err_msg, err_msg_length, "PNG file too short");
        return -EINVAL;
    }

    if (memcmp(data, png_signature, sizeof(png_signature))) {
        snprintf(err_msg, err_msg_length, "File is not correct PNG file");
        return -EINVAL;
    }

    // process first PNG chunk
    uint32_t pos = sizeof(png_signature);
    const struct png_chunk *chunk = (const struct png_chunk *)&data[pos];
    pos += sizeof(struct png_chunk);
    if (pos > length) {
        snprintf(err_msg, err_msg_length, "PNG file too short");
        return -EINVAL;
    }
    if (strncmp(chunk->type, png_chunk_header, 4) == 0) {
        // header found, process width and height, check errors
        struct png_header *header = &info->png;

        if (pos + sizeof(struct png_header) > length) {
            snprintf(err_msg, err_msg_length, "PNG file too short");
            return -EINVAL;
        }

        memcpy(header, &data[pos], sizeof(struct png_header));
        if (header->deflate != 0) {
            snprintf(err_msg, err_msg_length, "Deflate wrong in PNG header");
            return -EINVAL;
        }
        if (header->bitDepth == 0) {
            snprintf(err_msg, err_msg_length, "PNG file has zero bit depth");
            return -EINVAL;
        }
        // ensure the width and height values have the proper byte order
        // and copy them into the info struct.
        header->width = ntohl(header->width);
        header->height = ntohl(header->height);
        info->width = header->width;
        info->height = header->height;
        return 0;
    }
    snprintf(err_msg, err_msg_length, "Failed to read PNG file header");
    return -EINVAL;
}

// https://stackoverflow.com/a/42060129
#ifndef defer
struct defer_dummy {};
template <class F> struct deferrer { F f; ~deferrer() { f(); } };
template <class F> deferrer<F> operator*(defer_dummy, F f) { return {f}; }
#define DEFER_(LINE) zz_defer##LINE
#define DEFER(LINE) DEFER_(LINE)
#define defer auto DEFER(__LINE__) = defer_dummy{} *[&]()
#endif

// If you increase this, maybe change to heap allocation again.
#define PNG_MAX_PALETTE_SIZE 16

static int pdf_add_png_data(struct pdf_doc *pdf, struct pdf_object *page,
                            float x, float y, float display_width,
                            float display_height,
                            const struct pdf_img_info *img_info,
                            uint32_t colour_background_hint,
                            const uint8_t *png_data, size_t png_data_length)
{
    // string stream used for writing color space (and palette) info
    // into the pdf
    struct dstr colour_space = INIT_DSTR;
    defer {dstr_free(&colour_space);};

    struct pdf_object *obj = nullptr;
    uint32_t pos;
    size_t png_data_total_length = 0;
    uint8_t ncolours;

    pdf->scratch_str.used_len = 0;

    // Stores palette information for indexed PNGs
    struct rgb_value palette_buffer[PNG_MAX_PALETTE_SIZE];
    size_t palette_buffer_length = 0;

    const struct png_header *header = &img_info->png;

    // Father info from png header
    switch (header->colorType) {
    case PNG_COLOR_GREYSCALE:
        ncolours = 1;
        break;
    case PNG_COLOR_RGB:
        ncolours = 3;
        break;
    case PNG_COLOR_INDEXED:
        ncolours = 1;
        break;
    // PNG_COLOR_RGBA and PNG_COLOR_GREYSCALE_A are unsupported
    default:
        pdf_set_err(pdf, -EINVAL, "PNG has unsupported color type: %d",
                    header->colorType);
        return -EINVAL;
    }

    /* process PNG chunks */
    pos = sizeof(png_signature);

    while (1) {
        const struct png_chunk *chunk;

        chunk = (const struct png_chunk *)&png_data[pos];
        pos += sizeof(struct png_chunk);

        if (pos > png_data_length - 4) {
            pdf_set_err(pdf, -EINVAL, "PNG file too short");
            return -EINVAL;
        }
        const uint32_t chunk_length = ntohl(chunk->length);
        // chunk length + 4-bytes of CRC
        if (chunk_length > png_data_length - pos - 4) {
            pdf_set_err(pdf, -EINVAL, "PNG chunk exceeds file: %" PRIi32 " vs %" PRIu32,
                        chunk_length, png_data_length - pos - 4);
            return -EINVAL;
        }
        if (strncmp(chunk->type, png_chunk_header, 4) == 0) {
            // Ignoring the header, since it was parsed
            // before calling this function.
        } else if (strncmp(chunk->type, png_chunk_palette, 4) == 0) {
            // Palette chunk
            if (header->colorType == PNG_COLOR_INDEXED) {
                // palette chunk is needed for indexed images
                if (palette_buffer_length != 0) {
                    pdf_set_err(pdf, -EINVAL,
                                "PNG contains multiple palette chunks");
                    return -EINVAL;
                }
                if (chunk_length % 3 != 0) {
                    pdf_set_err(pdf, -EINVAL,
                                "PNG format error: palette chunk length is "
                                "not divisbly by 3!");
                    return -EINVAL;
                }
                palette_buffer_length = (size_t)(chunk_length / 3);
                if (palette_buffer_length > PNG_MAX_PALETTE_SIZE ||
                    palette_buffer_length == 0) {
                    pdf_set_err(pdf, -EINVAL,
                                "PNG palette length invalid or too large: %zd; max supported %d",
                                palette_buffer_length, PNG_MAX_PALETTE_SIZE);
                    return -EINVAL;
                }

                for (size_t i = 0; i < palette_buffer_length; i++) {
                    size_t offset = (i * 3) + pos;
                    palette_buffer[i].red = png_data[offset];
                    palette_buffer[i].green = png_data[offset + 1];
                    palette_buffer[i].blue = png_data[offset + 2];
                    palette_buffer[i].alpha = 0xFF;
                }
            } else if (header->colorType == PNG_COLOR_RGB ||
                       header->colorType == PNG_COLOR_RGBA) {
                // palette chunk is optional for RGB(A) images
                // but we do not process them
            } else {
                pdf_set_err(pdf, -EINVAL,
                            "Unexpected palette chunk for color type %d",
                            header->colorType);
                return -EINVAL;
            }
        } else if (strncmp(chunk->type, png_chunk_transparency, 4) == 0) {
            for (size_t i = 0; i < min(palette_buffer_length, chunk_length); i++) {
                palette_buffer[i].alpha = png_data[pos + i];
            }
            if (chunk_length == 1) {
                palette_buffer[0].red = RGB_R(colour_background_hint);
                palette_buffer[0].green = RGB_G(colour_background_hint);
                palette_buffer[0].blue = RGB_B(colour_background_hint);
            }
        } else if (strncmp(chunk->type, png_chunk_data, 4) == 0) {
            if (chunk_length > 0 && chunk_length < png_data_length - pos) {
                dstr_append_data(&pdf->scratch_str, png_data + pos, chunk_length);
                //png_data_temp.reserve(png_data_total_length + chunk_length);
                //png_data_temp.insert(png_data_temp.end(), png_data + pos, png_data + pos + chunk_length);
                png_data_total_length += chunk_length;
            }
        } else if (strncmp(chunk->type, png_chunk_end, 4) == 0) {
            /* end of file, exit */
            break;
        }

        if (chunk_length >= png_data_length) {
            pdf_set_err(pdf, -EINVAL, "PNG chunk length larger than file");
            return -EINVAL;
        }

        pos += chunk_length;     // add chunk length
        pos += sizeof(uint32_t); // add CRC length
    }

    /* if no length was found */
    if (png_data_total_length == 0) {
        pdf_set_err(pdf, -EINVAL, "PNG file has zero length");
        return -EINVAL;
    }

    switch (header->colorType) {
    case PNG_COLOR_GREYSCALE:
        dstr_append(&colour_space, "/DeviceGray");
        break;
    case PNG_COLOR_RGB:
        dstr_append(&colour_space, "/DeviceRGB");
        break;
    case PNG_COLOR_INDEXED: {
        if (palette_buffer_length == 0) {
            pdf_set_err(pdf, -EINVAL, "Indexed PNG contains no palette");
            return -EINVAL;
        }
        // Write the color palette to the color_palette buffer
        dstr_printf(&colour_space,
                    "[ /Indexed\r\n"
                    "  /DeviceRGB\r\n"
                    "  %zu\r\n"
                    "  <",
                    palette_buffer_length - 1);
        // write individual paletter values
        // the index value for every RGB value is determined by its position
        // (0, 1, 2, ...)
        int first_transparent = -1;
        int last_transparent = -1;
        for (size_t i = 0; i < palette_buffer_length; i++) {
            dstr_printf(&colour_space, "%02X%02X%02X ", palette_buffer[i].red,
                        palette_buffer[i].green, palette_buffer[i].blue);
            if (palette_buffer[i].alpha != 0xFF) {
                if (first_transparent == -1) {
                    first_transparent = i;
                    last_transparent = i;
                } else {
                    if (last_transparent == (i - 1))
                        last_transparent = i;
                    else {
                        pdf_set_err(pdf, -EINVAL,
                                    "Multiple transparent blocks in PNG not supported.");
                        return -EINVAL;
                    }
                }
            }
        }
        dstr_append(&colour_space, ">\r\n]");

        if (first_transparent != -1) {
            dstr_printf(&colour_space, "/Mask [%d %d]\r\n", first_transparent, last_transparent);
        }
        break;
    }

    default:
        pdf_set_err(pdf, -EINVAL,
                    "Cannot map PNG color type %d to PDF color space",
                    header->colorType);
        return -EINVAL;
        break;
    }

    obj = pdf_get_object(pdf, pdf->callback_context.current_obj_index);

    pdf_printf(pdf,
                "<<\r\n"
                "  /Type /XObject\r\n"
                "  /Name /Image%d\r\n"
                "  /Subtype /Image\r\n"
                "  /ColorSpace %s\r\n"
                "  /Width %u\r\n"
                "  /Height %u\r\n"
                "  /Interpolate false\r\n"
                "  /BitsPerComponent %u\r\n"
                "  /Filter /FlateDecode\r\n"
                "  /DecodeParms << /Predictor 15 /Colors %d "
                "/BitsPerComponent %u /Columns %u >>\r\n"
                "  /Length %zu\r\n"
                ">>stream\r\n",
                obj->index, dstr_data(&colour_space),
                header->width, header->height, header->bitDepth, ncolours,
                header->bitDepth, header->width, png_data_total_length);

    pdf_write(pdf, dstr_data(&pdf->scratch_str), dstr_len(&pdf->scratch_str)/*(const char *)png_data_temp.data(), png_data_temp.size()*/);
    pdf_printf(pdf, "\r\nendstream\r\n");

    if (get_img_display_dimensions(pdf, header->width, header->height,
                                   &display_width, &display_height)) {
        return -1;
    }

    if (page == nullptr)
        page = pdf_get_page(pdf, obj);

    struct pdf_object *image_stream = pdf_get_object(pdf, obj->index + page->page.image_count);
    obj->image.x = x;
    obj->image.y = y;
    image_stream->image_stream.width = display_width;
    image_stream->image_stream.height = display_height;

    return 0;
}


int pdf_add_png_image_data(struct pdf_doc *pdf, struct pdf_object *page, float x,
                       float y, float display_width, float display_height,
                       uint32_t colour_background_hint,
                       const uint8_t *data, size_t len)
{
    struct pdf_img_info info = {
        .image_format = IMAGE_PNG,
        .width = 0,
        .height = 0,
        .png = {}
    };

    int ret = parse_png_header(&info, data, len, pdf->errstr, sizeof(pdf->errstr));
    if (ret)
        return ret;

    return pdf_add_png_data(pdf, page, x, y, display_width, display_height, &info, colour_background_hint, data, len);
}

int pdf_add_write_callback(struct pdf_doc *pdf, std::function<ssize_t(const void *buf, size_t len)> &&cb) {
    pdf->write_fn = std::move(cb);
    return 0;
}

int pdf_add_stream_callback(struct pdf_doc *pdf, std::function<int(struct pdf_doc *pdf, uint32_t page_num, uint32_t stream_num)> &&cb) {
    pdf->stream_fn = std::move(cb);
    return 0;
}

int pdf_add_image_callback(struct pdf_doc *pdf, std::function<int(struct pdf_doc *pdf, uint32_t page_num, uint32_t image_num)> &&cb) {
    pdf->image_fn = std::move(cb);
    return 0;
}

int pdf_add_page_callback(struct pdf_doc *pdf, std::function<int(struct pdf_doc *pdf, uint32_t page_num)> &&cb) {
    pdf->page_fn = std::move(cb);
    return 0;
}
