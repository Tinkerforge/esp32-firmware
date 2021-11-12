/* ***********************************************************
 * This file was automatically generated on 2021-11-12.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.0         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_silent_stepper_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_silent_stepper_v2_callback_handler(void *dev, uint8_t fid, TF_Packetbuffer *payload) {
    TF_SilentStepperV2 *silent_stepper_v2 = (TF_SilentStepperV2 *) dev;
    (void)payload;

    switch(fid) {

        case TF_SILENT_STEPPER_V2_CALLBACK_UNDER_VOLTAGE: {
            TF_SilentStepperV2UnderVoltageHandler fn = silent_stepper_v2->under_voltage_handler;
            void *user_data = silent_stepper_v2->under_voltage_user_data;
            if (fn == NULL)
                return false;

            uint16_t voltage = tf_packetbuffer_read_uint16_t(payload);
            TF_HalCommon *common = tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal);
            common->locked = true;
            fn(silent_stepper_v2, voltage, user_data);
            common->locked = false;
            break;
        }

        case TF_SILENT_STEPPER_V2_CALLBACK_POSITION_REACHED: {
            TF_SilentStepperV2PositionReachedHandler fn = silent_stepper_v2->position_reached_handler;
            void *user_data = silent_stepper_v2->position_reached_user_data;
            if (fn == NULL)
                return false;

            int32_t position = tf_packetbuffer_read_int32_t(payload);
            TF_HalCommon *common = tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal);
            common->locked = true;
            fn(silent_stepper_v2, position, user_data);
            common->locked = false;
            break;
        }

        case TF_SILENT_STEPPER_V2_CALLBACK_ALL_DATA: {
            TF_SilentStepperV2AllDataHandler fn = silent_stepper_v2->all_data_handler;
            void *user_data = silent_stepper_v2->all_data_user_data;
            if (fn == NULL)
                return false;

            uint16_t current_velocity = tf_packetbuffer_read_uint16_t(payload);
            int32_t current_position = tf_packetbuffer_read_int32_t(payload);
            int32_t remaining_steps = tf_packetbuffer_read_int32_t(payload);
            uint16_t input_voltage = tf_packetbuffer_read_uint16_t(payload);
            uint16_t current_consumption = tf_packetbuffer_read_uint16_t(payload);
            TF_HalCommon *common = tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal);
            common->locked = true;
            fn(silent_stepper_v2, current_velocity, current_position, remaining_steps, input_voltage, current_consumption, user_data);
            common->locked = false;
            break;
        }

        case TF_SILENT_STEPPER_V2_CALLBACK_NEW_STATE: {
            TF_SilentStepperV2NewStateHandler fn = silent_stepper_v2->new_state_handler;
            void *user_data = silent_stepper_v2->new_state_user_data;
            if (fn == NULL)
                return false;

            uint8_t state_new = tf_packetbuffer_read_uint8_t(payload);
            uint8_t state_previous = tf_packetbuffer_read_uint8_t(payload);
            TF_HalCommon *common = tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal);
            common->locked = true;
            fn(silent_stepper_v2, state_new, state_previous, user_data);
            common->locked = false;
            break;
        }

        case TF_SILENT_STEPPER_V2_CALLBACK_GPIO_STATE: {
            TF_SilentStepperV2GPIOStateHandler fn = silent_stepper_v2->gpio_state_handler;
            void *user_data = silent_stepper_v2->gpio_state_user_data;
            if (fn == NULL)
                return false;

            bool gpio_state[2]; tf_packetbuffer_read_bool_array(payload, gpio_state, 2);
            TF_HalCommon *common = tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal);
            common->locked = true;
            fn(silent_stepper_v2, gpio_state, user_data);
            common->locked = false;
            break;
        }
        default:
            return false;
    }

    return true;
}
#else
static bool tf_silent_stepper_v2_callback_handler(void *dev, uint8_t fid, TF_Packetbuffer *payload) {
    return false;
}
#endif
int tf_silent_stepper_v2_create(TF_SilentStepperV2 *silent_stepper_v2, const char *uid, TF_HalContext *hal) {
    if (silent_stepper_v2 == NULL || uid == NULL || hal == NULL)
        return TF_E_NULL;

    memset(silent_stepper_v2, 0, sizeof(TF_SilentStepperV2));

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

    rc = tf_hal_get_tfp(hal, &silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_DEVICE_IDENTIFIER, inventory_index);
    if (rc != TF_E_OK) {
        return rc;
    }
    silent_stepper_v2->tfp->device = silent_stepper_v2;
    silent_stepper_v2->tfp->uid = numeric_uid;
    silent_stepper_v2->tfp->cb_handler = tf_silent_stepper_v2_callback_handler;
    silent_stepper_v2->response_expected[0] = 0x00;
    silent_stepper_v2->response_expected[1] = 0x00;
    silent_stepper_v2->response_expected[2] = 0x14;
    silent_stepper_v2->response_expected[3] = 0x00;
    return TF_E_OK;
}

int tf_silent_stepper_v2_destroy(TF_SilentStepperV2 *silent_stepper_v2) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    int result = tf_tfp_destroy(silent_stepper_v2->tfp);
    silent_stepper_v2->tfp = NULL;
    return result;
}

int tf_silent_stepper_v2_get_response_expected(TF_SilentStepperV2 *silent_stepper_v2, uint8_t function_id, bool *ret_response_expected) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    switch(function_id) {
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_MAX_VELOCITY:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 0)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_SPEED_RAMPING:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 1)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_FULL_BRAKE:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 2)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_CURRENT_POSITION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 3)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_TARGET_POSITION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 4)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_STEPS:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 5)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_STEP_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 6)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_FORWARD:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 7)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_BACKWARD:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 0)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_STOP:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 1)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_MOTOR_CURRENT:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 2)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_ENABLED:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 3)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_BASIC_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 4)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_SPREADCYCLE_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 5)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_STEALTH_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 6)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_COOLSTEP_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 7)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_MISC_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 0)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_ERROR_LED_CONFIG:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 1)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_MINIMUM_VOLTAGE:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 2)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_TIME_BASE:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 3)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_ALL_CALLBACK_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 4)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 5)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_ACTION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 6)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 7)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[3] & (1 << 0)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_RESET:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[3] & (1 << 1)) != 0;
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_WRITE_UID:
            if(ret_response_expected != NULL)
                *ret_response_expected = (silent_stepper_v2->response_expected[3] & (1 << 2)) != 0;
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

int tf_silent_stepper_v2_set_response_expected(TF_SilentStepperV2 *silent_stepper_v2, uint8_t function_id, bool response_expected) {
    switch(function_id) {
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_MAX_VELOCITY:
            if (response_expected) {
                silent_stepper_v2->response_expected[0] |= (1 << 0);
            } else {
                silent_stepper_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_SPEED_RAMPING:
            if (response_expected) {
                silent_stepper_v2->response_expected[0] |= (1 << 1);
            } else {
                silent_stepper_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_FULL_BRAKE:
            if (response_expected) {
                silent_stepper_v2->response_expected[0] |= (1 << 2);
            } else {
                silent_stepper_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_CURRENT_POSITION:
            if (response_expected) {
                silent_stepper_v2->response_expected[0] |= (1 << 3);
            } else {
                silent_stepper_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_TARGET_POSITION:
            if (response_expected) {
                silent_stepper_v2->response_expected[0] |= (1 << 4);
            } else {
                silent_stepper_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_STEPS:
            if (response_expected) {
                silent_stepper_v2->response_expected[0] |= (1 << 5);
            } else {
                silent_stepper_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_STEP_CONFIGURATION:
            if (response_expected) {
                silent_stepper_v2->response_expected[0] |= (1 << 6);
            } else {
                silent_stepper_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_FORWARD:
            if (response_expected) {
                silent_stepper_v2->response_expected[0] |= (1 << 7);
            } else {
                silent_stepper_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_BACKWARD:
            if (response_expected) {
                silent_stepper_v2->response_expected[1] |= (1 << 0);
            } else {
                silent_stepper_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_STOP:
            if (response_expected) {
                silent_stepper_v2->response_expected[1] |= (1 << 1);
            } else {
                silent_stepper_v2->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_MOTOR_CURRENT:
            if (response_expected) {
                silent_stepper_v2->response_expected[1] |= (1 << 2);
            } else {
                silent_stepper_v2->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_ENABLED:
            if (response_expected) {
                silent_stepper_v2->response_expected[1] |= (1 << 3);
            } else {
                silent_stepper_v2->response_expected[1] &= ~(1 << 3);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_BASIC_CONFIGURATION:
            if (response_expected) {
                silent_stepper_v2->response_expected[1] |= (1 << 4);
            } else {
                silent_stepper_v2->response_expected[1] &= ~(1 << 4);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_SPREADCYCLE_CONFIGURATION:
            if (response_expected) {
                silent_stepper_v2->response_expected[1] |= (1 << 5);
            } else {
                silent_stepper_v2->response_expected[1] &= ~(1 << 5);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_STEALTH_CONFIGURATION:
            if (response_expected) {
                silent_stepper_v2->response_expected[1] |= (1 << 6);
            } else {
                silent_stepper_v2->response_expected[1] &= ~(1 << 6);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_COOLSTEP_CONFIGURATION:
            if (response_expected) {
                silent_stepper_v2->response_expected[1] |= (1 << 7);
            } else {
                silent_stepper_v2->response_expected[1] &= ~(1 << 7);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_MISC_CONFIGURATION:
            if (response_expected) {
                silent_stepper_v2->response_expected[2] |= (1 << 0);
            } else {
                silent_stepper_v2->response_expected[2] &= ~(1 << 0);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_ERROR_LED_CONFIG:
            if (response_expected) {
                silent_stepper_v2->response_expected[2] |= (1 << 1);
            } else {
                silent_stepper_v2->response_expected[2] &= ~(1 << 1);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_MINIMUM_VOLTAGE:
            if (response_expected) {
                silent_stepper_v2->response_expected[2] |= (1 << 2);
            } else {
                silent_stepper_v2->response_expected[2] &= ~(1 << 2);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_TIME_BASE:
            if (response_expected) {
                silent_stepper_v2->response_expected[2] |= (1 << 3);
            } else {
                silent_stepper_v2->response_expected[2] &= ~(1 << 3);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_ALL_CALLBACK_CONFIGURATION:
            if (response_expected) {
                silent_stepper_v2->response_expected[2] |= (1 << 4);
            } else {
                silent_stepper_v2->response_expected[2] &= ~(1 << 4);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_CONFIGURATION:
            if (response_expected) {
                silent_stepper_v2->response_expected[2] |= (1 << 5);
            } else {
                silent_stepper_v2->response_expected[2] &= ~(1 << 5);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_ACTION:
            if (response_expected) {
                silent_stepper_v2->response_expected[2] |= (1 << 6);
            } else {
                silent_stepper_v2->response_expected[2] &= ~(1 << 6);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                silent_stepper_v2->response_expected[2] |= (1 << 7);
            } else {
                silent_stepper_v2->response_expected[2] &= ~(1 << 7);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                silent_stepper_v2->response_expected[3] |= (1 << 0);
            } else {
                silent_stepper_v2->response_expected[3] &= ~(1 << 0);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_RESET:
            if (response_expected) {
                silent_stepper_v2->response_expected[3] |= (1 << 1);
            } else {
                silent_stepper_v2->response_expected[3] &= ~(1 << 1);
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                silent_stepper_v2->response_expected[3] |= (1 << 2);
            } else {
                silent_stepper_v2->response_expected[3] &= ~(1 << 2);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

void tf_silent_stepper_v2_set_response_expected_all(TF_SilentStepperV2 *silent_stepper_v2, bool response_expected) {
    memset(silent_stepper_v2->response_expected, response_expected ? 0xFF : 0, 4);
}

int tf_silent_stepper_v2_set_max_velocity(TF_SilentStepperV2 *silent_stepper_v2, uint16_t velocity) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_MAX_VELOCITY, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_MAX_VELOCITY, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    velocity = tf_leconvert_uint16_to(velocity); memcpy(buf + 0, &velocity, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_max_velocity(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_velocity) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_MAX_VELOCITY, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_velocity != NULL) { *ret_velocity = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_current_velocity(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_velocity) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_CURRENT_VELOCITY, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_velocity != NULL) { *ret_velocity = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_speed_ramping(TF_SilentStepperV2 *silent_stepper_v2, uint16_t acceleration, uint16_t deacceleration) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_SPEED_RAMPING, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_SPEED_RAMPING, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    acceleration = tf_leconvert_uint16_to(acceleration); memcpy(buf + 0, &acceleration, 2);
    deacceleration = tf_leconvert_uint16_to(deacceleration); memcpy(buf + 2, &deacceleration, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_speed_ramping(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_acceleration, uint16_t *ret_deacceleration) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_SPEED_RAMPING, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_acceleration != NULL) { *ret_acceleration = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_deacceleration != NULL) { *ret_deacceleration = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_full_brake(TF_SilentStepperV2 *silent_stepper_v2) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_FULL_BRAKE, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_FULL_BRAKE, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_current_position(TF_SilentStepperV2 *silent_stepper_v2, int32_t position) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_CURRENT_POSITION, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_CURRENT_POSITION, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    position = tf_leconvert_int32_to(position); memcpy(buf + 0, &position, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_current_position(TF_SilentStepperV2 *silent_stepper_v2, int32_t *ret_position) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_CURRENT_POSITION, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_position != NULL) { *ret_position = tf_packetbuffer_read_int32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_target_position(TF_SilentStepperV2 *silent_stepper_v2, int32_t position) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_TARGET_POSITION, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_TARGET_POSITION, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    position = tf_leconvert_int32_to(position); memcpy(buf + 0, &position, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_target_position(TF_SilentStepperV2 *silent_stepper_v2, int32_t *ret_position) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_TARGET_POSITION, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_position != NULL) { *ret_position = tf_packetbuffer_read_int32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_steps(TF_SilentStepperV2 *silent_stepper_v2, int32_t steps) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_STEPS, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_STEPS, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    steps = tf_leconvert_int32_to(steps); memcpy(buf + 0, &steps, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_steps(TF_SilentStepperV2 *silent_stepper_v2, int32_t *ret_steps) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_STEPS, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_steps != NULL) { *ret_steps = tf_packetbuffer_read_int32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_remaining_steps(TF_SilentStepperV2 *silent_stepper_v2, int32_t *ret_steps) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_REMAINING_STEPS, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_steps != NULL) { *ret_steps = tf_packetbuffer_read_int32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_step_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t step_resolution, bool interpolation) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_STEP_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_STEP_CONFIGURATION, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = (uint8_t)step_resolution;
    buf[1] = interpolation ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_step_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_step_resolution, bool *ret_interpolation) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_STEP_CONFIGURATION, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_step_resolution != NULL) { *ret_step_resolution = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_interpolation != NULL) { *ret_interpolation = tf_packetbuffer_read_bool(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_drive_forward(TF_SilentStepperV2 *silent_stepper_v2) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_FORWARD, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_FORWARD, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_drive_backward(TF_SilentStepperV2 *silent_stepper_v2) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_BACKWARD, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_BACKWARD, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_stop(TF_SilentStepperV2 *silent_stepper_v2) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_STOP, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_STOP, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_input_voltage(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_voltage) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_INPUT_VOLTAGE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_voltage != NULL) { *ret_voltage = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_motor_current(TF_SilentStepperV2 *silent_stepper_v2, uint16_t current) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_MOTOR_CURRENT, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_MOTOR_CURRENT, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    current = tf_leconvert_uint16_to(current); memcpy(buf + 0, &current, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_motor_current(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_current) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_MOTOR_CURRENT, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_current != NULL) { *ret_current = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_enabled(TF_SilentStepperV2 *silent_stepper_v2, bool enabled) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_ENABLED, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_ENABLED, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = enabled ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_enabled(TF_SilentStepperV2 *silent_stepper_v2, bool *ret_enabled) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_ENABLED, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_enabled != NULL) { *ret_enabled = tf_packetbuffer_read_bool(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_basic_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint16_t standstill_current, uint16_t motor_run_current, uint16_t standstill_delay_time, uint16_t power_down_time, uint16_t stealth_threshold, uint16_t coolstep_threshold, uint16_t classic_threshold, bool high_velocity_chopper_mode) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_BASIC_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_BASIC_CONFIGURATION, 15, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    standstill_current = tf_leconvert_uint16_to(standstill_current); memcpy(buf + 0, &standstill_current, 2);
    motor_run_current = tf_leconvert_uint16_to(motor_run_current); memcpy(buf + 2, &motor_run_current, 2);
    standstill_delay_time = tf_leconvert_uint16_to(standstill_delay_time); memcpy(buf + 4, &standstill_delay_time, 2);
    power_down_time = tf_leconvert_uint16_to(power_down_time); memcpy(buf + 6, &power_down_time, 2);
    stealth_threshold = tf_leconvert_uint16_to(stealth_threshold); memcpy(buf + 8, &stealth_threshold, 2);
    coolstep_threshold = tf_leconvert_uint16_to(coolstep_threshold); memcpy(buf + 10, &coolstep_threshold, 2);
    classic_threshold = tf_leconvert_uint16_to(classic_threshold); memcpy(buf + 12, &classic_threshold, 2);
    buf[14] = high_velocity_chopper_mode ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_basic_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_standstill_current, uint16_t *ret_motor_run_current, uint16_t *ret_standstill_delay_time, uint16_t *ret_power_down_time, uint16_t *ret_stealth_threshold, uint16_t *ret_coolstep_threshold, uint16_t *ret_classic_threshold, bool *ret_high_velocity_chopper_mode) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_BASIC_CONFIGURATION, 0, 15, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_standstill_current != NULL) { *ret_standstill_current = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_motor_run_current != NULL) { *ret_motor_run_current = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_standstill_delay_time != NULL) { *ret_standstill_delay_time = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_power_down_time != NULL) { *ret_power_down_time = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_stealth_threshold != NULL) { *ret_stealth_threshold = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_coolstep_threshold != NULL) { *ret_coolstep_threshold = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_classic_threshold != NULL) { *ret_classic_threshold = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_high_velocity_chopper_mode != NULL) { *ret_high_velocity_chopper_mode = tf_packetbuffer_read_bool(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_spreadcycle_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t slow_decay_duration, bool enable_random_slow_decay, uint8_t fast_decay_duration, uint8_t hysteresis_start_value, int8_t hysteresis_end_value, int8_t sine_wave_offset, uint8_t chopper_mode, uint8_t comparator_blank_time, bool fast_decay_without_comparator) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_SPREADCYCLE_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_SPREADCYCLE_CONFIGURATION, 9, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = (uint8_t)slow_decay_duration;
    buf[1] = enable_random_slow_decay ? 1 : 0;
    buf[2] = (uint8_t)fast_decay_duration;
    buf[3] = (uint8_t)hysteresis_start_value;
    buf[4] = (uint8_t)hysteresis_end_value;
    buf[5] = (uint8_t)sine_wave_offset;
    buf[6] = (uint8_t)chopper_mode;
    buf[7] = (uint8_t)comparator_blank_time;
    buf[8] = fast_decay_without_comparator ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_spreadcycle_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_slow_decay_duration, bool *ret_enable_random_slow_decay, uint8_t *ret_fast_decay_duration, uint8_t *ret_hysteresis_start_value, int8_t *ret_hysteresis_end_value, int8_t *ret_sine_wave_offset, uint8_t *ret_chopper_mode, uint8_t *ret_comparator_blank_time, bool *ret_fast_decay_without_comparator) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_SPREADCYCLE_CONFIGURATION, 0, 9, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_slow_decay_duration != NULL) { *ret_slow_decay_duration = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_enable_random_slow_decay != NULL) { *ret_enable_random_slow_decay = tf_packetbuffer_read_bool(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_fast_decay_duration != NULL) { *ret_fast_decay_duration = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_hysteresis_start_value != NULL) { *ret_hysteresis_start_value = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_hysteresis_end_value != NULL) { *ret_hysteresis_end_value = tf_packetbuffer_read_int8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_sine_wave_offset != NULL) { *ret_sine_wave_offset = tf_packetbuffer_read_int8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_chopper_mode != NULL) { *ret_chopper_mode = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_comparator_blank_time != NULL) { *ret_comparator_blank_time = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_fast_decay_without_comparator != NULL) { *ret_fast_decay_without_comparator = tf_packetbuffer_read_bool(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_stealth_configuration(TF_SilentStepperV2 *silent_stepper_v2, bool enable_stealth, uint8_t amplitude, uint8_t gradient, bool enable_autoscale, bool force_symmetric, uint8_t freewheel_mode) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_STEALTH_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_STEALTH_CONFIGURATION, 6, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = enable_stealth ? 1 : 0;
    buf[1] = (uint8_t)amplitude;
    buf[2] = (uint8_t)gradient;
    buf[3] = enable_autoscale ? 1 : 0;
    buf[4] = force_symmetric ? 1 : 0;
    buf[5] = (uint8_t)freewheel_mode;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_stealth_configuration(TF_SilentStepperV2 *silent_stepper_v2, bool *ret_enable_stealth, uint8_t *ret_amplitude, uint8_t *ret_gradient, bool *ret_enable_autoscale, bool *ret_force_symmetric, uint8_t *ret_freewheel_mode) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_STEALTH_CONFIGURATION, 0, 6, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_enable_stealth != NULL) { *ret_enable_stealth = tf_packetbuffer_read_bool(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_amplitude != NULL) { *ret_amplitude = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_gradient != NULL) { *ret_gradient = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_enable_autoscale != NULL) { *ret_enable_autoscale = tf_packetbuffer_read_bool(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_force_symmetric != NULL) { *ret_force_symmetric = tf_packetbuffer_read_bool(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_freewheel_mode != NULL) { *ret_freewheel_mode = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_coolstep_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t minimum_stallguard_value, uint8_t maximum_stallguard_value, uint8_t current_up_step_width, uint8_t current_down_step_width, uint8_t minimum_current, int8_t stallguard_threshold_value, uint8_t stallguard_mode) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_COOLSTEP_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_COOLSTEP_CONFIGURATION, 7, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = (uint8_t)minimum_stallguard_value;
    buf[1] = (uint8_t)maximum_stallguard_value;
    buf[2] = (uint8_t)current_up_step_width;
    buf[3] = (uint8_t)current_down_step_width;
    buf[4] = (uint8_t)minimum_current;
    buf[5] = (uint8_t)stallguard_threshold_value;
    buf[6] = (uint8_t)stallguard_mode;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_coolstep_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_minimum_stallguard_value, uint8_t *ret_maximum_stallguard_value, uint8_t *ret_current_up_step_width, uint8_t *ret_current_down_step_width, uint8_t *ret_minimum_current, int8_t *ret_stallguard_threshold_value, uint8_t *ret_stallguard_mode) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_COOLSTEP_CONFIGURATION, 0, 7, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_minimum_stallguard_value != NULL) { *ret_minimum_stallguard_value = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_maximum_stallguard_value != NULL) { *ret_maximum_stallguard_value = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_current_up_step_width != NULL) { *ret_current_up_step_width = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_current_down_step_width != NULL) { *ret_current_down_step_width = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_minimum_current != NULL) { *ret_minimum_current = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_stallguard_threshold_value != NULL) { *ret_stallguard_threshold_value = tf_packetbuffer_read_int8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_stallguard_mode != NULL) { *ret_stallguard_mode = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_misc_configuration(TF_SilentStepperV2 *silent_stepper_v2, bool disable_short_to_ground_protection, uint8_t synchronize_phase_frequency) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_MISC_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_MISC_CONFIGURATION, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = disable_short_to_ground_protection ? 1 : 0;
    buf[1] = (uint8_t)synchronize_phase_frequency;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_misc_configuration(TF_SilentStepperV2 *silent_stepper_v2, bool *ret_disable_short_to_ground_protection, uint8_t *ret_synchronize_phase_frequency) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_MISC_CONFIGURATION, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_disable_short_to_ground_protection != NULL) { *ret_disable_short_to_ground_protection = tf_packetbuffer_read_bool(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_synchronize_phase_frequency != NULL) { *ret_synchronize_phase_frequency = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_error_led_config(TF_SilentStepperV2 *silent_stepper_v2, uint8_t config) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_ERROR_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_ERROR_LED_CONFIG, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_error_led_config(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_config) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_ERROR_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_config != NULL) { *ret_config = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_driver_status(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_open_load, uint8_t *ret_short_to_ground, uint8_t *ret_over_temperature, bool *ret_motor_stalled, uint8_t *ret_actual_motor_current, bool *ret_full_step_active, uint8_t *ret_stallguard_result, uint8_t *ret_stealth_voltage_amplitude) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_DRIVER_STATUS, 0, 8, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_open_load != NULL) { *ret_open_load = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_short_to_ground != NULL) { *ret_short_to_ground = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_over_temperature != NULL) { *ret_over_temperature = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_motor_stalled != NULL) { *ret_motor_stalled = tf_packetbuffer_read_bool(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_actual_motor_current != NULL) { *ret_actual_motor_current = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_full_step_active != NULL) { *ret_full_step_active = tf_packetbuffer_read_bool(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_stallguard_result != NULL) { *ret_stallguard_result = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_stealth_voltage_amplitude != NULL) { *ret_stealth_voltage_amplitude = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_minimum_voltage(TF_SilentStepperV2 *silent_stepper_v2, uint16_t voltage) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_MINIMUM_VOLTAGE, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_MINIMUM_VOLTAGE, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    voltage = tf_leconvert_uint16_to(voltage); memcpy(buf + 0, &voltage, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_minimum_voltage(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_voltage) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_MINIMUM_VOLTAGE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_voltage != NULL) { *ret_voltage = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_time_base(TF_SilentStepperV2 *silent_stepper_v2, uint32_t time_base) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_TIME_BASE, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_TIME_BASE, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    time_base = tf_leconvert_uint32_to(time_base); memcpy(buf + 0, &time_base, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_time_base(TF_SilentStepperV2 *silent_stepper_v2, uint32_t *ret_time_base) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_TIME_BASE, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_time_base != NULL) { *ret_time_base = tf_packetbuffer_read_uint32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_all_data(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_current_velocity, int32_t *ret_current_position, int32_t *ret_remaining_steps, uint16_t *ret_input_voltage, uint16_t *ret_current_consumption) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_ALL_DATA, 0, 14, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_current_velocity != NULL) { *ret_current_velocity = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_current_position != NULL) { *ret_current_position = tf_packetbuffer_read_int32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_remaining_steps != NULL) { *ret_remaining_steps = tf_packetbuffer_read_int32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_input_voltage != NULL) { *ret_input_voltage = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_current_consumption != NULL) { *ret_current_consumption = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_all_callback_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint32_t period) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_ALL_CALLBACK_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_ALL_CALLBACK_CONFIGURATION, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(buf + 0, &period, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_all_data_callback_configuraton(TF_SilentStepperV2 *silent_stepper_v2, uint32_t *ret_period) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_ALL_DATA_CALLBACK_CONFIGURATON, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_period != NULL) { *ret_period = tf_packetbuffer_read_uint32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_gpio_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t channel, uint16_t debounce, uint16_t stop_deceleration) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_CONFIGURATION, 5, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = (uint8_t)channel;
    debounce = tf_leconvert_uint16_to(debounce); memcpy(buf + 1, &debounce, 2);
    stop_deceleration = tf_leconvert_uint16_to(stop_deceleration); memcpy(buf + 3, &stop_deceleration, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_gpio_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t channel, uint16_t *ret_debounce, uint16_t *ret_stop_deceleration) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_GPIO_CONFIGURATION, 1, 4, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = (uint8_t)channel;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_debounce != NULL) { *ret_debounce = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_stop_deceleration != NULL) { *ret_stop_deceleration = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_gpio_action(TF_SilentStepperV2 *silent_stepper_v2, uint8_t channel, uint32_t action) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_ACTION, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_ACTION, 5, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = (uint8_t)channel;
    action = tf_leconvert_uint32_to(action); memcpy(buf + 1, &action, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_gpio_action(TF_SilentStepperV2 *silent_stepper_v2, uint8_t channel, uint32_t *ret_action) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_GPIO_ACTION, 1, 4, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = (uint8_t)channel;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_action != NULL) { *ret_action = tf_packetbuffer_read_uint32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_gpio_state(TF_SilentStepperV2 *silent_stepper_v2, bool ret_gpio_state[2]) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_GPIO_STATE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_gpio_state != NULL) { tf_packetbuffer_read_bool_array(&silent_stepper_v2->tfp->spitfp->recv_buf, ret_gpio_state, 2);} else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_spitfp_error_count(TF_SilentStepperV2 *silent_stepper_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, 16, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packetbuffer_read_uint32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packetbuffer_read_uint32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packetbuffer_read_uint32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packetbuffer_read_uint32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_bootloader_mode(TF_SilentStepperV2 *silent_stepper_v2, uint8_t mode, uint8_t *ret_status) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = (uint8_t)mode;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_bootloader_mode(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_mode) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_mode != NULL) { *ret_mode = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_write_firmware_pointer(TF_SilentStepperV2 *silent_stepper_v2, uint32_t pointer) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(buf + 0, &pointer, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_write_firmware(TF_SilentStepperV2 *silent_stepper_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_WRITE_FIRMWARE, 64, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    memcpy(buf + 0, data, 64);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_set_status_led_config(TF_SilentStepperV2 *silent_stepper_v2, uint8_t config) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_STATUS_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_status_led_config(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_config) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_config != NULL) { *ret_config = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_chip_temperature(TF_SilentStepperV2 *silent_stepper_v2, int16_t *ret_temperature) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_temperature != NULL) { *ret_temperature = tf_packetbuffer_read_int16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_reset(TF_SilentStepperV2 *silent_stepper_v2) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_RESET, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_RESET, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_write_uid(TF_SilentStepperV2 *silent_stepper_v2, uint32_t uid) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_WRITE_UID, &response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_WRITE_UID, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(silent_stepper_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(buf + 0, &uid, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_read_uid(TF_SilentStepperV2 *silent_stepper_v2, uint32_t *ret_uid) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_READ_UID, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_uid != NULL) { *ret_uid = tf_packetbuffer_read_uint32_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_silent_stepper_v2_get_identity(TF_SilentStepperV2 *silent_stepper_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_IDENTITY, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)silent_stepper_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(silent_stepper_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        char tmp_connected_uid[8] = {0};
        if (ret_uid != NULL) { tf_packetbuffer_pop_n(&silent_stepper_v2->tfp->spitfp->recv_buf, (uint8_t*)ret_uid, 8);} else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 8); }
        tf_packetbuffer_pop_n(&silent_stepper_v2->tfp->spitfp->recv_buf, (uint8_t*)tmp_connected_uid, 8);
        if (ret_position != NULL) { *ret_position = tf_packetbuffer_read_char(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_hardware_version != NULL) { for (i = 0; i < 3; ++i) ret_hardware_version[i] = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 3); }
        if (ret_firmware_version != NULL) { for (i = 0; i < 3; ++i) ret_firmware_version[i] = tf_packetbuffer_read_uint8_t(&silent_stepper_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 3); }
        if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packetbuffer_read_uint16_t(&silent_stepper_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&silent_stepper_v2->tfp->spitfp->recv_buf, 2); }
        if (tmp_connected_uid[0] == 0 && ret_position != NULL) {
            *ret_position = tf_hal_get_port_name((TF_HalContext*)silent_stepper_v2->tfp->hal, silent_stepper_v2->tfp->spitfp->port_id);
        }
        if (ret_connected_uid != NULL) {
            memcpy(ret_connected_uid, tmp_connected_uid, 8);
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }

    result = tf_tfp_finish_send(silent_stepper_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_silent_stepper_v2_register_under_voltage_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2UnderVoltageHandler handler, void *user_data) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if (handler == NULL) {
        silent_stepper_v2->tfp->needs_callback_tick = false;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->position_reached_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->all_data_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->new_state_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->gpio_state_handler != NULL;
    } else {
        silent_stepper_v2->tfp->needs_callback_tick = true;
    }
    silent_stepper_v2->under_voltage_handler = handler;
    silent_stepper_v2->under_voltage_user_data = user_data;
    return TF_E_OK;
}


int tf_silent_stepper_v2_register_position_reached_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2PositionReachedHandler handler, void *user_data) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if (handler == NULL) {
        silent_stepper_v2->tfp->needs_callback_tick = false;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->under_voltage_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->all_data_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->new_state_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->gpio_state_handler != NULL;
    } else {
        silent_stepper_v2->tfp->needs_callback_tick = true;
    }
    silent_stepper_v2->position_reached_handler = handler;
    silent_stepper_v2->position_reached_user_data = user_data;
    return TF_E_OK;
}


int tf_silent_stepper_v2_register_all_data_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2AllDataHandler handler, void *user_data) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if (handler == NULL) {
        silent_stepper_v2->tfp->needs_callback_tick = false;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->under_voltage_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->position_reached_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->new_state_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->gpio_state_handler != NULL;
    } else {
        silent_stepper_v2->tfp->needs_callback_tick = true;
    }
    silent_stepper_v2->all_data_handler = handler;
    silent_stepper_v2->all_data_user_data = user_data;
    return TF_E_OK;
}


int tf_silent_stepper_v2_register_new_state_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2NewStateHandler handler, void *user_data) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if (handler == NULL) {
        silent_stepper_v2->tfp->needs_callback_tick = false;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->under_voltage_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->position_reached_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->all_data_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->gpio_state_handler != NULL;
    } else {
        silent_stepper_v2->tfp->needs_callback_tick = true;
    }
    silent_stepper_v2->new_state_handler = handler;
    silent_stepper_v2->new_state_user_data = user_data;
    return TF_E_OK;
}


int tf_silent_stepper_v2_register_gpio_state_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2GPIOStateHandler handler, void *user_data) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    if (handler == NULL) {
        silent_stepper_v2->tfp->needs_callback_tick = false;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->under_voltage_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->position_reached_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->all_data_handler != NULL;
        silent_stepper_v2->tfp->needs_callback_tick |= silent_stepper_v2->new_state_handler != NULL;
    } else {
        silent_stepper_v2->tfp->needs_callback_tick = true;
    }
    silent_stepper_v2->gpio_state_handler = handler;
    silent_stepper_v2->gpio_state_user_data = user_data;
    return TF_E_OK;
}
#endif
int tf_silent_stepper_v2_callback_tick(TF_SilentStepperV2 *silent_stepper_v2, uint32_t timeout_us) {
    if (silent_stepper_v2 == NULL)
        return TF_E_NULL;

    return tf_tfp_callback_tick(silent_stepper_v2->tfp, tf_hal_current_time_us((TF_HalContext*)silent_stepper_v2->tfp->hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
