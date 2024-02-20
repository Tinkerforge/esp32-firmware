/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_io16_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_io16_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_IO16V2 *io16_v2 = (TF_IO16V2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(io16_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_IO16_V2_CALLBACK_INPUT_VALUE: {
            TF_IO16V2_InputValueHandler fn = io16_v2->input_value_handler;
            void *user_data = io16_v2->input_value_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t channel = tf_packet_buffer_read_uint8_t(payload);
            bool changed = tf_packet_buffer_read_bool(payload);
            bool value = tf_packet_buffer_read_bool(payload);
            hal_common->locked = true;
            fn(io16_v2, channel, changed, value, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_IO16_V2_CALLBACK_ALL_INPUT_VALUE: {
            TF_IO16V2_AllInputValueHandler fn = io16_v2->all_input_value_handler;
            void *user_data = io16_v2->all_input_value_user_data;
            if (fn == NULL) {
                return false;
            }

            bool changed[16]; tf_packet_buffer_read_bool_array(payload, changed, 16);
            bool value[16]; tf_packet_buffer_read_bool_array(payload, value, 16);
            hal_common->locked = true;
            fn(io16_v2, changed, value, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_IO16_V2_CALLBACK_MONOFLOP_DONE: {
            TF_IO16V2_MonoflopDoneHandler fn = io16_v2->monoflop_done_handler;
            void *user_data = io16_v2->monoflop_done_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t channel = tf_packet_buffer_read_uint8_t(payload);
            bool value = tf_packet_buffer_read_bool(payload);
            hal_common->locked = true;
            fn(io16_v2, channel, value, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_io16_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_io16_v2_create(TF_IO16V2 *io16_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (io16_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(io16_v2, 0, sizeof(TF_IO16V2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_IO16_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    io16_v2->tfp = tfp;
    io16_v2->tfp->device = io16_v2;
    io16_v2->tfp->cb_handler = tf_io16_v2_callback_handler;
    io16_v2->magic = 0x5446;
    io16_v2->response_expected[0] = 0x18;
    io16_v2->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_io16_v2_destroy(TF_IO16V2 *io16_v2) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }
    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    io16_v2->tfp->cb_handler = NULL;
    io16_v2->tfp->device = NULL;
    io16_v2->tfp = NULL;
    io16_v2->magic = 0;

    return TF_E_OK;
}

int tf_io16_v2_get_response_expected(TF_IO16V2 *io16_v2, uint8_t function_id, bool *ret_response_expected) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_IO16_V2_FUNCTION_SET_VALUE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (io16_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_SELECTED_VALUE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (io16_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (io16_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_INPUT_VALUE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (io16_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_ALL_INPUT_VALUE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (io16_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_MONOFLOP:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (io16_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_EDGE_COUNT_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (io16_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (io16_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (io16_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_IO16_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (io16_v2->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_IO16_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (io16_v2->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_io16_v2_set_response_expected(TF_IO16V2 *io16_v2, uint8_t function_id, bool response_expected) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_IO16_V2_FUNCTION_SET_VALUE:
            if (response_expected) {
                io16_v2->response_expected[0] |= (1 << 0);
            } else {
                io16_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_SELECTED_VALUE:
            if (response_expected) {
                io16_v2->response_expected[0] |= (1 << 1);
            } else {
                io16_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_CONFIGURATION:
            if (response_expected) {
                io16_v2->response_expected[0] |= (1 << 2);
            } else {
                io16_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_INPUT_VALUE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                io16_v2->response_expected[0] |= (1 << 3);
            } else {
                io16_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_ALL_INPUT_VALUE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                io16_v2->response_expected[0] |= (1 << 4);
            } else {
                io16_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_MONOFLOP:
            if (response_expected) {
                io16_v2->response_expected[0] |= (1 << 5);
            } else {
                io16_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_EDGE_COUNT_CONFIGURATION:
            if (response_expected) {
                io16_v2->response_expected[0] |= (1 << 6);
            } else {
                io16_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                io16_v2->response_expected[0] |= (1 << 7);
            } else {
                io16_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_IO16_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                io16_v2->response_expected[1] |= (1 << 0);
            } else {
                io16_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_IO16_V2_FUNCTION_RESET:
            if (response_expected) {
                io16_v2->response_expected[1] |= (1 << 1);
            } else {
                io16_v2->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_IO16_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                io16_v2->response_expected[1] |= (1 << 2);
            } else {
                io16_v2->response_expected[1] &= ~(1 << 2);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_io16_v2_set_response_expected_all(TF_IO16V2 *io16_v2, bool response_expected) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(io16_v2->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_io16_v2_set_value(TF_IO16V2 *io16_v2, const bool value[16]) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_io16_v2_get_response_expected(io16_v2, TF_IO16_V2_FUNCTION_SET_VALUE, &_response_expected);
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_SET_VALUE, 2, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    memset(_send_buf + 0, 0, 2); for (_i = 0; _i < 16; ++_i) _send_buf[0 + (_i / 8)] |= (value[_i] ? 1 : 0) << (_i % 8);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_get_value(TF_IO16V2 *io16_v2, bool ret_value[16]) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_GET_VALUE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_value != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_value, 16);} else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_set_selected_value(TF_IO16V2 *io16_v2, uint8_t channel, bool value) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_io16_v2_get_response_expected(io16_v2, TF_IO16_V2_FUNCTION_SET_SELECTED_VALUE, &_response_expected);
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_SET_SELECTED_VALUE, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = value ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_set_configuration(TF_IO16V2 *io16_v2, uint8_t channel, char direction, bool value) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_io16_v2_get_response_expected(io16_v2, TF_IO16_V2_FUNCTION_SET_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_SET_CONFIGURATION, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)direction;
    _send_buf[2] = value ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_get_configuration(TF_IO16V2 *io16_v2, uint8_t channel, char *ret_direction, bool *ret_value) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_GET_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_direction != NULL) { *ret_direction = tf_packet_buffer_read_char(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_value != NULL) { *ret_value = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_set_input_value_callback_configuration(TF_IO16V2 *io16_v2, uint8_t channel, uint32_t period, bool value_has_to_change) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_io16_v2_get_response_expected(io16_v2, TF_IO16_V2_FUNCTION_SET_INPUT_VALUE_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_SET_INPUT_VALUE_CALLBACK_CONFIGURATION, 6, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    _send_buf[0] = (uint8_t)channel;
    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 1, &period, 4);
    _send_buf[5] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_get_input_value_callback_configuration(TF_IO16V2 *io16_v2, uint8_t channel, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_GET_INPUT_VALUE_CALLBACK_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_set_all_input_value_callback_configuration(TF_IO16V2 *io16_v2, uint32_t period, bool value_has_to_change) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_io16_v2_get_response_expected(io16_v2, TF_IO16_V2_FUNCTION_SET_ALL_INPUT_VALUE_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_SET_ALL_INPUT_VALUE_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_get_all_input_value_callback_configuration(TF_IO16V2 *io16_v2, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_GET_ALL_INPUT_VALUE_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_set_monoflop(TF_IO16V2 *io16_v2, uint8_t channel, bool value, uint32_t time) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_io16_v2_get_response_expected(io16_v2, TF_IO16_V2_FUNCTION_SET_MONOFLOP, &_response_expected);
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_SET_MONOFLOP, 6, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = value ? 1 : 0;
    time = tf_leconvert_uint32_to(time); memcpy(_send_buf + 2, &time, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_get_monoflop(TF_IO16V2 *io16_v2, uint8_t channel, bool *ret_value, uint32_t *ret_time, uint32_t *ret_time_remaining) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_GET_MONOFLOP, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 9) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_value != NULL) { *ret_value = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_time != NULL) { *ret_time = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_time_remaining != NULL) { *ret_time_remaining = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 9) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_get_edge_count(TF_IO16V2 *io16_v2, uint8_t channel, bool reset_counter, uint32_t *ret_count) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_GET_EDGE_COUNT, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = reset_counter ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_count != NULL) { *ret_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_set_edge_count_configuration(TF_IO16V2 *io16_v2, uint8_t channel, uint8_t edge_type, uint8_t debounce) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_io16_v2_get_response_expected(io16_v2, TF_IO16_V2_FUNCTION_SET_EDGE_COUNT_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_SET_EDGE_COUNT_CONFIGURATION, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)edge_type;
    _send_buf[2] = (uint8_t)debounce;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_get_edge_count_configuration(TF_IO16V2 *io16_v2, uint8_t channel, uint8_t *ret_edge_type, uint8_t *ret_debounce) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_GET_EDGE_COUNT_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_edge_type != NULL) { *ret_edge_type = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_debounce != NULL) { *ret_debounce = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_get_spitfp_error_count(TF_IO16V2 *io16_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_set_bootloader_mode(TF_IO16V2 *io16_v2, uint8_t mode, uint8_t *ret_status) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_get_bootloader_mode(TF_IO16V2 *io16_v2, uint8_t *ret_mode) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_set_write_firmware_pointer(TF_IO16V2 *io16_v2, uint32_t pointer) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_io16_v2_get_response_expected(io16_v2, TF_IO16_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_write_firmware(TF_IO16V2 *io16_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_set_status_led_config(TF_IO16V2 *io16_v2, uint8_t config) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_io16_v2_get_response_expected(io16_v2, TF_IO16_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_get_status_led_config(TF_IO16V2 *io16_v2, uint8_t *ret_config) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_get_chip_temperature(TF_IO16V2 *io16_v2, int16_t *ret_temperature) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_reset(TF_IO16V2 *io16_v2) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_io16_v2_get_response_expected(io16_v2, TF_IO16_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_write_uid(TF_IO16V2 *io16_v2, uint32_t uid) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_io16_v2_get_response_expected(io16_v2, TF_IO16_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(io16_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_read_uid(TF_IO16V2 *io16_v2, uint32_t *ret_uid) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_io16_v2_get_identity(TF_IO16V2 *io16_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = io16_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(io16_v2->tfp, TF_IO16_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(io16_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(io16_v2->tfp);
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
        tf_tfp_packet_processed(io16_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(io16_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_io16_v2_register_input_value_callback(TF_IO16V2 *io16_v2, TF_IO16V2_InputValueHandler handler, void *user_data) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    io16_v2->input_value_handler = handler;
    io16_v2->input_value_user_data = user_data;

    return TF_E_OK;
}


int tf_io16_v2_register_all_input_value_callback(TF_IO16V2 *io16_v2, TF_IO16V2_AllInputValueHandler handler, void *user_data) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    io16_v2->all_input_value_handler = handler;
    io16_v2->all_input_value_user_data = user_data;

    return TF_E_OK;
}


int tf_io16_v2_register_monoflop_done_callback(TF_IO16V2 *io16_v2, TF_IO16V2_MonoflopDoneHandler handler, void *user_data) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    io16_v2->monoflop_done_handler = handler;
    io16_v2->monoflop_done_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_io16_v2_callback_tick(TF_IO16V2 *io16_v2, uint32_t timeout_us) {
    if (io16_v2 == NULL) {
        return TF_E_NULL;
    }

    if (io16_v2->magic != 0x5446 || io16_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = io16_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(io16_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
