/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_laser_range_finder_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_laser_range_finder_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_LaserRangeFinderV2 *laser_range_finder_v2 = (TF_LaserRangeFinderV2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(laser_range_finder_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_LASER_RANGE_FINDER_V2_CALLBACK_DISTANCE: {
            TF_LaserRangeFinderV2_DistanceHandler fn = laser_range_finder_v2->distance_handler;
            void *user_data = laser_range_finder_v2->distance_user_data;
            if (fn == NULL) {
                return false;
            }

            int16_t distance = tf_packet_buffer_read_int16_t(payload);
            hal_common->locked = true;
            fn(laser_range_finder_v2, distance, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_LASER_RANGE_FINDER_V2_CALLBACK_VELOCITY: {
            TF_LaserRangeFinderV2_VelocityHandler fn = laser_range_finder_v2->velocity_handler;
            void *user_data = laser_range_finder_v2->velocity_user_data;
            if (fn == NULL) {
                return false;
            }

            int16_t velocity = tf_packet_buffer_read_int16_t(payload);
            hal_common->locked = true;
            fn(laser_range_finder_v2, velocity, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_laser_range_finder_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_laser_range_finder_v2_create(TF_LaserRangeFinderV2 *laser_range_finder_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (laser_range_finder_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(laser_range_finder_v2, 0, sizeof(TF_LaserRangeFinderV2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_LASER_RANGE_FINDER_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    laser_range_finder_v2->tfp = tfp;
    laser_range_finder_v2->tfp->device = laser_range_finder_v2;
    laser_range_finder_v2->tfp->cb_handler = tf_laser_range_finder_v2_callback_handler;
    laser_range_finder_v2->magic = 0x5446;
    laser_range_finder_v2->response_expected[0] = 0x03;
    laser_range_finder_v2->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_laser_range_finder_v2_destroy(TF_LaserRangeFinderV2 *laser_range_finder_v2) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }
    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    laser_range_finder_v2->tfp->cb_handler = NULL;
    laser_range_finder_v2->tfp->device = NULL;
    laser_range_finder_v2->tfp = NULL;
    laser_range_finder_v2->magic = 0;

    return TF_E_OK;
}

int tf_laser_range_finder_v2_get_response_expected(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint8_t function_id, bool *ret_response_expected) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_DISTANCE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (laser_range_finder_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_VELOCITY_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (laser_range_finder_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_ENABLE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (laser_range_finder_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (laser_range_finder_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_MOVING_AVERAGE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (laser_range_finder_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_OFFSET_CALIBRATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (laser_range_finder_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_DISTANCE_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (laser_range_finder_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (laser_range_finder_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (laser_range_finder_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (laser_range_finder_v2->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (laser_range_finder_v2->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_laser_range_finder_v2_set_response_expected(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint8_t function_id, bool response_expected) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_DISTANCE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                laser_range_finder_v2->response_expected[0] |= (1 << 0);
            } else {
                laser_range_finder_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_VELOCITY_CALLBACK_CONFIGURATION:
            if (response_expected) {
                laser_range_finder_v2->response_expected[0] |= (1 << 1);
            } else {
                laser_range_finder_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_ENABLE:
            if (response_expected) {
                laser_range_finder_v2->response_expected[0] |= (1 << 2);
            } else {
                laser_range_finder_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_CONFIGURATION:
            if (response_expected) {
                laser_range_finder_v2->response_expected[0] |= (1 << 3);
            } else {
                laser_range_finder_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_MOVING_AVERAGE:
            if (response_expected) {
                laser_range_finder_v2->response_expected[0] |= (1 << 4);
            } else {
                laser_range_finder_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_OFFSET_CALIBRATION:
            if (response_expected) {
                laser_range_finder_v2->response_expected[0] |= (1 << 5);
            } else {
                laser_range_finder_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_DISTANCE_LED_CONFIG:
            if (response_expected) {
                laser_range_finder_v2->response_expected[0] |= (1 << 6);
            } else {
                laser_range_finder_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                laser_range_finder_v2->response_expected[0] |= (1 << 7);
            } else {
                laser_range_finder_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                laser_range_finder_v2->response_expected[1] |= (1 << 0);
            } else {
                laser_range_finder_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_RESET:
            if (response_expected) {
                laser_range_finder_v2->response_expected[1] |= (1 << 1);
            } else {
                laser_range_finder_v2->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_LASER_RANGE_FINDER_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                laser_range_finder_v2->response_expected[1] |= (1 << 2);
            } else {
                laser_range_finder_v2->response_expected[1] &= ~(1 << 2);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_laser_range_finder_v2_set_response_expected_all(TF_LaserRangeFinderV2 *laser_range_finder_v2, bool response_expected) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(laser_range_finder_v2->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_laser_range_finder_v2_get_distance(TF_LaserRangeFinderV2 *laser_range_finder_v2, int16_t *ret_distance) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_DISTANCE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_distance != NULL) { *ret_distance = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_set_distance_callback_configuration(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint32_t period, bool value_has_to_change, char option, int16_t min, int16_t max) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_laser_range_finder_v2_get_response_expected(laser_range_finder_v2, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_DISTANCE_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_DISTANCE_CALLBACK_CONFIGURATION, 10, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(laser_range_finder_v2->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;
    _send_buf[5] = (uint8_t)option;
    min = tf_leconvert_int16_to(min); memcpy(_send_buf + 6, &min, 2);
    max = tf_leconvert_int16_to(max); memcpy(_send_buf + 8, &max, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_distance_callback_configuration(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, int16_t *ret_min, int16_t *ret_max) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_DISTANCE_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 10) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_option != NULL) { *ret_option = tf_packet_buffer_read_char(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_min != NULL) { *ret_min = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_max != NULL) { *ret_max = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 10) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_velocity(TF_LaserRangeFinderV2 *laser_range_finder_v2, int16_t *ret_velocity) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_VELOCITY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_velocity != NULL) { *ret_velocity = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_set_velocity_callback_configuration(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint32_t period, bool value_has_to_change, char option, int16_t min, int16_t max) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_laser_range_finder_v2_get_response_expected(laser_range_finder_v2, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_VELOCITY_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_VELOCITY_CALLBACK_CONFIGURATION, 10, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(laser_range_finder_v2->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;
    _send_buf[5] = (uint8_t)option;
    min = tf_leconvert_int16_to(min); memcpy(_send_buf + 6, &min, 2);
    max = tf_leconvert_int16_to(max); memcpy(_send_buf + 8, &max, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_velocity_callback_configuration(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, int16_t *ret_min, int16_t *ret_max) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_VELOCITY_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 10) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_option != NULL) { *ret_option = tf_packet_buffer_read_char(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_min != NULL) { *ret_min = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_max != NULL) { *ret_max = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 10) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_set_enable(TF_LaserRangeFinderV2 *laser_range_finder_v2, bool enable) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_laser_range_finder_v2_get_response_expected(laser_range_finder_v2, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_ENABLE, &_response_expected);
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_ENABLE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(laser_range_finder_v2->tfp);

    _send_buf[0] = enable ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_enable(TF_LaserRangeFinderV2 *laser_range_finder_v2, bool *ret_enable) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_ENABLE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enable != NULL) { *ret_enable = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_set_configuration(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint8_t acquisition_count, bool enable_quick_termination, uint8_t threshold_value, uint16_t measurement_frequency) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_laser_range_finder_v2_get_response_expected(laser_range_finder_v2, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(laser_range_finder_v2->tfp);

    _send_buf[0] = (uint8_t)acquisition_count;
    _send_buf[1] = enable_quick_termination ? 1 : 0;
    _send_buf[2] = (uint8_t)threshold_value;
    measurement_frequency = tf_leconvert_uint16_to(measurement_frequency); memcpy(_send_buf + 3, &measurement_frequency, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_configuration(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint8_t *ret_acquisition_count, bool *ret_enable_quick_termination, uint8_t *ret_threshold_value, uint16_t *ret_measurement_frequency) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_acquisition_count != NULL) { *ret_acquisition_count = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_enable_quick_termination != NULL) { *ret_enable_quick_termination = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_threshold_value != NULL) { *ret_threshold_value = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_measurement_frequency != NULL) { *ret_measurement_frequency = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_set_moving_average(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint8_t distance_average_length, uint8_t velocity_average_length) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_laser_range_finder_v2_get_response_expected(laser_range_finder_v2, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_MOVING_AVERAGE, &_response_expected);
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_MOVING_AVERAGE, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(laser_range_finder_v2->tfp);

    _send_buf[0] = (uint8_t)distance_average_length;
    _send_buf[1] = (uint8_t)velocity_average_length;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_moving_average(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint8_t *ret_distance_average_length, uint8_t *ret_velocity_average_length) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_MOVING_AVERAGE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_distance_average_length != NULL) { *ret_distance_average_length = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_velocity_average_length != NULL) { *ret_velocity_average_length = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_set_offset_calibration(TF_LaserRangeFinderV2 *laser_range_finder_v2, int16_t offset) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_laser_range_finder_v2_get_response_expected(laser_range_finder_v2, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_OFFSET_CALIBRATION, &_response_expected);
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_OFFSET_CALIBRATION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(laser_range_finder_v2->tfp);

    offset = tf_leconvert_int16_to(offset); memcpy(_send_buf + 0, &offset, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_offset_calibration(TF_LaserRangeFinderV2 *laser_range_finder_v2, int16_t *ret_offset) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_OFFSET_CALIBRATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_offset != NULL) { *ret_offset = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_set_distance_led_config(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint8_t config) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_laser_range_finder_v2_get_response_expected(laser_range_finder_v2, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_DISTANCE_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_DISTANCE_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(laser_range_finder_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_distance_led_config(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint8_t *ret_config) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_DISTANCE_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_spitfp_error_count(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_set_bootloader_mode(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint8_t mode, uint8_t *ret_status) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(laser_range_finder_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_bootloader_mode(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint8_t *ret_mode) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_set_write_firmware_pointer(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint32_t pointer) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_laser_range_finder_v2_get_response_expected(laser_range_finder_v2, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(laser_range_finder_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_write_firmware(TF_LaserRangeFinderV2 *laser_range_finder_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(laser_range_finder_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_set_status_led_config(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint8_t config) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_laser_range_finder_v2_get_response_expected(laser_range_finder_v2, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(laser_range_finder_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_status_led_config(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint8_t *ret_config) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_chip_temperature(TF_LaserRangeFinderV2 *laser_range_finder_v2, int16_t *ret_temperature) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_reset(TF_LaserRangeFinderV2 *laser_range_finder_v2) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_laser_range_finder_v2_get_response_expected(laser_range_finder_v2, TF_LASER_RANGE_FINDER_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_write_uid(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint32_t uid) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_laser_range_finder_v2_get_response_expected(laser_range_finder_v2, TF_LASER_RANGE_FINDER_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(laser_range_finder_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_read_uid(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint32_t *ret_uid) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_laser_range_finder_v2_get_identity(TF_LaserRangeFinderV2 *laser_range_finder_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = laser_range_finder_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(laser_range_finder_v2->tfp, TF_LASER_RANGE_FINDER_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(laser_range_finder_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(laser_range_finder_v2->tfp);
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
        tf_tfp_packet_processed(laser_range_finder_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(laser_range_finder_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_laser_range_finder_v2_register_distance_callback(TF_LaserRangeFinderV2 *laser_range_finder_v2, TF_LaserRangeFinderV2_DistanceHandler handler, void *user_data) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    laser_range_finder_v2->distance_handler = handler;
    laser_range_finder_v2->distance_user_data = user_data;

    return TF_E_OK;
}


int tf_laser_range_finder_v2_register_velocity_callback(TF_LaserRangeFinderV2 *laser_range_finder_v2, TF_LaserRangeFinderV2_VelocityHandler handler, void *user_data) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    laser_range_finder_v2->velocity_handler = handler;
    laser_range_finder_v2->velocity_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_laser_range_finder_v2_callback_tick(TF_LaserRangeFinderV2 *laser_range_finder_v2, uint32_t timeout_us) {
    if (laser_range_finder_v2 == NULL) {
        return TF_E_NULL;
    }

    if (laser_range_finder_v2->magic != 0x5446 || laser_range_finder_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = laser_range_finder_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(laser_range_finder_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
