/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_SEGMENT_DISPLAY_4X7_V2_H
#define TF_SEGMENT_DISPLAY_4X7_V2_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_SegmentDisplay4x7V2 Segment Display 4x7 Bricklet 2.0
 */

struct TF_SegmentDisplay4x7V2;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_SegmentDisplay4x7V2_CounterFinishedHandler)(struct TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, void *user_data);

#endif
/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Four 7-segment displays with switchable dots
 */
typedef struct TF_SegmentDisplay4x7V2 {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_SegmentDisplay4x7V2_CounterFinishedHandler counter_finished_handler;
    void *counter_finished_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[2];
} TF_SegmentDisplay4x7V2;

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_SEGMENTS 1

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_SEGMENTS 2

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_BRIGHTNESS 3

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_BRIGHTNESS 4

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_NUMERIC_VALUE 5

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_SELECTED_SEGMENT 6

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_SELECTED_SEGMENT 7

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_START_COUNTER 8

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_COUNTER_VALUE 9

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_RESET 243

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_READ_UID 249

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_CALLBACK_COUNTER_FINISHED 10

#endif

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_SegmentDisplay4x7V2
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * This constant is used to identify a Segment Display 4x7 Bricklet 2.0.
 *
 * The {@link segment_display_4x7_v2_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_DEVICE_IDENTIFIER 2137

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * This constant represents the display name of a Segment Display 4x7 Bricklet 2.0.
 */
#define TF_SEGMENT_DISPLAY_4X7_V2_DEVICE_DISPLAY_NAME "Segment Display 4x7 Bricklet 2.0"

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Creates the device object \c segment_display_4x7_v2 with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_segment_display_4x7_v2_create(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Removes the device object \c segment_display_4x7_v2 from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_segment_display_4x7_v2_destroy(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_segment_display_4x7_v2_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_segment_display_4x7_v2_get_response_expected(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_SegmentDisplay4x7V2
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
int tf_segment_display_4x7_v2_set_response_expected(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_segment_display_4x7_v2_set_response_expected_all(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Registers the given \c handler to the Counter Finished callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(void *user_data) \endcode
 *
 * This callback is triggered when the counter (see {@link tf_segment_display_4x7_v2_start_counter}) is
 * finished.
 */
int tf_segment_display_4x7_v2_register_counter_finished_callback(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, TF_SegmentDisplay4x7V2_CounterFinishedHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_segment_display_4x7_v2_callback_tick(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Sets the segments of the Segment Display 4x7 Bricklet 2.0 segment-by-segment.
 *
 * The data is split into the four digits, two colon dots and the tick mark.
 *
 * The indices of the segments in the digit and colon parameters are as follows:
 *
 * .. image:: /Images/Bricklets/bricklet_segment_display_4x7_v2_segment_index.png
 *    :scale: 100 %
 *    :alt: Indices of segments
 *    :align: center
 */
int tf_segment_display_4x7_v2_set_segments(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, const bool digit0[8], const bool digit1[8], const bool digit2[8], const bool digit3[8], const bool colon[2], bool tick);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Returns the segment data as set by {@link tf_segment_display_4x7_v2_set_segments}.
 */
int tf_segment_display_4x7_v2_get_segments(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, bool ret_digit0[8], bool ret_digit1[8], bool ret_digit2[8], bool ret_digit3[8], bool ret_colon[2], bool *ret_tick);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * The brightness can be set between 0 (dark) and 7 (bright).
 */
int tf_segment_display_4x7_v2_set_brightness(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t brightness);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Returns the brightness as set by {@link tf_segment_display_4x7_v2_set_brightness}.
 */
int tf_segment_display_4x7_v2_get_brightness(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t *ret_brightness);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Sets a numeric value for each of the digits. They represent:
 *
 * * -2: minus sign
 * * -1: blank
 * * 0-9: 0-9
 * * 10: A
 * * 11: b
 * * 12: C
 * * 13: d
 * * 14: E
 * * 15: F
 *
 * Example: A call with [-2, -1, 4, 2] will result in a display of "- 42".
 */
int tf_segment_display_4x7_v2_set_numeric_value(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, const int8_t value[4]);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Turns one specified segment on or off.
 *
 * The indices of the segments are as follows:
 *
 * .. image:: /Images/Bricklets/bricklet_segment_display_4x7_v2_selected_segment_index.png
 *    :scale: 100 %
 *    :alt: Indices of selected segments
 *    :align: center
 */
int tf_segment_display_4x7_v2_set_selected_segment(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t segment, bool value);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Returns the value of a single segment.
 */
int tf_segment_display_4x7_v2_get_selected_segment(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t segment, bool *ret_value);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Starts a counter with the *from* value that counts to the *to*
 * value with the each step incremented by *increment*.
 * *length* is the pause between each increment.
 *
 * Example: If you set *from* to 0, *to* to 100, *increment* to 1 and
 * *length* to 1000, a counter that goes from 0 to 100 with one second
 * pause between each increment will be started.
 *
 * Using a negative *increment* allows to count backwards.
 *
 * You can stop the counter at every time by calling {@link tf_segment_display_4x7_v2_set_segments}
 * or {@link tf_segment_display_4x7_v2_set_numeric_value}.
 */
int tf_segment_display_4x7_v2_start_counter(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, int16_t value_from, int16_t value_to, int16_t increment, uint32_t length);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Returns the counter value that is currently shown on the display.
 *
 * If there is no counter running a 0 will be returned.
 */
int tf_segment_display_4x7_v2_get_counter_value(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint16_t *ret_value);

/**
 * \ingroup TF_SegmentDisplay4x7V2
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
int tf_segment_display_4x7_v2_get_spitfp_error_count(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_SegmentDisplay4x7V2
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
int tf_segment_display_4x7_v2_set_bootloader_mode(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Returns the current bootloader mode, see {@link tf_segment_display_4x7_v2_set_bootloader_mode}.
 */
int tf_segment_display_4x7_v2_get_bootloader_mode(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t *ret_mode);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Sets the firmware pointer for {@link tf_segment_display_4x7_v2_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_segment_display_4x7_v2_set_write_firmware_pointer(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint32_t pointer);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_segment_display_4x7_v2_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_segment_display_4x7_v2_write_firmware(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_segment_display_4x7_v2_set_status_led_config(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t config);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Returns the configuration as set by {@link tf_segment_display_4x7_v2_set_status_led_config}
 */
int tf_segment_display_4x7_v2_get_status_led_config(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t *ret_config);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_segment_display_4x7_v2_get_chip_temperature(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, int16_t *ret_temperature);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_segment_display_4x7_v2_reset(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_segment_display_4x7_v2_write_uid(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint32_t uid);

/**
 * \ingroup TF_SegmentDisplay4x7V2
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_segment_display_4x7_v2_read_uid(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint32_t *ret_uid);

/**
 * \ingroup TF_SegmentDisplay4x7V2
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
int tf_segment_display_4x7_v2_get_identity(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
