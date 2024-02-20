/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_performance_stepper.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


static bool tf_performance_stepper_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    (void)device;
    (void)fid;
    (void)payload;

    return false;
}
int tf_performance_stepper_create(TF_PerformanceStepper *performance_stepper, const char *uid_or_port_name, TF_HAL *hal) {
    if (performance_stepper == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(performance_stepper, 0, sizeof(TF_PerformanceStepper));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_PERFORMANCE_STEPPER_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    performance_stepper->tfp = tfp;
    performance_stepper->tfp->device = performance_stepper;
    performance_stepper->tfp->cb_handler = tf_performance_stepper_callback_handler;
    performance_stepper->magic = 0x5446;
    performance_stepper->response_expected[0] = 0x00;
    performance_stepper->response_expected[1] = 0x00;
    performance_stepper->response_expected[2] = 0x00;
    return TF_E_OK;
}

int tf_performance_stepper_destroy(TF_PerformanceStepper *performance_stepper) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }
    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    performance_stepper->tfp->cb_handler = NULL;
    performance_stepper->tfp->device = NULL;
    performance_stepper->tfp = NULL;
    performance_stepper->magic = 0;

    return TF_E_OK;
}

int tf_performance_stepper_get_response_expected(TF_PerformanceStepper *performance_stepper, uint8_t function_id, bool *ret_response_expected) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTION_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_CURRENT_POSITION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_TARGET_POSITION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEP_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTOR_CURRENT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLED:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_BASIC_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_SPREADCYCLE_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEALTH_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_COOLSTEP_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_SHORT_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 4)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_ACTION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 5)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_ERROR_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 6)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLE_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 7)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[2] & (1 << 0)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[2] & (1 << 1)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[2] & (1 << 2)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[2] & (1 << 3)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[2] & (1 << 4)) != 0;
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_stepper->response_expected[2] & (1 << 5)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_performance_stepper_set_response_expected(TF_PerformanceStepper *performance_stepper, uint8_t function_id, bool response_expected) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTION_CONFIGURATION:
            if (response_expected) {
                performance_stepper->response_expected[0] |= (1 << 0);
            } else {
                performance_stepper->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_CURRENT_POSITION:
            if (response_expected) {
                performance_stepper->response_expected[0] |= (1 << 1);
            } else {
                performance_stepper->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_TARGET_POSITION:
            if (response_expected) {
                performance_stepper->response_expected[0] |= (1 << 2);
            } else {
                performance_stepper->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS:
            if (response_expected) {
                performance_stepper->response_expected[0] |= (1 << 3);
            } else {
                performance_stepper->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEP_CONFIGURATION:
            if (response_expected) {
                performance_stepper->response_expected[0] |= (1 << 4);
            } else {
                performance_stepper->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTOR_CURRENT:
            if (response_expected) {
                performance_stepper->response_expected[0] |= (1 << 5);
            } else {
                performance_stepper->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLED:
            if (response_expected) {
                performance_stepper->response_expected[0] |= (1 << 6);
            } else {
                performance_stepper->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_BASIC_CONFIGURATION:
            if (response_expected) {
                performance_stepper->response_expected[0] |= (1 << 7);
            } else {
                performance_stepper->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_SPREADCYCLE_CONFIGURATION:
            if (response_expected) {
                performance_stepper->response_expected[1] |= (1 << 0);
            } else {
                performance_stepper->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEALTH_CONFIGURATION:
            if (response_expected) {
                performance_stepper->response_expected[1] |= (1 << 1);
            } else {
                performance_stepper->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_COOLSTEP_CONFIGURATION:
            if (response_expected) {
                performance_stepper->response_expected[1] |= (1 << 2);
            } else {
                performance_stepper->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_SHORT_CONFIGURATION:
            if (response_expected) {
                performance_stepper->response_expected[1] |= (1 << 3);
            } else {
                performance_stepper->response_expected[1] &= ~(1 << 3);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_CONFIGURATION:
            if (response_expected) {
                performance_stepper->response_expected[1] |= (1 << 4);
            } else {
                performance_stepper->response_expected[1] &= ~(1 << 4);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_ACTION:
            if (response_expected) {
                performance_stepper->response_expected[1] |= (1 << 5);
            } else {
                performance_stepper->response_expected[1] &= ~(1 << 5);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_ERROR_LED_CONFIG:
            if (response_expected) {
                performance_stepper->response_expected[1] |= (1 << 6);
            } else {
                performance_stepper->response_expected[1] &= ~(1 << 6);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLE_LED_CONFIG:
            if (response_expected) {
                performance_stepper->response_expected[1] |= (1 << 7);
            } else {
                performance_stepper->response_expected[1] &= ~(1 << 7);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS_LED_CONFIG:
            if (response_expected) {
                performance_stepper->response_expected[2] |= (1 << 0);
            } else {
                performance_stepper->response_expected[2] &= ~(1 << 0);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_LED_CONFIG:
            if (response_expected) {
                performance_stepper->response_expected[2] |= (1 << 1);
            } else {
                performance_stepper->response_expected[2] &= ~(1 << 1);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                performance_stepper->response_expected[2] |= (1 << 2);
            } else {
                performance_stepper->response_expected[2] &= ~(1 << 2);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                performance_stepper->response_expected[2] |= (1 << 3);
            } else {
                performance_stepper->response_expected[2] &= ~(1 << 3);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_RESET:
            if (response_expected) {
                performance_stepper->response_expected[2] |= (1 << 4);
            } else {
                performance_stepper->response_expected[2] &= ~(1 << 4);
            }
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_UID:
            if (response_expected) {
                performance_stepper->response_expected[2] |= (1 << 5);
            } else {
                performance_stepper->response_expected[2] &= ~(1 << 5);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_performance_stepper_set_response_expected_all(TF_PerformanceStepper *performance_stepper, bool response_expected) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(performance_stepper->response_expected, response_expected ? 0xFF : 0, 3);

    return TF_E_OK;
}

int tf_performance_stepper_set_motion_configuration(TF_PerformanceStepper *performance_stepper, uint8_t ramping_mode, int32_t velocity_start, int32_t acceleration_1, int32_t velocity_1, int32_t acceleration_max, int32_t velocity_max, int32_t deceleration_max, int32_t deceleration_1, int32_t velocity_stop, int32_t ramp_zero_wait) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTION_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTION_CONFIGURATION, 37, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)ramping_mode;
    velocity_start = tf_leconvert_int32_to(velocity_start); memcpy(_send_buf + 1, &velocity_start, 4);
    acceleration_1 = tf_leconvert_int32_to(acceleration_1); memcpy(_send_buf + 5, &acceleration_1, 4);
    velocity_1 = tf_leconvert_int32_to(velocity_1); memcpy(_send_buf + 9, &velocity_1, 4);
    acceleration_max = tf_leconvert_int32_to(acceleration_max); memcpy(_send_buf + 13, &acceleration_max, 4);
    velocity_max = tf_leconvert_int32_to(velocity_max); memcpy(_send_buf + 17, &velocity_max, 4);
    deceleration_max = tf_leconvert_int32_to(deceleration_max); memcpy(_send_buf + 21, &deceleration_max, 4);
    deceleration_1 = tf_leconvert_int32_to(deceleration_1); memcpy(_send_buf + 25, &deceleration_1, 4);
    velocity_stop = tf_leconvert_int32_to(velocity_stop); memcpy(_send_buf + 29, &velocity_stop, 4);
    ramp_zero_wait = tf_leconvert_int32_to(ramp_zero_wait); memcpy(_send_buf + 33, &ramp_zero_wait, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_motion_configuration(TF_PerformanceStepper *performance_stepper, uint8_t *ret_ramping_mode, int32_t *ret_velocity_start, int32_t *ret_acceleration_1, int32_t *ret_velocity_1, int32_t *ret_acceleration_max, int32_t *ret_velocity_max, int32_t *ret_deceleration_max, int32_t *ret_deceleration_1, int32_t *ret_velocity_stop, int32_t *ret_ramp_zero_wait) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_MOTION_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 37) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_ramping_mode != NULL) { *ret_ramping_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_velocity_start != NULL) { *ret_velocity_start = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_acceleration_1 != NULL) { *ret_acceleration_1 = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_velocity_1 != NULL) { *ret_velocity_1 = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_acceleration_max != NULL) { *ret_acceleration_max = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_velocity_max != NULL) { *ret_velocity_max = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_deceleration_max != NULL) { *ret_deceleration_max = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_deceleration_1 != NULL) { *ret_deceleration_1 = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_velocity_stop != NULL) { *ret_velocity_stop = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_ramp_zero_wait != NULL) { *ret_ramp_zero_wait = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 37) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_current_position(TF_PerformanceStepper *performance_stepper, int32_t position) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_CURRENT_POSITION, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_CURRENT_POSITION, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    position = tf_leconvert_int32_to(position); memcpy(_send_buf + 0, &position, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_current_position(TF_PerformanceStepper *performance_stepper, int32_t *ret_position) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_CURRENT_POSITION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_position != NULL) { *ret_position = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_current_velocity(TF_PerformanceStepper *performance_stepper, int32_t *ret_velocity) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_CURRENT_VELOCITY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_velocity != NULL) { *ret_velocity = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_target_position(TF_PerformanceStepper *performance_stepper, int32_t position) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_TARGET_POSITION, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_TARGET_POSITION, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    position = tf_leconvert_int32_to(position); memcpy(_send_buf + 0, &position, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_target_position(TF_PerformanceStepper *performance_stepper, int32_t *ret_position) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_TARGET_POSITION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_position != NULL) { *ret_position = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_steps(TF_PerformanceStepper *performance_stepper, int32_t steps) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    steps = tf_leconvert_int32_to(steps); memcpy(_send_buf + 0, &steps, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_steps(TF_PerformanceStepper *performance_stepper, int32_t *ret_steps) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_STEPS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_steps != NULL) { *ret_steps = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_remaining_steps(TF_PerformanceStepper *performance_stepper, int32_t *ret_steps) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_REMAINING_STEPS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_steps != NULL) { *ret_steps = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_step_configuration(TF_PerformanceStepper *performance_stepper, uint8_t step_resolution, bool interpolation) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEP_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEP_CONFIGURATION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)step_resolution;
    _send_buf[1] = interpolation ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_step_configuration(TF_PerformanceStepper *performance_stepper, uint8_t *ret_step_resolution, bool *ret_interpolation) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_STEP_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_step_resolution != NULL) { *ret_step_resolution = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_interpolation != NULL) { *ret_interpolation = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_motor_current(TF_PerformanceStepper *performance_stepper, uint16_t current) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTOR_CURRENT, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTOR_CURRENT, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    current = tf_leconvert_uint16_to(current); memcpy(_send_buf + 0, &current, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_motor_current(TF_PerformanceStepper *performance_stepper, uint16_t *ret_current) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_MOTOR_CURRENT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_current != NULL) { *ret_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_enabled(TF_PerformanceStepper *performance_stepper, bool enabled) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLED, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLED, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_enabled(TF_PerformanceStepper *performance_stepper, bool *ret_enabled) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_ENABLED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_basic_configuration(TF_PerformanceStepper *performance_stepper, uint16_t standstill_current, uint16_t motor_run_current, uint16_t standstill_delay_time, uint16_t power_down_time, uint16_t stealth_threshold, uint16_t coolstep_threshold, uint16_t classic_threshold, bool high_velocity_chopper_mode) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_BASIC_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_BASIC_CONFIGURATION, 15, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    standstill_current = tf_leconvert_uint16_to(standstill_current); memcpy(_send_buf + 0, &standstill_current, 2);
    motor_run_current = tf_leconvert_uint16_to(motor_run_current); memcpy(_send_buf + 2, &motor_run_current, 2);
    standstill_delay_time = tf_leconvert_uint16_to(standstill_delay_time); memcpy(_send_buf + 4, &standstill_delay_time, 2);
    power_down_time = tf_leconvert_uint16_to(power_down_time); memcpy(_send_buf + 6, &power_down_time, 2);
    stealth_threshold = tf_leconvert_uint16_to(stealth_threshold); memcpy(_send_buf + 8, &stealth_threshold, 2);
    coolstep_threshold = tf_leconvert_uint16_to(coolstep_threshold); memcpy(_send_buf + 10, &coolstep_threshold, 2);
    classic_threshold = tf_leconvert_uint16_to(classic_threshold); memcpy(_send_buf + 12, &classic_threshold, 2);
    _send_buf[14] = high_velocity_chopper_mode ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_basic_configuration(TF_PerformanceStepper *performance_stepper, uint16_t *ret_standstill_current, uint16_t *ret_motor_run_current, uint16_t *ret_standstill_delay_time, uint16_t *ret_power_down_time, uint16_t *ret_stealth_threshold, uint16_t *ret_coolstep_threshold, uint16_t *ret_classic_threshold, bool *ret_high_velocity_chopper_mode) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_BASIC_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 15) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_standstill_current != NULL) { *ret_standstill_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_motor_run_current != NULL) { *ret_motor_run_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_standstill_delay_time != NULL) { *ret_standstill_delay_time = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_power_down_time != NULL) { *ret_power_down_time = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_stealth_threshold != NULL) { *ret_stealth_threshold = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_coolstep_threshold != NULL) { *ret_coolstep_threshold = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_classic_threshold != NULL) { *ret_classic_threshold = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_high_velocity_chopper_mode != NULL) { *ret_high_velocity_chopper_mode = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 15) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_spreadcycle_configuration(TF_PerformanceStepper *performance_stepper, uint8_t slow_decay_duration, bool high_velocity_fullstep, uint8_t fast_decay_duration, uint8_t hysteresis_start_value, int8_t hysteresis_end_value, int8_t sine_wave_offset, uint8_t chopper_mode, uint8_t comparator_blank_time, bool fast_decay_without_comparator) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_SPREADCYCLE_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_SPREADCYCLE_CONFIGURATION, 9, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)slow_decay_duration;
    _send_buf[1] = high_velocity_fullstep ? 1 : 0;
    _send_buf[2] = (uint8_t)fast_decay_duration;
    _send_buf[3] = (uint8_t)hysteresis_start_value;
    _send_buf[4] = (uint8_t)hysteresis_end_value;
    _send_buf[5] = (uint8_t)sine_wave_offset;
    _send_buf[6] = (uint8_t)chopper_mode;
    _send_buf[7] = (uint8_t)comparator_blank_time;
    _send_buf[8] = fast_decay_without_comparator ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_spreadcycle_configuration(TF_PerformanceStepper *performance_stepper, uint8_t *ret_slow_decay_duration, bool *ret_high_velocity_fullstep, bool *ret_enable_random_slow_decay, uint8_t *ret_fast_decay_duration, uint8_t *ret_hysteresis_start_value, int8_t *ret_hysteresis_end_value, int8_t *ret_sine_wave_offset, uint8_t *ret_chopper_mode, uint8_t *ret_comparator_blank_time, bool *ret_fast_decay_without_comparator) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_SPREADCYCLE_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 10) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_slow_decay_duration != NULL) { *ret_slow_decay_duration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_high_velocity_fullstep != NULL) { *ret_high_velocity_fullstep = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_enable_random_slow_decay != NULL) { *ret_enable_random_slow_decay = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_fast_decay_duration != NULL) { *ret_fast_decay_duration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_hysteresis_start_value != NULL) { *ret_hysteresis_start_value = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_hysteresis_end_value != NULL) { *ret_hysteresis_end_value = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_sine_wave_offset != NULL) { *ret_sine_wave_offset = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_chopper_mode != NULL) { *ret_chopper_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_comparator_blank_time != NULL) { *ret_comparator_blank_time = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_fast_decay_without_comparator != NULL) { *ret_fast_decay_without_comparator = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 10) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_stealth_configuration(TF_PerformanceStepper *performance_stepper, bool enable_stealth, uint8_t offset, uint8_t gradient, bool enable_autoscale, bool enable_autogradient, uint8_t freewheel_mode, uint8_t regulation_loop_gradient, uint8_t amplitude_limit) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEALTH_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEALTH_CONFIGURATION, 8, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = enable_stealth ? 1 : 0;
    _send_buf[1] = (uint8_t)offset;
    _send_buf[2] = (uint8_t)gradient;
    _send_buf[3] = enable_autoscale ? 1 : 0;
    _send_buf[4] = enable_autogradient ? 1 : 0;
    _send_buf[5] = (uint8_t)freewheel_mode;
    _send_buf[6] = (uint8_t)regulation_loop_gradient;
    _send_buf[7] = (uint8_t)amplitude_limit;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_stealth_configuration(TF_PerformanceStepper *performance_stepper, bool *ret_enable_stealth, uint8_t *ret_offset, uint8_t *ret_gradient, bool *ret_enable_autoscale, bool *ret_enable_autogradient, uint8_t *ret_freewheel_mode, uint8_t *ret_regulation_loop_gradient, uint8_t *ret_amplitude_limit) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_STEALTH_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enable_stealth != NULL) { *ret_enable_stealth = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_offset != NULL) { *ret_offset = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_gradient != NULL) { *ret_gradient = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_enable_autoscale != NULL) { *ret_enable_autoscale = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_enable_autogradient != NULL) { *ret_enable_autogradient = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_freewheel_mode != NULL) { *ret_freewheel_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_regulation_loop_gradient != NULL) { *ret_regulation_loop_gradient = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_amplitude_limit != NULL) { *ret_amplitude_limit = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_coolstep_configuration(TF_PerformanceStepper *performance_stepper, uint8_t minimum_stallguard_value, uint8_t maximum_stallguard_value, uint8_t current_up_step_width, uint8_t current_down_step_width, uint8_t minimum_current, int8_t stallguard_threshold_value, uint8_t stallguard_mode) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_COOLSTEP_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_COOLSTEP_CONFIGURATION, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)minimum_stallguard_value;
    _send_buf[1] = (uint8_t)maximum_stallguard_value;
    _send_buf[2] = (uint8_t)current_up_step_width;
    _send_buf[3] = (uint8_t)current_down_step_width;
    _send_buf[4] = (uint8_t)minimum_current;
    _send_buf[5] = (uint8_t)stallguard_threshold_value;
    _send_buf[6] = (uint8_t)stallguard_mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_coolstep_configuration(TF_PerformanceStepper *performance_stepper, uint8_t *ret_minimum_stallguard_value, uint8_t *ret_maximum_stallguard_value, uint8_t *ret_current_up_step_width, uint8_t *ret_current_down_step_width, uint8_t *ret_minimum_current, int8_t *ret_stallguard_threshold_value, uint8_t *ret_stallguard_mode) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_COOLSTEP_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 7) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_minimum_stallguard_value != NULL) { *ret_minimum_stallguard_value = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_maximum_stallguard_value != NULL) { *ret_maximum_stallguard_value = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_current_up_step_width != NULL) { *ret_current_up_step_width = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_current_down_step_width != NULL) { *ret_current_down_step_width = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_minimum_current != NULL) { *ret_minimum_current = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_stallguard_threshold_value != NULL) { *ret_stallguard_threshold_value = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_stallguard_mode != NULL) { *ret_stallguard_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 7) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_short_configuration(TF_PerformanceStepper *performance_stepper, bool disable_short_to_voltage_protection, bool disable_short_to_ground_protection, uint8_t short_to_voltage_level, uint8_t short_to_ground_level, uint8_t spike_filter_bandwidth, bool short_detection_delay, uint8_t filter_time) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_SHORT_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_SHORT_CONFIGURATION, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = disable_short_to_voltage_protection ? 1 : 0;
    _send_buf[1] = disable_short_to_ground_protection ? 1 : 0;
    _send_buf[2] = (uint8_t)short_to_voltage_level;
    _send_buf[3] = (uint8_t)short_to_ground_level;
    _send_buf[4] = (uint8_t)spike_filter_bandwidth;
    _send_buf[5] = short_detection_delay ? 1 : 0;
    _send_buf[6] = (uint8_t)filter_time;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_short_configuration(TF_PerformanceStepper *performance_stepper, bool *ret_disable_short_to_voltage_protection, bool *ret_disable_short_to_ground_protection, uint8_t *ret_short_to_voltage_level, uint8_t *ret_short_to_ground_level, uint8_t *ret_spike_filter_bandwidth, bool *ret_short_detection_delay, uint8_t *ret_filter_time) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_SHORT_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 7) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_disable_short_to_voltage_protection != NULL) { *ret_disable_short_to_voltage_protection = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_disable_short_to_ground_protection != NULL) { *ret_disable_short_to_ground_protection = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_short_to_voltage_level != NULL) { *ret_short_to_voltage_level = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_short_to_ground_level != NULL) { *ret_short_to_ground_level = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_spike_filter_bandwidth != NULL) { *ret_spike_filter_bandwidth = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_short_detection_delay != NULL) { *ret_short_detection_delay = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_filter_time != NULL) { *ret_filter_time = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 7) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_driver_status(TF_PerformanceStepper *performance_stepper, uint8_t *ret_open_load, uint8_t *ret_short_to_ground, uint8_t *ret_over_temperature, bool *ret_motor_stalled, uint8_t *ret_actual_motor_current, bool *ret_full_step_active, uint8_t *ret_stallguard_result, uint8_t *ret_stealth_voltage_amplitude) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_DRIVER_STATUS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_open_load != NULL) { *ret_open_load = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_short_to_ground != NULL) { *ret_short_to_ground = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_over_temperature != NULL) { *ret_over_temperature = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_motor_stalled != NULL) { *ret_motor_stalled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_actual_motor_current != NULL) { *ret_actual_motor_current = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_full_step_active != NULL) { *ret_full_step_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_stallguard_result != NULL) { *ret_stallguard_result = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_stealth_voltage_amplitude != NULL) { *ret_stealth_voltage_amplitude = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_input_voltage(TF_PerformanceStepper *performance_stepper, uint16_t *ret_voltage) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_INPUT_VOLTAGE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_voltage != NULL) { *ret_voltage = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_temperature(TF_PerformanceStepper *performance_stepper, int16_t *ret_temperature) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_gpio_configuration(TF_PerformanceStepper *performance_stepper, uint16_t debounce, int32_t stop_deceleration) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_CONFIGURATION, 6, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    debounce = tf_leconvert_uint16_to(debounce); memcpy(_send_buf + 0, &debounce, 2);
    stop_deceleration = tf_leconvert_int32_to(stop_deceleration); memcpy(_send_buf + 2, &stop_deceleration, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_gpio_configuration(TF_PerformanceStepper *performance_stepper, uint16_t *ret_debounce, int32_t *ret_stop_deceleration) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_GPIO_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_debounce != NULL) { *ret_debounce = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_stop_deceleration != NULL) { *ret_stop_deceleration = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_gpio_action(TF_PerformanceStepper *performance_stepper, uint8_t channel, uint32_t action) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_ACTION, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_ACTION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)channel;
    action = tf_leconvert_uint32_to(action); memcpy(_send_buf + 1, &action, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_gpio_action(TF_PerformanceStepper *performance_stepper, uint8_t channel, uint32_t *ret_action) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_GPIO_ACTION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_action != NULL) { *ret_action = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_gpio_state(TF_PerformanceStepper *performance_stepper, bool ret_gpio_state[2]) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_GPIO_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_gpio_state != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_gpio_state, 2);} else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_error_led_config(TF_PerformanceStepper *performance_stepper, uint8_t config) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_ERROR_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_ERROR_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_error_led_config(TF_PerformanceStepper *performance_stepper, uint8_t *ret_config) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_ERROR_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_enable_led_config(TF_PerformanceStepper *performance_stepper, uint8_t config) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLE_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLE_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_enable_led_config(TF_PerformanceStepper *performance_stepper, uint8_t *ret_config) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_ENABLE_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_steps_led_config(TF_PerformanceStepper *performance_stepper, uint8_t config) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_steps_led_config(TF_PerformanceStepper *performance_stepper, uint8_t *ret_config) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_STEPS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_gpio_led_config(TF_PerformanceStepper *performance_stepper, uint8_t channel, uint8_t config) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_LED_CONFIG, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_gpio_led_config(TF_PerformanceStepper *performance_stepper, uint8_t channel, uint8_t *ret_config) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_GPIO_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_write_register(TF_PerformanceStepper *performance_stepper, uint8_t register_, uint32_t value, uint8_t *ret_status) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_REGISTER, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)register_;
    value = tf_leconvert_uint32_to(value); memcpy(_send_buf + 1, &value, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_read_register(TF_PerformanceStepper *performance_stepper, uint8_t register_, uint8_t *ret_status, uint32_t *ret_value) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_READ_REGISTER, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)register_;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_value != NULL) { *ret_value = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_spitfp_error_count(TF_PerformanceStepper *performance_stepper, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_bootloader_mode(TF_PerformanceStepper *performance_stepper, uint8_t mode, uint8_t *ret_status) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_bootloader_mode(TF_PerformanceStepper *performance_stepper, uint8_t *ret_mode) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_write_firmware_pointer(TF_PerformanceStepper *performance_stepper, uint32_t pointer) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_write_firmware(TF_PerformanceStepper *performance_stepper, const uint8_t data[64], uint8_t *ret_status) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_set_status_led_config(TF_PerformanceStepper *performance_stepper, uint8_t config) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_status_led_config(TF_PerformanceStepper *performance_stepper, uint8_t *ret_config) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_chip_temperature(TF_PerformanceStepper *performance_stepper, int16_t *ret_temperature) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_reset(TF_PerformanceStepper *performance_stepper) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_write_uid(TF_PerformanceStepper *performance_stepper, uint32_t uid) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_stepper->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_read_uid(TF_PerformanceStepper *performance_stepper, uint32_t *ret_uid) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_stepper_get_identity(TF_PerformanceStepper *performance_stepper, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_stepper->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_stepper->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_stepper->tfp);
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
        tf_tfp_packet_processed(performance_stepper->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_stepper->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}


int tf_performance_stepper_callback_tick(TF_PerformanceStepper *performance_stepper, uint32_t timeout_us) {
    if (performance_stepper == NULL) {
        return TF_E_NULL;
    }

    if (performance_stepper->magic != 0x5446 || performance_stepper->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = performance_stepper->tfp->spitfp->hal;

    return tf_tfp_callback_tick(performance_stepper->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
