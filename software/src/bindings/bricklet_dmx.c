/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_dmx.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_dmx_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_DMX *dmx = (TF_DMX *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(dmx->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_DMX_CALLBACK_FRAME_STARTED: {
            TF_DMX_FrameStartedHandler fn = dmx->frame_started_handler;
            void *user_data = dmx->frame_started_user_data;
            if (fn == NULL) {
                return false;
            }


            hal_common->locked = true;
            fn(dmx, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_DMX_CALLBACK_FRAME_AVAILABLE: {
            TF_DMX_FrameAvailableHandler fn = dmx->frame_available_handler;
            void *user_data = dmx->frame_available_user_data;
            if (fn == NULL) {
                return false;
            }

            uint32_t frame_number = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(dmx, frame_number, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_DMX_CALLBACK_FRAME_LOW_LEVEL: {
            TF_DMX_FrameLowLevelHandler fn = dmx->frame_low_level_handler;
            void *user_data = dmx->frame_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint16_t frame_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t frame_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            uint8_t frame_chunk_data[56]; for (_i = 0; _i < 56; ++_i) frame_chunk_data[_i] = tf_packet_buffer_read_uint8_t(payload);
            uint32_t frame_number = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(dmx, frame_length, frame_chunk_offset, frame_chunk_data, frame_number, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_DMX_CALLBACK_FRAME_ERROR_COUNT: {
            TF_DMX_FrameErrorCountHandler fn = dmx->frame_error_count_handler;
            void *user_data = dmx->frame_error_count_user_data;
            if (fn == NULL) {
                return false;
            }

            uint32_t overrun_error_count = tf_packet_buffer_read_uint32_t(payload);
            uint32_t framing_error_count = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(dmx, overrun_error_count, framing_error_count, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_dmx_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_dmx_create(TF_DMX *dmx, const char *uid_or_port_name, TF_HAL *hal) {
    if (dmx == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(dmx, 0, sizeof(TF_DMX));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_DMX_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    dmx->tfp = tfp;
    dmx->tfp->device = dmx;
    dmx->tfp->cb_handler = tf_dmx_callback_handler;
    dmx->magic = 0x5446;
    dmx->response_expected[0] = 0x22;
    dmx->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_dmx_destroy(TF_DMX *dmx) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }
    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    dmx->tfp->cb_handler = NULL;
    dmx->tfp->device = NULL;
    dmx->tfp = NULL;
    dmx->magic = 0;

    return TF_E_OK;
}

int tf_dmx_get_response_expected(TF_DMX *dmx, uint8_t function_id, bool *ret_response_expected) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_DMX_FUNCTION_SET_DMX_MODE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (dmx->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_DMX_FUNCTION_WRITE_FRAME_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (dmx->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_DMX_FUNCTION_SET_FRAME_DURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (dmx->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_DMX_FUNCTION_SET_COMMUNICATION_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (dmx->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_DMX_FUNCTION_SET_ERROR_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (dmx->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_DMX_FUNCTION_SET_FRAME_CALLBACK_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (dmx->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_DMX_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (dmx->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_DMX_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (dmx->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_DMX_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (dmx->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_DMX_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (dmx->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_dmx_set_response_expected(TF_DMX *dmx, uint8_t function_id, bool response_expected) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_DMX_FUNCTION_SET_DMX_MODE:
            if (response_expected) {
                dmx->response_expected[0] |= (1 << 0);
            } else {
                dmx->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_DMX_FUNCTION_WRITE_FRAME_LOW_LEVEL:
            if (response_expected) {
                dmx->response_expected[0] |= (1 << 1);
            } else {
                dmx->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_DMX_FUNCTION_SET_FRAME_DURATION:
            if (response_expected) {
                dmx->response_expected[0] |= (1 << 2);
            } else {
                dmx->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_DMX_FUNCTION_SET_COMMUNICATION_LED_CONFIG:
            if (response_expected) {
                dmx->response_expected[0] |= (1 << 3);
            } else {
                dmx->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_DMX_FUNCTION_SET_ERROR_LED_CONFIG:
            if (response_expected) {
                dmx->response_expected[0] |= (1 << 4);
            } else {
                dmx->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_DMX_FUNCTION_SET_FRAME_CALLBACK_CONFIG:
            if (response_expected) {
                dmx->response_expected[0] |= (1 << 5);
            } else {
                dmx->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_DMX_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                dmx->response_expected[0] |= (1 << 6);
            } else {
                dmx->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_DMX_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                dmx->response_expected[0] |= (1 << 7);
            } else {
                dmx->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_DMX_FUNCTION_RESET:
            if (response_expected) {
                dmx->response_expected[1] |= (1 << 0);
            } else {
                dmx->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_DMX_FUNCTION_WRITE_UID:
            if (response_expected) {
                dmx->response_expected[1] |= (1 << 1);
            } else {
                dmx->response_expected[1] &= ~(1 << 1);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_dmx_set_response_expected_all(TF_DMX *dmx, bool response_expected) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(dmx->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_dmx_set_dmx_mode(TF_DMX *dmx, uint8_t dmx_mode) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_dmx_get_response_expected(dmx, TF_DMX_FUNCTION_SET_DMX_MODE, &_response_expected);
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_SET_DMX_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(dmx->tfp);

    _send_buf[0] = (uint8_t)dmx_mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_get_dmx_mode(TF_DMX *dmx, uint8_t *ret_dmx_mode) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_GET_DMX_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_dmx_mode != NULL) { *ret_dmx_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_write_frame_low_level(TF_DMX *dmx, uint16_t frame_length, uint16_t frame_chunk_offset, const uint8_t frame_chunk_data[60]) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_dmx_get_response_expected(dmx, TF_DMX_FUNCTION_WRITE_FRAME_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_WRITE_FRAME_LOW_LEVEL, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(dmx->tfp);

    frame_length = tf_leconvert_uint16_to(frame_length); memcpy(_send_buf + 0, &frame_length, 2);
    frame_chunk_offset = tf_leconvert_uint16_to(frame_chunk_offset); memcpy(_send_buf + 2, &frame_chunk_offset, 2);
    memcpy(_send_buf + 4, frame_chunk_data, 60);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_read_frame_low_level(TF_DMX *dmx, uint16_t *ret_frame_length, uint16_t *ret_frame_chunk_offset, uint8_t ret_frame_chunk_data[56], uint32_t *ret_frame_number) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_READ_FRAME_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_frame_length != NULL) { *ret_frame_length = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_frame_chunk_offset != NULL) { *ret_frame_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_frame_chunk_data != NULL) { for (_i = 0; _i < 56; ++_i) ret_frame_chunk_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 56); }
            if (ret_frame_number != NULL) { *ret_frame_number = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_set_frame_duration(TF_DMX *dmx, uint16_t frame_duration) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_dmx_get_response_expected(dmx, TF_DMX_FUNCTION_SET_FRAME_DURATION, &_response_expected);
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_SET_FRAME_DURATION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(dmx->tfp);

    frame_duration = tf_leconvert_uint16_to(frame_duration); memcpy(_send_buf + 0, &frame_duration, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_get_frame_duration(TF_DMX *dmx, uint16_t *ret_frame_duration) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_GET_FRAME_DURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_frame_duration != NULL) { *ret_frame_duration = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_get_frame_error_count(TF_DMX *dmx, uint32_t *ret_overrun_error_count, uint32_t *ret_framing_error_count) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_GET_FRAME_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_overrun_error_count != NULL) { *ret_overrun_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_framing_error_count != NULL) { *ret_framing_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_set_communication_led_config(TF_DMX *dmx, uint8_t config) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_dmx_get_response_expected(dmx, TF_DMX_FUNCTION_SET_COMMUNICATION_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_SET_COMMUNICATION_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(dmx->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_get_communication_led_config(TF_DMX *dmx, uint8_t *ret_config) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_GET_COMMUNICATION_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_set_error_led_config(TF_DMX *dmx, uint8_t config) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_dmx_get_response_expected(dmx, TF_DMX_FUNCTION_SET_ERROR_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_SET_ERROR_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(dmx->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_get_error_led_config(TF_DMX *dmx, uint8_t *ret_config) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_GET_ERROR_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_set_frame_callback_config(TF_DMX *dmx, bool frame_started_callback_enabled, bool frame_available_callback_enabled, bool frame_callback_enabled, bool frame_error_count_callback_enabled) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_dmx_get_response_expected(dmx, TF_DMX_FUNCTION_SET_FRAME_CALLBACK_CONFIG, &_response_expected);
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_SET_FRAME_CALLBACK_CONFIG, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(dmx->tfp);

    _send_buf[0] = frame_started_callback_enabled ? 1 : 0;
    _send_buf[1] = frame_available_callback_enabled ? 1 : 0;
    _send_buf[2] = frame_callback_enabled ? 1 : 0;
    _send_buf[3] = frame_error_count_callback_enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_get_frame_callback_config(TF_DMX *dmx, bool *ret_frame_started_callback_enabled, bool *ret_frame_available_callback_enabled, bool *ret_frame_callback_enabled, bool *ret_frame_error_count_callback_enabled) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_GET_FRAME_CALLBACK_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_frame_started_callback_enabled != NULL) { *ret_frame_started_callback_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_frame_available_callback_enabled != NULL) { *ret_frame_available_callback_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_frame_callback_enabled != NULL) { *ret_frame_callback_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_frame_error_count_callback_enabled != NULL) { *ret_frame_error_count_callback_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_get_spitfp_error_count(TF_DMX *dmx, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_set_bootloader_mode(TF_DMX *dmx, uint8_t mode, uint8_t *ret_status) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(dmx->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_get_bootloader_mode(TF_DMX *dmx, uint8_t *ret_mode) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_set_write_firmware_pointer(TF_DMX *dmx, uint32_t pointer) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_dmx_get_response_expected(dmx, TF_DMX_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(dmx->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_write_firmware(TF_DMX *dmx, const uint8_t data[64], uint8_t *ret_status) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(dmx->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_set_status_led_config(TF_DMX *dmx, uint8_t config) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_dmx_get_response_expected(dmx, TF_DMX_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(dmx->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_get_status_led_config(TF_DMX *dmx, uint8_t *ret_config) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_get_chip_temperature(TF_DMX *dmx, int16_t *ret_temperature) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_reset(TF_DMX *dmx) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_dmx_get_response_expected(dmx, TF_DMX_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_write_uid(TF_DMX *dmx, uint32_t uid) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_dmx_get_response_expected(dmx, TF_DMX_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(dmx->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_read_uid(TF_DMX *dmx, uint32_t *ret_uid) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_dmx_get_identity(TF_DMX *dmx, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = dmx->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(dmx->tfp, TF_DMX_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(dmx->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(dmx->tfp);
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
        tf_tfp_packet_processed(dmx->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(dmx->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

static int tf_dmx_write_frame_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    (void)wrapper_data;
    uint16_t frame_chunk_offset = (uint16_t)chunk_offset;
    uint16_t frame_length = (uint16_t)stream_length;
    uint32_t frame_chunk_written = 60;

    uint8_t *frame_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_dmx_write_frame_low_level((TF_DMX *)device, frame_length, frame_chunk_offset, frame_chunk_data);

    *ret_chunk_written = (uint32_t) frame_chunk_written;
    return ret;
}

int tf_dmx_write_frame(TF_DMX *dmx, const uint8_t *frame, uint16_t frame_length) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }


    uint32_t _stream_length = frame_length;
    uint32_t _frame_written = 0;
    uint8_t _chunk_data[60];

    int ret = tf_stream_in(dmx, tf_dmx_write_frame_ll_wrapper, NULL, frame, _stream_length, _chunk_data, &_frame_written, 60, tf_copy_items_uint8_t);



    return ret;
}


typedef struct TF_DMX_ReadFrameLLWrapperData {
    uint32_t *ret_frame_number;
} TF_DMX_ReadFrameLLWrapperData;


static int tf_dmx_read_frame_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    TF_DMX_ReadFrameLLWrapperData *data = (TF_DMX_ReadFrameLLWrapperData *) wrapper_data;
    uint16_t frame_length = 0;
    uint16_t frame_chunk_offset = 0;
    uint8_t *frame_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_dmx_read_frame_low_level((TF_DMX *)device, &frame_length, &frame_chunk_offset, frame_chunk_data, data->ret_frame_number);

    *ret_stream_length = (uint32_t)frame_length;
    *ret_chunk_offset = (uint32_t)frame_chunk_offset;
    return ret;
}

int tf_dmx_read_frame(TF_DMX *dmx, uint8_t *ret_frame, uint16_t *ret_frame_length, uint32_t *ret_frame_number) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_DMX_ReadFrameLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.ret_frame_number = ret_frame_number;
    uint32_t _frame_length = 0;
    uint8_t _frame_chunk_data[56];

    int ret = tf_stream_out(dmx, tf_dmx_read_frame_ll_wrapper, &_wrapper_data, ret_frame, &_frame_length, _frame_chunk_data, 56, tf_copy_items_uint8_t);

    if (ret_frame_length != NULL) {
        *ret_frame_length = (uint16_t)_frame_length;
    }
    return ret;
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_dmx_register_frame_started_callback(TF_DMX *dmx, TF_DMX_FrameStartedHandler handler, void *user_data) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    dmx->frame_started_handler = handler;
    dmx->frame_started_user_data = user_data;

    return TF_E_OK;
}


int tf_dmx_register_frame_available_callback(TF_DMX *dmx, TF_DMX_FrameAvailableHandler handler, void *user_data) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    dmx->frame_available_handler = handler;
    dmx->frame_available_user_data = user_data;

    return TF_E_OK;
}


int tf_dmx_register_frame_low_level_callback(TF_DMX *dmx, TF_DMX_FrameLowLevelHandler handler, void *user_data) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    dmx->frame_low_level_handler = handler;
    dmx->frame_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_dmx_frame_wrapper(TF_DMX *dmx, uint16_t frame_length, uint16_t frame_chunk_offset, uint8_t frame_chunk_data[56], uint32_t frame_number, void *user_data) {
    uint32_t stream_length = (uint32_t) frame_length;
    uint32_t chunk_offset = (uint32_t) frame_chunk_offset;
    if (!tf_stream_out_callback(&dmx->frame_hlc, stream_length, chunk_offset, frame_chunk_data, 56, tf_copy_items_uint8_t)) {
        return;
    }

    // Stream is either complete or out of sync
    uint8_t *frame = (uint8_t *) (dmx->frame_hlc.length == 0 ? NULL : dmx->frame_hlc.data);
    dmx->frame_handler(dmx, frame, frame_length, frame_number, user_data);

    dmx->frame_hlc.stream_in_progress = false;
    dmx->frame_hlc.length = 0;
}

int tf_dmx_register_frame_callback(TF_DMX *dmx, TF_DMX_FrameHandler handler, uint8_t *frame_buffer, void *user_data) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    dmx->frame_handler = handler;

    dmx->frame_hlc.data = frame_buffer;
    dmx->frame_hlc.length = 0;
    dmx->frame_hlc.stream_in_progress = false;

    return tf_dmx_register_frame_low_level_callback(dmx, handler == NULL ? NULL : tf_dmx_frame_wrapper, user_data);
}


int tf_dmx_register_frame_error_count_callback(TF_DMX *dmx, TF_DMX_FrameErrorCountHandler handler, void *user_data) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    dmx->frame_error_count_handler = handler;
    dmx->frame_error_count_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_dmx_callback_tick(TF_DMX *dmx, uint32_t timeout_us) {
    if (dmx == NULL) {
        return TF_E_NULL;
    }

    if (dmx->magic != 0x5446 || dmx->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = dmx->tfp->spitfp->hal;

    return tf_tfp_callback_tick(dmx->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
