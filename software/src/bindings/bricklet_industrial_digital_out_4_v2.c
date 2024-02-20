/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_industrial_digital_out_4_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_industrial_digital_out_4_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2 = (TF_IndustrialDigitalOut4V2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(industrial_digital_out_4_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_CALLBACK_MONOFLOP_DONE: {
            TF_IndustrialDigitalOut4V2_MonoflopDoneHandler fn = industrial_digital_out_4_v2->monoflop_done_handler;
            void *user_data = industrial_digital_out_4_v2->monoflop_done_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t channel = tf_packet_buffer_read_uint8_t(payload);
            bool value = tf_packet_buffer_read_bool(payload);
            hal_common->locked = true;
            fn(industrial_digital_out_4_v2, channel, value, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_industrial_digital_out_4_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_industrial_digital_out_4_v2_create(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (industrial_digital_out_4_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(industrial_digital_out_4_v2, 0, sizeof(TF_IndustrialDigitalOut4V2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    industrial_digital_out_4_v2->tfp = tfp;
    industrial_digital_out_4_v2->tfp->device = industrial_digital_out_4_v2;
    industrial_digital_out_4_v2->tfp->cb_handler = tf_industrial_digital_out_4_v2_callback_handler;
    industrial_digital_out_4_v2->magic = 0x5446;
    industrial_digital_out_4_v2->response_expected[0] = 0x00;
    industrial_digital_out_4_v2->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_industrial_digital_out_4_v2_destroy(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }
    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    industrial_digital_out_4_v2->tfp->cb_handler = NULL;
    industrial_digital_out_4_v2->tfp->device = NULL;
    industrial_digital_out_4_v2->tfp = NULL;
    industrial_digital_out_4_v2->magic = 0;

    return TF_E_OK;
}

int tf_industrial_digital_out_4_v2_get_response_expected(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t function_id, bool *ret_response_expected) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_VALUE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_digital_out_4_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_SELECTED_VALUE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_digital_out_4_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_MONOFLOP:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_digital_out_4_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_CHANNEL_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_digital_out_4_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_PWM_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_digital_out_4_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_digital_out_4_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_digital_out_4_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_digital_out_4_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_digital_out_4_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_industrial_digital_out_4_v2_set_response_expected(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t function_id, bool response_expected) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_VALUE:
            if (response_expected) {
                industrial_digital_out_4_v2->response_expected[0] |= (1 << 0);
            } else {
                industrial_digital_out_4_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_SELECTED_VALUE:
            if (response_expected) {
                industrial_digital_out_4_v2->response_expected[0] |= (1 << 1);
            } else {
                industrial_digital_out_4_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_MONOFLOP:
            if (response_expected) {
                industrial_digital_out_4_v2->response_expected[0] |= (1 << 2);
            } else {
                industrial_digital_out_4_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_CHANNEL_LED_CONFIG:
            if (response_expected) {
                industrial_digital_out_4_v2->response_expected[0] |= (1 << 3);
            } else {
                industrial_digital_out_4_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_PWM_CONFIGURATION:
            if (response_expected) {
                industrial_digital_out_4_v2->response_expected[0] |= (1 << 4);
            } else {
                industrial_digital_out_4_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                industrial_digital_out_4_v2->response_expected[0] |= (1 << 5);
            } else {
                industrial_digital_out_4_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                industrial_digital_out_4_v2->response_expected[0] |= (1 << 6);
            } else {
                industrial_digital_out_4_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_RESET:
            if (response_expected) {
                industrial_digital_out_4_v2->response_expected[0] |= (1 << 7);
            } else {
                industrial_digital_out_4_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                industrial_digital_out_4_v2->response_expected[1] |= (1 << 0);
            } else {
                industrial_digital_out_4_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_industrial_digital_out_4_v2_set_response_expected_all(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, bool response_expected) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(industrial_digital_out_4_v2->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_industrial_digital_out_4_v2_set_value(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, const bool value[4]) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_digital_out_4_v2_get_response_expected(industrial_digital_out_4_v2, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_VALUE, &_response_expected);
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_VALUE, 1, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    memset(_send_buf + 0, 0, 1); for (_i = 0; _i < 4; ++_i) _send_buf[0 + (_i / 8)] |= (value[_i] ? 1 : 0) << (_i % 8);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_get_value(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, bool ret_value[4]) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_GET_VALUE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_digital_out_4_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_value != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_value, 4);} else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_set_selected_value(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t channel, bool value) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_digital_out_4_v2_get_response_expected(industrial_digital_out_4_v2, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_SELECTED_VALUE, &_response_expected);
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_SELECTED_VALUE, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = value ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_set_monoflop(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t channel, bool value, uint32_t time) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_digital_out_4_v2_get_response_expected(industrial_digital_out_4_v2, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_MONOFLOP, &_response_expected);
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_MONOFLOP, 6, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = value ? 1 : 0;
    time = tf_leconvert_uint32_to(time); memcpy(_send_buf + 2, &time, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_get_monoflop(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t channel, bool *ret_value, uint32_t *ret_time, uint32_t *ret_time_remaining) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_GET_MONOFLOP, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_digital_out_4_v2->tfp);
        if (_error_code != 0 || _length != 9) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_value != NULL) { *ret_value = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_time != NULL) { *ret_time = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_time_remaining != NULL) { *ret_time_remaining = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 9) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_set_channel_led_config(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t channel, uint8_t config) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_digital_out_4_v2_get_response_expected(industrial_digital_out_4_v2, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_CHANNEL_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_CHANNEL_LED_CONFIG, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_get_channel_led_config(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t channel, uint8_t *ret_config) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_GET_CHANNEL_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_digital_out_4_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_set_pwm_configuration(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t channel, uint32_t frequency, uint16_t duty_cycle) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_digital_out_4_v2_get_response_expected(industrial_digital_out_4_v2, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_PWM_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_PWM_CONFIGURATION, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    _send_buf[0] = (uint8_t)channel;
    frequency = tf_leconvert_uint32_to(frequency); memcpy(_send_buf + 1, &frequency, 4);
    duty_cycle = tf_leconvert_uint16_to(duty_cycle); memcpy(_send_buf + 5, &duty_cycle, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_get_pwm_configuration(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t channel, uint32_t *ret_frequency, uint16_t *ret_duty_cycle) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_GET_PWM_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_digital_out_4_v2->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_frequency != NULL) { *ret_frequency = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_duty_cycle != NULL) { *ret_duty_cycle = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_get_spitfp_error_count(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_digital_out_4_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_set_bootloader_mode(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t mode, uint8_t *ret_status) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_digital_out_4_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_get_bootloader_mode(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t *ret_mode) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_digital_out_4_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_set_write_firmware_pointer(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint32_t pointer) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_digital_out_4_v2_get_response_expected(industrial_digital_out_4_v2, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_write_firmware(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_digital_out_4_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_set_status_led_config(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t config) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_digital_out_4_v2_get_response_expected(industrial_digital_out_4_v2, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_get_status_led_config(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint8_t *ret_config) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_digital_out_4_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_get_chip_temperature(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, int16_t *ret_temperature) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_digital_out_4_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_reset(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_digital_out_4_v2_get_response_expected(industrial_digital_out_4_v2, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_write_uid(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint32_t uid) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_digital_out_4_v2_get_response_expected(industrial_digital_out_4_v2, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_digital_out_4_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_read_uid(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint32_t *ret_uid) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_digital_out_4_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_digital_out_4_v2_get_identity(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_digital_out_4_v2->tfp, TF_INDUSTRIAL_DIGITAL_OUT_4_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_digital_out_4_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_digital_out_4_v2->tfp);
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
        tf_tfp_packet_processed(industrial_digital_out_4_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_digital_out_4_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_industrial_digital_out_4_v2_register_monoflop_done_callback(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, TF_IndustrialDigitalOut4V2_MonoflopDoneHandler handler, void *user_data) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    industrial_digital_out_4_v2->monoflop_done_handler = handler;
    industrial_digital_out_4_v2->monoflop_done_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_industrial_digital_out_4_v2_callback_tick(TF_IndustrialDigitalOut4V2 *industrial_digital_out_4_v2, uint32_t timeout_us) {
    if (industrial_digital_out_4_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_digital_out_4_v2->magic != 0x5446 || industrial_digital_out_4_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = industrial_digital_out_4_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(industrial_digital_out_4_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
