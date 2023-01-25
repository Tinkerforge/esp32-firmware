/* ***********************************************************
 * This file was automatically generated on 2023-01-25.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.3         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_ROTARY_ENCODER_V2_H
#define TF_ROTARY_ENCODER_V2_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_RotaryEncoderV2 Rotary Encoder Bricklet 2.0
 */

struct TF_RotaryEncoderV2;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_RotaryEncoderV2_CountHandler)(struct TF_RotaryEncoderV2 *rotary_encoder_v2, int32_t count, void *user_data);
typedef void (*TF_RotaryEncoderV2_PressedHandler)(struct TF_RotaryEncoderV2 *rotary_encoder_v2, void *user_data);
typedef void (*TF_RotaryEncoderV2_ReleasedHandler)(struct TF_RotaryEncoderV2 *rotary_encoder_v2, void *user_data);

#endif
/**
 * \ingroup TF_RotaryEncoderV2
 *
 * 360° rotary encoder with push-button
 */
typedef struct TF_RotaryEncoderV2 {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_RotaryEncoderV2_CountHandler count_handler;
    void *count_user_data;

    TF_RotaryEncoderV2_PressedHandler pressed_handler;
    void *pressed_user_data;

    TF_RotaryEncoderV2_ReleasedHandler released_handler;
    void *released_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[1];
} TF_RotaryEncoderV2;

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_GET_COUNT 1

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_SET_COUNT_CALLBACK_CONFIGURATION 2

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_GET_COUNT_CALLBACK_CONFIGURATION 3

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_IS_PRESSED 5

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_RESET 243

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_READ_UID 249

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_CALLBACK_COUNT 4

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_CALLBACK_PRESSED 6

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_CALLBACK_RELEASED 7

#endif

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_THRESHOLD_OPTION_OFF 'x'

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_THRESHOLD_OPTION_OUTSIDE 'o'

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_THRESHOLD_OPTION_INSIDE 'i'

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_THRESHOLD_OPTION_SMALLER '<'

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_THRESHOLD_OPTION_GREATER '>'

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_RotaryEncoderV2
 */
#define TF_ROTARY_ENCODER_V2_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * This constant is used to identify a Rotary Encoder Bricklet 2.0.
 *
 * The {@link rotary_encoder_v2_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_ROTARY_ENCODER_V2_DEVICE_IDENTIFIER 294

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * This constant represents the display name of a Rotary Encoder Bricklet 2.0.
 */
#define TF_ROTARY_ENCODER_V2_DEVICE_DISPLAY_NAME "Rotary Encoder Bricklet 2.0"

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Creates the device object \c rotary_encoder_v2 with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_rotary_encoder_v2_create(TF_RotaryEncoderV2 *rotary_encoder_v2, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Removes the device object \c rotary_encoder_v2 from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_rotary_encoder_v2_destroy(TF_RotaryEncoderV2 *rotary_encoder_v2);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_rotary_encoder_v2_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_rotary_encoder_v2_get_response_expected(TF_RotaryEncoderV2 *rotary_encoder_v2, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_RotaryEncoderV2
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
int tf_rotary_encoder_v2_set_response_expected(TF_RotaryEncoderV2 *rotary_encoder_v2, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_rotary_encoder_v2_set_response_expected_all(TF_RotaryEncoderV2 *rotary_encoder_v2, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Registers the given \c handler to the Count callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(int32_t count, void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_rotary_encoder_v2_set_count_callback_configuration}.
 *
 * The parameter is the same as {@link tf_rotary_encoder_v2_get_count}.
 */
int tf_rotary_encoder_v2_register_count_callback(TF_RotaryEncoderV2 *rotary_encoder_v2, TF_RotaryEncoderV2_CountHandler handler, void *user_data);


/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Registers the given \c handler to the Pressed callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(void *user_data) \endcode
 *
 * This callback is triggered when the button is pressed.
 */
int tf_rotary_encoder_v2_register_pressed_callback(TF_RotaryEncoderV2 *rotary_encoder_v2, TF_RotaryEncoderV2_PressedHandler handler, void *user_data);


/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Registers the given \c handler to the Released callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(void *user_data) \endcode
 *
 * This callback is triggered when the button is released.
 */
int tf_rotary_encoder_v2_register_released_callback(TF_RotaryEncoderV2 *rotary_encoder_v2, TF_RotaryEncoderV2_ReleasedHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_rotary_encoder_v2_callback_tick(TF_RotaryEncoderV2 *rotary_encoder_v2, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Returns the current count of the encoder. If you set reset
 * to true, the count is set back to 0 directly after the
 * current count is read.
 *
 * The encoder has 24 steps per rotation.
 *
 * Turning the encoder to the left decrements the counter,
 * so a negative count is possible.
 *
 *
 * If you want to get the value periodically, it is recommended to use the
 * {@link tf_rotary_encoder_v2_register_count_callback} callback. You can set the callback configuration
 * with {@link tf_rotary_encoder_v2_set_count_callback_configuration}.
 */
int tf_rotary_encoder_v2_get_count(TF_RotaryEncoderV2 *rotary_encoder_v2, bool reset, int32_t *ret_count);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * The period is the period with which the {@link tf_rotary_encoder_v2_register_count_callback} callback is triggered
 * periodically. A value of 0 turns the callback off.
 *
 * If the `value has to change`-parameter is set to true, the callback is only
 * triggered after the value has changed. If the value didn't change
 * within the period, the callback is triggered immediately on change.
 *
 * If it is set to false, the callback is continuously triggered with the period,
 * independent of the value.
 *
 * It is furthermore possible to constrain the callback with thresholds.
 *
 * The `option`-parameter together with min/max sets a threshold for the {@link tf_rotary_encoder_v2_register_count_callback} callback.
 *
 * The following options are possible:
 *
 * \verbatim
 *  "Option", "Description"
 *
 *  "'x'",    "Threshold is turned off"
 *  "'o'",    "Threshold is triggered when the value is *outside* the min and max values"
 *  "'i'",    "Threshold is triggered when the value is *inside* or equal to the min and max values"
 *  "'<'",    "Threshold is triggered when the value is smaller than the min value (max is ignored)"
 *  "'>'",    "Threshold is triggered when the value is greater than the min value (max is ignored)"
 * \endverbatim
 *
 * If the option is set to 'x' (threshold turned off) the callback is triggered with the fixed period.
 */
int tf_rotary_encoder_v2_set_count_callback_configuration(TF_RotaryEncoderV2 *rotary_encoder_v2, uint32_t period, bool value_has_to_change, char option, int32_t min, int32_t max);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Returns the callback configuration as set by {@link tf_rotary_encoder_v2_set_count_callback_configuration}.
 */
int tf_rotary_encoder_v2_get_count_callback_configuration(TF_RotaryEncoderV2 *rotary_encoder_v2, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, int32_t *ret_min, int32_t *ret_max);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Returns *true* if the button is pressed and *false* otherwise.
 *
 * It is recommended to use the {@link tf_rotary_encoder_v2_register_pressed_callback} and {@link tf_rotary_encoder_v2_register_released_callback} callbacks
 * to handle the button.
 */
int tf_rotary_encoder_v2_is_pressed(TF_RotaryEncoderV2 *rotary_encoder_v2, bool *ret_pressed);

/**
 * \ingroup TF_RotaryEncoderV2
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
int tf_rotary_encoder_v2_get_spitfp_error_count(TF_RotaryEncoderV2 *rotary_encoder_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_RotaryEncoderV2
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
int tf_rotary_encoder_v2_set_bootloader_mode(TF_RotaryEncoderV2 *rotary_encoder_v2, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Returns the current bootloader mode, see {@link tf_rotary_encoder_v2_set_bootloader_mode}.
 */
int tf_rotary_encoder_v2_get_bootloader_mode(TF_RotaryEncoderV2 *rotary_encoder_v2, uint8_t *ret_mode);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Sets the firmware pointer for {@link tf_rotary_encoder_v2_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_rotary_encoder_v2_set_write_firmware_pointer(TF_RotaryEncoderV2 *rotary_encoder_v2, uint32_t pointer);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_rotary_encoder_v2_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_rotary_encoder_v2_write_firmware(TF_RotaryEncoderV2 *rotary_encoder_v2, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_rotary_encoder_v2_set_status_led_config(TF_RotaryEncoderV2 *rotary_encoder_v2, uint8_t config);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Returns the configuration as set by {@link tf_rotary_encoder_v2_set_status_led_config}
 */
int tf_rotary_encoder_v2_get_status_led_config(TF_RotaryEncoderV2 *rotary_encoder_v2, uint8_t *ret_config);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_rotary_encoder_v2_get_chip_temperature(TF_RotaryEncoderV2 *rotary_encoder_v2, int16_t *ret_temperature);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_rotary_encoder_v2_reset(TF_RotaryEncoderV2 *rotary_encoder_v2);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_rotary_encoder_v2_write_uid(TF_RotaryEncoderV2 *rotary_encoder_v2, uint32_t uid);

/**
 * \ingroup TF_RotaryEncoderV2
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_rotary_encoder_v2_read_uid(TF_RotaryEncoderV2 *rotary_encoder_v2, uint32_t *ret_uid);

/**
 * \ingroup TF_RotaryEncoderV2
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
int tf_rotary_encoder_v2_get_identity(TF_RotaryEncoderV2 *rotary_encoder_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
