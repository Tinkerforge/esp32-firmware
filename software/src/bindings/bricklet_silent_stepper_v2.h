/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_SILENT_STEPPER_V2_H
#define TF_SILENT_STEPPER_V2_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_SilentStepperV2 Silent Stepper Bricklet 2.0
 */

struct TF_SilentStepperV2;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_SilentStepperV2_UnderVoltageHandler)(struct TF_SilentStepperV2 *silent_stepper_v2, uint16_t voltage, void *user_data);
typedef void (*TF_SilentStepperV2_PositionReachedHandler)(struct TF_SilentStepperV2 *silent_stepper_v2, int32_t position, void *user_data);
typedef void (*TF_SilentStepperV2_AllDataHandler)(struct TF_SilentStepperV2 *silent_stepper_v2, uint16_t current_velocity, int32_t current_position, int32_t remaining_steps, uint16_t input_voltage, uint16_t current_consumption, void *user_data);
typedef void (*TF_SilentStepperV2_NewStateHandler)(struct TF_SilentStepperV2 *silent_stepper_v2, uint8_t state_new, uint8_t state_previous, void *user_data);
typedef void (*TF_SilentStepperV2_GPIOStateHandler)(struct TF_SilentStepperV2 *silent_stepper_v2, bool gpio_state[2], void *user_data);

#endif
/**
 * \ingroup TF_SilentStepperV2
 *
 * Silently drives one bipolar stepper motor with up to 46V and 1.6A per phase
 */
typedef struct TF_SilentStepperV2 {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_SilentStepperV2_UnderVoltageHandler under_voltage_handler;
    void *under_voltage_user_data;

    TF_SilentStepperV2_PositionReachedHandler position_reached_handler;
    void *position_reached_user_data;

    TF_SilentStepperV2_AllDataHandler all_data_handler;
    void *all_data_user_data;

    TF_SilentStepperV2_NewStateHandler new_state_handler;
    void *new_state_user_data;

    TF_SilentStepperV2_GPIOStateHandler gpio_state_handler;
    void *gpio_state_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[4];
} TF_SilentStepperV2;

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_MAX_VELOCITY 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_MAX_VELOCITY 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_CURRENT_VELOCITY 3

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_SPEED_RAMPING 4

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_SPEED_RAMPING 5

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_FULL_BRAKE 6

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_CURRENT_POSITION 7

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_CURRENT_POSITION 8

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_TARGET_POSITION 9

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_TARGET_POSITION 10

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_STEPS 11

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_STEPS 12

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_REMAINING_STEPS 13

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_STEP_CONFIGURATION 14

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_STEP_CONFIGURATION 15

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_FORWARD 16

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_BACKWARD 17

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_STOP 18

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_INPUT_VOLTAGE 19

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_MOTOR_CURRENT 22

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_MOTOR_CURRENT 23

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_ENABLED 24

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_ENABLED 25

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_BASIC_CONFIGURATION 26

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_BASIC_CONFIGURATION 27

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_SPREADCYCLE_CONFIGURATION 28

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_SPREADCYCLE_CONFIGURATION 29

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_STEALTH_CONFIGURATION 30

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_STEALTH_CONFIGURATION 31

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_COOLSTEP_CONFIGURATION 32

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_COOLSTEP_CONFIGURATION 33

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_MISC_CONFIGURATION 34

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_MISC_CONFIGURATION 35

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_ERROR_LED_CONFIG 36

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_ERROR_LED_CONFIG 37

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_DRIVER_STATUS 38

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_MINIMUM_VOLTAGE 39

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_MINIMUM_VOLTAGE 40

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_TIME_BASE 43

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_TIME_BASE 44

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_ALL_DATA 45

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_ALL_CALLBACK_CONFIGURATION 46

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_ALL_DATA_CALLBACK_CONFIGURATON 47

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_CONFIGURATION 48

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_GPIO_CONFIGURATION 49

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_ACTION 50

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_GPIO_ACTION 51

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_GPIO_STATE 52

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_RESET 243

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_READ_UID 249

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CALLBACK_UNDER_VOLTAGE 41

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CALLBACK_POSITION_REACHED 42

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CALLBACK_ALL_DATA 53

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CALLBACK_NEW_STATE 54

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CALLBACK_GPIO_STATE 55

#endif

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STEP_RESOLUTION_1 8

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STEP_RESOLUTION_2 7

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STEP_RESOLUTION_4 6

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STEP_RESOLUTION_8 5

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STEP_RESOLUTION_16 4

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STEP_RESOLUTION_32 3

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STEP_RESOLUTION_64 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STEP_RESOLUTION_128 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STEP_RESOLUTION_256 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CHOPPER_MODE_SPREAD_CYCLE 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CHOPPER_MODE_FAST_DECAY 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FREEWHEEL_MODE_NORMAL 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FREEWHEEL_MODE_FREEWHEELING 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FREEWHEEL_MODE_COIL_SHORT_LS 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_FREEWHEEL_MODE_COIL_SHORT_HS 3

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CURRENT_UP_STEP_INCREMENT_1 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CURRENT_UP_STEP_INCREMENT_2 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CURRENT_UP_STEP_INCREMENT_4 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CURRENT_UP_STEP_INCREMENT_8 3

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CURRENT_DOWN_STEP_DECREMENT_1 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CURRENT_DOWN_STEP_DECREMENT_2 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CURRENT_DOWN_STEP_DECREMENT_8 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_CURRENT_DOWN_STEP_DECREMENT_32 3

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_MINIMUM_CURRENT_HALF 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_MINIMUM_CURRENT_QUARTER 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STALLGUARD_MODE_STANDARD 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STALLGUARD_MODE_FILTERED 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_OPEN_LOAD_NONE 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_OPEN_LOAD_PHASE_A 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_OPEN_LOAD_PHASE_B 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_OPEN_LOAD_PHASE_AB 3

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_SHORT_TO_GROUND_NONE 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_SHORT_TO_GROUND_PHASE_A 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_SHORT_TO_GROUND_PHASE_B 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_SHORT_TO_GROUND_PHASE_AB 3

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_OVER_TEMPERATURE_NONE 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_OVER_TEMPERATURE_WARNING 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_OVER_TEMPERATURE_LIMIT 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STATE_STOP 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STATE_ACCELERATION 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STATE_RUN 3

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STATE_DEACCELERATION 4

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STATE_DIRECTION_CHANGE_TO_FORWARD 5

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STATE_DIRECTION_CHANGE_TO_BACKWARD 6

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_GPIO_ACTION_NONE 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_GPIO_ACTION_NORMAL_STOP_RISING_EDGE 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_GPIO_ACTION_NORMAL_STOP_FALLING_EDGE 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_GPIO_ACTION_FULL_BRAKE_RISING_EDGE 4

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_GPIO_ACTION_FULL_BRAKE_FALLING_EDGE 8

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_GPIO_ACTION_CALLBACK_RISING_EDGE 16

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_GPIO_ACTION_CALLBACK_FALLING_EDGE 32

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_ERROR_LED_CONFIG_OFF 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_ERROR_LED_CONFIG_ON 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_ERROR_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_ERROR_LED_CONFIG_SHOW_ERROR 3

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_SilentStepperV2
 */
#define TF_SILENT_STEPPER_V2_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_SilentStepperV2
 *
 * This constant is used to identify a Silent Stepper Bricklet 2.0.
 *
 * The {@link silent_stepper_v2_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_SILENT_STEPPER_V2_DEVICE_IDENTIFIER 2166

/**
 * \ingroup TF_SilentStepperV2
 *
 * This constant represents the display name of a Silent Stepper Bricklet 2.0.
 */
#define TF_SILENT_STEPPER_V2_DEVICE_DISPLAY_NAME "Silent Stepper Bricklet 2.0"

/**
 * \ingroup TF_SilentStepperV2
 *
 * Creates the device object \c silent_stepper_v2 with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_silent_stepper_v2_create(TF_SilentStepperV2 *silent_stepper_v2, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Removes the device object \c silent_stepper_v2 from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_silent_stepper_v2_destroy(TF_SilentStepperV2 *silent_stepper_v2);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_silent_stepper_v2_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_silent_stepper_v2_get_response_expected(TF_SilentStepperV2 *silent_stepper_v2, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_SilentStepperV2
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
int tf_silent_stepper_v2_set_response_expected(TF_SilentStepperV2 *silent_stepper_v2, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_silent_stepper_v2_set_response_expected_all(TF_SilentStepperV2 *silent_stepper_v2, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_SilentStepperV2
 *
 * Registers the given \c handler to the Under Voltage callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t voltage, void *user_data) \endcode
 *
 * This callback is triggered when the input voltage drops below the value set by
 * {@link tf_silent_stepper_v2_set_minimum_voltage}. The parameter is the current voltage.
 */
int tf_silent_stepper_v2_register_under_voltage_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2_UnderVoltageHandler handler, void *user_data);


/**
 * \ingroup TF_SilentStepperV2
 *
 * Registers the given \c handler to the Position Reached callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(int32_t position, void *user_data) \endcode
 *
 * This callback is triggered when a position set by {@link tf_silent_stepper_v2_set_steps} or
 * {@link tf_silent_stepper_v2_set_target_position} is reached.
 *
 * \note
 *  Since we can't get any feedback from the stepper motor, this only works if the
 *  acceleration (see {@link tf_silent_stepper_v2_set_speed_ramping}) is set smaller or equal to the
 *  maximum acceleration of the motor. Otherwise the motor will lag behind the
 *  control value and the callback will be triggered too early.
 */
int tf_silent_stepper_v2_register_position_reached_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2_PositionReachedHandler handler, void *user_data);


/**
 * \ingroup TF_SilentStepperV2
 *
 * Registers the given \c handler to the All Data callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t current_velocity, int32_t current_position, int32_t remaining_steps, uint16_t input_voltage, uint16_t current_consumption, void *user_data) \endcode
 *
 * This callback is triggered periodically with the period that is set by
 * {@link tf_silent_stepper_v2_set_all_callback_configuration}. The parameters are: the current velocity,
 * the current position, the remaining steps, the stack voltage, the external
 * voltage and the current consumption of the stepper motor.
 */
int tf_silent_stepper_v2_register_all_data_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2_AllDataHandler handler, void *user_data);


/**
 * \ingroup TF_SilentStepperV2
 *
 * Registers the given \c handler to the New State callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t state_new, uint8_t state_previous, void *user_data) \endcode
 *
 * This callback is triggered whenever the Silent Stepper Bricklet 2.0 enters a new state.
 * It returns the new state as well as the previous state.
 */
int tf_silent_stepper_v2_register_new_state_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2_NewStateHandler handler, void *user_data);


/**
 * \ingroup TF_SilentStepperV2
 *
 * Registers the given \c handler to the GPIO State callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(bool gpio_state[2], void *user_data) \endcode
 *
 * This callback is triggered by GPIO changes if it is activated through {@link tf_silent_stepper_v2_set_gpio_action}.
 */
int tf_silent_stepper_v2_register_gpio_state_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2_GPIOStateHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_SilentStepperV2
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_silent_stepper_v2_callback_tick(TF_SilentStepperV2 *silent_stepper_v2, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the maximum velocity of the stepper motor.
 * This function does *not* start the motor, it merely sets the maximum
 * velocity the stepper motor is accelerated to. To get the motor running use
 * either {@link tf_silent_stepper_v2_set_target_position}, {@link tf_silent_stepper_v2_set_steps}, {@link tf_silent_stepper_v2_drive_forward} or
 * {@link tf_silent_stepper_v2_drive_backward}.
 */
int tf_silent_stepper_v2_set_max_velocity(TF_SilentStepperV2 *silent_stepper_v2, uint16_t velocity);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the velocity as set by {@link tf_silent_stepper_v2_set_max_velocity}.
 */
int tf_silent_stepper_v2_get_max_velocity(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_velocity);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the *current* velocity of the stepper motor.
 */
int tf_silent_stepper_v2_get_current_velocity(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_velocity);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the acceleration and deacceleration of the stepper motor.
 * An acceleration of 1000 means, that
 * every second the velocity is increased by 1000 *steps/s*.
 *
 * For example: If the current velocity is 0 and you want to accelerate to a
 * velocity of 8000 *steps/s* in 10 seconds, you should set an acceleration
 * of 800 *steps/s²*.
 *
 * An acceleration/deacceleration of 0 means instantaneous
 * acceleration/deacceleration (not recommended)
 */
int tf_silent_stepper_v2_set_speed_ramping(TF_SilentStepperV2 *silent_stepper_v2, uint16_t acceleration, uint16_t deacceleration);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the acceleration and deacceleration as set by
 * {@link tf_silent_stepper_v2_set_speed_ramping}.
 */
int tf_silent_stepper_v2_get_speed_ramping(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_acceleration, uint16_t *ret_deacceleration);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Executes an active full brake.
 *
 * \warning
 *  This function is for emergency purposes,
 *  where an immediate brake is necessary. Depending on the current velocity and
 *  the strength of the motor, a full brake can be quite violent.
 *
 * Call {@link tf_silent_stepper_v2_stop} if you just want to stop the motor.
 */
int tf_silent_stepper_v2_full_brake(TF_SilentStepperV2 *silent_stepper_v2);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the current steps of the internal step counter. This can be used to
 * set the current position to 0 when some kind of starting position
 * is reached (e.g. when a CNC machine reaches a corner).
 */
int tf_silent_stepper_v2_set_current_position(TF_SilentStepperV2 *silent_stepper_v2, int32_t position);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the current position of the stepper motor in steps. On startup
 * the position is 0. The steps are counted with all possible driving
 * functions ({@link tf_silent_stepper_v2_set_target_position}, {@link tf_silent_stepper_v2_set_steps}, {@link tf_silent_stepper_v2_drive_forward} or
 * {@link tf_silent_stepper_v2_drive_backward}). It also is possible to reset the steps to 0 or
 * set them to any other desired value with {@link tf_silent_stepper_v2_set_current_position}.
 */
int tf_silent_stepper_v2_get_current_position(TF_SilentStepperV2 *silent_stepper_v2, int32_t *ret_position);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the target position of the stepper motor in steps. For example,
 * if the current position of the motor is 500 and {@link tf_silent_stepper_v2_set_target_position} is
 * called with 1000, the stepper motor will drive 500 steps forward. It will
 * use the velocity, acceleration and deacceleration as set by
 * {@link tf_silent_stepper_v2_set_max_velocity} and {@link tf_silent_stepper_v2_set_speed_ramping}.
 *
 * A call of {@link tf_silent_stepper_v2_set_target_position} with the parameter *x* is equivalent to
 * a call of {@link tf_silent_stepper_v2_set_steps} with the parameter
 * (*x* - {@link tf_silent_stepper_v2_get_current_position}).
 */
int tf_silent_stepper_v2_set_target_position(TF_SilentStepperV2 *silent_stepper_v2, int32_t position);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the last target position as set by {@link tf_silent_stepper_v2_set_target_position}.
 */
int tf_silent_stepper_v2_get_target_position(TF_SilentStepperV2 *silent_stepper_v2, int32_t *ret_position);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the number of steps the stepper motor should run. Positive values
 * will drive the motor forward and negative values backward.
 * The velocity, acceleration and deacceleration as set by
 * {@link tf_silent_stepper_v2_set_max_velocity} and {@link tf_silent_stepper_v2_set_speed_ramping} will be used.
 */
int tf_silent_stepper_v2_set_steps(TF_SilentStepperV2 *silent_stepper_v2, int32_t steps);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the last steps as set by {@link tf_silent_stepper_v2_set_steps}.
 */
int tf_silent_stepper_v2_get_steps(TF_SilentStepperV2 *silent_stepper_v2, int32_t *ret_steps);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the remaining steps of the last call of {@link tf_silent_stepper_v2_set_steps}.
 * For example, if {@link tf_silent_stepper_v2_set_steps} is called with 2000 and
 * {@link tf_silent_stepper_v2_get_remaining_steps} is called after the motor has run for 500 steps,
 * it will return 1500.
 */
int tf_silent_stepper_v2_get_remaining_steps(TF_SilentStepperV2 *silent_stepper_v2, int32_t *ret_steps);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the step resolution from full-step up to 1/256-step.
 *
 * If interpolation is turned on, the Silent Stepper Bricklet 2.0 will always interpolate
 * your step inputs as 1/256-step. If you use full-step mode with interpolation, each
 * step will generate 256 1/256 steps.
 *
 * For maximum torque use full-step without interpolation. For maximum resolution use
 * 1/256-step. Turn interpolation on to make the Stepper driving less noisy.
 *
 * If you often change the speed with high acceleration you should turn the
 * interpolation off.
 */
int tf_silent_stepper_v2_set_step_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t step_resolution, bool interpolation);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the step mode as set by {@link tf_silent_stepper_v2_set_step_configuration}.
 */
int tf_silent_stepper_v2_get_step_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_step_resolution, bool *ret_interpolation);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Drives the stepper motor forward until {@link tf_silent_stepper_v2_drive_backward} or
 * {@link tf_silent_stepper_v2_stop} is called. The velocity, acceleration and deacceleration as
 * set by {@link tf_silent_stepper_v2_set_max_velocity} and {@link tf_silent_stepper_v2_set_speed_ramping} will be used.
 */
int tf_silent_stepper_v2_drive_forward(TF_SilentStepperV2 *silent_stepper_v2);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Drives the stepper motor backward until {@link tf_silent_stepper_v2_drive_forward} or
 * {@link tf_silent_stepper_v2_stop} is triggered. The velocity, acceleration and deacceleration as
 * set by {@link tf_silent_stepper_v2_set_max_velocity} and {@link tf_silent_stepper_v2_set_speed_ramping} will be used.
 */
int tf_silent_stepper_v2_drive_backward(TF_SilentStepperV2 *silent_stepper_v2);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Stops the stepper motor with the deacceleration as set by
 * {@link tf_silent_stepper_v2_set_speed_ramping}.
 */
int tf_silent_stepper_v2_stop(TF_SilentStepperV2 *silent_stepper_v2);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the external input voltage. The external input voltage is
 * given via the black power input connector on the Silent Stepper Bricklet 2.0.
 *
 * If there is an external input voltage and a stack input voltage, the motor
 * will be driven by the external input voltage. If there is only a stack
 * voltage present, the motor will be driven by this voltage.
 *
 * \warning
 *  This means, if you have a high stack voltage and a low external voltage,
 *  the motor will be driven with the low external voltage. If you then remove
 *  the external connection, it will immediately be driven by the high
 *  stack voltage
 */
int tf_silent_stepper_v2_get_input_voltage(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_voltage);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the current with which the motor will be driven.
 *
 * \warning
 *  Do not set this value above the specifications of your stepper motor.
 *  Otherwise it may damage your motor.
 */
int tf_silent_stepper_v2_set_motor_current(TF_SilentStepperV2 *silent_stepper_v2, uint16_t current);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the current as set by {@link tf_silent_stepper_v2_set_motor_current}.
 */
int tf_silent_stepper_v2_get_motor_current(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_current);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Enables/Disables the driver chip. The driver parameters can be configured (maximum velocity,
 * acceleration, etc) before it is enabled.
 *
 * \warning
 *  Disabling the driver chip while the motor is still turning can damage the
 *  driver chip. The motor should be stopped calling {@link tf_silent_stepper_v2_stop} function
 *  before disabling the motor power. The {@link tf_silent_stepper_v2_stop} function will **not**
 *  wait until the motor is actually stopped. You have to explicitly wait for the
 *  appropriate time after calling the {@link tf_silent_stepper_v2_stop} function before calling
 *  the {@link tf_silent_stepper_v2_set_enabled} with false function.
 */
int tf_silent_stepper_v2_set_enabled(TF_SilentStepperV2 *silent_stepper_v2, bool enabled);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns *true* if the stepper driver is enabled, *false* otherwise.
 */
int tf_silent_stepper_v2_get_enabled(TF_SilentStepperV2 *silent_stepper_v2, bool *ret_enabled);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the basic configuration parameters for the different modes (Stealth, Coolstep, Classic).
 *
 * * Standstill Current: This value can be used to lower the current during stand still. This might
 *   be reasonable to reduce the heating of the motor and the Bricklet 2.0. When the motor is in standstill
 *   the configured motor phase current will be driven until the configured
 *   Power Down Time is elapsed. After that the phase current will be reduced to the standstill
 *   current. The elapsed time for this reduction can be configured with the Standstill Delay Time.
 *   The maximum allowed value is the configured maximum motor current
 *   (see {@link tf_silent_stepper_v2_set_motor_current}).
 *
 * * Motor Run Current: The value sets the motor current when the motor is running.
 *   Use a value of at least one half of the global maximum motor current for a good
 *   microstep performance. The maximum allowed value is the current
 *   motor current. The API maps the entered value to 1/32 ... 32/32 of the maximum
 *   motor current. This value should be used to change the motor current during motor movement,
 *   whereas the global maximum motor current should not be changed while the motor is moving
 *   (see {@link tf_silent_stepper_v2_set_motor_current}).
 *
 * * Standstill Delay Time: Controls the duration for motor power down after a motion
 *   as soon as standstill is detected and the Power Down Time is expired. A high Standstill Delay
 *   Time results in a smooth transition that avoids motor jerk during power down.
 *
 * * Power Down Time: Sets the delay time after a stand still.
 *
 * * Stealth Threshold: Sets the upper threshold for Stealth mode.
 *   If the velocity of the motor goes above this value, Stealth mode is turned
 *   off. Otherwise it is turned on. In Stealth mode the torque declines with high speed.
 *
 * * Coolstep Threshold: Sets the lower threshold for Coolstep mode.
 *   The Coolstep Threshold needs to be above the Stealth Threshold.
 *
 * * Classic Threshold: Sets the lower threshold for classic mode.
 *   In classic mode the stepper becomes more noisy, but the torque is maximized.
 *
 * * High Velocity Chopper Mode: If High Velocity Chopper Mode is enabled, the stepper control
 *   is optimized to run the stepper motors at high velocities.
 *
 * If you want to use all three thresholds make sure that
 * Stealth Threshold < Coolstep Threshold < Classic Threshold.
 */
int tf_silent_stepper_v2_set_basic_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint16_t standstill_current, uint16_t motor_run_current, uint16_t standstill_delay_time, uint16_t power_down_time, uint16_t stealth_threshold, uint16_t coolstep_threshold, uint16_t classic_threshold, bool high_velocity_chopper_mode);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the configuration as set by {@link tf_silent_stepper_v2_set_basic_configuration}.
 */
int tf_silent_stepper_v2_get_basic_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_standstill_current, uint16_t *ret_motor_run_current, uint16_t *ret_standstill_delay_time, uint16_t *ret_power_down_time, uint16_t *ret_stealth_threshold, uint16_t *ret_coolstep_threshold, uint16_t *ret_classic_threshold, bool *ret_high_velocity_chopper_mode);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Note: If you don't know what any of this means you can very likely keep all of
 * the values as default!
 *
 * Sets the Spreadcycle configuration parameters. Spreadcycle is a chopper algorithm which actively
 * controls the motor current flow. More information can be found in the TMC2130 datasheet on page
 * 47 (7 spreadCycle and Classic Chopper).
 *
 * * Slow Decay Duration: Controls duration of off time setting of slow decay phase.
 *   0 = driver disabled, all bridges off. Use 1 only with Comparator Blank time >= 2.
 *
 * * Enable Random Slow Decay: Set to false to fix chopper off time as set by Slow Decay Duration.
 *   If you set it to true, Decay Duration is randomly modulated.
 *
 * * Fast Decay Duration: Sets the fast decay duration. This parameters is
 *   only used if the Chopper Mode is set to Fast Decay.
 *
 * * Hysteresis Start Value: Sets the hysteresis start value. This parameter is
 *   only used if the Chopper Mode is set to Spread Cycle.
 *
 * * Hysteresis End Value: Sets the hysteresis end value. This parameter is
 *   only used if the Chopper Mode is set to Spread Cycle.
 *
 * * Sine Wave Offset: Sets the sine wave offset. This parameters is
 *   only used if the Chopper Mode is set to Fast Decay. 1/512 of the value becomes added to the absolute
 *   value of the sine wave.
 *
 * * Chopper Mode: 0 = Spread Cycle, 1 = Fast Decay.
 *
 * * Comparator Blank Time: Sets the blank time of the comparator. Available values are
 *
 *   * 0 = 16 clocks,
 *   * 1 = 24 clocks,
 *   * 2 = 36 clocks and
 *   * 3 = 54 clocks.
 *
 *   A value of 1 or 2 is recommended for most applications.
 *
 * * Fast Decay Without Comparator: If set to true the current comparator usage for termination of the
 *   fast decay cycle is disabled.
 */
int tf_silent_stepper_v2_set_spreadcycle_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t slow_decay_duration, bool enable_random_slow_decay, uint8_t fast_decay_duration, uint8_t hysteresis_start_value, int8_t hysteresis_end_value, int8_t sine_wave_offset, uint8_t chopper_mode, uint8_t comparator_blank_time, bool fast_decay_without_comparator);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the configuration as set by {@link tf_silent_stepper_v2_set_basic_configuration}.
 */
int tf_silent_stepper_v2_get_spreadcycle_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_slow_decay_duration, bool *ret_enable_random_slow_decay, uint8_t *ret_fast_decay_duration, uint8_t *ret_hysteresis_start_value, int8_t *ret_hysteresis_end_value, int8_t *ret_sine_wave_offset, uint8_t *ret_chopper_mode, uint8_t *ret_comparator_blank_time, bool *ret_fast_decay_without_comparator);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Note: If you don't know what any of this means you can very likely keep all of
 * the values as default!
 *
 * Sets the configuration relevant for Stealth mode.
 *
 * * Enable Stealth: If set to true the stealth mode is enabled, if set to false the
 *   stealth mode is disabled, even if the speed is below the threshold set in {@link tf_silent_stepper_v2_set_basic_configuration}.
 *
 * * Amplitude: If autoscale is disabled, the PWM amplitude is scaled by this value. If autoscale is enabled,
 *   this value defines the maximum PWM amplitude change per half wave.
 *
 * * Gradient: If autoscale is disabled, the PWM gradient is scaled by this value. If autoscale is enabled,
 *   this value defines the maximum PWM gradient. With autoscale a value above 64 is recommended,
 *   otherwise the regulation might not be able to measure the current.
 *
 * * Enable Autoscale: If set to true, automatic current control is used. Otherwise the user defined
 *   amplitude and gradient are used.
 *
 * * Force Symmetric: If true, A symmetric PWM cycle is enforced. Otherwise the PWM value may change within each
 *   PWM cycle.
 *
 * * Freewheel Mode: The freewheel mode defines the behavior in stand still if the Standstill Current
 *   (see {@link tf_silent_stepper_v2_set_basic_configuration}) is set to 0.
 */
int tf_silent_stepper_v2_set_stealth_configuration(TF_SilentStepperV2 *silent_stepper_v2, bool enable_stealth, uint8_t amplitude, uint8_t gradient, bool enable_autoscale, bool force_symmetric, uint8_t freewheel_mode);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the configuration as set by {@link tf_silent_stepper_v2_set_stealth_configuration}.
 */
int tf_silent_stepper_v2_get_stealth_configuration(TF_SilentStepperV2 *silent_stepper_v2, bool *ret_enable_stealth, uint8_t *ret_amplitude, uint8_t *ret_gradient, bool *ret_enable_autoscale, bool *ret_force_symmetric, uint8_t *ret_freewheel_mode);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Note: If you don't know what any of this means you can very likely keep all of
 * the values as default!
 *
 * Sets the configuration relevant for Coolstep.
 *
 * * Minimum Stallguard Value: If the Stallguard result falls below this value*32, the motor current
 *   is increased to reduce motor load angle. A value of 0 turns Coolstep off.
 *
 * * Maximum Stallguard Value: If the Stallguard result goes above
 *   (Min Stallguard Value + Max Stallguard Value + 1) * 32, the motor current is decreased to save
 *   energy.
 *
 * * Current Up Step Width: Sets the up step increment per Stallguard value. The value range is 0-3,
 *   corresponding to the increments 1, 2, 4 and 8.
 *
 * * Current Down Step Width: Sets the down step decrement per Stallguard value. The value range is 0-3,
 *   corresponding to the decrements 1, 2, 8 and 16.
 *
 * * Minimum Current: Sets the minimum current for Coolstep current control. You can choose between
 *   half and quarter of the run current.
 *
 * * Stallguard Threshold Value: Sets the level for stall output (see {@link tf_silent_stepper_v2_get_driver_status}).
 *   A lower value gives a higher sensitivity. You have to find a suitable value for your
 *   motor by trial and error, 0 works for most motors.
 *
 * * Stallguard Mode: Set to 0 for standard resolution or 1 for filtered mode. In filtered mode the Stallguard
 *   signal will be updated every four full-steps.
 */
int tf_silent_stepper_v2_set_coolstep_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t minimum_stallguard_value, uint8_t maximum_stallguard_value, uint8_t current_up_step_width, uint8_t current_down_step_width, uint8_t minimum_current, int8_t stallguard_threshold_value, uint8_t stallguard_mode);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the configuration as set by {@link tf_silent_stepper_v2_set_coolstep_configuration}.
 */
int tf_silent_stepper_v2_get_coolstep_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_minimum_stallguard_value, uint8_t *ret_maximum_stallguard_value, uint8_t *ret_current_up_step_width, uint8_t *ret_current_down_step_width, uint8_t *ret_minimum_current, int8_t *ret_stallguard_threshold_value, uint8_t *ret_stallguard_mode);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Note: If you don't know what any of this means you can very likely keep all of
 * the values as default!
 *
 * Sets miscellaneous configuration parameters.
 *
 * * Disable Short To Ground Protection: Set to false to enable short to ground protection, otherwise
 *   it is disabled.
 *
 * * Synchronize Phase Frequency: With this parameter you can synchronize the chopper for both phases
 *   of a two phase motor to avoid the occurrence of a beat. The value range is 0-15. If set to 0,
 *   the synchronization is turned off. Otherwise the synchronization is done through the formula
 *   f_sync = f_clk/(value*64). In Classic Mode the synchronization is automatically switched off.
 *   f_clk is 12.8MHz.
 */
int tf_silent_stepper_v2_set_misc_configuration(TF_SilentStepperV2 *silent_stepper_v2, bool disable_short_to_ground_protection, uint8_t synchronize_phase_frequency);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the configuration as set by {@link tf_silent_stepper_v2_set_misc_configuration}.
 */
int tf_silent_stepper_v2_get_misc_configuration(TF_SilentStepperV2 *silent_stepper_v2, bool *ret_disable_short_to_ground_protection, uint8_t *ret_synchronize_phase_frequency);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Configures the error LED to be either turned off, turned on, blink in
 * heartbeat mode or show an error.
 *
 * If the LED is configured to show errors it has three different states:
 *
 * * Off: No error present.
 * * 250ms interval blink: Overtemperature warning.
 * * 1s interval blink: Input voltage too small.
 * * full red: motor disabled because of short to ground in phase a or b or because of overtemperature.
 */
int tf_silent_stepper_v2_set_error_led_config(TF_SilentStepperV2 *silent_stepper_v2, uint8_t config);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the LED configuration as set by {@link tf_silent_stepper_v2_set_error_led_config}
 */
int tf_silent_stepper_v2_get_error_led_config(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_config);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the current driver status.
 *
 * * Open Load: Indicates if an open load is present on phase A, B or both. This could mean that there is a problem
 *   with the wiring of the motor. False detection can occur in fast motion as well as during stand still.
 *
 * * Short To Ground: Indicates if a short to ground is present on phase A, B or both. If this is detected the driver
 *   automatically becomes disabled and stays disabled until it is enabled again manually.
 *
 * * Over Temperature: The over temperature indicator switches to "Warning" if the driver IC warms up. The warning flag
 *   is expected during long duration stepper uses. If the temperature limit is reached the indicator switches
 *   to "Limit". In this case the driver becomes disabled until it cools down again.
 *
 * * Motor Stalled: Is true if a motor stall was detected.
 *
 * * Actual Motor Current: Indicates the actual current control scaling as used in Coolstep mode.
 *   It represents a multiplier of 1/32 to 32/32 of the
 *   ``Motor Run Current`` as set by {@link tf_silent_stepper_v2_set_basic_configuration}. Example: If a ``Motor Run Current``
 *   of 1000mA was set and the returned value is 15, the ``Actual Motor Current`` is 16/32*1000mA = 500mA.
 *
 * * Stallguard Result: Indicates the load of the motor. A lower value signals a higher load. Per trial and error
 *   you can find out which value corresponds to a suitable torque for the velocity used in your application.
 *   After that you can use this threshold value to find out if a motor stall becomes probable and react on it (e.g.
 *   decrease velocity).
 *   During stand still this value can not be used for stall detection, it shows the chopper on-time for motor coil A.
 *
 * * Stealth Voltage Amplitude: Shows the actual PWM scaling. In Stealth mode it can be used to detect motor load and
 *   stall if autoscale is enabled (see {@link tf_silent_stepper_v2_set_stealth_configuration}).
 */
int tf_silent_stepper_v2_get_driver_status(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_open_load, uint8_t *ret_short_to_ground, uint8_t *ret_over_temperature, bool *ret_motor_stalled, uint8_t *ret_actual_motor_current, bool *ret_full_step_active, uint8_t *ret_stallguard_result, uint8_t *ret_stealth_voltage_amplitude);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the minimum voltage, below which the {@link tf_silent_stepper_v2_register_under_voltage_callback} callback
 * is triggered. The minimum possible value that works with the Silent Stepper
 * Bricklet 2.0 is 8V.
 * You can use this function to detect the discharge of a battery that is used
 * to drive the stepper motor. If you have a fixed power supply, you likely do
 * not need this functionality.
 */
int tf_silent_stepper_v2_set_minimum_voltage(TF_SilentStepperV2 *silent_stepper_v2, uint16_t voltage);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the minimum voltage as set by {@link tf_silent_stepper_v2_set_minimum_voltage}.
 */
int tf_silent_stepper_v2_get_minimum_voltage(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_voltage);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the time base of the velocity and the acceleration of the Silent Stepper
 * Bricklet 2.0.
 *
 * For example, if you want to make one step every 1.5 seconds, you can set
 * the time base to 15 and the velocity to 10. Now the velocity is
 * 10steps/15s = 1steps/1.5s.
 */
int tf_silent_stepper_v2_set_time_base(TF_SilentStepperV2 *silent_stepper_v2, uint32_t time_base);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the time base as set by {@link tf_silent_stepper_v2_set_time_base}.
 */
int tf_silent_stepper_v2_get_time_base(TF_SilentStepperV2 *silent_stepper_v2, uint32_t *ret_time_base);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the following parameters: The current velocity,
 * the current position, the remaining steps, the stack voltage, the external
 * voltage and the current consumption of the stepper motor.
 *
 * The current consumption is calculated by multiplying the ``Actual Motor Current``
 * value (see {@link tf_silent_stepper_v2_set_basic_configuration}) with the ``Motor Run Current``
 * (see {@link tf_silent_stepper_v2_get_driver_status}). This is an internal calculation of the
 * driver, not an independent external measurement.
 *
 * The current consumption calculation was broken up to firmware 2.0.1, it is fixed
 * since firmware 2.0.2.
 *
 * There is also a callback for this function, see {@link tf_silent_stepper_v2_register_all_data_callback} callback.
 */
int tf_silent_stepper_v2_get_all_data(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_current_velocity, int32_t *ret_current_position, int32_t *ret_remaining_steps, uint16_t *ret_input_voltage, uint16_t *ret_current_consumption);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the period with which the {@link tf_silent_stepper_v2_register_all_data_callback} callback is triggered
 * periodically. A value of 0 turns the callback off.
 */
int tf_silent_stepper_v2_set_all_callback_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint32_t period);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the period as set by {@link tf_silent_stepper_v2_set_all_callback_configuration}.
 */
int tf_silent_stepper_v2_get_all_data_callback_configuraton(TF_SilentStepperV2 *silent_stepper_v2, uint32_t *ret_period);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the GPIO configuration for the given channel.
 * You can configure a debounce and the deceleration that is used if the action is
 * configured as ``normal stop``. See {@link tf_silent_stepper_v2_set_gpio_action}.
 */
int tf_silent_stepper_v2_set_gpio_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t channel, uint16_t debounce, uint16_t stop_deceleration);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the GPIO configuration for a channel as set by {@link tf_silent_stepper_v2_set_gpio_configuration}.
 */
int tf_silent_stepper_v2_get_gpio_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t channel, uint16_t *ret_debounce, uint16_t *ret_stop_deceleration);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the GPIO action for the given channel.
 *
 * The action can be a normal stop, a full brake or a callback. Each for a rising
 * edge or falling edge. The actions are a bitmask they can be used at the same time.
 * You can for example trigger a full brake and a callback at the same time or for
 * rising and falling edge.
 *
 * The deceleration speed for the normal stop can be configured with
 * {@link tf_silent_stepper_v2_set_gpio_configuration}.
 */
int tf_silent_stepper_v2_set_gpio_action(TF_SilentStepperV2 *silent_stepper_v2, uint8_t channel, uint32_t action);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the GPIO action for a channel as set by {@link tf_silent_stepper_v2_set_gpio_action}.
 */
int tf_silent_stepper_v2_get_gpio_action(TF_SilentStepperV2 *silent_stepper_v2, uint8_t channel, uint32_t *ret_action);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the GPIO state for both channels. True if the state is ``high`` and
 * false if the state is ``low``.
 */
int tf_silent_stepper_v2_get_gpio_state(TF_SilentStepperV2 *silent_stepper_v2, bool ret_gpio_state[2]);

/**
 * \ingroup TF_SilentStepperV2
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
int tf_silent_stepper_v2_get_spitfp_error_count(TF_SilentStepperV2 *silent_stepper_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_SilentStepperV2
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
int tf_silent_stepper_v2_set_bootloader_mode(TF_SilentStepperV2 *silent_stepper_v2, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the current bootloader mode, see {@link tf_silent_stepper_v2_set_bootloader_mode}.
 */
int tf_silent_stepper_v2_get_bootloader_mode(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_mode);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the firmware pointer for {@link tf_silent_stepper_v2_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_silent_stepper_v2_set_write_firmware_pointer(TF_SilentStepperV2 *silent_stepper_v2, uint32_t pointer);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_silent_stepper_v2_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_silent_stepper_v2_write_firmware(TF_SilentStepperV2 *silent_stepper_v2, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_silent_stepper_v2_set_status_led_config(TF_SilentStepperV2 *silent_stepper_v2, uint8_t config);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the configuration as set by {@link tf_silent_stepper_v2_set_status_led_config}
 */
int tf_silent_stepper_v2_get_status_led_config(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_config);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_silent_stepper_v2_get_chip_temperature(TF_SilentStepperV2 *silent_stepper_v2, int16_t *ret_temperature);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_silent_stepper_v2_reset(TF_SilentStepperV2 *silent_stepper_v2);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_silent_stepper_v2_write_uid(TF_SilentStepperV2 *silent_stepper_v2, uint32_t uid);

/**
 * \ingroup TF_SilentStepperV2
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_silent_stepper_v2_read_uid(TF_SilentStepperV2 *silent_stepper_v2, uint32_t *ret_uid);

/**
 * \ingroup TF_SilentStepperV2
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
int tf_silent_stepper_v2_get_identity(TF_SilentStepperV2 *silent_stepper_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
