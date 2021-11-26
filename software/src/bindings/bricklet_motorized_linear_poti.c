/* ***********************************************************
 * This file was automatically generated on 2021-11-26.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.0         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_motorized_linear_poti.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_motorized_linear_poti_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_MotorizedLinearPoti *motorized_linear_poti = (TF_MotorizedLinearPoti *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(motorized_linear_poti->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_MOTORIZED_LINEAR_POTI_CALLBACK_POSITION: {
            TF_MotorizedLinearPoti_PositionHandler fn = motorized_linear_poti->position_handler;
            void *user_data = motorized_linear_poti->position_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t position = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(motorized_linear_poti, position, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_MOTORIZED_LINEAR_POTI_CALLBACK_POSITION_REACHED: {
            TF_MotorizedLinearPoti_PositionReachedHandler fn = motorized_linear_poti->position_reached_handler;
            void *user_data = motorized_linear_poti->position_reached_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t position = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(motorized_linear_poti, position, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_motorized_linear_poti_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_motorized_linear_poti_create(TF_MotorizedLinearPoti *motorized_linear_poti, const char *uid, TF_HAL *hal) {
    if (motorized_linear_poti == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    static uint16_t next_tfp_index = 0;

    memset(motorized_linear_poti, 0, sizeof(TF_MotorizedLinearPoti));

    TF_TFP *tfp;

    if (uid != NULL && *uid != '\0') {
        uint32_t uid_num = 0;
        int rc = tf_base58_decode(uid, &uid_num);

        if (rc != TF_E_OK) {
            return rc;
        }

        tfp = tf_hal_get_tfp(hal, &next_tfp_index, &uid_num, NULL, NULL);

        if (tfp == NULL) {
            return TF_E_DEVICE_NOT_FOUND;
        }

        if (tfp->device_id != TF_MOTORIZED_LINEAR_POTI_DEVICE_IDENTIFIER) {
            return TF_E_WRONG_DEVICE_TYPE;
        }
    } else {
        uint16_t device_id = TF_MOTORIZED_LINEAR_POTI_DEVICE_IDENTIFIER;

        tfp = tf_hal_get_tfp(hal, &next_tfp_index, NULL, NULL, &device_id);

        if (tfp == NULL) {
            return TF_E_DEVICE_NOT_FOUND;
        }
    }

    if (tfp->device != NULL) {
        return TF_E_DEVICE_ALREADY_IN_USE;
    }

    motorized_linear_poti->tfp = tfp;
    motorized_linear_poti->tfp->device = motorized_linear_poti;
    motorized_linear_poti->tfp->cb_handler = tf_motorized_linear_poti_callback_handler;
    motorized_linear_poti->response_expected[0] = 0x09;

    return TF_E_OK;
}

int tf_motorized_linear_poti_destroy(TF_MotorizedLinearPoti *motorized_linear_poti) {
    if (motorized_linear_poti == NULL || motorized_linear_poti->tfp == NULL) {
        return TF_E_NULL;
    }

    motorized_linear_poti->tfp->cb_handler = NULL;
    motorized_linear_poti->tfp->device = NULL;
    motorized_linear_poti->tfp = NULL;

    return TF_E_OK;
}

int tf_motorized_linear_poti_get_response_expected(TF_MotorizedLinearPoti *motorized_linear_poti, uint8_t function_id, bool *ret_response_expected) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    switch (function_id) {
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_POSITION_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motorized_linear_poti->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_MOTOR_POSITION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motorized_linear_poti->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_CALIBRATE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motorized_linear_poti->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_POSITION_REACHED_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motorized_linear_poti->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motorized_linear_poti->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motorized_linear_poti->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motorized_linear_poti->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motorized_linear_poti->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_motorized_linear_poti_set_response_expected(TF_MotorizedLinearPoti *motorized_linear_poti, uint8_t function_id, bool response_expected) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    switch (function_id) {
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_POSITION_CALLBACK_CONFIGURATION:
            if (response_expected) {
                motorized_linear_poti->response_expected[0] |= (1 << 0);
            } else {
                motorized_linear_poti->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_MOTOR_POSITION:
            if (response_expected) {
                motorized_linear_poti->response_expected[0] |= (1 << 1);
            } else {
                motorized_linear_poti->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_CALIBRATE:
            if (response_expected) {
                motorized_linear_poti->response_expected[0] |= (1 << 2);
            } else {
                motorized_linear_poti->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_POSITION_REACHED_CALLBACK_CONFIGURATION:
            if (response_expected) {
                motorized_linear_poti->response_expected[0] |= (1 << 3);
            } else {
                motorized_linear_poti->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                motorized_linear_poti->response_expected[0] |= (1 << 4);
            } else {
                motorized_linear_poti->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                motorized_linear_poti->response_expected[0] |= (1 << 5);
            } else {
                motorized_linear_poti->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_RESET:
            if (response_expected) {
                motorized_linear_poti->response_expected[0] |= (1 << 6);
            } else {
                motorized_linear_poti->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_MOTORIZED_LINEAR_POTI_FUNCTION_WRITE_UID:
            if (response_expected) {
                motorized_linear_poti->response_expected[0] |= (1 << 7);
            } else {
                motorized_linear_poti->response_expected[0] &= ~(1 << 7);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_motorized_linear_poti_set_response_expected_all(TF_MotorizedLinearPoti *motorized_linear_poti, bool response_expected) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    memset(motorized_linear_poti->response_expected, response_expected ? 0xFF : 0, 1);

    return TF_E_OK;
}

int tf_motorized_linear_poti_get_position(TF_MotorizedLinearPoti *motorized_linear_poti, uint16_t *ret_position) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_GET_POSITION, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(motorized_linear_poti->tfp);
        if (ret_position != NULL) { *ret_position = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(motorized_linear_poti->tfp);
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_set_position_callback_configuration(TF_MotorizedLinearPoti *motorized_linear_poti, uint32_t period, bool value_has_to_change, char option, uint16_t min, uint16_t max) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_motorized_linear_poti_get_response_expected(motorized_linear_poti, TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_POSITION_CALLBACK_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_POSITION_CALLBACK_CONFIGURATION, 10, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(motorized_linear_poti->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(send_buf + 0, &period, 4);
    send_buf[4] = value_has_to_change ? 1 : 0;
    send_buf[5] = (uint8_t)option;
    min = tf_leconvert_uint16_to(min); memcpy(send_buf + 6, &min, 2);
    max = tf_leconvert_uint16_to(max); memcpy(send_buf + 8, &max, 2);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_get_position_callback_configuration(TF_MotorizedLinearPoti *motorized_linear_poti, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, uint16_t *ret_min, uint16_t *ret_max) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_GET_POSITION_CALLBACK_CONFIGURATION, 0, 10, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(motorized_linear_poti->tfp);
        if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_option != NULL) { *ret_option = tf_packet_buffer_read_char(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_min != NULL) { *ret_min = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        if (ret_max != NULL) { *ret_max = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(motorized_linear_poti->tfp);
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_set_motor_position(TF_MotorizedLinearPoti *motorized_linear_poti, uint16_t position, uint8_t drive_mode, bool hold_position) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_motorized_linear_poti_get_response_expected(motorized_linear_poti, TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_MOTOR_POSITION, &response_expected);
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_MOTOR_POSITION, 4, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(motorized_linear_poti->tfp);

    position = tf_leconvert_uint16_to(position); memcpy(send_buf + 0, &position, 2);
    send_buf[2] = (uint8_t)drive_mode;
    send_buf[3] = hold_position ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_get_motor_position(TF_MotorizedLinearPoti *motorized_linear_poti, uint16_t *ret_position, uint8_t *ret_drive_mode, bool *ret_hold_position, bool *ret_position_reached) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_GET_MOTOR_POSITION, 0, 5, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(motorized_linear_poti->tfp);
        if (ret_position != NULL) { *ret_position = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        if (ret_drive_mode != NULL) { *ret_drive_mode = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_hold_position != NULL) { *ret_hold_position = tf_packet_buffer_read_bool(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_position_reached != NULL) { *ret_position_reached = tf_packet_buffer_read_bool(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(motorized_linear_poti->tfp);
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_calibrate(TF_MotorizedLinearPoti *motorized_linear_poti) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_motorized_linear_poti_get_response_expected(motorized_linear_poti, TF_MOTORIZED_LINEAR_POTI_FUNCTION_CALIBRATE, &response_expected);
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_CALIBRATE, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_set_position_reached_callback_configuration(TF_MotorizedLinearPoti *motorized_linear_poti, bool enabled) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_motorized_linear_poti_get_response_expected(motorized_linear_poti, TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_POSITION_REACHED_CALLBACK_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_POSITION_REACHED_CALLBACK_CONFIGURATION, 1, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(motorized_linear_poti->tfp);

    send_buf[0] = enabled ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_get_position_reached_callback_configuration(TF_MotorizedLinearPoti *motorized_linear_poti, bool *ret_enabled) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_GET_POSITION_REACHED_CALLBACK_CONFIGURATION, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(motorized_linear_poti->tfp);
        if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(motorized_linear_poti->tfp);
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_get_spitfp_error_count(TF_MotorizedLinearPoti *motorized_linear_poti, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, 16, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(motorized_linear_poti->tfp);
        if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        tf_tfp_packet_processed(motorized_linear_poti->tfp);
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_set_bootloader_mode(TF_MotorizedLinearPoti *motorized_linear_poti, uint8_t mode, uint8_t *ret_status) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_BOOTLOADER_MODE, 1, 1, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(motorized_linear_poti->tfp);

    send_buf[0] = (uint8_t)mode;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(motorized_linear_poti->tfp);
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(motorized_linear_poti->tfp);
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_get_bootloader_mode(TF_MotorizedLinearPoti *motorized_linear_poti, uint8_t *ret_mode) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_GET_BOOTLOADER_MODE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(motorized_linear_poti->tfp);
        if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(motorized_linear_poti->tfp);
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_set_write_firmware_pointer(TF_MotorizedLinearPoti *motorized_linear_poti, uint32_t pointer) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_motorized_linear_poti_get_response_expected(motorized_linear_poti, TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &response_expected);
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(motorized_linear_poti->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(send_buf + 0, &pointer, 4);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_write_firmware(TF_MotorizedLinearPoti *motorized_linear_poti, const uint8_t data[64], uint8_t *ret_status) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_WRITE_FIRMWARE, 64, 1, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(motorized_linear_poti->tfp);

    memcpy(send_buf + 0, data, 64);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(motorized_linear_poti->tfp);
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(motorized_linear_poti->tfp);
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_set_status_led_config(TF_MotorizedLinearPoti *motorized_linear_poti, uint8_t config) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_motorized_linear_poti_get_response_expected(motorized_linear_poti, TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_STATUS_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_SET_STATUS_LED_CONFIG, 1, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(motorized_linear_poti->tfp);

    send_buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_get_status_led_config(TF_MotorizedLinearPoti *motorized_linear_poti, uint8_t *ret_config) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_GET_STATUS_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(motorized_linear_poti->tfp);
        if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(motorized_linear_poti->tfp);
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_get_chip_temperature(TF_MotorizedLinearPoti *motorized_linear_poti, int16_t *ret_temperature) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_GET_CHIP_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(motorized_linear_poti->tfp);
        if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(motorized_linear_poti->tfp);
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_reset(TF_MotorizedLinearPoti *motorized_linear_poti) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_motorized_linear_poti_get_response_expected(motorized_linear_poti, TF_MOTORIZED_LINEAR_POTI_FUNCTION_RESET, &response_expected);
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_RESET, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_write_uid(TF_MotorizedLinearPoti *motorized_linear_poti, uint32_t uid) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_motorized_linear_poti_get_response_expected(motorized_linear_poti, TF_MOTORIZED_LINEAR_POTI_FUNCTION_WRITE_UID, &response_expected);
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_WRITE_UID, 4, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(motorized_linear_poti->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(send_buf + 0, &uid, 4);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_read_uid(TF_MotorizedLinearPoti *motorized_linear_poti, uint32_t *ret_uid) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_READ_UID, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(motorized_linear_poti->tfp);
        if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        tf_tfp_packet_processed(motorized_linear_poti->tfp);
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_motorized_linear_poti_get_identity(TF_MotorizedLinearPoti *motorized_linear_poti, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(motorized_linear_poti->tfp, TF_MOTORIZED_LINEAR_POTI_FUNCTION_GET_IDENTITY, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(motorized_linear_poti->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(motorized_linear_poti->tfp);
        if (ret_uid != NULL) { tf_packet_buffer_pop_n(recv_buf, (uint8_t *)ret_uid, 8);} else { tf_packet_buffer_remove(recv_buf, 8); }
        if (ret_connected_uid != NULL) { tf_packet_buffer_pop_n(recv_buf, (uint8_t *)ret_connected_uid, 8);} else { tf_packet_buffer_remove(recv_buf, 8); }
        if (ret_position != NULL) { *ret_position = tf_packet_buffer_read_char(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_hardware_version != NULL) { for (i = 0; i < 3; ++i) ret_hardware_version[i] = tf_packet_buffer_read_uint8_t(recv_buf);} else { tf_packet_buffer_remove(recv_buf, 3); }
        if (ret_firmware_version != NULL) { for (i = 0; i < 3; ++i) ret_firmware_version[i] = tf_packet_buffer_read_uint8_t(recv_buf);} else { tf_packet_buffer_remove(recv_buf, 3); }
        if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(motorized_linear_poti->tfp);
    }

    result = tf_tfp_finish_send(motorized_linear_poti->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_motorized_linear_poti_register_position_callback(TF_MotorizedLinearPoti *motorized_linear_poti, TF_MotorizedLinearPoti_PositionHandler handler, void *user_data) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    if (handler == NULL) {
        motorized_linear_poti->tfp->needs_callback_tick = false;
        motorized_linear_poti->tfp->needs_callback_tick |= motorized_linear_poti->position_reached_handler != NULL;
    } else {
        motorized_linear_poti->tfp->needs_callback_tick = true;
    }

    motorized_linear_poti->position_handler = handler;
    motorized_linear_poti->position_user_data = user_data;

    return TF_E_OK;
}


int tf_motorized_linear_poti_register_position_reached_callback(TF_MotorizedLinearPoti *motorized_linear_poti, TF_MotorizedLinearPoti_PositionReachedHandler handler, void *user_data) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    if (handler == NULL) {
        motorized_linear_poti->tfp->needs_callback_tick = false;
        motorized_linear_poti->tfp->needs_callback_tick |= motorized_linear_poti->position_handler != NULL;
    } else {
        motorized_linear_poti->tfp->needs_callback_tick = true;
    }

    motorized_linear_poti->position_reached_handler = handler;
    motorized_linear_poti->position_reached_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_motorized_linear_poti_callback_tick(TF_MotorizedLinearPoti *motorized_linear_poti, uint32_t timeout_us) {
    if (motorized_linear_poti == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = motorized_linear_poti->tfp->spitfp->hal;

    return tf_tfp_callback_tick(motorized_linear_poti->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
