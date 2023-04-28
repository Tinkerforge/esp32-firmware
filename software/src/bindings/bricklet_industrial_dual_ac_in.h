/* ***********************************************************
 * This file was automatically generated on 2023-04-25.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.3         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_INDUSTRIAL_DUAL_AC_IN_H
#define TF_INDUSTRIAL_DUAL_AC_IN_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_IndustrialDualACIn Industrial Dual AC In Bricklet
 */

struct TF_IndustrialDualACIn;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_IndustrialDualACIn_ValueHandler)(struct TF_IndustrialDualACIn *industrial_dual_ac_in, uint8_t channel, bool changed, bool value, void *user_data);
typedef void (*TF_IndustrialDualACIn_AllValueHandler)(struct TF_IndustrialDualACIn *industrial_dual_ac_in, bool changed[2], bool value[2], void *user_data);

#endif
/**
 * \ingroup TF_IndustrialDualACIn
 *
 * 2 inputs that can detect AC voltages of up to 230V
 */
typedef struct TF_IndustrialDualACIn {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_IndustrialDualACIn_ValueHandler value_handler;
    void *value_user_data;

    TF_IndustrialDualACIn_AllValueHandler all_value_handler;
    void *all_value_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[1];
} TF_IndustrialDualACIn;

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_GET_VALUE 1

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_SET_VALUE_CALLBACK_CONFIGURATION 2

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_GET_VALUE_CALLBACK_CONFIGURATION 3

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_SET_ALL_VALUE_CALLBACK_CONFIGURATION 4

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_GET_ALL_VALUE_CALLBACK_CONFIGURATION 5

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_SET_CHANNEL_LED_CONFIG 6

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_GET_CHANNEL_LED_CONFIG 7

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_RESET 243

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_READ_UID 249

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_CALLBACK_VALUE 8

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_CALLBACK_ALL_VALUE 9

#endif

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_CHANNEL_0 0

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_CHANNEL_1 1

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_CHANNEL_LED_CONFIG_OFF 0

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_CHANNEL_LED_CONFIG_ON 1

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_CHANNEL_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_CHANNEL_LED_CONFIG_SHOW_CHANNEL_STATUS 3

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_IndustrialDualACIn
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * This constant is used to identify a Industrial Dual AC In Bricklet.
 *
 * The {@link industrial_dual_ac_in_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_DEVICE_IDENTIFIER 2174

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * This constant represents the display name of a Industrial Dual AC In Bricklet.
 */
#define TF_INDUSTRIAL_DUAL_AC_IN_DEVICE_DISPLAY_NAME "Industrial Dual AC In Bricklet"

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Creates the device object \c industrial_dual_ac_in with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_industrial_dual_ac_in_create(TF_IndustrialDualACIn *industrial_dual_ac_in, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Removes the device object \c industrial_dual_ac_in from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_industrial_dual_ac_in_destroy(TF_IndustrialDualACIn *industrial_dual_ac_in);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_industrial_dual_ac_in_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_industrial_dual_ac_in_get_response_expected(TF_IndustrialDualACIn *industrial_dual_ac_in, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_IndustrialDualACIn
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
int tf_industrial_dual_ac_in_set_response_expected(TF_IndustrialDualACIn *industrial_dual_ac_in, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_industrial_dual_ac_in_set_response_expected_all(TF_IndustrialDualACIn *industrial_dual_ac_in, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Registers the given \c handler to the Value callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t channel, bool changed, bool value, void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_industrial_dual_ac_in_set_value_callback_configuration}.
 *
 * The parameters are the channel, a value-changed indicator and the actual
 * value for the channel. The `changed` parameter is true if the value has changed
 * since the last callback.
 */
int tf_industrial_dual_ac_in_register_value_callback(TF_IndustrialDualACIn *industrial_dual_ac_in, TF_IndustrialDualACIn_ValueHandler handler, void *user_data);


/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Registers the given \c handler to the All Value callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(bool changed[2], bool value[2], void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_industrial_dual_ac_in_set_all_value_callback_configuration}.
 *
 * The parameters are the same as {@link tf_industrial_dual_ac_in_get_value}. Additional the
 * `changed` parameter is true if the value has changed since
 * the last callback.
 */
int tf_industrial_dual_ac_in_register_all_value_callback(TF_IndustrialDualACIn *industrial_dual_ac_in, TF_IndustrialDualACIn_AllValueHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_industrial_dual_ac_in_callback_tick(TF_IndustrialDualACIn *industrial_dual_ac_in, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Returns the input values as bools, *true* refers to "AC voltage detected" and *false* refers to no AC "voltage detected".
 */
int tf_industrial_dual_ac_in_get_value(TF_IndustrialDualACIn *industrial_dual_ac_in, bool ret_value[2]);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * This callback can be configured per channel.
 *
 * The period is the period with which the {@link tf_industrial_dual_ac_in_register_value_callback}
 * callback is triggered periodically. A value of 0 turns the callback off.
 *
 * If the `value has to change`-parameter is set to true, the callback is only
 * triggered after the value has changed. If the value didn't change within the
 * period, the callback is triggered immediately on change.
 *
 * If it is set to false, the callback is continuously triggered with the period,
 * independent of the value.
 */
int tf_industrial_dual_ac_in_set_value_callback_configuration(TF_IndustrialDualACIn *industrial_dual_ac_in, uint8_t channel, uint32_t period, bool value_has_to_change);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Returns the callback configuration for the given channel as set by
 * {@link tf_industrial_dual_ac_in_set_value_callback_configuration}.
 */
int tf_industrial_dual_ac_in_get_value_callback_configuration(TF_IndustrialDualACIn *industrial_dual_ac_in, uint8_t channel, uint32_t *ret_period, bool *ret_value_has_to_change);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * The period is the period with which the {@link tf_industrial_dual_ac_in_register_all_value_callback}
 * callback is triggered periodically. A value of 0 turns the callback off.
 *
 * If the `value has to change`-parameter is set to true, the callback is only
 * triggered after the value has changed. If the value didn't change within the
 * period, the callback is triggered immediately on change.
 *
 * If it is set to false, the callback is continuously triggered with the period,
 * independent of the value.
 */
int tf_industrial_dual_ac_in_set_all_value_callback_configuration(TF_IndustrialDualACIn *industrial_dual_ac_in, uint32_t period, bool value_has_to_change);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Returns the callback configuration as set by
 * {@link tf_industrial_dual_ac_in_set_all_value_callback_configuration}.
 */
int tf_industrial_dual_ac_in_get_all_value_callback_configuration(TF_IndustrialDualACIn *industrial_dual_ac_in, uint32_t *ret_period, bool *ret_value_has_to_change);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Each channel has a corresponding LED. You can turn the LED off, on or show a
 * heartbeat. You can also set the LED to "Channel Status". In this mode the
 * LED is on if the channel is high and off otherwise.
 *
 * By default all channel LEDs are configured as "Channel Status".
 */
int tf_industrial_dual_ac_in_set_channel_led_config(TF_IndustrialDualACIn *industrial_dual_ac_in, uint8_t channel, uint8_t config);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Returns the channel LED configuration as set by {@link tf_industrial_dual_ac_in_set_channel_led_config}
 */
int tf_industrial_dual_ac_in_get_channel_led_config(TF_IndustrialDualACIn *industrial_dual_ac_in, uint8_t channel, uint8_t *ret_config);

/**
 * \ingroup TF_IndustrialDualACIn
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
int tf_industrial_dual_ac_in_get_spitfp_error_count(TF_IndustrialDualACIn *industrial_dual_ac_in, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_IndustrialDualACIn
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
int tf_industrial_dual_ac_in_set_bootloader_mode(TF_IndustrialDualACIn *industrial_dual_ac_in, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Returns the current bootloader mode, see {@link tf_industrial_dual_ac_in_set_bootloader_mode}.
 */
int tf_industrial_dual_ac_in_get_bootloader_mode(TF_IndustrialDualACIn *industrial_dual_ac_in, uint8_t *ret_mode);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Sets the firmware pointer for {@link tf_industrial_dual_ac_in_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_industrial_dual_ac_in_set_write_firmware_pointer(TF_IndustrialDualACIn *industrial_dual_ac_in, uint32_t pointer);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_industrial_dual_ac_in_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_industrial_dual_ac_in_write_firmware(TF_IndustrialDualACIn *industrial_dual_ac_in, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_industrial_dual_ac_in_set_status_led_config(TF_IndustrialDualACIn *industrial_dual_ac_in, uint8_t config);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Returns the configuration as set by {@link tf_industrial_dual_ac_in_set_status_led_config}
 */
int tf_industrial_dual_ac_in_get_status_led_config(TF_IndustrialDualACIn *industrial_dual_ac_in, uint8_t *ret_config);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_industrial_dual_ac_in_get_chip_temperature(TF_IndustrialDualACIn *industrial_dual_ac_in, int16_t *ret_temperature);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_industrial_dual_ac_in_reset(TF_IndustrialDualACIn *industrial_dual_ac_in);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_industrial_dual_ac_in_write_uid(TF_IndustrialDualACIn *industrial_dual_ac_in, uint32_t uid);

/**
 * \ingroup TF_IndustrialDualACIn
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_industrial_dual_ac_in_read_uid(TF_IndustrialDualACIn *industrial_dual_ac_in, uint32_t *ret_uid);

/**
 * \ingroup TF_IndustrialDualACIn
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
int tf_industrial_dual_ac_in_get_identity(TF_IndustrialDualACIn *industrial_dual_ac_in, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
