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

#define PDF_MAX_OBJECTS 2400
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

    std::function<ssize_t(const void *buf, size_t len)> write_fn;
    std::function<int(struct pdf_doc *pdf, uint32_t page_num, uint32_t stream_num)> stream_fn;
    std::function<int(struct pdf_doc *pdf, uint32_t page_num, uint32_t image_num)> image_fn;
    std::function<int(struct pdf_doc *pdf, uint32_t page_num)> page_fn;

    char write_buf[2048];
    size_t write_buf_used;
    size_t write_buf_written = 0;
    size_t last_write_buf_written = 0;

    struct dstr scratch_str;

    struct {
        int current_obj_index;
        bool is_image;
    } callback_context;
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

        new_len = len + 4096;

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

struct pdf_doc *pdf_create(float width, float height, const struct pdf_info *info)
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
        // ISO 8601 date time with time offset
        strftime(obj->info->date, sizeof(obj->info->date), "%FT%T%z", &tm);
    }

    if (pdf_set_font(pdf, DEFAULT_FONT) < 0) {
        pdf_destroy(pdf);
        return nullptr;
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
        struct pdf_object *obj = pdf_add_object(pdf, OBJ_image);
        obj->page_id = page->index;
    }

    for (size_t i = 0; i < page->page.image_count; ++i) {
        struct pdf_object *obj = pdf_add_object(pdf, OBJ_imagestream);
        obj->page_id = page->index;
    }
}

static void pdf_flush_write_buf(struct pdf_doc *pdf, int target_free_space) {
    auto *head = pdf->write_buf;
    target_free_space = std::max(target_free_space, 128);

    if (target_free_space > ARRAY_SIZE(pdf->write_buf))
        target_free_space = ARRAY_SIZE(pdf->write_buf);

    while (target_free_space > (ARRAY_SIZE(pdf->write_buf) - pdf->write_buf_used)) {
        ssize_t written = pdf->write_fn(head, pdf->write_buf_used);
        if (written <= 0) {
            printf("write_fn failed %zd.", written);
            pdf->write_error_occurred = true;
        }
        pdf->write_buf_used -= written;
        head += written;
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

    va_start(ap, fmt);
    va_copy(aq, ap);
    len = vsnprintf(nullptr, 0, fmt, ap);
    if (len > ARRAY_SIZE(pdf->write_buf)) {
        printf("write buf too small! %u, but required are %d.", ARRAY_SIZE(pdf->write_buf), len);
        exit(-1);
    }

    pdf_flush_write_buf(pdf, len);

    vsprintf(pdf->write_buf + pdf->write_buf_used, fmt, aq);
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

        size_t to_write = min(count, ARRAY_SIZE(pdf->write_buf) - pdf->write_buf_used);
        memcpy(pdf->write_buf + pdf->write_buf_used, buf, to_write);
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

static int pdf_save_object(struct pdf_doc *pdf, int index)
{
    struct pdf_object *object = pdf_get_object(pdf, index);
    if (!object)
        return -ENOENT;

    if (object->type == OBJ_none)
        return -ENOENT;

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
    case OBJ_image: {
        struct pdf_object *page = pdf_get_page(pdf, object);

        pdf->callback_context.current_obj_index = object->index;
        pdf->callback_context.is_image = true;
        pdf->image_fn(pdf, page->page.page_number, object->index - page->index - page->page.stream_count - 1);
        pdf->callback_context.current_obj_index = -1;
        break;
    }
    case OBJ_imagestream: {
        struct pdf_object *page = pdf_get_page(pdf, object);
        struct pdf_object *image = pdf_get_object(pdf, object->index - page->page.image_count);
        pdf_add_image(pdf, nullptr, image, object, image->image.x, image->image.y, object->image_stream.width, object->image_stream.height);
        break;
    }
    case OBJ_info: {
        struct pdf_info *info = object->info;

        pdf_printf(pdf, "<<\r\n");
        if (info->creator[0])
            pdf_printf(pdf, "  /Creator (%s)\r\n", info->creator);
        if (info->producer[0])
            pdf_printf(pdf, "  /Producer (%s)\r\n", info->producer);
        if (info->title[0])
            pdf_printf(pdf, "  /Title (%s)\r\n", info->title);
        if (info->author[0])
            pdf_printf(pdf, "  /Author (%s)\r\n", info->author);
        if (info->subject[0])
            pdf_printf(pdf, "  /Subject (%s)\r\n", info->subject);
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
        //for (struct pdf_object *font = pdf_find_first_object(pdf, OBJ_font); font; font = pdf_find_next_object(pdf, font, OBJ_font))
        pdf_printf(pdf, "      /F%d %d 0 R\r\n", 1, 2/*font->font.index, font->index*/);
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

    pdf_flush_write_buf(pdf, ARRAY_SIZE(pdf->write_buf));

    return 0;
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

static int utf8_to_utf32(const char *utf8, int len, uint32_t *utf32)
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

static int utf8_to_pdfencoding(struct pdf_doc *pdf, const char *utf8, int len,
                               uint8_t *res)
{
    uint32_t code;
    int code_len;

    *res = 0;

    code_len = utf8_to_utf32(utf8, len, &code);
    if (code_len < 0) {
        return pdf_set_err(pdf, -EINVAL, "Invalid UTF-8 encoding");
    }

    if (code > 255) {
        /* We support *some* minimal UTF-8 characters */
        // See Appendix D of
        // https://www.adobe.com/content/dam/acom/en/devnet/pdf/pdfs/pdf_reference_archives/PDFReference.pdf
        // These are all in WinAnsiEncoding
        switch (code) {
        case 0x160: // Latin Capital Letter S with Caron
            *res = 0212;
            break;
        case 0x161: // Latin Small Letter S with Caron
            *res = 0232;
            break;
        case 0x17d: // Latin Capital Letter Z with Caron
            *res = 0216;
            break;
        case 0x17e: // Latin Small Letter Z with Caron
            *res = 0236;
            break;
        case 0x2014: // emdash
            *res = 0227;
            break;
        case 0x2018: // left single quote
            *res = 0221;
            break;
        case 0x2019: // right single quote
            *res = 0222;
            break;
        case 0x201c: // left double quote
            *res = 0223;
            break;
        case 0x201d: // right double quote
            *res = 0224;
            break;
        case 0x20ac: // Euro
            *res = 0200;
            break;
        default:
            return pdf_set_err(pdf, -EINVAL,
                               "Unsupported UTF-8 character: 0x%x 0o%o", code,
                               code);
        }
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
        code_len = utf8_to_pdfencoding(pdf, &text[i], len - i, &pdf_char);
        if (code_len < 0) {
            dstr_free(&str);
            return code_len;
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
                code_len = utf8_to_pdfencoding(pdf, &text_head[i], len - i, &pdf_char);
                if (code_len < 0) {
                    dstr_free(&inner);
                    return code_len;
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
            utf8_to_pdfencoding(pdf, &text[i], text_len - i, &pdf_char);
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
#if 0
static const char *find_word_break(const char *string)
{
    /* Skip over the actual word */
    while (string && *string && (*string < 0 || !isspace(*string)))
        string++;

    return string;
}

int pdf_add_text_wrap(struct pdf_doc *pdf, struct pdf_object *page,
                      const char *text, float size, float xoff, float yoff,
                      uint32_t colour, float wrap_width, int align,
                      float *height)
{
    /* Move through the text string, stopping at word boundaries,
     * trying to find the longest text string we can fit in the given width
     */
    const char *start = text;
    const char *last_best = text;
    const char *end = text;
    char line[512];
    const uint16_t *widths;
    float orig_yoff = yoff;

    widths = find_font_widths(pdf->current_font->font.name);
    if (!widths)
        return pdf_set_err(pdf, -EINVAL,
                           "Unable to determine width for font '%s'",
                           pdf->current_font->font.name);

    while (start && *start) {
        const char *new_end = find_word_break(end + 1);
        float line_width;
        int output = 0;
        float xoff_align = xoff;
        int e;

        end = new_end;

        e = pdf_text_point_width(pdf, start, end - start, size, widths,
                                 &line_width);
        if (e < 0)
            return e;

        if (line_width >= wrap_width) {
            if (last_best == start) {
                /* There is a single word that is too long for the line */
                ptrdiff_t i;
                /* Find the best character to chop it at */
                for (i = end - start - 1; i > 0; i--) {
                    float this_width;
                    // Don't look at places that are in the middle of a utf-8
                    // sequence
                    if ((start[i - 1] & 0xc0) == 0xc0 ||
                        ((start[i - 1] & 0xc0) == 0x80 &&
                         (start[i] & 0xc0) == 0x80))
                        continue;
                    e = pdf_text_point_width(pdf, start, i, size, widths,
                                             &this_width);
                    if (e < 0)
                        return e;
                    if (this_width < wrap_width)
                        break;
                }
                if (i == 0)
                    return pdf_set_err(pdf, -EINVAL,
                                       "Unable to find suitable line break");

                end = start + i;
            } else
                end = last_best;
            output = 1;
        }
        if (*end == '\0')
            output = 1;

        if (*end == '\n' || *end == '\r')
            output = 1;

        if (output) {
            int len = end - start;
            float char_spacing = 0;
            if (len >= (int)sizeof(line))
                len = (int)sizeof(line) - 1;
            strncpy(line, start, len);
            line[len] = '\0';

            e = pdf_text_point_width(pdf, start, len, size, widths,
                                     &line_width);
            if (e < 0)
                return e;

            switch (align) {
            case PDF_ALIGN_RIGHT:
                xoff_align += wrap_width - line_width;
                break;
            case PDF_ALIGN_CENTER:
                xoff_align += (wrap_width - line_width) / 2;
                break;
            case PDF_ALIGN_JUSTIFY:
                if ((len - 1) > 0 && *end != '\r' && *end != '\n' &&
                    *end != '\0')
                    char_spacing = (wrap_width - line_width) / (len - 2);
                break;
            case PDF_ALIGN_JUSTIFY_ALL:
                if ((len - 1) > 0)
                    char_spacing = (wrap_width - line_width) / (len - 2);
                break;
            }

            if (align != PDF_ALIGN_NO_WRITE) {
                pdf_add_text_spacing(pdf, page, line, size, xoff_align, yoff,
                                     colour, char_spacing);
            }

            if (*end == ' ')
                end++;

            start = last_best = end;
            yoff -= size;
        } else
            last_best = end;
    }

    if (height)
        *height = orig_yoff - yoff;
    return 0;
}
#endif
int pdf_add_line(struct pdf_doc *pdf, struct pdf_object *page, float x1,
                 float y1, float x2, float y2, float width, uint32_t colour)
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

int pdf_add_horizontal_lines(struct pdf_doc *pdf, struct pdf_object *page, float x1,
                 float y1, float x2, float y2, float width, uint32_t colour, float spacing, int count, bool first_line_double_wide)
{
    int ret;
    //struct dstr str = INIT_DSTR;
    pdf->scratch_str.used_len = 0;

    dstr_printf(&pdf->scratch_str, "%f w ", width);
    dstr_printf(&pdf->scratch_str, "/DeviceRGB CS ");
    dstr_printf(&pdf->scratch_str, "%f %f %f RG ", PDF_RGB_R(colour), PDF_RGB_G(colour),
                PDF_RGB_B(colour));

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
            pdf_set_err(pdf, -EINVAL, "PNG chunk exceeds file: %d vs %u",
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

int pdf_add_write_callback(struct pdf_doc *pdf, std::function<ssize_t(const void *buf, size_t len)> cb) {
    pdf->write_fn = cb;
    return 0;
}

int pdf_add_stream_callback(struct pdf_doc *pdf, std::function<int(struct pdf_doc *pdf, uint32_t page_num, uint32_t stream_num)> cb) {
    pdf->stream_fn = cb;
    return 0;
}

int pdf_add_image_callback(struct pdf_doc *pdf, std::function<int(struct pdf_doc *pdf, uint32_t page_num, uint32_t image_num)> cb) {
    pdf->image_fn = cb;
    return 0;
}

int pdf_add_page_callback(struct pdf_doc *pdf, std::function<int(struct pdf_doc *pdf, uint32_t page_num)> cb) {
    pdf->page_fn = cb;
    return 0;
}
