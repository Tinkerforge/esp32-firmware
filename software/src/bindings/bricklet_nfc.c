/* ***********************************************************
 * This file was automatically generated on 2024-10-25.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_nfc.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_nfc_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_NFC *nfc = (TF_NFC *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(nfc->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_NFC_CALLBACK_READER_STATE_CHANGED: {
            TF_NFC_ReaderStateChangedHandler fn = nfc->reader_state_changed_handler;
            void *user_data = nfc->reader_state_changed_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t state = tf_packet_buffer_read_uint8_t(payload);
            bool idle = tf_packet_buffer_read_bool(payload);
            hal_common->locked = true;
            fn(nfc, state, idle, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_NFC_CALLBACK_CARDEMU_STATE_CHANGED: {
            TF_NFC_CardemuStateChangedHandler fn = nfc->cardemu_state_changed_handler;
            void *user_data = nfc->cardemu_state_changed_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t state = tf_packet_buffer_read_uint8_t(payload);
            bool idle = tf_packet_buffer_read_bool(payload);
            hal_common->locked = true;
            fn(nfc, state, idle, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_NFC_CALLBACK_P2P_STATE_CHANGED: {
            TF_NFC_P2PStateChangedHandler fn = nfc->p2p_state_changed_handler;
            void *user_data = nfc->p2p_state_changed_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t state = tf_packet_buffer_read_uint8_t(payload);
            bool idle = tf_packet_buffer_read_bool(payload);
            hal_common->locked = true;
            fn(nfc, state, idle, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_nfc_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_nfc_create(TF_NFC *nfc, const char *uid_or_port_name, TF_HAL *hal) {
    if (nfc == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(nfc, 0, sizeof(TF_NFC));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_NFC_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    nfc->tfp = tfp;
    nfc->tfp->device = nfc;
    nfc->tfp->cb_handler = tf_nfc_callback_handler;
    nfc->magic = 0x5446;
    nfc->response_expected[0] = 0x24;
    nfc->response_expected[1] = 0x09;
    nfc->response_expected[2] = 0x00;
    return TF_E_OK;
}

int tf_nfc_destroy(TF_NFC *nfc) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }
    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    nfc->tfp->cb_handler = NULL;
    nfc->tfp->device = NULL;
    nfc->tfp = NULL;
    nfc->magic = 0;

    return TF_E_OK;
}

int tf_nfc_get_response_expected(TF_NFC *nfc, uint8_t function_id, bool *ret_response_expected) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_NFC_FUNCTION_SET_MODE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_READER_REQUEST_TAG_ID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_READER_WRITE_NDEF_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_READER_REQUEST_NDEF:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_READER_AUTHENTICATE_MIFARE_CLASSIC_PAGE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_READER_WRITE_PAGE_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_READER_REQUEST_PAGE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_CARDEMU_START_DISCOVERY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_CARDEMU_WRITE_NDEF_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_CARDEMU_START_TRANSFER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_P2P_START_DISCOVERY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_P2P_WRITE_NDEF_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_P2P_START_TRANSFER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[1] & (1 << 4)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_SET_DETECTION_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[1] & (1 << 5)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_SET_MAXIMUM_TIMEOUT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[1] & (1 << 6)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_CARDEMU_SET_TAG_ID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[1] & (1 << 7)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[2] & (1 << 0)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[2] & (1 << 1)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[2] & (1 << 2)) != 0;
            }
            break;
        case TF_NFC_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (nfc->response_expected[2] & (1 << 3)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_nfc_set_response_expected(TF_NFC *nfc, uint8_t function_id, bool response_expected) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_NFC_FUNCTION_SET_MODE:
            if (response_expected) {
                nfc->response_expected[0] |= (1 << 0);
            } else {
                nfc->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_NFC_FUNCTION_READER_REQUEST_TAG_ID:
            if (response_expected) {
                nfc->response_expected[0] |= (1 << 1);
            } else {
                nfc->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_NFC_FUNCTION_READER_WRITE_NDEF_LOW_LEVEL:
            if (response_expected) {
                nfc->response_expected[0] |= (1 << 2);
            } else {
                nfc->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_NFC_FUNCTION_READER_REQUEST_NDEF:
            if (response_expected) {
                nfc->response_expected[0] |= (1 << 3);
            } else {
                nfc->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_NFC_FUNCTION_READER_AUTHENTICATE_MIFARE_CLASSIC_PAGE:
            if (response_expected) {
                nfc->response_expected[0] |= (1 << 4);
            } else {
                nfc->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_NFC_FUNCTION_READER_WRITE_PAGE_LOW_LEVEL:
            if (response_expected) {
                nfc->response_expected[0] |= (1 << 5);
            } else {
                nfc->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_NFC_FUNCTION_READER_REQUEST_PAGE:
            if (response_expected) {
                nfc->response_expected[0] |= (1 << 6);
            } else {
                nfc->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_NFC_FUNCTION_CARDEMU_START_DISCOVERY:
            if (response_expected) {
                nfc->response_expected[0] |= (1 << 7);
            } else {
                nfc->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_NFC_FUNCTION_CARDEMU_WRITE_NDEF_LOW_LEVEL:
            if (response_expected) {
                nfc->response_expected[1] |= (1 << 0);
            } else {
                nfc->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_NFC_FUNCTION_CARDEMU_START_TRANSFER:
            if (response_expected) {
                nfc->response_expected[1] |= (1 << 1);
            } else {
                nfc->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_NFC_FUNCTION_P2P_START_DISCOVERY:
            if (response_expected) {
                nfc->response_expected[1] |= (1 << 2);
            } else {
                nfc->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_NFC_FUNCTION_P2P_WRITE_NDEF_LOW_LEVEL:
            if (response_expected) {
                nfc->response_expected[1] |= (1 << 3);
            } else {
                nfc->response_expected[1] &= ~(1 << 3);
            }
            break;
        case TF_NFC_FUNCTION_P2P_START_TRANSFER:
            if (response_expected) {
                nfc->response_expected[1] |= (1 << 4);
            } else {
                nfc->response_expected[1] &= ~(1 << 4);
            }
            break;
        case TF_NFC_FUNCTION_SET_DETECTION_LED_CONFIG:
            if (response_expected) {
                nfc->response_expected[1] |= (1 << 5);
            } else {
                nfc->response_expected[1] &= ~(1 << 5);
            }
            break;
        case TF_NFC_FUNCTION_SET_MAXIMUM_TIMEOUT:
            if (response_expected) {
                nfc->response_expected[1] |= (1 << 6);
            } else {
                nfc->response_expected[1] &= ~(1 << 6);
            }
            break;
        case TF_NFC_FUNCTION_CARDEMU_SET_TAG_ID:
            if (response_expected) {
                nfc->response_expected[1] |= (1 << 7);
            } else {
                nfc->response_expected[1] &= ~(1 << 7);
            }
            break;
        case TF_NFC_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                nfc->response_expected[2] |= (1 << 0);
            } else {
                nfc->response_expected[2] &= ~(1 << 0);
            }
            break;
        case TF_NFC_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                nfc->response_expected[2] |= (1 << 1);
            } else {
                nfc->response_expected[2] &= ~(1 << 1);
            }
            break;
        case TF_NFC_FUNCTION_RESET:
            if (response_expected) {
                nfc->response_expected[2] |= (1 << 2);
            } else {
                nfc->response_expected[2] &= ~(1 << 2);
            }
            break;
        case TF_NFC_FUNCTION_WRITE_UID:
            if (response_expected) {
                nfc->response_expected[2] |= (1 << 3);
            } else {
                nfc->response_expected[2] &= ~(1 << 3);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_nfc_set_response_expected_all(TF_NFC *nfc, bool response_expected) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(nfc->response_expected, response_expected ? 0xFF : 0, 3);

    return TF_E_OK;
}

int tf_nfc_set_mode(TF_NFC *nfc, uint8_t mode) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_SET_MODE, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_SET_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_get_mode(TF_NFC *nfc, uint8_t *ret_mode) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_GET_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_reader_request_tag_id(TF_NFC *nfc) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_READER_REQUEST_TAG_ID, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_READER_REQUEST_TAG_ID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_reader_get_tag_id_low_level(TF_NFC *nfc, uint8_t *ret_tag_type, uint8_t *ret_tag_id_length, uint8_t ret_tag_id_data[32]) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_READER_GET_TAG_ID_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 34) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_tag_type != NULL) { *ret_tag_type = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_tag_id_length != NULL) { *ret_tag_id_length = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_tag_id_data != NULL) { for (_i = 0; _i < 32; ++_i) ret_tag_id_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 32); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 34) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_reader_get_state(TF_NFC *nfc, uint8_t *ret_state, bool *ret_idle) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_READER_GET_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_state != NULL) { *ret_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_idle != NULL) { *ret_idle = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_reader_write_ndef_low_level(TF_NFC *nfc, uint16_t ndef_length, uint16_t ndef_chunk_offset, const uint8_t ndef_chunk_data[60]) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_READER_WRITE_NDEF_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_READER_WRITE_NDEF_LOW_LEVEL, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    ndef_length = tf_leconvert_uint16_to(ndef_length); memcpy(_send_buf + 0, &ndef_length, 2);
    ndef_chunk_offset = tf_leconvert_uint16_to(ndef_chunk_offset); memcpy(_send_buf + 2, &ndef_chunk_offset, 2);
    memcpy(_send_buf + 4, ndef_chunk_data, 60);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_reader_request_ndef(TF_NFC *nfc) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_READER_REQUEST_NDEF, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_READER_REQUEST_NDEF, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_reader_read_ndef_low_level(TF_NFC *nfc, uint16_t *ret_ndef_length, uint16_t *ret_ndef_chunk_offset, uint8_t ret_ndef_chunk_data[60]) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_READER_READ_NDEF_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_ndef_length != NULL) { *ret_ndef_length = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_ndef_chunk_offset != NULL) { *ret_ndef_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_ndef_chunk_data != NULL) { for (_i = 0; _i < 60; ++_i) ret_ndef_chunk_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_reader_authenticate_mifare_classic_page(TF_NFC *nfc, uint16_t page, uint8_t key_number, const uint8_t key[6]) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_READER_AUTHENTICATE_MIFARE_CLASSIC_PAGE, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_READER_AUTHENTICATE_MIFARE_CLASSIC_PAGE, 9, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    page = tf_leconvert_uint16_to(page); memcpy(_send_buf + 0, &page, 2);
    _send_buf[2] = (uint8_t)key_number;
    memcpy(_send_buf + 3, key, 6);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_reader_write_page_low_level(TF_NFC *nfc, uint16_t page, uint16_t data_length, uint16_t data_chunk_offset, const uint8_t data_chunk_data[58]) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_READER_WRITE_PAGE_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_READER_WRITE_PAGE_LOW_LEVEL, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    page = tf_leconvert_uint16_to(page); memcpy(_send_buf + 0, &page, 2);
    data_length = tf_leconvert_uint16_to(data_length); memcpy(_send_buf + 2, &data_length, 2);
    data_chunk_offset = tf_leconvert_uint16_to(data_chunk_offset); memcpy(_send_buf + 4, &data_chunk_offset, 2);
    memcpy(_send_buf + 6, data_chunk_data, 58);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_reader_request_page(TF_NFC *nfc, uint16_t page, uint16_t length) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_READER_REQUEST_PAGE, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_READER_REQUEST_PAGE, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    page = tf_leconvert_uint16_to(page); memcpy(_send_buf + 0, &page, 2);
    length = tf_leconvert_uint16_to(length); memcpy(_send_buf + 2, &length, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_reader_read_page_low_level(TF_NFC *nfc, uint16_t *ret_data_length, uint16_t *ret_data_chunk_offset, uint8_t ret_data_chunk_data[60]) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_READER_READ_PAGE_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_data_length != NULL) { *ret_data_length = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_data_chunk_offset != NULL) { *ret_data_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_data_chunk_data != NULL) { for (_i = 0; _i < 60; ++_i) ret_data_chunk_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_cardemu_get_state(TF_NFC *nfc, uint8_t *ret_state, bool *ret_idle) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_CARDEMU_GET_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_state != NULL) { *ret_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_idle != NULL) { *ret_idle = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_cardemu_start_discovery(TF_NFC *nfc) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_CARDEMU_START_DISCOVERY, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_CARDEMU_START_DISCOVERY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_cardemu_write_ndef_low_level(TF_NFC *nfc, uint16_t ndef_length, uint16_t ndef_chunk_offset, const uint8_t ndef_chunk_data[60]) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_CARDEMU_WRITE_NDEF_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_CARDEMU_WRITE_NDEF_LOW_LEVEL, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    ndef_length = tf_leconvert_uint16_to(ndef_length); memcpy(_send_buf + 0, &ndef_length, 2);
    ndef_chunk_offset = tf_leconvert_uint16_to(ndef_chunk_offset); memcpy(_send_buf + 2, &ndef_chunk_offset, 2);
    memcpy(_send_buf + 4, ndef_chunk_data, 60);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_cardemu_start_transfer(TF_NFC *nfc, uint8_t transfer) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_CARDEMU_START_TRANSFER, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_CARDEMU_START_TRANSFER, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    _send_buf[0] = (uint8_t)transfer;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_p2p_get_state(TF_NFC *nfc, uint8_t *ret_state, bool *ret_idle) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_P2P_GET_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_state != NULL) { *ret_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_idle != NULL) { *ret_idle = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_p2p_start_discovery(TF_NFC *nfc) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_P2P_START_DISCOVERY, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_P2P_START_DISCOVERY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_p2p_write_ndef_low_level(TF_NFC *nfc, uint16_t ndef_length, uint16_t ndef_chunk_offset, const uint8_t ndef_chunk_data[60]) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_P2P_WRITE_NDEF_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_P2P_WRITE_NDEF_LOW_LEVEL, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    ndef_length = tf_leconvert_uint16_to(ndef_length); memcpy(_send_buf + 0, &ndef_length, 2);
    ndef_chunk_offset = tf_leconvert_uint16_to(ndef_chunk_offset); memcpy(_send_buf + 2, &ndef_chunk_offset, 2);
    memcpy(_send_buf + 4, ndef_chunk_data, 60);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_p2p_start_transfer(TF_NFC *nfc, uint8_t transfer) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_P2P_START_TRANSFER, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_P2P_START_TRANSFER, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    _send_buf[0] = (uint8_t)transfer;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_p2p_read_ndef_low_level(TF_NFC *nfc, uint16_t *ret_ndef_length, uint16_t *ret_ndef_chunk_offset, uint8_t ret_ndef_chunk_data[60]) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_P2P_READ_NDEF_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_ndef_length != NULL) { *ret_ndef_length = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_ndef_chunk_offset != NULL) { *ret_ndef_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_ndef_chunk_data != NULL) { for (_i = 0; _i < 60; ++_i) ret_ndef_chunk_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_set_detection_led_config(TF_NFC *nfc, uint8_t config) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_SET_DETECTION_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_SET_DETECTION_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_get_detection_led_config(TF_NFC *nfc, uint8_t *ret_config) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_GET_DETECTION_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_set_maximum_timeout(TF_NFC *nfc, uint16_t timeout) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_SET_MAXIMUM_TIMEOUT, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_SET_MAXIMUM_TIMEOUT, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    timeout = tf_leconvert_uint16_to(timeout); memcpy(_send_buf + 0, &timeout, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_get_maximum_timeout(TF_NFC *nfc, uint16_t *ret_timeout) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_GET_MAXIMUM_TIMEOUT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_timeout != NULL) { *ret_timeout = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_simple_get_tag_id_low_level(TF_NFC *nfc, uint8_t index, uint8_t *ret_tag_type, uint8_t *ret_tag_id_length, uint8_t ret_tag_id_data[10], uint32_t *ret_last_seen) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_SIMPLE_GET_TAG_ID_LOW_LEVEL, 1, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_tag_type != NULL) { *ret_tag_type = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_tag_id_length != NULL) { *ret_tag_id_length = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_tag_id_data != NULL) { for (_i = 0; _i < 10; ++_i) ret_tag_id_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 10); }
            if (ret_last_seen != NULL) { *ret_last_seen = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_cardemu_set_tag_id(TF_NFC *nfc, uint8_t tag_id_length, const uint8_t tag_id_data[7]) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_CARDEMU_SET_TAG_ID, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_CARDEMU_SET_TAG_ID, 8, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    _send_buf[0] = (uint8_t)tag_id_length;
    memcpy(_send_buf + 1, tag_id_data, 7);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_cardemu_get_tag_id(TF_NFC *nfc, uint8_t *ret_tag_id_length, uint8_t ret_tag_id_data[7]) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_CARDEMU_GET_TAG_ID, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_tag_id_length != NULL) { *ret_tag_id_length = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_tag_id_data != NULL) { for (_i = 0; _i < 7; ++_i) ret_tag_id_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 7); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_get_spitfp_error_count(TF_NFC *nfc, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_set_bootloader_mode(TF_NFC *nfc, uint8_t mode, uint8_t *ret_status) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_get_bootloader_mode(TF_NFC *nfc, uint8_t *ret_mode) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_set_write_firmware_pointer(TF_NFC *nfc, uint32_t pointer) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_write_firmware(TF_NFC *nfc, const uint8_t data[64], uint8_t *ret_status) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_set_status_led_config(TF_NFC *nfc, uint8_t config) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_get_status_led_config(TF_NFC *nfc, uint8_t *ret_config) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_get_chip_temperature(TF_NFC *nfc, int16_t *ret_temperature) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_reset(TF_NFC *nfc) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_write_uid(TF_NFC *nfc, uint32_t uid) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_nfc_get_response_expected(nfc, TF_NFC_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(nfc->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_read_uid(TF_NFC *nfc, uint32_t *ret_uid) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_get_identity(TF_NFC *nfc, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = nfc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(nfc->tfp, TF_NFC_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(nfc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(nfc->tfp);
        if (_error_code != 0 || _length != 25) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { tf_packet_buffer_pop_n(_recv_buf, (uint8_t *)ret_uid, 8);} else { tf_packet_buffer_remove(_recv_buf, 8); }
            if (ret_connected_uid != NULL) { tf_packet_buffer_pop_n(_recv_buf, (uint8_t *)ret_connected_uid, 8);} else { tf_packet_buffer_remove(_recv_buf, 8); }
            if (ret_position != NULL) { *ret_position = tf_packet_buffer_read_char(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_hardware_version != NULL) { for (_i = 0; _i < 3; ++_i) ret_hardware_version[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 3); }
            if (ret_firmware_version != NULL) { for (_i = 0; _i < 3; ++_i) ret_firmware_version[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 3); }
            if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(nfc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(nfc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_nfc_reader_get_tag_id(TF_NFC *nfc, uint8_t *ret_tag_type, uint8_t *ret_tag_id, uint8_t *ret_tag_id_length) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    int ret = TF_E_OK;
    uint8_t tag_id_length = 0;
    uint8_t tag_id_data[32];

    if (ret_tag_id_length != NULL) {
        *ret_tag_id_length = 0;
    }

    ret = tf_nfc_reader_get_tag_id_low_level(nfc, ret_tag_type, &tag_id_length, tag_id_data);

    if (ret != TF_E_OK) {
        return ret;
    }

    if (ret_tag_id != NULL) {
        memcpy(ret_tag_id, tag_id_data, sizeof(uint8_t) * tag_id_length);
        memset(&ret_tag_id[tag_id_length], 0, sizeof(uint8_t) * (32 - tag_id_length));
    }

    if (ret_tag_id_length != NULL) {
        *ret_tag_id_length = tag_id_length;
    }

    return ret;
}

static int tf_nfc_reader_write_ndef_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    (void)wrapper_data;
    uint16_t ndef_chunk_offset = (uint16_t)chunk_offset;
    uint16_t ndef_length = (uint16_t)stream_length;
    uint32_t ndef_chunk_written = 60;

    uint8_t *ndef_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_nfc_reader_write_ndef_low_level((TF_NFC *)device, ndef_length, ndef_chunk_offset, ndef_chunk_data);

    *ret_chunk_written = (uint32_t) ndef_chunk_written;
    return ret;
}

int tf_nfc_reader_write_ndef(TF_NFC *nfc, const uint8_t *ndef, uint16_t ndef_length) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }


    uint32_t _stream_length = ndef_length;
    uint32_t _ndef_written = 0;
    uint8_t _chunk_data[60];

    int ret = tf_stream_in(nfc, tf_nfc_reader_write_ndef_ll_wrapper, NULL, ndef, _stream_length, _chunk_data, &_ndef_written, 60, tf_copy_items_uint8_t);



    return ret;
}


static int tf_nfc_reader_read_ndef_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    (void)wrapper_data;
    uint16_t ndef_length = 0;
    uint16_t ndef_chunk_offset = 0;
    uint8_t *ndef_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_nfc_reader_read_ndef_low_level((TF_NFC *)device, &ndef_length, &ndef_chunk_offset, ndef_chunk_data);

    *ret_stream_length = (uint32_t)ndef_length;
    *ret_chunk_offset = (uint32_t)ndef_chunk_offset;
    return ret;
}

int tf_nfc_reader_read_ndef(TF_NFC *nfc, uint8_t *ret_ndef, uint16_t *ret_ndef_length) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    uint32_t _ndef_length = 0;
    uint8_t _ndef_chunk_data[60];

    int ret = tf_stream_out(nfc, tf_nfc_reader_read_ndef_ll_wrapper, NULL, ret_ndef, &_ndef_length, _ndef_chunk_data, 60, tf_copy_items_uint8_t);

    if (ret_ndef_length != NULL) {
        *ret_ndef_length = (uint16_t)_ndef_length;
    }
    return ret;
}

typedef struct TF_NFC_ReaderWritePageLLWrapperData {
    uint16_t page;
} TF_NFC_ReaderWritePageLLWrapperData;


static int tf_nfc_reader_write_page_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    TF_NFC_ReaderWritePageLLWrapperData *data = (TF_NFC_ReaderWritePageLLWrapperData *) wrapper_data;
    uint16_t data_chunk_offset = (uint16_t)chunk_offset;
    uint16_t data_length = (uint16_t)stream_length;
    uint32_t data_chunk_written = 58;

    uint8_t *data_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_nfc_reader_write_page_low_level((TF_NFC *)device, data->page, data_length, data_chunk_offset, data_chunk_data);

    *ret_chunk_written = (uint32_t) data_chunk_written;
    return ret;
}

int tf_nfc_reader_write_page(TF_NFC *nfc, uint16_t page, const uint8_t *data, uint16_t data_length) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_NFC_ReaderWritePageLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.page = page;

    uint32_t _stream_length = data_length;
    uint32_t _data_written = 0;
    uint8_t _chunk_data[58];

    int ret = tf_stream_in(nfc, tf_nfc_reader_write_page_ll_wrapper, &_wrapper_data, data, _stream_length, _chunk_data, &_data_written, 58, tf_copy_items_uint8_t);



    return ret;
}


static int tf_nfc_reader_read_page_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    (void)wrapper_data;
    uint16_t data_length = 0;
    uint16_t data_chunk_offset = 0;
    uint8_t *data_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_nfc_reader_read_page_low_level((TF_NFC *)device, &data_length, &data_chunk_offset, data_chunk_data);

    *ret_stream_length = (uint32_t)data_length;
    *ret_chunk_offset = (uint32_t)data_chunk_offset;
    return ret;
}

int tf_nfc_reader_read_page(TF_NFC *nfc, uint8_t *ret_data, uint16_t *ret_data_length) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    uint32_t _data_length = 0;
    uint8_t _data_chunk_data[60];

    int ret = tf_stream_out(nfc, tf_nfc_reader_read_page_ll_wrapper, NULL, ret_data, &_data_length, _data_chunk_data, 60, tf_copy_items_uint8_t);

    if (ret_data_length != NULL) {
        *ret_data_length = (uint16_t)_data_length;
    }
    return ret;
}

static int tf_nfc_cardemu_write_ndef_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    (void)wrapper_data;
    uint16_t ndef_chunk_offset = (uint16_t)chunk_offset;
    uint16_t ndef_length = (uint16_t)stream_length;
    uint32_t ndef_chunk_written = 60;

    uint8_t *ndef_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_nfc_cardemu_write_ndef_low_level((TF_NFC *)device, ndef_length, ndef_chunk_offset, ndef_chunk_data);

    *ret_chunk_written = (uint32_t) ndef_chunk_written;
    return ret;
}

int tf_nfc_cardemu_write_ndef(TF_NFC *nfc, const uint8_t *ndef, uint16_t ndef_length) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }


    uint32_t _stream_length = ndef_length;
    uint32_t _ndef_written = 0;
    uint8_t _chunk_data[60];

    int ret = tf_stream_in(nfc, tf_nfc_cardemu_write_ndef_ll_wrapper, NULL, ndef, _stream_length, _chunk_data, &_ndef_written, 60, tf_copy_items_uint8_t);



    return ret;
}


static int tf_nfc_p2p_write_ndef_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    (void)wrapper_data;
    uint16_t ndef_chunk_offset = (uint16_t)chunk_offset;
    uint16_t ndef_length = (uint16_t)stream_length;
    uint32_t ndef_chunk_written = 60;

    uint8_t *ndef_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_nfc_p2p_write_ndef_low_level((TF_NFC *)device, ndef_length, ndef_chunk_offset, ndef_chunk_data);

    *ret_chunk_written = (uint32_t) ndef_chunk_written;
    return ret;
}

int tf_nfc_p2p_write_ndef(TF_NFC *nfc, const uint8_t *ndef, uint16_t ndef_length) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }


    uint32_t _stream_length = ndef_length;
    uint32_t _ndef_written = 0;
    uint8_t _chunk_data[60];

    int ret = tf_stream_in(nfc, tf_nfc_p2p_write_ndef_ll_wrapper, NULL, ndef, _stream_length, _chunk_data, &_ndef_written, 60, tf_copy_items_uint8_t);



    return ret;
}


static int tf_nfc_p2p_read_ndef_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    (void)wrapper_data;
    uint16_t ndef_length = 0;
    uint16_t ndef_chunk_offset = 0;
    uint8_t *ndef_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_nfc_p2p_read_ndef_low_level((TF_NFC *)device, &ndef_length, &ndef_chunk_offset, ndef_chunk_data);

    *ret_stream_length = (uint32_t)ndef_length;
    *ret_chunk_offset = (uint32_t)ndef_chunk_offset;
    return ret;
}

int tf_nfc_p2p_read_ndef(TF_NFC *nfc, uint8_t *ret_ndef, uint16_t *ret_ndef_length) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    uint32_t _ndef_length = 0;
    uint8_t _ndef_chunk_data[60];

    int ret = tf_stream_out(nfc, tf_nfc_p2p_read_ndef_ll_wrapper, NULL, ret_ndef, &_ndef_length, _ndef_chunk_data, 60, tf_copy_items_uint8_t);

    if (ret_ndef_length != NULL) {
        *ret_ndef_length = (uint16_t)_ndef_length;
    }
    return ret;
}

int tf_nfc_simple_get_tag_id(TF_NFC *nfc, uint8_t index, uint8_t *ret_tag_type, uint8_t *ret_tag_id, uint8_t *ret_tag_id_length, uint32_t *ret_last_seen) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    int ret = TF_E_OK;
    uint8_t tag_id_length = 0;
    uint8_t tag_id_data[10];

    if (ret_tag_id_length != NULL) {
        *ret_tag_id_length = 0;
    }

    ret = tf_nfc_simple_get_tag_id_low_level(nfc, index, ret_tag_type, &tag_id_length, tag_id_data, ret_last_seen);

    if (ret != TF_E_OK) {
        return ret;
    }

    if (ret_tag_id != NULL) {
        memcpy(ret_tag_id, tag_id_data, sizeof(uint8_t) * tag_id_length);
        memset(&ret_tag_id[tag_id_length], 0, sizeof(uint8_t) * (10 - tag_id_length));
    }

    if (ret_tag_id_length != NULL) {
        *ret_tag_id_length = tag_id_length;
    }

    return ret;
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_nfc_register_reader_state_changed_callback(TF_NFC *nfc, TF_NFC_ReaderStateChangedHandler handler, void *user_data) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    nfc->reader_state_changed_handler = handler;
    nfc->reader_state_changed_user_data = user_data;

    return TF_E_OK;
}


int tf_nfc_register_cardemu_state_changed_callback(TF_NFC *nfc, TF_NFC_CardemuStateChangedHandler handler, void *user_data) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    nfc->cardemu_state_changed_handler = handler;
    nfc->cardemu_state_changed_user_data = user_data;

    return TF_E_OK;
}


int tf_nfc_register_p2p_state_changed_callback(TF_NFC *nfc, TF_NFC_P2PStateChangedHandler handler, void *user_data) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    nfc->p2p_state_changed_handler = handler;
    nfc->p2p_state_changed_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_nfc_callback_tick(TF_NFC *nfc, uint32_t timeout_us) {
    if (nfc == NULL) {
        return TF_E_NULL;
    }

    if (nfc->magic != 0x5446 || nfc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = nfc->tfp->spitfp->hal;

    return tf_tfp_callback_tick(nfc->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
