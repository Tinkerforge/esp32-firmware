/* ***********************************************************
 * This file was automatically generated on 2023-01-25.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.3         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_distance_ir_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_distance_ir_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_DistanceIRV2 *distance_ir_v2 = (TF_DistanceIRV2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(distance_ir_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_DISTANCE_IR_V2_CALLBACK_DISTANCE: {
            TF_DistanceIRV2_DistanceHandler fn = distance_ir_v2->distance_handler;
            void *user_data = distance_ir_v2->distance_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t distance = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(distance_ir_v2, distance, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_DISTANCE_IR_V2_CALLBACK_ANALOG_VALUE: {
            TF_DistanceIRV2_AnalogValueHandler fn = distance_ir_v2->analog_value_handler;
            void *user_data = distance_ir_v2->analog_value_user_data;
            if (fn == NULL) {
                return false;
            }

            uint32_t analog_value = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(distance_ir_v2, analog_value, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_distance_ir_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_distance_ir_v2_create(TF_DistanceIRV2 *distance_ir_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (distance_ir_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(distance_ir_v2, 0, sizeof(TF_DistanceIRV2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_DISTANCE_IR_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    distance_ir_v2->tfp = tfp;
    distance_ir_v2->tfp->device = distance_ir_v2;
    distance_ir_v2->tfp->cb_handler = tf_distance_ir_v2_callback_handler;
    distance_ir_v2->magic = 0x5446;
    distance_ir_v2->response_expected[0] = 0x03;
    distance_ir_v2->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_distance_ir_v2_destroy(TF_DistanceIRV2 *distance_ir_v2) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }
    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    distance_ir_v2->tfp->cb_handler = NULL;
    distance_ir_v2->tfp->device = NULL;
    distance_ir_v2->tfp = NULL;
    distance_ir_v2->magic = 0;

    return TF_E_OK;
}

int tf_distance_ir_v2_get_response_expected(TF_DistanceIRV2 *distance_ir_v2, uint8_t function_id, bool *ret_response_expected) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_DISTANCE_IR_V2_FUNCTION_SET_DISTANCE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (distance_ir_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_SET_ANALOG_VALUE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (distance_ir_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_SET_MOVING_AVERAGE_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (distance_ir_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_SET_DISTANCE_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (distance_ir_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_SET_SENSOR_TYPE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (distance_ir_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (distance_ir_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (distance_ir_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (distance_ir_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (distance_ir_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_distance_ir_v2_set_response_expected(TF_DistanceIRV2 *distance_ir_v2, uint8_t function_id, bool response_expected) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_DISTANCE_IR_V2_FUNCTION_SET_DISTANCE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                distance_ir_v2->response_expected[0] |= (1 << 0);
            } else {
                distance_ir_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_SET_ANALOG_VALUE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                distance_ir_v2->response_expected[0] |= (1 << 1);
            } else {
                distance_ir_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_SET_MOVING_AVERAGE_CONFIGURATION:
            if (response_expected) {
                distance_ir_v2->response_expected[0] |= (1 << 2);
            } else {
                distance_ir_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_SET_DISTANCE_LED_CONFIG:
            if (response_expected) {
                distance_ir_v2->response_expected[0] |= (1 << 3);
            } else {
                distance_ir_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_SET_SENSOR_TYPE:
            if (response_expected) {
                distance_ir_v2->response_expected[0] |= (1 << 4);
            } else {
                distance_ir_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                distance_ir_v2->response_expected[0] |= (1 << 5);
            } else {
                distance_ir_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                distance_ir_v2->response_expected[0] |= (1 << 6);
            } else {
                distance_ir_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_RESET:
            if (response_expected) {
                distance_ir_v2->response_expected[0] |= (1 << 7);
            } else {
                distance_ir_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_DISTANCE_IR_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                distance_ir_v2->response_expected[1] |= (1 << 0);
            } else {
                distance_ir_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_distance_ir_v2_set_response_expected_all(TF_DistanceIRV2 *distance_ir_v2, bool response_expected) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(distance_ir_v2->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_distance_ir_v2_get_distance(TF_DistanceIRV2 *distance_ir_v2, uint16_t *ret_distance) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_GET_DISTANCE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_distance != NULL) { *ret_distance = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_set_distance_callback_configuration(TF_DistanceIRV2 *distance_ir_v2, uint32_t period, bool value_has_to_change, char option, uint16_t min, uint16_t max) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_distance_ir_v2_get_response_expected(distance_ir_v2, TF_DISTANCE_IR_V2_FUNCTION_SET_DISTANCE_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_SET_DISTANCE_CALLBACK_CONFIGURATION, 10, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(distance_ir_v2->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;
    _send_buf[5] = (uint8_t)option;
    min = tf_leconvert_uint16_to(min); memcpy(_send_buf + 6, &min, 2);
    max = tf_leconvert_uint16_to(max); memcpy(_send_buf + 8, &max, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_get_distance_callback_configuration(TF_DistanceIRV2 *distance_ir_v2, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, uint16_t *ret_min, uint16_t *ret_max) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_GET_DISTANCE_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 10) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_option != NULL) { *ret_option = tf_packet_buffer_read_char(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_min != NULL) { *ret_min = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_max != NULL) { *ret_max = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 10) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_get_analog_value(TF_DistanceIRV2 *distance_ir_v2, uint32_t *ret_analog_value) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_GET_ANALOG_VALUE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_analog_value != NULL) { *ret_analog_value = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_set_analog_value_callback_configuration(TF_DistanceIRV2 *distance_ir_v2, uint32_t period, bool value_has_to_change, char option, uint32_t min, uint32_t max) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_distance_ir_v2_get_response_expected(distance_ir_v2, TF_DISTANCE_IR_V2_FUNCTION_SET_ANALOG_VALUE_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_SET_ANALOG_VALUE_CALLBACK_CONFIGURATION, 14, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(distance_ir_v2->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;
    _send_buf[5] = (uint8_t)option;
    min = tf_leconvert_uint32_to(min); memcpy(_send_buf + 6, &min, 4);
    max = tf_leconvert_uint32_to(max); memcpy(_send_buf + 10, &max, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_get_analog_value_callback_configuration(TF_DistanceIRV2 *distance_ir_v2, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, uint32_t *ret_min, uint32_t *ret_max) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_GET_ANALOG_VALUE_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 14) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_option != NULL) { *ret_option = tf_packet_buffer_read_char(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_min != NULL) { *ret_min = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_max != NULL) { *ret_max = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 14) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_set_moving_average_configuration(TF_DistanceIRV2 *distance_ir_v2, uint16_t moving_average_length) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_distance_ir_v2_get_response_expected(distance_ir_v2, TF_DISTANCE_IR_V2_FUNCTION_SET_MOVING_AVERAGE_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_SET_MOVING_AVERAGE_CONFIGURATION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(distance_ir_v2->tfp);

    moving_average_length = tf_leconvert_uint16_to(moving_average_length); memcpy(_send_buf + 0, &moving_average_length, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_get_moving_average_configuration(TF_DistanceIRV2 *distance_ir_v2, uint16_t *ret_moving_average_length) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_GET_MOVING_AVERAGE_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_moving_average_length != NULL) { *ret_moving_average_length = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_set_distance_led_config(TF_DistanceIRV2 *distance_ir_v2, uint8_t config) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_distance_ir_v2_get_response_expected(distance_ir_v2, TF_DISTANCE_IR_V2_FUNCTION_SET_DISTANCE_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_SET_DISTANCE_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(distance_ir_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_get_distance_led_config(TF_DistanceIRV2 *distance_ir_v2, uint8_t *ret_config) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_GET_DISTANCE_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_set_sensor_type(TF_DistanceIRV2 *distance_ir_v2, uint8_t sensor) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_distance_ir_v2_get_response_expected(distance_ir_v2, TF_DISTANCE_IR_V2_FUNCTION_SET_SENSOR_TYPE, &_response_expected);
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_SET_SENSOR_TYPE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(distance_ir_v2->tfp);

    _send_buf[0] = (uint8_t)sensor;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_get_sensor_type(TF_DistanceIRV2 *distance_ir_v2, uint8_t *ret_sensor) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_GET_SENSOR_TYPE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_sensor != NULL) { *ret_sensor = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_get_spitfp_error_count(TF_DistanceIRV2 *distance_ir_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_set_bootloader_mode(TF_DistanceIRV2 *distance_ir_v2, uint8_t mode, uint8_t *ret_status) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(distance_ir_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_get_bootloader_mode(TF_DistanceIRV2 *distance_ir_v2, uint8_t *ret_mode) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_set_write_firmware_pointer(TF_DistanceIRV2 *distance_ir_v2, uint32_t pointer) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_distance_ir_v2_get_response_expected(distance_ir_v2, TF_DISTANCE_IR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(distance_ir_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_write_firmware(TF_DistanceIRV2 *distance_ir_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(distance_ir_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_set_status_led_config(TF_DistanceIRV2 *distance_ir_v2, uint8_t config) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_distance_ir_v2_get_response_expected(distance_ir_v2, TF_DISTANCE_IR_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(distance_ir_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_get_status_led_config(TF_DistanceIRV2 *distance_ir_v2, uint8_t *ret_config) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_get_chip_temperature(TF_DistanceIRV2 *distance_ir_v2, int16_t *ret_temperature) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_reset(TF_DistanceIRV2 *distance_ir_v2) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_distance_ir_v2_get_response_expected(distance_ir_v2, TF_DISTANCE_IR_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_write_uid(TF_DistanceIRV2 *distance_ir_v2, uint32_t uid) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_distance_ir_v2_get_response_expected(distance_ir_v2, TF_DISTANCE_IR_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(distance_ir_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_read_uid(TF_DistanceIRV2 *distance_ir_v2, uint32_t *ret_uid) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_distance_ir_v2_get_identity(TF_DistanceIRV2 *distance_ir_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = distance_ir_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(distance_ir_v2->tfp, TF_DISTANCE_IR_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(distance_ir_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(distance_ir_v2->tfp);
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
        tf_tfp_packet_processed(distance_ir_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(distance_ir_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_distance_ir_v2_register_distance_callback(TF_DistanceIRV2 *distance_ir_v2, TF_DistanceIRV2_DistanceHandler handler, void *user_data) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    distance_ir_v2->distance_handler = handler;
    distance_ir_v2->distance_user_data = user_data;

    return TF_E_OK;
}


int tf_distance_ir_v2_register_analog_value_callback(TF_DistanceIRV2 *distance_ir_v2, TF_DistanceIRV2_AnalogValueHandler handler, void *user_data) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    distance_ir_v2->analog_value_handler = handler;
    distance_ir_v2->analog_value_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_distance_ir_v2_callback_tick(TF_DistanceIRV2 *distance_ir_v2, uint32_t timeout_us) {
    if (distance_ir_v2 == NULL) {
        return TF_E_NULL;
    }

    if (distance_ir_v2->magic != 0x5446 || distance_ir_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = distance_ir_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(distance_ir_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
