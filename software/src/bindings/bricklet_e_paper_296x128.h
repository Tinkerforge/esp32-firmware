/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_E_PAPER_296X128_H
#define TF_E_PAPER_296X128_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_EPaper296x128 E-Paper 296x128 Bricklet
 */

struct TF_EPaper296x128;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_EPaper296x128_DrawStatusHandler)(struct TF_EPaper296x128 *e_paper_296x128, uint8_t draw_status, void *user_data);

#endif
/**
 * \ingroup TF_EPaper296x128
 *
 * Three color 296x128 e-paper display
 */
typedef struct TF_EPaper296x128 {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_EPaper296x128_DrawStatusHandler draw_status_handler;
    void *draw_status_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[2];
} TF_EPaper296x128;

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_DRAW 1

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_GET_DRAW_STATUS 2

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_WRITE_BLACK_WHITE_LOW_LEVEL 3

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_READ_BLACK_WHITE_LOW_LEVEL 4

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_WRITE_COLOR_LOW_LEVEL 5

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_READ_COLOR_LOW_LEVEL 6

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_FILL_DISPLAY 7

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_DRAW_TEXT 8

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_DRAW_LINE 9

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_DRAW_BOX 10

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_SET_UPDATE_MODE 12

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_GET_UPDATE_MODE 13

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_SET_DISPLAY_TYPE 14

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_GET_DISPLAY_TYPE 15

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_SET_DISPLAY_DRIVER 16

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_GET_DISPLAY_DRIVER 17

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_RESET 243

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_READ_UID 249

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_CALLBACK_DRAW_STATUS 11

#endif

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_DRAW_STATUS_IDLE 0

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_DRAW_STATUS_COPYING 1

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_DRAW_STATUS_DRAWING 2

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_COLOR_BLACK 0

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_COLOR_WHITE 1

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_COLOR_RED 2

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_COLOR_GRAY 2

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FONT_6X8 0

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FONT_6X16 1

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FONT_6X24 2

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FONT_6X32 3

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FONT_12X16 4

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FONT_12X24 5

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FONT_12X32 6

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FONT_18X24 7

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FONT_18X32 8

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_FONT_24X32 9

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_ORIENTATION_HORIZONTAL 0

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_ORIENTATION_VERTICAL 1

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_UPDATE_MODE_DEFAULT 0

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_UPDATE_MODE_BLACK_WHITE 1

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_UPDATE_MODE_DELTA 2

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_DISPLAY_TYPE_BLACK_WHITE_RED 0

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_DISPLAY_TYPE_BLACK_WHITE_GRAY 1

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_DISPLAY_DRIVER_SSD1675A 0

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_DISPLAY_DRIVER_SSD1680 1

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_EPaper296x128
 */
#define TF_E_PAPER_296X128_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_EPaper296x128
 *
 * This constant is used to identify a E-Paper 296x128 Bricklet.
 *
 * The {@link e_paper_296x128_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_E_PAPER_296X128_DEVICE_IDENTIFIER 2146

/**
 * \ingroup TF_EPaper296x128
 *
 * This constant represents the display name of a E-Paper 296x128 Bricklet.
 */
#define TF_E_PAPER_296X128_DEVICE_DISPLAY_NAME "E-Paper 296x128 Bricklet"

/**
 * \ingroup TF_EPaper296x128
 *
 * Creates the device object \c e_paper_296x128 with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_e_paper_296x128_create(TF_EPaper296x128 *e_paper_296x128, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_EPaper296x128
 *
 * Removes the device object \c e_paper_296x128 from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_e_paper_296x128_destroy(TF_EPaper296x128 *e_paper_296x128);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_e_paper_296x128_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_e_paper_296x128_get_response_expected(TF_EPaper296x128 *e_paper_296x128, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_EPaper296x128
 *
 * Changes the response expected flag of the function specified by the
 * \c function_id parameter. This flag can only be changed for setter
 * (default value: *false*) and callback configuration functions
 * (default value: *true*). For getter functions it is always enabled.
 *
 * Enabling the response expected flag for a setter function allows to detect
 * timeouts and other error conditions calls of this setter as well. The device
 * will then send a response for this purpose. If this flag is disabled for a
 * setter function then no response is sent and errors are silently ignored,
 * because they cannot be detected.
 */
int tf_e_paper_296x128_set_response_expected(TF_EPaper296x128 *e_paper_296x128, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_EPaper296x128
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_e_paper_296x128_set_response_expected_all(TF_EPaper296x128 *e_paper_296x128, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_EPaper296x128
 *
 * Registers the given \c handler to the Draw Status callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t draw_status, void *user_data) \endcode
 *
 * Callback for the current draw status. Will be called every time the
 * draw status changes (see {@link tf_e_paper_296x128_get_draw_status}).
 */
int tf_e_paper_296x128_register_draw_status_callback(TF_EPaper296x128 *e_paper_296x128, TF_EPaper296x128_DrawStatusHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_EPaper296x128
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_e_paper_296x128_callback_tick(TF_EPaper296x128 *e_paper_296x128, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_EPaper296x128
 *
 * Draws the current black/white and red or gray buffer to the e-paper display.
 *
 * The Bricklet does not have any double-buffering. You should not call
 * this function while writing to the buffer. See {@link tf_e_paper_296x128_get_draw_status}.
 */
int tf_e_paper_296x128_draw(TF_EPaper296x128 *e_paper_296x128);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns one of three draw statuses:
 *
 * * Idle
 * * Copying: Data is being copied from the buffer of the Bricklet to the buffer of the display.
 * * Drawing: The display is updating its content (during this phase the flickering etc happens).
 *
 * You can write to the buffer (through one of the write or draw functions) when the status is
 * either *idle* or *drawing*. You should not write to the buffer while it is being *copied* to the
 * display. There is no double-buffering.
 */
int tf_e_paper_296x128_get_draw_status(TF_EPaper296x128 *e_paper_296x128, uint8_t *ret_draw_status);

/**
 * \ingroup TF_EPaper296x128
 *
 * Writes black/white pixels to the specified window into the buffer.
 *
 * The pixels are written into the window line by line top to bottom
 * and each line is written from left to right.
 *
 * The value 0 (false) corresponds to a black pixel and the value 1 (true) to a
 * white pixel.
 *
 * This function writes the pixels into the black/white pixel buffer, to draw the
 * buffer to the display use {@link tf_e_paper_296x128_draw}.
 *
 * Use {@link tf_e_paper_296x128_write_color} to write red or gray pixels.
 */
int tf_e_paper_296x128_write_black_white_low_level(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, uint16_t pixels_length, uint16_t pixels_chunk_offset, const bool pixels_chunk_data[432]);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the current content of the black/white pixel buffer for the specified window.
 *
 * The pixels are read into the window line by line top to bottom and
 * each line is read from left to right.
 *
 * The current content of the buffer does not have to be the current content of the display.
 * It is possible that the data was not drawn to the display yet and after a restart of
 * the Bricklet the buffer will be reset to black, while the display retains its content.
 */
int tf_e_paper_296x128_read_black_white_low_level(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, uint16_t *ret_pixels_length, uint16_t *ret_pixels_chunk_offset, bool ret_pixels_chunk_data[464]);

/**
 * \ingroup TF_EPaper296x128
 *
 * The E-Paper 296x128 Bricklet is available with the colors black/white/red and
 * black/white/gray. Depending on the model this function writes either red or
 * gray pixels to the specified window into the buffer.
 *
 * The pixels are written into the window line by line top to bottom
 * and each line is written from left to right.
 *
 * The value 0 (false) means that this pixel does not have color. It will be either black
 * or white (see {@link tf_e_paper_296x128_write_black_white}). The value 1 (true) corresponds to a red or gray
 * pixel, depending on the Bricklet model.
 *
 * This function writes the pixels into the red or gray pixel buffer, to draw the buffer
 * to the display use {@link tf_e_paper_296x128_draw}.
 *
 * Use {@link tf_e_paper_296x128_write_black_white} to write black/white pixels.
 */
int tf_e_paper_296x128_write_color_low_level(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, uint16_t pixels_length, uint16_t pixels_chunk_offset, const bool pixels_chunk_data[432]);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the current content of the red or gray pixel buffer for the specified window.
 *
 * The pixels are written into the window line by line top to bottom
 * and each line is written from left to right.
 *
 * The current content of the buffer does not have to be the current content of the display.
 * It is possible that the data was not drawn to the display yet and after a restart of
 * the Bricklet the buffer will be reset to black, while the display retains its content.
 */
int tf_e_paper_296x128_read_color_low_level(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, uint16_t *ret_pixels_length, uint16_t *ret_pixels_chunk_offset, bool ret_pixels_chunk_data[464]);

/**
 * \ingroup TF_EPaper296x128
 *
 * Fills the complete content of the display with the given color.
 *
 * This function writes the pixels into the black/white/red|gray pixel buffer, to draw the buffer
 * to the display use {@link tf_e_paper_296x128_draw}.
 */
int tf_e_paper_296x128_fill_display(TF_EPaper296x128 *e_paper_296x128, uint8_t color);

/**
 * \ingroup TF_EPaper296x128
 *
 * Draws a text with up to 50 characters at the pixel position (x, y).
 *
 * You can use one of 9 different font sizes and draw the text in
 * black/white/red|gray. The text can be drawn horizontal or vertical.
 *
 * This function writes the pixels into the black/white/red|gray pixel buffer, to draw the buffer
 * to the display use {@link tf_e_paper_296x128_draw}.
 *
 * The font conforms to code page 437.
 */
int tf_e_paper_296x128_draw_text(TF_EPaper296x128 *e_paper_296x128, uint16_t position_x, uint8_t position_y, uint8_t font, uint8_t color, uint8_t orientation, const char *text);

/**
 * \ingroup TF_EPaper296x128
 *
 * Draws a line from (x, y)-start to (x, y)-end in the given color.
 *
 * This function writes the pixels into the black/white/red|gray pixel buffer, to draw the buffer
 * to the display use {@link tf_e_paper_296x128_draw}.
 */
int tf_e_paper_296x128_draw_line(TF_EPaper296x128 *e_paper_296x128, uint16_t position_x_start, uint8_t position_y_start, uint16_t position_x_end, uint8_t position_y_end, uint8_t color);

/**
 * \ingroup TF_EPaper296x128
 *
 * Draws a box from (x, y)-start to (x, y)-end in the given color.
 *
 * If you set fill to true, the box will be filled with the
 * color. Otherwise only the outline will be drawn.
 *
 * This function writes the pixels into the black/white/red|gray pixel buffer, to draw the buffer
 * to the display use {@link tf_e_paper_296x128_draw}.
 */
int tf_e_paper_296x128_draw_box(TF_EPaper296x128 *e_paper_296x128, uint16_t position_x_start, uint8_t position_y_start, uint16_t position_x_end, uint8_t position_y_end, bool fill, uint8_t color);

/**
 * \ingroup TF_EPaper296x128
 *
 * \note
 *  The default update mode corresponds to the default e-paper display
 *  manufacturer settings. All of the other modes are experimental and
 *  will result in increased ghosting and possibly other long-term
 *  side effects.
 *
 *  If you want to know more about the inner workings of an e-paper display
 *  take a look at this excellent video from Ben Krasnow:
 *  `https://www.youtube.com/watch?v=MsbiO8EAsGw <https://www.youtube.com/watch?v=MsbiO8EAsGw>`__.
 *
 *  If you are not sure about this option, leave the update mode at default.
 *
 * Currently there are three update modes available:
 *
 * * Default: Settings as given by the manufacturer. An update will take about
 *   7.5 seconds and during the update the screen will flicker several times.
 * * Black/White: This will only update the black/white pixel. It uses the manufacturer
 *   settings for black/white and ignores the red or gray pixel buffer. With this mode the
 *   display will flicker once and it takes about 2.5 seconds. Compared to the default settings
 *   there is more ghosting.
 * * Delta: This will only update the black/white pixel. It uses an aggressive method where
 *   the changes are not applied for a whole buffer but only for the delta between the last
 *   and the next buffer. With this mode the display will not flicker during an update and
 *   it takes about 900-950ms. Compared to the other two settings there is more ghosting. This
 *   mode can be used for something like a flicker-free live update of a text.
 *
 * With the black/white/red display if you use either the black/white or the delta mode,
 * after a while of going back and forth between black and white the white color will
 * start to appear red-ish or pink-ish.
 *
 * If you use the aggressive delta mode and rapidly change the content, we recommend that you
 * change back to the default mode every few hours and in the default mode cycle between the
 * three available colors a few times. This will get rid of the ghosting and after that you can
 * go back to the delta mode with flicker-free updates.
 */
int tf_e_paper_296x128_set_update_mode(TF_EPaper296x128 *e_paper_296x128, uint8_t update_mode);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the update mode as set by {@link tf_e_paper_296x128_set_update_mode}.
 */
int tf_e_paper_296x128_get_update_mode(TF_EPaper296x128 *e_paper_296x128, uint8_t *ret_update_mode);

/**
 * \ingroup TF_EPaper296x128
 *
 * Sets the type of the display. The e-paper display is available
 * in black/white/red and black/white/gray. This will be factory set
 * during the flashing and testing phase. The value is saved in
 * non-volatile memory and will stay after a power cycle.
 */
int tf_e_paper_296x128_set_display_type(TF_EPaper296x128 *e_paper_296x128, uint8_t display_type);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the type of the e-paper display. It can either be
 * black/white/red or black/white/gray.
 */
int tf_e_paper_296x128_get_display_type(TF_EPaper296x128 *e_paper_296x128, uint8_t *ret_display_type);

/**
 * \ingroup TF_EPaper296x128
 *
 * Sets the type of display driver. The Bricklet can currently support
 * SSD1675A and SSD1680. This will be factory set
 * during the flashing and testing phase. The value is saved in
 * non-volatile memory and will stay after a power cycle.
 *
 * .. versionadded:: 2.0.3$nbsp;(Plugin)
 */
int tf_e_paper_296x128_set_display_driver(TF_EPaper296x128 *e_paper_296x128, uint8_t display_driver);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the e-paper display driver.
 *
 * .. versionadded:: 2.0.3$nbsp;(Plugin)
 */
int tf_e_paper_296x128_get_display_driver(TF_EPaper296x128 *e_paper_296x128, uint8_t *ret_display_driver);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the error count for the communication between Brick and Bricklet.
 *
 * The errors are divided into
 *
 * * ACK checksum errors,
 * * message checksum errors,
 * * framing errors and
 * * overflow errors.
 *
 * The errors counts are for errors that occur on the Bricklet side. All
 * Bricks have a similar function that returns the errors on the Brick side.
 */
int tf_e_paper_296x128_get_spitfp_error_count(TF_EPaper296x128 *e_paper_296x128, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_EPaper296x128
 *
 * Sets the bootloader mode and returns the status after the requested
 * mode change was instigated.
 *
 * You can change from bootloader mode to firmware mode and vice versa. A change
 * from bootloader mode to firmware mode will only take place if the entry function,
 * device identifier and CRC are present and correct.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_e_paper_296x128_set_bootloader_mode(TF_EPaper296x128 *e_paper_296x128, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the current bootloader mode, see {@link tf_e_paper_296x128_set_bootloader_mode}.
 */
int tf_e_paper_296x128_get_bootloader_mode(TF_EPaper296x128 *e_paper_296x128, uint8_t *ret_mode);

/**
 * \ingroup TF_EPaper296x128
 *
 * Sets the firmware pointer for {@link tf_e_paper_296x128_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_e_paper_296x128_set_write_firmware_pointer(TF_EPaper296x128 *e_paper_296x128, uint32_t pointer);

/**
 * \ingroup TF_EPaper296x128
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_e_paper_296x128_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_e_paper_296x128_write_firmware(TF_EPaper296x128 *e_paper_296x128, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_EPaper296x128
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_e_paper_296x128_set_status_led_config(TF_EPaper296x128 *e_paper_296x128, uint8_t config);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the configuration as set by {@link tf_e_paper_296x128_set_status_led_config}
 */
int tf_e_paper_296x128_get_status_led_config(TF_EPaper296x128 *e_paper_296x128, uint8_t *ret_config);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_e_paper_296x128_get_chip_temperature(TF_EPaper296x128 *e_paper_296x128, int16_t *ret_temperature);

/**
 * \ingroup TF_EPaper296x128
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_e_paper_296x128_reset(TF_EPaper296x128 *e_paper_296x128);

/**
 * \ingroup TF_EPaper296x128
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_e_paper_296x128_write_uid(TF_EPaper296x128 *e_paper_296x128, uint32_t uid);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_e_paper_296x128_read_uid(TF_EPaper296x128 *e_paper_296x128, uint32_t *ret_uid);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the UID, the UID where the Bricklet is connected to,
 * the position, the hardware and firmware version as well as the
 * device identifier.
 *
 * The position can be 'a', 'b', 'c', 'd', 'e', 'f', 'g' or 'h' (Bricklet Port).
 * A Bricklet connected to an :ref:`Isolator Bricklet <isolator_bricklet>` is always at
 * position 'z'.
 *
 * The device identifier numbers can be found :ref:`here <device_identifier>`.
 * |device_identifier_constant|
 */
int tf_e_paper_296x128_get_identity(TF_EPaper296x128 *e_paper_296x128, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

/**
 * \ingroup TF_EPaper296x128
 *
 * Writes black/white pixels to the specified window into the buffer.
 *
 * The pixels are written into the window line by line top to bottom
 * and each line is written from left to right.
 *
 * The value 0 (false) corresponds to a black pixel and the value 1 (true) to a
 * white pixel.
 *
 * This function writes the pixels into the black/white pixel buffer, to draw the
 * buffer to the display use {@link tf_e_paper_296x128_draw}.
 *
 * Use {@link tf_e_paper_296x128_write_color} to write red or gray pixels.
 */
int tf_e_paper_296x128_write_black_white(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, const bool *pixels, uint16_t pixels_length);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the current content of the black/white pixel buffer for the specified window.
 *
 * The pixels are read into the window line by line top to bottom and
 * each line is read from left to right.
 *
 * The current content of the buffer does not have to be the current content of the display.
 * It is possible that the data was not drawn to the display yet and after a restart of
 * the Bricklet the buffer will be reset to black, while the display retains its content.
 */
int tf_e_paper_296x128_read_black_white(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, bool *ret_pixels, uint16_t *ret_pixels_length);

/**
 * \ingroup TF_EPaper296x128
 *
 * The E-Paper 296x128 Bricklet is available with the colors black/white/red and
 * black/white/gray. Depending on the model this function writes either red or
 * gray pixels to the specified window into the buffer.
 *
 * The pixels are written into the window line by line top to bottom
 * and each line is written from left to right.
 *
 * The value 0 (false) means that this pixel does not have color. It will be either black
 * or white (see {@link tf_e_paper_296x128_write_black_white}). The value 1 (true) corresponds to a red or gray
 * pixel, depending on the Bricklet model.
 *
 * This function writes the pixels into the red or gray pixel buffer, to draw the buffer
 * to the display use {@link tf_e_paper_296x128_draw}.
 *
 * Use {@link tf_e_paper_296x128_write_black_white} to write black/white pixels.
 */
int tf_e_paper_296x128_write_color(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, const bool *pixels, uint16_t pixels_length);

/**
 * \ingroup TF_EPaper296x128
 *
 * Returns the current content of the red or gray pixel buffer for the specified window.
 *
 * The pixels are written into the window line by line top to bottom
 * and each line is written from left to right.
 *
 * The current content of the buffer does not have to be the current content of the display.
 * It is possible that the data was not drawn to the display yet and after a restart of
 * the Bricklet the buffer will be reset to black, while the display retains its content.
 */
int tf_e_paper_296x128_read_color(TF_EPaper296x128 *e_paper_296x128, uint16_t x_start, uint8_t y_start, uint16_t x_end, uint8_t y_end, bool *ret_pixels, uint16_t *ret_pixels_length);

#ifdef __cplusplus
}
#endif

#endif
