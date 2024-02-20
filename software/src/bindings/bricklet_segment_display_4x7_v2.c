/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_segment_display_4x7_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_segment_display_4x7_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_SegmentDisplay4x7V2 *segment_display_4x7_v2 = (TF_SegmentDisplay4x7V2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(segment_display_4x7_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_SEGMENT_DISPLAY_4X7_V2_CALLBACK_COUNTER_FINISHED: {
            TF_SegmentDisplay4x7V2_CounterFinishedHandler fn = segment_display_4x7_v2->counter_finished_handler;
            void *user_data = segment_display_4x7_v2->counter_finished_user_data;
            if (fn == NULL) {
                return false;
            }


            hal_common->locked = true;
            fn(segment_display_4x7_v2, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_segment_display_4x7_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_segment_display_4x7_v2_create(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (segment_display_4x7_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(segment_display_4x7_v2, 0, sizeof(TF_SegmentDisplay4x7V2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_SEGMENT_DISPLAY_4X7_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    segment_display_4x7_v2->tfp = tfp;
    segment_display_4x7_v2->tfp->device = segment_display_4x7_v2;
    segment_display_4x7_v2->tfp->cb_handler = tf_segment_display_4x7_v2_callback_handler;
    segment_display_4x7_v2->magic = 0x5446;
    segment_display_4x7_v2->response_expected[0] = 0x00;
    segment_display_4x7_v2->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_segment_display_4x7_v2_destroy(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }
    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    segment_display_4x7_v2->tfp->cb_handler = NULL;
    segment_display_4x7_v2->tfp->device = NULL;
    segment_display_4x7_v2->tfp = NULL;
    segment_display_4x7_v2->magic = 0;

    return TF_E_OK;
}

int tf_segment_display_4x7_v2_get_response_expected(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t function_id, bool *ret_response_expected) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_SEGMENTS:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (segment_display_4x7_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_BRIGHTNESS:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (segment_display_4x7_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_NUMERIC_VALUE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (segment_display_4x7_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_SELECTED_SEGMENT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (segment_display_4x7_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_START_COUNTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (segment_display_4x7_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (segment_display_4x7_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (segment_display_4x7_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (segment_display_4x7_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (segment_display_4x7_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_segment_display_4x7_v2_set_response_expected(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t function_id, bool response_expected) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_SEGMENTS:
            if (response_expected) {
                segment_display_4x7_v2->response_expected[0] |= (1 << 0);
            } else {
                segment_display_4x7_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_BRIGHTNESS:
            if (response_expected) {
                segment_display_4x7_v2->response_expected[0] |= (1 << 1);
            } else {
                segment_display_4x7_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_NUMERIC_VALUE:
            if (response_expected) {
                segment_display_4x7_v2->response_expected[0] |= (1 << 2);
            } else {
                segment_display_4x7_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_SELECTED_SEGMENT:
            if (response_expected) {
                segment_display_4x7_v2->response_expected[0] |= (1 << 3);
            } else {
                segment_display_4x7_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_START_COUNTER:
            if (response_expected) {
                segment_display_4x7_v2->response_expected[0] |= (1 << 4);
            } else {
                segment_display_4x7_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                segment_display_4x7_v2->response_expected[0] |= (1 << 5);
            } else {
                segment_display_4x7_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                segment_display_4x7_v2->response_expected[0] |= (1 << 6);
            } else {
                segment_display_4x7_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_RESET:
            if (response_expected) {
                segment_display_4x7_v2->response_expected[0] |= (1 << 7);
            } else {
                segment_display_4x7_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                segment_display_4x7_v2->response_expected[1] |= (1 << 0);
            } else {
                segment_display_4x7_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_segment_display_4x7_v2_set_response_expected_all(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, bool response_expected) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(segment_display_4x7_v2->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_segment_display_4x7_v2_set_segments(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, const bool digit0[8], const bool digit1[8], const bool digit2[8], const bool digit3[8], const bool colon[2], bool tick) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_segment_display_4x7_v2_get_response_expected(segment_display_4x7_v2, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_SEGMENTS, &_response_expected);
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_SEGMENTS, 6, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(segment_display_4x7_v2->tfp);

    memset(_send_buf + 0, 0, 1); for (_i = 0; _i < 8; ++_i) _send_buf[0 + (_i / 8)] |= (digit0[_i] ? 1 : 0) << (_i % 8);
    memset(_send_buf + 1, 0, 1); for (_i = 0; _i < 8; ++_i) _send_buf[1 + (_i / 8)] |= (digit1[_i] ? 1 : 0) << (_i % 8);
    memset(_send_buf + 2, 0, 1); for (_i = 0; _i < 8; ++_i) _send_buf[2 + (_i / 8)] |= (digit2[_i] ? 1 : 0) << (_i % 8);
    memset(_send_buf + 3, 0, 1); for (_i = 0; _i < 8; ++_i) _send_buf[3 + (_i / 8)] |= (digit3[_i] ? 1 : 0) << (_i % 8);
    memset(_send_buf + 4, 0, 1); for (_i = 0; _i < 2; ++_i) _send_buf[4 + (_i / 8)] |= (colon[_i] ? 1 : 0) << (_i % 8);
    _send_buf[5] = tick ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_get_segments(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, bool ret_digit0[8], bool ret_digit1[8], bool ret_digit2[8], bool ret_digit3[8], bool ret_colon[2], bool *ret_tick) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_SEGMENTS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(segment_display_4x7_v2->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_digit0 != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_digit0, 8);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_digit1 != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_digit1, 8);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_digit2 != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_digit2, 8);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_digit3 != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_digit3, 8);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_colon != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_colon, 2);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_tick != NULL) { *ret_tick = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_set_brightness(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t brightness) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_segment_display_4x7_v2_get_response_expected(segment_display_4x7_v2, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_BRIGHTNESS, &_response_expected);
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_BRIGHTNESS, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(segment_display_4x7_v2->tfp);

    _send_buf[0] = (uint8_t)brightness;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_get_brightness(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t *ret_brightness) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_BRIGHTNESS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(segment_display_4x7_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_brightness != NULL) { *ret_brightness = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_set_numeric_value(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, const int8_t value[4]) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_segment_display_4x7_v2_get_response_expected(segment_display_4x7_v2, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_NUMERIC_VALUE, &_response_expected);
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_NUMERIC_VALUE, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(segment_display_4x7_v2->tfp);

    memcpy(_send_buf + 0, value, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_set_selected_segment(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t segment, bool value) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_segment_display_4x7_v2_get_response_expected(segment_display_4x7_v2, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_SELECTED_SEGMENT, &_response_expected);
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_SELECTED_SEGMENT, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(segment_display_4x7_v2->tfp);

    _send_buf[0] = (uint8_t)segment;
    _send_buf[1] = value ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_get_selected_segment(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t segment, bool *ret_value) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_SELECTED_SEGMENT, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(segment_display_4x7_v2->tfp);

    _send_buf[0] = (uint8_t)segment;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(segment_display_4x7_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_value != NULL) { *ret_value = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_start_counter(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, int16_t value_from, int16_t value_to, int16_t increment, uint32_t length) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_segment_display_4x7_v2_get_response_expected(segment_display_4x7_v2, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_START_COUNTER, &_response_expected);
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_START_COUNTER, 10, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(segment_display_4x7_v2->tfp);

    value_from = tf_leconvert_int16_to(value_from); memcpy(_send_buf + 0, &value_from, 2);
    value_to = tf_leconvert_int16_to(value_to); memcpy(_send_buf + 2, &value_to, 2);
    increment = tf_leconvert_int16_to(increment); memcpy(_send_buf + 4, &increment, 2);
    length = tf_leconvert_uint32_to(length); memcpy(_send_buf + 6, &length, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_get_counter_value(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint16_t *ret_value) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_COUNTER_VALUE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(segment_display_4x7_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_value != NULL) { *ret_value = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_get_spitfp_error_count(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(segment_display_4x7_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_set_bootloader_mode(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t mode, uint8_t *ret_status) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(segment_display_4x7_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(segment_display_4x7_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_get_bootloader_mode(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t *ret_mode) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(segment_display_4x7_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_set_write_firmware_pointer(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint32_t pointer) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_segment_display_4x7_v2_get_response_expected(segment_display_4x7_v2, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(segment_display_4x7_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_write_firmware(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(segment_display_4x7_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(segment_display_4x7_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_set_status_led_config(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t config) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_segment_display_4x7_v2_get_response_expected(segment_display_4x7_v2, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(segment_display_4x7_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_get_status_led_config(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint8_t *ret_config) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(segment_display_4x7_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_get_chip_temperature(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, int16_t *ret_temperature) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(segment_display_4x7_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_reset(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_segment_display_4x7_v2_get_response_expected(segment_display_4x7_v2, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_write_uid(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint32_t uid) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_segment_display_4x7_v2_get_response_expected(segment_display_4x7_v2, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(segment_display_4x7_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_read_uid(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint32_t *ret_uid) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(segment_display_4x7_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_segment_display_4x7_v2_get_identity(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = segment_display_4x7_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(segment_display_4x7_v2->tfp, TF_SEGMENT_DISPLAY_4X7_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(segment_display_4x7_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(segment_display_4x7_v2->tfp);
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
        tf_tfp_packet_processed(segment_display_4x7_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(segment_display_4x7_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_segment_display_4x7_v2_register_counter_finished_callback(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, TF_SegmentDisplay4x7V2_CounterFinishedHandler handler, void *user_data) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    segment_display_4x7_v2->counter_finished_handler = handler;
    segment_display_4x7_v2->counter_finished_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_segment_display_4x7_v2_callback_tick(TF_SegmentDisplay4x7V2 *segment_display_4x7_v2, uint32_t timeout_us) {
    if (segment_display_4x7_v2 == NULL) {
        return TF_E_NULL;
    }

    if (segment_display_4x7_v2->magic != 0x5446 || segment_display_4x7_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = segment_display_4x7_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(segment_display_4x7_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
