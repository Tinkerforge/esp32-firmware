/* ***********************************************************
 * This file was automatically generated on 2024-09-06.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_WARP_ENERGY_MANAGER_H
#define TF_WARP_ENERGY_MANAGER_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_WARPEnergyManager WARP Energy Manager Bricklet
 */

struct TF_WARPEnergyManager;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_WARPEnergyManager_SDWallboxDataPointsLowLevelHandler)(struct TF_WARPEnergyManager *warp_energy_manager, uint16_t data_length, uint16_t data_chunk_offset, uint8_t data_chunk_data[60], void *user_data);
typedef void (*TF_WARPEnergyManager_SDWallboxDataPointsHandler)(struct TF_WARPEnergyManager *warp_energy_manager, uint8_t *data, uint16_t data_length, void *user_data);
typedef void (*TF_WARPEnergyManager_SDWallboxDailyDataPointsLowLevelHandler)(struct TF_WARPEnergyManager *warp_energy_manager, uint16_t data_length, uint16_t data_chunk_offset, uint32_t data_chunk_data[15], void *user_data);
typedef void (*TF_WARPEnergyManager_SDWallboxDailyDataPointsHandler)(struct TF_WARPEnergyManager *warp_energy_manager, uint32_t *data, uint16_t data_length, void *user_data);
typedef void (*TF_WARPEnergyManager_SDEnergyManagerDataPointsLowLevelHandler)(struct TF_WARPEnergyManager *warp_energy_manager, uint16_t data_length, uint16_t data_chunk_offset, uint8_t data_chunk_data[33], void *user_data);
typedef void (*TF_WARPEnergyManager_SDEnergyManagerDataPointsHandler)(struct TF_WARPEnergyManager *warp_energy_manager, uint8_t *data, uint16_t data_length, void *user_data);
typedef void (*TF_WARPEnergyManager_SDEnergyManagerDailyDataPointsLowLevelHandler)(struct TF_WARPEnergyManager *warp_energy_manager, uint16_t data_length, uint16_t data_chunk_offset, uint32_t data_chunk_data[15], void *user_data);
typedef void (*TF_WARPEnergyManager_SDEnergyManagerDailyDataPointsHandler)(struct TF_WARPEnergyManager *warp_energy_manager, uint32_t *data, uint16_t data_length, void *user_data);

#endif
/**
 * \ingroup TF_WARPEnergyManager
 *
 * TBD
 */
typedef struct TF_WARPEnergyManager {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_WARPEnergyManager_SDWallboxDataPointsLowLevelHandler sd_wallbox_data_points_low_level_handler;
    void *sd_wallbox_data_points_low_level_user_data;

    TF_WARPEnergyManager_SDWallboxDailyDataPointsLowLevelHandler sd_wallbox_daily_data_points_low_level_handler;
    void *sd_wallbox_daily_data_points_low_level_user_data;

    TF_WARPEnergyManager_SDEnergyManagerDataPointsLowLevelHandler sd_energy_manager_data_points_low_level_handler;
    void *sd_energy_manager_data_points_low_level_user_data;

    TF_WARPEnergyManager_SDEnergyManagerDailyDataPointsLowLevelHandler sd_energy_manager_daily_data_points_low_level_handler;
    void *sd_energy_manager_daily_data_points_low_level_user_data;

    TF_WARPEnergyManager_SDWallboxDataPointsHandler sd_wallbox_data_points_handler;
    TF_HighLevelCallback sd_wallbox_data_points_hlc;

    TF_WARPEnergyManager_SDWallboxDailyDataPointsHandler sd_wallbox_daily_data_points_handler;
    TF_HighLevelCallback sd_wallbox_daily_data_points_hlc;

    TF_WARPEnergyManager_SDEnergyManagerDataPointsHandler sd_energy_manager_data_points_handler;
    TF_HighLevelCallback sd_energy_manager_data_points_hlc;

    TF_WARPEnergyManager_SDEnergyManagerDailyDataPointsHandler sd_energy_manager_daily_data_points_handler;
    TF_HighLevelCallback sd_energy_manager_daily_data_points_hlc;

#endif
    uint16_t magic;
    uint8_t response_expected[2];
} TF_WARPEnergyManager;

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_CONTACTOR 1

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_CONTACTOR 2

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_RGB_VALUE 3

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_RGB_VALUE 4

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_ENERGY_METER_VALUES 5

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_ENERGY_METER_DETAILED_VALUES_LOW_LEVEL 6

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_ENERGY_METER_STATE 7

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_INPUT 8

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_OUTPUT 9

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_OUTPUT 10

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_INPUT_VOLTAGE 11

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_STATE 12

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_UPTIME 13

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_ALL_DATA_1 14

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_SD_INFORMATION 15

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_SD_WALLBOX_DATA_POINT 16

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_SD_WALLBOX_DATA_POINTS 17

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_SD_WALLBOX_DAILY_DATA_POINT 18

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_SD_WALLBOX_DAILY_DATA_POINTS 19

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_SD_ENERGY_MANAGER_DATA_POINT 20

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_SD_ENERGY_MANAGER_DATA_POINTS 21

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_SD_ENERGY_MANAGER_DAILY_DATA_POINT 22

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_SD_ENERGY_MANAGER_DAILY_DATA_POINTS 23

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_FORMAT_SD 28

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_DATE_TIME 29

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_DATE_TIME 30

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_LED_STATE 31

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_LED_STATE 32

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_DATA_STORAGE 33

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_DATA_STORAGE 34

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY 35

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_RESET 243

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_READ_UID 249

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_CALLBACK_SD_WALLBOX_DATA_POINTS_LOW_LEVEL 24

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_CALLBACK_SD_WALLBOX_DAILY_DATA_POINTS_LOW_LEVEL 25

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_CALLBACK_SD_ENERGY_MANAGER_DATA_POINTS_LOW_LEVEL 26

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_CALLBACK_SD_ENERGY_MANAGER_DAILY_DATA_POINTS_LOW_LEVEL 27

#endif

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_ENERGY_METER_TYPE_NOT_AVAILABLE 0

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_ENERGY_METER_TYPE_SDM72 1

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_ENERGY_METER_TYPE_SDM630 2

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_ENERGY_METER_TYPE_SDM72V2 3

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_ENERGY_METER_TYPE_SDM72CTM 4

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_ENERGY_METER_TYPE_SDM630MCTV2 5

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_ENERGY_METER_TYPE_DSZ15DZMOD 6

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_ENERGY_METER_TYPE_DEM4A 7

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_DATA_STATUS_OK 0

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_DATA_STATUS_SD_ERROR 1

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_DATA_STATUS_LFS_ERROR 2

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_DATA_STATUS_QUEUE_FULL 3

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_DATA_STATUS_DATE_OUT_OF_RANGE 4

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FORMAT_STATUS_OK 0

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FORMAT_STATUS_PASSWORD_ERROR 1

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_FORMAT_STATUS_FORMAT_ERROR 2

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_LED_PATTERN_OFF 0

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_LED_PATTERN_ON 1

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_LED_PATTERN_BLINKING 2

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_LED_PATTERN_BREATHING 3

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_WARPEnergyManager
 */
#define TF_WARP_ENERGY_MANAGER_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_WARPEnergyManager
 *
 * This constant is used to identify a WARP Energy Manager Bricklet.
 *
 * The {@link warp_energy_manager_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_WARP_ENERGY_MANAGER_DEVICE_IDENTIFIER 2169

/**
 * \ingroup TF_WARPEnergyManager
 *
 * This constant represents the display name of a WARP Energy Manager Bricklet.
 */
#define TF_WARP_ENERGY_MANAGER_DEVICE_DISPLAY_NAME "WARP Energy Manager Bricklet"

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Creates the device object \c warp_energy_manager with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_warp_energy_manager_create(TF_WARPEnergyManager *warp_energy_manager, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Removes the device object \c warp_energy_manager from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_warp_energy_manager_destroy(TF_WARPEnergyManager *warp_energy_manager);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_warp_energy_manager_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_warp_energy_manager_get_response_expected(TF_WARPEnergyManager *warp_energy_manager, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_WARPEnergyManager
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
int tf_warp_energy_manager_set_response_expected(TF_WARPEnergyManager *warp_energy_manager, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_warp_energy_manager_set_response_expected_all(TF_WARPEnergyManager *warp_energy_manager, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_WARPEnergyManager
 *
 * Registers the given \c handler to the SD Wallbox Data Points Low Level callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t data_length, uint16_t data_chunk_offset, uint8_t data_chunk_data[60], void *user_data) \endcode
 *
 * TODO
 */
int tf_warp_energy_manager_register_sd_wallbox_data_points_low_level_callback(TF_WARPEnergyManager *warp_energy_manager, TF_WARPEnergyManager_SDWallboxDataPointsLowLevelHandler handler, void *user_data);


/**
 * \ingroup TF_WARPEnergyManager
 *
 * Registers the given \c handler to the SD Wallbox Data Points callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t *data, uint16_t data_length, void *user_data) \endcode
 *
 * TODO
 */
int tf_warp_energy_manager_register_sd_wallbox_data_points_callback(TF_WARPEnergyManager *warp_energy_manager, TF_WARPEnergyManager_SDWallboxDataPointsHandler handler, uint8_t *data, void *user_data);


/**
 * \ingroup TF_WARPEnergyManager
 *
 * Registers the given \c handler to the SD Wallbox Daily Data Points Low Level callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t data_length, uint16_t data_chunk_offset, uint32_t data_chunk_data[15], void *user_data) \endcode
 *
 * TODO
 */
int tf_warp_energy_manager_register_sd_wallbox_daily_data_points_low_level_callback(TF_WARPEnergyManager *warp_energy_manager, TF_WARPEnergyManager_SDWallboxDailyDataPointsLowLevelHandler handler, void *user_data);


/**
 * \ingroup TF_WARPEnergyManager
 *
 * Registers the given \c handler to the SD Wallbox Daily Data Points callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint32_t *data, uint16_t data_length, void *user_data) \endcode
 *
 * TODO
 */
int tf_warp_energy_manager_register_sd_wallbox_daily_data_points_callback(TF_WARPEnergyManager *warp_energy_manager, TF_WARPEnergyManager_SDWallboxDailyDataPointsHandler handler, uint32_t *data, void *user_data);


/**
 * \ingroup TF_WARPEnergyManager
 *
 * Registers the given \c handler to the SD Energy Manager Data Points Low Level callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t data_length, uint16_t data_chunk_offset, uint8_t data_chunk_data[33], void *user_data) \endcode
 *
 * TODO
 */
int tf_warp_energy_manager_register_sd_energy_manager_data_points_low_level_callback(TF_WARPEnergyManager *warp_energy_manager, TF_WARPEnergyManager_SDEnergyManagerDataPointsLowLevelHandler handler, void *user_data);


/**
 * \ingroup TF_WARPEnergyManager
 *
 * Registers the given \c handler to the SD Energy Manager Data Points callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t *data, uint16_t data_length, void *user_data) \endcode
 *
 * TODO
 */
int tf_warp_energy_manager_register_sd_energy_manager_data_points_callback(TF_WARPEnergyManager *warp_energy_manager, TF_WARPEnergyManager_SDEnergyManagerDataPointsHandler handler, uint8_t *data, void *user_data);


/**
 * \ingroup TF_WARPEnergyManager
 *
 * Registers the given \c handler to the SD Energy Manager Daily Data Points Low Level callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t data_length, uint16_t data_chunk_offset, uint32_t data_chunk_data[15], void *user_data) \endcode
 *
 * TODO
 */
int tf_warp_energy_manager_register_sd_energy_manager_daily_data_points_low_level_callback(TF_WARPEnergyManager *warp_energy_manager, TF_WARPEnergyManager_SDEnergyManagerDailyDataPointsLowLevelHandler handler, void *user_data);


/**
 * \ingroup TF_WARPEnergyManager
 *
 * Registers the given \c handler to the SD Energy Manager Daily Data Points callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint32_t *data, uint16_t data_length, void *user_data) \endcode
 *
 * TODO
 */
int tf_warp_energy_manager_register_sd_energy_manager_daily_data_points_callback(TF_WARPEnergyManager *warp_energy_manager, TF_WARPEnergyManager_SDEnergyManagerDailyDataPointsHandler handler, uint32_t *data, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_WARPEnergyManager
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_warp_energy_manager_callback_tick(TF_WARPEnergyManager *warp_energy_manager, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TBD
 */
int tf_warp_energy_manager_set_contactor(TF_WARPEnergyManager *warp_energy_manager, bool contactor_value);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TBD
 */
int tf_warp_energy_manager_get_contactor(TF_WARPEnergyManager *warp_energy_manager, bool *ret_contactor_value);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Sets the *r*, *g* and *b* values for the LED.
 */
int tf_warp_energy_manager_set_rgb_value(TF_WARPEnergyManager *warp_energy_manager, uint8_t r, uint8_t g, uint8_t b);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Returns the *r*, *g* and *b* values of the LED as set by {@link tf_warp_energy_manager_set_rgb_value}.
 */
int tf_warp_energy_manager_get_rgb_value(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_r, uint8_t *ret_g, uint8_t *ret_b);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_energy_meter_values(TF_WARPEnergyManager *warp_energy_manager, float *ret_power, float ret_current[3]);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TBD
 */
int tf_warp_energy_manager_get_energy_meter_detailed_values_low_level(TF_WARPEnergyManager *warp_energy_manager, uint16_t *ret_values_chunk_offset, float ret_values_chunk_data[15]);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_energy_meter_state(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_energy_meter_type, uint32_t ret_error_count[6]);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_input(TF_WARPEnergyManager *warp_energy_manager, bool ret_input[2]);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_set_output(TF_WARPEnergyManager *warp_energy_manager, bool output);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_output(TF_WARPEnergyManager *warp_energy_manager, bool *ret_output);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_input_voltage(TF_WARPEnergyManager *warp_energy_manager, uint16_t *ret_voltage);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_state(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_contactor_check_state);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_uptime(TF_WARPEnergyManager *warp_energy_manager, uint32_t *ret_uptime);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_all_data_1(TF_WARPEnergyManager *warp_energy_manager, bool *ret_contactor_value, uint8_t *ret_r, uint8_t *ret_g, uint8_t *ret_b, float *ret_power, float ret_current[3], uint8_t *ret_energy_meter_type, uint32_t ret_error_count[6], bool ret_input[2], bool *ret_output, uint16_t *ret_voltage, uint8_t *ret_contactor_check_state, uint32_t *ret_uptime);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_sd_information(TF_WARPEnergyManager *warp_energy_manager, uint32_t *ret_sd_status, uint32_t *ret_lfs_status, uint16_t *ret_sector_size, uint32_t *ret_sector_count, uint32_t *ret_card_type, uint8_t *ret_product_rev, char ret_product_name[5], uint8_t *ret_manufacturer_id);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_set_sd_wallbox_data_point(TF_WARPEnergyManager *warp_energy_manager, uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t flags, uint16_t power, uint8_t *ret_status);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_sd_wallbox_data_points(TF_WARPEnergyManager *warp_energy_manager, uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_set_sd_wallbox_daily_data_point(TF_WARPEnergyManager *warp_energy_manager, uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint32_t energy, uint8_t *ret_status);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_sd_wallbox_daily_data_points(TF_WARPEnergyManager *warp_energy_manager, uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_set_sd_energy_manager_data_point(TF_WARPEnergyManager *warp_energy_manager, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t flags, int32_t power_grid, const int32_t power_general[6], uint32_t price, uint8_t *ret_status);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_sd_energy_manager_data_points(TF_WARPEnergyManager *warp_energy_manager, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_set_sd_energy_manager_daily_data_point(TF_WARPEnergyManager *warp_energy_manager, uint8_t year, uint8_t month, uint8_t day, uint32_t energy_grid_in, uint32_t energy_grid_out, const uint32_t energy_general_in[6], const uint32_t energy_general_out[6], uint32_t price, uint8_t *ret_status);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_sd_energy_manager_daily_data_points(TF_WARPEnergyManager *warp_energy_manager, uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_format_sd(TF_WARPEnergyManager *warp_energy_manager, uint32_t password, uint8_t *ret_format_status);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_set_date_time(TF_WARPEnergyManager *warp_energy_manager, uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t days, uint8_t days_of_week, uint8_t month, uint16_t year);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_date_time(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_seconds, uint8_t *ret_minutes, uint8_t *ret_hours, uint8_t *ret_days, uint8_t *ret_days_of_week, uint8_t *ret_month, uint16_t *ret_year);

/**
 * \ingroup TF_WARPEnergyManager
 *
 *
 */
int tf_warp_energy_manager_set_led_state(TF_WARPEnergyManager *warp_energy_manager, uint8_t pattern, uint16_t hue);

/**
 * \ingroup TF_WARPEnergyManager
 *
 *
 */
int tf_warp_energy_manager_get_led_state(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_pattern, uint16_t *ret_hue);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_get_data_storage(TF_WARPEnergyManager *warp_energy_manager, uint8_t page, uint8_t ret_data[63]);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_set_data_storage(TF_WARPEnergyManager *warp_energy_manager, uint8_t page, const uint8_t data[63]);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TODO
 */
int tf_warp_energy_manager_reset_energy_meter_relative_energy(TF_WARPEnergyManager *warp_energy_manager);

/**
 * \ingroup TF_WARPEnergyManager
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
int tf_warp_energy_manager_get_spitfp_error_count(TF_WARPEnergyManager *warp_energy_manager, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_WARPEnergyManager
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
int tf_warp_energy_manager_set_bootloader_mode(TF_WARPEnergyManager *warp_energy_manager, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Returns the current bootloader mode, see {@link tf_warp_energy_manager_set_bootloader_mode}.
 */
int tf_warp_energy_manager_get_bootloader_mode(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_mode);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Sets the firmware pointer for {@link tf_warp_energy_manager_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_warp_energy_manager_set_write_firmware_pointer(TF_WARPEnergyManager *warp_energy_manager, uint32_t pointer);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_warp_energy_manager_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_warp_energy_manager_write_firmware(TF_WARPEnergyManager *warp_energy_manager, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_warp_energy_manager_set_status_led_config(TF_WARPEnergyManager *warp_energy_manager, uint8_t config);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Returns the configuration as set by {@link tf_warp_energy_manager_set_status_led_config}
 */
int tf_warp_energy_manager_get_status_led_config(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_config);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_warp_energy_manager_get_chip_temperature(TF_WARPEnergyManager *warp_energy_manager, int16_t *ret_temperature);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_warp_energy_manager_reset(TF_WARPEnergyManager *warp_energy_manager);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_warp_energy_manager_write_uid(TF_WARPEnergyManager *warp_energy_manager, uint32_t uid);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_warp_energy_manager_read_uid(TF_WARPEnergyManager *warp_energy_manager, uint32_t *ret_uid);

/**
 * \ingroup TF_WARPEnergyManager
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
int tf_warp_energy_manager_get_identity(TF_WARPEnergyManager *warp_energy_manager, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

/**
 * \ingroup TF_WARPEnergyManager
 *
 * TBD
 */
int tf_warp_energy_manager_get_energy_meter_detailed_values(TF_WARPEnergyManager *warp_energy_manager, float *ret_values, uint16_t *ret_values_length);

#ifdef __cplusplus
}
#endif

#endif
