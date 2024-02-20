/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_performance_dc.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_performance_dc_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_PerformanceDC *performance_dc = (TF_PerformanceDC *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(performance_dc->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_PERFORMANCE_DC_CALLBACK_EMERGENCY_SHUTDOWN: {
            TF_PerformanceDC_EmergencyShutdownHandler fn = performance_dc->emergency_shutdown_handler;
            void *user_data = performance_dc->emergency_shutdown_user_data;
            if (fn == NULL) {
                return false;
            }


            hal_common->locked = true;
            fn(performance_dc, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_PERFORMANCE_DC_CALLBACK_VELOCITY_REACHED: {
            TF_PerformanceDC_VelocityReachedHandler fn = performance_dc->velocity_reached_handler;
            void *user_data = performance_dc->velocity_reached_user_data;
            if (fn == NULL) {
                return false;
            }

            int16_t velocity = tf_packet_buffer_read_int16_t(payload);
            hal_common->locked = true;
            fn(performance_dc, velocity, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_PERFORMANCE_DC_CALLBACK_CURRENT_VELOCITY: {
            TF_PerformanceDC_CurrentVelocityHandler fn = performance_dc->current_velocity_handler;
            void *user_data = performance_dc->current_velocity_user_data;
            if (fn == NULL) {
                return false;
            }

            int16_t velocity = tf_packet_buffer_read_int16_t(payload);
            hal_common->locked = true;
            fn(performance_dc, velocity, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_PERFORMANCE_DC_CALLBACK_GPIO_STATE: {
            TF_PerformanceDC_GPIOStateHandler fn = performance_dc->gpio_state_handler;
            void *user_data = performance_dc->gpio_state_user_data;
            if (fn == NULL) {
                return false;
            }

            bool gpio_state[2]; tf_packet_buffer_read_bool_array(payload, gpio_state, 2);
            hal_common->locked = true;
            fn(performance_dc, gpio_state, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_performance_dc_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_performance_dc_create(TF_PerformanceDC *performance_dc, const char *uid_or_port_name, TF_HAL *hal) {
    if (performance_dc == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(performance_dc, 0, sizeof(TF_PerformanceDC));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_PERFORMANCE_DC_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    performance_dc->tfp = tfp;
    performance_dc->tfp->device = performance_dc;
    performance_dc->tfp->cb_handler = tf_performance_dc_callback_handler;
    performance_dc->magic = 0x5446;
    performance_dc->response_expected[0] = 0x00;
    performance_dc->response_expected[1] = 0xE0;
    performance_dc->response_expected[2] = 0x00;
    return TF_E_OK;
}

int tf_performance_dc_destroy(TF_PerformanceDC *performance_dc) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }
    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    performance_dc->tfp->cb_handler = NULL;
    performance_dc->tfp->device = NULL;
    performance_dc->tfp = NULL;
    performance_dc->magic = 0;

    return TF_E_OK;
}

int tf_performance_dc_get_response_expected(TF_PerformanceDC *performance_dc, uint8_t function_id, bool *ret_response_expected) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_PERFORMANCE_DC_FUNCTION_SET_ENABLED:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_VELOCITY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_MOTION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_FULL_BRAKE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_DRIVE_MODE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_PWM_FREQUENCY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_THERMAL_SHUTDOWN:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_GPIO_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_GPIO_ACTION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_ERROR_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_CW_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_CCW_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_GPIO_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[1] & (1 << 4)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_EMERGENCY_SHUTDOWN_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[1] & (1 << 5)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_VELOCITY_REACHED_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[1] & (1 << 6)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_CURRENT_VELOCITY_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[1] & (1 << 7)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[2] & (1 << 0)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[2] & (1 << 1)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[2] & (1 << 2)) != 0;
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (performance_dc->response_expected[2] & (1 << 3)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_performance_dc_set_response_expected(TF_PerformanceDC *performance_dc, uint8_t function_id, bool response_expected) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_PERFORMANCE_DC_FUNCTION_SET_ENABLED:
            if (response_expected) {
                performance_dc->response_expected[0] |= (1 << 0);
            } else {
                performance_dc->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_VELOCITY:
            if (response_expected) {
                performance_dc->response_expected[0] |= (1 << 1);
            } else {
                performance_dc->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_MOTION:
            if (response_expected) {
                performance_dc->response_expected[0] |= (1 << 2);
            } else {
                performance_dc->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_FULL_BRAKE:
            if (response_expected) {
                performance_dc->response_expected[0] |= (1 << 3);
            } else {
                performance_dc->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_DRIVE_MODE:
            if (response_expected) {
                performance_dc->response_expected[0] |= (1 << 4);
            } else {
                performance_dc->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_PWM_FREQUENCY:
            if (response_expected) {
                performance_dc->response_expected[0] |= (1 << 5);
            } else {
                performance_dc->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_THERMAL_SHUTDOWN:
            if (response_expected) {
                performance_dc->response_expected[0] |= (1 << 6);
            } else {
                performance_dc->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_GPIO_CONFIGURATION:
            if (response_expected) {
                performance_dc->response_expected[0] |= (1 << 7);
            } else {
                performance_dc->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_GPIO_ACTION:
            if (response_expected) {
                performance_dc->response_expected[1] |= (1 << 0);
            } else {
                performance_dc->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_ERROR_LED_CONFIG:
            if (response_expected) {
                performance_dc->response_expected[1] |= (1 << 1);
            } else {
                performance_dc->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_CW_LED_CONFIG:
            if (response_expected) {
                performance_dc->response_expected[1] |= (1 << 2);
            } else {
                performance_dc->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_CCW_LED_CONFIG:
            if (response_expected) {
                performance_dc->response_expected[1] |= (1 << 3);
            } else {
                performance_dc->response_expected[1] &= ~(1 << 3);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_GPIO_LED_CONFIG:
            if (response_expected) {
                performance_dc->response_expected[1] |= (1 << 4);
            } else {
                performance_dc->response_expected[1] &= ~(1 << 4);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_EMERGENCY_SHUTDOWN_CALLBACK_CONFIGURATION:
            if (response_expected) {
                performance_dc->response_expected[1] |= (1 << 5);
            } else {
                performance_dc->response_expected[1] &= ~(1 << 5);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_VELOCITY_REACHED_CALLBACK_CONFIGURATION:
            if (response_expected) {
                performance_dc->response_expected[1] |= (1 << 6);
            } else {
                performance_dc->response_expected[1] &= ~(1 << 6);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_CURRENT_VELOCITY_CALLBACK_CONFIGURATION:
            if (response_expected) {
                performance_dc->response_expected[1] |= (1 << 7);
            } else {
                performance_dc->response_expected[1] &= ~(1 << 7);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                performance_dc->response_expected[2] |= (1 << 0);
            } else {
                performance_dc->response_expected[2] &= ~(1 << 0);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                performance_dc->response_expected[2] |= (1 << 1);
            } else {
                performance_dc->response_expected[2] &= ~(1 << 1);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_RESET:
            if (response_expected) {
                performance_dc->response_expected[2] |= (1 << 2);
            } else {
                performance_dc->response_expected[2] &= ~(1 << 2);
            }
            break;
        case TF_PERFORMANCE_DC_FUNCTION_WRITE_UID:
            if (response_expected) {
                performance_dc->response_expected[2] |= (1 << 3);
            } else {
                performance_dc->response_expected[2] &= ~(1 << 3);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_performance_dc_set_response_expected_all(TF_PerformanceDC *performance_dc, bool response_expected) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(performance_dc->response_expected, response_expected ? 0xFF : 0, 3);

    return TF_E_OK;
}

int tf_performance_dc_set_enabled(TF_PerformanceDC *performance_dc, bool enabled) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_ENABLED, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_ENABLED, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_enabled(TF_PerformanceDC *performance_dc, bool *ret_enabled) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_ENABLED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_velocity(TF_PerformanceDC *performance_dc, int16_t velocity) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_VELOCITY, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_VELOCITY, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    velocity = tf_leconvert_int16_to(velocity); memcpy(_send_buf + 0, &velocity, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_velocity(TF_PerformanceDC *performance_dc, int16_t *ret_velocity) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_VELOCITY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_velocity != NULL) { *ret_velocity = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_current_velocity(TF_PerformanceDC *performance_dc, int16_t *ret_velocity) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_CURRENT_VELOCITY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_velocity != NULL) { *ret_velocity = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_motion(TF_PerformanceDC *performance_dc, uint16_t acceleration, uint16_t deceleration) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_MOTION, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_MOTION, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    acceleration = tf_leconvert_uint16_to(acceleration); memcpy(_send_buf + 0, &acceleration, 2);
    deceleration = tf_leconvert_uint16_to(deceleration); memcpy(_send_buf + 2, &deceleration, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_motion(TF_PerformanceDC *performance_dc, uint16_t *ret_acceleration, uint16_t *ret_deceleration) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_MOTION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_acceleration != NULL) { *ret_acceleration = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_deceleration != NULL) { *ret_deceleration = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_full_brake(TF_PerformanceDC *performance_dc) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_FULL_BRAKE, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_FULL_BRAKE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_drive_mode(TF_PerformanceDC *performance_dc, uint8_t mode) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_DRIVE_MODE, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_DRIVE_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_drive_mode(TF_PerformanceDC *performance_dc, uint8_t *ret_mode) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_DRIVE_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_pwm_frequency(TF_PerformanceDC *performance_dc, uint16_t frequency) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_PWM_FREQUENCY, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_PWM_FREQUENCY, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    frequency = tf_leconvert_uint16_to(frequency); memcpy(_send_buf + 0, &frequency, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_pwm_frequency(TF_PerformanceDC *performance_dc, uint16_t *ret_frequency) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_PWM_FREQUENCY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_frequency != NULL) { *ret_frequency = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_power_statistics(TF_PerformanceDC *performance_dc, uint16_t *ret_voltage, uint16_t *ret_current, int16_t *ret_temperature) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_POWER_STATISTICS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_voltage != NULL) { *ret_voltage = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_current != NULL) { *ret_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_thermal_shutdown(TF_PerformanceDC *performance_dc, uint8_t temperature) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_THERMAL_SHUTDOWN, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_THERMAL_SHUTDOWN, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)temperature;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_thermal_shutdown(TF_PerformanceDC *performance_dc, uint8_t *ret_temperature) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_THERMAL_SHUTDOWN, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_gpio_configuration(TF_PerformanceDC *performance_dc, uint8_t channel, uint16_t debounce, uint16_t stop_deceleration) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_GPIO_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_GPIO_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)channel;
    debounce = tf_leconvert_uint16_to(debounce); memcpy(_send_buf + 1, &debounce, 2);
    stop_deceleration = tf_leconvert_uint16_to(stop_deceleration); memcpy(_send_buf + 3, &stop_deceleration, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_gpio_configuration(TF_PerformanceDC *performance_dc, uint8_t channel, uint16_t *ret_debounce, uint16_t *ret_stop_deceleration) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_GPIO_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_debounce != NULL) { *ret_debounce = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_stop_deceleration != NULL) { *ret_stop_deceleration = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_gpio_action(TF_PerformanceDC *performance_dc, uint8_t channel, uint32_t action) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_GPIO_ACTION, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_GPIO_ACTION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)channel;
    action = tf_leconvert_uint32_to(action); memcpy(_send_buf + 1, &action, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_gpio_action(TF_PerformanceDC *performance_dc, uint8_t channel, uint32_t *ret_action) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_GPIO_ACTION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_action != NULL) { *ret_action = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_gpio_state(TF_PerformanceDC *performance_dc, bool ret_gpio_state[2]) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_GPIO_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_gpio_state != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_gpio_state, 2);} else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_error_led_config(TF_PerformanceDC *performance_dc, uint8_t config) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_ERROR_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_ERROR_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_error_led_config(TF_PerformanceDC *performance_dc, uint8_t *ret_config) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_ERROR_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_cw_led_config(TF_PerformanceDC *performance_dc, uint8_t config) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_CW_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_CW_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_cw_led_config(TF_PerformanceDC *performance_dc, uint8_t *ret_config) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_CW_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_ccw_led_config(TF_PerformanceDC *performance_dc, uint8_t config) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_CCW_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_CCW_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_ccw_led_config(TF_PerformanceDC *performance_dc, uint8_t *ret_config) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_CCW_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_gpio_led_config(TF_PerformanceDC *performance_dc, uint8_t channel, uint8_t config) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_GPIO_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_GPIO_LED_CONFIG, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_gpio_led_config(TF_PerformanceDC *performance_dc, uint8_t channel, uint8_t *ret_config) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_GPIO_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_emergency_shutdown_callback_configuration(TF_PerformanceDC *performance_dc, bool enabled) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_EMERGENCY_SHUTDOWN_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_EMERGENCY_SHUTDOWN_CALLBACK_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_emergency_shutdown_callback_configuration(TF_PerformanceDC *performance_dc, bool *ret_enabled) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_EMERGENCY_SHUTDOWN_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_velocity_reached_callback_configuration(TF_PerformanceDC *performance_dc, bool enabled) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_VELOCITY_REACHED_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_VELOCITY_REACHED_CALLBACK_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_velocity_reached_callback_configuration(TF_PerformanceDC *performance_dc, bool *ret_enabled) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_VELOCITY_REACHED_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_current_velocity_callback_configuration(TF_PerformanceDC *performance_dc, uint32_t period, bool value_has_to_change) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_CURRENT_VELOCITY_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_CURRENT_VELOCITY_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_current_velocity_callback_configuration(TF_PerformanceDC *performance_dc, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_CURRENT_VELOCITY_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_spitfp_error_count(TF_PerformanceDC *performance_dc, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_bootloader_mode(TF_PerformanceDC *performance_dc, uint8_t mode, uint8_t *ret_status) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_bootloader_mode(TF_PerformanceDC *performance_dc, uint8_t *ret_mode) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_write_firmware_pointer(TF_PerformanceDC *performance_dc, uint32_t pointer) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_write_firmware(TF_PerformanceDC *performance_dc, const uint8_t data[64], uint8_t *ret_status) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_set_status_led_config(TF_PerformanceDC *performance_dc, uint8_t config) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_status_led_config(TF_PerformanceDC *performance_dc, uint8_t *ret_config) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_chip_temperature(TF_PerformanceDC *performance_dc, int16_t *ret_temperature) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_reset(TF_PerformanceDC *performance_dc) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_write_uid(TF_PerformanceDC *performance_dc, uint32_t uid) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_performance_dc_get_response_expected(performance_dc, TF_PERFORMANCE_DC_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(performance_dc->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_read_uid(TF_PerformanceDC *performance_dc, uint32_t *ret_uid) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_performance_dc_get_identity(TF_PerformanceDC *performance_dc, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = performance_dc->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(performance_dc->tfp, TF_PERFORMANCE_DC_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(performance_dc->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(performance_dc->tfp);
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
        tf_tfp_packet_processed(performance_dc->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(performance_dc->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_performance_dc_register_emergency_shutdown_callback(TF_PerformanceDC *performance_dc, TF_PerformanceDC_EmergencyShutdownHandler handler, void *user_data) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    performance_dc->emergency_shutdown_handler = handler;
    performance_dc->emergency_shutdown_user_data = user_data;

    return TF_E_OK;
}


int tf_performance_dc_register_velocity_reached_callback(TF_PerformanceDC *performance_dc, TF_PerformanceDC_VelocityReachedHandler handler, void *user_data) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    performance_dc->velocity_reached_handler = handler;
    performance_dc->velocity_reached_user_data = user_data;

    return TF_E_OK;
}


int tf_performance_dc_register_current_velocity_callback(TF_PerformanceDC *performance_dc, TF_PerformanceDC_CurrentVelocityHandler handler, void *user_data) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    performance_dc->current_velocity_handler = handler;
    performance_dc->current_velocity_user_data = user_data;

    return TF_E_OK;
}


int tf_performance_dc_register_gpio_state_callback(TF_PerformanceDC *performance_dc, TF_PerformanceDC_GPIOStateHandler handler, void *user_data) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    performance_dc->gpio_state_handler = handler;
    performance_dc->gpio_state_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_performance_dc_callback_tick(TF_PerformanceDC *performance_dc, uint32_t timeout_us) {
    if (performance_dc == NULL) {
        return TF_E_NULL;
    }

    if (performance_dc->magic != 0x5446 || performance_dc->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = performance_dc->tfp->spitfp->hal;

    return tf_tfp_callback_tick(performance_dc->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
