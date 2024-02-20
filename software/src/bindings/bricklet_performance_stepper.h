/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_PERFORMANCE_STEPPER_H
#define TF_PERFORMANCE_STEPPER_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_PerformanceStepper Performance Stepper Bricklet
 */

struct TF_PerformanceStepper;
#if TF_IMPLEMENT_CALLBACKS != 0


#endif
/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
typedef struct TF_PerformanceStepper {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0

#endif
    uint16_t magic;
    uint8_t response_expected[3];
} TF_PerformanceStepper;

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTION_CONFIGURATION 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_MOTION_CONFIGURATION 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_CURRENT_POSITION 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_CURRENT_POSITION 4

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_CURRENT_VELOCITY 5

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_TARGET_POSITION 6

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_TARGET_POSITION 7

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS 8

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_STEPS 9

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_REMAINING_STEPS 10

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEP_CONFIGURATION 11

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_STEP_CONFIGURATION 12

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTOR_CURRENT 22

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_MOTOR_CURRENT 23

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLED 24

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_ENABLED 25

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_BASIC_CONFIGURATION 26

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_BASIC_CONFIGURATION 27

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_SPREADCYCLE_CONFIGURATION 28

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_SPREADCYCLE_CONFIGURATION 29

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEALTH_CONFIGURATION 30

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_STEALTH_CONFIGURATION 31

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_COOLSTEP_CONFIGURATION 32

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_COOLSTEP_CONFIGURATION 33

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_SHORT_CONFIGURATION 34

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_SHORT_CONFIGURATION 35

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_DRIVER_STATUS 36

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_INPUT_VOLTAGE 37

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_TEMPERATURE 38

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_CONFIGURATION 39

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_GPIO_CONFIGURATION 40

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_ACTION 41

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_GPIO_ACTION 42

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_GPIO_STATE 43

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_ERROR_LED_CONFIG 44

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_ERROR_LED_CONFIG 45

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLE_LED_CONFIG 46

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_ENABLE_LED_CONFIG 47

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS_LED_CONFIG 48

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_STEPS_LED_CONFIG 49

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_LED_CONFIG 50

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_GPIO_LED_CONFIG 51

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_REGISTER 52

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_READ_REGISTER 53

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_RESET 243

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_READ_UID 249

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

#endif

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEP_RESOLUTION_1 8

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEP_RESOLUTION_2 7

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEP_RESOLUTION_4 6

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEP_RESOLUTION_8 5

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEP_RESOLUTION_16 4

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEP_RESOLUTION_32 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEP_RESOLUTION_64 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEP_RESOLUTION_128 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEP_RESOLUTION_256 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_RAMPING_MODE_POSITIONING 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_RAMPING_MODE_VELOCITY_NEGATIVE 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_RAMPING_MODE_VELOCITY_POSITIVE 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_RAMPING_MODE_HOLD 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_CHOPPER_MODE_SPREAD_CYCLE 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_CHOPPER_MODE_FAST_DECAY 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FREEWHEEL_MODE_NORMAL 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FREEWHEEL_MODE_FREEWHEELING 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FREEWHEEL_MODE_COIL_SHORT_LS 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FREEWHEEL_MODE_COIL_SHORT_HS 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_CURRENT_UP_STEP_INCREMENT_1 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_CURRENT_UP_STEP_INCREMENT_2 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_CURRENT_UP_STEP_INCREMENT_4 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_CURRENT_UP_STEP_INCREMENT_8 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_CURRENT_DOWN_STEP_DECREMENT_1 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_CURRENT_DOWN_STEP_DECREMENT_2 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_CURRENT_DOWN_STEP_DECREMENT_8 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_CURRENT_DOWN_STEP_DECREMENT_32 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_MINIMUM_CURRENT_HALF 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_MINIMUM_CURRENT_QUARTER 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STALLGUARD_MODE_STANDARD 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STALLGUARD_MODE_FILTERED 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_OPEN_LOAD_NONE 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_OPEN_LOAD_PHASE_A 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_OPEN_LOAD_PHASE_B 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_OPEN_LOAD_PHASE_AB 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_SHORT_TO_GROUND_NONE 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_SHORT_TO_GROUND_PHASE_A 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_SHORT_TO_GROUND_PHASE_B 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_SHORT_TO_GROUND_PHASE_AB 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_OVER_TEMPERATURE_NONE 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_OVER_TEMPERATURE_WARNING 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_OVER_TEMPERATURE_LIMIT 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STATE_STOP 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STATE_ACCELERATION 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STATE_RUN 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STATE_DEACCELERATION 4

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STATE_DIRECTION_CHANGE_TO_FORWARD 5

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STATE_DIRECTION_CHANGE_TO_BACKWARD 6

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_GPIO_ACTION_NONE 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_GPIO_ACTION_NORMAL_STOP_RISING_EDGE 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_GPIO_ACTION_NORMAL_STOP_FALLING_EDGE 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_GPIO_ACTION_EMERGENCY_STOP_RISING_EDGE 4

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_GPIO_ACTION_EMERGENCY_STOP_FALLING_EDGE 8

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_GPIO_ACTION_CALLBACK_RISING_EDGE 16

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_GPIO_ACTION_CALLBACK_FALLING_EDGE 32

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_ERROR_LED_CONFIG_OFF 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_ERROR_LED_CONFIG_ON 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_ERROR_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_ERROR_LED_CONFIG_SHOW_ERROR 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_ENABLE_LED_CONFIG_OFF 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_ENABLE_LED_CONFIG_ON 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_ENABLE_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_ENABLE_LED_CONFIG_SHOW_ENABLE 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEPS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEPS_LED_CONFIG_ON 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEPS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STEPS_LED_CONFIG_SHOW_STEPS 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_GPIO_LED_CONFIG_OFF 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_GPIO_LED_CONFIG_ON 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_GPIO_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_GPIO_LED_CONFIG_SHOW_GPIO_ACTIVE_HIGH 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_GPIO_LED_CONFIG_SHOW_GPIO_ACTIVE_LOW 4

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FILTER_TIME_100 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FILTER_TIME_200 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FILTER_TIME_300 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_FILTER_TIME_400 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_SPIKE_FILTER_BANDWIDTH_100 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_SPIKE_FILTER_BANDWIDTH_1000 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_SPIKE_FILTER_BANDWIDTH_2000 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_SPIKE_FILTER_BANDWIDTH_3000 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_PerformanceStepper
 */
#define TF_PERFORMANCE_STEPPER_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_PerformanceStepper
 *
 * This constant is used to identify a Performance Stepper Bricklet.
 *
 * The {@link performance_stepper_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_PERFORMANCE_STEPPER_DEVICE_IDENTIFIER 2158

/**
 * \ingroup TF_PerformanceStepper
 *
 * This constant represents the display name of a Performance Stepper Bricklet.
 */
#define TF_PERFORMANCE_STEPPER_DEVICE_DISPLAY_NAME "Performance Stepper Bricklet"

/**
 * \ingroup TF_PerformanceStepper
 *
 * Creates the device object \c performance_stepper with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_performance_stepper_create(TF_PerformanceStepper *performance_stepper, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Removes the device object \c performance_stepper from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_performance_stepper_destroy(TF_PerformanceStepper *performance_stepper);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_performance_stepper_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_performance_stepper_get_response_expected(TF_PerformanceStepper *performance_stepper, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_PerformanceStepper
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
int tf_performance_stepper_set_response_expected(TF_PerformanceStepper *performance_stepper, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_performance_stepper_set_response_expected_all(TF_PerformanceStepper *performance_stepper, bool response_expected);


#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_PerformanceStepper
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_performance_stepper_callback_tick(TF_PerformanceStepper *performance_stepper, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_set_motion_configuration(TF_PerformanceStepper *performance_stepper, uint8_t ramping_mode, int32_t velocity_start, int32_t acceleration_1, int32_t velocity_1, int32_t acceleration_max, int32_t velocity_max, int32_t deceleration_max, int32_t deceleration_1, int32_t velocity_stop, int32_t ramp_zero_wait);

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_get_motion_configuration(TF_PerformanceStepper *performance_stepper, uint8_t *ret_ramping_mode, int32_t *ret_velocity_start, int32_t *ret_acceleration_1, int32_t *ret_velocity_1, int32_t *ret_acceleration_max, int32_t *ret_velocity_max, int32_t *ret_deceleration_max, int32_t *ret_deceleration_1, int32_t *ret_velocity_stop, int32_t *ret_ramp_zero_wait);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Sets the current steps of the internal step counter. This can be used to
 * set the current position to 0 when some kind of starting position
 * is reached (e.g. when a CNC machine reaches a corner).
 */
int tf_performance_stepper_set_current_position(TF_PerformanceStepper *performance_stepper, int32_t position);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the current position of the stepper motor in steps. On startup
 * the position is 0. The steps are counted with all possible driving
 * functions ({@link tf_performance_stepper_set_target_position}, {@link tf_performance_stepper_set_steps}).
 * It also is possible to reset the steps to 0 or
 * set them to any other desired value with {@link tf_performance_stepper_set_current_position}.
 */
int tf_performance_stepper_get_current_position(TF_PerformanceStepper *performance_stepper, int32_t *ret_position);

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_get_current_velocity(TF_PerformanceStepper *performance_stepper, int32_t *ret_velocity);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Sets the target position of the stepper motor in steps. For example,
 * if the current position of the motor is 500 and {@link tf_performance_stepper_set_target_position} is
 * called with 1000, the stepper motor will drive 500 steps forward. It will
 * use the velocity, acceleration and deacceleration as set by
 * {@link tf_performance_stepper_set_motion_configuration}.
 *
 * A call of {@link tf_performance_stepper_set_target_position} with the parameter *x* is equivalent to
 * a call of {@link tf_performance_stepper_set_steps} with the parameter
 * (*x* - {@link tf_performance_stepper_get_current_position}).
 */
int tf_performance_stepper_set_target_position(TF_PerformanceStepper *performance_stepper, int32_t position);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the last target position as set by {@link tf_performance_stepper_set_target_position}.
 */
int tf_performance_stepper_get_target_position(TF_PerformanceStepper *performance_stepper, int32_t *ret_position);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Sets the number of steps the stepper motor should run. Positive values
 * will drive the motor forward and negative values backward.
 * The velocity, acceleration and deacceleration as set by
 * {@link tf_performance_stepper_set_motion_configuration} will be used.
 */
int tf_performance_stepper_set_steps(TF_PerformanceStepper *performance_stepper, int32_t steps);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the last steps as set by {@link tf_performance_stepper_set_steps}.
 */
int tf_performance_stepper_get_steps(TF_PerformanceStepper *performance_stepper, int32_t *ret_steps);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the remaining steps of the last call of {@link tf_performance_stepper_set_steps}.
 * For example, if {@link tf_performance_stepper_set_steps} is called with 2000 and
 * {@link tf_performance_stepper_get_remaining_steps} is called after the motor has run for 500 steps,
 * it will return 1500.
 */
int tf_performance_stepper_get_remaining_steps(TF_PerformanceStepper *performance_stepper, int32_t *ret_steps);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Sets the step resolution from full-step up to 1/256-step.
 *
 * If interpolation is turned on, the Silent Stepper Brick will always interpolate
 * your step inputs as 1/256-step. If you use full-step mode with interpolation, each
 * step will generate 256 1/256 steps.
 *
 * For maximum torque use full-step without interpolation. For maximum resolution use
 * 1/256-step. Turn interpolation on to make the Stepper driving less noisy.
 *
 * If you often change the speed with high acceleration you should turn the
 * interpolation off.
 */
int tf_performance_stepper_set_step_configuration(TF_PerformanceStepper *performance_stepper, uint8_t step_resolution, bool interpolation);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the step mode as set by {@link tf_performance_stepper_set_step_configuration}.
 */
int tf_performance_stepper_get_step_configuration(TF_PerformanceStepper *performance_stepper, uint8_t *ret_step_resolution, bool *ret_interpolation);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Sets the current with which the motor will be driven.
 *
 * \warning
 *  Do not set this value above the specifications of your stepper motor.
 *  Otherwise it may damage your motor.
 */
int tf_performance_stepper_set_motor_current(TF_PerformanceStepper *performance_stepper, uint16_t current);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the current as set by {@link tf_performance_stepper_set_motor_current}.
 */
int tf_performance_stepper_get_motor_current(TF_PerformanceStepper *performance_stepper, uint16_t *ret_current);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Enables the driver chip. The driver parameters can be configured (maximum velocity,
 * acceleration, etc) before it is enabled.
 */
int tf_performance_stepper_set_enabled(TF_PerformanceStepper *performance_stepper, bool enabled);

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_get_enabled(TF_PerformanceStepper *performance_stepper, bool *ret_enabled);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Sets the basic configuration parameters for the different modes (Stealth, Coolstep, Classic).
 *
 * * Standstill Current: This value can be used to lower the current during stand still. This might
 *   be reasonable to reduce the heating of the motor and the Brick. When the motor is in standstill
 *   the configured motor phase current will be driven until the configured
 *   Power Down Time is elapsed. After that the phase current will be reduced to the standstill
 *   current. The elapsed time for this reduction can be configured with the Standstill Delay Time.
 *   The maximum allowed value is the configured maximum motor current
 *   (see {@link tf_performance_stepper_set_motor_current}).
 *
 * * Motor Run Current: The value sets the motor current when the motor is running.
 *   Use a value of at least one half of the global maximum motor current for a good
 *   microstep performance. The maximum allowed value is the current
 *   motor current. The API maps the entered value to 1/32 ... 32/32 of the maximum
 *   motor current. This value should be used to change the motor current during motor movement,
 *   whereas the global maximum motor current should not be changed while the motor is moving
 *   (see {@link tf_performance_stepper_set_motor_current}).
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
int tf_performance_stepper_set_basic_configuration(TF_PerformanceStepper *performance_stepper, uint16_t standstill_current, uint16_t motor_run_current, uint16_t standstill_delay_time, uint16_t power_down_time, uint16_t stealth_threshold, uint16_t coolstep_threshold, uint16_t classic_threshold, bool high_velocity_chopper_mode);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the configuration as set by {@link tf_performance_stepper_set_basic_configuration}.
 */
int tf_performance_stepper_get_basic_configuration(TF_PerformanceStepper *performance_stepper, uint16_t *ret_standstill_current, uint16_t *ret_motor_run_current, uint16_t *ret_standstill_delay_time, uint16_t *ret_power_down_time, uint16_t *ret_stealth_threshold, uint16_t *ret_coolstep_threshold, uint16_t *ret_classic_threshold, bool *ret_high_velocity_chopper_mode);

/**
 * \ingroup TF_PerformanceStepper
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
 * * High Velocity Fullstep: TODO.
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
int tf_performance_stepper_set_spreadcycle_configuration(TF_PerformanceStepper *performance_stepper, uint8_t slow_decay_duration, bool high_velocity_fullstep, uint8_t fast_decay_duration, uint8_t hysteresis_start_value, int8_t hysteresis_end_value, int8_t sine_wave_offset, uint8_t chopper_mode, uint8_t comparator_blank_time, bool fast_decay_without_comparator);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the configuration as set by {@link tf_performance_stepper_set_basic_configuration}.
 */
int tf_performance_stepper_get_spreadcycle_configuration(TF_PerformanceStepper *performance_stepper, uint8_t *ret_slow_decay_duration, bool *ret_high_velocity_fullstep, bool *ret_enable_random_slow_decay, uint8_t *ret_fast_decay_duration, uint8_t *ret_hysteresis_start_value, int8_t *ret_hysteresis_end_value, int8_t *ret_sine_wave_offset, uint8_t *ret_chopper_mode, uint8_t *ret_comparator_blank_time, bool *ret_fast_decay_without_comparator);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Note: If you don't know what any of this means you can very likely keep all of
 * the values as default!
 *
 * Sets the configuration relevant for Stealth mode.
 *
 * * Enable Stealth: If set to true the stealth mode is enabled, if set to false the
 *   stealth mode is disabled, even if the speed is below the threshold set in {@link tf_performance_stepper_set_basic_configuration}.
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
 * * Enable Autogradient: TODO
 *
 * * Freewheel Mode: The freewheel mode defines the behavior in stand still if the Standstill Current
 *   (see {@link tf_performance_stepper_set_basic_configuration}) is set to 0.
 */
int tf_performance_stepper_set_stealth_configuration(TF_PerformanceStepper *performance_stepper, bool enable_stealth, uint8_t offset, uint8_t gradient, bool enable_autoscale, bool enable_autogradient, uint8_t freewheel_mode, uint8_t regulation_loop_gradient, uint8_t amplitude_limit);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the configuration as set by {@link tf_performance_stepper_set_stealth_configuration}.
 */
int tf_performance_stepper_get_stealth_configuration(TF_PerformanceStepper *performance_stepper, bool *ret_enable_stealth, uint8_t *ret_offset, uint8_t *ret_gradient, bool *ret_enable_autoscale, bool *ret_enable_autogradient, uint8_t *ret_freewheel_mode, uint8_t *ret_regulation_loop_gradient, uint8_t *ret_amplitude_limit);

/**
 * \ingroup TF_PerformanceStepper
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
 * * Stallguard Threshold Value: Sets the level for stall output (see {@link tf_performance_stepper_get_driver_status}).
 *   A lower value gives a higher sensitivity. You have to find a suitable value for your
 *   motor by trial and error, 0 works for most motors.
 *
 * * Stallguard Mode: Set to 0 for standard resolution or 1 for filtered mode. In filtered mode the Stallguard
 *   signal will be updated every four full-steps.
 */
int tf_performance_stepper_set_coolstep_configuration(TF_PerformanceStepper *performance_stepper, uint8_t minimum_stallguard_value, uint8_t maximum_stallguard_value, uint8_t current_up_step_width, uint8_t current_down_step_width, uint8_t minimum_current, int8_t stallguard_threshold_value, uint8_t stallguard_mode);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the configuration as set by {@link tf_performance_stepper_set_coolstep_configuration}.
 */
int tf_performance_stepper_get_coolstep_configuration(TF_PerformanceStepper *performance_stepper, uint8_t *ret_minimum_stallguard_value, uint8_t *ret_maximum_stallguard_value, uint8_t *ret_current_up_step_width, uint8_t *ret_current_down_step_width, uint8_t *ret_minimum_current, int8_t *ret_stallguard_threshold_value, uint8_t *ret_stallguard_mode);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Note: If you don't know what any of this means you can very likely keep all of
 * the values as default!
 *
 * Sets miscellaneous configuration parameters.
 *
 * * Disable Short To Ground Protection: Set to false to enable short to ground protection, otherwise
 *   it is disabled.
 *
 * * TODO
 */
int tf_performance_stepper_set_short_configuration(TF_PerformanceStepper *performance_stepper, bool disable_short_to_voltage_protection, bool disable_short_to_ground_protection, uint8_t short_to_voltage_level, uint8_t short_to_ground_level, uint8_t spike_filter_bandwidth, bool short_detection_delay, uint8_t filter_time);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the configuration as set by {@link tf_performance_stepper_set_short_configuration}.
 */
int tf_performance_stepper_get_short_configuration(TF_PerformanceStepper *performance_stepper, bool *ret_disable_short_to_voltage_protection, bool *ret_disable_short_to_ground_protection, uint8_t *ret_short_to_voltage_level, uint8_t *ret_short_to_ground_level, uint8_t *ret_spike_filter_bandwidth, bool *ret_short_detection_delay, uint8_t *ret_filter_time);

/**
 * \ingroup TF_PerformanceStepper
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
 *   ``Motor Run Current`` as set by {@link tf_performance_stepper_set_basic_configuration}. Example: If a ``Motor Run Current``
 *   of 1000mA was set and the returned value is 15, the ``Actual Motor Current`` is 16/32*1000mA = 500mA.
 *
 * * Stallguard Result: Indicates the load of the motor. A lower value signals a higher load. Per trial and error
 *   you can find out which value corresponds to a suitable torque for the velocity used in your application.
 *   After that you can use this threshold value to find out if a motor stall becomes probable and react on it (e.g.
 *   decrease velocity).
 *   During stand still this value can not be used for stall detection, it shows the chopper on-time for motor coil A.
 *
 * * Stealth Voltage Amplitude: Shows the actual PWM scaling. In Stealth mode it can be used to detect motor load and
 *   stall if autoscale is enabled (see {@link tf_performance_stepper_set_stealth_configuration}).
 */
int tf_performance_stepper_get_driver_status(TF_PerformanceStepper *performance_stepper, uint8_t *ret_open_load, uint8_t *ret_short_to_ground, uint8_t *ret_over_temperature, bool *ret_motor_stalled, uint8_t *ret_actual_motor_current, bool *ret_full_step_active, uint8_t *ret_stallguard_result, uint8_t *ret_stealth_voltage_amplitude);

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_get_input_voltage(TF_PerformanceStepper *performance_stepper, uint16_t *ret_voltage);

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_get_temperature(TF_PerformanceStepper *performance_stepper, int16_t *ret_temperature);

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_set_gpio_configuration(TF_PerformanceStepper *performance_stepper, uint16_t debounce, int32_t stop_deceleration);

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_get_gpio_configuration(TF_PerformanceStepper *performance_stepper, uint16_t *ret_debounce, int32_t *ret_stop_deceleration);

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_set_gpio_action(TF_PerformanceStepper *performance_stepper, uint8_t channel, uint32_t action);

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_get_gpio_action(TF_PerformanceStepper *performance_stepper, uint8_t channel, uint32_t *ret_action);

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_get_gpio_state(TF_PerformanceStepper *performance_stepper, bool ret_gpio_state[2]);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Configures the touch LED to be either turned off, turned on, blink in
 * heartbeat mode or show TBD.
 *
 * TODO:
 *
 * * one second interval blink: Input voltage too small
 * * 250ms interval blink: Overtemperature warning
 * * full red: motor disabled because of short to ground in phase a or b or because of overtemperature
 */
int tf_performance_stepper_set_error_led_config(TF_PerformanceStepper *performance_stepper, uint8_t config);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the LED configuration as set by {@link tf_performance_stepper_set_error_led_config}
 */
int tf_performance_stepper_get_error_led_config(TF_PerformanceStepper *performance_stepper, uint8_t *ret_config);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Configures the touch LED to be either turned off, turned on, blink in
 * heartbeat mode or show TBD.
 */
int tf_performance_stepper_set_enable_led_config(TF_PerformanceStepper *performance_stepper, uint8_t config);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the LED configuration as set by {@link tf_performance_stepper_set_enable_led_config}
 */
int tf_performance_stepper_get_enable_led_config(TF_PerformanceStepper *performance_stepper, uint8_t *ret_config);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Configures the touch LED to be either turned off, turned on, blink in
 * heartbeat mode or show TBD.
 */
int tf_performance_stepper_set_steps_led_config(TF_PerformanceStepper *performance_stepper, uint8_t config);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the LED configuration as set by {@link tf_performance_stepper_set_steps_led_config}
 */
int tf_performance_stepper_get_steps_led_config(TF_PerformanceStepper *performance_stepper, uint8_t *ret_config);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Configures the touch LED to be either turned off, turned on, blink in
 * heartbeat mode or show TBD.
 */
int tf_performance_stepper_set_gpio_led_config(TF_PerformanceStepper *performance_stepper, uint8_t channel, uint8_t config);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the LED configuration as set by {@link tf_performance_stepper_set_gpio_led_config}
 */
int tf_performance_stepper_get_gpio_led_config(TF_PerformanceStepper *performance_stepper, uint8_t channel, uint8_t *ret_config);

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_write_register(TF_PerformanceStepper *performance_stepper, uint8_t register_, uint32_t value, uint8_t *ret_status);

/**
 * \ingroup TF_PerformanceStepper
 *
 * TBD
 */
int tf_performance_stepper_read_register(TF_PerformanceStepper *performance_stepper, uint8_t register_, uint8_t *ret_status, uint32_t *ret_value);

/**
 * \ingroup TF_PerformanceStepper
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
int tf_performance_stepper_get_spitfp_error_count(TF_PerformanceStepper *performance_stepper, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_PerformanceStepper
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
int tf_performance_stepper_set_bootloader_mode(TF_PerformanceStepper *performance_stepper, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the current bootloader mode, see {@link tf_performance_stepper_set_bootloader_mode}.
 */
int tf_performance_stepper_get_bootloader_mode(TF_PerformanceStepper *performance_stepper, uint8_t *ret_mode);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Sets the firmware pointer for {@link tf_performance_stepper_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_performance_stepper_set_write_firmware_pointer(TF_PerformanceStepper *performance_stepper, uint32_t pointer);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_performance_stepper_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_performance_stepper_write_firmware(TF_PerformanceStepper *performance_stepper, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_performance_stepper_set_status_led_config(TF_PerformanceStepper *performance_stepper, uint8_t config);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the configuration as set by {@link tf_performance_stepper_set_status_led_config}
 */
int tf_performance_stepper_get_status_led_config(TF_PerformanceStepper *performance_stepper, uint8_t *ret_config);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_performance_stepper_get_chip_temperature(TF_PerformanceStepper *performance_stepper, int16_t *ret_temperature);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_performance_stepper_reset(TF_PerformanceStepper *performance_stepper);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_performance_stepper_write_uid(TF_PerformanceStepper *performance_stepper, uint32_t uid);

/**
 * \ingroup TF_PerformanceStepper
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_performance_stepper_read_uid(TF_PerformanceStepper *performance_stepper, uint32_t *ret_uid);

/**
 * \ingroup TF_PerformanceStepper
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
int tf_performance_stepper_get_identity(TF_PerformanceStepper *performance_stepper, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
