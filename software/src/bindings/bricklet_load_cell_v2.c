/* ***********************************************************
 * This file was automatically generated on 2023-01-25.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.3         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_load_cell_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_load_cell_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_LoadCellV2 *load_cell_v2 = (TF_LoadCellV2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(load_cell_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_LOAD_CELL_V2_CALLBACK_WEIGHT: {
            TF_LoadCellV2_WeightHandler fn = load_cell_v2->weight_handler;
            void *user_data = load_cell_v2->weight_user_data;
            if (fn == NULL) {
                return false;
            }

            int32_t weight = tf_packet_buffer_read_int32_t(payload);
            hal_common->locked = true;
            fn(load_cell_v2, weight, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_load_cell_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_load_cell_v2_create(TF_LoadCellV2 *load_cell_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (load_cell_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(load_cell_v2, 0, sizeof(TF_LoadCellV2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_LOAD_CELL_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    load_cell_v2->tfp = tfp;
    load_cell_v2->tfp->device = load_cell_v2;
    load_cell_v2->tfp->cb_handler = tf_load_cell_v2_callback_handler;
    load_cell_v2->magic = 0x5446;
    load_cell_v2->response_expected[0] = 0x01;
    load_cell_v2->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_load_cell_v2_destroy(TF_LoadCellV2 *load_cell_v2) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }
    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    load_cell_v2->tfp->cb_handler = NULL;
    load_cell_v2->tfp->device = NULL;
    load_cell_v2->tfp = NULL;
    load_cell_v2->magic = 0;

    return TF_E_OK;
}

int tf_load_cell_v2_get_response_expected(TF_LoadCellV2 *load_cell_v2, uint8_t function_id, bool *ret_response_expected) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_LOAD_CELL_V2_FUNCTION_SET_WEIGHT_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (load_cell_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_SET_MOVING_AVERAGE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (load_cell_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_SET_INFO_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (load_cell_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_CALIBRATE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (load_cell_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_TARE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (load_cell_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_SET_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (load_cell_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (load_cell_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (load_cell_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (load_cell_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (load_cell_v2->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_load_cell_v2_set_response_expected(TF_LoadCellV2 *load_cell_v2, uint8_t function_id, bool response_expected) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_LOAD_CELL_V2_FUNCTION_SET_WEIGHT_CALLBACK_CONFIGURATION:
            if (response_expected) {
                load_cell_v2->response_expected[0] |= (1 << 0);
            } else {
                load_cell_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_SET_MOVING_AVERAGE:
            if (response_expected) {
                load_cell_v2->response_expected[0] |= (1 << 1);
            } else {
                load_cell_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_SET_INFO_LED_CONFIG:
            if (response_expected) {
                load_cell_v2->response_expected[0] |= (1 << 2);
            } else {
                load_cell_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_CALIBRATE:
            if (response_expected) {
                load_cell_v2->response_expected[0] |= (1 << 3);
            } else {
                load_cell_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_TARE:
            if (response_expected) {
                load_cell_v2->response_expected[0] |= (1 << 4);
            } else {
                load_cell_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_SET_CONFIGURATION:
            if (response_expected) {
                load_cell_v2->response_expected[0] |= (1 << 5);
            } else {
                load_cell_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                load_cell_v2->response_expected[0] |= (1 << 6);
            } else {
                load_cell_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                load_cell_v2->response_expected[0] |= (1 << 7);
            } else {
                load_cell_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_RESET:
            if (response_expected) {
                load_cell_v2->response_expected[1] |= (1 << 0);
            } else {
                load_cell_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_LOAD_CELL_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                load_cell_v2->response_expected[1] |= (1 << 1);
            } else {
                load_cell_v2->response_expected[1] &= ~(1 << 1);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_load_cell_v2_set_response_expected_all(TF_LoadCellV2 *load_cell_v2, bool response_expected) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(load_cell_v2->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_load_cell_v2_get_weight(TF_LoadCellV2 *load_cell_v2, int32_t *ret_weight) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_GET_WEIGHT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_weight != NULL) { *ret_weight = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_set_weight_callback_configuration(TF_LoadCellV2 *load_cell_v2, uint32_t period, bool value_has_to_change, char option, int32_t min, int32_t max) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_load_cell_v2_get_response_expected(load_cell_v2, TF_LOAD_CELL_V2_FUNCTION_SET_WEIGHT_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_SET_WEIGHT_CALLBACK_CONFIGURATION, 14, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(load_cell_v2->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;
    _send_buf[5] = (uint8_t)option;
    min = tf_leconvert_int32_to(min); memcpy(_send_buf + 6, &min, 4);
    max = tf_leconvert_int32_to(max); memcpy(_send_buf + 10, &max, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_get_weight_callback_configuration(TF_LoadCellV2 *load_cell_v2, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, int32_t *ret_min, int32_t *ret_max) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_GET_WEIGHT_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
        if (_error_code != 0 || _length != 14) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_option != NULL) { *ret_option = tf_packet_buffer_read_char(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_min != NULL) { *ret_min = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_max != NULL) { *ret_max = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 14) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_set_moving_average(TF_LoadCellV2 *load_cell_v2, uint16_t average) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_load_cell_v2_get_response_expected(load_cell_v2, TF_LOAD_CELL_V2_FUNCTION_SET_MOVING_AVERAGE, &_response_expected);
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_SET_MOVING_AVERAGE, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(load_cell_v2->tfp);

    average = tf_leconvert_uint16_to(average); memcpy(_send_buf + 0, &average, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_get_moving_average(TF_LoadCellV2 *load_cell_v2, uint16_t *ret_average) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_GET_MOVING_AVERAGE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_average != NULL) { *ret_average = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_set_info_led_config(TF_LoadCellV2 *load_cell_v2, uint8_t config) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_load_cell_v2_get_response_expected(load_cell_v2, TF_LOAD_CELL_V2_FUNCTION_SET_INFO_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_SET_INFO_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(load_cell_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_get_info_led_config(TF_LoadCellV2 *load_cell_v2, uint8_t *ret_config) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_GET_INFO_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_calibrate(TF_LoadCellV2 *load_cell_v2, uint32_t weight) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_load_cell_v2_get_response_expected(load_cell_v2, TF_LOAD_CELL_V2_FUNCTION_CALIBRATE, &_response_expected);
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_CALIBRATE, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(load_cell_v2->tfp);

    weight = tf_leconvert_uint32_to(weight); memcpy(_send_buf + 0, &weight, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_tare(TF_LoadCellV2 *load_cell_v2) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_load_cell_v2_get_response_expected(load_cell_v2, TF_LOAD_CELL_V2_FUNCTION_TARE, &_response_expected);
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_TARE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_set_configuration(TF_LoadCellV2 *load_cell_v2, uint8_t rate, uint8_t gain) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_load_cell_v2_get_response_expected(load_cell_v2, TF_LOAD_CELL_V2_FUNCTION_SET_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_SET_CONFIGURATION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(load_cell_v2->tfp);

    _send_buf[0] = (uint8_t)rate;
    _send_buf[1] = (uint8_t)gain;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_get_configuration(TF_LoadCellV2 *load_cell_v2, uint8_t *ret_rate, uint8_t *ret_gain) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_GET_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_rate != NULL) { *ret_rate = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_gain != NULL) { *ret_gain = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_get_spitfp_error_count(TF_LoadCellV2 *load_cell_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_set_bootloader_mode(TF_LoadCellV2 *load_cell_v2, uint8_t mode, uint8_t *ret_status) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(load_cell_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_get_bootloader_mode(TF_LoadCellV2 *load_cell_v2, uint8_t *ret_mode) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_set_write_firmware_pointer(TF_LoadCellV2 *load_cell_v2, uint32_t pointer) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_load_cell_v2_get_response_expected(load_cell_v2, TF_LOAD_CELL_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(load_cell_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_write_firmware(TF_LoadCellV2 *load_cell_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(load_cell_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_set_status_led_config(TF_LoadCellV2 *load_cell_v2, uint8_t config) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_load_cell_v2_get_response_expected(load_cell_v2, TF_LOAD_CELL_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(load_cell_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_get_status_led_config(TF_LoadCellV2 *load_cell_v2, uint8_t *ret_config) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_get_chip_temperature(TF_LoadCellV2 *load_cell_v2, int16_t *ret_temperature) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_reset(TF_LoadCellV2 *load_cell_v2) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_load_cell_v2_get_response_expected(load_cell_v2, TF_LOAD_CELL_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_write_uid(TF_LoadCellV2 *load_cell_v2, uint32_t uid) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_load_cell_v2_get_response_expected(load_cell_v2, TF_LOAD_CELL_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(load_cell_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_read_uid(TF_LoadCellV2 *load_cell_v2, uint32_t *ret_uid) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_load_cell_v2_get_identity(TF_LoadCellV2 *load_cell_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = load_cell_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(load_cell_v2->tfp, TF_LOAD_CELL_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(load_cell_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(load_cell_v2->tfp);
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
        tf_tfp_packet_processed(load_cell_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(load_cell_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_load_cell_v2_register_weight_callback(TF_LoadCellV2 *load_cell_v2, TF_LoadCellV2_WeightHandler handler, void *user_data) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    load_cell_v2->weight_handler = handler;
    load_cell_v2->weight_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_load_cell_v2_callback_tick(TF_LoadCellV2 *load_cell_v2, uint32_t timeout_us) {
    if (load_cell_v2 == NULL) {
        return TF_E_NULL;
    }

    if (load_cell_v2->magic != 0x5446 || load_cell_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = load_cell_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(load_cell_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
