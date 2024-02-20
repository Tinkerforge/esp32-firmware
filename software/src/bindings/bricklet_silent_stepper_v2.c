/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
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
static bool tf_silent_stepper_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_SilentStepperV2 *silent_stepper_v2 = (TF_SilentStepperV2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(silent_stepper_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_SILENT_STEPPER_V2_CALLBACK_UNDER_VOLTAGE: {
            TF_SilentStepperV2_UnderVoltageHandler fn = silent_stepper_v2->under_voltage_handler;
            void *user_data = silent_stepper_v2->under_voltage_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t voltage = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(silent_stepper_v2, voltage, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_SILENT_STEPPER_V2_CALLBACK_POSITION_REACHED: {
            TF_SilentStepperV2_PositionReachedHandler fn = silent_stepper_v2->position_reached_handler;
            void *user_data = silent_stepper_v2->position_reached_user_data;
            if (fn == NULL) {
                return false;
            }

            int32_t position = tf_packet_buffer_read_int32_t(payload);
            hal_common->locked = true;
            fn(silent_stepper_v2, position, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_SILENT_STEPPER_V2_CALLBACK_ALL_DATA: {
            TF_SilentStepperV2_AllDataHandler fn = silent_stepper_v2->all_data_handler;
            void *user_data = silent_stepper_v2->all_data_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t current_velocity = tf_packet_buffer_read_uint16_t(payload);
            int32_t current_position = tf_packet_buffer_read_int32_t(payload);
            int32_t remaining_steps = tf_packet_buffer_read_int32_t(payload);
            uint16_t input_voltage = tf_packet_buffer_read_uint16_t(payload);
            uint16_t current_consumption = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(silent_stepper_v2, current_velocity, current_position, remaining_steps, input_voltage, current_consumption, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_SILENT_STEPPER_V2_CALLBACK_NEW_STATE: {
            TF_SilentStepperV2_NewStateHandler fn = silent_stepper_v2->new_state_handler;
            void *user_data = silent_stepper_v2->new_state_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t state_new = tf_packet_buffer_read_uint8_t(payload);
            uint8_t state_previous = tf_packet_buffer_read_uint8_t(payload);
            hal_common->locked = true;
            fn(silent_stepper_v2, state_new, state_previous, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_SILENT_STEPPER_V2_CALLBACK_GPIO_STATE: {
            TF_SilentStepperV2_GPIOStateHandler fn = silent_stepper_v2->gpio_state_handler;
            void *user_data = silent_stepper_v2->gpio_state_user_data;
            if (fn == NULL) {
                return false;
            }

            bool gpio_state[2]; tf_packet_buffer_read_bool_array(payload, gpio_state, 2);
            hal_common->locked = true;
            fn(silent_stepper_v2, gpio_state, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_silent_stepper_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_silent_stepper_v2_create(TF_SilentStepperV2 *silent_stepper_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (silent_stepper_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(silent_stepper_v2, 0, sizeof(TF_SilentStepperV2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_SILENT_STEPPER_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    silent_stepper_v2->tfp = tfp;
    silent_stepper_v2->tfp->device = silent_stepper_v2;
    silent_stepper_v2->tfp->cb_handler = tf_silent_stepper_v2_callback_handler;
    silent_stepper_v2->magic = 0x5446;
    silent_stepper_v2->response_expected[0] = 0x00;
    silent_stepper_v2->response_expected[1] = 0x00;
    silent_stepper_v2->response_expected[2] = 0x14;
    silent_stepper_v2->response_expected[3] = 0x00;
    return TF_E_OK;
}

int tf_silent_stepper_v2_destroy(TF_SilentStepperV2 *silent_stepper_v2) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }
    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    silent_stepper_v2->tfp->cb_handler = NULL;
    silent_stepper_v2->tfp->device = NULL;
    silent_stepper_v2->tfp = NULL;
    silent_stepper_v2->magic = 0;

    return TF_E_OK;
}

int tf_silent_stepper_v2_get_response_expected(TF_SilentStepperV2 *silent_stepper_v2, uint8_t function_id, bool *ret_response_expected) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_MAX_VELOCITY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_SPEED_RAMPING:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_FULL_BRAKE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_CURRENT_POSITION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_TARGET_POSITION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_STEPS:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_STEP_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_FORWARD:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_BACKWARD:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_STOP:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_MOTOR_CURRENT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_ENABLED:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_BASIC_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 4)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_SPREADCYCLE_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 5)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_STEALTH_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 6)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_COOLSTEP_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[1] & (1 << 7)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_MISC_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 0)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_ERROR_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 1)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_MINIMUM_VOLTAGE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 2)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_TIME_BASE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 3)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_ALL_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 4)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 5)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_ACTION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 6)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[2] & (1 << 7)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[3] & (1 << 0)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[3] & (1 << 1)) != 0;
            }
            break;
        case TF_SILENT_STEPPER_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (silent_stepper_v2->response_expected[3] & (1 << 2)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_silent_stepper_v2_set_response_expected(TF_SilentStepperV2 *silent_stepper_v2, uint8_t function_id, bool response_expected) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
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

int tf_silent_stepper_v2_set_response_expected_all(TF_SilentStepperV2 *silent_stepper_v2, bool response_expected) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(silent_stepper_v2->response_expected, response_expected ? 0xFF : 0, 4);

    return TF_E_OK;
}

int tf_silent_stepper_v2_set_max_velocity(TF_SilentStepperV2 *silent_stepper_v2, uint16_t velocity) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_MAX_VELOCITY, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_MAX_VELOCITY, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    velocity = tf_leconvert_uint16_to(velocity); memcpy(_send_buf + 0, &velocity, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_max_velocity(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_velocity) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_MAX_VELOCITY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_velocity != NULL) { *ret_velocity = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_current_velocity(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_velocity) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_CURRENT_VELOCITY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_velocity != NULL) { *ret_velocity = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_speed_ramping(TF_SilentStepperV2 *silent_stepper_v2, uint16_t acceleration, uint16_t deacceleration) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_SPEED_RAMPING, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_SPEED_RAMPING, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    acceleration = tf_leconvert_uint16_to(acceleration); memcpy(_send_buf + 0, &acceleration, 2);
    deacceleration = tf_leconvert_uint16_to(deacceleration); memcpy(_send_buf + 2, &deacceleration, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_speed_ramping(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_acceleration, uint16_t *ret_deacceleration) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_SPEED_RAMPING, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_acceleration != NULL) { *ret_acceleration = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_deacceleration != NULL) { *ret_deacceleration = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_full_brake(TF_SilentStepperV2 *silent_stepper_v2) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_FULL_BRAKE, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_FULL_BRAKE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_current_position(TF_SilentStepperV2 *silent_stepper_v2, int32_t position) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_CURRENT_POSITION, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_CURRENT_POSITION, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    position = tf_leconvert_int32_to(position); memcpy(_send_buf + 0, &position, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_current_position(TF_SilentStepperV2 *silent_stepper_v2, int32_t *ret_position) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_CURRENT_POSITION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_position != NULL) { *ret_position = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_target_position(TF_SilentStepperV2 *silent_stepper_v2, int32_t position) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_TARGET_POSITION, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_TARGET_POSITION, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    position = tf_leconvert_int32_to(position); memcpy(_send_buf + 0, &position, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_target_position(TF_SilentStepperV2 *silent_stepper_v2, int32_t *ret_position) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_TARGET_POSITION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_position != NULL) { *ret_position = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_steps(TF_SilentStepperV2 *silent_stepper_v2, int32_t steps) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_STEPS, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_STEPS, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    steps = tf_leconvert_int32_to(steps); memcpy(_send_buf + 0, &steps, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_steps(TF_SilentStepperV2 *silent_stepper_v2, int32_t *ret_steps) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_STEPS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_steps != NULL) { *ret_steps = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_remaining_steps(TF_SilentStepperV2 *silent_stepper_v2, int32_t *ret_steps) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_REMAINING_STEPS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_steps != NULL) { *ret_steps = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_step_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t step_resolution, bool interpolation) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_STEP_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_STEP_CONFIGURATION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    _send_buf[0] = (uint8_t)step_resolution;
    _send_buf[1] = interpolation ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_step_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_step_resolution, bool *ret_interpolation) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_STEP_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_step_resolution != NULL) { *ret_step_resolution = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_interpolation != NULL) { *ret_interpolation = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_drive_forward(TF_SilentStepperV2 *silent_stepper_v2) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_FORWARD, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_FORWARD, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_drive_backward(TF_SilentStepperV2 *silent_stepper_v2) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_BACKWARD, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_DRIVE_BACKWARD, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_stop(TF_SilentStepperV2 *silent_stepper_v2) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_STOP, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_STOP, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_input_voltage(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_voltage) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_INPUT_VOLTAGE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_voltage != NULL) { *ret_voltage = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_motor_current(TF_SilentStepperV2 *silent_stepper_v2, uint16_t current) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_MOTOR_CURRENT, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_MOTOR_CURRENT, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    current = tf_leconvert_uint16_to(current); memcpy(_send_buf + 0, &current, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_motor_current(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_current) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_MOTOR_CURRENT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_current != NULL) { *ret_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_enabled(TF_SilentStepperV2 *silent_stepper_v2, bool enabled) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_ENABLED, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_ENABLED, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    _send_buf[0] = enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_enabled(TF_SilentStepperV2 *silent_stepper_v2, bool *ret_enabled) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_ENABLED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_basic_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint16_t standstill_current, uint16_t motor_run_current, uint16_t standstill_delay_time, uint16_t power_down_time, uint16_t stealth_threshold, uint16_t coolstep_threshold, uint16_t classic_threshold, bool high_velocity_chopper_mode) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_BASIC_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_BASIC_CONFIGURATION, 15, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

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
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_basic_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_standstill_current, uint16_t *ret_motor_run_current, uint16_t *ret_standstill_delay_time, uint16_t *ret_power_down_time, uint16_t *ret_stealth_threshold, uint16_t *ret_coolstep_threshold, uint16_t *ret_classic_threshold, bool *ret_high_velocity_chopper_mode) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_BASIC_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
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
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 15) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_spreadcycle_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t slow_decay_duration, bool enable_random_slow_decay, uint8_t fast_decay_duration, uint8_t hysteresis_start_value, int8_t hysteresis_end_value, int8_t sine_wave_offset, uint8_t chopper_mode, uint8_t comparator_blank_time, bool fast_decay_without_comparator) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_SPREADCYCLE_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_SPREADCYCLE_CONFIGURATION, 9, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    _send_buf[0] = (uint8_t)slow_decay_duration;
    _send_buf[1] = enable_random_slow_decay ? 1 : 0;
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
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_spreadcycle_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_slow_decay_duration, bool *ret_enable_random_slow_decay, uint8_t *ret_fast_decay_duration, uint8_t *ret_hysteresis_start_value, int8_t *ret_hysteresis_end_value, int8_t *ret_sine_wave_offset, uint8_t *ret_chopper_mode, uint8_t *ret_comparator_blank_time, bool *ret_fast_decay_without_comparator) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_SPREADCYCLE_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 9) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_slow_decay_duration != NULL) { *ret_slow_decay_duration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_enable_random_slow_decay != NULL) { *ret_enable_random_slow_decay = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_fast_decay_duration != NULL) { *ret_fast_decay_duration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_hysteresis_start_value != NULL) { *ret_hysteresis_start_value = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_hysteresis_end_value != NULL) { *ret_hysteresis_end_value = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_sine_wave_offset != NULL) { *ret_sine_wave_offset = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_chopper_mode != NULL) { *ret_chopper_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_comparator_blank_time != NULL) { *ret_comparator_blank_time = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_fast_decay_without_comparator != NULL) { *ret_fast_decay_without_comparator = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 9) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_stealth_configuration(TF_SilentStepperV2 *silent_stepper_v2, bool enable_stealth, uint8_t amplitude, uint8_t gradient, bool enable_autoscale, bool force_symmetric, uint8_t freewheel_mode) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_STEALTH_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_STEALTH_CONFIGURATION, 6, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    _send_buf[0] = enable_stealth ? 1 : 0;
    _send_buf[1] = (uint8_t)amplitude;
    _send_buf[2] = (uint8_t)gradient;
    _send_buf[3] = enable_autoscale ? 1 : 0;
    _send_buf[4] = force_symmetric ? 1 : 0;
    _send_buf[5] = (uint8_t)freewheel_mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_stealth_configuration(TF_SilentStepperV2 *silent_stepper_v2, bool *ret_enable_stealth, uint8_t *ret_amplitude, uint8_t *ret_gradient, bool *ret_enable_autoscale, bool *ret_force_symmetric, uint8_t *ret_freewheel_mode) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_STEALTH_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enable_stealth != NULL) { *ret_enable_stealth = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_amplitude != NULL) { *ret_amplitude = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_gradient != NULL) { *ret_gradient = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_enable_autoscale != NULL) { *ret_enable_autoscale = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_force_symmetric != NULL) { *ret_force_symmetric = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_freewheel_mode != NULL) { *ret_freewheel_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_coolstep_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t minimum_stallguard_value, uint8_t maximum_stallguard_value, uint8_t current_up_step_width, uint8_t current_down_step_width, uint8_t minimum_current, int8_t stallguard_threshold_value, uint8_t stallguard_mode) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_COOLSTEP_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_COOLSTEP_CONFIGURATION, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

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
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_coolstep_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_minimum_stallguard_value, uint8_t *ret_maximum_stallguard_value, uint8_t *ret_current_up_step_width, uint8_t *ret_current_down_step_width, uint8_t *ret_minimum_current, int8_t *ret_stallguard_threshold_value, uint8_t *ret_stallguard_mode) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_COOLSTEP_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
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
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 7) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_misc_configuration(TF_SilentStepperV2 *silent_stepper_v2, bool disable_short_to_ground_protection, uint8_t synchronize_phase_frequency) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_MISC_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_MISC_CONFIGURATION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    _send_buf[0] = disable_short_to_ground_protection ? 1 : 0;
    _send_buf[1] = (uint8_t)synchronize_phase_frequency;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_misc_configuration(TF_SilentStepperV2 *silent_stepper_v2, bool *ret_disable_short_to_ground_protection, uint8_t *ret_synchronize_phase_frequency) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_MISC_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_disable_short_to_ground_protection != NULL) { *ret_disable_short_to_ground_protection = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_synchronize_phase_frequency != NULL) { *ret_synchronize_phase_frequency = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_error_led_config(TF_SilentStepperV2 *silent_stepper_v2, uint8_t config) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_ERROR_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_ERROR_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_error_led_config(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_config) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_ERROR_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_driver_status(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_open_load, uint8_t *ret_short_to_ground, uint8_t *ret_over_temperature, bool *ret_motor_stalled, uint8_t *ret_actual_motor_current, bool *ret_full_step_active, uint8_t *ret_stallguard_result, uint8_t *ret_stealth_voltage_amplitude) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_DRIVER_STATUS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
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
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_minimum_voltage(TF_SilentStepperV2 *silent_stepper_v2, uint16_t voltage) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_MINIMUM_VOLTAGE, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_MINIMUM_VOLTAGE, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    voltage = tf_leconvert_uint16_to(voltage); memcpy(_send_buf + 0, &voltage, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_minimum_voltage(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_voltage) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_MINIMUM_VOLTAGE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_voltage != NULL) { *ret_voltage = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_time_base(TF_SilentStepperV2 *silent_stepper_v2, uint32_t time_base) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_TIME_BASE, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_TIME_BASE, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    time_base = tf_leconvert_uint32_to(time_base); memcpy(_send_buf + 0, &time_base, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_time_base(TF_SilentStepperV2 *silent_stepper_v2, uint32_t *ret_time_base) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_TIME_BASE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_time_base != NULL) { *ret_time_base = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_all_data(TF_SilentStepperV2 *silent_stepper_v2, uint16_t *ret_current_velocity, int32_t *ret_current_position, int32_t *ret_remaining_steps, uint16_t *ret_input_voltage, uint16_t *ret_current_consumption) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_ALL_DATA, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 14) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_current_velocity != NULL) { *ret_current_velocity = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_current_position != NULL) { *ret_current_position = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_remaining_steps != NULL) { *ret_remaining_steps = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_input_voltage != NULL) { *ret_input_voltage = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_current_consumption != NULL) { *ret_current_consumption = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 14) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_all_callback_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint32_t period) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_ALL_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_ALL_CALLBACK_CONFIGURATION, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_all_data_callback_configuraton(TF_SilentStepperV2 *silent_stepper_v2, uint32_t *ret_period) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_ALL_DATA_CALLBACK_CONFIGURATON, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_gpio_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t channel, uint16_t debounce, uint16_t stop_deceleration) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    _send_buf[0] = (uint8_t)channel;
    debounce = tf_leconvert_uint16_to(debounce); memcpy(_send_buf + 1, &debounce, 2);
    stop_deceleration = tf_leconvert_uint16_to(stop_deceleration); memcpy(_send_buf + 3, &stop_deceleration, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_gpio_configuration(TF_SilentStepperV2 *silent_stepper_v2, uint8_t channel, uint16_t *ret_debounce, uint16_t *ret_stop_deceleration) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_GPIO_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_debounce != NULL) { *ret_debounce = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_stop_deceleration != NULL) { *ret_stop_deceleration = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_gpio_action(TF_SilentStepperV2 *silent_stepper_v2, uint8_t channel, uint32_t action) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_ACTION, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_GPIO_ACTION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    _send_buf[0] = (uint8_t)channel;
    action = tf_leconvert_uint32_to(action); memcpy(_send_buf + 1, &action, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_gpio_action(TF_SilentStepperV2 *silent_stepper_v2, uint8_t channel, uint32_t *ret_action) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_GPIO_ACTION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_action != NULL) { *ret_action = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_gpio_state(TF_SilentStepperV2 *silent_stepper_v2, bool ret_gpio_state[2]) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_GPIO_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_gpio_state != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_gpio_state, 2);} else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_spitfp_error_count(TF_SilentStepperV2 *silent_stepper_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_bootloader_mode(TF_SilentStepperV2 *silent_stepper_v2, uint8_t mode, uint8_t *ret_status) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_bootloader_mode(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_mode) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_write_firmware_pointer(TF_SilentStepperV2 *silent_stepper_v2, uint32_t pointer) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_write_firmware(TF_SilentStepperV2 *silent_stepper_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_set_status_led_config(TF_SilentStepperV2 *silent_stepper_v2, uint8_t config) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_status_led_config(TF_SilentStepperV2 *silent_stepper_v2, uint8_t *ret_config) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_chip_temperature(TF_SilentStepperV2 *silent_stepper_v2, int16_t *ret_temperature) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_reset(TF_SilentStepperV2 *silent_stepper_v2) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_write_uid(TF_SilentStepperV2 *silent_stepper_v2, uint32_t uid) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_silent_stepper_v2_get_response_expected(silent_stepper_v2, TF_SILENT_STEPPER_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(silent_stepper_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_read_uid(TF_SilentStepperV2 *silent_stepper_v2, uint32_t *ret_uid) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_silent_stepper_v2_get_identity(TF_SilentStepperV2 *silent_stepper_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = silent_stepper_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(silent_stepper_v2->tfp, TF_SILENT_STEPPER_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(silent_stepper_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(silent_stepper_v2->tfp);
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
        tf_tfp_packet_processed(silent_stepper_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(silent_stepper_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_silent_stepper_v2_register_under_voltage_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2_UnderVoltageHandler handler, void *user_data) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    silent_stepper_v2->under_voltage_handler = handler;
    silent_stepper_v2->under_voltage_user_data = user_data;

    return TF_E_OK;
}


int tf_silent_stepper_v2_register_position_reached_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2_PositionReachedHandler handler, void *user_data) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    silent_stepper_v2->position_reached_handler = handler;
    silent_stepper_v2->position_reached_user_data = user_data;

    return TF_E_OK;
}


int tf_silent_stepper_v2_register_all_data_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2_AllDataHandler handler, void *user_data) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    silent_stepper_v2->all_data_handler = handler;
    silent_stepper_v2->all_data_user_data = user_data;

    return TF_E_OK;
}


int tf_silent_stepper_v2_register_new_state_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2_NewStateHandler handler, void *user_data) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    silent_stepper_v2->new_state_handler = handler;
    silent_stepper_v2->new_state_user_data = user_data;

    return TF_E_OK;
}


int tf_silent_stepper_v2_register_gpio_state_callback(TF_SilentStepperV2 *silent_stepper_v2, TF_SilentStepperV2_GPIOStateHandler handler, void *user_data) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    silent_stepper_v2->gpio_state_handler = handler;
    silent_stepper_v2->gpio_state_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_silent_stepper_v2_callback_tick(TF_SilentStepperV2 *silent_stepper_v2, uint32_t timeout_us) {
    if (silent_stepper_v2 == NULL) {
        return TF_E_NULL;
    }

    if (silent_stepper_v2->magic != 0x5446 || silent_stepper_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = silent_stepper_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(silent_stepper_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
