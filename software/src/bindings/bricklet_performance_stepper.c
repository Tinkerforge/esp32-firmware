/* ***********************************************************
 * This file was automatically generated on 2021-11-08.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.0         *
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


static bool tf_performance_stepper_callback_handler(void *dev, uint8_t fid, TF_Packetbuffer *payload) {
    (void)dev;
    (void)fid;
    (void)payload;
    return false;
}
int tf_performance_stepper_create(TF_PerformanceStepper *performance_stepper, const char *uid, TF_HalContext *hal) {
    if (performance_stepper == NULL || uid == NULL || hal == NULL)
        return TF_E_NULL;

    memset(performance_stepper, 0, sizeof(TF_PerformanceStepper));

    uint32_t numeric_uid;
    int rc = tf_base58_decode(uid, &numeric_uid);
    if (rc != TF_E_OK) {
        return rc;
    }

    uint8_t port_id;
    uint8_t inventory_index;
    rc = tf_hal_get_port_id(hal, numeric_uid, &port_id, &inventory_index);
    if (rc < 0) {
        return rc;
    }

    rc = tf_hal_get_tfp(hal, &performance_stepper->tfp, TF_PERFORMANCE_STEPPER_DEVICE_IDENTIFIER, inventory_index);
    if (rc != TF_E_OK) {
        return rc;
    }
    performance_stepper->tfp->device = performance_stepper;
    performance_stepper->tfp->uid = numeric_uid;
    performance_stepper->tfp->cb_handler = tf_performance_stepper_callback_handler;
    performance_stepper->response_expected[0] = 0x00;
    performance_stepper->response_expected[1] = 0x00;
    performance_stepper->response_expected[2] = 0x00;
    return TF_E_OK;
}

int tf_performance_stepper_destroy(TF_PerformanceStepper *performance_stepper) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    int result = tf_tfp_destroy(performance_stepper->tfp);
    performance_stepper->tfp = NULL;
    return result;
}

int tf_performance_stepper_get_response_expected(TF_PerformanceStepper *performance_stepper, uint8_t function_id, bool *ret_response_expected) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    switch(function_id) {
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTION_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 0)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_CURRENT_POSITION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 1)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_TARGET_POSITION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 2)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 3)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEP_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 4)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTOR_CURRENT:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 5)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLED:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 6)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_BASIC_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[0] & (1 << 7)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_SPREADCYCLE_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 0)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEALTH_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 1)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_COOLSTEP_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 2)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_SHORT_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 3)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 4)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_ACTION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 5)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_ERROR_LED_CONFIG:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 6)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLE_LED_CONFIG:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[1] & (1 << 7)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS_LED_CONFIG:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[2] & (1 << 0)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_LED_CONFIG:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[2] & (1 << 1)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[2] & (1 << 2)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_SET_STATUS_LED_CONFIG:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[2] & (1 << 3)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_RESET:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[2] & (1 << 4)) != 0;
            break;
        case TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_UID:
            if(ret_response_expected != NULL)
                *ret_response_expected = (performance_stepper->response_expected[2] & (1 << 5)) != 0;
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

int tf_performance_stepper_set_response_expected(TF_PerformanceStepper *performance_stepper, uint8_t function_id, bool response_expected) {
    switch(function_id) {
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

void tf_performance_stepper_set_response_expected_all(TF_PerformanceStepper *performance_stepper, bool response_expected) {
    memset(performance_stepper->response_expected, response_expected ? 0xFF : 0, 3);
}

int tf_performance_stepper_set_motion_configuration(TF_PerformanceStepper *performance_stepper, uint8_t ramping_mode, int32_t velocity_start, int32_t acceleration_1, int32_t velocity_1, int32_t acceleration_max, int32_t velocity_max, int32_t deceleration_max, int32_t deceleration_1, int32_t velocity_stop, int32_t ramp_zero_wait) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTION_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTION_CONFIGURATION, 37, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)ramping_mode;
    velocity_start = tf_leconvert_int32_to(velocity_start); memcpy(buf + 1, &velocity_start, 4);
    acceleration_1 = tf_leconvert_int32_to(acceleration_1); memcpy(buf + 5, &acceleration_1, 4);
    velocity_1 = tf_leconvert_int32_to(velocity_1); memcpy(buf + 9, &velocity_1, 4);
    acceleration_max = tf_leconvert_int32_to(acceleration_max); memcpy(buf + 13, &acceleration_max, 4);
    velocity_max = tf_leconvert_int32_to(velocity_max); memcpy(buf + 17, &velocity_max, 4);
    deceleration_max = tf_leconvert_int32_to(deceleration_max); memcpy(buf + 21, &deceleration_max, 4);
    deceleration_1 = tf_leconvert_int32_to(deceleration_1); memcpy(buf + 25, &deceleration_1, 4);
    velocity_stop = tf_leconvert_int32_to(velocity_stop); memcpy(buf + 29, &velocity_stop, 4);
    ramp_zero_wait = tf_leconvert_int32_to(ramp_zero_wait); memcpy(buf + 33, &ramp_zero_wait, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_motion_configuration(TF_PerformanceStepper *performance_stepper, uint8_t *ret_ramping_mode, int32_t *ret_velocity_start, int32_t *ret_acceleration_1, int32_t *ret_velocity_1, int32_t *ret_acceleration_max, int32_t *ret_velocity_max, int32_t *ret_deceleration_max, int32_t *ret_deceleration_1, int32_t *ret_velocity_stop, int32_t *ret_ramp_zero_wait) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_MOTION_CONFIGURATION, 0, 37, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_ramping_mode != NULL) { *ret_ramping_mode = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_velocity_start != NULL) { *ret_velocity_start = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        if (ret_acceleration_1 != NULL) { *ret_acceleration_1 = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        if (ret_velocity_1 != NULL) { *ret_velocity_1 = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        if (ret_acceleration_max != NULL) { *ret_acceleration_max = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        if (ret_velocity_max != NULL) { *ret_velocity_max = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        if (ret_deceleration_max != NULL) { *ret_deceleration_max = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        if (ret_deceleration_1 != NULL) { *ret_deceleration_1 = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        if (ret_velocity_stop != NULL) { *ret_velocity_stop = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        if (ret_ramp_zero_wait != NULL) { *ret_ramp_zero_wait = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_current_position(TF_PerformanceStepper *performance_stepper, int32_t position) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_CURRENT_POSITION, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_CURRENT_POSITION, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    position = tf_leconvert_int32_to(position); memcpy(buf + 0, &position, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_current_position(TF_PerformanceStepper *performance_stepper, int32_t *ret_position) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_CURRENT_POSITION, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_position != NULL) { *ret_position = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_current_velocity(TF_PerformanceStepper *performance_stepper, int32_t *ret_velocity) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_CURRENT_VELOCITY, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_velocity != NULL) { *ret_velocity = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_target_position(TF_PerformanceStepper *performance_stepper, int32_t position) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_TARGET_POSITION, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_TARGET_POSITION, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    position = tf_leconvert_int32_to(position); memcpy(buf + 0, &position, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_target_position(TF_PerformanceStepper *performance_stepper, int32_t *ret_position) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_TARGET_POSITION, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_position != NULL) { *ret_position = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_steps(TF_PerformanceStepper *performance_stepper, int32_t steps) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    steps = tf_leconvert_int32_to(steps); memcpy(buf + 0, &steps, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_steps(TF_PerformanceStepper *performance_stepper, int32_t *ret_steps) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_STEPS, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_steps != NULL) { *ret_steps = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_remaining_steps(TF_PerformanceStepper *performance_stepper, int32_t *ret_steps) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_REMAINING_STEPS, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_steps != NULL) { *ret_steps = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_step_configuration(TF_PerformanceStepper *performance_stepper, uint8_t step_resolution, bool interpolation) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEP_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEP_CONFIGURATION, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)step_resolution;
    buf[1] = interpolation ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_step_configuration(TF_PerformanceStepper *performance_stepper, uint8_t *ret_step_resolution, bool *ret_interpolation) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_STEP_CONFIGURATION, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_step_resolution != NULL) { *ret_step_resolution = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_interpolation != NULL) { *ret_interpolation = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_motor_current(TF_PerformanceStepper *performance_stepper, uint16_t current) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTOR_CURRENT, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_MOTOR_CURRENT, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    current = tf_leconvert_uint16_to(current); memcpy(buf + 0, &current, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_motor_current(TF_PerformanceStepper *performance_stepper, uint16_t *ret_current) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_MOTOR_CURRENT, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_current != NULL) { *ret_current = tf_packetbuffer_read_uint16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_enabled(TF_PerformanceStepper *performance_stepper, bool enabled) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLED, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLED, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = enabled ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_enabled(TF_PerformanceStepper *performance_stepper, bool *ret_enabled) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_ENABLED, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_enabled != NULL) { *ret_enabled = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_basic_configuration(TF_PerformanceStepper *performance_stepper, uint16_t standstill_current, uint16_t motor_run_current, uint16_t standstill_delay_time, uint16_t power_down_time, uint16_t stealth_threshold, uint16_t coolstep_threshold, uint16_t classic_threshold, bool high_velocity_chopper_mode) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_BASIC_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_BASIC_CONFIGURATION, 15, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    standstill_current = tf_leconvert_uint16_to(standstill_current); memcpy(buf + 0, &standstill_current, 2);
    motor_run_current = tf_leconvert_uint16_to(motor_run_current); memcpy(buf + 2, &motor_run_current, 2);
    standstill_delay_time = tf_leconvert_uint16_to(standstill_delay_time); memcpy(buf + 4, &standstill_delay_time, 2);
    power_down_time = tf_leconvert_uint16_to(power_down_time); memcpy(buf + 6, &power_down_time, 2);
    stealth_threshold = tf_leconvert_uint16_to(stealth_threshold); memcpy(buf + 8, &stealth_threshold, 2);
    coolstep_threshold = tf_leconvert_uint16_to(coolstep_threshold); memcpy(buf + 10, &coolstep_threshold, 2);
    classic_threshold = tf_leconvert_uint16_to(classic_threshold); memcpy(buf + 12, &classic_threshold, 2);
    buf[14] = high_velocity_chopper_mode ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_basic_configuration(TF_PerformanceStepper *performance_stepper, uint16_t *ret_standstill_current, uint16_t *ret_motor_run_current, uint16_t *ret_standstill_delay_time, uint16_t *ret_power_down_time, uint16_t *ret_stealth_threshold, uint16_t *ret_coolstep_threshold, uint16_t *ret_classic_threshold, bool *ret_high_velocity_chopper_mode) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_BASIC_CONFIGURATION, 0, 15, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_standstill_current != NULL) { *ret_standstill_current = tf_packetbuffer_read_uint16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        if (ret_motor_run_current != NULL) { *ret_motor_run_current = tf_packetbuffer_read_uint16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        if (ret_standstill_delay_time != NULL) { *ret_standstill_delay_time = tf_packetbuffer_read_uint16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        if (ret_power_down_time != NULL) { *ret_power_down_time = tf_packetbuffer_read_uint16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        if (ret_stealth_threshold != NULL) { *ret_stealth_threshold = tf_packetbuffer_read_uint16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        if (ret_coolstep_threshold != NULL) { *ret_coolstep_threshold = tf_packetbuffer_read_uint16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        if (ret_classic_threshold != NULL) { *ret_classic_threshold = tf_packetbuffer_read_uint16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        if (ret_high_velocity_chopper_mode != NULL) { *ret_high_velocity_chopper_mode = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_spreadcycle_configuration(TF_PerformanceStepper *performance_stepper, uint8_t slow_decay_duration, bool high_velocity_fullstep, uint8_t fast_decay_duration, uint8_t hysteresis_start_value, int8_t hysteresis_end_value, int8_t sine_wave_offset, uint8_t chopper_mode, uint8_t comparator_blank_time, bool fast_decay_without_comparator) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_SPREADCYCLE_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_SPREADCYCLE_CONFIGURATION, 9, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)slow_decay_duration;
    buf[1] = high_velocity_fullstep ? 1 : 0;
    buf[2] = (uint8_t)fast_decay_duration;
    buf[3] = (uint8_t)hysteresis_start_value;
    buf[4] = (uint8_t)hysteresis_end_value;
    buf[5] = (uint8_t)sine_wave_offset;
    buf[6] = (uint8_t)chopper_mode;
    buf[7] = (uint8_t)comparator_blank_time;
    buf[8] = fast_decay_without_comparator ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_spreadcycle_configuration(TF_PerformanceStepper *performance_stepper, uint8_t *ret_slow_decay_duration, bool *ret_high_velocity_fullstep, bool *ret_enable_random_slow_decay, uint8_t *ret_fast_decay_duration, uint8_t *ret_hysteresis_start_value, int8_t *ret_hysteresis_end_value, int8_t *ret_sine_wave_offset, uint8_t *ret_chopper_mode, uint8_t *ret_comparator_blank_time, bool *ret_fast_decay_without_comparator) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_SPREADCYCLE_CONFIGURATION, 0, 10, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_slow_decay_duration != NULL) { *ret_slow_decay_duration = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_high_velocity_fullstep != NULL) { *ret_high_velocity_fullstep = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_enable_random_slow_decay != NULL) { *ret_enable_random_slow_decay = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_fast_decay_duration != NULL) { *ret_fast_decay_duration = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_hysteresis_start_value != NULL) { *ret_hysteresis_start_value = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_hysteresis_end_value != NULL) { *ret_hysteresis_end_value = tf_packetbuffer_read_int8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_sine_wave_offset != NULL) { *ret_sine_wave_offset = tf_packetbuffer_read_int8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_chopper_mode != NULL) { *ret_chopper_mode = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_comparator_blank_time != NULL) { *ret_comparator_blank_time = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_fast_decay_without_comparator != NULL) { *ret_fast_decay_without_comparator = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_stealth_configuration(TF_PerformanceStepper *performance_stepper, bool enable_stealth, uint8_t offset, uint8_t gradient, bool enable_autoscale, bool enable_autogradient, uint8_t freewheel_mode, uint8_t regulation_loop_gradient, uint8_t amplitude_limit) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEALTH_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEALTH_CONFIGURATION, 8, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = enable_stealth ? 1 : 0;
    buf[1] = (uint8_t)offset;
    buf[2] = (uint8_t)gradient;
    buf[3] = enable_autoscale ? 1 : 0;
    buf[4] = enable_autogradient ? 1 : 0;
    buf[5] = (uint8_t)freewheel_mode;
    buf[6] = (uint8_t)regulation_loop_gradient;
    buf[7] = (uint8_t)amplitude_limit;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_stealth_configuration(TF_PerformanceStepper *performance_stepper, bool *ret_enable_stealth, uint8_t *ret_offset, uint8_t *ret_gradient, bool *ret_enable_autoscale, bool *ret_enable_autogradient, uint8_t *ret_freewheel_mode, uint8_t *ret_regulation_loop_gradient, uint8_t *ret_amplitude_limit) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_STEALTH_CONFIGURATION, 0, 8, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_enable_stealth != NULL) { *ret_enable_stealth = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_offset != NULL) { *ret_offset = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_gradient != NULL) { *ret_gradient = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_enable_autoscale != NULL) { *ret_enable_autoscale = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_enable_autogradient != NULL) { *ret_enable_autogradient = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_freewheel_mode != NULL) { *ret_freewheel_mode = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_regulation_loop_gradient != NULL) { *ret_regulation_loop_gradient = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_amplitude_limit != NULL) { *ret_amplitude_limit = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_coolstep_configuration(TF_PerformanceStepper *performance_stepper, uint8_t minimum_stallguard_value, uint8_t maximum_stallguard_value, uint8_t current_up_step_width, uint8_t current_down_step_width, uint8_t minimum_current, int8_t stallguard_threshold_value, uint8_t stallguard_mode) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_COOLSTEP_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_COOLSTEP_CONFIGURATION, 7, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)minimum_stallguard_value;
    buf[1] = (uint8_t)maximum_stallguard_value;
    buf[2] = (uint8_t)current_up_step_width;
    buf[3] = (uint8_t)current_down_step_width;
    buf[4] = (uint8_t)minimum_current;
    buf[5] = (uint8_t)stallguard_threshold_value;
    buf[6] = (uint8_t)stallguard_mode;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_coolstep_configuration(TF_PerformanceStepper *performance_stepper, uint8_t *ret_minimum_stallguard_value, uint8_t *ret_maximum_stallguard_value, uint8_t *ret_current_up_step_width, uint8_t *ret_current_down_step_width, uint8_t *ret_minimum_current, int8_t *ret_stallguard_threshold_value, uint8_t *ret_stallguard_mode) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_COOLSTEP_CONFIGURATION, 0, 7, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_minimum_stallguard_value != NULL) { *ret_minimum_stallguard_value = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_maximum_stallguard_value != NULL) { *ret_maximum_stallguard_value = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_current_up_step_width != NULL) { *ret_current_up_step_width = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_current_down_step_width != NULL) { *ret_current_down_step_width = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_minimum_current != NULL) { *ret_minimum_current = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_stallguard_threshold_value != NULL) { *ret_stallguard_threshold_value = tf_packetbuffer_read_int8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_stallguard_mode != NULL) { *ret_stallguard_mode = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_short_configuration(TF_PerformanceStepper *performance_stepper, bool disable_short_to_voltage_protection, bool disable_short_to_ground_protection, uint8_t short_to_voltage_level, uint8_t short_to_ground_level, uint8_t spike_filter_bandwidth, bool short_detection_delay, uint8_t filter_time) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_SHORT_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_SHORT_CONFIGURATION, 7, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = disable_short_to_voltage_protection ? 1 : 0;
    buf[1] = disable_short_to_ground_protection ? 1 : 0;
    buf[2] = (uint8_t)short_to_voltage_level;
    buf[3] = (uint8_t)short_to_ground_level;
    buf[4] = (uint8_t)spike_filter_bandwidth;
    buf[5] = short_detection_delay ? 1 : 0;
    buf[6] = (uint8_t)filter_time;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_short_configuration(TF_PerformanceStepper *performance_stepper, bool *ret_disable_short_to_voltage_protection, bool *ret_disable_short_to_ground_protection, uint8_t *ret_short_to_voltage_level, uint8_t *ret_short_to_ground_level, uint8_t *ret_spike_filter_bandwidth, bool *ret_short_detection_delay, uint8_t *ret_filter_time) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_SHORT_CONFIGURATION, 0, 7, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_disable_short_to_voltage_protection != NULL) { *ret_disable_short_to_voltage_protection = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_disable_short_to_ground_protection != NULL) { *ret_disable_short_to_ground_protection = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_short_to_voltage_level != NULL) { *ret_short_to_voltage_level = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_short_to_ground_level != NULL) { *ret_short_to_ground_level = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_spike_filter_bandwidth != NULL) { *ret_spike_filter_bandwidth = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_short_detection_delay != NULL) { *ret_short_detection_delay = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_filter_time != NULL) { *ret_filter_time = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_driver_status(TF_PerformanceStepper *performance_stepper, uint8_t *ret_open_load, uint8_t *ret_short_to_ground, uint8_t *ret_over_temperature, bool *ret_motor_stalled, uint8_t *ret_actual_motor_current, bool *ret_full_step_active, uint8_t *ret_stallguard_result, uint8_t *ret_stealth_voltage_amplitude) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_DRIVER_STATUS, 0, 8, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_open_load != NULL) { *ret_open_load = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_short_to_ground != NULL) { *ret_short_to_ground = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_over_temperature != NULL) { *ret_over_temperature = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_motor_stalled != NULL) { *ret_motor_stalled = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_actual_motor_current != NULL) { *ret_actual_motor_current = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_full_step_active != NULL) { *ret_full_step_active = tf_packetbuffer_read_bool(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_stallguard_result != NULL) { *ret_stallguard_result = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_stealth_voltage_amplitude != NULL) { *ret_stealth_voltage_amplitude = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_input_voltage(TF_PerformanceStepper *performance_stepper, uint16_t *ret_voltage) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_INPUT_VOLTAGE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_voltage != NULL) { *ret_voltage = tf_packetbuffer_read_uint16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_temperature(TF_PerformanceStepper *performance_stepper, int16_t *ret_temperature) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_temperature != NULL) { *ret_temperature = tf_packetbuffer_read_int16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_gpio_configuration(TF_PerformanceStepper *performance_stepper, uint16_t debounce, int32_t stop_deceleration) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_CONFIGURATION, 6, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    debounce = tf_leconvert_uint16_to(debounce); memcpy(buf + 0, &debounce, 2);
    stop_deceleration = tf_leconvert_int32_to(stop_deceleration); memcpy(buf + 2, &stop_deceleration, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_gpio_configuration(TF_PerformanceStepper *performance_stepper, uint16_t *ret_debounce, int32_t *ret_stop_deceleration) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_GPIO_CONFIGURATION, 0, 6, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_debounce != NULL) { *ret_debounce = tf_packetbuffer_read_uint16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        if (ret_stop_deceleration != NULL) { *ret_stop_deceleration = tf_packetbuffer_read_int32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_gpio_action(TF_PerformanceStepper *performance_stepper, uint8_t channel, uint32_t action) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_ACTION, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_ACTION, 5, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)channel;
    action = tf_leconvert_uint32_to(action); memcpy(buf + 1, &action, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_gpio_action(TF_PerformanceStepper *performance_stepper, uint8_t channel, uint32_t *ret_action) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_GPIO_ACTION, 1, 4, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)channel;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_action != NULL) { *ret_action = tf_packetbuffer_read_uint32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_gpio_state(TF_PerformanceStepper *performance_stepper, bool ret_gpio_state[2]) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_GPIO_STATE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_gpio_state != NULL) { tf_packetbuffer_read_bool_array(&performance_stepper->tfp->spitfp->recv_buf, ret_gpio_state, 2);} else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_error_led_config(TF_PerformanceStepper *performance_stepper, uint8_t config) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_ERROR_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_ERROR_LED_CONFIG, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_error_led_config(TF_PerformanceStepper *performance_stepper, uint8_t *ret_config) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_ERROR_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_config != NULL) { *ret_config = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_enable_led_config(TF_PerformanceStepper *performance_stepper, uint8_t config) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLE_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_ENABLE_LED_CONFIG, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_enable_led_config(TF_PerformanceStepper *performance_stepper, uint8_t *ret_config) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_ENABLE_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_config != NULL) { *ret_config = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_steps_led_config(TF_PerformanceStepper *performance_stepper, uint8_t config) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STEPS_LED_CONFIG, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_steps_led_config(TF_PerformanceStepper *performance_stepper, uint8_t *ret_config) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_STEPS_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_config != NULL) { *ret_config = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_gpio_led_config(TF_PerformanceStepper *performance_stepper, uint8_t channel, uint8_t config) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_GPIO_LED_CONFIG, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)channel;
    buf[1] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_gpio_led_config(TF_PerformanceStepper *performance_stepper, uint8_t channel, uint8_t *ret_config) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_GPIO_LED_CONFIG, 1, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)channel;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_config != NULL) { *ret_config = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_write_register(TF_PerformanceStepper *performance_stepper, uint8_t register_, uint32_t value, uint8_t *ret_status) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_REGISTER, 5, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)register_;
    value = tf_leconvert_uint32_to(value); memcpy(buf + 1, &value, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_read_register(TF_PerformanceStepper *performance_stepper, uint8_t register_, uint8_t *ret_status, uint32_t *ret_value) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_READ_REGISTER, 1, 5, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)register_;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_value != NULL) { *ret_value = tf_packetbuffer_read_uint32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_spitfp_error_count(TF_PerformanceStepper *performance_stepper, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, 16, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packetbuffer_read_uint32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packetbuffer_read_uint32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packetbuffer_read_uint32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packetbuffer_read_uint32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_bootloader_mode(TF_PerformanceStepper *performance_stepper, uint8_t mode, uint8_t *ret_status) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_BOOTLOADER_MODE, 1, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)mode;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_bootloader_mode(TF_PerformanceStepper *performance_stepper, uint8_t *ret_mode) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_BOOTLOADER_MODE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_mode != NULL) { *ret_mode = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_write_firmware_pointer(TF_PerformanceStepper *performance_stepper, uint32_t pointer) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(buf + 0, &pointer, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_write_firmware(TF_PerformanceStepper *performance_stepper, const uint8_t data[64], uint8_t *ret_status) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_FIRMWARE, 64, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    memcpy(buf + 0, data, 64);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_set_status_led_config(TF_PerformanceStepper *performance_stepper, uint8_t config) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STATUS_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_SET_STATUS_LED_CONFIG, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_status_led_config(TF_PerformanceStepper *performance_stepper, uint8_t *ret_config) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_STATUS_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_config != NULL) { *ret_config = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_chip_temperature(TF_PerformanceStepper *performance_stepper, int16_t *ret_temperature) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_CHIP_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_temperature != NULL) { *ret_temperature = tf_packetbuffer_read_int16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_reset(TF_PerformanceStepper *performance_stepper) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_RESET, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_RESET, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_write_uid(TF_PerformanceStepper *performance_stepper, uint32_t uid) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_performance_stepper_get_response_expected(performance_stepper, TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_UID, &response_expected);
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_WRITE_UID, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(performance_stepper->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(buf + 0, &uid, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_read_uid(TF_PerformanceStepper *performance_stepper, uint32_t *ret_uid) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_READ_UID, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_uid != NULL) { *ret_uid = tf_packetbuffer_read_uint32_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_performance_stepper_get_identity(TF_PerformanceStepper *performance_stepper, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(performance_stepper->tfp, TF_PERFORMANCE_STEPPER_FUNCTION_GET_IDENTITY, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + tf_hal_get_common((TF_HalContext*)performance_stepper->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(performance_stepper->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        char tmp_connected_uid[8] = {0};
        if (ret_uid != NULL) { tf_packetbuffer_pop_n(&performance_stepper->tfp->spitfp->recv_buf, (uint8_t*)ret_uid, 8);} else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 8); }
        tf_packetbuffer_pop_n(&performance_stepper->tfp->spitfp->recv_buf, (uint8_t*)tmp_connected_uid, 8);
        if (ret_position != NULL) { *ret_position = tf_packetbuffer_read_char(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 1); }
        if (ret_hardware_version != NULL) { for (i = 0; i < 3; ++i) ret_hardware_version[i] = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 3); }
        if (ret_firmware_version != NULL) { for (i = 0; i < 3; ++i) ret_firmware_version[i] = tf_packetbuffer_read_uint8_t(&performance_stepper->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 3); }
        if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packetbuffer_read_uint16_t(&performance_stepper->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&performance_stepper->tfp->spitfp->recv_buf, 2); }
        if (tmp_connected_uid[0] == 0 && ret_position != NULL) {
            *ret_position = tf_hal_get_port_name((TF_HalContext*)performance_stepper->tfp->hal, performance_stepper->tfp->spitfp->port_id);
        }
        if (ret_connected_uid != NULL) {
            memcpy(ret_connected_uid, tmp_connected_uid, 8);
        }
        tf_tfp_packet_processed(performance_stepper->tfp);
    }

    result = tf_tfp_finish_send(performance_stepper->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}


int tf_performance_stepper_callback_tick(TF_PerformanceStepper *performance_stepper, uint32_t timeout_us) {
    if (performance_stepper == NULL)
        return TF_E_NULL;

    return tf_tfp_callback_tick(performance_stepper->tfp, tf_hal_current_time_us((TF_HalContext*)performance_stepper->tfp->hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
