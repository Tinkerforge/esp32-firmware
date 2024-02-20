/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_AIR_QUALITY_H
#define TF_AIR_QUALITY_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_AirQuality Air Quality Bricklet
 */

struct TF_AirQuality;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_AirQuality_AllValuesHandler)(struct TF_AirQuality *air_quality, int32_t iaq_index, uint8_t iaq_index_accuracy, int32_t temperature, int32_t humidity, int32_t air_pressure, void *user_data);
typedef void (*TF_AirQuality_IAQIndexHandler)(struct TF_AirQuality *air_quality, int32_t iaq_index, uint8_t iaq_index_accuracy, void *user_data);
typedef void (*TF_AirQuality_TemperatureHandler)(struct TF_AirQuality *air_quality, int32_t temperature, void *user_data);
typedef void (*TF_AirQuality_HumidityHandler)(struct TF_AirQuality *air_quality, int32_t humidity, void *user_data);
typedef void (*TF_AirQuality_AirPressureHandler)(struct TF_AirQuality *air_quality, int32_t air_pressure, void *user_data);

#endif
/**
 * \ingroup TF_AirQuality
 *
 * Measures IAQ index, temperature, humidity and air pressure
 */
typedef struct TF_AirQuality {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_AirQuality_AllValuesHandler all_values_handler;
    void *all_values_user_data;

    TF_AirQuality_IAQIndexHandler iaq_index_handler;
    void *iaq_index_user_data;

    TF_AirQuality_TemperatureHandler temperature_handler;
    void *temperature_user_data;

    TF_AirQuality_HumidityHandler humidity_handler;
    void *humidity_user_data;

    TF_AirQuality_AirPressureHandler air_pressure_handler;
    void *air_pressure_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[2];
} TF_AirQuality;

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_ALL_VALUES 1

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_SET_TEMPERATURE_OFFSET 2

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_TEMPERATURE_OFFSET 3

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_SET_ALL_VALUES_CALLBACK_CONFIGURATION 4

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_ALL_VALUES_CALLBACK_CONFIGURATION 5

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_IAQ_INDEX 7

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_SET_IAQ_INDEX_CALLBACK_CONFIGURATION 8

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_IAQ_INDEX_CALLBACK_CONFIGURATION 9

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_TEMPERATURE 11

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_SET_TEMPERATURE_CALLBACK_CONFIGURATION 12

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_TEMPERATURE_CALLBACK_CONFIGURATION 13

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_HUMIDITY 15

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_SET_HUMIDITY_CALLBACK_CONFIGURATION 16

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_HUMIDITY_CALLBACK_CONFIGURATION 17

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_AIR_PRESSURE 19

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_SET_AIR_PRESSURE_CALLBACK_CONFIGURATION 20

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_AIR_PRESSURE_CALLBACK_CONFIGURATION 21

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_REMOVE_CALIBRATION 23

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_SET_BACKGROUND_CALIBRATION_DURATION 24

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_BACKGROUND_CALIBRATION_DURATION 25

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_RESET 243

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_READ_UID 249

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_CALLBACK_ALL_VALUES 6

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_CALLBACK_IAQ_INDEX 10

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_CALLBACK_TEMPERATURE 14

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_CALLBACK_HUMIDITY 18

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_CALLBACK_AIR_PRESSURE 22

#endif

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_ACCURACY_UNRELIABLE 0

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_ACCURACY_LOW 1

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_ACCURACY_MEDIUM 2

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_ACCURACY_HIGH 3

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_THRESHOLD_OPTION_OFF 'x'

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_THRESHOLD_OPTION_OUTSIDE 'o'

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_THRESHOLD_OPTION_INSIDE 'i'

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_THRESHOLD_OPTION_SMALLER '<'

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_THRESHOLD_OPTION_GREATER '>'

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_DURATION_4_DAYS 0

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_DURATION_28_DAYS 1

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_AirQuality
 */
#define TF_AIR_QUALITY_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_AirQuality
 *
 * This constant is used to identify a Air Quality Bricklet.
 *
 * The {@link air_quality_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_AIR_QUALITY_DEVICE_IDENTIFIER 297

/**
 * \ingroup TF_AirQuality
 *
 * This constant represents the display name of a Air Quality Bricklet.
 */
#define TF_AIR_QUALITY_DEVICE_DISPLAY_NAME "Air Quality Bricklet"

/**
 * \ingroup TF_AirQuality
 *
 * Creates the device object \c air_quality with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_air_quality_create(TF_AirQuality *air_quality, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_AirQuality
 *
 * Removes the device object \c air_quality from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_air_quality_destroy(TF_AirQuality *air_quality);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_air_quality_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_air_quality_get_response_expected(TF_AirQuality *air_quality, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_AirQuality
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
int tf_air_quality_set_response_expected(TF_AirQuality *air_quality, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_AirQuality
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_air_quality_set_response_expected_all(TF_AirQuality *air_quality, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_AirQuality
 *
 * Registers the given \c handler to the All Values callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(int32_t iaq_index, uint8_t iaq_index_accuracy, int32_t temperature, int32_t humidity, int32_t air_pressure, void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_air_quality_set_all_values_callback_configuration}.
 *
 * The parameters are the same as {@link tf_air_quality_get_all_values}.
 */
int tf_air_quality_register_all_values_callback(TF_AirQuality *air_quality, TF_AirQuality_AllValuesHandler handler, void *user_data);


/**
 * \ingroup TF_AirQuality
 *
 * Registers the given \c handler to the IAQ Index callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(int32_t iaq_index, uint8_t iaq_index_accuracy, void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_air_quality_set_iaq_index_callback_configuration}.
 *
 * The parameters are the same as {@link tf_air_quality_get_iaq_index}.
 */
int tf_air_quality_register_iaq_index_callback(TF_AirQuality *air_quality, TF_AirQuality_IAQIndexHandler handler, void *user_data);


/**
 * \ingroup TF_AirQuality
 *
 * Registers the given \c handler to the Temperature callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(int32_t temperature, void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_air_quality_set_temperature_callback_configuration}.
 *
 * The parameter is the same as {@link tf_air_quality_get_temperature}.
 */
int tf_air_quality_register_temperature_callback(TF_AirQuality *air_quality, TF_AirQuality_TemperatureHandler handler, void *user_data);


/**
 * \ingroup TF_AirQuality
 *
 * Registers the given \c handler to the Humidity callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(int32_t humidity, void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_air_quality_set_humidity_callback_configuration}.
 *
 * The parameter is the same as {@link tf_air_quality_get_humidity}.
 */
int tf_air_quality_register_humidity_callback(TF_AirQuality *air_quality, TF_AirQuality_HumidityHandler handler, void *user_data);


/**
 * \ingroup TF_AirQuality
 *
 * Registers the given \c handler to the Air Pressure callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(int32_t air_pressure, void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_air_quality_set_air_pressure_callback_configuration}.
 *
 * The parameter is the same as {@link tf_air_quality_get_air_pressure}.
 */
int tf_air_quality_register_air_pressure_callback(TF_AirQuality *air_quality, TF_AirQuality_AirPressureHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_AirQuality
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_air_quality_callback_tick(TF_AirQuality *air_quality, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_AirQuality
 *
 * Returns all values measured by the Air Quality Bricklet. The values are
 * IAQ (Indoor Air Quality) Index (higher value means greater level of air pollution), IAQ Index Accuracy, Temperature, Humidity and
 * Air Pressure.
 *
 * .. image:: /Images/Misc/bricklet_air_quality_iaq_index.png
 *    :scale: 100 %
 *    :alt: Air Quality Index description
 *    :align: center
 *    :target: ../../_images/Misc/bricklet_air_quality_iaq_index.png
 */
int tf_air_quality_get_all_values(TF_AirQuality *air_quality, int32_t *ret_iaq_index, uint8_t *ret_iaq_index_accuracy, int32_t *ret_temperature, int32_t *ret_humidity, int32_t *ret_air_pressure);

/**
 * \ingroup TF_AirQuality
 *
 * Sets a temperature offset. A offset of 10 will decrease the measured temperature by 0.1 °C.
 *
 * If you install this Bricklet into an enclosure and you want to measure the ambient
 * temperature, you may have to decrease the measured temperature by some value to
 * compensate for the error because of the heating inside of the enclosure.
 *
 * We recommend that you leave the parts in the enclosure running for at least
 * 24 hours such that a temperature equilibrium can be reached. After that you can measure
 * the temperature directly outside of enclosure and set the difference as offset.
 *
 * This temperature offset is used to calculate the relative humidity and
 * IAQ index measurements. In case the Bricklet is installed in an enclosure, we
 * recommend to measure and set the temperature offset to improve the accuracy of
 * the measurements.
 */
int tf_air_quality_set_temperature_offset(TF_AirQuality *air_quality, int32_t offset);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the temperature offset as set by
 * {@link tf_air_quality_set_temperature_offset}.
 */
int tf_air_quality_get_temperature_offset(TF_AirQuality *air_quality, int32_t *ret_offset);

/**
 * \ingroup TF_AirQuality
 *
 * The period is the period with which the {@link tf_air_quality_register_all_values_callback}
 * callback is triggered periodically. A value of 0 turns the callback off.
 *
 * If the `value has to change`-parameter is set to true, the callback is only
 * triggered after at least one of the values has changed. If the values didn't
 * change within the period, the callback is triggered immediately on change.
 *
 * If it is set to false, the callback is continuously triggered with the period,
 * independent of the value.
 */
int tf_air_quality_set_all_values_callback_configuration(TF_AirQuality *air_quality, uint32_t period, bool value_has_to_change);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the callback configuration as set by
 * {@link tf_air_quality_set_all_values_callback_configuration}.
 */
int tf_air_quality_get_all_values_callback_configuration(TF_AirQuality *air_quality, uint32_t *ret_period, bool *ret_value_has_to_change);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the IAQ index and accuracy. The higher the IAQ index, the greater the level of air pollution.
 *
 * .. image:: /Images/Misc/bricklet_air_quality_iaq_index.png
 *    :scale: 100 %
 *    :alt: IAQ index description
 *    :align: center
 *    :target: ../../_images/Misc/bricklet_air_quality_iaq_index.png
 *
 * If you want to get the value periodically, it is recommended to use the
 * {@link tf_air_quality_register_iaq_index_callback} callback. You can set the callback configuration
 * with {@link tf_air_quality_set_iaq_index_callback_configuration}.
 */
int tf_air_quality_get_iaq_index(TF_AirQuality *air_quality, int32_t *ret_iaq_index, uint8_t *ret_iaq_index_accuracy);

/**
 * \ingroup TF_AirQuality
 *
 * The period is the period with which the {@link tf_air_quality_register_iaq_index_callback}
 * callback is triggered periodically. A value of 0 turns the callback off.
 *
 * If the `value has to change`-parameter is set to true, the callback is only
 * triggered after at least one of the values has changed. If the values didn't
 * change within the period, the callback is triggered immediately on change.
 *
 * If it is set to false, the callback is continuously triggered with the period,
 * independent of the value.
 */
int tf_air_quality_set_iaq_index_callback_configuration(TF_AirQuality *air_quality, uint32_t period, bool value_has_to_change);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the callback configuration as set by
 * {@link tf_air_quality_set_iaq_index_callback_configuration}.
 */
int tf_air_quality_get_iaq_index_callback_configuration(TF_AirQuality *air_quality, uint32_t *ret_period, bool *ret_value_has_to_change);

/**
 * \ingroup TF_AirQuality
 *
 * Returns temperature.
 *
 *
 * If you want to get the value periodically, it is recommended to use the
 * {@link tf_air_quality_register_temperature_callback} callback. You can set the callback configuration
 * with {@link tf_air_quality_set_temperature_callback_configuration}.
 */
int tf_air_quality_get_temperature(TF_AirQuality *air_quality, int32_t *ret_temperature);

/**
 * \ingroup TF_AirQuality
 *
 * The period is the period with which the {@link tf_air_quality_register_temperature_callback} callback is triggered
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
 * The `option`-parameter together with min/max sets a threshold for the {@link tf_air_quality_register_temperature_callback} callback.
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
int tf_air_quality_set_temperature_callback_configuration(TF_AirQuality *air_quality, uint32_t period, bool value_has_to_change, char option, int32_t min, int32_t max);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the callback configuration as set by {@link tf_air_quality_set_temperature_callback_configuration}.
 */
int tf_air_quality_get_temperature_callback_configuration(TF_AirQuality *air_quality, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, int32_t *ret_min, int32_t *ret_max);

/**
 * \ingroup TF_AirQuality
 *
 * Returns relative humidity.
 *
 *
 * If you want to get the value periodically, it is recommended to use the
 * {@link tf_air_quality_register_humidity_callback} callback. You can set the callback configuration
 * with {@link tf_air_quality_set_humidity_callback_configuration}.
 */
int tf_air_quality_get_humidity(TF_AirQuality *air_quality, int32_t *ret_humidity);

/**
 * \ingroup TF_AirQuality
 *
 * The period is the period with which the {@link tf_air_quality_register_humidity_callback} callback is triggered
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
 * The `option`-parameter together with min/max sets a threshold for the {@link tf_air_quality_register_humidity_callback} callback.
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
int tf_air_quality_set_humidity_callback_configuration(TF_AirQuality *air_quality, uint32_t period, bool value_has_to_change, char option, int32_t min, int32_t max);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the callback configuration as set by {@link tf_air_quality_set_humidity_callback_configuration}.
 */
int tf_air_quality_get_humidity_callback_configuration(TF_AirQuality *air_quality, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, int32_t *ret_min, int32_t *ret_max);

/**
 * \ingroup TF_AirQuality
 *
 * Returns air pressure.
 *
 *
 * If you want to get the value periodically, it is recommended to use the
 * {@link tf_air_quality_register_air_pressure_callback} callback. You can set the callback configuration
 * with {@link tf_air_quality_set_air_pressure_callback_configuration}.
 */
int tf_air_quality_get_air_pressure(TF_AirQuality *air_quality, int32_t *ret_air_pressure);

/**
 * \ingroup TF_AirQuality
 *
 * The period is the period with which the {@link tf_air_quality_register_air_pressure_callback} callback is triggered
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
 * The `option`-parameter together with min/max sets a threshold for the {@link tf_air_quality_register_air_pressure_callback} callback.
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
int tf_air_quality_set_air_pressure_callback_configuration(TF_AirQuality *air_quality, uint32_t period, bool value_has_to_change, char option, int32_t min, int32_t max);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the callback configuration as set by {@link tf_air_quality_set_air_pressure_callback_configuration}.
 */
int tf_air_quality_get_air_pressure_callback_configuration(TF_AirQuality *air_quality, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, int32_t *ret_min, int32_t *ret_max);

/**
 * \ingroup TF_AirQuality
 *
 * Deletes the calibration from flash. After you call this function,
 * you need to power cycle the Air Quality Bricklet.
 *
 * On the next power up the Bricklet will start a new calibration, as
 * if it was started for the very first time.
 *
 * The calibration is based on the data of the last four days, so it takes
 * four days until a full calibration is re-established.
 *
 * .. versionadded:: 2.0.3$nbsp;(Plugin)
 */
int tf_air_quality_remove_calibration(TF_AirQuality *air_quality);

/**
 * \ingroup TF_AirQuality
 *
 * The Air Quality Bricklet uses an automatic background calibration mechanism to
 * calculate the IAQ Index. This calibration mechanism considers a history of
 * measured data. The duration of this history can be configured to either be
 * 4 days or 28 days.
 *
 * If you keep the Bricklet mostly at one place and it does not get moved around
 * to different environments, we recommend that you use a duration of 28 days.
 *
 * If you change the duration, the current calibration will be discarded and
 * the calibration will start from beginning again. The configuration of the
 * duration is saved in flash, so you should only have to call this function
 * once in the lifetime of the Bricklet.
 *
 * The Bricklet has to be power cycled after this function is called
 * for a duration change to take effect.
 *
 * Before firmware version 2.0.3 this was not configurable and the duration was
 * 4 days.
 *
 * The default value (since firmware version 2.0.3) is 28 days.
 *
 * .. versionadded:: 2.0.3$nbsp;(Plugin)
 */
int tf_air_quality_set_background_calibration_duration(TF_AirQuality *air_quality, uint8_t duration);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the background calibration duration as set by
 * {@link tf_air_quality_set_background_calibration_duration}.
 *
 * .. versionadded:: 2.0.3$nbsp;(Plugin)
 */
int tf_air_quality_get_background_calibration_duration(TF_AirQuality *air_quality, uint8_t *ret_duration);

/**
 * \ingroup TF_AirQuality
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
int tf_air_quality_get_spitfp_error_count(TF_AirQuality *air_quality, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_AirQuality
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
int tf_air_quality_set_bootloader_mode(TF_AirQuality *air_quality, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the current bootloader mode, see {@link tf_air_quality_set_bootloader_mode}.
 */
int tf_air_quality_get_bootloader_mode(TF_AirQuality *air_quality, uint8_t *ret_mode);

/**
 * \ingroup TF_AirQuality
 *
 * Sets the firmware pointer for {@link tf_air_quality_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_air_quality_set_write_firmware_pointer(TF_AirQuality *air_quality, uint32_t pointer);

/**
 * \ingroup TF_AirQuality
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_air_quality_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_air_quality_write_firmware(TF_AirQuality *air_quality, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_AirQuality
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_air_quality_set_status_led_config(TF_AirQuality *air_quality, uint8_t config);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the configuration as set by {@link tf_air_quality_set_status_led_config}
 */
int tf_air_quality_get_status_led_config(TF_AirQuality *air_quality, uint8_t *ret_config);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_air_quality_get_chip_temperature(TF_AirQuality *air_quality, int16_t *ret_temperature);

/**
 * \ingroup TF_AirQuality
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_air_quality_reset(TF_AirQuality *air_quality);

/**
 * \ingroup TF_AirQuality
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_air_quality_write_uid(TF_AirQuality *air_quality, uint32_t uid);

/**
 * \ingroup TF_AirQuality
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_air_quality_read_uid(TF_AirQuality *air_quality, uint32_t *ret_uid);

/**
 * \ingroup TF_AirQuality
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
int tf_air_quality_get_identity(TF_AirQuality *air_quality, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
