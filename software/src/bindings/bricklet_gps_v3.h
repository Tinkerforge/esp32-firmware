/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_GPS_V3_H
#define TF_GPS_V3_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_GPSV3 GPS Bricklet 3.0
 */

struct TF_GPSV3;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_GPSV3_PulsePerSecondHandler)(struct TF_GPSV3 *gps_v3, void *user_data);
typedef void (*TF_GPSV3_CoordinatesHandler)(struct TF_GPSV3 *gps_v3, uint32_t latitude, char ns, uint32_t longitude, char ew, void *user_data);
typedef void (*TF_GPSV3_StatusHandler)(struct TF_GPSV3 *gps_v3, bool has_fix, uint8_t satellites_view, void *user_data);
typedef void (*TF_GPSV3_AltitudeHandler)(struct TF_GPSV3 *gps_v3, int32_t altitude, int32_t geoidal_separation, void *user_data);
typedef void (*TF_GPSV3_MotionHandler)(struct TF_GPSV3 *gps_v3, uint32_t course, uint32_t speed, void *user_data);
typedef void (*TF_GPSV3_DateTimeHandler)(struct TF_GPSV3 *gps_v3, uint32_t date, uint32_t time, void *user_data);

#endif
/**
 * \ingroup TF_GPSV3
 *
 * Determine position, velocity and altitude using GPS
 */
typedef struct TF_GPSV3 {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_GPSV3_PulsePerSecondHandler pulse_per_second_handler;
    void *pulse_per_second_user_data;

    TF_GPSV3_CoordinatesHandler coordinates_handler;
    void *coordinates_user_data;

    TF_GPSV3_StatusHandler status_handler;
    void *status_user_data;

    TF_GPSV3_AltitudeHandler altitude_handler;
    void *altitude_user_data;

    TF_GPSV3_MotionHandler motion_handler;
    void *motion_user_data;

    TF_GPSV3_DateTimeHandler date_time_handler;
    void *date_time_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[2];
} TF_GPSV3;

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_COORDINATES 1

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_STATUS 2

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_ALTITUDE 3

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_MOTION 4

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_DATE_TIME 5

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_RESTART 6

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_SATELLITE_SYSTEM_STATUS_LOW_LEVEL 7

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_SATELLITE_STATUS 8

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_SET_FIX_LED_CONFIG 9

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_FIX_LED_CONFIG 10

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_SET_COORDINATES_CALLBACK_PERIOD 11

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_COORDINATES_CALLBACK_PERIOD 12

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_SET_STATUS_CALLBACK_PERIOD 13

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_STATUS_CALLBACK_PERIOD 14

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD 15

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_ALTITUDE_CALLBACK_PERIOD 16

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_SET_MOTION_CALLBACK_PERIOD 17

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_MOTION_CALLBACK_PERIOD 18

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD 19

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_DATE_TIME_CALLBACK_PERIOD 20

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_SET_SBAS_CONFIG 27

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_SBAS_CONFIG 28

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_RESET 243

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_READ_UID 249

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_CALLBACK_PULSE_PER_SECOND 21

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_CALLBACK_COORDINATES 22

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_CALLBACK_STATUS 23

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_CALLBACK_ALTITUDE 24

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_CALLBACK_MOTION 25

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_CALLBACK_DATE_TIME 26

#endif

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_RESTART_TYPE_HOT_START 0

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_RESTART_TYPE_WARM_START 1

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_RESTART_TYPE_COLD_START 2

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_RESTART_TYPE_FACTORY_RESET 3

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_SATELLITE_SYSTEM_GPS 0

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_SATELLITE_SYSTEM_GLONASS 1

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_SATELLITE_SYSTEM_GALILEO 2

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FIX_NO_FIX 1

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FIX_2D_FIX 2

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FIX_3D_FIX 3

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FIX_LED_CONFIG_OFF 0

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FIX_LED_CONFIG_ON 1

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FIX_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FIX_LED_CONFIG_SHOW_FIX 3

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_FIX_LED_CONFIG_SHOW_PPS 4

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_SBAS_ENABLED 0

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_SBAS_DISABLED 1

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_GPSV3
 */
#define TF_GPS_V3_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_GPSV3
 *
 * This constant is used to identify a GPS Bricklet 3.0.
 *
 * The {@link gps_v3_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_GPS_V3_DEVICE_IDENTIFIER 2171

/**
 * \ingroup TF_GPSV3
 *
 * This constant represents the display name of a GPS Bricklet 3.0.
 */
#define TF_GPS_V3_DEVICE_DISPLAY_NAME "GPS Bricklet 3.0"

/**
 * \ingroup TF_GPSV3
 *
 * Creates the device object \c gps_v3 with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_gps_v3_create(TF_GPSV3 *gps_v3, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_GPSV3
 *
 * Removes the device object \c gps_v3 from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_gps_v3_destroy(TF_GPSV3 *gps_v3);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_gps_v3_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_gps_v3_get_response_expected(TF_GPSV3 *gps_v3, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_GPSV3
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
int tf_gps_v3_set_response_expected(TF_GPSV3 *gps_v3, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_GPSV3
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_gps_v3_set_response_expected_all(TF_GPSV3 *gps_v3, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_GPSV3
 *
 * Registers the given \c handler to the Pulse Per Second callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(void *user_data) \endcode
 *
 * This callback is triggered precisely once per second,
 * see `PPS <https://en.wikipedia.org/wiki/Pulse-per-second_signal>`__.
 *
 * The precision of two subsequent pulses will be skewed because
 * of the latency in the USB/RS485/Ethernet connection. But in the
 * long run this will be very precise. For example a count of
 * 3600 pulses will take exactly 1 hour.
 */
int tf_gps_v3_register_pulse_per_second_callback(TF_GPSV3 *gps_v3, TF_GPSV3_PulsePerSecondHandler handler, void *user_data);


/**
 * \ingroup TF_GPSV3
 *
 * Registers the given \c handler to the Coordinates callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint32_t latitude, char ns, uint32_t longitude, char ew, void *user_data) \endcode
 *
 * This callback is triggered periodically with the period that is set by
 * {@link tf_gps_v3_set_coordinates_callback_period}. The parameters are the same
 * as for {@link tf_gps_v3_get_coordinates}.
 *
 * The {@link tf_gps_v3_register_coordinates_callback} callback is only triggered if the coordinates changed
 * since the last triggering and if there is currently a fix as indicated by
 * {@link tf_gps_v3_get_status}.
 */
int tf_gps_v3_register_coordinates_callback(TF_GPSV3 *gps_v3, TF_GPSV3_CoordinatesHandler handler, void *user_data);


/**
 * \ingroup TF_GPSV3
 *
 * Registers the given \c handler to the Status callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(bool has_fix, uint8_t satellites_view, void *user_data) \endcode
 *
 * This callback is triggered periodically with the period that is set by
 * {@link tf_gps_v3_set_status_callback_period}. The parameters are the same
 * as for {@link tf_gps_v3_get_status}.
 *
 * The {@link tf_gps_v3_register_status_callback} callback is only triggered if the status changed since the
 * last triggering.
 */
int tf_gps_v3_register_status_callback(TF_GPSV3 *gps_v3, TF_GPSV3_StatusHandler handler, void *user_data);


/**
 * \ingroup TF_GPSV3
 *
 * Registers the given \c handler to the Altitude callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(int32_t altitude, int32_t geoidal_separation, void *user_data) \endcode
 *
 * This callback is triggered periodically with the period that is set by
 * {@link tf_gps_v3_set_altitude_callback_period}. The parameters are the same
 * as for {@link tf_gps_v3_get_altitude}.
 *
 * The {@link tf_gps_v3_register_altitude_callback} callback is only triggered if the altitude changed since the
 * last triggering and if there is currently a fix as indicated by
 * {@link tf_gps_v3_get_status}.
 */
int tf_gps_v3_register_altitude_callback(TF_GPSV3 *gps_v3, TF_GPSV3_AltitudeHandler handler, void *user_data);


/**
 * \ingroup TF_GPSV3
 *
 * Registers the given \c handler to the Motion callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint32_t course, uint32_t speed, void *user_data) \endcode
 *
 * This callback is triggered periodically with the period that is set by
 * {@link tf_gps_v3_set_motion_callback_period}. The parameters are the same
 * as for {@link tf_gps_v3_get_motion}.
 *
 * The {@link tf_gps_v3_register_motion_callback} callback is only triggered if the motion changed since the
 * last triggering and if there is currently a fix as indicated by
 * {@link tf_gps_v3_get_status}.
 */
int tf_gps_v3_register_motion_callback(TF_GPSV3 *gps_v3, TF_GPSV3_MotionHandler handler, void *user_data);


/**
 * \ingroup TF_GPSV3
 *
 * Registers the given \c handler to the Date Time callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint32_t date, uint32_t time, void *user_data) \endcode
 *
 * This callback is triggered periodically with the period that is set by
 * {@link tf_gps_v3_set_date_time_callback_period}. The parameters are the same
 * as for {@link tf_gps_v3_get_date_time}.
 *
 * The {@link tf_gps_v3_register_date_time_callback} callback is only triggered if the date or time changed
 * since the last triggering.
 */
int tf_gps_v3_register_date_time_callback(TF_GPSV3 *gps_v3, TF_GPSV3_DateTimeHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_GPSV3
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_gps_v3_callback_tick(TF_GPSV3 *gps_v3, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_GPSV3
 *
 * Returns the GPS coordinates. Latitude and longitude are given in the
 * ``DD.dddddd°`` format, the value 57123468 means 57.123468°.
 * The parameter ``ns`` and ``ew`` are the cardinal directions for
 * latitude and longitude. Possible values for ``ns`` and ``ew`` are 'N', 'S', 'E'
 * and 'W' (north, south, east and west).
 *
 * This data is only valid if there is currently a fix as indicated by
 * {@link tf_gps_v3_get_status}.
 */
int tf_gps_v3_get_coordinates(TF_GPSV3 *gps_v3, uint32_t *ret_latitude, char *ret_ns, uint32_t *ret_longitude, char *ret_ew);

/**
 * \ingroup TF_GPSV3
 *
 * Returns if a fix is currently available as well as the number of
 * satellites that are in view.
 *
 * There is also a :ref:`green LED <gps_v2_bricklet_fix_led>` on the Bricklet that
 * indicates the fix status.
 */
int tf_gps_v3_get_status(TF_GPSV3 *gps_v3, bool *ret_has_fix, uint8_t *ret_satellites_view);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the current altitude and corresponding geoidal separation.
 *
 * This data is only valid if there is currently a fix as indicated by
 * {@link tf_gps_v3_get_status}.
 */
int tf_gps_v3_get_altitude(TF_GPSV3 *gps_v3, int32_t *ret_altitude, int32_t *ret_geoidal_separation);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the current course and speed. A course of 0° means the Bricklet is
 * traveling north bound and 90° means it is traveling east bound.
 *
 * Please note that this only returns useful values if an actual movement
 * is present.
 *
 * This data is only valid if there is currently a fix as indicated by
 * {@link tf_gps_v3_get_status}.
 */
int tf_gps_v3_get_motion(TF_GPSV3 *gps_v3, uint32_t *ret_course, uint32_t *ret_speed);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the current date and time. The date is
 * given in the format ``ddmmyy`` and the time is given
 * in the format ``hhmmss.sss``. For example, 140713 means
 * 14.07.13 as date and 195923568 means 19:59:23.568 as time.
 */
int tf_gps_v3_get_date_time(TF_GPSV3 *gps_v3, uint32_t *ret_date, uint32_t *ret_time);

/**
 * \ingroup TF_GPSV3
 *
 * Restarts the GPS Bricklet, the following restart types are available:
 *
 * \verbatim
 *  "Value", "Description"
 *
 *  "0", "Hot start (use all available data in the NV store)"
 *  "1", "Warm start (don't use ephemeris at restart)"
 *  "2", "Cold start (don't use time, position, almanacs and ephemeris at restart)"
 *  "3", "Factory reset (clear all system/user configurations at restart)"
 * \endverbatim
 */
int tf_gps_v3_restart(TF_GPSV3 *gps_v3, uint8_t restart_type);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the
 *
 * * satellite numbers list (up to 12 items)
 * * fix value,
 * * PDOP value,
 * * HDOP value and
 * * VDOP value
 *
 * for a given satellite system. Currently GPS and GLONASS are supported, Galileo
 * is not yet supported.
 *
 * The GPS and GLONASS satellites have unique numbers and the satellite list gives
 * the numbers of the satellites that are currently utilized. The number 0 is not
 * a valid satellite number and can be ignored in the list.
 */
int tf_gps_v3_get_satellite_system_status_low_level(TF_GPSV3 *gps_v3, uint8_t satellite_system, uint8_t *ret_satellite_numbers_length, uint8_t ret_satellite_numbers_data[12], uint8_t *ret_fix, uint16_t *ret_pdop, uint16_t *ret_hdop, uint16_t *ret_vdop);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the current elevation, azimuth and SNR
 * for a given satellite and satellite system.
 *
 * The satellite number here always goes from 1 to 32. For GLONASS it corresponds to
 * the satellites 65-96.
 *
 * Galileo is not yet supported.
 */
int tf_gps_v3_get_satellite_status(TF_GPSV3 *gps_v3, uint8_t satellite_system, uint8_t satellite_number, int16_t *ret_elevation, int16_t *ret_azimuth, int16_t *ret_snr);

/**
 * \ingroup TF_GPSV3
 *
 * Sets the fix LED configuration. By default the LED shows if
 * the Bricklet got a GPS fix yet. If a fix is established the LED turns on.
 * If there is no fix then the LED is turned off.
 *
 * You can also turn the LED permanently on/off, show a heartbeat or let it blink
 * in sync with the PPS (pulse per second) output of the GPS module.
 *
 * If the Bricklet is in bootloader mode, the LED is off.
 */
int tf_gps_v3_set_fix_led_config(TF_GPSV3 *gps_v3, uint8_t config);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the configuration as set by {@link tf_gps_v3_set_fix_led_config}
 */
int tf_gps_v3_get_fix_led_config(TF_GPSV3 *gps_v3, uint8_t *ret_config);

/**
 * \ingroup TF_GPSV3
 *
 * Sets the period with which the {@link tf_gps_v3_register_coordinates_callback} callback is triggered
 * periodically. A value of 0 turns the callback off.
 *
 * The {@link tf_gps_v3_register_coordinates_callback} callback is only triggered if the coordinates changed
 * since the last triggering.
 */
int tf_gps_v3_set_coordinates_callback_period(TF_GPSV3 *gps_v3, uint32_t period);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the period as set by {@link tf_gps_v3_set_coordinates_callback_period}.
 */
int tf_gps_v3_get_coordinates_callback_period(TF_GPSV3 *gps_v3, uint32_t *ret_period);

/**
 * \ingroup TF_GPSV3
 *
 * Sets the period with which the {@link tf_gps_v3_register_status_callback} callback is triggered
 * periodically. A value of 0 turns the callback off.
 *
 * The {@link tf_gps_v3_register_status_callback} callback is only triggered if the status changed since the
 * last triggering.
 */
int tf_gps_v3_set_status_callback_period(TF_GPSV3 *gps_v3, uint32_t period);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the period as set by {@link tf_gps_v3_set_status_callback_period}.
 */
int tf_gps_v3_get_status_callback_period(TF_GPSV3 *gps_v3, uint32_t *ret_period);

/**
 * \ingroup TF_GPSV3
 *
 * Sets the period with which the {@link tf_gps_v3_register_altitude_callback} callback is triggered
 * periodically. A value of 0 turns the callback off.
 *
 * The {@link tf_gps_v3_register_altitude_callback} callback is only triggered if the altitude changed since the
 * last triggering.
 */
int tf_gps_v3_set_altitude_callback_period(TF_GPSV3 *gps_v3, uint32_t period);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the period as set by {@link tf_gps_v3_set_altitude_callback_period}.
 */
int tf_gps_v3_get_altitude_callback_period(TF_GPSV3 *gps_v3, uint32_t *ret_period);

/**
 * \ingroup TF_GPSV3
 *
 * Sets the period with which the {@link tf_gps_v3_register_motion_callback} callback is triggered
 * periodically. A value of 0 turns the callback off.
 *
 * The {@link tf_gps_v3_register_motion_callback} callback is only triggered if the motion changed since the
 * last triggering.
 */
int tf_gps_v3_set_motion_callback_period(TF_GPSV3 *gps_v3, uint32_t period);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the period as set by {@link tf_gps_v3_set_motion_callback_period}.
 */
int tf_gps_v3_get_motion_callback_period(TF_GPSV3 *gps_v3, uint32_t *ret_period);

/**
 * \ingroup TF_GPSV3
 *
 * Sets the period with which the {@link tf_gps_v3_register_date_time_callback} callback is triggered
 * periodically. A value of 0 turns the callback off.
 *
 * The {@link tf_gps_v3_register_date_time_callback} callback is only triggered if the date or time changed
 * since the last triggering.
 */
int tf_gps_v3_set_date_time_callback_period(TF_GPSV3 *gps_v3, uint32_t period);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the period as set by {@link tf_gps_v3_set_date_time_callback_period}.
 */
int tf_gps_v3_get_date_time_callback_period(TF_GPSV3 *gps_v3, uint32_t *ret_period);

/**
 * \ingroup TF_GPSV3
 *
 * If `SBAS <https://en.wikipedia.org/wiki/GNSS_augmentation#Satellite-based_augmentation_system>`__ is enabled,
 * the position accuracy increases (if SBAS satellites are in view),
 * but the update rate is limited to 5Hz. With SBAS disabled the update rate is increased to 10Hz.
 */
int tf_gps_v3_set_sbas_config(TF_GPSV3 *gps_v3, uint8_t sbas_config);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the SBAS configuration as set by {@link tf_gps_v3_set_sbas_config}
 */
int tf_gps_v3_get_sbas_config(TF_GPSV3 *gps_v3, uint8_t *ret_sbas_config);

/**
 * \ingroup TF_GPSV3
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
int tf_gps_v3_get_spitfp_error_count(TF_GPSV3 *gps_v3, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_GPSV3
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
int tf_gps_v3_set_bootloader_mode(TF_GPSV3 *gps_v3, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the current bootloader mode, see {@link tf_gps_v3_set_bootloader_mode}.
 */
int tf_gps_v3_get_bootloader_mode(TF_GPSV3 *gps_v3, uint8_t *ret_mode);

/**
 * \ingroup TF_GPSV3
 *
 * Sets the firmware pointer for {@link tf_gps_v3_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_gps_v3_set_write_firmware_pointer(TF_GPSV3 *gps_v3, uint32_t pointer);

/**
 * \ingroup TF_GPSV3
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_gps_v3_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_gps_v3_write_firmware(TF_GPSV3 *gps_v3, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_GPSV3
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_gps_v3_set_status_led_config(TF_GPSV3 *gps_v3, uint8_t config);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the configuration as set by {@link tf_gps_v3_set_status_led_config}
 */
int tf_gps_v3_get_status_led_config(TF_GPSV3 *gps_v3, uint8_t *ret_config);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_gps_v3_get_chip_temperature(TF_GPSV3 *gps_v3, int16_t *ret_temperature);

/**
 * \ingroup TF_GPSV3
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_gps_v3_reset(TF_GPSV3 *gps_v3);

/**
 * \ingroup TF_GPSV3
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_gps_v3_write_uid(TF_GPSV3 *gps_v3, uint32_t uid);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_gps_v3_read_uid(TF_GPSV3 *gps_v3, uint32_t *ret_uid);

/**
 * \ingroup TF_GPSV3
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
int tf_gps_v3_get_identity(TF_GPSV3 *gps_v3, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

/**
 * \ingroup TF_GPSV3
 *
 * Returns the
 *
 * * satellite numbers list (up to 12 items)
 * * fix value,
 * * PDOP value,
 * * HDOP value and
 * * VDOP value
 *
 * for a given satellite system. Currently GPS and GLONASS are supported, Galileo
 * is not yet supported.
 *
 * The GPS and GLONASS satellites have unique numbers and the satellite list gives
 * the numbers of the satellites that are currently utilized. The number 0 is not
 * a valid satellite number and can be ignored in the list.
 */
int tf_gps_v3_get_satellite_system_status(TF_GPSV3 *gps_v3, uint8_t satellite_system, uint8_t *ret_satellite_numbers, uint8_t *ret_satellite_numbers_length, uint8_t *ret_fix, uint16_t *ret_pdop, uint16_t *ret_hdop, uint16_t *ret_vdop);

#ifdef __cplusplus
}
#endif

#endif
