// This is based on https://github.com/AndreRenaud/PDFGen
// But modified to stream out the PDF while it is generated.
// This reduced the memory requirement to about 10 to 15k for charge tracker reports.

/**
 * Simple engine for creating PDF files.
 * It supports text, shapes, images etc...
 * Capable of handling millions of objects without too much performance
 * penalty.
 * Public domain license - no warrenty implied; use at your own risk.
 * @file pdfgen.h
 */
#ifndef PDFGEN_H
#define PDFGEN_H

#include <stdint.h>
#include <stdio.h>
#include <functional>

/**
 * @defgroup subsystem Simple PDF Generation
 * Allows for quick generation of simple PDF documents.
 * This is useful for producing easily printed output from C code, where
 * advanced formatting is not required
 *
 * Note: All coordinates/sizes are in points (1/72 of an inch).
 * All coordinates are based on 0,0 being the bottom left of the page.
 * All colours are specified as a packed 32-bit value - see @ref PDF_RGB.
 * Text strings are interpreted as UTF-8 encoded, but only a small subset of
 * characters beyond 7-bit ascii are supported (see @ref pdf_add_text for
 * details).
 *
 * @par PDF library example:
 * @code
#include "pdfgen.h"
 ...
struct pdf_info info = {
         .creator = "My software",
         .producer = "My software",
         .title = "My document",
         .author = "My name",
         .subject = "My subject",
         .date = "Today"
         };
struct pdf_doc *pdf = pdf_create(PDF_A4_WIDTH, PDF_A4_HEIGHT, &info);
pdf_set_font(pdf, "Times-Roman");
pdf_append_page(pdf);
pdf_add_text(pdf, NULL, "This is text", 12, 50, 20, PDF_BLACK);
pdf_add_line(pdf, NULL, 50, 24, 150, 24);
pdf_save(pdf, "output.pdf");
pdf_destroy(pdf);
 * @endcode
 */

struct pdf_doc;
struct pdf_object;

/**
 * pdf_info describes the metadata to be inserted into the
 * header of the output PDF
 */
struct pdf_info {
    char creator[64];  //!< Software used to create the PDF
    char producer[64]; //!< Software used to create the PDF
    char title[64];    //!< The title of the PDF (typically displayed in the
                       //!< window bar when viewing)
    char author[64];   //!< Who created the PDF
    char subject[64];  //!< What is the PDF about
    char date[64];     //!< The date the PDF was created
};

/**
 * Enum that declares the different image file formats we currently support.
 * Each value has a corresponding header struct used within
 * the format_specific_img_info union.
 */
enum {
    IMAGE_PNG,

    IMAGE_UNKNOWN
};

/**
 * Information about color type of PNG format
 * As defined by https://www.w3.org/TR/2003/REC-PNG-20031110/#6Colour-values
 */
enum /* png colortype */ {
    // Greyscale
    PNG_COLOR_GREYSCALE = 0,
    // Truecolour
    PNG_COLOR_RGB = 2,
    // Indexed-colour
    PNG_COLOR_INDEXED = 3,
    // Greyscale with alpha
    PNG_COLOR_GREYSCALE_A = 4,
    // Truecolour with alpha
    PNG_COLOR_RGBA = 6,

    PNG_COLOR_INVALID = 255
};

/**
 * png_header describes the header information extracted from .PNG files
 */
struct png_header {
    uint32_t width;    //!< Width in pixels
    uint32_t height;   //!< Height in pixels
    uint8_t bitDepth;  //!< Bit Depth
    uint8_t colorType; //!< Color type - see PNG_COLOR_xx
    uint8_t deflate;   //!< Deflate setting
    uint8_t filtering; //!< Filtering
    uint8_t interlace; //!< Interlacing
};

/**
 * pdf_img_info describes the metadata for an arbitrary image
 */
struct pdf_img_info {
    int image_format; //!< Indicates the image format (IMAGE_PNG, ...)
    uint32_t width;   //!< Width in pixels
    uint32_t height;  //!< Height in pixels

    struct png_header png;   //!< PNG header info
};

/**
 * Convert a value in inches into a number of points.
 * @param inch inches value to convert to points
 */
#define PDF_INCH_TO_POINT(inch) ((float)((inch)*72.0f))

/**
 * Convert a value in milli-meters into a number of points.
 * @param mm millimeter value to convert to points
 */
#define PDF_MM_TO_POINT(mm) ((float)((mm)*72.0f / 25.4f))

/*! Point width of a standard US-Letter page */
#define PDF_LETTER_WIDTH PDF_INCH_TO_POINT(8.5f)

/*! Point height of a standard US-Letter page */
#define PDF_LETTER_HEIGHT PDF_INCH_TO_POINT(11.0f)

/*! Point width of a standard A4 page */
#define PDF_A4_WIDTH PDF_MM_TO_POINT(210.0f)

/*! Point height of a standard A4 page */
#define PDF_A4_HEIGHT PDF_MM_TO_POINT(297.0f)

/*! Point width of a standard A3 page */
#define PDF_A3_WIDTH PDF_MM_TO_POINT(297.0f)

/*! Point height of a standard A3 page */
#define PDF_A3_HEIGHT PDF_MM_TO_POINT(420.0f)

/**
 * Convert three 8-bit RGB values into a single packed 32-bit
 * colour. These 32-bit colours are used by various functions
 * in PDFGen
 */
#define PDF_RGB(r, g, b)                                                     \
    (uint32_t)((((r)&0xff) << 16) | (((g)&0xff) << 8) | (((b)&0xff)))

/**
 * Convert four 8-bit ARGB values into a single packed 32-bit
 * colour. These 32-bit colours are used by various functions
 * in PDFGen. Alpha values range from 0 (opaque) to 0xff
 * (transparent)
 */
#define PDF_ARGB(a, r, g, b)                                                 \
    (uint32_t)(((uint32_t)((a)&0xff) << 24) | (((r)&0xff) << 16) |           \
               (((g)&0xff) << 8) | (((b)&0xff)))

/*! Utility macro to provide bright red */
#define PDF_RED PDF_RGB(0xff, 0, 0)

/*! Utility macro to provide bright green */
#define PDF_GREEN PDF_RGB(0, 0xff, 0)

/*! Utility macro to provide bright blue */
#define PDF_BLUE PDF_RGB(0, 0, 0xff)

/*! Utility macro to provide black */
#define PDF_BLACK PDF_RGB(0, 0, 0)

/*! Utility macro to provide white */
#define PDF_WHITE PDF_RGB(0xff, 0xff, 0xff)

/*!
 * Utility macro to provide a transparent colour
 * This is used in some places for 'fill' colours, where no fill is required
 */
#define PDF_TRANSPARENT (uint32_t)(0xffu << 24)

/**
 * Different alignment options for rendering text
 */
enum {
    PDF_ALIGN_LEFT,    //!< Align text to the left
    PDF_ALIGN_RIGHT,   //!< Align text to the right
    PDF_ALIGN_CENTER,  //!< Align text in the center
    PDF_ALIGN_JUSTIFY, //!< Align text in the center, with padding to fill the
                       //!< available space
    PDF_ALIGN_JUSTIFY_ALL, //!< Like PDF_ALIGN_JUSTIFY, except even short
                           //!< lines will be fully justified
    PDF_ALIGN_NO_WRITE, //!< Fake alignment for only checking wrap height with
                        //!< no writes
};

/**
 * Create a new PDF object, with the given page
 * width/height
 * @param width Width of the page
 * @param height Height of the page
 * @param info Optional information to be put into the PDF header
 * @return PDF document object, or NULL on failure
 */
struct pdf_doc *pdf_create(float width, float height, const struct pdf_info *info);

/**
 * Destroy the pdf object, and all of its associated memory
 * @param pdf PDF document to clean up
 */
void pdf_destroy(struct pdf_doc *pdf);

/**
 * Retrieve the error message if any operation fails
 * @param pdf pdf document to retrieve error message from
 * @param errval optional pointer to an integer to be set to the error code
 * @return NULL if no error message, string description of error otherwise
 */
const char *pdf_get_err(const struct pdf_doc *pdf, int *errval);

/**
 * Acknowledge an outstanding pdf error
 * @param pdf pdf document to clear the error message from
 */
void pdf_clear_err(struct pdf_doc *pdf);

/**
 * Sets the font to use for text objects. Default value is Times-Roman if
 * this function is not called.
 * Note: The font selection should be done before text is output,
 * and will remain until pdf_set_font is called again.
 * @param pdf PDF document to update font on
 * @param font New font to use. This must be one of the standard PDF fonts:
 *  Courier, Courier-Bold, Courier-BoldOblique, Courier-Oblique,
 *  Helvetica, Helvetica-Bold, Helvetica-BoldOblique, Helvetica-Oblique,
 *  Times-Roman, Times-Bold, Times-Italic, Times-BoldItalic,
 *  Symbol or ZapfDingbats
 * @return < 0 on failure, 0 on success
 */
int pdf_set_font(struct pdf_doc *pdf, const char *font);

/**
 * Calculate the width of a given string in the current font
 * @param pdf PDF document
 * @param font_name Name of the font to get the width of.
 *  This must be one of the standard PDF fonts:
 *  Courier, Courier-Bold, Courier-BoldOblique, Courier-Oblique,
 *  Helvetica, Helvetica-Bold, Helvetica-BoldOblique, Helvetica-Oblique,
 *  Times-Roman, Times-Bold, Times-Italic, Times-BoldItalic,
 *  Symbol or ZapfDingbats
 * @param text Text to determine width of
 * @param size Size of the text, in points
 * @param text_width area to store calculated width in
 * @return < 0 on failure, 0 on success
 */
int pdf_get_font_text_width(struct pdf_doc *pdf, const char *font_name,
                            const char *text, float size, float *text_width);

/**
 * Retrieves a PDF document height
 * @param pdf PDF document to get height of
 * @return height of PDF document (in points)
 */
float pdf_height(const struct pdf_doc *pdf);

/**
 * Retrieves a PDF document width
 * @param pdf PDF document to get width of
 * @return width of PDF document (in points)
 */
float pdf_width(const struct pdf_doc *pdf);

/**
 * Retrieves page height
 * @param page Page object to get height of
 * @return height of page (in points)
 */
float pdf_page_height(const struct pdf_object *page);

/**
 * Retrieves page width
 * @param page Page object to get width of
 * @return width of page (in points)
 */
float pdf_page_width(const struct pdf_object *page);

/**
 * Add a new page to the given pdf
 * @param pdf PDF document to append page to
 * @return new page object
 */
struct pdf_object *pdf_append_page(struct pdf_doc *pdf, uint32_t stream_count, uint32_t image_count);

/**
 * Save the given pdf document to the given FILE output
 * @param pdf PDF document to save
 * @param fp FILE pointer to store the data into (must be writable)
 * @return < 0 on failure, >= 0 on success
 */
int pdf_save_file(struct pdf_doc *pdf);

/**
 * Add a text string to the document
 * @param pdf PDF document to add to
 * @param page Page to add object to (NULL => most recently added page)
 * @param text String to display
 * @param size Point size of the font
 * @param xoff X location to put it in
 * @param yoff Y location to put it in
 * @param colour Colour to draw the text
 * @return 0 on success, < 0 on failure
 */
int pdf_add_text(struct pdf_doc *pdf, struct pdf_object *page,
                 const char *text, float size, float xoff, float yoff,
                 uint32_t colour);

int pdf_add_multiple_text_spacing(struct pdf_doc *pdf, struct pdf_object *page,
                                const char *text, size_t text_lines, size_t text_cols, float size, float xoff,
                                float yoff, uint32_t colour, float spacing, float leading, const float *col_offsets, bool truncate_cells = true);

int pdf_add_horizontal_lines(struct pdf_doc *pdf, struct pdf_object *page, float x1,
                 float y1, float x2, float y2, float width, uint32_t colour, float spacing, int count, bool first_line_double_wide);

/**
 * Add a text string to the document, making it wrap if it is too
 * long
 * @param pdf PDF document to add to
 * @param page Page to add object to (NULL => most recently added page)
 * @param text String to display
 * @param size Point size of the font
 * @param xoff X location to put it in
 * @param yoff Y location to put it in
 * @param colour Colour to draw the text
 * @param wrap_width Width at which to wrap the text
 * @param align Text alignment (see PDF_ALIGN_xxx)
 * @param height Store the final height of the wrapped text here (optional)
 * @return < 0 on failure, >= 0 on success
 */
int pdf_add_text_wrap(struct pdf_doc *pdf, struct pdf_object *page,
                      const char *text, float size, float xoff, float yoff,
                      uint32_t colour, float wrap_width, int align,
                      float *height);

/**
 * Add a line to the document
 * @param pdf PDF document to add to
 * @param page Page to add object to (NULL => most recently added page)
 * @param x1 X offset of start of line
 * @param y1 Y offset of start of line
 * @param x2 X offset of end of line
 * @param y2 Y offset of end of line
 * @param width Width of the line
 * @param colour Colour to draw the line
 * @return 0 on success, < 0 on failure
 */
int pdf_add_line(struct pdf_doc *pdf, struct pdf_object *page, float x1,
                 float y1, float x2, float y2, float width, uint32_t colour);


/**
 * Add an outline rectangle to the document
 * @param pdf PDF document to add to
 * @param page Page to add object to (NULL => most recently added page)
 * @param x X offset to start rectangle at
 * @param y Y offset to start rectangle at
 * @param width Width of rectangle
 * @param height Height of rectangle
 * @param border_width Width of rectangle border
 * @param colour Colour to draw the rectangle
 * @return 0 on success, < 0 on failure
 */
int pdf_add_rectangle(struct pdf_doc *pdf, struct pdf_object *page, float x,
                      float y, float width, float height, float border_width,
                      uint32_t colour);

/**
 * Add a filled rectangle to the document
 * @param pdf PDF document to add to
 * @param page Page to add object to (NULL => most recently added page)
 * @param x X offset to start rectangle at
 * @param y Y offset to start rectangle at
 * @param width Width of rectangle
 * @param height Height of rectangle
 * @param border_width Width of rectangle border
 * @param colour_fill Colour to fill the rectangle
 * @param colour_border Colour to draw the rectangle
 * @return 0 on success, < 0 on failure
 */
int pdf_add_filled_rectangle(struct pdf_doc *pdf, struct pdf_object *page,
                             float x, float y, float width, float height,
                             float border_width, uint32_t colour_fill,
                             uint32_t colour_border);

// Simple data container to store a single 24 Bit RGB value, used for
// processing PNG images
struct rgb_value {
    uint8_t red;
    uint8_t blue;
    uint8_t green;
    uint8_t alpha;
};

/**
 * Add image data as an image to the document.
 * Image data must be one of: JPEG, PNG, PPM, PGM or BMP formats
 * Passing 0 for either the display width or height will
 * include the image but not render it visible.
 * Passing a negative number either the display height or width will
 * have the image be resized while keeping the original aspect ratio.
 * @param pdf PDF document to add image to
 * @param page Page to add image to (NULL => most recently added page)
 * @param x X offset to put image at
 * @param y Y offset to put image at
 * @param display_width Displayed width of image
 * @param display_height Displayed height of image
 * @param data Image data bytes
 * @param len Length of data
 * @return < 0 on failure, >= 0 on success
 */
int pdf_add_png_image_data(struct pdf_doc *pdf, struct pdf_object *page, float x,
                       float y, float display_width, float display_height,
                       uint32_t colour_background_hint,
                       const uint8_t *data, size_t len);


int pdf_add_write_callback(struct pdf_doc *pdf, std::function<ssize_t(const void *buf, size_t len)> cb);
int pdf_add_stream_callback(struct pdf_doc *pdf, std::function<int(struct pdf_doc *pdf, uint32_t page_num, uint32_t stream_num)> cb);
int pdf_add_image_callback(struct pdf_doc *pdf, std::function<int(struct pdf_doc *pdf, uint32_t page_num, uint32_t image_num)> cb);
int pdf_add_page_callback(struct pdf_doc *pdf, std::function<int(struct pdf_doc *pdf, uint32_t page_num)> cb);

void pdf_notify_page(struct pdf_doc *pdf, uint32_t stream_count, uint32_t image_count);

/*int pdf_add_image(struct pdf_doc *pdf, struct pdf_object *page,
                         struct pdf_object *image, float x, float y,
                         float width, float height);*/

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define DEFAULT_FONT "Times-Roman"

#endif // PDFGEN_H
