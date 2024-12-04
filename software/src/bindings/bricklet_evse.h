/* ***********************************************************
 * This file was automatically generated on 2024-12-04.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_EVSE_H
#define TF_EVSE_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_EVSE EVSE Bricklet
 */

struct TF_EVSE;
#if TF_IMPLEMENT_CALLBACKS != 0


#endif
/**
 * \ingroup TF_EVSE
 *
 * TBD
 */
typedef struct TF_EVSE {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0

#endif
    uint16_t magic;
    uint8_t response_expected[2];
} TF_EVSE;

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_STATE 1

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_HARDWARE_CONFIGURATION 2

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_LOW_LEVEL_STATE 3

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_SET_CHARGING_SLOT 4

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT 5

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_SET_CHARGING_SLOT_ACTIVE 6

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT 7

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_CHARGING_SLOT 8

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_ALL_CHARGING_SLOTS 9

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_SET_CHARGING_SLOT_DEFAULT 10

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_CHARGING_SLOT_DEFAULT 11

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_CALIBRATE 12

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_USER_CALIBRATION 13

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_SET_USER_CALIBRATION 14

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_DATA_STORAGE 15

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_SET_DATA_STORAGE 16

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_INDICATOR_LED 17

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_SET_INDICATOR_LED 18

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_BUTTON_STATE 19

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_ALL_DATA_1 20

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_FACTORY_RESET 21

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_SET_BOOST_MODE 22

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_BOOST_MODE 23

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_RESET 243

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_READ_UID 249

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

#endif

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_IEC61851_STATE_A 0

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_IEC61851_STATE_B 1

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_IEC61851_STATE_C 2

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_IEC61851_STATE_D 3

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_IEC61851_STATE_EF 4

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_LED_STATE_OFF 0

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_LED_STATE_ON 1

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_LED_STATE_BLINKING 2

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_LED_STATE_FLICKER 3

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_LED_STATE_BREATHING 4

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_CHARGER_STATE_NOT_CONNECTED 0

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_CHARGER_STATE_WAITING_FOR_CHARGE_RELEASE 1

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_CHARGER_STATE_READY_TO_CHARGE 2

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_CHARGER_STATE_CHARGING 3

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_CHARGER_STATE_ERROR 4

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_CONTACTOR_STATE_AC1_NLIVE_AC2_NLIVE 0

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE 1

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_CONTACTOR_STATE_AC1_NLIVE_AC2_LIVE 2

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_CONTACTOR_STATE_AC1_LIVE_AC2_LIVE 3

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_LOCK_STATE_INIT 0

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_LOCK_STATE_OPEN 1

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_LOCK_STATE_CLOSING 2

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_LOCK_STATE_CLOSE 3

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_LOCK_STATE_OPENING 4

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_LOCK_STATE_ERROR 5

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_ERROR_STATE_OK 0

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_ERROR_STATE_SWITCH 2

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_ERROR_STATE_CALIBRATION 3

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_ERROR_STATE_CONTACTOR 4

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_ERROR_STATE_COMMUNICATION 5

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_JUMPER_CONFIGURATION_6A 0

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_JUMPER_CONFIGURATION_10A 1

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_JUMPER_CONFIGURATION_13A 2

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_JUMPER_CONFIGURATION_16A 3

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_JUMPER_CONFIGURATION_20A 4

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_JUMPER_CONFIGURATION_25A 5

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_JUMPER_CONFIGURATION_32A 6

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_JUMPER_CONFIGURATION_SOFTWARE 7

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_JUMPER_CONFIGURATION_UNCONFIGURED 8

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_EVSE
 */
#define TF_EVSE_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_EVSE
 *
 * This constant is used to identify a EVSE Bricklet.
 *
 * The {@link evse_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_EVSE_DEVICE_IDENTIFIER 2159

/**
 * \ingroup TF_EVSE
 *
 * This constant represents the display name of a EVSE Bricklet.
 */
#define TF_EVSE_DEVICE_DISPLAY_NAME "EVSE Bricklet"

/**
 * \ingroup TF_EVSE
 *
 * Creates the device object \c evse with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_evse_create(TF_EVSE *evse, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_EVSE
 *
 * Removes the device object \c evse from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_evse_destroy(TF_EVSE *evse);

/**
 * \ingroup TF_EVSE
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_evse_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_evse_get_response_expected(TF_EVSE *evse, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_EVSE
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
int tf_evse_set_response_expected(TF_EVSE *evse, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_EVSE
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_evse_set_response_expected_all(TF_EVSE *evse, bool response_expected);


#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_EVSE
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_evse_callback_tick(TF_EVSE *evse, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_EVSE
 *
 * TODO
 *
 * .. versionadded:: 2.0.5$nbsp;(Plugin)
 */
int tf_evse_get_state(TF_EVSE *evse, uint8_t *ret_iec61851_state, uint8_t *ret_charger_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_get_hardware_configuration(TF_EVSE *evse, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch, uint8_t *ret_evse_version);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_get_low_level_state(TF_EVSE *evse, uint8_t *ret_led_state, uint16_t *ret_cp_pwm_duty_cycle, uint16_t ret_adc_values[2], int16_t ret_voltages[3], uint32_t ret_resistances[2], bool ret_gpio[5], bool *ret_car_stopped_charging, uint32_t *ret_time_since_state_change, uint32_t *ret_uptime);

/**
 * \ingroup TF_EVSE
 *
 * fixed slots:
 * 0: incoming cable (read-only, configured through slide switch)
 * 1: outgoing cable (read-only, configured through resistor)
 * 2: gpio input 0 (shutdown input)
 * 3: gpio input 1 (input)
 * 4: button (0A <-> 32A, can be controlled from web interface with start button and physical button if configured)
 *
 *
 * .. versionadded:: 2.1.0$nbsp;(Plugin)
 */
int tf_evse_set_charging_slot(TF_EVSE *evse, uint8_t slot, uint16_t max_current, bool active, bool clear_on_disconnect);

/**
 * \ingroup TF_EVSE
 *
 * .. versionadded:: 2.1.0$nbsp;(Plugin)
 */
int tf_evse_set_charging_slot_max_current(TF_EVSE *evse, uint8_t slot, uint16_t max_current);

/**
 * \ingroup TF_EVSE
 *
 * .. versionadded:: 2.1.0$nbsp;(Plugin)
 */
int tf_evse_set_charging_slot_active(TF_EVSE *evse, uint8_t slot, bool active);

/**
 * \ingroup TF_EVSE
 *
 * .. versionadded:: 2.1.0$nbsp;(Plugin)
 */
int tf_evse_set_charging_slot_clear_on_disconnect(TF_EVSE *evse, uint8_t slot, bool clear_on_disconnect);

/**
 * \ingroup TF_EVSE
 *
 * .. versionadded:: 2.1.0$nbsp;(Plugin)
 */
int tf_evse_get_charging_slot(TF_EVSE *evse, uint8_t slot, uint16_t *ret_max_current, bool *ret_active, bool *ret_clear_on_disconnect);

/**
 * \ingroup TF_EVSE
 *
 * packed getter
 *
 *
 * .. versionadded:: 2.1.0$nbsp;(Plugin)
 */
int tf_evse_get_all_charging_slots(TF_EVSE *evse, uint16_t ret_max_current[20], uint8_t ret_active_and_clear_on_disconnect[20]);

/**
 * \ingroup TF_EVSE
 *
 * fixed slots:
 * 0: incoming cable (read-only, configured through slide switch)
 * 1: outgoing cable (read-only, configured through resistor)
 * 2: gpio input 0 (shutdown input)
 * 3: gpio input 1 (input)
 *
 *
 * .. versionadded:: 2.1.0$nbsp;(Plugin)
 */
int tf_evse_set_charging_slot_default(TF_EVSE *evse, uint8_t slot, uint16_t max_current, bool active, bool clear_on_disconnect);

/**
 * \ingroup TF_EVSE
 *
 * .. versionadded:: 2.1.0$nbsp;(Plugin)
 */
int tf_evse_get_charging_slot_default(TF_EVSE *evse, uint8_t slot, uint16_t *ret_max_current, bool *ret_active, bool *ret_clear_on_disconnect);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_calibrate(TF_EVSE *evse, uint8_t state, uint32_t password, int32_t value, bool *ret_success);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_get_user_calibration(TF_EVSE *evse, bool *ret_user_calibration_active, int16_t *ret_voltage_diff, int16_t *ret_voltage_mul, int16_t *ret_voltage_div, int16_t *ret_resistance_2700, int16_t ret_resistance_880[14]);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_set_user_calibration(TF_EVSE *evse, uint32_t password, bool user_calibration_active, int16_t voltage_diff, int16_t voltage_mul, int16_t voltage_div, int16_t resistance_2700, const int16_t resistance_880[14]);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_get_data_storage(TF_EVSE *evse, uint8_t page, uint8_t ret_data[63]);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_set_data_storage(TF_EVSE *evse, uint8_t page, const uint8_t data[63]);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_get_indicator_led(TF_EVSE *evse, int16_t *ret_indication, uint16_t *ret_duration);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_set_indicator_led(TF_EVSE *evse, int16_t indication, uint16_t duration, uint8_t *ret_status);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_get_button_state(TF_EVSE *evse, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_get_all_data_1(TF_EVSE *evse, uint8_t *ret_iec61851_state, uint8_t *ret_charger_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch, uint8_t *ret_evse_version, uint8_t *ret_led_state, uint16_t *ret_cp_pwm_duty_cycle, uint16_t ret_adc_values[2], int16_t ret_voltages[3], uint32_t ret_resistances[2], bool ret_gpio[5], bool *ret_car_stopped_charging, uint32_t *ret_time_since_state_change, uint32_t *ret_uptime, int16_t *ret_indication, uint16_t *ret_duration, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed, bool *ret_boost_mode_enabled);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_factory_reset(TF_EVSE *evse, uint32_t password);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_set_boost_mode(TF_EVSE *evse, bool boost_mode_enabled);

/**
 * \ingroup TF_EVSE
 *
 * TODO
 */
int tf_evse_get_boost_mode(TF_EVSE *evse, bool *ret_boost_mode_enabled);

/**
 * \ingroup TF_EVSE
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
int tf_evse_get_spitfp_error_count(TF_EVSE *evse, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_EVSE
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
int tf_evse_set_bootloader_mode(TF_EVSE *evse, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_EVSE
 *
 * Returns the current bootloader mode, see {@link tf_evse_set_bootloader_mode}.
 */
int tf_evse_get_bootloader_mode(TF_EVSE *evse, uint8_t *ret_mode);

/**
 * \ingroup TF_EVSE
 *
 * Sets the firmware pointer for {@link tf_evse_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_evse_set_write_firmware_pointer(TF_EVSE *evse, uint32_t pointer);

/**
 * \ingroup TF_EVSE
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_evse_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_evse_write_firmware(TF_EVSE *evse, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_EVSE
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_evse_set_status_led_config(TF_EVSE *evse, uint8_t config);

/**
 * \ingroup TF_EVSE
 *
 * Returns the configuration as set by {@link tf_evse_set_status_led_config}
 */
int tf_evse_get_status_led_config(TF_EVSE *evse, uint8_t *ret_config);

/**
 * \ingroup TF_EVSE
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_evse_get_chip_temperature(TF_EVSE *evse, int16_t *ret_temperature);

/**
 * \ingroup TF_EVSE
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_evse_reset(TF_EVSE *evse);

/**
 * \ingroup TF_EVSE
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_evse_write_uid(TF_EVSE *evse, uint32_t uid);

/**
 * \ingroup TF_EVSE
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_evse_read_uid(TF_EVSE *evse, uint32_t *ret_uid);

/**
 * \ingroup TF_EVSE
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
int tf_evse_get_identity(TF_EVSE *evse, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
