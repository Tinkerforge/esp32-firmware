/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_industrial_counter.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_industrial_counter_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_IndustrialCounter *industrial_counter = (TF_IndustrialCounter *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(industrial_counter->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_INDUSTRIAL_COUNTER_CALLBACK_ALL_COUNTER: {
            TF_IndustrialCounter_AllCounterHandler fn = industrial_counter->all_counter_handler;
            void *user_data = industrial_counter->all_counter_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            int64_t counter[4]; for (_i = 0; _i < 4; ++_i) counter[_i] = tf_packet_buffer_read_int64_t(payload);
            hal_common->locked = true;
            fn(industrial_counter, counter, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_INDUSTRIAL_COUNTER_CALLBACK_ALL_SIGNAL_DATA: {
            TF_IndustrialCounter_AllSignalDataHandler fn = industrial_counter->all_signal_data_handler;
            void *user_data = industrial_counter->all_signal_data_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint16_t duty_cycle[4]; for (_i = 0; _i < 4; ++_i) duty_cycle[_i] = tf_packet_buffer_read_uint16_t(payload);
            uint64_t period[4]; for (_i = 0; _i < 4; ++_i) period[_i] = tf_packet_buffer_read_uint64_t(payload);
            uint32_t frequency[4]; for (_i = 0; _i < 4; ++_i) frequency[_i] = tf_packet_buffer_read_uint32_t(payload);
            bool value[4]; tf_packet_buffer_read_bool_array(payload, value, 4);
            hal_common->locked = true;
            fn(industrial_counter, duty_cycle, period, frequency, value, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_industrial_counter_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_industrial_counter_create(TF_IndustrialCounter *industrial_counter, const char *uid_or_port_name, TF_HAL *hal) {
    if (industrial_counter == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(industrial_counter, 0, sizeof(TF_IndustrialCounter));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_INDUSTRIAL_COUNTER_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    industrial_counter->tfp = tfp;
    industrial_counter->tfp->device = industrial_counter;
    industrial_counter->tfp->cb_handler = tf_industrial_counter_callback_handler;
    industrial_counter->magic = 0x5446;
    industrial_counter->response_expected[0] = 0x60;
    industrial_counter->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_industrial_counter_destroy(TF_IndustrialCounter *industrial_counter) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }
    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    industrial_counter->tfp->cb_handler = NULL;
    industrial_counter->tfp->device = NULL;
    industrial_counter->tfp = NULL;
    industrial_counter->magic = 0;

    return TF_E_OK;
}

int tf_industrial_counter_get_response_expected(TF_IndustrialCounter *industrial_counter, uint8_t function_id, bool *ret_response_expected) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_COUNTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_counter->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_COUNTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_counter->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_COUNTER_ACTIVE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_counter->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_COUNTER_ACTIVE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_counter->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_COUNTER_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_counter->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_COUNTER_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_counter->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_counter->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_CHANNEL_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_counter->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_counter->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_counter->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_counter->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_counter->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_industrial_counter_set_response_expected(TF_IndustrialCounter *industrial_counter, uint8_t function_id, bool response_expected) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_COUNTER:
            if (response_expected) {
                industrial_counter->response_expected[0] |= (1 << 0);
            } else {
                industrial_counter->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_COUNTER:
            if (response_expected) {
                industrial_counter->response_expected[0] |= (1 << 1);
            } else {
                industrial_counter->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_COUNTER_ACTIVE:
            if (response_expected) {
                industrial_counter->response_expected[0] |= (1 << 2);
            } else {
                industrial_counter->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_COUNTER_ACTIVE:
            if (response_expected) {
                industrial_counter->response_expected[0] |= (1 << 3);
            } else {
                industrial_counter->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_COUNTER_CONFIGURATION:
            if (response_expected) {
                industrial_counter->response_expected[0] |= (1 << 4);
            } else {
                industrial_counter->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_COUNTER_CALLBACK_CONFIGURATION:
            if (response_expected) {
                industrial_counter->response_expected[0] |= (1 << 5);
            } else {
                industrial_counter->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION:
            if (response_expected) {
                industrial_counter->response_expected[0] |= (1 << 6);
            } else {
                industrial_counter->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_CHANNEL_LED_CONFIG:
            if (response_expected) {
                industrial_counter->response_expected[0] |= (1 << 7);
            } else {
                industrial_counter->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                industrial_counter->response_expected[1] |= (1 << 0);
            } else {
                industrial_counter->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                industrial_counter->response_expected[1] |= (1 << 1);
            } else {
                industrial_counter->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_RESET:
            if (response_expected) {
                industrial_counter->response_expected[1] |= (1 << 2);
            } else {
                industrial_counter->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_INDUSTRIAL_COUNTER_FUNCTION_WRITE_UID:
            if (response_expected) {
                industrial_counter->response_expected[1] |= (1 << 3);
            } else {
                industrial_counter->response_expected[1] &= ~(1 << 3);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_industrial_counter_set_response_expected_all(TF_IndustrialCounter *industrial_counter, bool response_expected) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(industrial_counter->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_industrial_counter_get_counter(TF_IndustrialCounter *industrial_counter, uint8_t channel, int64_t *ret_counter) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_COUNTER, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_counter != NULL) { *ret_counter = tf_packet_buffer_read_int64_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 8); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_all_counter(TF_IndustrialCounter *industrial_counter, int64_t ret_counter[4]) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_ALL_COUNTER, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 32) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_counter != NULL) { for (_i = 0; _i < 4; ++_i) ret_counter[_i] = tf_packet_buffer_read_int64_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 32); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 32) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_set_counter(TF_IndustrialCounter *industrial_counter, uint8_t channel, int64_t counter) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_counter_get_response_expected(industrial_counter, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_COUNTER, &_response_expected);
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_COUNTER, 9, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    _send_buf[0] = (uint8_t)channel;
    counter = tf_leconvert_int64_to(counter); memcpy(_send_buf + 1, &counter, 8);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_set_all_counter(TF_IndustrialCounter *industrial_counter, const int64_t counter[4]) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_counter_get_response_expected(industrial_counter, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_COUNTER, &_response_expected);
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_COUNTER, 32, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    for (_i = 0; _i < 4; _i++) { int64_t tmp_counter = tf_leconvert_int64_to(counter[_i]); memcpy(_send_buf + 0 + (_i * sizeof(int64_t)), &tmp_counter, sizeof(int64_t)); }

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_signal_data(TF_IndustrialCounter *industrial_counter, uint8_t channel, uint16_t *ret_duty_cycle, uint64_t *ret_period, uint32_t *ret_frequency, bool *ret_value) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_SIGNAL_DATA, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 15) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_duty_cycle != NULL) { *ret_duty_cycle = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint64_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 8); }
            if (ret_frequency != NULL) { *ret_frequency = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value != NULL) { *ret_value = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 15) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_all_signal_data(TF_IndustrialCounter *industrial_counter, uint16_t ret_duty_cycle[4], uint64_t ret_period[4], uint32_t ret_frequency[4], bool ret_value[4]) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_ALL_SIGNAL_DATA, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 57) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_duty_cycle != NULL) { for (_i = 0; _i < 4; ++_i) ret_duty_cycle[_i] = tf_packet_buffer_read_uint16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 8); }
            if (ret_period != NULL) { for (_i = 0; _i < 4; ++_i) ret_period[_i] = tf_packet_buffer_read_uint64_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 32); }
            if (ret_frequency != NULL) { for (_i = 0; _i < 4; ++_i) ret_frequency[_i] = tf_packet_buffer_read_uint32_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 16); }
            if (ret_value != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_value, 4);} else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 57) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_set_counter_active(TF_IndustrialCounter *industrial_counter, uint8_t channel, bool active) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_counter_get_response_expected(industrial_counter, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_COUNTER_ACTIVE, &_response_expected);
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_COUNTER_ACTIVE, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = active ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_set_all_counter_active(TF_IndustrialCounter *industrial_counter, const bool active[4]) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_counter_get_response_expected(industrial_counter, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_COUNTER_ACTIVE, &_response_expected);
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_COUNTER_ACTIVE, 1, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    memset(_send_buf + 0, 0, 1); for (_i = 0; _i < 4; ++_i) _send_buf[0 + (_i / 8)] |= (active[_i] ? 1 : 0) << (_i % 8);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_counter_active(TF_IndustrialCounter *industrial_counter, uint8_t channel, bool *ret_active) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_COUNTER_ACTIVE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_active != NULL) { *ret_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_all_counter_active(TF_IndustrialCounter *industrial_counter, bool ret_active[4]) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_ALL_COUNTER_ACTIVE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_active != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_active, 4);} else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_set_counter_configuration(TF_IndustrialCounter *industrial_counter, uint8_t channel, uint8_t count_edge, uint8_t count_direction, uint8_t duty_cycle_prescaler, uint8_t frequency_integration_time) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_counter_get_response_expected(industrial_counter, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_COUNTER_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_COUNTER_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)count_edge;
    _send_buf[2] = (uint8_t)count_direction;
    _send_buf[3] = (uint8_t)duty_cycle_prescaler;
    _send_buf[4] = (uint8_t)frequency_integration_time;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_counter_configuration(TF_IndustrialCounter *industrial_counter, uint8_t channel, uint8_t *ret_count_edge, uint8_t *ret_count_direction, uint8_t *ret_duty_cycle_prescaler, uint8_t *ret_frequency_integration_time) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_COUNTER_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_count_edge != NULL) { *ret_count_edge = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_count_direction != NULL) { *ret_count_direction = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_duty_cycle_prescaler != NULL) { *ret_duty_cycle_prescaler = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_frequency_integration_time != NULL) { *ret_frequency_integration_time = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_set_all_counter_callback_configuration(TF_IndustrialCounter *industrial_counter, uint32_t period, bool value_has_to_change) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_counter_get_response_expected(industrial_counter, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_COUNTER_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_COUNTER_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_all_counter_callback_configuration(TF_IndustrialCounter *industrial_counter, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_ALL_COUNTER_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_set_all_signal_data_callback_configuration(TF_IndustrialCounter *industrial_counter, uint32_t period, bool value_has_to_change) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_counter_get_response_expected(industrial_counter, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_all_signal_data_callback_configuration(TF_IndustrialCounter *industrial_counter, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_set_channel_led_config(TF_IndustrialCounter *industrial_counter, uint8_t channel, uint8_t config) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_counter_get_response_expected(industrial_counter, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_CHANNEL_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_CHANNEL_LED_CONFIG, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_channel_led_config(TF_IndustrialCounter *industrial_counter, uint8_t channel, uint8_t *ret_config) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_CHANNEL_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_spitfp_error_count(TF_IndustrialCounter *industrial_counter, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_set_bootloader_mode(TF_IndustrialCounter *industrial_counter, uint8_t mode, uint8_t *ret_status) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_bootloader_mode(TF_IndustrialCounter *industrial_counter, uint8_t *ret_mode) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_set_write_firmware_pointer(TF_IndustrialCounter *industrial_counter, uint32_t pointer) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_counter_get_response_expected(industrial_counter, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_write_firmware(TF_IndustrialCounter *industrial_counter, const uint8_t data[64], uint8_t *ret_status) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_set_status_led_config(TF_IndustrialCounter *industrial_counter, uint8_t config) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_counter_get_response_expected(industrial_counter, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_status_led_config(TF_IndustrialCounter *industrial_counter, uint8_t *ret_config) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_chip_temperature(TF_IndustrialCounter *industrial_counter, int16_t *ret_temperature) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_reset(TF_IndustrialCounter *industrial_counter) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_counter_get_response_expected(industrial_counter, TF_INDUSTRIAL_COUNTER_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_write_uid(TF_IndustrialCounter *industrial_counter, uint32_t uid) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_counter_get_response_expected(industrial_counter, TF_INDUSTRIAL_COUNTER_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_counter->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_read_uid(TF_IndustrialCounter *industrial_counter, uint32_t *ret_uid) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_counter_get_identity(TF_IndustrialCounter *industrial_counter, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_counter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_counter->tfp, TF_INDUSTRIAL_COUNTER_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_counter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_counter->tfp);
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
        tf_tfp_packet_processed(industrial_counter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_counter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_industrial_counter_register_all_counter_callback(TF_IndustrialCounter *industrial_counter, TF_IndustrialCounter_AllCounterHandler handler, void *user_data) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    industrial_counter->all_counter_handler = handler;
    industrial_counter->all_counter_user_data = user_data;

    return TF_E_OK;
}


int tf_industrial_counter_register_all_signal_data_callback(TF_IndustrialCounter *industrial_counter, TF_IndustrialCounter_AllSignalDataHandler handler, void *user_data) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    industrial_counter->all_signal_data_handler = handler;
    industrial_counter->all_signal_data_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_industrial_counter_callback_tick(TF_IndustrialCounter *industrial_counter, uint32_t timeout_us) {
    if (industrial_counter == NULL) {
        return TF_E_NULL;
    }

    if (industrial_counter->magic != 0x5446 || industrial_counter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = industrial_counter->tfp->spitfp->hal;

    return tf_tfp_callback_tick(industrial_counter->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
