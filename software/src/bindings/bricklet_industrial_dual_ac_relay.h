/* ***********************************************************
 * This file was automatically generated on 2023-01-25.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.3         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_INDUSTRIAL_DUAL_AC_RELAY_H
#define TF_INDUSTRIAL_DUAL_AC_RELAY_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_IndustrialDualACRelay Industrial Dual AC Relay Bricklet
 */

struct TF_IndustrialDualACRelay;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_IndustrialDualACRelay_MonoflopDoneHandler)(struct TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint8_t channel, bool value, void *user_data);

#endif
/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Two relays to switch AC devices
 */
typedef struct TF_IndustrialDualACRelay {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_IndustrialDualACRelay_MonoflopDoneHandler monoflop_done_handler;
    void *monoflop_done_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[1];
} TF_IndustrialDualACRelay;

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_SET_VALUE 1

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_GET_VALUE 2

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_SET_CHANNEL_LED_CONFIG 3

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_GET_CHANNEL_LED_CONFIG 4

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_SET_MONOFLOP 5

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_GET_MONOFLOP 6

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_SET_SELECTED_VALUE 8

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_RESET 243

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_READ_UID 249

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_CALLBACK_MONOFLOP_DONE 7

#endif

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_CHANNEL_LED_CONFIG_OFF 0

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_CHANNEL_LED_CONFIG_ON 1

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_CHANNEL_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_CHANNEL_LED_CONFIG_SHOW_CHANNEL_STATUS 3

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_IndustrialDualACRelay
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * This constant is used to identify a Industrial Dual AC Relay Bricklet.
 *
 * The {@link industrial_dual_ac_relay_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_DEVICE_IDENTIFIER 2162

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * This constant represents the display name of a Industrial Dual AC Relay Bricklet.
 */
#define TF_INDUSTRIAL_DUAL_AC_RELAY_DEVICE_DISPLAY_NAME "Industrial Dual AC Relay Bricklet"

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Creates the device object \c industrial_dual_ac_relay with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_industrial_dual_ac_relay_create(TF_IndustrialDualACRelay *industrial_dual_ac_relay, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Removes the device object \c industrial_dual_ac_relay from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_industrial_dual_ac_relay_destroy(TF_IndustrialDualACRelay *industrial_dual_ac_relay);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_industrial_dual_ac_relay_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_industrial_dual_ac_relay_get_response_expected(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_IndustrialDualACRelay
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
int tf_industrial_dual_ac_relay_set_response_expected(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_industrial_dual_ac_relay_set_response_expected_all(TF_IndustrialDualACRelay *industrial_dual_ac_relay, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Registers the given \c handler to the Monoflop Done callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t channel, bool value, void *user_data) \endcode
 *
 * This callback is triggered whenever a monoflop timer reaches 0. The
 * parameters contain the relay and the current state of the relay
 * (the state after the monoflop).
 */
int tf_industrial_dual_ac_relay_register_monoflop_done_callback(TF_IndustrialDualACRelay *industrial_dual_ac_relay, TF_IndustrialDualACRelay_MonoflopDoneHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_industrial_dual_ac_relay_callback_tick(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Sets the state of the relays, *true* means on and *false* means off.
 * For example: (true, false) turns relay 0 on and relay 1 off.
 *
 * If you just want to set one of the relays and don't know the current state
 * of the other relay, you can get the state with {@link tf_industrial_dual_ac_relay_get_value} or you
 * can use {@link tf_industrial_dual_ac_relay_set_selected_value}.
 *
 * All running monoflop timers will be aborted if this function is called.
 */
int tf_industrial_dual_ac_relay_set_value(TF_IndustrialDualACRelay *industrial_dual_ac_relay, bool channel0, bool channel1);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Returns the state of the relays, *true* means on and *false* means off.
 */
int tf_industrial_dual_ac_relay_get_value(TF_IndustrialDualACRelay *industrial_dual_ac_relay, bool *ret_channel0, bool *ret_channel1);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Each channel has a corresponding LED. You can turn the LED off, on or show a
 * heartbeat. You can also set the LED to "Channel Status". In this mode the
 * LED is on if the channel is high and off otherwise.
 */
int tf_industrial_dual_ac_relay_set_channel_led_config(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint8_t channel, uint8_t config);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Returns the channel LED configuration as set by {@link tf_industrial_dual_ac_relay_set_channel_led_config}
 */
int tf_industrial_dual_ac_relay_get_channel_led_config(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint8_t channel, uint8_t *ret_config);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * The first parameter can be 0 or 1 (relay 0 or relay 1). The second parameter
 * is the desired state of the relay (*true* means on and *false* means off).
 * The third parameter indicates the time that the relay should hold
 * the state.
 *
 * If this function is called with the parameters (1, true, 1500):
 * Relay 1 will turn on and in 1.5s it will turn off again.
 *
 * A monoflop can be used as a failsafe mechanism. For example: Lets assume you
 * have a RS485 bus and a Industrial Dual AC Relay Bricklet connected to one of the
 * slave stacks. You can now call this function every second, with a time parameter
 * of two seconds. The relay will be on all the time. If now the RS485
 * connection is lost, the relay will turn off in at most two seconds.
 */
int tf_industrial_dual_ac_relay_set_monoflop(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint8_t channel, bool value, uint32_t time);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Returns (for the given relay) the current state and the time as set by
 * {@link tf_industrial_dual_ac_relay_set_monoflop} as well as the remaining time until the state flips.
 *
 * If the timer is not running currently, the remaining time will be returned
 * as 0.
 */
int tf_industrial_dual_ac_relay_get_monoflop(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint8_t channel, bool *ret_value, uint32_t *ret_time, uint32_t *ret_time_remaining);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Sets the state of the selected relay, *true* means on and *false*
 * means off.
 *
 * A running monoflop timer for the selected relay will be aborted if this function
 * is called.
 *
 * The other relay remains untouched.
 */
int tf_industrial_dual_ac_relay_set_selected_value(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint8_t channel, bool value);

/**
 * \ingroup TF_IndustrialDualACRelay
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
int tf_industrial_dual_ac_relay_get_spitfp_error_count(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_IndustrialDualACRelay
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
int tf_industrial_dual_ac_relay_set_bootloader_mode(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Returns the current bootloader mode, see {@link tf_industrial_dual_ac_relay_set_bootloader_mode}.
 */
int tf_industrial_dual_ac_relay_get_bootloader_mode(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint8_t *ret_mode);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Sets the firmware pointer for {@link tf_industrial_dual_ac_relay_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_industrial_dual_ac_relay_set_write_firmware_pointer(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint32_t pointer);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_industrial_dual_ac_relay_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_industrial_dual_ac_relay_write_firmware(TF_IndustrialDualACRelay *industrial_dual_ac_relay, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_industrial_dual_ac_relay_set_status_led_config(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint8_t config);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Returns the configuration as set by {@link tf_industrial_dual_ac_relay_set_status_led_config}
 */
int tf_industrial_dual_ac_relay_get_status_led_config(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint8_t *ret_config);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_industrial_dual_ac_relay_get_chip_temperature(TF_IndustrialDualACRelay *industrial_dual_ac_relay, int16_t *ret_temperature);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_industrial_dual_ac_relay_reset(TF_IndustrialDualACRelay *industrial_dual_ac_relay);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_industrial_dual_ac_relay_write_uid(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint32_t uid);

/**
 * \ingroup TF_IndustrialDualACRelay
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_industrial_dual_ac_relay_read_uid(TF_IndustrialDualACRelay *industrial_dual_ac_relay, uint32_t *ret_uid);

/**
 * \ingroup TF_IndustrialDualACRelay
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
int tf_industrial_dual_ac_relay_get_identity(TF_IndustrialDualACRelay *industrial_dual_ac_relay, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
