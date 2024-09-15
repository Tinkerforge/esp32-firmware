/* ***********************************************************
 * This file was automatically generated on 2024-09-15.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_WARP_FRONT_PANEL_H
#define TF_WARP_FRONT_PANEL_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_WARPFrontPanel WARP Front Panel Bricklet
 */

struct TF_WARPFrontPanel;
#if TF_IMPLEMENT_CALLBACKS != 0


#endif
/**
 * \ingroup TF_WARPFrontPanel
 *
 * TBD
 */
typedef struct TF_WARPFrontPanel {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0

#endif
    uint16_t magic;
    uint8_t response_expected[2];
} TF_WARPFrontPanel;

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_SET_FLASH_INDEX 1

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_FLASH_INDEX 2

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_SET_FLASH_DATA 3

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_ERASE_FLASH_SECTOR 4

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_ERASE_FLASH 5

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_SET_STATUS_BAR 6

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_STATUS_BAR 7

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_PAGE_INDEX 8

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_DISPLAY_PAGE_INDEX 9

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_FRONT_PAGE_ICON 10

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_DISPLAY_FRONT_PAGE_ICON 11

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_WIFI_SETUP_1 12

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_DISPLAY_WIFI_SETUP_1 13

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_WIFI_SETUP_2 14

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_DISPLAY_WIFI_SETUP_2 15

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_SET_LED_STATE 16

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_LED_STATE 17

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY 18

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_DISPLAY 19

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_FLASH_METADATA 20

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_RESET 243

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_READ_UID 249

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

#endif

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FLASH_STATUS_OK 0

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_FLASH_STATUS_BUSY 1

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_LED_PATTERN_OFF 0

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_LED_PATTERN_ON 1

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_LED_PATTERN_BLINKING 2

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_LED_PATTERN_BREATHING 3

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_LED_COLOR_GREEN 0

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_LED_COLOR_RED 1

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_LED_COLOR_YELLOW 2

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_DISPLAY_OFF 0

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_DISPLAY_AUTOMATIC 1

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_WARPFrontPanel
 */
#define TF_WARP_FRONT_PANEL_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_WARPFrontPanel
 *
 * This constant is used to identify a WARP Front Panel Bricklet.
 *
 * The {@link warp_front_panel_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_WARP_FRONT_PANEL_DEVICE_IDENTIFIER 2179

/**
 * \ingroup TF_WARPFrontPanel
 *
 * This constant represents the display name of a WARP Front Panel Bricklet.
 */
#define TF_WARP_FRONT_PANEL_DEVICE_DISPLAY_NAME "WARP Front Panel Bricklet"

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Creates the device object \c warp_front_panel with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_warp_front_panel_create(TF_WARPFrontPanel *warp_front_panel, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Removes the device object \c warp_front_panel from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_warp_front_panel_destroy(TF_WARPFrontPanel *warp_front_panel);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_warp_front_panel_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_warp_front_panel_get_response_expected(TF_WARPFrontPanel *warp_front_panel, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_WARPFrontPanel
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
int tf_warp_front_panel_set_response_expected(TF_WARPFrontPanel *warp_front_panel, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_warp_front_panel_set_response_expected_all(TF_WARPFrontPanel *warp_front_panel, bool response_expected);


#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_WARPFrontPanel
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_warp_front_panel_callback_tick(TF_WARPFrontPanel *warp_front_panel, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_set_flash_index(TF_WARPFrontPanel *warp_front_panel, uint32_t page_index, uint8_t sub_page_index);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_get_flash_index(TF_WARPFrontPanel *warp_front_panel, uint32_t *ret_page_index, uint8_t *ret_sub_page_index);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_set_flash_data(TF_WARPFrontPanel *warp_front_panel, const uint8_t data[64], uint32_t *ret_next_page_index, uint8_t *ret_next_sub_page_index, uint8_t *ret_status);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_erase_flash_sector(TF_WARPFrontPanel *warp_front_panel, uint16_t sector_index, uint8_t *ret_status);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_erase_flash(TF_WARPFrontPanel *warp_front_panel, uint8_t *ret_status);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_set_status_bar(TF_WARPFrontPanel *warp_front_panel, uint32_t ethernet_status, uint32_t wifi_status, uint8_t hours, uint8_t minutes, uint8_t seconds);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_get_status_bar(TF_WARPFrontPanel *warp_front_panel, uint32_t *ret_ethernet_status, uint32_t *ret_wifi_status, uint8_t *ret_hours, uint8_t *ret_minutes, uint8_t *ret_seconds);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_set_display_page_index(TF_WARPFrontPanel *warp_front_panel, uint32_t page_index);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_get_display_page_index(TF_WARPFrontPanel *warp_front_panel, uint32_t *ret_page_index);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_set_display_front_page_icon(TF_WARPFrontPanel *warp_front_panel, uint32_t icon_index, bool active, uint32_t sprite_index, const char text_1[6], uint8_t font_index_1, const char text_2[6], uint8_t font_index_2);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_get_display_front_page_icon(TF_WARPFrontPanel *warp_front_panel, uint32_t icon_index, bool *ret_active, uint32_t *ret_sprite_index, char ret_text_1[10], uint8_t *ret_font_index_1, char ret_text_2[10], uint8_t *ret_font_index_2);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_set_display_wifi_setup_1(TF_WARPFrontPanel *warp_front_panel, const char ip_address[15], const char ssid[49]);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_get_display_wifi_setup_1(TF_WARPFrontPanel *warp_front_panel, char ret_ip_address[15], char ret_ssid[49]);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_set_display_wifi_setup_2(TF_WARPFrontPanel *warp_front_panel, const char password[64]);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * TODO
 */
int tf_warp_front_panel_get_display_wifi_setup_2(TF_WARPFrontPanel *warp_front_panel, char ret_password[64]);

/**
 * \ingroup TF_WARPFrontPanel
 *
 *
 */
int tf_warp_front_panel_set_led_state(TF_WARPFrontPanel *warp_front_panel, uint8_t pattern, uint8_t color);

/**
 * \ingroup TF_WARPFrontPanel
 *
 *
 */
int tf_warp_front_panel_get_led_state(TF_WARPFrontPanel *warp_front_panel, uint8_t *ret_pattern, uint8_t *ret_color);

/**
 * \ingroup TF_WARPFrontPanel
 *
 *
 */
int tf_warp_front_panel_set_display(TF_WARPFrontPanel *warp_front_panel, uint8_t display);

/**
 * \ingroup TF_WARPFrontPanel
 *
 *
 */
int tf_warp_front_panel_get_display(TF_WARPFrontPanel *warp_front_panel, uint8_t *ret_display, uint32_t *ret_countdown);

/**
 * \ingroup TF_WARPFrontPanel
 *
 *
 */
int tf_warp_front_panel_get_flash_metadata(TF_WARPFrontPanel *warp_front_panel, uint32_t *ret_version_flash, uint32_t *ret_version_expected, uint32_t *ret_length_flash, uint32_t *ret_length_expected, uint32_t *ret_checksum_flash, uint32_t *ret_checksum_expected);

/**
 * \ingroup TF_WARPFrontPanel
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
int tf_warp_front_panel_get_spitfp_error_count(TF_WARPFrontPanel *warp_front_panel, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_WARPFrontPanel
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
int tf_warp_front_panel_set_bootloader_mode(TF_WARPFrontPanel *warp_front_panel, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Returns the current bootloader mode, see {@link tf_warp_front_panel_set_bootloader_mode}.
 */
int tf_warp_front_panel_get_bootloader_mode(TF_WARPFrontPanel *warp_front_panel, uint8_t *ret_mode);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Sets the firmware pointer for {@link tf_warp_front_panel_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_warp_front_panel_set_write_firmware_pointer(TF_WARPFrontPanel *warp_front_panel, uint32_t pointer);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_warp_front_panel_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_warp_front_panel_write_firmware(TF_WARPFrontPanel *warp_front_panel, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_warp_front_panel_set_status_led_config(TF_WARPFrontPanel *warp_front_panel, uint8_t config);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Returns the configuration as set by {@link tf_warp_front_panel_set_status_led_config}
 */
int tf_warp_front_panel_get_status_led_config(TF_WARPFrontPanel *warp_front_panel, uint8_t *ret_config);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_warp_front_panel_get_chip_temperature(TF_WARPFrontPanel *warp_front_panel, int16_t *ret_temperature);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_warp_front_panel_reset(TF_WARPFrontPanel *warp_front_panel);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_warp_front_panel_write_uid(TF_WARPFrontPanel *warp_front_panel, uint32_t uid);

/**
 * \ingroup TF_WARPFrontPanel
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_warp_front_panel_read_uid(TF_WARPFrontPanel *warp_front_panel, uint32_t *ret_uid);

/**
 * \ingroup TF_WARPFrontPanel
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
int tf_warp_front_panel_get_identity(TF_WARPFrontPanel *warp_front_panel, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
