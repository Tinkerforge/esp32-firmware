/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_DMX_H
#define TF_DMX_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_DMX DMX Bricklet
 */

struct TF_DMX;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_DMX_FrameStartedHandler)(struct TF_DMX *dmx, void *user_data);
typedef void (*TF_DMX_FrameAvailableHandler)(struct TF_DMX *dmx, uint32_t frame_number, void *user_data);
typedef void (*TF_DMX_FrameLowLevelHandler)(struct TF_DMX *dmx, uint16_t frame_length, uint16_t frame_chunk_offset, uint8_t frame_chunk_data[56], uint32_t frame_number, void *user_data);
typedef void (*TF_DMX_FrameHandler)(struct TF_DMX *dmx, uint8_t *frame, uint16_t frame_length, uint32_t frame_number, void *user_data);
typedef void (*TF_DMX_FrameErrorCountHandler)(struct TF_DMX *dmx, uint32_t overrun_error_count, uint32_t framing_error_count, void *user_data);

#endif
/**
 * \ingroup TF_DMX
 *
 * DMX master and slave
 */
typedef struct TF_DMX {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_DMX_FrameStartedHandler frame_started_handler;
    void *frame_started_user_data;

    TF_DMX_FrameAvailableHandler frame_available_handler;
    void *frame_available_user_data;

    TF_DMX_FrameLowLevelHandler frame_low_level_handler;
    void *frame_low_level_user_data;

    TF_DMX_FrameErrorCountHandler frame_error_count_handler;
    void *frame_error_count_user_data;

    TF_DMX_FrameHandler frame_handler;
    TF_HighLevelCallback frame_hlc;

#endif
    uint16_t magic;
    uint8_t response_expected[2];
} TF_DMX;

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_SET_DMX_MODE 1

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_GET_DMX_MODE 2

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_WRITE_FRAME_LOW_LEVEL 3

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_READ_FRAME_LOW_LEVEL 4

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_SET_FRAME_DURATION 5

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_GET_FRAME_DURATION 6

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_GET_FRAME_ERROR_COUNT 7

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_SET_COMMUNICATION_LED_CONFIG 8

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_GET_COMMUNICATION_LED_CONFIG 9

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_SET_ERROR_LED_CONFIG 10

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_GET_ERROR_LED_CONFIG 11

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_SET_FRAME_CALLBACK_CONFIG 12

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_GET_FRAME_CALLBACK_CONFIG 13

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_RESET 243

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_READ_UID 249

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_CALLBACK_FRAME_STARTED 14

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_CALLBACK_FRAME_AVAILABLE 15

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_CALLBACK_FRAME_LOW_LEVEL 16

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_CALLBACK_FRAME_ERROR_COUNT 17

#endif

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_DMX_MODE_MASTER 0

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_DMX_MODE_SLAVE 1

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_COMMUNICATION_LED_CONFIG_OFF 0

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_COMMUNICATION_LED_CONFIG_ON 1

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_COMMUNICATION_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_COMMUNICATION_LED_CONFIG_SHOW_COMMUNICATION 3

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_ERROR_LED_CONFIG_OFF 0

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_ERROR_LED_CONFIG_ON 1

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_ERROR_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_ERROR_LED_CONFIG_SHOW_ERROR 3

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_DMX
 */
#define TF_DMX_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_DMX
 *
 * This constant is used to identify a DMX Bricklet.
 *
 * The {@link dmx_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_DMX_DEVICE_IDENTIFIER 285

/**
 * \ingroup TF_DMX
 *
 * This constant represents the display name of a DMX Bricklet.
 */
#define TF_DMX_DEVICE_DISPLAY_NAME "DMX Bricklet"

/**
 * \ingroup TF_DMX
 *
 * Creates the device object \c dmx with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_dmx_create(TF_DMX *dmx, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_DMX
 *
 * Removes the device object \c dmx from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_dmx_destroy(TF_DMX *dmx);

/**
 * \ingroup TF_DMX
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_dmx_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_dmx_get_response_expected(TF_DMX *dmx, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_DMX
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
int tf_dmx_set_response_expected(TF_DMX *dmx, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_DMX
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_dmx_set_response_expected_all(TF_DMX *dmx, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_DMX
 *
 * Registers the given \c handler to the Frame Started callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(void *user_data) \endcode
 *
 * This callback is triggered as soon as a new frame write is started.
 * You should send the data for the next frame directly after this callback
 * was triggered.
 *
 * For an explanation of the general approach see {@link tf_dmx_write_frame}.
 *
 * This callback can be enabled via {@link tf_dmx_set_frame_callback_config}.
 *
 * This callback can only be triggered in master mode.
 */
int tf_dmx_register_frame_started_callback(TF_DMX *dmx, TF_DMX_FrameStartedHandler handler, void *user_data);


/**
 * \ingroup TF_DMX
 *
 * Registers the given \c handler to the Frame Available callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint32_t frame_number, void *user_data) \endcode
 *
 * This callback is triggered in slave mode when a new frame was received from the DMX master
 * and it can be read out. You have to read the frame before the master has written
 * the next frame, see {@link tf_dmx_read_frame} for more details.
 *
 * The parameter is the frame number, it is increased by one with each received frame.
 *
 * This callback can be enabled via {@link tf_dmx_set_frame_callback_config}.
 *
 * This callback can only be triggered in slave mode.
 */
int tf_dmx_register_frame_available_callback(TF_DMX *dmx, TF_DMX_FrameAvailableHandler handler, void *user_data);


/**
 * \ingroup TF_DMX
 *
 * Registers the given \c handler to the Frame Low Level callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint16_t frame_length, uint16_t frame_chunk_offset, uint8_t frame_chunk_data[56], uint32_t frame_number, void *user_data) \endcode
 *
 * This callback is called as soon as a new frame is available
 * (written by the DMX master).
 *
 * The size of the array is equivalent to the number of channels in
 * the frame. Each byte represents one channel.
 *
 * This callback can be enabled via {@link tf_dmx_set_frame_callback_config}.
 *
 * This callback can only be triggered in slave mode.
 */
int tf_dmx_register_frame_low_level_callback(TF_DMX *dmx, TF_DMX_FrameLowLevelHandler handler, void *user_data);


/**
 * \ingroup TF_DMX
 *
 * Registers the given \c handler to the Frame callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t *frame, uint16_t frame_length, uint32_t frame_number, void *user_data) \endcode
 *
 * This callback is called as soon as a new frame is available
 * (written by the DMX master).
 *
 * The size of the array is equivalent to the number of channels in
 * the frame. Each byte represents one channel.
 *
 * This callback can be enabled via {@link tf_dmx_set_frame_callback_config}.
 *
 * This callback can only be triggered in slave mode.
 */
int tf_dmx_register_frame_callback(TF_DMX *dmx, TF_DMX_FrameHandler handler, uint8_t *frame, void *user_data);


/**
 * \ingroup TF_DMX
 *
 * Registers the given \c handler to the Frame Error Count callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint32_t overrun_error_count, uint32_t framing_error_count, void *user_data) \endcode
 *
 * This callback is called if a new error occurs. It returns
 * the current overrun and framing error count.
 */
int tf_dmx_register_frame_error_count_callback(TF_DMX *dmx, TF_DMX_FrameErrorCountHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_DMX
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_dmx_callback_tick(TF_DMX *dmx, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_DMX
 *
 * Sets the DMX mode to either master or slave.
 *
 * Calling this function sets frame number to 0.
 */
int tf_dmx_set_dmx_mode(TF_DMX *dmx, uint8_t dmx_mode);

/**
 * \ingroup TF_DMX
 *
 * Returns the DMX mode, as set by {@link tf_dmx_set_dmx_mode}.
 */
int tf_dmx_get_dmx_mode(TF_DMX *dmx, uint8_t *ret_dmx_mode);

/**
 * \ingroup TF_DMX
 *
 * Writes a DMX frame. The maximum frame size is 512 byte. Each byte represents one channel.
 *
 * The next frame can be written after the {@link tf_dmx_register_frame_started_callback} callback was called. The frame
 * is double buffered, so a new frame can be written as soon as the writing of the prior frame
 * starts.
 *
 * The data will be transfered when the next frame duration ends, see {@link tf_dmx_set_frame_duration}.
 *
 * Generic approach:
 *
 * * Set the frame duration to a value that represents the number of frames per second you want to achieve.
 * * Set channels for first frame.
 * * Wait for the {@link tf_dmx_register_frame_started_callback} callback.
 * * Set channels for next frame.
 * * Wait for the {@link tf_dmx_register_frame_started_callback} callback.
 * * and so on.
 *
 * This approach ensures that you can set new DMX data with a fixed frame rate.
 *
 * This function can only be called in master mode.
 */
int tf_dmx_write_frame_low_level(TF_DMX *dmx, uint16_t frame_length, uint16_t frame_chunk_offset, const uint8_t frame_chunk_data[60]);

/**
 * \ingroup TF_DMX
 *
 * Returns the last frame that was written by the DMX master. The size of the array
 * is equivalent to the number of channels in the frame. Each byte represents one channel.
 *
 * The next frame is available after the {@link tf_dmx_register_frame_available_callback} callback was called.
 *
 * Generic approach:
 *
 * * Call {@link tf_dmx_read_frame} to get first frame.
 * * Wait for the {@link tf_dmx_register_frame_available_callback} callback.
 * * Call {@link tf_dmx_read_frame} to get second frame.
 * * Wait for the {@link tf_dmx_register_frame_available_callback} callback.
 * * and so on.
 *
 * Instead of polling this function you can also use the {@link tf_dmx_register_frame_callback} callback.
 * You can enable it with {@link tf_dmx_set_frame_callback_config}.
 *
 * The frame number starts at 0 and it is increased by one with each received frame.
 *
 * This function can only be called in slave mode.
 */
int tf_dmx_read_frame_low_level(TF_DMX *dmx, uint16_t *ret_frame_length, uint16_t *ret_frame_chunk_offset, uint8_t ret_frame_chunk_data[56], uint32_t *ret_frame_number);

/**
 * \ingroup TF_DMX
 *
 * Sets the duration of a frame.
 *
 * Example: If you want to achieve 20 frames per second, you should
 * set the frame duration to 50ms (50ms * 20 = 1 second).
 *
 * If you always want to send a frame as fast as possible you can set
 * this value to 0.
 *
 * This setting is only used in master mode.
 */
int tf_dmx_set_frame_duration(TF_DMX *dmx, uint16_t frame_duration);

/**
 * \ingroup TF_DMX
 *
 * Returns the frame duration as set by {@link tf_dmx_set_frame_duration}.
 */
int tf_dmx_get_frame_duration(TF_DMX *dmx, uint16_t *ret_frame_duration);

/**
 * \ingroup TF_DMX
 *
 * Returns the current number of overrun and framing errors.
 */
int tf_dmx_get_frame_error_count(TF_DMX *dmx, uint32_t *ret_overrun_error_count, uint32_t *ret_framing_error_count);

/**
 * \ingroup TF_DMX
 *
 * Sets the communication LED configuration. By default the LED shows
 * communication traffic, it flickers once for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is off.
 */
int tf_dmx_set_communication_led_config(TF_DMX *dmx, uint8_t config);

/**
 * \ingroup TF_DMX
 *
 * Returns the configuration as set by {@link tf_dmx_set_communication_led_config}
 */
int tf_dmx_get_communication_led_config(TF_DMX *dmx, uint8_t *ret_config);

/**
 * \ingroup TF_DMX
 *
 * Sets the error LED configuration.
 *
 * By default the error LED turns on if there is any error (see {@link tf_dmx_register_frame_error_count_callback}
 * callback). If you call this function with the Show-Error option again, the LED
 * will turn off until the next error occurs.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is off.
 */
int tf_dmx_set_error_led_config(TF_DMX *dmx, uint8_t config);

/**
 * \ingroup TF_DMX
 *
 * Returns the configuration as set by {@link tf_dmx_set_error_led_config}.
 */
int tf_dmx_get_error_led_config(TF_DMX *dmx, uint8_t *ret_config);

/**
 * \ingroup TF_DMX
 *
 * Enables/Disables the different callbacks. By default the
 * {@link tf_dmx_register_frame_started_callback} callback and {@link tf_dmx_register_frame_available_callback} callback are enabled while
 * the {@link tf_dmx_register_frame_callback} callback and {@link tf_dmx_register_frame_error_count_callback} callback are disabled.
 *
 * If you want to use the {@link tf_dmx_register_frame_callback} callback you can enable it and disable
 * the {@link tf_dmx_register_frame_available_callback} callback at the same time. It becomes redundant in
 * this case.
 */
int tf_dmx_set_frame_callback_config(TF_DMX *dmx, bool frame_started_callback_enabled, bool frame_available_callback_enabled, bool frame_callback_enabled, bool frame_error_count_callback_enabled);

/**
 * \ingroup TF_DMX
 *
 * Returns the frame callback config as set by {@link tf_dmx_set_frame_callback_config}.
 */
int tf_dmx_get_frame_callback_config(TF_DMX *dmx, bool *ret_frame_started_callback_enabled, bool *ret_frame_available_callback_enabled, bool *ret_frame_callback_enabled, bool *ret_frame_error_count_callback_enabled);

/**
 * \ingroup TF_DMX
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
int tf_dmx_get_spitfp_error_count(TF_DMX *dmx, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_DMX
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
int tf_dmx_set_bootloader_mode(TF_DMX *dmx, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_DMX
 *
 * Returns the current bootloader mode, see {@link tf_dmx_set_bootloader_mode}.
 */
int tf_dmx_get_bootloader_mode(TF_DMX *dmx, uint8_t *ret_mode);

/**
 * \ingroup TF_DMX
 *
 * Sets the firmware pointer for {@link tf_dmx_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_dmx_set_write_firmware_pointer(TF_DMX *dmx, uint32_t pointer);

/**
 * \ingroup TF_DMX
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_dmx_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_dmx_write_firmware(TF_DMX *dmx, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_DMX
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_dmx_set_status_led_config(TF_DMX *dmx, uint8_t config);

/**
 * \ingroup TF_DMX
 *
 * Returns the configuration as set by {@link tf_dmx_set_status_led_config}
 */
int tf_dmx_get_status_led_config(TF_DMX *dmx, uint8_t *ret_config);

/**
 * \ingroup TF_DMX
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_dmx_get_chip_temperature(TF_DMX *dmx, int16_t *ret_temperature);

/**
 * \ingroup TF_DMX
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_dmx_reset(TF_DMX *dmx);

/**
 * \ingroup TF_DMX
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_dmx_write_uid(TF_DMX *dmx, uint32_t uid);

/**
 * \ingroup TF_DMX
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_dmx_read_uid(TF_DMX *dmx, uint32_t *ret_uid);

/**
 * \ingroup TF_DMX
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
int tf_dmx_get_identity(TF_DMX *dmx, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

/**
 * \ingroup TF_DMX
 *
 * Writes a DMX frame. The maximum frame size is 512 byte. Each byte represents one channel.
 *
 * The next frame can be written after the {@link tf_dmx_register_frame_started_callback} callback was called. The frame
 * is double buffered, so a new frame can be written as soon as the writing of the prior frame
 * starts.
 *
 * The data will be transfered when the next frame duration ends, see {@link tf_dmx_set_frame_duration}.
 *
 * Generic approach:
 *
 * * Set the frame duration to a value that represents the number of frames per second you want to achieve.
 * * Set channels for first frame.
 * * Wait for the {@link tf_dmx_register_frame_started_callback} callback.
 * * Set channels for next frame.
 * * Wait for the {@link tf_dmx_register_frame_started_callback} callback.
 * * and so on.
 *
 * This approach ensures that you can set new DMX data with a fixed frame rate.
 *
 * This function can only be called in master mode.
 */
int tf_dmx_write_frame(TF_DMX *dmx, const uint8_t *frame, uint16_t frame_length);

/**
 * \ingroup TF_DMX
 *
 * Returns the last frame that was written by the DMX master. The size of the array
 * is equivalent to the number of channels in the frame. Each byte represents one channel.
 *
 * The next frame is available after the {@link tf_dmx_register_frame_available_callback} callback was called.
 *
 * Generic approach:
 *
 * * Call {@link tf_dmx_read_frame} to get first frame.
 * * Wait for the {@link tf_dmx_register_frame_available_callback} callback.
 * * Call {@link tf_dmx_read_frame} to get second frame.
 * * Wait for the {@link tf_dmx_register_frame_available_callback} callback.
 * * and so on.
 *
 * Instead of polling this function you can also use the {@link tf_dmx_register_frame_callback} callback.
 * You can enable it with {@link tf_dmx_set_frame_callback_config}.
 *
 * The frame number starts at 0 and it is increased by one with each received frame.
 *
 * This function can only be called in slave mode.
 */
int tf_dmx_read_frame(TF_DMX *dmx, uint8_t *ret_frame, uint16_t *ret_frame_length, uint32_t *ret_frame_number);

#ifdef __cplusplus
}
#endif

#endif
