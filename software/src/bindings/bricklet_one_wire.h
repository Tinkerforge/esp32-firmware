/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_ONE_WIRE_H
#define TF_ONE_WIRE_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_OneWire One Wire Bricklet
 */

struct TF_OneWire;
#if TF_IMPLEMENT_CALLBACKS != 0


#endif
/**
 * \ingroup TF_OneWire
 *
 * Communicates with up 64 1-Wire devices
 */
typedef struct TF_OneWire {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0

#endif
    uint16_t magic;
    uint8_t response_expected[1];
} TF_OneWire;

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_SEARCH_BUS_LOW_LEVEL 1

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_RESET_BUS 2

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_WRITE 3

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_READ 4

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_WRITE_COMMAND 5

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_SET_COMMUNICATION_LED_CONFIG 6

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_GET_COMMUNICATION_LED_CONFIG 7

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_RESET 243

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_READ_UID 249

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

#endif

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_STATUS_OK 0

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_STATUS_BUSY 1

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_STATUS_NO_PRESENCE 2

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_STATUS_TIMEOUT 3

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_STATUS_ERROR 4

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_COMMUNICATION_LED_CONFIG_OFF 0

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_COMMUNICATION_LED_CONFIG_ON 1

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_COMMUNICATION_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_COMMUNICATION_LED_CONFIG_SHOW_COMMUNICATION 3

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_OneWire
 */
#define TF_ONE_WIRE_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_OneWire
 *
 * This constant is used to identify a One Wire Bricklet.
 *
 * The {@link one_wire_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_ONE_WIRE_DEVICE_IDENTIFIER 2123

/**
 * \ingroup TF_OneWire
 *
 * This constant represents the display name of a One Wire Bricklet.
 */
#define TF_ONE_WIRE_DEVICE_DISPLAY_NAME "One Wire Bricklet"

/**
 * \ingroup TF_OneWire
 *
 * Creates the device object \c one_wire with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_one_wire_create(TF_OneWire *one_wire, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_OneWire
 *
 * Removes the device object \c one_wire from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_one_wire_destroy(TF_OneWire *one_wire);

/**
 * \ingroup TF_OneWire
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_one_wire_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_one_wire_get_response_expected(TF_OneWire *one_wire, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_OneWire
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
int tf_one_wire_set_response_expected(TF_OneWire *one_wire, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_OneWire
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_one_wire_set_response_expected_all(TF_OneWire *one_wire, bool response_expected);


#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_OneWire
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_one_wire_callback_tick(TF_OneWire *one_wire, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_OneWire
 *
 * Returns a list of up to 64 identifiers of the connected 1-Wire devices.
 * Each identifier is 64-bit and consists of 8-bit family code, 48-bit ID and
 * 8-bit CRC.
 *
 * To get these identifiers the Bricklet runs the
 * `SEARCH ROM algorithm <https://www.maximintegrated.com/en/app-notes/index.mvp/id/187>`__,
 * as defined by Maxim.
 */
int tf_one_wire_search_bus_low_level(TF_OneWire *one_wire, uint16_t *ret_identifier_length, uint16_t *ret_identifier_chunk_offset, uint64_t ret_identifier_chunk_data[7], uint8_t *ret_status);

/**
 * \ingroup TF_OneWire
 *
 * Resets the bus with the 1-Wire reset operation.
 */
int tf_one_wire_reset_bus(TF_OneWire *one_wire, uint8_t *ret_status);

/**
 * \ingroup TF_OneWire
 *
 * Writes a byte of data to the 1-Wire bus.
 */
int tf_one_wire_write(TF_OneWire *one_wire, uint8_t data, uint8_t *ret_status);

/**
 * \ingroup TF_OneWire
 *
 * Reads a byte of data from the 1-Wire bus.
 */
int tf_one_wire_read(TF_OneWire *one_wire, uint8_t *ret_data, uint8_t *ret_status);

/**
 * \ingroup TF_OneWire
 *
 * Writes a command to the 1-Wire device with the given identifier. You can obtain
 * the identifier by calling {@link tf_one_wire_search_bus}. The MATCH ROM operation is used to
 * write the command.
 *
 * If you only have one device connected or want to broadcast to all devices
 * you can set the identifier to 0. In this case the SKIP ROM operation is used to
 * write the command.
 */
int tf_one_wire_write_command(TF_OneWire *one_wire, uint64_t identifier, uint8_t command, uint8_t *ret_status);

/**
 * \ingroup TF_OneWire
 *
 * Sets the communication LED configuration. By default the LED shows 1-wire
 * communication traffic by flickering.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is off.
 */
int tf_one_wire_set_communication_led_config(TF_OneWire *one_wire, uint8_t config);

/**
 * \ingroup TF_OneWire
 *
 * Returns the configuration as set by {@link tf_one_wire_set_communication_led_config}
 */
int tf_one_wire_get_communication_led_config(TF_OneWire *one_wire, uint8_t *ret_config);

/**
 * \ingroup TF_OneWire
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
int tf_one_wire_get_spitfp_error_count(TF_OneWire *one_wire, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_OneWire
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
int tf_one_wire_set_bootloader_mode(TF_OneWire *one_wire, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_OneWire
 *
 * Returns the current bootloader mode, see {@link tf_one_wire_set_bootloader_mode}.
 */
int tf_one_wire_get_bootloader_mode(TF_OneWire *one_wire, uint8_t *ret_mode);

/**
 * \ingroup TF_OneWire
 *
 * Sets the firmware pointer for {@link tf_one_wire_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_one_wire_set_write_firmware_pointer(TF_OneWire *one_wire, uint32_t pointer);

/**
 * \ingroup TF_OneWire
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_one_wire_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_one_wire_write_firmware(TF_OneWire *one_wire, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_OneWire
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_one_wire_set_status_led_config(TF_OneWire *one_wire, uint8_t config);

/**
 * \ingroup TF_OneWire
 *
 * Returns the configuration as set by {@link tf_one_wire_set_status_led_config}
 */
int tf_one_wire_get_status_led_config(TF_OneWire *one_wire, uint8_t *ret_config);

/**
 * \ingroup TF_OneWire
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_one_wire_get_chip_temperature(TF_OneWire *one_wire, int16_t *ret_temperature);

/**
 * \ingroup TF_OneWire
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_one_wire_reset(TF_OneWire *one_wire);

/**
 * \ingroup TF_OneWire
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_one_wire_write_uid(TF_OneWire *one_wire, uint32_t uid);

/**
 * \ingroup TF_OneWire
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_one_wire_read_uid(TF_OneWire *one_wire, uint32_t *ret_uid);

/**
 * \ingroup TF_OneWire
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
int tf_one_wire_get_identity(TF_OneWire *one_wire, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

/**
 * \ingroup TF_OneWire
 *
 * Returns a list of up to 64 identifiers of the connected 1-Wire devices.
 * Each identifier is 64-bit and consists of 8-bit family code, 48-bit ID and
 * 8-bit CRC.
 *
 * To get these identifiers the Bricklet runs the
 * `SEARCH ROM algorithm <https://www.maximintegrated.com/en/app-notes/index.mvp/id/187>`__,
 * as defined by Maxim.
 */
int tf_one_wire_search_bus(TF_OneWire *one_wire, uint64_t *ret_identifier, uint16_t *ret_identifier_length, uint8_t *ret_status);

#ifdef __cplusplus
}
#endif

#endif
