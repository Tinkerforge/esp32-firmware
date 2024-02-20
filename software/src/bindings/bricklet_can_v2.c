/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_can_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_can_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_CANV2 *can_v2 = (TF_CANV2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(can_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_CAN_V2_CALLBACK_FRAME_READ_LOW_LEVEL: {
            TF_CANV2_FrameReadLowLevelHandler fn = can_v2->frame_read_low_level_handler;
            void *user_data = can_v2->frame_read_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint8_t frame_type = tf_packet_buffer_read_uint8_t(payload);
            uint32_t identifier = tf_packet_buffer_read_uint32_t(payload);
            uint8_t data_length = tf_packet_buffer_read_uint8_t(payload);
            uint8_t data_data[15]; for (_i = 0; _i < 15; ++_i) data_data[_i] = tf_packet_buffer_read_uint8_t(payload);
            hal_common->locked = true;
            fn(can_v2, frame_type, identifier, data_length, data_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_CAN_V2_CALLBACK_FRAME_READABLE: {
            TF_CANV2_FrameReadableHandler fn = can_v2->frame_readable_handler;
            void *user_data = can_v2->frame_readable_user_data;
            if (fn == NULL) {
                return false;
            }


            hal_common->locked = true;
            fn(can_v2, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_CAN_V2_CALLBACK_ERROR_OCCURRED: {
            TF_CANV2_ErrorOccurredHandler fn = can_v2->error_occurred_handler;
            void *user_data = can_v2->error_occurred_user_data;
            if (fn == NULL) {
                return false;
            }


            hal_common->locked = true;
            fn(can_v2, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_can_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_can_v2_create(TF_CANV2 *can_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (can_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(can_v2, 0, sizeof(TF_CANV2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_CAN_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    can_v2->tfp = tfp;
    can_v2->tfp->device = can_v2;
    can_v2->tfp->cb_handler = tf_can_v2_callback_handler;
    can_v2->magic = 0x5446;
    can_v2->response_expected[0] = 0xC5;
    can_v2->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_can_v2_destroy(TF_CANV2 *can_v2) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }
    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    can_v2->tfp->cb_handler = NULL;
    can_v2->tfp->device = NULL;
    can_v2->tfp = NULL;
    can_v2->magic = 0;

    return TF_E_OK;
}

int tf_can_v2_get_response_expected(TF_CANV2 *can_v2, uint8_t function_id, bool *ret_response_expected) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_CAN_V2_FUNCTION_SET_FRAME_READ_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (can_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_TRANSCEIVER_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (can_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_QUEUE_CONFIGURATION_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (can_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_READ_FILTER_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (can_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_COMMUNICATION_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (can_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_ERROR_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (can_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (can_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_ERROR_OCCURRED_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (can_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (can_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (can_v2->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_CAN_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (can_v2->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_CAN_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (can_v2->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_can_v2_set_response_expected(TF_CANV2 *can_v2, uint8_t function_id, bool response_expected) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_CAN_V2_FUNCTION_SET_FRAME_READ_CALLBACK_CONFIGURATION:
            if (response_expected) {
                can_v2->response_expected[0] |= (1 << 0);
            } else {
                can_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_TRANSCEIVER_CONFIGURATION:
            if (response_expected) {
                can_v2->response_expected[0] |= (1 << 1);
            } else {
                can_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_QUEUE_CONFIGURATION_LOW_LEVEL:
            if (response_expected) {
                can_v2->response_expected[0] |= (1 << 2);
            } else {
                can_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_READ_FILTER_CONFIGURATION:
            if (response_expected) {
                can_v2->response_expected[0] |= (1 << 3);
            } else {
                can_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_COMMUNICATION_LED_CONFIG:
            if (response_expected) {
                can_v2->response_expected[0] |= (1 << 4);
            } else {
                can_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_ERROR_LED_CONFIG:
            if (response_expected) {
                can_v2->response_expected[0] |= (1 << 5);
            } else {
                can_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                can_v2->response_expected[0] |= (1 << 6);
            } else {
                can_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_ERROR_OCCURRED_CALLBACK_CONFIGURATION:
            if (response_expected) {
                can_v2->response_expected[0] |= (1 << 7);
            } else {
                can_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                can_v2->response_expected[1] |= (1 << 0);
            } else {
                can_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_CAN_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                can_v2->response_expected[1] |= (1 << 1);
            } else {
                can_v2->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_CAN_V2_FUNCTION_RESET:
            if (response_expected) {
                can_v2->response_expected[1] |= (1 << 2);
            } else {
                can_v2->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_CAN_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                can_v2->response_expected[1] |= (1 << 3);
            } else {
                can_v2->response_expected[1] &= ~(1 << 3);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_can_v2_set_response_expected_all(TF_CANV2 *can_v2, bool response_expected) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(can_v2->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_can_v2_write_frame_low_level(TF_CANV2 *can_v2, uint8_t frame_type, uint32_t identifier, uint8_t data_length, const uint8_t data_data[15], bool *ret_success) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_WRITE_FRAME_LOW_LEVEL, 21, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    _send_buf[0] = (uint8_t)frame_type;
    identifier = tf_leconvert_uint32_to(identifier); memcpy(_send_buf + 1, &identifier, 4);
    _send_buf[5] = (uint8_t)data_length;
    memcpy(_send_buf + 6, data_data, 15);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_success != NULL) { *ret_success = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_read_frame_low_level(TF_CANV2 *can_v2, bool *ret_success, uint8_t *ret_frame_type, uint32_t *ret_identifier, uint8_t *ret_data_length, uint8_t ret_data_data[15]) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_READ_FRAME_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 22) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_success != NULL) { *ret_success = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_frame_type != NULL) { *ret_frame_type = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_identifier != NULL) { *ret_identifier = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_data_length != NULL) { *ret_data_length = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_data_data != NULL) { for (_i = 0; _i < 15; ++_i) ret_data_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 15); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 22) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_set_frame_read_callback_configuration(TF_CANV2 *can_v2, bool enabled) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_can_v2_get_response_expected(can_v2, TF_CAN_V2_FUNCTION_SET_FRAME_READ_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_SET_FRAME_READ_CALLBACK_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    _send_buf[0] = enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_frame_read_callback_configuration(TF_CANV2 *can_v2, bool *ret_enabled) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_FRAME_READ_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_set_transceiver_configuration(TF_CANV2 *can_v2, uint32_t baud_rate, uint16_t sample_point, uint8_t transceiver_mode) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_can_v2_get_response_expected(can_v2, TF_CAN_V2_FUNCTION_SET_TRANSCEIVER_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_SET_TRANSCEIVER_CONFIGURATION, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    baud_rate = tf_leconvert_uint32_to(baud_rate); memcpy(_send_buf + 0, &baud_rate, 4);
    sample_point = tf_leconvert_uint16_to(sample_point); memcpy(_send_buf + 4, &sample_point, 2);
    _send_buf[6] = (uint8_t)transceiver_mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_transceiver_configuration(TF_CANV2 *can_v2, uint32_t *ret_baud_rate, uint16_t *ret_sample_point, uint8_t *ret_transceiver_mode) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_TRANSCEIVER_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 7) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_baud_rate != NULL) { *ret_baud_rate = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_sample_point != NULL) { *ret_sample_point = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_transceiver_mode != NULL) { *ret_transceiver_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 7) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_set_queue_configuration_low_level(TF_CANV2 *can_v2, uint8_t write_buffer_size, int32_t write_buffer_timeout, uint16_t write_backlog_size, uint8_t read_buffer_sizes_length, const int8_t read_buffer_sizes_data[32], uint16_t read_backlog_size) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_can_v2_get_response_expected(can_v2, TF_CAN_V2_FUNCTION_SET_QUEUE_CONFIGURATION_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_SET_QUEUE_CONFIGURATION_LOW_LEVEL, 42, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    _send_buf[0] = (uint8_t)write_buffer_size;
    write_buffer_timeout = tf_leconvert_int32_to(write_buffer_timeout); memcpy(_send_buf + 1, &write_buffer_timeout, 4);
    write_backlog_size = tf_leconvert_uint16_to(write_backlog_size); memcpy(_send_buf + 5, &write_backlog_size, 2);
    _send_buf[7] = (uint8_t)read_buffer_sizes_length;
    memcpy(_send_buf + 8, read_buffer_sizes_data, 32);
    read_backlog_size = tf_leconvert_uint16_to(read_backlog_size); memcpy(_send_buf + 40, &read_backlog_size, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_queue_configuration_low_level(TF_CANV2 *can_v2, uint8_t *ret_write_buffer_size, int32_t *ret_write_buffer_timeout, uint16_t *ret_write_backlog_size, uint8_t *ret_read_buffer_sizes_length, int8_t ret_read_buffer_sizes_data[32], uint16_t *ret_read_backlog_size) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_QUEUE_CONFIGURATION_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 42) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_write_buffer_size != NULL) { *ret_write_buffer_size = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_write_buffer_timeout != NULL) { *ret_write_buffer_timeout = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_write_backlog_size != NULL) { *ret_write_backlog_size = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_read_buffer_sizes_length != NULL) { *ret_read_buffer_sizes_length = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_read_buffer_sizes_data != NULL) { for (_i = 0; _i < 32; ++_i) ret_read_buffer_sizes_data[_i] = tf_packet_buffer_read_int8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 32); }
            if (ret_read_backlog_size != NULL) { *ret_read_backlog_size = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 42) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_set_read_filter_configuration(TF_CANV2 *can_v2, uint8_t buffer_index, uint8_t filter_mode, uint32_t filter_mask, uint32_t filter_identifier) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_can_v2_get_response_expected(can_v2, TF_CAN_V2_FUNCTION_SET_READ_FILTER_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_SET_READ_FILTER_CONFIGURATION, 10, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    _send_buf[0] = (uint8_t)buffer_index;
    _send_buf[1] = (uint8_t)filter_mode;
    filter_mask = tf_leconvert_uint32_to(filter_mask); memcpy(_send_buf + 2, &filter_mask, 4);
    filter_identifier = tf_leconvert_uint32_to(filter_identifier); memcpy(_send_buf + 6, &filter_identifier, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_read_filter_configuration(TF_CANV2 *can_v2, uint8_t buffer_index, uint8_t *ret_filter_mode, uint32_t *ret_filter_mask, uint32_t *ret_filter_identifier) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_READ_FILTER_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    _send_buf[0] = (uint8_t)buffer_index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 9) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_filter_mode != NULL) { *ret_filter_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_filter_mask != NULL) { *ret_filter_mask = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_filter_identifier != NULL) { *ret_filter_identifier = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 9) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_error_log_low_level(TF_CANV2 *can_v2, uint8_t *ret_transceiver_state, uint8_t *ret_transceiver_write_error_level, uint8_t *ret_transceiver_read_error_level, uint32_t *ret_transceiver_stuffing_error_count, uint32_t *ret_transceiver_format_error_count, uint32_t *ret_transceiver_ack_error_count, uint32_t *ret_transceiver_bit1_error_count, uint32_t *ret_transceiver_bit0_error_count, uint32_t *ret_transceiver_crc_error_count, uint32_t *ret_write_buffer_timeout_error_count, uint32_t *ret_read_buffer_overflow_error_count, uint8_t *ret_read_buffer_overflow_error_occurred_length, bool ret_read_buffer_overflow_error_occurred_data[32], uint32_t *ret_read_backlog_overflow_error_count) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_ERROR_LOG_LOW_LEVEL, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 44) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_transceiver_state != NULL) { *ret_transceiver_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_transceiver_write_error_level != NULL) { *ret_transceiver_write_error_level = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_transceiver_read_error_level != NULL) { *ret_transceiver_read_error_level = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_transceiver_stuffing_error_count != NULL) { *ret_transceiver_stuffing_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_transceiver_format_error_count != NULL) { *ret_transceiver_format_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_transceiver_ack_error_count != NULL) { *ret_transceiver_ack_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_transceiver_bit1_error_count != NULL) { *ret_transceiver_bit1_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_transceiver_bit0_error_count != NULL) { *ret_transceiver_bit0_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_transceiver_crc_error_count != NULL) { *ret_transceiver_crc_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_write_buffer_timeout_error_count != NULL) { *ret_write_buffer_timeout_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_read_buffer_overflow_error_count != NULL) { *ret_read_buffer_overflow_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_read_buffer_overflow_error_occurred_length != NULL) { *ret_read_buffer_overflow_error_occurred_length = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_read_buffer_overflow_error_occurred_data != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_read_buffer_overflow_error_occurred_data, 32);} else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_read_backlog_overflow_error_count != NULL) { *ret_read_backlog_overflow_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 44) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_set_communication_led_config(TF_CANV2 *can_v2, uint8_t config) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_can_v2_get_response_expected(can_v2, TF_CAN_V2_FUNCTION_SET_COMMUNICATION_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_SET_COMMUNICATION_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_communication_led_config(TF_CANV2 *can_v2, uint8_t *ret_config) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_COMMUNICATION_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_set_error_led_config(TF_CANV2 *can_v2, uint8_t config) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_can_v2_get_response_expected(can_v2, TF_CAN_V2_FUNCTION_SET_ERROR_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_SET_ERROR_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_error_led_config(TF_CANV2 *can_v2, uint8_t *ret_config) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_ERROR_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_set_frame_readable_callback_configuration(TF_CANV2 *can_v2, bool enabled) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_can_v2_get_response_expected(can_v2, TF_CAN_V2_FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    _send_buf[0] = enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_frame_readable_callback_configuration(TF_CANV2 *can_v2, bool *ret_enabled) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_FRAME_READABLE_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_set_error_occurred_callback_configuration(TF_CANV2 *can_v2, bool enabled) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_can_v2_get_response_expected(can_v2, TF_CAN_V2_FUNCTION_SET_ERROR_OCCURRED_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_SET_ERROR_OCCURRED_CALLBACK_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    _send_buf[0] = enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_error_occurred_callback_configuration(TF_CANV2 *can_v2, bool *ret_enabled) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_ERROR_OCCURRED_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_spitfp_error_count(TF_CANV2 *can_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_set_bootloader_mode(TF_CANV2 *can_v2, uint8_t mode, uint8_t *ret_status) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_bootloader_mode(TF_CANV2 *can_v2, uint8_t *ret_mode) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_set_write_firmware_pointer(TF_CANV2 *can_v2, uint32_t pointer) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_can_v2_get_response_expected(can_v2, TF_CAN_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_write_firmware(TF_CANV2 *can_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_set_status_led_config(TF_CANV2 *can_v2, uint8_t config) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_can_v2_get_response_expected(can_v2, TF_CAN_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_status_led_config(TF_CANV2 *can_v2, uint8_t *ret_config) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_chip_temperature(TF_CANV2 *can_v2, int16_t *ret_temperature) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_reset(TF_CANV2 *can_v2) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_can_v2_get_response_expected(can_v2, TF_CAN_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_write_uid(TF_CANV2 *can_v2, uint32_t uid) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_can_v2_get_response_expected(can_v2, TF_CAN_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(can_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_read_uid(TF_CANV2 *can_v2, uint32_t *ret_uid) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_can_v2_get_identity(TF_CANV2 *can_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = can_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(can_v2->tfp, TF_CAN_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(can_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(can_v2->tfp);
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
        tf_tfp_packet_processed(can_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(can_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

typedef struct TF_CANV2_WriteFrameLLWrapperData {
    uint8_t frame_type;
    uint32_t identifier;
    bool *ret_success;
} TF_CANV2_WriteFrameLLWrapperData;


static int tf_can_v2_write_frame_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    TF_CANV2_WriteFrameLLWrapperData *data = (TF_CANV2_WriteFrameLLWrapperData *) wrapper_data;
    (void) chunk_offset;uint8_t data_length = (uint8_t)stream_length;
    uint32_t data_written = 15;

    uint8_t *data_data = (uint8_t *) chunk_data;
    int ret = tf_can_v2_write_frame_low_level((TF_CANV2 *)device, data->frame_type, data->identifier, data_length, data_data, data->ret_success);

    *ret_chunk_written = (uint32_t) data_written;
    return ret;
}

int tf_can_v2_write_frame(TF_CANV2 *can_v2, uint8_t frame_type, uint32_t identifier, const uint8_t *data, uint8_t data_length, bool *ret_success) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_CANV2_WriteFrameLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.frame_type = frame_type;
    _wrapper_data.identifier = identifier;
    _wrapper_data.ret_success = ret_success;

    uint32_t _stream_length = data_length;
    uint32_t _data_written = 0;
    uint8_t _chunk_data[15];

    int ret = tf_stream_in(can_v2, tf_can_v2_write_frame_ll_wrapper, &_wrapper_data, data, _stream_length, _chunk_data, &_data_written, 15, tf_copy_items_uint8_t);



    return ret;
}


int tf_can_v2_read_frame(TF_CANV2 *can_v2, bool *ret_success, uint8_t *ret_frame_type, uint32_t *ret_identifier, uint8_t *ret_data, uint8_t *ret_data_length) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    int ret = TF_E_OK;
    uint8_t data_length = 0;
    uint8_t data_data[15];

    if (ret_data_length != NULL) {
        *ret_data_length = 0;
    }

    ret = tf_can_v2_read_frame_low_level(can_v2, ret_success, ret_frame_type, ret_identifier, &data_length, data_data);

    if (ret != TF_E_OK) {
        return ret;
    }

    if (ret_data != NULL) {
        memcpy(ret_data, data_data, sizeof(uint8_t) * data_length);
        memset(&ret_data[data_length], 0, sizeof(uint8_t) * (15 - data_length));
    }

    if (ret_data_length != NULL) {
        *ret_data_length = data_length;
    }

    return ret;
}

typedef struct TF_CANV2_SetQueueConfigurationLLWrapperData {
    uint8_t write_buffer_size;
    int32_t write_buffer_timeout;
    uint16_t write_backlog_size;
    uint16_t read_backlog_size;
} TF_CANV2_SetQueueConfigurationLLWrapperData;


static int tf_can_v2_set_queue_configuration_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    TF_CANV2_SetQueueConfigurationLLWrapperData *data = (TF_CANV2_SetQueueConfigurationLLWrapperData *) wrapper_data;
    (void) chunk_offset;uint8_t read_buffer_sizes_length = (uint8_t)stream_length;
    uint32_t read_buffer_sizes_written = 32;

    int8_t *read_buffer_sizes_data = (int8_t *) chunk_data;
    int ret = tf_can_v2_set_queue_configuration_low_level((TF_CANV2 *)device, data->write_buffer_size, data->write_buffer_timeout, data->write_backlog_size, read_buffer_sizes_length, read_buffer_sizes_data, data->read_backlog_size);

    *ret_chunk_written = (uint32_t) read_buffer_sizes_written;
    return ret;
}

int tf_can_v2_set_queue_configuration(TF_CANV2 *can_v2, uint8_t write_buffer_size, int32_t write_buffer_timeout, uint16_t write_backlog_size, const int8_t *read_buffer_sizes, uint8_t read_buffer_sizes_length, uint16_t read_backlog_size) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_CANV2_SetQueueConfigurationLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.write_buffer_size = write_buffer_size;
    _wrapper_data.write_buffer_timeout = write_buffer_timeout;
    _wrapper_data.write_backlog_size = write_backlog_size;
    _wrapper_data.read_backlog_size = read_backlog_size;

    uint32_t _stream_length = read_buffer_sizes_length;
    uint32_t _read_buffer_sizes_written = 0;
    int8_t _chunk_data[32];

    int ret = tf_stream_in(can_v2, tf_can_v2_set_queue_configuration_ll_wrapper, &_wrapper_data, read_buffer_sizes, _stream_length, _chunk_data, &_read_buffer_sizes_written, 32, tf_copy_items_int8_t);



    return ret;
}


int tf_can_v2_get_queue_configuration(TF_CANV2 *can_v2, uint8_t *ret_write_buffer_size, int32_t *ret_write_buffer_timeout, uint16_t *ret_write_backlog_size, int8_t *ret_read_buffer_sizes, uint8_t *ret_read_buffer_sizes_length, uint16_t *ret_read_backlog_size) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    int ret = TF_E_OK;
    uint8_t read_buffer_sizes_length = 0;
    int8_t read_buffer_sizes_data[32];

    if (ret_read_buffer_sizes_length != NULL) {
        *ret_read_buffer_sizes_length = 0;
    }

    ret = tf_can_v2_get_queue_configuration_low_level(can_v2, ret_write_buffer_size, ret_write_buffer_timeout, ret_write_backlog_size, &read_buffer_sizes_length, read_buffer_sizes_data, ret_read_backlog_size);

    if (ret != TF_E_OK) {
        return ret;
    }

    if (ret_read_buffer_sizes != NULL) {
        memcpy(ret_read_buffer_sizes, read_buffer_sizes_data, sizeof(int8_t) * read_buffer_sizes_length);
        memset(&ret_read_buffer_sizes[read_buffer_sizes_length], 0, sizeof(int8_t) * (32 - read_buffer_sizes_length));
    }

    if (ret_read_buffer_sizes_length != NULL) {
        *ret_read_buffer_sizes_length = read_buffer_sizes_length;
    }

    return ret;
}

int tf_can_v2_get_error_log(TF_CANV2 *can_v2, uint8_t *ret_transceiver_state, uint8_t *ret_transceiver_write_error_level, uint8_t *ret_transceiver_read_error_level, uint32_t *ret_transceiver_stuffing_error_count, uint32_t *ret_transceiver_format_error_count, uint32_t *ret_transceiver_ack_error_count, uint32_t *ret_transceiver_bit1_error_count, uint32_t *ret_transceiver_bit0_error_count, uint32_t *ret_transceiver_crc_error_count, uint32_t *ret_write_buffer_timeout_error_count, uint32_t *ret_read_buffer_overflow_error_count, bool *ret_read_buffer_overflow_error_occurred, uint8_t *ret_read_buffer_overflow_error_occurred_length, uint32_t *ret_read_backlog_overflow_error_count) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    int ret = TF_E_OK;
    uint8_t read_buffer_overflow_error_occurred_length = 0;
    bool read_buffer_overflow_error_occurred_data[32];

    if (ret_read_buffer_overflow_error_occurred_length != NULL) {
        *ret_read_buffer_overflow_error_occurred_length = 0;
    }

    ret = tf_can_v2_get_error_log_low_level(can_v2, ret_transceiver_state, ret_transceiver_write_error_level, ret_transceiver_read_error_level, ret_transceiver_stuffing_error_count, ret_transceiver_format_error_count, ret_transceiver_ack_error_count, ret_transceiver_bit1_error_count, ret_transceiver_bit0_error_count, ret_transceiver_crc_error_count, ret_write_buffer_timeout_error_count, ret_read_buffer_overflow_error_count, &read_buffer_overflow_error_occurred_length, read_buffer_overflow_error_occurred_data, ret_read_backlog_overflow_error_count);

    if (ret != TF_E_OK) {
        return ret;
    }

    if (ret_read_buffer_overflow_error_occurred != NULL) {
        memcpy(ret_read_buffer_overflow_error_occurred, read_buffer_overflow_error_occurred_data, sizeof(bool) * read_buffer_overflow_error_occurred_length);
        memset(&ret_read_buffer_overflow_error_occurred[read_buffer_overflow_error_occurred_length], 0, sizeof(bool) * (32 - read_buffer_overflow_error_occurred_length));
    }

    if (ret_read_buffer_overflow_error_occurred_length != NULL) {
        *ret_read_buffer_overflow_error_occurred_length = read_buffer_overflow_error_occurred_length;
    }

    return ret;
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_can_v2_register_frame_read_low_level_callback(TF_CANV2 *can_v2, TF_CANV2_FrameReadLowLevelHandler handler, void *user_data) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    can_v2->frame_read_low_level_handler = handler;
    can_v2->frame_read_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_can_v2_frame_read_wrapper(TF_CANV2 *can_v2, uint8_t frame_type, uint32_t identifier, uint8_t data_length, uint8_t data_data[15], void *user_data) {
    uint32_t stream_length = (uint32_t) data_length;
    uint32_t chunk_offset = (uint32_t) 0;
    if (!tf_stream_out_callback(&can_v2->frame_read_hlc, stream_length, chunk_offset, data_data, 15, tf_copy_items_uint8_t)) {
        return;
    }

    // Stream is either complete or out of sync
    uint8_t *data = (uint8_t *) (can_v2->frame_read_hlc.length == 0 ? NULL : can_v2->frame_read_hlc.data);
    can_v2->frame_read_handler(can_v2, frame_type, identifier, data, data_length, user_data);

    can_v2->frame_read_hlc.stream_in_progress = false;
    can_v2->frame_read_hlc.length = 0;
}

int tf_can_v2_register_frame_read_callback(TF_CANV2 *can_v2, TF_CANV2_FrameReadHandler handler, uint8_t *data_buffer, void *user_data) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    can_v2->frame_read_handler = handler;

    can_v2->frame_read_hlc.data = data_buffer;
    can_v2->frame_read_hlc.length = 0;
    can_v2->frame_read_hlc.stream_in_progress = false;

    return tf_can_v2_register_frame_read_low_level_callback(can_v2, handler == NULL ? NULL : tf_can_v2_frame_read_wrapper, user_data);
}


int tf_can_v2_register_frame_readable_callback(TF_CANV2 *can_v2, TF_CANV2_FrameReadableHandler handler, void *user_data) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    can_v2->frame_readable_handler = handler;
    can_v2->frame_readable_user_data = user_data;

    return TF_E_OK;
}


int tf_can_v2_register_error_occurred_callback(TF_CANV2 *can_v2, TF_CANV2_ErrorOccurredHandler handler, void *user_data) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    can_v2->error_occurred_handler = handler;
    can_v2->error_occurred_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_can_v2_callback_tick(TF_CANV2 *can_v2, uint32_t timeout_us) {
    if (can_v2 == NULL) {
        return TF_E_NULL;
    }

    if (can_v2->magic != 0x5446 || can_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = can_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(can_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
