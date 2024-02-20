/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_ARINC429_H
#define TF_ARINC429_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_ARINC429 ARINC429 Bricklet
 */

struct TF_ARINC429;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_ARINC429_HeartbeatMessageHandler)(struct TF_ARINC429 *arinc429, uint8_t channel, uint8_t status, uint8_t seq_number, uint16_t timestamp, uint16_t frames_processed, uint16_t frames_lost, void *user_data);
typedef void (*TF_ARINC429_FrameMessageHandler)(struct TF_ARINC429 *arinc429, uint8_t channel, uint8_t status, uint8_t seq_number, uint16_t timestamp, uint32_t frame, uint16_t age, void *user_data);
typedef void (*TF_ARINC429_SchedulerMessageHandler)(struct TF_ARINC429 *arinc429, uint8_t channel, uint8_t status, uint8_t seq_number, uint16_t timestamp, uint8_t userdata, void *user_data);

#endif
/**
 * \ingroup TF_ARINC429
 *
 * ARINC429 single transmitter and dual receiver
 */
typedef struct TF_ARINC429 {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_ARINC429_HeartbeatMessageHandler heartbeat_message_handler;
    void *heartbeat_message_user_data;

    TF_ARINC429_FrameMessageHandler frame_message_handler;
    void *frame_message_user_data;

    TF_ARINC429_SchedulerMessageHandler scheduler_message_handler;
    void *scheduler_message_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[2];
} TF_ARINC429;

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_GET_CAPABILITIES 1

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_SET_HEARTBEAT_CALLBACK_CONFIGURATION 2

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_GET_HEARTBEAT_CALLBACK_CONFIGURATION 3

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_SET_CHANNEL_CONFIGURATION 5

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_GET_CHANNEL_CONFIGURATION 6

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_SET_CHANNEL_MODE 7

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_GET_CHANNEL_MODE 8

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_CLEAR_ALL_RX_FILTERS 9

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_CLEAR_RX_FILTER 10

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_SET_RX_STANDARD_FILTERS 11

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_SET_RX_FILTER 12

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_GET_RX_FILTER 13

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_READ_FRAME 14

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_SET_RX_CALLBACK_CONFIGURATION 15

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_GET_RX_CALLBACK_CONFIGURATION 16

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_WRITE_FRAME_DIRECT 18

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_WRITE_FRAME_SCHEDULED 19

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_CLEAR_SCHEDULE_ENTRIES 20

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_SET_SCHEDULE_ENTRY 21

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_GET_SCHEDULE_ENTRY 22

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_RESTART 23

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_SET_FRAME_MODE 25

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_RESET 243

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_READ_UID 249

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_CALLBACK_HEARTBEAT_MESSAGE 4

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_CALLBACK_FRAME_MESSAGE 17

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_CALLBACK_SCHEDULER_MESSAGE 24

#endif

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_CHANNEL_TX 0

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_CHANNEL_TX1 1

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_CHANNEL_RX 32

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_CHANNEL_RX1 33

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_CHANNEL_RX2 34

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SDI_SDI0 0

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SDI_SDI1 1

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SDI_SDI2 2

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SDI_SDI3 3

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SDI_DATA 4

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_PARITY_DATA 0

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_PARITY_AUTO 1

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SPEED_HS 0

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SPEED_LS 1

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_CHANNEL_MODE_PASSIVE 0

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_CHANNEL_MODE_ACTIVE 1

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_CHANNEL_MODE_RUN 2

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_STATUS_NEW 0

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_STATUS_UPDATE 1

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_STATUS_TIMEOUT 2

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_STATUS_SCHEDULER 3

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_STATUS_STATISTICS 4

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SCHEDULER_JOB_SKIP 0

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SCHEDULER_JOB_CALLBACK 1

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SCHEDULER_JOB_STOP 2

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SCHEDULER_JOB_JUMP 3

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SCHEDULER_JOB_RETURN 4

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SCHEDULER_JOB_DWELL 5

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SCHEDULER_JOB_SINGLE 6

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SCHEDULER_JOB_CYCLIC 7

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SCHEDULER_JOB_RETRANS_RX1 8

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_SCHEDULER_JOB_RETRANS_RX2 9

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_TX_MODE_TRANSMIT 0

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_TX_MODE_MUTE 1

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_ARINC429
 */
#define TF_ARINC429_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_ARINC429
 *
 * This constant is used to identify a ARINC429 Bricklet.
 *
 * The {@link arinc429_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_ARINC429_DEVICE_IDENTIFIER 2160

/**
 * \ingroup TF_ARINC429
 *
 * This constant represents the display name of a ARINC429 Bricklet.
 */
#define TF_ARINC429_DEVICE_DISPLAY_NAME "ARINC429 Bricklet"

/**
 * \ingroup TF_ARINC429
 *
 * Creates the device object \c arinc429 with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_arinc429_create(TF_ARINC429 *arinc429, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_ARINC429
 *
 * Removes the device object \c arinc429 from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_arinc429_destroy(TF_ARINC429 *arinc429);

/**
 * \ingroup TF_ARINC429
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_arinc429_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_arinc429_get_response_expected(TF_ARINC429 *arinc429, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_ARINC429
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
int tf_arinc429_set_response_expected(TF_ARINC429 *arinc429, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_ARINC429
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_arinc429_set_response_expected_all(TF_ARINC429 *arinc429, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_ARINC429
 *
 * Registers the given \c handler to the Heartbeat Message callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t channel, uint8_t status, uint8_t seq_number, uint16_t timestamp, uint16_t frames_processed, uint16_t frames_lost, void *user_data) \endcode
 *
 * This callback is triggered periodically according to the configuration set by
 * {@link tf_arinc429_set_heartbeat_callback_configuration}. It reports the statistics counters
 * for processed frames and lost frames for all TX and RX channels.
 *
 * * Channel:          channel from which this heartbeat messages originates.
 * * Status:           will always be 'statistics'.
 * * Seq Number:       running counter that is incremented with each callback, starting with 0 and rolling over after 255 to 1. It will restart from 0 whenever the callback is turned off and on again. This counter can be used to detect lost callbacks.
 * * Timestamp:        running counter that is incremented on every millisecond, starting when the bricklet is powered up and rolling over after 65535 to 0. This counter can be used to measure the relative timing between frame receptions.
 * * Frames Processed: number of Arinc429 frames that are transmitted or received on the respective channels TX, RX1 and RX2.
 * * Frames Lost:      TX channel: number of Arinc429 frames that could not be transmitted due to a full transmit FIFO buffer, RX channels: number of received Arinc429 frames that could not be reported due to a full callback FIFO buffer.
 */
int tf_arinc429_register_heartbeat_message_callback(TF_ARINC429 *arinc429, TF_ARINC429_HeartbeatMessageHandler handler, void *user_data);


/**
 * \ingroup TF_ARINC429
 *
 * Registers the given \c handler to the Frame Message callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t channel, uint8_t status, uint8_t seq_number, uint16_t timestamp, uint32_t frame, uint16_t age, void *user_data) \endcode
 *
 * This callback is triggered according to the configuration set by {@link tf_arinc429_set_rx_callback_configuration}.
 *
 * * Channel:      channel from which this frame messages originates.
 * * Status:       'new' signals that the frame (label + SDI combination) was received for the first time ever or again after a previous timeout. 'update' signals that a new frame was received. 'timeout' signals that the frame (label and SDI combination) encountered the timeout state.
 * * Seq Number:   running counter that is incremented with each callback, starting with 0 and rolling over after 255 to 1. It will restart from 0 whenever the callback is turned off and on again. This counter can be used to detect lost callbacks.
 * * Timestamp:    running counter that is incremented on every millisecond, starting when the bricklet is powered up and rolling over after 65535 to 0. This counter can be used to measure the relative timing between frame receptions.
 * * Frame:        holds the complete Arinc429 frame including the label and SDI bits as a 32 bit integer. If 'parity_auto' is set for the channel, the parity bit will always come as 0. Opposite to the line transmission format, in the API functions the label code is mirrored such that the label code can directly be extracted from the frame by simply grabbing the lower 8 bits.
 * * Age:          time in milliseconds since this frame (label + SDI combination) was received last. If not received before or after a previous timeout, 60000 or the timeout value set with the {@link tf_arinc429_set_rx_callback_configuration} will be returned.
 */
int tf_arinc429_register_frame_message_callback(TF_ARINC429 *arinc429, TF_ARINC429_FrameMessageHandler handler, void *user_data);


/**
 * \ingroup TF_ARINC429
 *
 * Registers the given \c handler to the Scheduler Message callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t channel, uint8_t status, uint8_t seq_number, uint16_t timestamp, uint8_t userdata, void *user_data) \endcode
 *
 * This callback is triggered by the 'Callback' job in the transmit schedule.
 *
 * * Channel:          channel from which this frame messages originates, will always be 'CHANNEL_TX1'.
 * * Status:           will always be 'scheduler'
 * * Seq Number:       running counter that is incremented with each callback, starting with 0 and rolling over after 255 to 1. It will restart from 0 whenever the callback is turned off and on again. This counter can be used to detect lost callbacks.
 * * Timestamp:        running counter that is incremented on every millisecond, starting when the bricklet is powered up and rolling over after 65535 to 0. This counter can be used to measure the relative timing between frame receptions.
 * * Userdata:         8 bit number as set in the scheduler callback job
 */
int tf_arinc429_register_scheduler_message_callback(TF_ARINC429 *arinc429, TF_ARINC429_SchedulerMessageHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_ARINC429
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_arinc429_callback_tick(TF_ARINC429 *arinc429, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_ARINC429
 *
 * Gets the capabilities of the ARINC429 Bricklet as of the currently loaded firmware:
 *
 * * TX Total Scheduler Jobs: total number of job entries in the scheduling table.
 * * TX Used Scheduler Jobs:  number of job entries that are currently in use.
 * * RX Total Frame Filters:  total number of frame filters that can be defined per channel.
 * * RX Used Frame Filters:   number of frame filters that are currently in use per each channel.
 */
int tf_arinc429_get_capabilities(TF_ARINC429 *arinc429, uint16_t *ret_tx_total_scheduler_jobs, uint16_t *ret_tx_used_scheduler_jobs, uint16_t *ret_rx_total_frame_filters, uint16_t ret_rx_used_frame_filters[2]);

/**
 * \ingroup TF_ARINC429
 *
 * Sets the bricklet heartbeat callback function which reports the statistics counters for processed frames and lost frames.
 * The period is the period with which the {@link tf_arinc429_register_heartbeat_message_callback} callback is triggered periodically. A value of 0 turns the callback off.
 * When 'Value Has To Change' is enabled, the heartbeat will only be sent if there is a change in the statistics numbers.
 */
int tf_arinc429_set_heartbeat_callback_configuration(TF_ARINC429 *arinc429, uint8_t channel, bool enabled, bool value_has_to_change, uint16_t period);

/**
 * \ingroup TF_ARINC429
 *
 * Gets the current configuration of the bricklet heartbeat callback, see {@link tf_arinc429_set_heartbeat_callback_configuration}.
 */
int tf_arinc429_get_heartbeat_callback_configuration(TF_ARINC429 *arinc429, uint8_t channel, bool *ret_enabled, bool *ret_value_has_to_change, uint16_t *ret_period);

/**
 * \ingroup TF_ARINC429
 *
 * Sets the data transmission properties of the selected channel:
 *
 * * Channel: channel to configure
 * * Parity:  'parity_auto' for automatic parity adjustment, 'parity_data' for parity bit supplied by the application or if used for data.
 * * Speed:   'speed_hs' for high speed mode (100 kbit/s), 'speed_ls' for low speed mode (12.5 kbit/s).
 *
 * When parity set to 'parity_auto', frames received with a parity error will be counted in the lost frames counter but discarded otherwise.
 */
int tf_arinc429_set_channel_configuration(TF_ARINC429 *arinc429, uint8_t channel, uint8_t parity, uint8_t speed);

/**
 * \ingroup TF_ARINC429
 *
 * Gets the data transmission properties of the selected channel. The channel parameter and the data returned use the same constants
 * as the {@link tf_arinc429_set_channel_configuration},  despite that the all-channels constants CHANNEL_TX and CHANNEL_RX can not be used.
 */
int tf_arinc429_get_channel_configuration(TF_ARINC429 *arinc429, uint8_t channel, uint8_t *ret_parity, uint8_t *ret_speed);

/**
 * \ingroup TF_ARINC429
 *
 * Sets the operating mode of the selected channel(s):
 *
 * * passive: TX channel: all transmissions are stopped and the hardware interface becomes high-Z. RX channels: all arriving frames will be discarded.
 * * active:  TX channel: Arinc429 frames can be sent via the 'Write Frame Direct' function. RX channels: arriving frames will be processed according to the frame filter and callback settings.
 * * run:     TX channels only: the scheduler will run and transmit frames according to the entries made in the scheduler job table.
 */
int tf_arinc429_set_channel_mode(TF_ARINC429 *arinc429, uint8_t channel, uint8_t mode);

/**
 * \ingroup TF_ARINC429
 *
 * Gets the operating mode of the selected channel.  The channel parameter and the data returned use the same constants as the
 * {@link tf_arinc429_set_channel_configuration}, despite that the all-channels constants CHANNEL_TX and CHANNEL_RX can not be used.
 */
int tf_arinc429_get_channel_mode(TF_ARINC429 *arinc429, uint8_t channel, uint8_t *ret_mode);

/**
 * \ingroup TF_ARINC429
 *
 * Clears all receive filters on the selected RX channel(s). The RX channels will only process those Arinc429 frames that pass the
 * input filtering stage. With this command, all filters are cleared, thus all incoming Arinc429 frames will be blocked from further
 * processing.
 */
int tf_arinc429_clear_all_rx_filters(TF_ARINC429 *arinc429, uint8_t channel);

/**
 * \ingroup TF_ARINC429
 *
 * Clears a specific receive filter on the selected RX channel(s). The RX channels will only process those Arinc429 frames that pass
 * the input filtering stage. With this command, an Arinc429 frame matching the given parameters will be blocked by the filter.
 *
 * * Channel: selected channel.
 * * Label:   label code of the filter.
 * * SDI:     SDI code of the filter (SDI_SDI0 to SDI_SDI3 or SDI_DATA if SDI bits are used for data).
 *
 * The function either returns 'True' if the filter was cleared or 'False' if a respective filter was not set.
 */
int tf_arinc429_clear_rx_filter(TF_ARINC429 *arinc429, uint8_t channel, uint8_t label, uint8_t sdi, bool *ret_success);

/**
 * \ingroup TF_ARINC429
 *
 * Sets a receive filter for each label value (0-255 / 0o000-0o377) with the SDI bits set for data. Any previously existing filters will be overwritten.
 */
int tf_arinc429_set_rx_standard_filters(TF_ARINC429 *arinc429, uint8_t channel);

/**
 * \ingroup TF_ARINC429
 *
 * Sets a specific receive filter on the selected channel(s):
 *
 * * Channel: selected channel.
 * * Label:   label code for the filter.
 * * SDI:     SDI code for the filter (SDI_SDI0 to SDI_SDI3 or SDI_DATA if SDI bits are used for data).
 *
 * The function either returns 'True' if the filter was set or 'False' if a respective filter could not be created e.g. because the given combination
 * of label and SDI collides with an already existing filter, or because all available filters are used up (see the {@link tf_arinc429_get_capabilities} function).
 */
int tf_arinc429_set_rx_filter(TF_ARINC429 *arinc429, uint8_t channel, uint8_t label, uint8_t sdi, bool *ret_success);

/**
 * \ingroup TF_ARINC429
 *
 * Queries if a filter for the given combination of label and SDI is set up or not:
 *
 * * Channel:    channel to query.
 * * Label:      label code to query for.
 * * SDI:        SDI usage to query for (SDI_SDI0 to SDI_SDI3 or SDI_DATA if SDI bits shall be used for data).
 *
 * The function will return 'True' if the queried filter filter exists, else 'False'.
 */
int tf_arinc429_get_rx_filter(TF_ARINC429 *arinc429, uint8_t channel, uint8_t label, uint8_t sdi, bool *ret_configured);

/**
 * \ingroup TF_ARINC429
 *
 * Executes a direct read of an Arinc429 frame, i.e. without using the callback mechanism.
 * In order to be able to do a direct read of a frame with a certain label and SDI combination, a respective receive filter needs to be set up beforehand.
 *
 * * Channel: RX channel to read from.
 * * Label:   label code of the frame to read. Beware that the label codes are usually given in octal notation, so make sure to use the correct notation (i.e. 0o377).
 * * SDI:     SDI code of the frame to read (SDI_SDI0 to SDI_SDI3 or SDI_DATA if SDI bits are used for data).
 *
 * The function return the following data:
 *
 * * Status:  returns 'True' if a respective frame was received, else 'False'.
 * * Frame:   returns the complete Arinc429 frame including the label and SDI bits as a 32 bit integer. If 'parity_auto' is set for the channel, the parity bit will always come as 0. Opposite to the line transmission format, in the API functions the label code is mirrored such that the label code can be directly extracted from the frame by simply grabbing the lower 8 bits.
 * * Age:     time in milliseconds since a frame matching the label & SDI combination was received last. If no frame was received so far or after a previous timeout, either 60000 or the timeout value set with the {@link tf_arinc429_set_rx_callback_configuration} will be returned.
 */
int tf_arinc429_read_frame(TF_ARINC429 *arinc429, uint8_t channel, uint8_t label, uint8_t sdi, bool *ret_status, uint32_t *ret_frame, uint16_t *ret_age);

/**
 * \ingroup TF_ARINC429
 *
 * Sets the configuration of the Arinc429 frame reception callback:
 *
 * * Channel:             selected RX channel.
 * * Enabled:             select 'True' for activating the frame callbacks and 'False' for deactivating them.
 * * Value Has To Change: select 'True' if callbacks shall only be sent for frames whose data have changed. With 'False' a callback will be sent on every frame reception.
 * * Timeout:             time period for all frames (label and SDI combinations) on this channel.
 *
 * Despite on frame reception, a callback is also generated if a frame encounters a timeout, i.e. if it is not periodically received again before
 * the set timeout period has expired. In order to have callbacks being generated at all, respective receive filters need to be set up.
 */
int tf_arinc429_set_rx_callback_configuration(TF_ARINC429 *arinc429, uint8_t channel, bool enabled, bool value_has_to_change, uint16_t timeout);

/**
 * \ingroup TF_ARINC429
 *
 * Gets the configuration of the frame reception callback, see the {@link tf_arinc429_set_rx_callback_configuration}.
 */
int tf_arinc429_get_rx_callback_configuration(TF_ARINC429 *arinc429, uint8_t channel, bool *ret_enabled, bool *ret_value_has_to_change, uint16_t *ret_timeout);

/**
 * \ingroup TF_ARINC429
 *
 * Immediately transmits an Arinc429 frame, given that the channel is in either ACTIVE or RUN mode. If the channel is in RUN mode and frames are sent
 * as per programmed schedule, using this function will inject additional frames into the transmission, independent of the scheduler's activities.
 *
 * * Channel: selected transmit channel, either CHANNEL_TX or CHANNEL_TX1 can be used as there is only one TX channel.
 * * frame:   complete Arinc429 frame including the label and SDI bits.
 *
 * The frame needs to be passed as a 32 bit integer. Opposite to the line transmission format, in the API functions
 * the label code is mirrored such that the label code can be directly written 1:1 into the lower 8 bits.
 * Beware that the label codes are usually given in octal notation, so make sure to use the correct notation
 * (i.e. 0o377). If 'parity_auto' is set for the channel, the parity bit will be set (adjusted) automatically.
 *
 * Between the API and the actual Arinc429 line output, there is a 32 entry deep FIFO. If frames are written via
 * the API and/or in combination with a running TX scheduler, the FIFO may become overfilled and subsequently
 * frames will get lost. Such frame losses will be indicated in the statistics data sent with the heartbeat callback.
 */
int tf_arinc429_write_frame_direct(TF_ARINC429 *arinc429, uint8_t channel, uint32_t frame);

/**
 * \ingroup TF_ARINC429
 *
 * Sets or updates an Arinc429 frame that is to be transmitted via the scheduler using the scheduler job types 'Single' or 'Cyclic'.
 *
 * * Channel:     selected transmit channel, either CHANNEL_TX or CHANNEL_TX1 can be used as there is only one TX channel.
 * * Frame Index: index number (0-255) that will be used in the transmit scheduler job table to refer to this frame.
 * * Frame:       complete Arinc429 frame including the label and SDI bits.
 *
 * The frame needs to be passed as a 32 bit integer. Opposite to the line transmission format, in the API functions
 * the label code is mirrored such that the label code can be directly written 1:1 into the lower 8 bits.
 * Beware that the label codes are usually given in octal notation, so make sure to use the correct notation
 * (i.e. 0o377). If 'parity_auto' is set for the channel, the parity bit will be set (adjusted) automatically.
 *
 * If the frame is used by a 'single transmit' scheduler job entry, setting or updating the frame with this function
 * arms the frame for its next transmission.
 */
int tf_arinc429_write_frame_scheduled(TF_ARINC429 *arinc429, uint8_t channel, uint16_t frame_index, uint32_t frame);

/**
 * \ingroup TF_ARINC429
 *
 * Clears a range of transmit scheduler job table entries:
 *
 * * Channel: selected TX channel.
 * * First:   index of the first table entry to be cleared.
 * * Last:    index of the last  table entry to be cleared.
 *
 * To clear a single entry, set 'First' and 'Last' to the one index of the one entry to be cleared.
 * Clearing scheduler entries actually means they are set to the job command 'Skip'.
 */
int tf_arinc429_clear_schedule_entries(TF_ARINC429 *arinc429, uint8_t channel, uint16_t job_index_first, uint16_t job_index_last);

/**
 * \ingroup TF_ARINC429
 *
 * Sets an entry in the transmit scheduler job table:
 *
 * * Channel:     selected TX channel, either CHANNEL_TX or CHANNEL_TX1 can be used as there is only one TX channel.
 * * Job Index:   index number of the job, the scheduler processes the job table in ascending order of these index numbers. The index starts with 0, see the output of {@link tf_arinc429_get_capabilities} function for the total number of job indexes available. In firmware 2.4.0 it is 1000.
 * * Job:         activity assigned to this entry, see below.
 * * Frame Index: generally, the frame assigned to this job by the 'Frame Index' used along with the {@link tf_arinc429_write_frame_scheduled}.
 *                In case of a RX1 or RX2 retransmit job, the extended label (label + SDI) of the frame to be retransmitted.
 *                In case of the Jump command, the Job Index at which execution shall continue.
 *                In case of the Callback command, this number will be sent as 'Token' code (values 0-255 only).
 *                In all other cases (Skip, Stop, Dwell, Return) this parameter is not used.
 * * Dwell Time:  time to wait before executing the next job table entry (0-250 milliseconds).
 *
 * When the scheduler is set to 'run' mode via the {@link tf_arinc429_set_channel_mode}, it continuously loops through the job table and executes
 * the assigned tasks. It starts with the job stored at job index 0.
 * The scheduler can execute the following activity types (jobs):
 *
 * * Skip:        the job is skipped, i.e. no frame is transmitted and no dwelling is done. The frame index and dwell time are not used.
 * * Stop:        the scheduler is stopped, i.e. the channel mode is reverted from 'run' to 'active'. The frame index and dwell time are not used.
 * * Jump:        the scheduler immediately continues at the Job Index position given by the Frame Index parameter. The assigned dwell time will be executed when the scheduler runs into the next Return job.
 * * Return:      the scheduler immediately continues at the next Job Index position following the last Jump command. Nested Jumps are not supported. The frame index and dwell time are not used.
 * * Callback:    the scheduler triggers a callback message and immediately continues with executing the next job (dwell time is not used).
 * * Dwell        the scheduler executes the dwelling but does not transmit any frame. The frame index is not used.
 * * Single:      the scheduler transmits the referenced frame, but only once. On subsequent executions the frame is not sent until it is renewed via the {@link tf_arinc429_write_frame_scheduled}, then the process repeats.
 * * Cyclic:      the scheduler transmits the referenced frame and executs the dwelling on each round.
 * * Retrans RX1: the scheduler retransmits a frame that was previously received on the RX1 channel. The frame to send is referenced by setting the 'Frame Index' to its extended label code, which is a 10 bit number made of the label code in the lower bits and the two SDI bits in the upper bits. If the SDI bits are used for data, set the SDI bits to zero. As long as the referenced frame was not received yet, or if it is in timeout, no frame will be sent.
 * * Retrans RX2: same as before, but for frames received on the RX2 channel.
 *
 * The value assigned to the 'Frame Index' parameter varies with the activity type (job):
 *
 * * Single or Cyclic: frame index as used with the {@link tf_arinc429_write_frame_scheduled} of the frame to transmit. Valid range: 0-255
 * * Retrans RX1/RX2:  extended label (label + SDI) of the frame to re-transmit. Valid range: 0-1023
 * * Callback:         arbitrary number decided by the user, it will be reported in the callback via the 'Userdata' parameter. Valid range: 0-255
 * * Jump:             next job index to jump to.
 *
 * The {@link tf_arinc429_set_schedule_entry} can be called while the TX scheduler is running, i.e. the channel mode is set to 'RUN'.
 * Any change will take immediate effect once the scheduler passes along and executes the changed job entry.
 * Every time the scheduler is started, it will begin with the job stored at job index 0.
 * At the end of the programmed schedule there should be a 'Jump' command back to index 0 to avoid the scheduler wasting time in processing all the remaining 'Skip' commands.
 * Two or more TX schedules can be pre-programmed and then selected for execution by placing - and changing as desired - a 'Jump' at index 0 that then branches to the
 * sequence of commands destined to be executed. This can be arranged in arbitrary ways, e.g. to create schedules with fixed and variable parts, just by using the 'Jump'
 * command alike a track switch in railway.
 *
 * When the dwell time of a transmit command is set to zero, the respective Arinc429 frames will be transmitted back-to-back on the physical link.
 * Beware that there is a FIFO between the scheduler and the actual physical transmitter that is limited to 32 frames. So after latest 32 frames enqueued with zero dwell
 * time, the scheduler needs to be commanded to do some dwelling. How much dwelling is required can be computed by the number of back-to-back frames and the speed setting:
 * in high speed mode each frame takes 0.36 ms, in low speed mode 2.88 ms.
 * If a certain sequence of frames is to be transmitted multiple times in a schedule, this sequence just needs to be put once into the scheduler table with a 'Return'
 * command at its end. This way, this sequence can be called from multiple placed (job indexes) throughout the main schedule using the 'Jump' command.
 * Please note that this kind of calling a subroutine can not be nested, i.e. there is no return index stack, the 'Return' command always branches to the job index following
 * the index of the last 'Jump' command encountered. In case a dwell time > 0 is set with the 'Jump' command, this dwell time will actually be executed on encountering the
 * 'Return' command, thus as a dwell time to be done after the execution of the subsequence that was jumped to before.
 *
 * The 'Callback' command can be used to notify the application program via a callback when the scheduler passes at the respective job index. This can be used for pure
 * reporting / surveillance purpose, or as a means to set up a self-clocked system in which the called application program's function in return does some modification of
 * the programmed sequence or alike.
 * The scheduler can also be programmed to stop itself via the 'Stop' command, e.g. to run a pre-programmed, accurately timed single-shot sequence of frame transmissions.
 * Placing a' Callback' command right before the 'Stop' command will inform the application program via a callback when the sequence is done.
 * When using several 'Callback' commands in a schedule, each of them can be uniquely identified in the receiving application program by assigning a different 'userdata'
 * value to each callback command.
 *
 * With the aid of the 'Retrans' commands, a frame transmission schedule can be set up whose frame timing is defined by the schedule, but whose frame's payload is taken
 * from the frames received via the RX1 or RX2 channel. This opens possibilities to create an autonomously operating time base corrector or re-scheduling machinery, to
 * zip the frames from two A429 buses onto one common bus, to create inline filers to remove certain frames (by their label & SDI code), to insert frames into a stream,
 * to exchange the payload of certain frames on-the-fly, and much more.
 */
int tf_arinc429_set_schedule_entry(TF_ARINC429 *arinc429, uint8_t channel, uint16_t job_index, uint8_t job, uint16_t frame_index, uint8_t dwell_time);

/**
 * \ingroup TF_ARINC429
 *
 * Gets the definition of a transmit scheduler job table entry, refer to the {@link tf_arinc429_set_schedule_entry}.
 */
int tf_arinc429_get_schedule_entry(TF_ARINC429 *arinc429, uint8_t channel, uint16_t job_index, uint8_t *ret_job, uint16_t *ret_frame_index, uint32_t *ret_frame, uint8_t *ret_dwell_time);

/**
 * \ingroup TF_ARINC429
 *
 * Reverts the whole bricklet into its power-up default state.
 */
int tf_arinc429_restart(TF_ARINC429 *arinc429);

/**
 * \ingroup TF_ARINC429
 *
 * Stops / resumes the transmission of a specific frame or trigger another single-transmit. This
 * function only works on frames that are sent via the TX scheduler jobs 'single' and 'cyclic'.
 *
 * * Channel:     selected transmit channel, either CHANNEL_TX or CHANNEL_TX1 can be used as there is only one TX channel.
 * * Frame Index: index number that will be used in the transmit scheduler job table to refer to this frame.
 * * Mode:        either 'Transmit' to transmit the frame / trigger a new single transmit, or 'Mute' to stop the transmission of the frame.
 */
int tf_arinc429_set_frame_mode(TF_ARINC429 *arinc429, uint8_t channel, uint16_t frame_index, uint8_t mode);

/**
 * \ingroup TF_ARINC429
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
int tf_arinc429_get_spitfp_error_count(TF_ARINC429 *arinc429, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_ARINC429
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
int tf_arinc429_set_bootloader_mode(TF_ARINC429 *arinc429, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_ARINC429
 *
 * Returns the current bootloader mode, see {@link tf_arinc429_set_bootloader_mode}.
 */
int tf_arinc429_get_bootloader_mode(TF_ARINC429 *arinc429, uint8_t *ret_mode);

/**
 * \ingroup TF_ARINC429
 *
 * Sets the firmware pointer for {@link tf_arinc429_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_arinc429_set_write_firmware_pointer(TF_ARINC429 *arinc429, uint32_t pointer);

/**
 * \ingroup TF_ARINC429
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_arinc429_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_arinc429_write_firmware(TF_ARINC429 *arinc429, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_ARINC429
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_arinc429_set_status_led_config(TF_ARINC429 *arinc429, uint8_t config);

/**
 * \ingroup TF_ARINC429
 *
 * Returns the configuration as set by {@link tf_arinc429_set_status_led_config}
 */
int tf_arinc429_get_status_led_config(TF_ARINC429 *arinc429, uint8_t *ret_config);

/**
 * \ingroup TF_ARINC429
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_arinc429_get_chip_temperature(TF_ARINC429 *arinc429, int16_t *ret_temperature);

/**
 * \ingroup TF_ARINC429
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_arinc429_reset(TF_ARINC429 *arinc429);

/**
 * \ingroup TF_ARINC429
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_arinc429_write_uid(TF_ARINC429 *arinc429, uint32_t uid);

/**
 * \ingroup TF_ARINC429
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_arinc429_read_uid(TF_ARINC429 *arinc429, uint32_t *ret_uid);

/**
 * \ingroup TF_ARINC429
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
int tf_arinc429_get_identity(TF_ARINC429 *arinc429, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

#ifdef __cplusplus
}
#endif

#endif
