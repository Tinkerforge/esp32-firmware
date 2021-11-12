/* ***********************************************************
 * This file was automatically generated on 2021-11-08.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.0         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_BRICKLET_EVSE_V2_H
#define TF_BRICKLET_EVSE_V2_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup BrickletEVSEV2 EVSE Bricklet 2.0
 */

struct TF_EVSEV2;
#ifdef TF_IMPLEMENT_CALLBACKS


#endif
/**
 * \ingroup BrickletEVSEV2
 *
 * TBD
 */
typedef struct TF_EVSEV2 {
    TF_TfpContext *tfp;
#ifdef TF_IMPLEMENT_CALLBACKS

#endif
    uint8_t response_expected[2];
} TF_EVSEV2;

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_STATE 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_HARDWARE_CONFIGURATION 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_LOW_LEVEL_STATE 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_MAX_CHARGING_CURRENT 4

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_MAX_CHARGING_CURRENT 5

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_START_CHARGING 6

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_STOP_CHARGING 7

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_CHARGING_AUTOSTART 8

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_CHARGING_AUTOSTART 9

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ENERGY_METER_VALUES 10

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ENERGY_METER_DETAILED_VALUES_LOW_LEVEL 11

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ENERGY_METER_STATE 12

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_RESET_ENERGY_METER 13

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_DC_FAULT_CURRENT_STATE 14

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_RESET_DC_FAULT_CURRENT 15

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_GPIO_CONFIGURATION 16

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_GPIO_CONFIGURATION 17

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_MANAGED 18

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_MANAGED 19

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_MANAGED_CURRENT 20

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_DATA_STORAGE 21

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_DATA_STORAGE 22

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_INDICATOR_LED 23

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_INDICATOR_LED 24

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_BUTTON_CONFIGURATION 25

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_BUTTON_CONFIGURATION 26

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_BUTTON_STATE 27

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_CONTROL_PILOT_CONFIGURATION 28

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_CONTROL_PILOT_CONFIGURATION 29

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ALL_DATA_1 30

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ALL_DATA_2 31

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ALL_DATA_3 32

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_RESET 243

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_WRITE_UID 248

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_READ_UID 249

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_IDENTITY 255

#ifdef TF_IMPLEMENT_CALLBACKS

#endif

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_IEC61851_STATE_A 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_IEC61851_STATE_B 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_IEC61851_STATE_C 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_IEC61851_STATE_D 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_IEC61851_STATE_EF 4

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_LED_STATE_OFF 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_LED_STATE_ON 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_LED_STATE_BLINKING 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_LED_STATE_FLICKER 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_LED_STATE_BREATHING 4

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_VEHICLE_STATE_NOT_CONNECTED 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_VEHICLE_STATE_CONNECTED 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_VEHICLE_STATE_CHARGING 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_VEHICLE_STATE_ERROR 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_CONTACTOR_STATE_AC1_NLIVE_AC2_NLIVE 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_CONTACTOR_STATE_AC1_NLIVE_AC2_LIVE 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_LIVE 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_LOCK_STATE_INIT 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_LOCK_STATE_OPEN 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_LOCK_STATE_CLOSING 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_LOCK_STATE_CLOSE 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_LOCK_STATE_OPENING 4

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_LOCK_STATE_ERROR 5

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_ERROR_STATE_OK 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_ERROR_STATE_SWITCH 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_ERROR_STATE_DC_FAULT 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_ERROR_STATE_CONTACTOR 4

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_ERROR_STATE_COMMUNICATION 5

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_6A 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_10A 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_13A 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_16A 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_20A 4

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_25A 5

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_32A 6

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_SOFTWARE 7

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_UNCONFIGURED 8

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_CHARGE_RELEASE_AUTOMATIC 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_CHARGE_RELEASE_MANUAL 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_CHARGE_RELEASE_DEACTIVATED 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_CHARGE_RELEASE_MANAGED 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_DC_FAULT_CURRENT_STATE_NORMAL_CONDITION 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_DC_FAULT_CURRENT_STATE_6_MA_ERROR 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_DC_FAULT_CURRENT_STATE_SYSTEM_ERROR 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_DC_FAULT_CURRENT_STATE_UNKNOWN_ERROR 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_DC_FAULT_CURRENT_STATE_CALIBRATION_ERROR 4

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_SHUTDOWN_INPUT_IGNORED 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_SHUTDOWN_INPUT_SHUTDOWN_ON_OPEN 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_SHUTDOWN_INPUT_SHUTDOWN_ON_CLOSE 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_OUTPUT_LOW 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_OUTPUT_HIGH 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BUTTON_CONFIGURATION_DEACTIVATED 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BUTTON_CONFIGURATION_START_CHARGING 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BUTTON_CONFIGURATION_STOP_CHARGING 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BUTTON_CONFIGURATION_START_AND_STOP_CHARGING 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_CONTROL_PILOT_DISCONNECTED 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_CONTROL_PILOT_CONNECTED 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_CONTROL_PILOT_AUTOMATIC 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup BrickletEVSEV2
 */
#define TF_EVSE_V2_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup BrickletEVSEV2
 *
 * This constant is used to identify a EVSE Bricklet 2.0.
 *
 * The {@link evse_v2_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_EVSE_V2_DEVICE_IDENTIFIER 2167

/**
 * \ingroup BrickletEVSEV2
 *
 * This constant represents the display name of a EVSE Bricklet 2.0.
 */
#define TF_EVSE_V2_DEVICE_DISPLAY_NAME "EVSE Bricklet 2.0"

/**
 * \ingroup BrickletEVSEV2
 *
 * Creates the device object \c evse_v2 with the unique device ID \c uid and adds
 * it to the IPConnection \c ipcon.
 */
int tf_evse_v2_create(TF_EVSEV2 *evse_v2, const char *uid, TF_HalContext *hal);

/**
 * \ingroup BrickletEVSEV2
 *
 * Removes the device object \c evse_v2 from its IPConnection and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_evse_v2_destroy(TF_EVSEV2 *evse_v2);

/**
 * \ingroup BrickletEVSEV2
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the evse_v2_set_response_expected function. For setter functions it is
 * disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_evse_v2_get_response_expected(TF_EVSEV2 *evse_v2, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup BrickletEVSEV2
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
int tf_evse_v2_set_response_expected(TF_EVSEV2 *evse_v2, uint8_t function_id, bool response_expected);

/**
 * \ingroup BrickletEVSEV2
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
void tf_evse_v2_set_response_expected_all(TF_EVSEV2 *evse_v2, bool response_expected);


#ifdef TF_IMPLEMENT_CALLBACKS
/**
 * \ingroup BrickletEVSEV2
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_evse_v2_callback_tick(TF_EVSEV2 *evse_v2, uint32_t timeout_us);
#endif

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_state(TF_EVSEV2 *evse_v2, uint8_t *ret_iec61851_state, uint8_t *ret_vehicle_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint8_t *ret_charge_release, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint32_t *ret_time_since_state_change, uint32_t *ret_uptime);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_hardware_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_low_level_state(TF_EVSEV2 *evse_v2, uint8_t *ret_led_state, uint16_t *ret_cp_pwm_duty_cycle, uint16_t ret_adc_values[7], int16_t ret_voltages[7], uint32_t ret_resistances[2], bool ret_gpio[24], uint32_t *ret_charging_time);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_max_charging_current(TF_EVSEV2 *evse_v2, uint16_t max_current);

/**
 * \ingroup BrickletEVSEV2
 *
 * * Max Current Configured -> set with {@link tf_evse_v2_set_max_charging_current}
 * * Max Current Incoming Cable -> set with jumper on EVSE
 * * Max Current Outgoing Cable -> set with resistor between PP/PE (if fixed cable is used)
 * 
 * TODO
 */
int tf_evse_v2_get_max_charging_current(TF_EVSEV2 *evse_v2, uint16_t *ret_max_current_configured, uint16_t *ret_max_current_incoming_cable, uint16_t *ret_max_current_outgoing_cable, uint16_t *ret_max_current_managed);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_start_charging(TF_EVSEV2 *evse_v2);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_stop_charging(TF_EVSEV2 *evse_v2);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_charging_autostart(TF_EVSEV2 *evse_v2, bool autostart);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_charging_autostart(TF_EVSEV2 *evse_v2, bool *ret_autostart);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_energy_meter_values(TF_EVSEV2 *evse_v2, float *ret_power, float *ret_energy_relative, float *ret_energy_absolute, bool ret_phases_active[3], bool ret_phases_connected[3]);

/**
 * \ingroup BrickletEVSEV2
 *
 * TBD
 */
int tf_evse_v2_get_energy_meter_detailed_values_low_level(TF_EVSEV2 *evse_v2, uint16_t *ret_values_chunk_offset, float ret_values_chunk_data[15]);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_energy_meter_state(TF_EVSEV2 *evse_v2, bool *ret_available, uint32_t ret_error_count[6]);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_reset_energy_meter(TF_EVSEV2 *evse_v2);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_dc_fault_current_state(TF_EVSEV2 *evse_v2, uint8_t *ret_dc_fault_current_state);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_reset_dc_fault_current(TF_EVSEV2 *evse_v2, uint32_t password);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_gpio_configuration(TF_EVSEV2 *evse_v2, uint8_t shutdown_input_configuration, uint8_t input_configuration, uint8_t output_configuration);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_gpio_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_shutdown_input_configuration, uint8_t *ret_input_configuration, uint8_t *ret_output_configuration);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_managed(TF_EVSEV2 *evse_v2, bool *ret_managed);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_managed(TF_EVSEV2 *evse_v2, bool managed, uint32_t password);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_managed_current(TF_EVSEV2 *evse_v2, uint16_t current);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_data_storage(TF_EVSEV2 *evse_v2, uint8_t page, uint8_t ret_data[63]);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_data_storage(TF_EVSEV2 *evse_v2, uint8_t page, const uint8_t data[63]);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_indicator_led(TF_EVSEV2 *evse_v2, int16_t *ret_indication, uint16_t *ret_duration);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_indicator_led(TF_EVSEV2 *evse_v2, int16_t indication, uint16_t duration, uint8_t *ret_status);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_button_configuration(TF_EVSEV2 *evse_v2, uint8_t button_configuration);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_button_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_button_configuration);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_button_state(TF_EVSEV2 *evse_v2, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_control_pilot_configuration(TF_EVSEV2 *evse_v2, uint8_t control_pilot);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_control_pilot_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_control_pilot);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_all_data_1(TF_EVSEV2 *evse_v2, uint8_t *ret_iec61851_state, uint8_t *ret_vehicle_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint8_t *ret_charge_release, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint32_t *ret_time_since_state_change, uint32_t *ret_uptime, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_all_data_2(TF_EVSEV2 *evse_v2, uint8_t *ret_led_state, uint16_t *ret_cp_pwm_duty_cycle, uint16_t ret_adc_values[7], int16_t ret_voltages[7], uint32_t ret_resistances[2], bool ret_gpio[24], uint32_t *ret_charging_time, uint16_t *ret_max_current_configured, uint16_t *ret_max_current_incoming_cable, uint16_t *ret_max_current_outgoing_cable, uint16_t *ret_max_current_managed, bool *ret_autostart);

/**
 * \ingroup BrickletEVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_all_data_3(TF_EVSEV2 *evse_v2, float *ret_power, float *ret_energy_relative, float *ret_energy_absolute, bool ret_phases_active[3], bool ret_phases_connected[3], bool *ret_available, uint32_t ret_error_count[6], uint8_t *ret_dc_fault_current_state, uint8_t *ret_shutdown_input_configuration, uint8_t *ret_input_configuration, uint8_t *ret_output_configuration, bool *ret_managed, int16_t *ret_indication, uint16_t *ret_duration, uint8_t *ret_button_configuration, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed, uint8_t *ret_control_pilot);

/**
 * \ingroup BrickletEVSEV2
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
int tf_evse_v2_get_spitfp_error_count(TF_EVSEV2 *evse_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup BrickletEVSEV2
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
int tf_evse_v2_set_bootloader_mode(TF_EVSEV2 *evse_v2, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup BrickletEVSEV2
 *
 * Returns the current bootloader mode, see {@link tf_evse_v2_set_bootloader_mode}.
 */
int tf_evse_v2_get_bootloader_mode(TF_EVSEV2 *evse_v2, uint8_t *ret_mode);

/**
 * \ingroup BrickletEVSEV2
 *
 * Sets the firmware pointer for {@link tf_evse_v2_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 * 
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_evse_v2_set_write_firmware_pointer(TF_EVSEV2 *evse_v2, uint32_t pointer);

/**
 * \ingroup BrickletEVSEV2
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_evse_v2_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 * 
 * You can only write firmware in bootloader mode.
 * 
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_evse_v2_write_firmware(TF_EVSEV2 *evse_v2, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup BrickletEVSEV2
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 * 
 * You can also turn the LED permanently on/off or show a heartbeat.
 * 
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_evse_v2_set_status_led_config(TF_EVSEV2 *evse_v2, uint8_t config);

/**
 * \ingroup BrickletEVSEV2
 *
 * Returns the configuration as set by {@link tf_evse_v2_set_status_led_config}
 */
int tf_evse_v2_get_status_led_config(TF_EVSEV2 *evse_v2, uint8_t *ret_config);

/**
 * \ingroup BrickletEVSEV2
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 * 
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_evse_v2_get_chip_temperature(TF_EVSEV2 *evse_v2, int16_t *ret_temperature);

/**
 * \ingroup BrickletEVSEV2
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 * 
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_evse_v2_reset(TF_EVSEV2 *evse_v2);

/**
 * \ingroup BrickletEVSEV2
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 * 
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_evse_v2_write_uid(TF_EVSEV2 *evse_v2, uint32_t uid);

/**
 * \ingroup BrickletEVSEV2
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_evse_v2_read_uid(TF_EVSEV2 *evse_v2, uint32_t *ret_uid);

/**
 * \ingroup BrickletEVSEV2
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
int tf_evse_v2_get_identity(TF_EVSEV2 *evse_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

/**
 * \ingroup BrickletEVSEV2
 *
 * TBD
 */
int tf_evse_v2_get_energy_meter_detailed_values(TF_EVSEV2 *evse_v2, float *ret_values, uint16_t *ret_values_length);

#ifdef __cplusplus
}
#endif

#endif
