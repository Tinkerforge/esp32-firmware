/* ***********************************************************
 * This file was automatically generated on 2024-10-25.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/

#ifndef TF_NFC_H
#define TF_NFC_H

#include "config.h"
#include "tfp.h"
#include "hal_common.h"
#include "macros.h"
#include "streaming.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup TF_NFC NFC Bricklet
 */

struct TF_NFC;
#if TF_IMPLEMENT_CALLBACKS != 0

typedef void (*TF_NFC_ReaderStateChangedHandler)(struct TF_NFC *nfc, uint8_t state, bool idle, void *user_data);
typedef void (*TF_NFC_CardemuStateChangedHandler)(struct TF_NFC *nfc, uint8_t state, bool idle, void *user_data);
typedef void (*TF_NFC_P2PStateChangedHandler)(struct TF_NFC *nfc, uint8_t state, bool idle, void *user_data);

#endif
/**
 * \ingroup TF_NFC
 *
 * NFC tag read/write, NFC P2P and Card Emulation
 */
typedef struct TF_NFC {
    TF_TFP *tfp;
#if TF_IMPLEMENT_CALLBACKS != 0
    TF_NFC_ReaderStateChangedHandler reader_state_changed_handler;
    void *reader_state_changed_user_data;

    TF_NFC_CardemuStateChangedHandler cardemu_state_changed_handler;
    void *cardemu_state_changed_user_data;

    TF_NFC_P2PStateChangedHandler p2p_state_changed_handler;
    void *p2p_state_changed_user_data;

#endif
    uint16_t magic;
    uint8_t response_expected[3];
} TF_NFC;

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_SET_MODE 1

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_GET_MODE 2

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_READER_REQUEST_TAG_ID 3

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_READER_GET_TAG_ID_LOW_LEVEL 4

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_READER_GET_STATE 5

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_READER_WRITE_NDEF_LOW_LEVEL 6

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_READER_REQUEST_NDEF 7

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_READER_READ_NDEF_LOW_LEVEL 8

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_READER_AUTHENTICATE_MIFARE_CLASSIC_PAGE 9

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_READER_WRITE_PAGE_LOW_LEVEL 10

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_READER_REQUEST_PAGE 11

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_READER_READ_PAGE_LOW_LEVEL 12

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_CARDEMU_GET_STATE 14

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_CARDEMU_START_DISCOVERY 15

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_CARDEMU_WRITE_NDEF_LOW_LEVEL 16

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_CARDEMU_START_TRANSFER 17

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_P2P_GET_STATE 19

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_P2P_START_DISCOVERY 20

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_P2P_WRITE_NDEF_LOW_LEVEL 21

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_P2P_START_TRANSFER 22

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_P2P_READ_NDEF_LOW_LEVEL 23

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_SET_DETECTION_LED_CONFIG 25

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_GET_DETECTION_LED_CONFIG 26

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_SET_MAXIMUM_TIMEOUT 27

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_GET_MAXIMUM_TIMEOUT 28

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_SIMPLE_GET_TAG_ID_LOW_LEVEL 29

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_CARDEMU_SET_TAG_ID 30

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_CARDEMU_GET_TAG_ID 31

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_GET_SPITFP_ERROR_COUNT 234

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_SET_BOOTLOADER_MODE 235

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_GET_BOOTLOADER_MODE 236

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_SET_WRITE_FIRMWARE_POINTER 237

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_WRITE_FIRMWARE 238

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_SET_STATUS_LED_CONFIG 239

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_GET_STATUS_LED_CONFIG 240

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_GET_CHIP_TEMPERATURE 242

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_RESET 243

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_WRITE_UID 248

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_READ_UID 249

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_FUNCTION_GET_IDENTITY 255

#if TF_IMPLEMENT_CALLBACKS != 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CALLBACK_READER_STATE_CHANGED 13

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CALLBACK_CARDEMU_STATE_CHANGED 18

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CALLBACK_P2P_STATE_CHANGED 24

#endif

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_MODE_OFF 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_MODE_CARDEMU 1

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_MODE_P2P 2

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_MODE_READER 3

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_MODE_SIMPLE 4

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_TAG_TYPE_MIFARE_CLASSIC 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_TAG_TYPE_TYPE1 1

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_TAG_TYPE_TYPE2 2

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_TAG_TYPE_TYPE3 3

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_TAG_TYPE_TYPE4 4

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_TAG_TYPE_TYPE5 5

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_INITIALIZATION 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_IDLE 128

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_ERROR 192

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_REQUEST_TAG_ID 2

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_REQUEST_TAG_ID_READY 130

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_REQUEST_TAG_ID_ERROR 194

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE 3

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE_READY 131

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE_ERROR 195

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_WRITE_PAGE 4

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_WRITE_PAGE_READY 132

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_WRITE_PAGE_ERROR 196

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_REQUEST_PAGE 5

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_REQUEST_PAGE_READY 133

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_REQUEST_PAGE_ERROR 197

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_WRITE_NDEF 6

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_WRITE_NDEF_READY 134

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_WRITE_NDEF_ERROR 198

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_REQUEST_NDEF 7

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_REQUEST_NDEF_READY 135

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_STATE_REQUEST_NDEF_ERROR 199

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_KEY_A 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_KEY_B 1

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_WRITE_TYPE4_CAPABILITY_CONTAINER 3

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_WRITE_TYPE4_NDEF 4

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_REQUEST_TYPE4_CAPABILITY_CONTAINER 3

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_READER_REQUEST_TYPE4_NDEF 4

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CARDEMU_STATE_INITIALIZATION 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CARDEMU_STATE_IDLE 128

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CARDEMU_STATE_ERROR 192

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CARDEMU_STATE_DISCOVER 2

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CARDEMU_STATE_DISCOVER_READY 130

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CARDEMU_STATE_DISCOVER_ERROR 194

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CARDEMU_STATE_TRANSFER_NDEF 3

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CARDEMU_STATE_TRANSFER_NDEF_READY 131

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CARDEMU_STATE_TRANSFER_NDEF_ERROR 195

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CARDEMU_TRANSFER_ABORT 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_CARDEMU_TRANSFER_WRITE 1

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_P2P_STATE_INITIALIZATION 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_P2P_STATE_IDLE 128

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_P2P_STATE_ERROR 192

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_P2P_STATE_DISCOVER 2

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_P2P_STATE_DISCOVER_READY 130

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_P2P_STATE_DISCOVER_ERROR 194

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_P2P_STATE_TRANSFER_NDEF 3

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_P2P_STATE_TRANSFER_NDEF_READY 131

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_P2P_STATE_TRANSFER_NDEF_ERROR 195

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_P2P_TRANSFER_ABORT 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_P2P_TRANSFER_WRITE 1

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_P2P_TRANSFER_READ 2

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_DETECTION_LED_CONFIG_OFF 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_DETECTION_LED_CONFIG_ON 1

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_DETECTION_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_DETECTION_LED_CONFIG_SHOW_DETECTION 3

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_BOOTLOADER_MODE_BOOTLOADER 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_BOOTLOADER_MODE_FIRMWARE 1

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_BOOTLOADER_STATUS_OK 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_BOOTLOADER_STATUS_INVALID_MODE 1

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_BOOTLOADER_STATUS_NO_CHANGE 2

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_BOOTLOADER_STATUS_CRC_MISMATCH 5

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_STATUS_LED_CONFIG_OFF 0

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_STATUS_LED_CONFIG_ON 1

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2

/**
 * \ingroup TF_NFC
 */
#define TF_NFC_STATUS_LED_CONFIG_SHOW_STATUS 3

/**
 * \ingroup TF_NFC
 *
 * This constant is used to identify a NFC Bricklet.
 *
 * The {@link nfc_get_identity} function and the
 * {@link IPCON_CALLBACK_ENUMERATE} callback of the IP Connection have a
 * \c device_identifier parameter to specify the Brick's or Bricklet's type.
 */
#define TF_NFC_DEVICE_IDENTIFIER 286

/**
 * \ingroup TF_NFC
 *
 * This constant represents the display name of a NFC Bricklet.
 */
#define TF_NFC_DEVICE_DISPLAY_NAME "NFC Bricklet"

/**
 * \ingroup TF_NFC
 *
 * Creates the device object \c nfc with the unique device ID \c uid and adds
 * it to the HAL \c hal.
 */
int tf_nfc_create(TF_NFC *nfc, const char *uid_or_port_name, TF_HAL *hal);

/**
 * \ingroup TF_NFC
 *
 * Removes the device object \c nfc from its HAL and destroys it.
 * The device object cannot be used anymore afterwards.
 */
int tf_nfc_destroy(TF_NFC *nfc);

/**
 * \ingroup TF_NFC
 *
 * Returns the response expected flag for the function specified by the
 * \c function_id parameter. It is *true* if the function is expected to
 * send a response, *false* otherwise.
 *
 * For getter functions this is enabled by default and cannot be disabled,
 * because those functions will always send a response. For callback
 * configuration functions it is enabled by default too, but can be disabled
 * via the tf_nfc_set_response_expected function. For setter
 * functions it is disabled by default and can be enabled.
 *
 * Enabling the response expected flag for a setter function allows to
 * detect timeouts and other error conditions calls of this setter as well.
 * The device will then send a response for this purpose. If this flag is
 * disabled for a setter function then no response is sent and errors are
 * silently ignored, because they cannot be detected.
 */
int tf_nfc_get_response_expected(TF_NFC *nfc, uint8_t function_id, bool *ret_response_expected);

/**
 * \ingroup TF_NFC
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
int tf_nfc_set_response_expected(TF_NFC *nfc, uint8_t function_id, bool response_expected);

/**
 * \ingroup TF_NFC
 *
 * Changes the response expected flag for all setter and callback configuration
 * functions of this device at once.
 */
int tf_nfc_set_response_expected_all(TF_NFC *nfc, bool response_expected);
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_NFC
 *
 * Registers the given \c handler to the Reader State Changed callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t state, bool idle, void *user_data) \endcode
 *
 * This callback is called if the reader state of the NFC Bricklet changes.
 * See {@link tf_nfc_reader_get_state} for more information about the possible states.
 */
int tf_nfc_register_reader_state_changed_callback(TF_NFC *nfc, TF_NFC_ReaderStateChangedHandler handler, void *user_data);


/**
 * \ingroup TF_NFC
 *
 * Registers the given \c handler to the Cardemu State Changed callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t state, bool idle, void *user_data) \endcode
 *
 * This callback is called if the cardemu state of the NFC Bricklet changes.
 * See {@link tf_nfc_cardemu_get_state} for more information about the possible states.
 */
int tf_nfc_register_cardemu_state_changed_callback(TF_NFC *nfc, TF_NFC_CardemuStateChangedHandler handler, void *user_data);


/**
 * \ingroup TF_NFC
 *
 * Registers the given \c handler to the P2P State Changed callback. The
 * \c user_data will be passed as the last parameter to the \c handler.
 *
 * Signature: \code void callback(uint8_t state, bool idle, void *user_data) \endcode
 *
 * This callback is called if the P2P state of the NFC Bricklet changes.
 * See {@link tf_nfc_p2p_get_state} for more information about the possible states.
 */
int tf_nfc_register_p2p_state_changed_callback(TF_NFC *nfc, TF_NFC_P2PStateChangedHandler handler, void *user_data);
#endif
#if TF_IMPLEMENT_CALLBACKS != 0
/**
 * \ingroup TF_NFC
 *
 * Polls for callbacks. Will block for the given timeout in microseconds.
 *
 * This function can be used in a non-blocking fashion by calling it with a timeout of 0.
 */
int tf_nfc_callback_tick(TF_NFC *nfc, uint32_t timeout_us);
#endif

/**
 * \ingroup TF_NFC
 *
 * Sets the mode. The NFC Bricklet supports four modes:
 *
 * * Off
 * * Card Emulation (Cardemu): Emulates a tag for other readers
 * * Peer to Peer (P2P): Exchange data with other readers
 * * Reader: Reads and writes tags
 * * Simple: Automatically reads tag IDs
 *
 * If you change a mode, the Bricklet will reconfigure the hardware for this mode.
 * Therefore, you can only use functions corresponding to the current mode. For
 * example, in Reader mode you can only use Reader functions.
 */
int tf_nfc_set_mode(TF_NFC *nfc, uint8_t mode);

/**
 * \ingroup TF_NFC
 *
 * Returns the mode as set by {@link tf_nfc_set_mode}.
 */
int tf_nfc_get_mode(TF_NFC *nfc, uint8_t *ret_mode);

/**
 * \ingroup TF_NFC
 *
 * After you call {@link tf_nfc_reader_request_tag_id} the NFC Bricklet will try to read
 * the tag ID from the tag. After this process is done the state will change.
 * You can either register the {@link tf_nfc_register_reader_state_changed_callback} callback or you can poll
 * {@link tf_nfc_reader_get_state} to find out about the state change.
 *
 * If the state changes to *ReaderRequestTagIDError* it means that either there was
 * no tag present or that the tag has an incompatible type. If the state
 * changes to *ReaderRequestTagIDReady* it means that a compatible tag was found
 * and that the tag ID has been saved. You can now read out the tag ID by
 * calling {@link tf_nfc_reader_get_tag_id}.
 *
 * If two tags are in the proximity of the NFC Bricklet, this
 * function will cycle through the tags. To select a specific tag you have
 * to call {@link tf_nfc_reader_request_tag_id} until the correct tag ID is found.
 *
 * In case of any *ReaderError* state the selection is lost and you have to
 * start again by calling {@link tf_nfc_reader_request_tag_id}.
 */
int tf_nfc_reader_request_tag_id(TF_NFC *nfc);

/**
 * \ingroup TF_NFC
 *
 * Returns the tag type and the tag ID. This function can only be called if the
 * NFC Bricklet is currently in one of the *ReaderReady* states. The returned tag ID
 * is the tag ID that was saved through the last call of {@link tf_nfc_reader_request_tag_id}.
 *
 * To get the tag ID of a tag the approach is as follows:
 *
 * 1. Call {@link tf_nfc_reader_request_tag_id}
 * 2. Wait for state to change to *ReaderRequestTagIDReady* (see {@link tf_nfc_reader_get_state} or
 *    {@link tf_nfc_register_reader_state_changed_callback} callback)
 * 3. Call {@link tf_nfc_reader_get_tag_id}
 */
int tf_nfc_reader_get_tag_id_low_level(TF_NFC *nfc, uint8_t *ret_tag_type, uint8_t *ret_tag_id_length, uint8_t ret_tag_id_data[32]);

/**
 * \ingroup TF_NFC
 *
 * Returns the current reader state of the NFC Bricklet.
 *
 * On startup the Bricklet will be in the *ReaderInitialization* state. The
 * initialization will only take about 20ms. After that it changes to *ReaderIdle*.
 *
 * The Bricklet is also reinitialized if the mode is changed, see {@link tf_nfc_set_mode}.
 *
 * The functions of this Bricklet can be called in the *ReaderIdle* state and all of
 * the *ReaderReady* and *ReaderError* states.
 *
 * Example: If you call {@link tf_nfc_reader_request_page}, the state will change to
 * *ReaderRequestPage* until the reading of the page is finished. Then it will change
 * to either *ReaderRequestPageReady* if it worked or to *ReaderRequestPageError* if it
 * didn't. If the request worked you can get the page by calling {@link tf_nfc_reader_read_page}.
 *
 * The same approach is used analogously for the other API functions.
 */
int tf_nfc_reader_get_state(TF_NFC *nfc, uint8_t *ret_state, bool *ret_idle);

/**
 * \ingroup TF_NFC
 *
 * Writes NDEF formated data.
 *
 * This function currently supports NFC Forum Type 2, 4, 5 and Mifare Classic.
 *
 * The general approach for writing a NDEF message is as follows:
 *
 * 1. Call {@link tf_nfc_reader_request_tag_id}
 * 2. Wait for state to change to *ReaderRequestTagIDReady* (see
 *    {@link tf_nfc_reader_get_state} or {@link tf_nfc_register_reader_state_changed_callback} callback)
 * 3. If looking for a specific tag then call {@link tf_nfc_reader_get_tag_id} and check
 *    if the expected tag was found, if it was not found got back to step 1
 * 4. Call {@link tf_nfc_reader_write_ndef} with the NDEF message that you want to write
 * 5. Wait for state to change to *ReaderWriteNDEFReady* (see {@link tf_nfc_reader_get_state}
 *    or {@link tf_nfc_register_reader_state_changed_callback} callback)
 */
int tf_nfc_reader_write_ndef_low_level(TF_NFC *nfc, uint16_t ndef_length, uint16_t ndef_chunk_offset, const uint8_t ndef_chunk_data[60]);

/**
 * \ingroup TF_NFC
 *
 * Reads NDEF formated data from a tag.
 *
 * This function currently supports NFC Forum Type 1, 2, 3, 4, 5 and Mifare Classic.
 *
 * The general approach for reading a NDEF message is as follows:
 *
 * 1. Call {@link tf_nfc_reader_request_tag_id}
 * 2. Wait for state to change to *RequestTagIDReady* (see {@link tf_nfc_reader_get_state}
 *    or {@link tf_nfc_register_reader_state_changed_callback} callback)
 * 3. If looking for a specific tag then call {@link tf_nfc_reader_get_tag_id} and check if the
 *    expected tag was found, if it was not found got back to step 1
 * 4. Call {@link tf_nfc_reader_request_ndef}
 * 5. Wait for state to change to *ReaderRequestNDEFReady* (see {@link tf_nfc_reader_get_state}
 *    or {@link tf_nfc_register_reader_state_changed_callback} callback)
 * 6. Call {@link tf_nfc_reader_read_ndef} to retrieve the NDEF message from the buffer
 */
int tf_nfc_reader_request_ndef(TF_NFC *nfc);

/**
 * \ingroup TF_NFC
 *
 * Returns the NDEF data from an internal buffer. To fill the buffer
 * with a NDEF message you have to call {@link tf_nfc_reader_request_ndef} beforehand.
 */
int tf_nfc_reader_read_ndef_low_level(TF_NFC *nfc, uint16_t *ret_ndef_length, uint16_t *ret_ndef_chunk_offset, uint8_t ret_ndef_chunk_data[60]);

/**
 * \ingroup TF_NFC
 *
 * Mifare Classic tags use authentication. If you want to read from or write to
 * a Mifare Classic page you have to authenticate it beforehand.
 * Each page can be authenticated with two keys: A (``key_number`` = 0) and B
 * (``key_number`` = 1). A new Mifare Classic
 * tag that has not yet been written to can be accessed with key A
 * and the default key ``[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]``.
 *
 * The approach to read or write a Mifare Classic page is as follows:
 *
 * 1. Call {@link tf_nfc_reader_request_tag_id}
 * 2. Wait for state to change to *ReaderRequestTagIDReady* (see {@link tf_nfc_reader_get_state}
 *    or {@link tf_nfc_register_reader_state_changed_callback} callback)
 * 3. If looking for a specific tag then call {@link tf_nfc_reader_get_tag_id} and check if the
 *    expected tag was found, if it was not found got back to step 1
 * 4. Call {@link tf_nfc_reader_authenticate_mifare_classic_page} with page and key for the page
 * 5. Wait for state to change to *ReaderAuthenticatingMifareClassicPageReady* (see
 *    {@link tf_nfc_reader_get_state} or {@link tf_nfc_register_reader_state_changed_callback} callback)
 * 6. Call {@link tf_nfc_reader_request_page} or {@link tf_nfc_reader_write_page} to read/write page
 *
 * The authentication will always work for one whole sector (4 pages).
 */
int tf_nfc_reader_authenticate_mifare_classic_page(TF_NFC *nfc, uint16_t page, uint8_t key_number, const uint8_t key[6]);

/**
 * \ingroup TF_NFC
 *
 * Writes a maximum of 8192 bytes starting from the given page. How many pages are written
 * depends on the tag type. The page sizes are as follows:
 *
 * * Mifare Classic page size: 16 byte
 * * NFC Forum Type 1 page size: 8 byte
 * * NFC Forum Type 2 page size: 4 byte
 * * NFC Forum Type 3 page size: 16 byte
 * * NFC Forum Type 4: No pages, page = file selection (CC or NDEF, see below)
 * * NFC Forum Type 5 page size: 4 byte
 *
 * The general approach for writing to a tag is as follows:
 *
 * 1. Call {@link tf_nfc_reader_request_tag_id}
 * 2. Wait for state to change to *ReaderRequestTagIDReady* (see {@link tf_nfc_reader_get_state} or
 *    {@link tf_nfc_register_reader_state_changed_callback} callback)
 * 3. If looking for a specific tag then call {@link tf_nfc_reader_get_tag_id} and check if the
 *    expected tag was found, if it was not found got back to step 1
 * 4. Call {@link tf_nfc_reader_write_page} with page number and data
 * 5. Wait for state to change to *ReaderWritePageReady* (see {@link tf_nfc_reader_get_state} or
 *    {@link tf_nfc_register_reader_state_changed_callback} callback)
 *
 * If you use a Mifare Classic tag you have to authenticate a page before you
 * can write to it. See {@link tf_nfc_reader_authenticate_mifare_classic_page}.
 *
 * NFC Forum Type 4 tags are not organized into pages but different files. We currently
 * support two files: Capability Container file (CC) and NDEF file.
 *
 * Choose CC by setting page to 3 or NDEF by setting page to 4.
 */
int tf_nfc_reader_write_page_low_level(TF_NFC *nfc, uint16_t page, uint16_t data_length, uint16_t data_chunk_offset, const uint8_t data_chunk_data[58]);

/**
 * \ingroup TF_NFC
 *
 * Reads a maximum of 8192 bytes starting from the given page and stores them into a buffer.
 * The buffer can then be read out with {@link tf_nfc_reader_read_page}.
 * How many pages are read depends on the tag type. The page sizes are
 * as follows:
 *
 * * Mifare Classic page size: 16 byte
 * * NFC Forum Type 1 page size: 8 byte
 * * NFC Forum Type 2 page size: 4 byte
 * * NFC Forum Type 3 page size: 16 byte
 * * NFC Forum Type 4: No pages, page = file selection (CC or NDEF, see below)
 * * NFC Forum Type 5 page size: 4 byte
 *
 * The general approach for reading a tag is as follows:
 *
 * 1. Call {@link tf_nfc_reader_request_tag_id}
 * 2. Wait for state to change to *RequestTagIDReady* (see {@link tf_nfc_reader_get_state}
 *    or {@link tf_nfc_register_reader_state_changed_callback} callback)
 * 3. If looking for a specific tag then call {@link tf_nfc_reader_get_tag_id} and check if the
 *    expected tag was found, if it was not found got back to step 1
 * 4. Call {@link tf_nfc_reader_request_page} with page number
 * 5. Wait for state to change to *ReaderRequestPageReady* (see {@link tf_nfc_reader_get_state}
 *    or {@link tf_nfc_register_reader_state_changed_callback} callback)
 * 6. Call {@link tf_nfc_reader_read_page} to retrieve the page from the buffer
 *
 * If you use a Mifare Classic tag you have to authenticate a page before you
 * can read it. See {@link tf_nfc_reader_authenticate_mifare_classic_page}.
 *
 * NFC Forum Type 4 tags are not organized into pages but different files. We currently
 * support two files: Capability Container file (CC) and NDEF file.
 *
 * Choose CC by setting page to 3 or NDEF by setting page to 4.
 */
int tf_nfc_reader_request_page(TF_NFC *nfc, uint16_t page, uint16_t length);

/**
 * \ingroup TF_NFC
 *
 * Returns the page data from an internal buffer. To fill the buffer
 * with specific pages you have to call {@link tf_nfc_reader_request_page} beforehand.
 */
int tf_nfc_reader_read_page_low_level(TF_NFC *nfc, uint16_t *ret_data_length, uint16_t *ret_data_chunk_offset, uint8_t ret_data_chunk_data[60]);

/**
 * \ingroup TF_NFC
 *
 * Returns the current cardemu state of the NFC Bricklet.
 *
 * On startup the Bricklet will be in the *CardemuInitialization* state. The
 * initialization will only take about 20ms. After that it changes to *CardemuIdle*.
 *
 * The Bricklet is also reinitialized if the mode is changed, see {@link tf_nfc_set_mode}.
 *
 * The functions of this Bricklet can be called in the *CardemuIdle* state and all of
 * the *CardemuReady* and *CardemuError* states.
 *
 * Example: If you call {@link tf_nfc_cardemu_start_discovery}, the state will change to
 * *CardemuDiscover* until the discovery is finished. Then it will change
 * to either *CardemuDiscoverReady* if it worked or to *CardemuDiscoverError* if it
 * didn't.
 *
 * The same approach is used analogously for the other API functions.
 */
int tf_nfc_cardemu_get_state(TF_NFC *nfc, uint8_t *ret_state, bool *ret_idle);

/**
 * \ingroup TF_NFC
 *
 * Starts the discovery process. If you call this function while a NFC
 * reader device is near to the NFC Bricklet the state will change from
 * *CardemuDiscovery* to *CardemuDiscoveryReady*.
 *
 * If no NFC reader device can be found or if there is an error during
 * discovery the cardemu state will change to *CardemuDiscoveryError*. In this case you
 * have to restart the discovery process.
 *
 * If the cardemu state changes to *CardemuDiscoveryReady* you can start the NDEF message
 * transfer with {@link tf_nfc_cardemu_write_ndef} and {@link tf_nfc_cardemu_start_transfer}.
 */
int tf_nfc_cardemu_start_discovery(TF_NFC *nfc);

/**
 * \ingroup TF_NFC
 *
 * Writes the NDEF message that is to be transferred to the NFC peer.
 *
 * The maximum supported NDEF message size in Cardemu mode is 255 byte.
 *
 * You can call this function at any time in Cardemu mode. The internal buffer
 * will not be overwritten until you call this function again or change the
 * mode.
 */
int tf_nfc_cardemu_write_ndef_low_level(TF_NFC *nfc, uint16_t ndef_length, uint16_t ndef_chunk_offset, const uint8_t ndef_chunk_data[60]);

/**
 * \ingroup TF_NFC
 *
 * You can start the transfer of a NDEF message if the cardemu state is *CardemuDiscoveryReady*.
 *
 * Before you call this function to start a write transfer, the NDEF message that
 * is to be transferred has to be written via {@link tf_nfc_cardemu_write_ndef} first.
 *
 * After you call this function the state will change to *CardemuTransferNDEF*. It will
 * change to *CardemuTransferNDEFReady* if the transfer was successful or
 * *CardemuTransferNDEFError* if it wasn't.
 */
int tf_nfc_cardemu_start_transfer(TF_NFC *nfc, uint8_t transfer);

/**
 * \ingroup TF_NFC
 *
 * Returns the current P2P state of the NFC Bricklet.
 *
 * On startup the Bricklet will be in the *P2PInitialization* state. The
 * initialization will only take about 20ms. After that it changes to *P2PIdle*.
 *
 * The Bricklet is also reinitialized if the mode is changed, see {@link tf_nfc_set_mode}.
 *
 * The functions of this Bricklet can be called in the *P2PIdle* state and all of
 * the *P2PReady* and *P2PError* states.
 *
 * Example: If you call {@link tf_nfc_p2p_start_discovery}, the state will change to
 * *P2PDiscover* until the discovery is finished. Then it will change
 * to either P2PDiscoverReady* if it worked or to *P2PDiscoverError* if it
 * didn't.
 *
 * The same approach is used analogously for the other API functions.
 */
int tf_nfc_p2p_get_state(TF_NFC *nfc, uint8_t *ret_state, bool *ret_idle);

/**
 * \ingroup TF_NFC
 *
 * Starts the discovery process. If you call this function while another NFC
 * P2P enabled device is near to the NFC Bricklet the state will change from
 * *P2PDiscovery* to *P2PDiscoveryReady*.
 *
 * If no NFC P2P enabled device can be found or if there is an error during
 * discovery the P2P state will change to *P2PDiscoveryError*. In this case you
 * have to restart the discovery process.
 *
 * If the P2P state changes to *P2PDiscoveryReady* you can start the NDEF message
 * transfer with {@link tf_nfc_p2p_start_transfer}.
 */
int tf_nfc_p2p_start_discovery(TF_NFC *nfc);

/**
 * \ingroup TF_NFC
 *
 * Writes the NDEF message that is to be transferred to the NFC peer.
 *
 * The maximum supported NDEF message size for P2P transfer is 255 byte.
 *
 * You can call this function at any time in P2P mode. The internal buffer
 * will not be overwritten until you call this function again, change the
 * mode or use P2P to read an NDEF messages.
 */
int tf_nfc_p2p_write_ndef_low_level(TF_NFC *nfc, uint16_t ndef_length, uint16_t ndef_chunk_offset, const uint8_t ndef_chunk_data[60]);

/**
 * \ingroup TF_NFC
 *
 * You can start the transfer of a NDEF message if the P2P state is *P2PDiscoveryReady*.
 *
 * Before you call this function to start a write transfer, the NDEF message that
 * is to be transferred has to be written via {@link tf_nfc_p2p_write_ndef} first.
 *
 * After you call this function the P2P state will change to *P2PTransferNDEF*. It will
 * change to *P2PTransferNDEFReady* if the transfer was successfull or
 * *P2PTransferNDEFError* if it wasn't.
 *
 * If you started a write transfer you are now done. If you started a read transfer
 * you can now use {@link tf_nfc_p2p_read_ndef} to read the NDEF message that was written
 * by the NFC peer.
 */
int tf_nfc_p2p_start_transfer(TF_NFC *nfc, uint8_t transfer);

/**
 * \ingroup TF_NFC
 *
 * Returns the NDEF message that was written by a NFC peer in NFC P2P mode.
 *
 * The NDEF message is ready if you called {@link tf_nfc_p2p_start_transfer} with a
 * read transfer and the P2P state changed to *P2PTransferNDEFReady*.
 */
int tf_nfc_p2p_read_ndef_low_level(TF_NFC *nfc, uint16_t *ret_ndef_length, uint16_t *ret_ndef_chunk_offset, uint8_t ret_ndef_chunk_data[60]);

/**
 * \ingroup TF_NFC
 *
 * Sets the detection LED configuration. By default the LED shows
 * if a card/reader is detected.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is off.
 */
int tf_nfc_set_detection_led_config(TF_NFC *nfc, uint8_t config);

/**
 * \ingroup TF_NFC
 *
 * Returns the configuration as set by {@link tf_nfc_set_detection_led_config}
 */
int tf_nfc_get_detection_led_config(TF_NFC *nfc, uint8_t *ret_config);

/**
 * \ingroup TF_NFC
 *
 * Sets the maximum timeout.
 *
 * This is a global maximum used for all internal state timeouts. The timeouts depend heavily
 * on the used tags etc. For example: If you use a Type 2 tag and you want to detect if
 * it is present, you have to use {@link tf_nfc_reader_request_tag_id} and wait for the state
 * to change to either the error state or the ready state.
 *
 * With the default configuration this takes 2-3 seconds. By setting the maximum timeout to
 * 100ms you can reduce this time to ~150-200ms. For Type 2 this would also still work
 * with a 20ms timeout (a Type 2 tag answers usually within 10ms). A type 4 tag can take
 * up to 500ms in our tests.
 *
 * If you need a fast response time to discover if a tag is present or not you can find
 * a good timeout value by trial and error for your specific tag.
 *
 * By default we use a very conservative timeout, to be sure that any tag can always
 * answer in time.
 *
 * .. versionadded:: 2.0.1$nbsp;(Plugin)
 */
int tf_nfc_set_maximum_timeout(TF_NFC *nfc, uint16_t timeout);

/**
 * \ingroup TF_NFC
 *
 * Returns the timeout as set by {@link tf_nfc_set_maximum_timeout}
 *
 * .. versionadded:: 2.0.1$nbsp;(Plugin)
 */
int tf_nfc_get_maximum_timeout(TF_NFC *nfc, uint16_t *ret_timeout);

/**
 * \ingroup TF_NFC
 *
 * Returns the tag type and tag ID from simple mode sorted by last seen time for a given index.
 *
 * Up to eight tags are saved.
 *
 * .. versionadded:: 2.0.6$nbsp;(Plugin)
 */
int tf_nfc_simple_get_tag_id_low_level(TF_NFC *nfc, uint8_t index, uint8_t *ret_tag_type, uint8_t *ret_tag_id_length, uint8_t ret_tag_id_data[10], uint32_t *ret_last_seen);

/**
 * \ingroup TF_NFC
 *
 * Sets the tag ID for cardemu mode. The tag ID can either have a length of 4 or 7.
 *
 * Set a length of 0 for random tag ID (default)
 *
 * .. versionadded:: 2.1.0$nbsp;(Plugin)
 */
int tf_nfc_cardemu_set_tag_id(TF_NFC *nfc, uint8_t tag_id_length, const uint8_t tag_id_data[7]);

/**
 * \ingroup TF_NFC
 *
 * Returns the tag ID and length as set by {@link tf_nfc_cardemu_set_tag_id}.
 *
 * .. versionadded:: 2.1.0$nbsp;(Plugin)
 */
int tf_nfc_cardemu_get_tag_id(TF_NFC *nfc, uint8_t *ret_tag_id_length, uint8_t ret_tag_id_data[7]);

/**
 * \ingroup TF_NFC
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
int tf_nfc_get_spitfp_error_count(TF_NFC *nfc, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow);

/**
 * \ingroup TF_NFC
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
int tf_nfc_set_bootloader_mode(TF_NFC *nfc, uint8_t mode, uint8_t *ret_status);

/**
 * \ingroup TF_NFC
 *
 * Returns the current bootloader mode, see {@link tf_nfc_set_bootloader_mode}.
 */
int tf_nfc_get_bootloader_mode(TF_NFC *nfc, uint8_t *ret_mode);

/**
 * \ingroup TF_NFC
 *
 * Sets the firmware pointer for {@link tf_nfc_write_firmware}. The pointer has
 * to be increased by chunks of size 64. The data is written to flash
 * every 4 chunks (which equals to one page of size 256).
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_nfc_set_write_firmware_pointer(TF_NFC *nfc, uint32_t pointer);

/**
 * \ingroup TF_NFC
 *
 * Writes 64 Bytes of firmware at the position as written by
 * {@link tf_nfc_set_write_firmware_pointer} before. The firmware is written
 * to flash every 4 chunks.
 *
 * You can only write firmware in bootloader mode.
 *
 * This function is used by Brick Viewer during flashing. It should not be
 * necessary to call it in a normal user program.
 */
int tf_nfc_write_firmware(TF_NFC *nfc, const uint8_t data[64], uint8_t *ret_status);

/**
 * \ingroup TF_NFC
 *
 * Sets the status LED configuration. By default the LED shows
 * communication traffic between Brick and Bricklet, it flickers once
 * for every 10 received data packets.
 *
 * You can also turn the LED permanently on/off or show a heartbeat.
 *
 * If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
 */
int tf_nfc_set_status_led_config(TF_NFC *nfc, uint8_t config);

/**
 * \ingroup TF_NFC
 *
 * Returns the configuration as set by {@link tf_nfc_set_status_led_config}
 */
int tf_nfc_get_status_led_config(TF_NFC *nfc, uint8_t *ret_config);

/**
 * \ingroup TF_NFC
 *
 * Returns the temperature as measured inside the microcontroller. The
 * value returned is not the ambient temperature!
 *
 * The temperature is only proportional to the real temperature and it has bad
 * accuracy. Practically it is only useful as an indicator for
 * temperature changes.
 */
int tf_nfc_get_chip_temperature(TF_NFC *nfc, int16_t *ret_temperature);

/**
 * \ingroup TF_NFC
 *
 * Calling this function will reset the Bricklet. All configurations
 * will be lost.
 *
 * After a reset you have to create new device objects,
 * calling functions on the existing ones will result in
 * undefined behavior!
 */
int tf_nfc_reset(TF_NFC *nfc);

/**
 * \ingroup TF_NFC
 *
 * Writes a new UID into flash. If you want to set a new UID
 * you have to decode the Base58 encoded UID string into an
 * integer first.
 *
 * We recommend that you use Brick Viewer to change the UID.
 */
int tf_nfc_write_uid(TF_NFC *nfc, uint32_t uid);

/**
 * \ingroup TF_NFC
 *
 * Returns the current UID as an integer. Encode as
 * Base58 to get the usual string version.
 */
int tf_nfc_read_uid(TF_NFC *nfc, uint32_t *ret_uid);

/**
 * \ingroup TF_NFC
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
int tf_nfc_get_identity(TF_NFC *nfc, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier);

/**
 * \ingroup TF_NFC
 *
 * Returns the tag type and the tag ID. This function can only be called if the
 * NFC Bricklet is currently in one of the *ReaderReady* states. The returned tag ID
 * is the tag ID that was saved through the last call of {@link tf_nfc_reader_request_tag_id}.
 *
 * To get the tag ID of a tag the approach is as follows:
 *
 * 1. Call {@link tf_nfc_reader_request_tag_id}
 * 2. Wait for state to change to *ReaderRequestTagIDReady* (see {@link tf_nfc_reader_get_state} or
 *    {@link tf_nfc_register_reader_state_changed_callback} callback)
 * 3. Call {@link tf_nfc_reader_get_tag_id}
 */
int tf_nfc_reader_get_tag_id(TF_NFC *nfc, uint8_t *ret_tag_type, uint8_t *ret_tag_id, uint8_t *ret_tag_id_length);

/**
 * \ingroup TF_NFC
 *
 * Writes NDEF formated data.
 *
 * This function currently supports NFC Forum Type 2, 4, 5 and Mifare Classic.
 *
 * The general approach for writing a NDEF message is as follows:
 *
 * 1. Call {@link tf_nfc_reader_request_tag_id}
 * 2. Wait for state to change to *ReaderRequestTagIDReady* (see
 *    {@link tf_nfc_reader_get_state} or {@link tf_nfc_register_reader_state_changed_callback} callback)
 * 3. If looking for a specific tag then call {@link tf_nfc_reader_get_tag_id} and check
 *    if the expected tag was found, if it was not found got back to step 1
 * 4. Call {@link tf_nfc_reader_write_ndef} with the NDEF message that you want to write
 * 5. Wait for state to change to *ReaderWriteNDEFReady* (see {@link tf_nfc_reader_get_state}
 *    or {@link tf_nfc_register_reader_state_changed_callback} callback)
 */
int tf_nfc_reader_write_ndef(TF_NFC *nfc, const uint8_t *ndef, uint16_t ndef_length);

/**
 * \ingroup TF_NFC
 *
 * Returns the NDEF data from an internal buffer. To fill the buffer
 * with a NDEF message you have to call {@link tf_nfc_reader_request_ndef} beforehand.
 */
int tf_nfc_reader_read_ndef(TF_NFC *nfc, uint8_t *ret_ndef, uint16_t *ret_ndef_length);

/**
 * \ingroup TF_NFC
 *
 * Writes a maximum of 8192 bytes starting from the given page. How many pages are written
 * depends on the tag type. The page sizes are as follows:
 *
 * * Mifare Classic page size: 16 byte
 * * NFC Forum Type 1 page size: 8 byte
 * * NFC Forum Type 2 page size: 4 byte
 * * NFC Forum Type 3 page size: 16 byte
 * * NFC Forum Type 4: No pages, page = file selection (CC or NDEF, see below)
 * * NFC Forum Type 5 page size: 4 byte
 *
 * The general approach for writing to a tag is as follows:
 *
 * 1. Call {@link tf_nfc_reader_request_tag_id}
 * 2. Wait for state to change to *ReaderRequestTagIDReady* (see {@link tf_nfc_reader_get_state} or
 *    {@link tf_nfc_register_reader_state_changed_callback} callback)
 * 3. If looking for a specific tag then call {@link tf_nfc_reader_get_tag_id} and check if the
 *    expected tag was found, if it was not found got back to step 1
 * 4. Call {@link tf_nfc_reader_write_page} with page number and data
 * 5. Wait for state to change to *ReaderWritePageReady* (see {@link tf_nfc_reader_get_state} or
 *    {@link tf_nfc_register_reader_state_changed_callback} callback)
 *
 * If you use a Mifare Classic tag you have to authenticate a page before you
 * can write to it. See {@link tf_nfc_reader_authenticate_mifare_classic_page}.
 *
 * NFC Forum Type 4 tags are not organized into pages but different files. We currently
 * support two files: Capability Container file (CC) and NDEF file.
 *
 * Choose CC by setting page to 3 or NDEF by setting page to 4.
 */
int tf_nfc_reader_write_page(TF_NFC *nfc, uint16_t page, const uint8_t *data, uint16_t data_length);

/**
 * \ingroup TF_NFC
 *
 * Returns the page data from an internal buffer. To fill the buffer
 * with specific pages you have to call {@link tf_nfc_reader_request_page} beforehand.
 */
int tf_nfc_reader_read_page(TF_NFC *nfc, uint8_t *ret_data, uint16_t *ret_data_length);

/**
 * \ingroup TF_NFC
 *
 * Writes the NDEF message that is to be transferred to the NFC peer.
 *
 * The maximum supported NDEF message size in Cardemu mode is 255 byte.
 *
 * You can call this function at any time in Cardemu mode. The internal buffer
 * will not be overwritten until you call this function again or change the
 * mode.
 */
int tf_nfc_cardemu_write_ndef(TF_NFC *nfc, const uint8_t *ndef, uint16_t ndef_length);

/**
 * \ingroup TF_NFC
 *
 * Writes the NDEF message that is to be transferred to the NFC peer.
 *
 * The maximum supported NDEF message size for P2P transfer is 255 byte.
 *
 * You can call this function at any time in P2P mode. The internal buffer
 * will not be overwritten until you call this function again, change the
 * mode or use P2P to read an NDEF messages.
 */
int tf_nfc_p2p_write_ndef(TF_NFC *nfc, const uint8_t *ndef, uint16_t ndef_length);

/**
 * \ingroup TF_NFC
 *
 * Returns the NDEF message that was written by a NFC peer in NFC P2P mode.
 *
 * The NDEF message is ready if you called {@link tf_nfc_p2p_start_transfer} with a
 * read transfer and the P2P state changed to *P2PTransferNDEFReady*.
 */
int tf_nfc_p2p_read_ndef(TF_NFC *nfc, uint8_t *ret_ndef, uint16_t *ret_ndef_length);

/**
 * \ingroup TF_NFC
 *
 * Returns the tag type and tag ID from simple mode sorted by last seen time for a given index.
 *
 * Up to eight tags are saved.
 *
 * .. versionadded:: 2.0.6$nbsp;(Plugin)
 */
int tf_nfc_simple_get_tag_id(TF_NFC *nfc, uint8_t index, uint8_t *ret_tag_type, uint8_t *ret_tag_id, uint8_t *ret_tag_id_length, uint32_t *ret_last_seen);

#ifdef __cplusplus
}
#endif

#endif
