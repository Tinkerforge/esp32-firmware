/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_COLOR_V2_H
#define TF_COLOR_V2_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_ColorV2 Color Bricklet 2.0
 */

struct TF_ColorV2;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_ColorV2_ColorHandler)(struct TF_ColorV2 *color_v2, uint16_t r, uint16_t g, uint16_t b, uint16_t c, void *user_data);
typedef void (*TF_ColorV2_IlluminanceHandler)(struct TF_ColorV2 *color_v2, uint32_t illuminance, void *user_data);
typedef void (*TF_ColorV2_ColorTemperatureHandler)(struct TF_ColorV2 *color_v2, uint16_t color_temperature, void *user_data);

#endif
/**
 * \ingroup TF_ColorV2
 *
 * Measures color (RGB value), illuminance and color temperature
 */
typedef struct TF_ColorV2 {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_ColorV2_ColorHandler color_handler;
    void *color_user_data;

    TF_ColorV2_IlluminanceHandler illuminance_handler;
    void *illuminance_user_data;

    TF_ColorV2_ColorTemperatureHandler color_temperature_handler;
    void *color_temperature_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[2];
} TF_ColorV2;

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_COLOR 1

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_SET_COLOR_CALLBACK_CONFIGURATION 2

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_COLOR_CALLBACK_CONFIGURATION 3

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_ILLUMINANCE 5

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_SET_ILLUMINANCE_CALLBACK_CONFIGURATION 6

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_ILLUMINANCE_CALLBACK_CONFIGURATION 7

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_COLOR_TEMPERATURE 9

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_SET_COLOR_TEMPERATURE_CALLBACK_CONFIGURATION 10

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_COLOR_TEMPERATURE_CALLBACK_CONFIGURATION 11

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_SET_LIGHT 13

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_LIGHT 14

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_SET_CONFIGURATION 15

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_CONFIGURATION 16

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_RESET 243

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_READ_UID 249

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_CALLBACK_COLOR 4

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_CALLBACK_ILLUMINANCE 8

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_CALLBACK_COLOR_TEMPERATURE 12

#endif

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_THRESHOLD_OPTION_OFF 'x'

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_THRESHOLD_OPTION_OUTSIDE 'o'

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_THRESHOLD_OPTION_INSIDE 'i'

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_THRESHOLD_OPTION_SMALLER '<'

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_THRESHOLD_OPTION_GREATER '>'

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_GAIN_1X 0

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_GAIN_4X 1

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_GAIN_16X 2

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_GAIN_60X 3

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_INTEGRATION_TIME_2MS 0

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_INTEGRATION_TIME_24MS 1

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_INTEGRATION_TIME_101MS 2

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_INTEGRATION_TIME_154MS 3

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_INTEGRATION_TIME_700MS 4

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_ColorV2
 */
#define TF_COLOR_V2_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_ColorV2
 *
 * This constant is used to identify a Color Bricklet 2.0.
 *
 * The {@link color_v2_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_COLOR_V2_DEVICE_IDENTIFIER 2128

/**
 * \ingroup TF_ColorV2
 *
 * This constant represents the display name of a Color Bricklet 2.0.
 */
#define TF_COLOR_V2_DEVICE_DISPLAY_NAME "Color Bricklet 2.0"

/**
 * \ingroup TF_ColorV2
 *
 * Creates the device object \c color_v2 with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_color_v2_create(TF_ColorV2 *color_v2, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_ColorV2
 *
 * Removes the device object \c color_v2 from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_color_v2_destroy(TF_ColorV2 *color_v2);

/**
 * \ingroup TF_ColorV2
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_color_v2_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_color_v2_get_response_expected(TF_ColorV2 *color_v2, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_ColorV2
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
int tf_color_v2_set_response_expected(TF_ColorV2 *color_v2, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_ColorV2
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_color_v2_set_response_expected_all(TF_ColorV2 *color_v2, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_ColorV2
 *
 * Registers the given \c handler to the Color callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t r, uint16_t g, uint16_t b, uint16_t c, void *user_data) \endcode
 *
 * This callback is triggered periodically with the period that is set by
 * {@link tf_color_v2_set_color_callback_configuration}. The parameter is the color
 * of the sensor as RGBC.
 *
 * The {@link tf_color_v2_register_color_callback} callback is only triggered if the color has changed since the
 * last triggering.
 */
int tf_color_v2_register_color_callback(TF_ColorV2 *color_v2, TF_ColorV2_ColorHandler handler, void *user_data);


/**
 * \ingroup TF_ColorV2
 *
 * Registers the given \c handler to the Illuminance callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint32_t illuminance, void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_color_v2_set_illuminance_callback_configuration}.
 *
 * The parameter is the same as {@link tf_color_v2_get_illuminance}.
 */
int tf_color_v2_register_illuminance_callback(TF_ColorV2 *color_v2, TF_ColorV2_IlluminanceHandler handler, void *user_data);


/**
 * \ingroup TF_ColorV2
 *
 * Registers the given \c handler to the Color Temperature callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t color_temperature, void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_color_v2_set_color_temperature_callback_configuration}.
 *
 * The parameter is the same as {@link tf_color_v2_get_color_temperature}.
 */
int tf_color_v2_register_color_temperature_callback(TF_ColorV2 *color_v2, TF_ColorV2_ColorTemperatureHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_ColorV2
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_color_v2_callback_tick(TF_ColorV2 *color_v2, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_ColorV2
 *
 * Returns the measured color of the sensor.
 *
 * The red (r), green (g), blue (b) and clear (c) colors are measured
 * with four different photodiodes that are responsive at different
 * wavelengths:
 *
 * .. image:: /Images/Bricklets/bricklet_color_wavelength_chart_600.jpg
 *    :scale: 100 %
 *    :alt: Chart Responsivity / Wavelength
 *    :align: center
 *    :target: ../../_images/Bricklets/bricklet_color_wavelength_chart_600.jpg
 *
 * If you want to get the color periodically, it is recommended
 * to use the {@link tf_color_v2_register_color_callback} callback and set the period with
 * {@link tf_color_v2_set_color_callback_configuration}.
 */
int tf_color_v2_get_color(TF_ColorV2 *color_v2, uint16_t *ret_r, uint16_t *ret_g, uint16_t *ret_b, uint16_t *ret_c);

/**
 * \ingroup TF_ColorV2
 *
 * The period is the period with which the {@link tf_color_v2_register_color_callback}
 * callback is triggered periodically. A value of 0 turns the callback off.
 *
 * If the `value has to change`-parameter is set to true, the callback is only
 * triggered after the value has changed. If the value didn't change within the
 * period, the callback is triggered immediately on change.
 *
 * If it is set to false, the callback is continuously triggered with the period,
 * independent of the value.
 */
int tf_color_v2_set_color_callback_configuration(TF_ColorV2 *color_v2, uint32_t period, bool value_has_to_change);

/**
 * \ingroup TF_ColorV2
 *
 * Returns the callback configuration as set by
 * {@link tf_color_v2_set_color_callback_configuration}.
 */
int tf_color_v2_get_color_callback_configuration(TF_ColorV2 *color_v2, uint32_t *ret_period, bool *ret_value_has_to_change);

/**
 * \ingroup TF_ColorV2
 *
 * Returns the illuminance affected by the gain and integration time as
 * set by {@link tf_color_v2_set_configuration}. To get the illuminance in Lux apply this formula::
 *
 *  lux = illuminance * 700 / gain / integration_time
 *
 * To get a correct illuminance measurement make sure that the color
 * values themselves are not saturated. The color value (R, G or B)
 * is saturated if it is equal to the maximum value of 65535.
 * In that case you have to reduce the gain, see {@link tf_color_v2_set_configuration}.
 *
 *
 * If you want to get the value periodically, it is recommended to use the
 * {@link tf_color_v2_register_illuminance_callback} callback. You can set the callback configuration
 * with {@link tf_color_v2_set_illuminance_callback_configuration}.
 */
int tf_color_v2_get_illuminance(TF_ColorV2 *color_v2, uint32_t *ret_illuminance);

/**
 * \ingroup TF_ColorV2
 *
 * The period is the period with which the {@link tf_color_v2_register_illuminance_callback} callback is triggered
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
 * The `option`-parameter together with min/max sets a threshold for the {@link tf_color_v2_register_illuminance_callback} callback.
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
int tf_color_v2_set_illuminance_callback_configuration(TF_ColorV2 *color_v2, uint32_t period, bool value_has_to_change, char option, uint32_t min, uint32_t max);

/**
 * \ingroup TF_ColorV2
 *
 * Returns the callback configuration as set by {@link tf_color_v2_set_illuminance_callback_configuration}.
 */
int tf_color_v2_get_illuminance_callback_configuration(TF_ColorV2 *color_v2, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, uint32_t *ret_min, uint32_t *ret_max);

/**
 * \ingroup TF_ColorV2
 *
 * Returns the color temperature.
 *
 * To get a correct color temperature measurement make sure that the color
 * values themselves are not saturated. The color value (R, G or B)
 * is saturated if it is equal to the maximum value of 65535.
 * In that case you have to reduce the gain, see {@link tf_color_v2_set_configuration}.
 *
 *
 * If you want to get the value periodically, it is recommended to use the
 * {@link tf_color_v2_register_color_temperature_callback} callback. You can set the callback configuration
 * with {@link tf_color_v2_set_color_temperature_callback_configuration}.
 */
int tf_color_v2_get_color_temperature(TF_ColorV2 *color_v2, uint16_t *ret_color_temperature);

/**
 * \ingroup TF_ColorV2
 *
 * The period is the period with which the {@link tf_color_v2_register_color_temperature_callback} callback is triggered
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
 * The `option`-parameter together with min/max sets a threshold for the {@link tf_color_v2_register_color_temperature_callback} callback.
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
int tf_color_v2_set_color_temperature_callback_configuration(TF_ColorV2 *color_v2, uint32_t period, bool value_has_to_change, char option, uint16_t min, uint16_t max);

/**
 * \ingroup TF_ColorV2
 *
 * Returns the callback configuration as set by {@link tf_color_v2_set_color_temperature_callback_configuration}.
 */
int tf_color_v2_get_color_temperature_callback_configuration(TF_ColorV2 *color_v2, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, uint16_t *ret_min, uint16_t *ret_max);

/**
 * \ingroup TF_ColorV2
 *
 * Turns the white LED on the Bricklet on/off.
 */
int tf_color_v2_set_light(TF_ColorV2 *color_v2, bool enable);

/**
 * \ingroup TF_ColorV2
 *
 * Returns the value as set by {@link tf_color_v2_set_light}.
 */
int tf_color_v2_get_light(TF_ColorV2 *color_v2, bool *ret_enable);

/**
 * \ingroup TF_ColorV2
 *
 * Sets the configuration of the sensor. Gain and integration time
 * can be configured this way.
 *
 * For configuring the gain:
 *
 * * 0: 1x Gain
 * * 1: 4x Gain
 * * 2: 16x Gain
 * * 3: 60x Gain
 *
 * For configuring the integration time:
 *
 * * 0: 2.4ms
 * * 1: 24ms
 * * 2: 101ms
 * * 3: 154ms
 * * 4: 700ms
 *
 * Increasing the gain enables the sensor to detect a
 * color from a higher distance.
 *
 * The integration time provides a trade-off between conversion time
 * and accuracy. With a longer integration time the values read will
 * be more accurate but it will take longer to get the conversion
 * results.
 */
int tf_color_v2_set_configuration(TF_ColorV2 *color_v2, uint8_t gain, uint8_t integration_time);

/**
 * \ingroup TF_ColorV2
 *
 * Returns the configuration as set by {@link tf_color_v2_set_configuration}.
 */
int tf_color_v2_get_configuration(TF_ColorV2 *color_v2, uint8_t *ret_gain, uint8_t *ret_integration_time);

/**
 * \ingroup TF_ColorV2
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
int tf_color_v2_get_spitfp_error_count(TF_ColorV2 *color_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_ColorV2
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
int tf_color_v2_set_bootloader_mode(TF_ColorV2 *color_v2, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_ColorV2
 *
 * Returns the current bootloader mode, see {@link tf_color_v2_set_bootloader_mode}.
 */
int tf_color_v2_get_bootloader_mode(TF_ColorV2 *color_v2, uint8_t *ret_mode);

/**
 * \ingroup TF_ColorV2
 *
 * Sets the firmware pointer for {@link tf_color_v2_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_color_v2_set_write_firmware_pointer(TF_ColorV2 *color_v2, uint32_t pointer);

/**
 * \ingroup TF_ColorV2
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_color_v2_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_color_v2_write_firmware(TF_ColorV2 *color_v2, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_ColorV2
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_color_v2_set_status_led_config(TF_ColorV2 *color_v2, uint8_t config);

/**
 * \ingroup TF_ColorV2
 *
 * Returns the configuration as set by {@link tf_color_v2_set_status_led_config}
 */
int tf_color_v2_get_status_led_config(TF_ColorV2 *color_v2, uint8_t *ret_config);

/**
 * \ingroup TF_ColorV2
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_color_v2_get_chip_temperature(TF_ColorV2 *color_v2, int16_t *ret_temperature);

/**
 * \ingroup TF_ColorV2
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_color_v2_reset(TF_ColorV2 *color_v2);

/**
 * \ingroup TF_ColorV2
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_color_v2_write_uid(TF_ColorV2 *color_v2, uint32_t uid);

/**
 * \ingroup TF_ColorV2
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_color_v2_read_uid(TF_ColorV2 *color_v2, uint32_t *ret_uid);

/**
 * \ingroup TF_ColorV2
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
int tf_color_v2_get_identity(TF_ColorV2 *color_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
