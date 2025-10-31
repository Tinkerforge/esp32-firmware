/* ***********************************************************
 * This file was automatically generated on 2025-10-31.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_EVSE_V2_H
#define TF_EVSE_V2_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_EVSEV2 EVSE Bricklet 2.0
 */

struct TF_EVSEV2;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_EVSEV2_EnergyMeterValuesHandler)(struct TF_EVSEV2 *evse_v2, float power, float current[3], bool phases_active[3], bool phases_connected[3], void *user_data);
typedef void (*TF_EVSEV2_EichrechtDatasetLowLevelHandler)(struct TF_EVSEV2 *evse_v2, uint16_t message_length, uint16_t message_chunk_offset, char message_chunk_data[60], void *user_data);
typedef void (*TF_EVSEV2_EichrechtDatasetHandler)(struct TF_EVSEV2 *evse_v2, char *message, uint16_t message_length, void *user_data);
typedef void (*TF_EVSEV2_EichrechtSignatureLowLevelHandler)(struct TF_EVSEV2 *evse_v2, uint16_t message_length, uint16_t message_chunk_offset, char message_chunk_data[60], void *user_data);
typedef void (*TF_EVSEV2_EichrechtSignatureHandler)(struct TF_EVSEV2 *evse_v2, char *message, uint16_t message_length, void *user_data);

#endif
/**
 * \ingroup TF_EVSEV2
 *
 * TBD
 */
typedef struct TF_EVSEV2 {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_EVSEV2_EnergyMeterValuesHandler energy_meter_values_handler;
    void *energy_meter_values_user_data;

    TF_EVSEV2_EichrechtDatasetLowLevelHandler eichrecht_dataset_low_level_handler;
    void *eichrecht_dataset_low_level_user_data;

    TF_EVSEV2_EichrechtSignatureLowLevelHandler eichrecht_signature_low_level_handler;
    void *eichrecht_signature_low_level_user_data;

    TF_EVSEV2_EichrechtDatasetHandler eichrecht_dataset_handler;
    TF_HighLevelCallback eichrecht_dataset_hlc;

    TF_EVSEV2_EichrechtSignatureHandler eichrecht_signature_handler;
    TF_HighLevelCallback eichrecht_signature_hlc;

#endif
    uint16_t magic;
    uint8_t response_expected[4];
} TF_EVSEV2;

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_STATE 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_HARDWARE_CONFIGURATION 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_LOW_LEVEL_STATE 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT 5

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_ACTIVE 6

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT 7

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_CHARGING_SLOT 8

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ALL_CHARGING_SLOTS 9

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_DEFAULT 10

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_CHARGING_SLOT_DEFAULT 11

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ENERGY_METER_VALUES 12

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ALL_ENERGY_METER_VALUES_LOW_LEVEL 13

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ENERGY_METER_ERRORS 14

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY 15

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_RESET_DC_FAULT_CURRENT_STATE 16

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_GPIO_CONFIGURATION 17

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_GPIO_CONFIGURATION 18

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_DATA_STORAGE 19

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_DATA_STORAGE 20

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_INDICATOR_LED 21

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_INDICATOR_LED 22

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_BUTTON_CONFIGURATION 23

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_BUTTON_CONFIGURATION 24

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_BUTTON_STATE 25

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_EV_WAKEUP 26

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_EV_WAKUEP 27

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_CONTROL_PILOT_DISCONNECT 28

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_CONTROL_PILOT_DISCONNECT 29

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ALL_DATA_1 30

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ALL_DATA_2 31

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_FACTORY_RESET 32

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_BUTTON_PRESS_BOOT_TIME 33

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_BOOST_MODE 34

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_BOOST_MODE 35

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_TRIGGER_DC_FAULT_TEST 36

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_GP_OUTPUT 37

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_TEMPERATURE 38

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_PHASE_CONTROL 39

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_PHASE_CONTROL 40

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_PHASE_AUTO_SWITCH 41

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_PHASE_AUTO_SWITCH 42

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_PHASES_CONNECTED 43

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_PHASES_CONNECTED 44

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_CHARGING_PROTOCOL 46

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_CHARGING_PROTOCOL 47

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_EICHRECHT_GENERAL_INFORMATION 48

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_EICHRECHT_GENERAL_INFORMATION 49

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_EICHRECHT_USER_ASSIGNMENT 50

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_EICHRECHT_USER_ASSIGNMENT 51

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_EICHRECHT_CHARGE_POINT 52

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_EICHRECHT_CHARGE_POINT 53

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_EICHRECHT_TRANSACTION 54

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_EICHRECHT_TRANSACTION 55

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_EICHRECHT_PUBLIC_KEY 56

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_ENUMERATE_CONFIGURATION 59

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ENUMERATE_CONFIGURATION 60

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_ENUMERATE_VALUE 61

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_ENUMERATE_VALUE 62

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_CP_RECONNECT_TIME 63

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_CP_RECONNECT_TIME 64

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_RESET 243

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_READ_UID 249

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CALLBACK_ENERGY_METER_VALUES 45

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CALLBACK_EICHRECHT_DATASET_LOW_LEVEL 57

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CALLBACK_EICHRECHT_SIGNATURE_LOW_LEVEL 58

#endif

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_IEC61851_STATE_A 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_IEC61851_STATE_B 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_IEC61851_STATE_C 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_IEC61851_STATE_D 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_IEC61851_STATE_EF 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_LED_STATE_OFF 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_LED_STATE_ON 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_LED_STATE_BLINKING 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_LED_STATE_FLICKER 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_LED_STATE_BREATHING 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CHARGER_STATE_NOT_CONNECTED 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CHARGER_STATE_WAITING_FOR_CHARGE_RELEASE 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CHARGER_STATE_READY_TO_CHARGE 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CHARGER_STATE_CHARGING 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CHARGER_STATE_ERROR 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CONTACTOR_STATE_AC1_NLIVE_AC2_NLIVE 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CONTACTOR_STATE_AC1_NLIVE_AC2_LIVE 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_LIVE 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_LOCK_STATE_INIT 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_LOCK_STATE_OPEN 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_LOCK_STATE_CLOSING 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_LOCK_STATE_CLOSE 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_LOCK_STATE_OPENING 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_LOCK_STATE_ERROR 5

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ERROR_STATE_OK 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ERROR_STATE_SWITCH 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ERROR_STATE_DC_FAULT 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ERROR_STATE_CONTACTOR 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ERROR_STATE_COMMUNICATION 5

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_6A 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_10A 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_13A 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_16A 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_20A 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_25A 5

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_32A 6

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_SOFTWARE 7

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_JUMPER_CONFIGURATION_UNCONFIGURED 8

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_DC_FAULT_CURRENT_STATE_NORMAL_CONDITION 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_DC_FAULT_CURRENT_STATE_6_MA_DC_ERROR 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_DC_FAULT_CURRENT_STATE_SYSTEM_ERROR 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_DC_FAULT_CURRENT_STATE_UNKNOWN_ERROR 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_DC_FAULT_CURRENT_STATE_CALIBRATION_ERROR 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_DC_FAULT_CURRENT_STATE_20_MA_AC_ERROR 5

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_DC_FAULT_CURRENT_STATE_6_MA_AC_AND_20_MA_AC_ERROR 6

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_SHUTDOWN_INPUT_IGNORED 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_SHUTDOWN_INPUT_SHUTDOWN_ON_OPEN 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_SHUTDOWN_INPUT_SHUTDOWN_ON_CLOSE 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_SHUTDOWN_INPUT_4200_WATT_ON_OPEN 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_SHUTDOWN_INPUT_4200_WATT_ON_CLOSE 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_OUTPUT_CONNECTED_TO_GROUND 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_OUTPUT_HIGH_IMPEDANCE 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BUTTON_CONFIGURATION_DEACTIVATED 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BUTTON_CONFIGURATION_START_CHARGING 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BUTTON_CONFIGURATION_STOP_CHARGING 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BUTTON_CONFIGURATION_START_AND_STOP_CHARGING 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BUTTON_CONFIGURATION_ENUMERATE 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CONTROL_PILOT_DISCONNECTED 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CONTROL_PILOT_CONNECTED 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CONTROL_PILOT_AUTOMATIC 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ENERGY_METER_TYPE_NOT_AVAILABLE 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ENERGY_METER_TYPE_SDM72 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ENERGY_METER_TYPE_SDM630 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ENERGY_METER_TYPE_SDM72V2 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ENERGY_METER_TYPE_SDM72CTM 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ENERGY_METER_TYPE_SDM630MCTV2 5

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ENERGY_METER_TYPE_DSZ15DZMOD 6

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ENERGY_METER_TYPE_DEM4A 7

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ENERGY_METER_TYPE_DMED341MID7ER 8

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ENERGY_METER_TYPE_DSZ16DZE 9

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_ENERGY_METER_TYPE_WM3M4C 10

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_UNCONFIGURED 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_LOW_MAX_0A 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_LOW_MAX_6A 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_LOW_MAX_8A 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_LOW_MAX_10A 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_LOW_MAX_13A 5

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_LOW_MAX_16A 6

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_LOW_MAX_20A 7

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_LOW_MAX_25A 8

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_HIGH_MAX_0A 9

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_HIGH_MAX_6A 10

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_HIGH_MAX_8A 11

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_HIGH_MAX_10A 12

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_HIGH_MAX_13A 13

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_HIGH_MAX_16A 14

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_HIGH_MAX_20A 15

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_INPUT_ACTIVE_HIGH_MAX_25A 16

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CHARGING_PROTOCOL_IEC61851 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CHARGING_PROTOCOL_ISO15118 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_STATE_OK 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_STATE_NOT_ALL_INFO_SET 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_STATE_BUSY 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_STATE_NOT_SUPPORTED 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_RFID_NONE 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_RFID_PLAIN 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_RFID_RELATED 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_RFID_PSK 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_OCPP_NONE 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_OCPP_RS 5

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_OCPP_AUTH 6

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_OCPP_RS_TLS 7

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_OCPP_AUTH_TLS 8

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_OCPP_CACHE 9

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_OCPP_WHITELIST 10

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_OCPP_CERTIFIED 11

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_ISO15118_NONE 12

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_ISO15118_PNC 13

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_PLMN_NONE 14

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_PLMN_RING 15

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_PLMN_SMS 16

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_FLAG_NOT_SET 17

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_NONE 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_DENIED 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_UNDEFINED 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_ISO14443 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_ISO15693 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_EMAID 5

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_EVCCID 6

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_EVCOID 7

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_ISO7812 8

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_CARD_TXN_NR 9

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_CENTRAL 10

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_CENTRAL_1 11

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_CENTRAL_2 12

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_LOCAL 13

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_LOCAL_1 14

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_LOCAL_2 15

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_PHONE_NUMBER 16

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_USER_ASSIGNMENT_IDENTIFICATION_TYPE_KEY_CODE 17

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_CHARGE_POINT_IDENTIFICATION_TYPE_EVSEID 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_CHARGE_POINT_IDENTIFICATION_TYPE_CBIDC 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_NOT_INITIALISED 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_IDLE 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_SIGNATURE_IN_PROGRESS 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_SIGNATURE_OK 15

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_INVALID_DATE_TIME 128

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_CHECKSUM_ERROR 129

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_INVALID_COMMAND 130

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_INVALID_STATE 131

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_INVALID_MEASUREMENT 132

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_TEST_MODE_ERROR 133

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_VERIFY_STATE_ERROR 243

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_SIGNATURE_STATE_ERROR 244

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_KEYPAIR_GENERATION 245

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_SHA_FAILED 246

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_INIT_FAILED 247

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_DATA_NOT_LOCKED 248

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_CONFIG_NOT_LOCKED 249

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_VERIFY_ERROR 250

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_PUBLIC_KEY_ERROR 251

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_INVALID_MESSAGE_FORMAT 252

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_INVALID_MESSAGE_SIZE 253

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_SIGNATURE_ERROR 254

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_STATUS_UNDEFINED_ERROR 255

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_FORMAT_ASN1 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_SIGNATURE_FORMAT_BASE64 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_MEASUREMENT_STATUS_IDLE 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_MEASUREMENT_STATUS_ACTIVE 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_MEASUREMENT_STATUS_ACTIVE_AFTER_POWER_FAILURE 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_MEASUREMENT_STATUS_ACTIVE_AFTER_RESET 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_MEASUREMENT_STATUS_INVALID_DATE_TIME 128

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_TRANSACTION_COMMAND_BEGIN 'B'

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_TRANSACTION_COMMAND_END 'E'

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_TRANSACTION_COMMAND_INTERMEDIATE 'C'

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_TRANSACTION_COMMAND_EXCEPTION 'X'

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_TRANSACTION_COMMAND_TARIFF_CHANGE 'T'

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_TRANSACTION_COMMAND_SUSPENDED 'S'

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_TRANSACTION_COMMAND_END_WITH_BEGIN 'r'

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_TRANSACTION_COMMAND_FISCAL_READING 'f'

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_TRANSACTION_COMMAND_HOLD_COMMAND 'h'

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_EICHRECHT_TRANSACTION_COMMAND_LAST_CHARGE_READING 'i'

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_DEFAULT 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_15_SECONDS 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_20_SECONDS 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_25_SECONDS 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_30_SECONDS 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_35_SECONDS 5

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_40_SECONDS 6

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_45_SECONDS 7

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_50_SECONDS 8

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_55_SECONDS 9

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_60_SECONDS 10

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_65_SECONDS 11

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_70_SECONDS 12

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_75_SECONDS 13

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_80_SECONDS 14

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_85_SECONDS 15

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_90_SECONDS 16

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_95_SECONDS 17

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_100_SECONDS 18

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_105_SECONDS 19

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_110_SECONDS 20

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_115_SECONDS 21

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_CP_RECONNECT_TIME_120_SECONDS 22

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_EVSEV2
 */
#define TF_EVSE_V2_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_EVSEV2
 *
 * This constant is used to identify a EVSE Bricklet 2.0.
 *
 * The {@link evse_v2_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_EVSE_V2_DEVICE_IDENTIFIER 2167

/**
 * \ingroup TF_EVSEV2
 *
 * This constant represents the display name of a EVSE Bricklet 2.0.
 */
#define TF_EVSE_V2_DEVICE_DISPLAY_NAME "EVSE Bricklet 2.0"

/**
 * \ingroup TF_EVSEV2
 *
 * Creates the device object \c evse_v2 with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_evse_v2_create(TF_EVSEV2 *evse_v2, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_EVSEV2
 *
 * Removes the device object \c evse_v2 from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_evse_v2_destroy(TF_EVSEV2 *evse_v2);

/**
 * \ingroup TF_EVSEV2
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_evse_v2_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_evse_v2_get_response_expected(TF_EVSEV2 *evse_v2, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_EVSEV2
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
 * \ingroup TF_EVSEV2
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_evse_v2_set_response_expected_all(TF_EVSEV2 *evse_v2, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_EVSEV2
 *
 * Registers the given \c handler to the Energy Meter Values callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(float power, float current[3], bool phases_active[3], bool phases_connected[3], void *user_data) \endcode
 *
 * TODO
 */
int tf_evse_v2_register_energy_meter_values_callback(TF_EVSEV2 *evse_v2, TF_EVSEV2_EnergyMeterValuesHandler handler, void *user_data);


/**
 * \ingroup TF_EVSEV2
 *
 * Registers the given \c handler to the Eichrecht Dataset Low Level callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t message_length, uint16_t message_chunk_offset, char message_chunk_data[60], void *user_data) \endcode
 *
 * TODO
 */
int tf_evse_v2_register_eichrecht_dataset_low_level_callback(TF_EVSEV2 *evse_v2, TF_EVSEV2_EichrechtDatasetLowLevelHandler handler, void *user_data);


/**
 * \ingroup TF_EVSEV2
 *
 * Registers the given \c handler to the Eichrecht Dataset callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(char *message, uint16_t message_length, void *user_data) \endcode
 *
 * TODO
 */
int tf_evse_v2_register_eichrecht_dataset_callback(TF_EVSEV2 *evse_v2, TF_EVSEV2_EichrechtDatasetHandler handler, char *message, void *user_data);


/**
 * \ingroup TF_EVSEV2
 *
 * Registers the given \c handler to the Eichrecht Signature Low Level callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t message_length, uint16_t message_chunk_offset, char message_chunk_data[60], void *user_data) \endcode
 *
 * TODO
 */
int tf_evse_v2_register_eichrecht_signature_low_level_callback(TF_EVSEV2 *evse_v2, TF_EVSEV2_EichrechtSignatureLowLevelHandler handler, void *user_data);


/**
 * \ingroup TF_EVSEV2
 *
 * Registers the given \c handler to the Eichrecht Signature callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(char *message, uint16_t message_length, void *user_data) \endcode
 *
 * TODO
 */
int tf_evse_v2_register_eichrecht_signature_callback(TF_EVSEV2 *evse_v2, TF_EVSEV2_EichrechtSignatureHandler handler, char *message, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_EVSEV2
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_evse_v2_callback_tick(TF_EVSEV2 *evse_v2, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_state(TF_EVSEV2 *evse_v2, uint8_t *ret_iec61851_state, uint8_t *ret_charger_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint8_t *ret_dc_fault_current_state);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_hardware_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch, uint8_t *ret_evse_version, uint8_t *ret_energy_meter_type);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_low_level_state(TF_EVSEV2 *evse_v2, uint8_t *ret_led_state, uint16_t *ret_cp_pwm_duty_cycle, uint16_t ret_adc_values[7], int16_t ret_voltages[7], uint32_t ret_resistances[2], bool ret_gpio[24], bool *ret_car_stopped_charging, uint32_t *ret_time_since_state_change, uint32_t *ret_time_since_dc_fault_check, uint32_t *ret_uptime);

/**
 * \ingroup TF_EVSEV2
 *
 * fixed slots:
 * 0: incoming cable (read-only, configured through slide switch)
 * 1: outgoing cable (read-only, configured through resistor)
 * 2: gpio input 0 (shutdown input)
 * 3: gpio input 1 (input)
 * 4: button (0A <-> 32A, can be controlled from web interface with start button and physical button if configured)
 */
int tf_evse_v2_set_charging_slot(TF_EVSEV2 *evse_v2, uint8_t slot, uint16_t max_current, bool active, bool clear_on_disconnect);

/**
 * \ingroup TF_EVSEV2
 *
 *
 */
int tf_evse_v2_set_charging_slot_max_current(TF_EVSEV2 *evse_v2, uint8_t slot, uint16_t max_current);

/**
 * \ingroup TF_EVSEV2
 *
 *
 */
int tf_evse_v2_set_charging_slot_active(TF_EVSEV2 *evse_v2, uint8_t slot, bool active);

/**
 * \ingroup TF_EVSEV2
 *
 *
 */
int tf_evse_v2_set_charging_slot_clear_on_disconnect(TF_EVSEV2 *evse_v2, uint8_t slot, bool clear_on_disconnect);

/**
 * \ingroup TF_EVSEV2
 *
 *
 */
int tf_evse_v2_get_charging_slot(TF_EVSEV2 *evse_v2, uint8_t slot, uint16_t *ret_max_current, bool *ret_active, bool *ret_clear_on_disconnect);

/**
 * \ingroup TF_EVSEV2
 *
 * packed getter
 */
int tf_evse_v2_get_all_charging_slots(TF_EVSEV2 *evse_v2, uint16_t ret_max_current[20], uint8_t ret_active_and_clear_on_disconnect[20]);

/**
 * \ingroup TF_EVSEV2
 *
 * fixed slots:
 * 0: incoming cable (read-only, configured through slide switch)
 * 1: outgoing cable (read-only, configured through resistor)
 * 2: gpio input 0 (shutdown input)
 * 3: gpio input 1 (input)
 */
int tf_evse_v2_set_charging_slot_default(TF_EVSEV2 *evse_v2, uint8_t slot, uint16_t max_current, bool active, bool clear_on_disconnect);

/**
 * \ingroup TF_EVSEV2
 *
 *
 */
int tf_evse_v2_get_charging_slot_default(TF_EVSEV2 *evse_v2, uint8_t slot, uint16_t *ret_max_current, bool *ret_active, bool *ret_clear_on_disconnect);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_energy_meter_values(TF_EVSEV2 *evse_v2, float *ret_power, float ret_current[3], bool ret_phases_active[3], bool ret_phases_connected[3]);

/**
 * \ingroup TF_EVSEV2
 *
 * TBD
 */
int tf_evse_v2_get_all_energy_meter_values_low_level(TF_EVSEV2 *evse_v2, uint16_t *ret_values_length, uint16_t *ret_values_chunk_offset, float ret_values_chunk_data[15]);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_energy_meter_errors(TF_EVSEV2 *evse_v2, uint32_t ret_error_count[6]);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_reset_energy_meter_relative_energy(TF_EVSEV2 *evse_v2);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_reset_dc_fault_current_state(TF_EVSEV2 *evse_v2, uint32_t password);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_gpio_configuration(TF_EVSEV2 *evse_v2, uint8_t shutdown_input_configuration, uint8_t input_configuration, uint8_t output_configuration);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_gpio_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_shutdown_input_configuration, uint8_t *ret_input_configuration, uint8_t *ret_output_configuration);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_data_storage(TF_EVSEV2 *evse_v2, uint8_t page, uint8_t ret_data[63]);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_data_storage(TF_EVSEV2 *evse_v2, uint8_t page, const uint8_t data[63]);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_indicator_led(TF_EVSEV2 *evse_v2, int16_t *ret_indication, uint16_t *ret_duration, uint16_t *ret_color_h, uint8_t *ret_color_s, uint8_t *ret_color_v);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_indicator_led(TF_EVSEV2 *evse_v2, int16_t indication, uint16_t duration, uint16_t color_h, uint8_t color_s, uint8_t color_v, uint8_t *ret_status);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_button_configuration(TF_EVSEV2 *evse_v2, uint8_t button_configuration);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_button_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_button_configuration);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_button_state(TF_EVSEV2 *evse_v2, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_ev_wakeup(TF_EVSEV2 *evse_v2, bool ev_wakeup_enabled);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_ev_wakuep(TF_EVSEV2 *evse_v2, bool *ret_ev_wakeup_enabled);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_control_pilot_disconnect(TF_EVSEV2 *evse_v2, bool control_pilot_disconnect, bool *ret_is_control_pilot_disconnect);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_control_pilot_disconnect(TF_EVSEV2 *evse_v2, bool *ret_control_pilot_disconnect);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_all_data_1(TF_EVSEV2 *evse_v2, uint8_t *ret_iec61851_state, uint8_t *ret_charger_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint8_t *ret_dc_fault_current_state, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch, uint8_t *ret_evse_version, uint8_t *ret_energy_meter_type, float *ret_power, float ret_current[3], bool ret_phases_active[3], bool ret_phases_connected[3], uint32_t ret_error_count[6]);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_all_data_2(TF_EVSEV2 *evse_v2, uint8_t *ret_shutdown_input_configuration, uint8_t *ret_input_configuration, uint8_t *ret_output_configuration, int16_t *ret_indication, uint16_t *ret_duration, uint16_t *ret_color_h, uint8_t *ret_color_s, uint8_t *ret_color_v, uint8_t *ret_button_configuration, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed, bool *ret_ev_wakeup_enabled, bool *ret_control_pilot_disconnect, bool *ret_boost_mode_enabled, int16_t *ret_temperature, uint8_t *ret_phases_current, uint8_t *ret_phases_requested, uint8_t *ret_phases_state, uint8_t *ret_phases_info, bool *ret_phase_auto_switch_enabled, uint8_t *ret_phases_connected, uint8_t *ret_enumerate_value, uint32_t *ret_enumerate_value_change_time, uint8_t *ret_cp_reconnect_time);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_factory_reset(TF_EVSEV2 *evse_v2, uint32_t password);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_button_press_boot_time(TF_EVSEV2 *evse_v2, bool reset, uint32_t *ret_button_press_boot_time);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_boost_mode(TF_EVSEV2 *evse_v2, bool boost_mode_enabled);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_boost_mode(TF_EVSEV2 *evse_v2, bool *ret_boost_mode_enabled);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_trigger_dc_fault_test(TF_EVSEV2 *evse_v2, uint32_t password, bool *ret_started);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_gp_output(TF_EVSEV2 *evse_v2, uint8_t gp_output);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_temperature(TF_EVSEV2 *evse_v2, int16_t *ret_temperature);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_phase_control(TF_EVSEV2 *evse_v2, uint8_t phases);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_phase_control(TF_EVSEV2 *evse_v2, uint8_t *ret_phases_current, uint8_t *ret_phases_requested, uint8_t *ret_phases_state, uint8_t *ret_phases_info);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_phase_auto_switch(TF_EVSEV2 *evse_v2, bool phase_auto_switch_enabled);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_phase_auto_switch(TF_EVSEV2 *evse_v2, bool *ret_phase_auto_switch_enabled);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_phases_connected(TF_EVSEV2 *evse_v2, uint8_t phases_connected);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_phases_connected(TF_EVSEV2 *evse_v2, uint8_t *ret_phases_connected);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_charging_protocol(TF_EVSEV2 *evse_v2, uint8_t charging_protocol, uint16_t cp_duty_cycle);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_charging_protocol(TF_EVSEV2 *evse_v2, uint8_t *ret_charging_protocol, uint16_t *ret_cp_duty_cycle);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_eichrecht_general_information(TF_EVSEV2 *evse_v2, const char gateway_identification[32], const char gateway_serial[32], uint8_t *ret_eichrecht_state);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_eichrecht_general_information(TF_EVSEV2 *evse_v2, char ret_gateway_identification[32], char ret_gateway_serial[32]);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_eichrecht_user_assignment(TF_EVSEV2 *evse_v2, bool identification_status, const uint8_t identification_flags[4], uint8_t identification_type, const char identification_data[40], uint8_t *ret_eichrecht_state);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_eichrecht_user_assignment(TF_EVSEV2 *evse_v2, bool *ret_identification_status, uint8_t ret_identification_flags[4], uint8_t *ret_identification_type, char ret_identification_data[40]);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_eichrecht_charge_point(TF_EVSEV2 *evse_v2, uint8_t identification_type, const char identification[20], uint8_t *ret_eichrecht_state);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_eichrecht_charge_point(TF_EVSEV2 *evse_v2, uint8_t *ret_identification_type, char ret_identification[20]);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_eichrecht_transaction(TF_EVSEV2 *evse_v2, char transaction, uint32_t unix_time, int16_t utc_time_offset, uint16_t signature_format, uint8_t *ret_eichrecht_state);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_eichrecht_transaction(TF_EVSEV2 *evse_v2, char *ret_transaction, uint8_t *ret_transaction_state, uint8_t *ret_transaction_inner_state, uint16_t *ret_measurement_status, uint16_t *ret_signature_status, uint8_t *ret_eichrecht_state);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_eichrecht_public_key(TF_EVSEV2 *evse_v2, uint8_t ret_public_key[64]);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_enumerate_configuration(TF_EVSEV2 *evse_v2, const uint16_t enumerator_h[8], const uint8_t enumerator_s[8], const uint8_t enumerator_v[8]);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_enumerate_configuration(TF_EVSEV2 *evse_v2, uint16_t ret_enumerator_h[8], uint8_t ret_enumerator_s[8], uint8_t ret_enumerator_v[8]);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_enumerate_value(TF_EVSEV2 *evse_v2, uint8_t value);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_enumerate_value(TF_EVSEV2 *evse_v2, uint8_t *ret_value, uint32_t *ret_value_change_time);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_set_cp_reconnect_time(TF_EVSEV2 *evse_v2, uint8_t cp_reconnect_time);

/**
 * \ingroup TF_EVSEV2
 *
 * TODO
 */
int tf_evse_v2_get_cp_reconnect_time(TF_EVSEV2 *evse_v2, uint8_t *ret_cp_reconnect_time);

/**
 * \ingroup TF_EVSEV2
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
 * \ingroup TF_EVSEV2
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
 * \ingroup TF_EVSEV2
 *
 * Returns the current bootloader mode, see {@link tf_evse_v2_set_bootloader_mode}.
 */
int tf_evse_v2_get_bootloader_mode(TF_EVSEV2 *evse_v2, uint8_t *ret_mode);

/**
 * \ingroup TF_EVSEV2
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
 * \ingroup TF_EVSEV2
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
 * \ingroup TF_EVSEV2
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
 * \ingroup TF_EVSEV2
 *
 * Returns the configuration as set by {@link tf_evse_v2_set_status_led_config}
 */
int tf_evse_v2_get_status_led_config(TF_EVSEV2 *evse_v2, uint8_t *ret_config);

/**
 * \ingroup TF_EVSEV2
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
 * \ingroup TF_EVSEV2
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
 * \ingroup TF_EVSEV2
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_evse_v2_write_uid(TF_EVSEV2 *evse_v2, uint32_t uid);

/**
 * \ingroup TF_EVSEV2
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_evse_v2_read_uid(TF_EVSEV2 *evse_v2, uint32_t *ret_uid);

/**
 * \ingroup TF_EVSEV2
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
 * \ingroup TF_EVSEV2
 *
 * TBD
 */
int tf_evse_v2_get_all_energy_meter_values(TF_EVSEV2 *evse_v2, float *ret_values, uint16_t *ret_values_length);

#ifdef __cplusplus
}
#endif

#endif
