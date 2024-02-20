/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_RGB_LED_MATRIX_H
#define TF_RGB_LED_MATRIX_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_RGBLEDMatrix RGB LED Matrix Bricklet
 */

struct TF_RGBLEDMatrix;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_RGBLEDMatrix_FrameStartedHandler)(struct TF_RGBLEDMatrix *rgb_led_matrix, uint32_t frame_number, void *user_data);

#endif
/**
 * \ingroup TF_RGBLEDMatrix
 *
 * RGB LED Matrix with 8x8 pixel
 */
typedef struct TF_RGBLEDMatrix {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_RGBLEDMatrix_FrameStartedHandler frame_started_handler;
    void *frame_started_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[2];
} TF_RGBLEDMatrix;

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_SET_RED 1

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_GET_RED 2

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_SET_GREEN 3

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_GET_GREEN 4

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_SET_BLUE 5

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_GET_BLUE 6

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_SET_FRAME_DURATION 7

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_GET_FRAME_DURATION 8

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_DRAW_FRAME 9

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_GET_SUPPLY_VOLTAGE 10

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_RESET 243

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_READ_UID 249

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_CALLBACK_FRAME_STARTED 11

#endif

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_RGBLEDMatrix
 */
#define TF_RGB_LED_MATRIX_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * This constant is used to identify a RGB LED Matrix Bricklet.
 *
 * The {@link rgb_led_matrix_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_RGB_LED_MATRIX_DEVICE_IDENTIFIER 272

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * This constant represents the display name of a RGB LED Matrix Bricklet.
 */
#define TF_RGB_LED_MATRIX_DEVICE_DISPLAY_NAME "RGB LED Matrix Bricklet"

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Creates the device object \c rgb_led_matrix with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_rgb_led_matrix_create(TF_RGBLEDMatrix *rgb_led_matrix, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Removes the device object \c rgb_led_matrix from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_rgb_led_matrix_destroy(TF_RGBLEDMatrix *rgb_led_matrix);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_rgb_led_matrix_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_rgb_led_matrix_get_response_expected(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_RGBLEDMatrix
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
int tf_rgb_led_matrix_set_response_expected(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_rgb_led_matrix_set_response_expected_all(TF_RGBLEDMatrix *rgb_led_matrix, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Registers the given \c handler to the Frame Started callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint32_t frame_number, void *user_data) \endcode
 *
 * This callback is triggered as soon as a new frame write is started.
 * The LED values are double buffered, so you can send the LED values
 * for the next frame directly after this callback is triggered.
 */
int tf_rgb_led_matrix_register_frame_started_callback(TF_RGBLEDMatrix *rgb_led_matrix, TF_RGBLEDMatrix_FrameStartedHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_rgb_led_matrix_callback_tick(TF_RGBLEDMatrix *rgb_led_matrix, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Sets the 64 red LED values of the matrix.
 */
int tf_rgb_led_matrix_set_red(TF_RGBLEDMatrix *rgb_led_matrix, const uint8_t red[64]);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Returns the red LED values as set by {@link tf_rgb_led_matrix_set_red}.
 */
int tf_rgb_led_matrix_get_red(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t ret_red[64]);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Sets the 64 green LED values of the matrix.
 */
int tf_rgb_led_matrix_set_green(TF_RGBLEDMatrix *rgb_led_matrix, const uint8_t green[64]);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Returns the green LED values as set by {@link tf_rgb_led_matrix_set_green}.
 */
int tf_rgb_led_matrix_get_green(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t ret_green[64]);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Sets the 64 blue LED values of the matrix.
 */
int tf_rgb_led_matrix_set_blue(TF_RGBLEDMatrix *rgb_led_matrix, const uint8_t blue[64]);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Returns the blue LED values as set by {@link tf_rgb_led_matrix_set_blue}.
 */
int tf_rgb_led_matrix_get_blue(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t ret_blue[64]);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Sets the frame duration.
 *
 * Example: If you want to achieve 20 frames per second, you should
 * set the frame duration to 50ms (50ms * 20 = 1 second).
 *
 * Set this value to 0 to turn the automatic frame write mechanism off.
 *
 * Approach:
 *
 * * Call {@link tf_rgb_led_matrix_set_frame_duration} with value > 0.
 * * Set LED values for first frame with {@link tf_rgb_led_matrix_set_red}, {@link tf_rgb_led_matrix_set_green}, {@link tf_rgb_led_matrix_set_blue}.
 * * Wait for {@link tf_rgb_led_matrix_register_frame_started_callback} callback.
 * * Set LED values for second frame with {@link tf_rgb_led_matrix_set_red}, {@link tf_rgb_led_matrix_set_green}, {@link tf_rgb_led_matrix_set_blue}.
 * * Wait for {@link tf_rgb_led_matrix_register_frame_started_callback} callback.
 * * and so on.
 *
 * For frame duration of 0 see {@link tf_rgb_led_matrix_draw_frame}.
 */
int tf_rgb_led_matrix_set_frame_duration(TF_RGBLEDMatrix *rgb_led_matrix, uint16_t frame_duration);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Returns the frame duration as set by {@link tf_rgb_led_matrix_set_frame_duration}.
 */
int tf_rgb_led_matrix_get_frame_duration(TF_RGBLEDMatrix *rgb_led_matrix, uint16_t *ret_frame_duration);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * If you set the frame duration to 0 (see {@link tf_rgb_led_matrix_set_frame_duration}), you can use this
 * function to transfer the frame to the matrix.
 *
 * Approach:
 *
 * * Call {@link tf_rgb_led_matrix_set_frame_duration} with 0.
 * * Set LED values for first frame with {@link tf_rgb_led_matrix_set_red}, {@link tf_rgb_led_matrix_set_green}, {@link tf_rgb_led_matrix_set_blue}.
 * * Call {@link tf_rgb_led_matrix_draw_frame}.
 * * Wait for {@link tf_rgb_led_matrix_register_frame_started_callback} callback.
 * * Set LED values for second frame with {@link tf_rgb_led_matrix_set_red}, {@link tf_rgb_led_matrix_set_green}, {@link tf_rgb_led_matrix_set_blue}.
 * * Call {@link tf_rgb_led_matrix_draw_frame}.
 * * Wait for {@link tf_rgb_led_matrix_register_frame_started_callback} callback.
 * * and so on.
 */
int tf_rgb_led_matrix_draw_frame(TF_RGBLEDMatrix *rgb_led_matrix);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Returns the current supply voltage of the Bricklet.
 */
int tf_rgb_led_matrix_get_supply_voltage(TF_RGBLEDMatrix *rgb_led_matrix, uint16_t *ret_voltage);

/**
 * \ingroup TF_RGBLEDMatrix
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
int tf_rgb_led_matrix_get_spitfp_error_count(TF_RGBLEDMatrix *rgb_led_matrix, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_RGBLEDMatrix
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
int tf_rgb_led_matrix_set_bootloader_mode(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Returns the current bootloader mode, see {@link tf_rgb_led_matrix_set_bootloader_mode}.
 */
int tf_rgb_led_matrix_get_bootloader_mode(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t *ret_mode);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Sets the firmware pointer for {@link tf_rgb_led_matrix_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_rgb_led_matrix_set_write_firmware_pointer(TF_RGBLEDMatrix *rgb_led_matrix, uint32_t pointer);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_rgb_led_matrix_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_rgb_led_matrix_write_firmware(TF_RGBLEDMatrix *rgb_led_matrix, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_rgb_led_matrix_set_status_led_config(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t config);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Returns the configuration as set by {@link tf_rgb_led_matrix_set_status_led_config}
 */
int tf_rgb_led_matrix_get_status_led_config(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t *ret_config);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_rgb_led_matrix_get_chip_temperature(TF_RGBLEDMatrix *rgb_led_matrix, int16_t *ret_temperature);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_rgb_led_matrix_reset(TF_RGBLEDMatrix *rgb_led_matrix);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_rgb_led_matrix_write_uid(TF_RGBLEDMatrix *rgb_led_matrix, uint32_t uid);

/**
 * \ingroup TF_RGBLEDMatrix
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_rgb_led_matrix_read_uid(TF_RGBLEDMatrix *rgb_led_matrix, uint32_t *ret_uid);

/**
 * \ingroup TF_RGBLEDMatrix
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
int tf_rgb_led_matrix_get_identity(TF_RGBLEDMatrix *rgb_led_matrix, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
