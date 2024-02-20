/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_DC_V2_H
#define TF_DC_V2_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_DCV2 DC Bricklet 2.0
 */

struct TF_DCV2;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_DCV2_EmergencyShutdownHandler)(struct TF_DCV2 *dc_v2, void *user_data);
typedef void (*TF_DCV2_VelocityReachedHandler)(struct TF_DCV2 *dc_v2, int16_t velocity, void *user_data);
typedef void (*TF_DCV2_CurrentVelocityHandler)(struct TF_DCV2 *dc_v2, int16_t velocity, void *user_data);

#endif
/**
 * \ingroup TF_DCV2
 *
 * Drives one brushed DC motor with up to 28V and 5A (peak)
 */
typedef struct TF_DCV2 {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_DCV2_EmergencyShutdownHandler emergency_shutdown_handler;
    void *emergency_shutdown_user_data;

    TF_DCV2_VelocityReachedHandler velocity_reached_handler;
    void *velocity_reached_user_data;

    TF_DCV2_CurrentVelocityHandler current_velocity_handler;
    void *current_velocity_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[2];
} TF_DCV2;

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_SET_ENABLED 1

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_ENABLED 2

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_SET_VELOCITY 3

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_VELOCITY 4

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_CURRENT_VELOCITY 5

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_SET_MOTION 6

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_MOTION 7

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_FULL_BRAKE 8

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_SET_DRIVE_MODE 9

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_DRIVE_MODE 10

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_SET_PWM_FREQUENCY 11

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_PWM_FREQUENCY 12

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_POWER_STATISTICS 13

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_SET_ERROR_LED_CONFIG 14

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_ERROR_LED_CONFIG 15

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_SET_EMERGENCY_SHUTDOWN_CALLBACK_CONFIGURATION 16

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_EMERGENCY_SHUTDOWN_CALLBACK_CONFIGURATION 17

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_SET_VELOCITY_REACHED_CALLBACK_CONFIGURATION 18

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_VELOCITY_REACHED_CALLBACK_CONFIGURATION 19

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_SET_CURRENT_VELOCITY_CALLBACK_CONFIGURATION 20

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_CURRENT_VELOCITY_CALLBACK_CONFIGURATION 21

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_RESET 243

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_READ_UID 249

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_CALLBACK_EMERGENCY_SHUTDOWN 22

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_CALLBACK_VELOCITY_REACHED 23

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_CALLBACK_CURRENT_VELOCITY 24

#endif

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_DRIVE_MODE_DRIVE_BRAKE 0

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_DRIVE_MODE_DRIVE_COAST 1

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_ERROR_LED_CONFIG_OFF 0

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_ERROR_LED_CONFIG_ON 1

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_ERROR_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_ERROR_LED_CONFIG_SHOW_ERROR 3

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_DCV2
 */
#define TF_DC_V2_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_DCV2
 *
 * This constant is used to identify a DC Bricklet 2.0.
 *
 * The {@link dc_v2_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_DC_V2_DEVICE_IDENTIFIER 2165

/**
 * \ingroup TF_DCV2
 *
 * This constant represents the display name of a DC Bricklet 2.0.
 */
#define TF_DC_V2_DEVICE_DISPLAY_NAME "DC Bricklet 2.0"

/**
 * \ingroup TF_DCV2
 *
 * Creates the device object \c dc_v2 with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_dc_v2_create(TF_DCV2 *dc_v2, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_DCV2
 *
 * Removes the device object \c dc_v2 from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_dc_v2_destroy(TF_DCV2 *dc_v2);

/**
 * \ingroup TF_DCV2
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_dc_v2_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_dc_v2_get_response_expected(TF_DCV2 *dc_v2, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_DCV2
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
int tf_dc_v2_set_response_expected(TF_DCV2 *dc_v2, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_DCV2
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_dc_v2_set_response_expected_all(TF_DCV2 *dc_v2, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_DCV2
 *
 * Registers the given \c handler to the Emergency Shutdown callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(void *user_data) \endcode
 *
 * This callback is triggered if either the current consumption
 * is too high (above 5A) or the temperature of the driver chip is too high
 * (above 175°C). These two possibilities are essentially the same, since the
 * temperature will reach this threshold immediately if the motor consumes too
 * much current. In case of a voltage below 3.3V (external or stack) this
 * callback is triggered as well.
 *
 * If this callback is triggered, the driver chip gets disabled at the same time.
 * That means, {@link tf_dc_v2_set_enabled} has to be called to drive the motor again.
 *
 * \note
 *  This callback only works in Drive/Brake mode (see {@link tf_dc_v2_set_drive_mode}). In
 *  Drive/Coast mode it is unfortunately impossible to reliably read the
 *  overcurrent/overtemperature signal from the driver chip.
 */
int tf_dc_v2_register_emergency_shutdown_callback(TF_DCV2 *dc_v2, TF_DCV2_EmergencyShutdownHandler handler, void *user_data);


/**
 * \ingroup TF_DCV2
 *
 * Registers the given \c handler to the Velocity Reached callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(int16_t velocity, void *user_data) \endcode
 *
 * This callback is triggered whenever a set velocity is reached. For example:
 * If a velocity of 0 is present, acceleration is set to 5000 and velocity
 * to 10000, the {@link tf_dc_v2_register_velocity_reached_callback} callback will be triggered after about
 * 2 seconds, when the set velocity is actually reached.
 *
 * \note
 *  Since we can't get any feedback from the DC motor, this only works if the
 *  acceleration (see {@link tf_dc_v2_set_motion}) is set smaller or equal to the
 *  maximum acceleration of the motor. Otherwise the motor will lag behind the
 *  control value and the callback will be triggered too early.
 */
int tf_dc_v2_register_velocity_reached_callback(TF_DCV2 *dc_v2, TF_DCV2_VelocityReachedHandler handler, void *user_data);


/**
 * \ingroup TF_DCV2
 *
 * Registers the given \c handler to the Current Velocity callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(int16_t velocity, void *user_data) \endcode
 *
 * This callback is triggered with the period that is set by
 * {@link tf_dc_v2_set_current_velocity_callback_configuration}. The parameter is the *current*
 * velocity used by the motor.
 *
 * The {@link tf_dc_v2_register_current_velocity_callback} callback is only triggered after the set period
 * if there is a change in the velocity.
 */
int tf_dc_v2_register_current_velocity_callback(TF_DCV2 *dc_v2, TF_DCV2_CurrentVelocityHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_DCV2
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_dc_v2_callback_tick(TF_DCV2 *dc_v2, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_DCV2
 *
 * Enables/Disables the driver chip. The driver parameters can be configured
 * (velocity, acceleration, etc) before it is enabled.
 */
int tf_dc_v2_set_enabled(TF_DCV2 *dc_v2, bool enabled);

/**
 * \ingroup TF_DCV2
 *
 * Returns *true* if the driver chip is enabled, *false* otherwise.
 */
int tf_dc_v2_get_enabled(TF_DCV2 *dc_v2, bool *ret_enabled);

/**
 * \ingroup TF_DCV2
 *
 * Sets the velocity of the motor. Whereas -32767 is full speed backward,
 * 0 is stop and 32767 is full speed forward. Depending on the
 * acceleration (see {@link tf_dc_v2_set_motion}), the motor is not immediately
 * brought to the velocity but smoothly accelerated.
 *
 * The velocity describes the duty cycle of the PWM with which the motor is
 * controlled, e.g. a velocity of 3277 sets a PWM with a 10% duty cycle.
 * You can not only control the duty cycle of the PWM but also the frequency,
 * see {@link tf_dc_v2_set_pwm_frequency}.
 */
int tf_dc_v2_set_velocity(TF_DCV2 *dc_v2, int16_t velocity);

/**
 * \ingroup TF_DCV2
 *
 * Returns the velocity as set by {@link tf_dc_v2_set_velocity}.
 */
int tf_dc_v2_get_velocity(TF_DCV2 *dc_v2, int16_t *ret_velocity);

/**
 * \ingroup TF_DCV2
 *
 * Returns the *current* velocity of the motor. This value is different
 * from {@link tf_dc_v2_get_velocity} whenever the motor is currently accelerating
 * to a goal set by {@link tf_dc_v2_set_velocity}.
 */
int tf_dc_v2_get_current_velocity(TF_DCV2 *dc_v2, int16_t *ret_velocity);

/**
 * \ingroup TF_DCV2
 *
 * Sets the acceleration and deceleration of the motor. It is given in *velocity/s*.
 * An acceleration of 10000 means, that every second the velocity is increased
 * by 10000 (or about 30% duty cycle).
 *
 * For example: If the current velocity is 0 and you want to accelerate to a
 * velocity of 16000 (about 50% duty cycle) in 10 seconds, you should set
 * an acceleration of 1600.
 *
 * If acceleration and deceleration is set to 0, there is no speed ramping, i.e. a
 * new velocity is immediately given to the motor.
 */
int tf_dc_v2_set_motion(TF_DCV2 *dc_v2, uint16_t acceleration, uint16_t deceleration);

/**
 * \ingroup TF_DCV2
 *
 * Returns the acceleration/deceleration as set by {@link tf_dc_v2_set_motion}.
 */
int tf_dc_v2_get_motion(TF_DCV2 *dc_v2, uint16_t *ret_acceleration, uint16_t *ret_deceleration);

/**
 * \ingroup TF_DCV2
 *
 * Executes an active full brake.
 *
 * \warning
 *  This function is for emergency purposes,
 *  where an immediate brake is necessary. Depending on the current velocity and
 *  the strength of the motor, a full brake can be quite violent.
 *
 * Call {@link tf_dc_v2_set_velocity} with 0 if you just want to stop the motor.
 */
int tf_dc_v2_full_brake(TF_DCV2 *dc_v2);

/**
 * \ingroup TF_DCV2
 *
 * Sets the drive mode. Possible modes are:
 *
 * * 0 = Drive/Brake
 * * 1 = Drive/Coast
 *
 * These modes are different kinds of motor controls.
 *
 * In Drive/Brake mode, the motor is always either driving or braking. There
 * is no freewheeling. Advantages are: A more linear correlation between
 * PWM and velocity, more exact accelerations and the possibility to drive
 * with slower velocities.
 *
 * In Drive/Coast mode, the motor is always either driving or freewheeling.
 * Advantages are: Less current consumption and less demands on the motor and
 * driver chip.
 */
int tf_dc_v2_set_drive_mode(TF_DCV2 *dc_v2, uint8_t mode);

/**
 * \ingroup TF_DCV2
 *
 * Returns the drive mode, as set by {@link tf_dc_v2_set_drive_mode}.
 */
int tf_dc_v2_get_drive_mode(TF_DCV2 *dc_v2, uint8_t *ret_mode);

/**
 * \ingroup TF_DCV2
 *
 * Sets the frequency of the PWM with which the motor is driven.
 * Often a high frequency
 * is less noisy and the motor runs smoother. However, with a low frequency
 * there are less switches and therefore fewer switching losses. Also with
 * most motors lower frequencies enable higher torque.
 *
 * If you have no idea what all this means, just ignore this function and use
 * the default frequency, it will very likely work fine.
 */
int tf_dc_v2_set_pwm_frequency(TF_DCV2 *dc_v2, uint16_t frequency);

/**
 * \ingroup TF_DCV2
 *
 * Returns the PWM frequency as set by {@link tf_dc_v2_set_pwm_frequency}.
 */
int tf_dc_v2_get_pwm_frequency(TF_DCV2 *dc_v2, uint16_t *ret_frequency);

/**
 * \ingroup TF_DCV2
 *
 * Returns input voltage and current usage of the driver.
 */
int tf_dc_v2_get_power_statistics(TF_DCV2 *dc_v2, uint16_t *ret_voltage, uint16_t *ret_current);

/**
 * \ingroup TF_DCV2
 *
 * Configures the error LED to be either turned off, turned on, blink in
 * heartbeat mode or show an error.
 *
 * If the LED is configured to show errors it has three different states:
 *
 * * Off: No error present.
 * * 1s interval blinking: Input voltage too low (below 6V).
 * * 250ms interval blinking: Overtemperature or overcurrent.
 */
int tf_dc_v2_set_error_led_config(TF_DCV2 *dc_v2, uint8_t config);

/**
 * \ingroup TF_DCV2
 *
 * Returns the LED configuration as set by {@link tf_dc_v2_set_error_led_config}
 */
int tf_dc_v2_get_error_led_config(TF_DCV2 *dc_v2, uint8_t *ret_config);

/**
 * \ingroup TF_DCV2
 *
 * Enable/Disable {@link tf_dc_v2_register_emergency_shutdown_callback} callback.
 */
int tf_dc_v2_set_emergency_shutdown_callback_configuration(TF_DCV2 *dc_v2, bool enabled);

/**
 * \ingroup TF_DCV2
 *
 * Returns the callback configuration as set by
 * {@link tf_dc_v2_set_emergency_shutdown_callback_configuration}.
 */
int tf_dc_v2_get_emergency_shutdown_callback_configuration(TF_DCV2 *dc_v2, bool *ret_enabled);

/**
 * \ingroup TF_DCV2
 *
 * Enable/Disable {@link tf_dc_v2_register_velocity_reached_callback} callback.
 */
int tf_dc_v2_set_velocity_reached_callback_configuration(TF_DCV2 *dc_v2, bool enabled);

/**
 * \ingroup TF_DCV2
 *
 * Returns the callback configuration as set by
 * {@link tf_dc_v2_set_velocity_reached_callback_configuration}.
 */
int tf_dc_v2_get_velocity_reached_callback_configuration(TF_DCV2 *dc_v2, bool *ret_enabled);

/**
 * \ingroup TF_DCV2
 *
 * The period is the period with which the {@link tf_dc_v2_register_current_velocity_callback}
 * callback is triggered periodically. A value of 0 turns the callback off.
 *
 * If the `value has to change`-parameter is set to true, the callback is only
 * triggered after the value has changed. If the value didn't change within the
 * period, the callback is triggered immediately on change.
 *
 * If it is set to false, the callback is continuously triggered with the period,
 * independent of the value.
 */
int tf_dc_v2_set_current_velocity_callback_configuration(TF_DCV2 *dc_v2, uint32_t period, bool value_has_to_change);

/**
 * \ingroup TF_DCV2
 *
 * Returns the callback configuration as set by
 * {@link tf_dc_v2_set_current_velocity_callback_configuration}.
 */
int tf_dc_v2_get_current_velocity_callback_configuration(TF_DCV2 *dc_v2, uint32_t *ret_period, bool *ret_value_has_to_change);

/**
 * \ingroup TF_DCV2
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
int tf_dc_v2_get_spitfp_error_count(TF_DCV2 *dc_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_DCV2
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
int tf_dc_v2_set_bootloader_mode(TF_DCV2 *dc_v2, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_DCV2
 *
 * Returns the current bootloader mode, see {@link tf_dc_v2_set_bootloader_mode}.
 */
int tf_dc_v2_get_bootloader_mode(TF_DCV2 *dc_v2, uint8_t *ret_mode);

/**
 * \ingroup TF_DCV2
 *
 * Sets the firmware pointer for {@link tf_dc_v2_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_dc_v2_set_write_firmware_pointer(TF_DCV2 *dc_v2, uint32_t pointer);

/**
 * \ingroup TF_DCV2
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_dc_v2_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_dc_v2_write_firmware(TF_DCV2 *dc_v2, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_DCV2
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_dc_v2_set_status_led_config(TF_DCV2 *dc_v2, uint8_t config);

/**
 * \ingroup TF_DCV2
 *
 * Returns the configuration as set by {@link tf_dc_v2_set_status_led_config}
 */
int tf_dc_v2_get_status_led_config(TF_DCV2 *dc_v2, uint8_t *ret_config);

/**
 * \ingroup TF_DCV2
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_dc_v2_get_chip_temperature(TF_DCV2 *dc_v2, int16_t *ret_temperature);

/**
 * \ingroup TF_DCV2
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_dc_v2_reset(TF_DCV2 *dc_v2);

/**
 * \ingroup TF_DCV2
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_dc_v2_write_uid(TF_DCV2 *dc_v2, uint32_t uid);

/**
 * \ingroup TF_DCV2
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_dc_v2_read_uid(TF_DCV2 *dc_v2, uint32_t *ret_uid);

/**
 * \ingroup TF_DCV2
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
int tf_dc_v2_get_identity(TF_DCV2 *dc_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
