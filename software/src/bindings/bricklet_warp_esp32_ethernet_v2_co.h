/* ***********************************************************
 * This file was automatically generated on 2025-07-10.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_WARP_ESP32_ETHERNET_V2_CO_H
#define TF_WARP_ESP32_ETHERNET_V2_CO_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_WARPESP32EthernetV2Co WARP ESP32 Ethernet 2.0 Co Bricklet
 */

struct TF_WARPESP32EthernetV2Co;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_WARPESP32EthernetV2Co_RMMIInterruptHandler)(struct TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, void *user_data);

#endif
/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * TBD
 */
typedef struct TF_WARPESP32EthernetV2Co {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_WARPESP32EthernetV2Co_RMMIInterruptHandler rmmi_interrupt_handler;
    void *rmmi_interrupt_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[1];
} TF_WARPESP32EthernetV2Co;

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_LED 1

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_LED 2

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_TEMPERATURE 3

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_DATE_TIME 4

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_DATE_TIME 5

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_UPTIME 6

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_FORMAT_SD 7

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_SD_INFORMATION 8

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_RESET 243

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_READ_UID 249

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_CALLBACK_RMMI_INTERRUPT 9

#endif

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_DATA_STATUS_OK 0

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_DATA_STATUS_SD_ERROR 1

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_DATA_STATUS_LFS_ERROR 2

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_DATA_STATUS_QUEUE_FULL 3

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_DATA_STATUS_DATE_OUT_OF_RANGE 4

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FORMAT_STATUS_OK 0

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FORMAT_STATUS_PASSWORD_ERROR 1

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_FORMAT_STATUS_FORMAT_ERROR 2

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_LED_STATE_OFF 0

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_LED_STATE_ON 1

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_LED_STATE_AUTO 2

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * This constant is used to identify a WARP ESP32 Ethernet 2.0 Co Bricklet.
 *
 * The {@link warp_esp32_ethernet_v2_co_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_DEVICE_IDENTIFIER 2184

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * This constant represents the display name of a WARP ESP32 Ethernet 2.0 Co Bricklet.
 */
#define TF_WARP_ESP32_ETHERNET_V2_CO_DEVICE_DISPLAY_NAME "WARP ESP32 Ethernet 2.0 Co Bricklet"

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Creates the device object \c warp_esp32_ethernet_v2_co with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_warp_esp32_ethernet_v2_co_create(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Removes the device object \c warp_esp32_ethernet_v2_co from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_warp_esp32_ethernet_v2_co_destroy(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_warp_esp32_ethernet_v2_co_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_warp_esp32_ethernet_v2_co_get_response_expected(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
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
int tf_warp_esp32_ethernet_v2_co_set_response_expected(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_warp_esp32_ethernet_v2_co_set_response_expected_all(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Registers the given \c handler to the RMMI Interrupt callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(void *user_data) \endcode
 *
 * TODO
 */
int tf_warp_esp32_ethernet_v2_co_register_rmmi_interrupt_callback(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, TF_WARPESP32EthernetV2Co_RMMIInterruptHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_warp_esp32_ethernet_v2_co_callback_tick(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * TODO
 */
int tf_warp_esp32_ethernet_v2_co_set_led(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t state);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * TODO
 */
int tf_warp_esp32_ethernet_v2_co_get_led(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t *ret_state);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * TODO
 */
int tf_warp_esp32_ethernet_v2_co_get_temperature(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, int16_t *ret_temperature);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * TODO
 */
int tf_warp_esp32_ethernet_v2_co_set_date_time(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t days, uint8_t days_of_week, uint8_t month, uint16_t year);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * TODO
 */
int tf_warp_esp32_ethernet_v2_co_get_date_time(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t *ret_seconds, uint8_t *ret_minutes, uint8_t *ret_hours, uint8_t *ret_days, uint8_t *ret_days_of_week, uint8_t *ret_month, uint16_t *ret_year);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * TODO
 */
int tf_warp_esp32_ethernet_v2_co_get_uptime(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t *ret_uptime);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * TODO
 */
int tf_warp_esp32_ethernet_v2_co_format_sd(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t password, uint8_t *ret_format_status);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * TODO
 */
int tf_warp_esp32_ethernet_v2_co_get_sd_information(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t *ret_sd_status, uint32_t *ret_lfs_status, uint16_t *ret_sector_size, uint32_t *ret_sector_count, uint32_t *ret_card_type, uint8_t *ret_product_rev, char ret_product_name[5], uint8_t *ret_manufacturer_id);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
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
int tf_warp_esp32_ethernet_v2_co_get_spitfp_error_count(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
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
int tf_warp_esp32_ethernet_v2_co_set_bootloader_mode(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Returns the current bootloader mode, see {@link tf_warp_esp32_ethernet_v2_co_set_bootloader_mode}.
 */
int tf_warp_esp32_ethernet_v2_co_get_bootloader_mode(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t *ret_mode);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Sets the firmware pointer for {@link tf_warp_esp32_ethernet_v2_co_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_warp_esp32_ethernet_v2_co_set_write_firmware_pointer(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t pointer);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_warp_esp32_ethernet_v2_co_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_warp_esp32_ethernet_v2_co_write_firmware(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_warp_esp32_ethernet_v2_co_set_status_led_config(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t config);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Returns the configuration as set by {@link tf_warp_esp32_ethernet_v2_co_set_status_led_config}
 */
int tf_warp_esp32_ethernet_v2_co_get_status_led_config(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t *ret_config);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_warp_esp32_ethernet_v2_co_get_chip_temperature(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, int16_t *ret_temperature);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_warp_esp32_ethernet_v2_co_reset(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_warp_esp32_ethernet_v2_co_write_uid(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t uid);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_warp_esp32_ethernet_v2_co_read_uid(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t *ret_uid);

/**
 * \ingroup TF_WARPESP32EthernetV2Co
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
int tf_warp_esp32_ethernet_v2_co_get_identity(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
