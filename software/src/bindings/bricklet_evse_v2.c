/* ***********************************************************
 * This file was automatically generated on 2024-12-04.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_evse_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_evse_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_EVSEV2 *evse_v2 = (TF_EVSEV2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(evse_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_EVSE_V2_CALLBACK_ENERGY_METER_VALUES: {
            TF_EVSEV2_EnergyMeterValuesHandler fn = evse_v2->energy_meter_values_handler;
            void *user_data = evse_v2->energy_meter_values_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            float power = tf_packet_buffer_read_float(payload);
            float current[3]; for (_i = 0; _i < 3; ++_i) current[_i] = tf_packet_buffer_read_float(payload);
            bool phases_active[3]; tf_packet_buffer_read_bool_array(payload, phases_active, 3);
            bool phases_connected[3]; tf_packet_buffer_read_bool_array(payload, phases_connected, 3);
            hal_common->locked = true;
            fn(evse_v2, power, current, phases_active, phases_connected, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_evse_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_evse_v2_create(TF_EVSEV2 *evse_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (evse_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(evse_v2, 0, sizeof(TF_EVSEV2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_EVSE_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    evse_v2->tfp = tfp;
    evse_v2->tfp->device = evse_v2;
    evse_v2->tfp->cb_handler = tf_evse_v2_callback_handler;
    evse_v2->magic = 0x5446;
    evse_v2->response_expected[0] = 0x40;
    evse_v2->response_expected[1] = 0x08;
    evse_v2->response_expected[2] = 0x00;
    return TF_E_OK;
}

int tf_evse_v2_destroy(TF_EVSEV2 *evse_v2) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }
    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    evse_v2->tfp->cb_handler = NULL;
    evse_v2->tfp->device = NULL;
    evse_v2->tfp = NULL;
    evse_v2->magic = 0;

    return TF_E_OK;
}

int tf_evse_v2_get_response_expected(TF_EVSEV2 *evse_v2, uint8_t function_id, bool *ret_response_expected) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_ACTIVE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_DEFAULT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_RESET_DC_FAULT_CURRENT_STATE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_GPIO_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_DATA_STORAGE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_BUTTON_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_EV_WAKEUP:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_FACTORY_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_BOOST_MODE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 4)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_GP_OUTPUT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 5)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_PHASE_CONTROL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 6)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_PHASE_AUTO_SWITCH:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 7)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_PHASES_CONNECTED:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[2] & (1 << 0)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[2] & (1 << 1)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[2] & (1 << 2)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[2] & (1 << 3)) != 0;
            }
            break;
        case TF_EVSE_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse_v2->response_expected[2] & (1 << 4)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_evse_v2_set_response_expected(TF_EVSEV2 *evse_v2, uint8_t function_id, bool response_expected) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 0);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 1);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_ACTIVE:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 2);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 3);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_DEFAULT:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 4);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_EVSE_V2_FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 5);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_EVSE_V2_FUNCTION_RESET_DC_FAULT_CURRENT_STATE:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 6);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_GPIO_CONFIGURATION:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 7);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_DATA_STORAGE:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 0);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_BUTTON_CONFIGURATION:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 1);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_EV_WAKEUP:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 2);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_EVSE_V2_FUNCTION_FACTORY_RESET:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 3);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 3);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_BOOST_MODE:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 4);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 4);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_GP_OUTPUT:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 5);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 5);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_PHASE_CONTROL:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 6);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 6);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_PHASE_AUTO_SWITCH:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 7);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 7);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_PHASES_CONNECTED:
            if (response_expected) {
                evse_v2->response_expected[2] |= (1 << 0);
            } else {
                evse_v2->response_expected[2] &= ~(1 << 0);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                evse_v2->response_expected[2] |= (1 << 1);
            } else {
                evse_v2->response_expected[2] &= ~(1 << 1);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                evse_v2->response_expected[2] |= (1 << 2);
            } else {
                evse_v2->response_expected[2] &= ~(1 << 2);
            }
            break;
        case TF_EVSE_V2_FUNCTION_RESET:
            if (response_expected) {
                evse_v2->response_expected[2] |= (1 << 3);
            } else {
                evse_v2->response_expected[2] &= ~(1 << 3);
            }
            break;
        case TF_EVSE_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                evse_v2->response_expected[2] |= (1 << 4);
            } else {
                evse_v2->response_expected[2] &= ~(1 << 4);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_evse_v2_set_response_expected_all(TF_EVSEV2 *evse_v2, bool response_expected) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(evse_v2->response_expected, response_expected ? 0xFF : 0, 3);

    return TF_E_OK;
}

int tf_evse_v2_get_state(TF_EVSEV2 *evse_v2, uint8_t *ret_iec61851_state, uint8_t *ret_charger_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint8_t *ret_dc_fault_current_state) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 9) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_iec61851_state != NULL) { *ret_iec61851_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_charger_state != NULL) { *ret_charger_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_contactor_state != NULL) { *ret_contactor_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_contactor_error != NULL) { *ret_contactor_error = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_allowed_charging_current != NULL) { *ret_allowed_charging_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_error_state != NULL) { *ret_error_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_lock_state != NULL) { *ret_lock_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_dc_fault_current_state != NULL) { *ret_dc_fault_current_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 9) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_hardware_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch, uint8_t *ret_evse_version, uint8_t *ret_energy_meter_type) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_HARDWARE_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_jumper_configuration != NULL) { *ret_jumper_configuration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_has_lock_switch != NULL) { *ret_has_lock_switch = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_evse_version != NULL) { *ret_evse_version = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_energy_meter_type != NULL) { *ret_energy_meter_type = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_low_level_state(TF_EVSEV2 *evse_v2, uint8_t *ret_led_state, uint16_t *ret_cp_pwm_duty_cycle, uint16_t ret_adc_values[7], int16_t ret_voltages[7], uint32_t ret_resistances[2], bool ret_gpio[24], bool *ret_car_stopped_charging, uint32_t *ret_time_since_state_change, uint32_t *ret_time_since_dc_fault_check, uint32_t *ret_uptime) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_LOW_LEVEL_STATE, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 55) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_led_state != NULL) { *ret_led_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_cp_pwm_duty_cycle != NULL) { *ret_cp_pwm_duty_cycle = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_adc_values != NULL) { for (_i = 0; _i < 7; ++_i) ret_adc_values[_i] = tf_packet_buffer_read_uint16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 14); }
            if (ret_voltages != NULL) { for (_i = 0; _i < 7; ++_i) ret_voltages[_i] = tf_packet_buffer_read_int16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 14); }
            if (ret_resistances != NULL) { for (_i = 0; _i < 2; ++_i) ret_resistances[_i] = tf_packet_buffer_read_uint32_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 8); }
            if (ret_gpio != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_gpio, 24);} else { tf_packet_buffer_remove(_recv_buf, 3); }
            if (ret_car_stopped_charging != NULL) { *ret_car_stopped_charging = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_time_since_state_change != NULL) { *ret_time_since_state_change = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_time_since_dc_fault_check != NULL) { *ret_time_since_dc_fault_check = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_uptime != NULL) { *ret_uptime = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 55) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_charging_slot(TF_EVSEV2 *evse_v2, uint8_t slot, uint16_t max_current, bool active, bool clear_on_disconnect) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)slot;
    max_current = tf_leconvert_uint16_to(max_current); memcpy(_send_buf + 1, &max_current, 2);
    _send_buf[3] = active ? 1 : 0;
    _send_buf[4] = clear_on_disconnect ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_charging_slot_max_current(TF_EVSEV2 *evse_v2, uint8_t slot, uint16_t max_current) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)slot;
    max_current = tf_leconvert_uint16_to(max_current); memcpy(_send_buf + 1, &max_current, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_charging_slot_active(TF_EVSEV2 *evse_v2, uint8_t slot, bool active) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_ACTIVE, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_ACTIVE, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)slot;
    _send_buf[1] = active ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_charging_slot_clear_on_disconnect(TF_EVSEV2 *evse_v2, uint8_t slot, bool clear_on_disconnect) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)slot;
    _send_buf[1] = clear_on_disconnect ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_charging_slot(TF_EVSEV2 *evse_v2, uint8_t slot, uint16_t *ret_max_current, bool *ret_active, bool *ret_clear_on_disconnect) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_CHARGING_SLOT, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)slot;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_max_current != NULL) { *ret_max_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_active != NULL) { *ret_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_clear_on_disconnect != NULL) { *ret_clear_on_disconnect = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_all_charging_slots(TF_EVSEV2 *evse_v2, uint16_t ret_max_current[20], uint8_t ret_active_and_clear_on_disconnect[20]) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_ALL_CHARGING_SLOTS, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 60) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_max_current != NULL) { for (_i = 0; _i < 20; ++_i) ret_max_current[_i] = tf_packet_buffer_read_uint16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 40); }
            if (ret_active_and_clear_on_disconnect != NULL) { for (_i = 0; _i < 20; ++_i) ret_active_and_clear_on_disconnect[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 20); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 60) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_charging_slot_default(TF_EVSEV2 *evse_v2, uint8_t slot, uint16_t max_current, bool active, bool clear_on_disconnect) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_DEFAULT, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_CHARGING_SLOT_DEFAULT, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)slot;
    max_current = tf_leconvert_uint16_to(max_current); memcpy(_send_buf + 1, &max_current, 2);
    _send_buf[3] = active ? 1 : 0;
    _send_buf[4] = clear_on_disconnect ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_charging_slot_default(TF_EVSEV2 *evse_v2, uint8_t slot, uint16_t *ret_max_current, bool *ret_active, bool *ret_clear_on_disconnect) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_CHARGING_SLOT_DEFAULT, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)slot;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_max_current != NULL) { *ret_max_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_active != NULL) { *ret_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_clear_on_disconnect != NULL) { *ret_clear_on_disconnect = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_energy_meter_values(TF_EVSEV2 *evse_v2, float *ret_power, float ret_current[3], bool ret_phases_active[3], bool ret_phases_connected[3]) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_ENERGY_METER_VALUES, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 18) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_power != NULL) { *ret_power = tf_packet_buffer_read_float(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_current != NULL) { for (_i = 0; _i < 3; ++_i) ret_current[_i] = tf_packet_buffer_read_float(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 12); }
            if (ret_phases_active != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_phases_active, 3);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_phases_connected != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_phases_connected, 3);} else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 18) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_all_energy_meter_values_low_level(TF_EVSEV2 *evse_v2, uint16_t *ret_values_chunk_offset, float ret_values_chunk_data[15]) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_ALL_ENERGY_METER_VALUES_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 62) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_values_chunk_offset != NULL) { *ret_values_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_values_chunk_data != NULL) { for (_i = 0; _i < 15; ++_i) ret_values_chunk_data[_i] = tf_packet_buffer_read_float(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 62) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_energy_meter_errors(TF_EVSEV2 *evse_v2, uint32_t ret_error_count[6]) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_ENERGY_METER_ERRORS, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 24) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count != NULL) { for (_i = 0; _i < 6; ++_i) ret_error_count[_i] = tf_packet_buffer_read_uint32_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 24); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 24) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_reset_energy_meter_relative_energy(TF_EVSEV2 *evse_v2) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_reset_dc_fault_current_state(TF_EVSEV2 *evse_v2, uint32_t password) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_RESET_DC_FAULT_CURRENT_STATE, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_RESET_DC_FAULT_CURRENT_STATE, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    password = tf_leconvert_uint32_to(password); memcpy(_send_buf + 0, &password, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_gpio_configuration(TF_EVSEV2 *evse_v2, uint8_t shutdown_input_configuration, uint8_t input_configuration, uint8_t output_configuration) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_GPIO_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_GPIO_CONFIGURATION, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)shutdown_input_configuration;
    _send_buf[1] = (uint8_t)input_configuration;
    _send_buf[2] = (uint8_t)output_configuration;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_gpio_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_shutdown_input_configuration, uint8_t *ret_input_configuration, uint8_t *ret_output_configuration) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_GPIO_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 3) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_shutdown_input_configuration != NULL) { *ret_shutdown_input_configuration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_input_configuration != NULL) { *ret_input_configuration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_output_configuration != NULL) { *ret_output_configuration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 3) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_data_storage(TF_EVSEV2 *evse_v2, uint8_t page, uint8_t ret_data[63]) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_DATA_STORAGE, 1, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)page;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 63) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_data != NULL) { for (_i = 0; _i < 63; ++_i) ret_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 63); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 63) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_data_storage(TF_EVSEV2 *evse_v2, uint8_t page, const uint8_t data[63]) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_DATA_STORAGE, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_DATA_STORAGE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)page;
    memcpy(_send_buf + 1, data, 63);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_indicator_led(TF_EVSEV2 *evse_v2, int16_t *ret_indication, uint16_t *ret_duration, uint16_t *ret_color_h, uint8_t *ret_color_s, uint8_t *ret_color_v) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_INDICATOR_LED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_indication != NULL) { *ret_indication = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_duration != NULL) { *ret_duration = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_color_h != NULL) { *ret_color_h = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_color_s != NULL) { *ret_color_s = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_color_v != NULL) { *ret_color_v = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_indicator_led(TF_EVSEV2 *evse_v2, int16_t indication, uint16_t duration, uint16_t color_h, uint8_t color_s, uint8_t color_v, uint8_t *ret_status) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_INDICATOR_LED, 8, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    indication = tf_leconvert_int16_to(indication); memcpy(_send_buf + 0, &indication, 2);
    duration = tf_leconvert_uint16_to(duration); memcpy(_send_buf + 2, &duration, 2);
    color_h = tf_leconvert_uint16_to(color_h); memcpy(_send_buf + 4, &color_h, 2);
    _send_buf[6] = (uint8_t)color_s;
    _send_buf[7] = (uint8_t)color_v;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_button_configuration(TF_EVSEV2 *evse_v2, uint8_t button_configuration) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_BUTTON_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_BUTTON_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)button_configuration;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_button_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_button_configuration) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_BUTTON_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_button_configuration != NULL) { *ret_button_configuration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_button_state(TF_EVSEV2 *evse_v2, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_BUTTON_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 9) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_button_press_time != NULL) { *ret_button_press_time = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_button_release_time != NULL) { *ret_button_release_time = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_button_pressed != NULL) { *ret_button_pressed = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 9) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_ev_wakeup(TF_EVSEV2 *evse_v2, bool ev_wakeup_enabled) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_EV_WAKEUP, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_EV_WAKEUP, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = ev_wakeup_enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_ev_wakuep(TF_EVSEV2 *evse_v2, bool *ret_ev_wakeup_enabled) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_EV_WAKUEP, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_ev_wakeup_enabled != NULL) { *ret_ev_wakeup_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_control_pilot_disconnect(TF_EVSEV2 *evse_v2, bool control_pilot_disconnect, bool *ret_is_control_pilot_disconnect) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_CONTROL_PILOT_DISCONNECT, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = control_pilot_disconnect ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_is_control_pilot_disconnect != NULL) { *ret_is_control_pilot_disconnect = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_control_pilot_disconnect(TF_EVSEV2 *evse_v2, bool *ret_control_pilot_disconnect) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_CONTROL_PILOT_DISCONNECT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_control_pilot_disconnect != NULL) { *ret_control_pilot_disconnect = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_all_data_1(TF_EVSEV2 *evse_v2, uint8_t *ret_iec61851_state, uint8_t *ret_charger_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint8_t *ret_dc_fault_current_state, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch, uint8_t *ret_evse_version, uint8_t *ret_energy_meter_type, float *ret_power, float ret_current[3], bool ret_phases_active[3], bool ret_phases_connected[3], uint32_t ret_error_count[6]) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_ALL_DATA_1, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 55) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_iec61851_state != NULL) { *ret_iec61851_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_charger_state != NULL) { *ret_charger_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_contactor_state != NULL) { *ret_contactor_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_contactor_error != NULL) { *ret_contactor_error = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_allowed_charging_current != NULL) { *ret_allowed_charging_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_error_state != NULL) { *ret_error_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_lock_state != NULL) { *ret_lock_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_dc_fault_current_state != NULL) { *ret_dc_fault_current_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_jumper_configuration != NULL) { *ret_jumper_configuration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_has_lock_switch != NULL) { *ret_has_lock_switch = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_evse_version != NULL) { *ret_evse_version = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_energy_meter_type != NULL) { *ret_energy_meter_type = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_power != NULL) { *ret_power = tf_packet_buffer_read_float(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_current != NULL) { for (_i = 0; _i < 3; ++_i) ret_current[_i] = tf_packet_buffer_read_float(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 12); }
            if (ret_phases_active != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_phases_active, 3);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_phases_connected != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_phases_connected, 3);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_error_count != NULL) { for (_i = 0; _i < 6; ++_i) ret_error_count[_i] = tf_packet_buffer_read_uint32_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 24); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 55) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_all_data_2(TF_EVSEV2 *evse_v2, uint8_t *ret_shutdown_input_configuration, uint8_t *ret_input_configuration, uint8_t *ret_output_configuration, int16_t *ret_indication, uint16_t *ret_duration, uint16_t *ret_color_h, uint8_t *ret_color_s, uint8_t *ret_color_v, uint8_t *ret_button_configuration, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed, bool *ret_ev_wakeup_enabled, bool *ret_control_pilot_disconnect, bool *ret_boost_mode_enabled, int16_t *ret_temperature, uint8_t *ret_phases_current, uint8_t *ret_phases_requested, uint8_t *ret_phases_state, uint8_t *ret_phases_info, bool *ret_phase_auto_switch_enabled, uint8_t *ret_phases_connected) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_ALL_DATA_2, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 32) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_shutdown_input_configuration != NULL) { *ret_shutdown_input_configuration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_input_configuration != NULL) { *ret_input_configuration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_output_configuration != NULL) { *ret_output_configuration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_indication != NULL) { *ret_indication = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_duration != NULL) { *ret_duration = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_color_h != NULL) { *ret_color_h = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_color_s != NULL) { *ret_color_s = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_color_v != NULL) { *ret_color_v = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_button_configuration != NULL) { *ret_button_configuration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_button_press_time != NULL) { *ret_button_press_time = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_button_release_time != NULL) { *ret_button_release_time = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_button_pressed != NULL) { *ret_button_pressed = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_ev_wakeup_enabled != NULL) { *ret_ev_wakeup_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_control_pilot_disconnect != NULL) { *ret_control_pilot_disconnect = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_boost_mode_enabled != NULL) { *ret_boost_mode_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_phases_current != NULL) { *ret_phases_current = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_phases_requested != NULL) { *ret_phases_requested = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_phases_state != NULL) { *ret_phases_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_phases_info != NULL) { *ret_phases_info = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_phase_auto_switch_enabled != NULL) { *ret_phase_auto_switch_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_phases_connected != NULL) { *ret_phases_connected = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 32) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_factory_reset(TF_EVSEV2 *evse_v2, uint32_t password) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_FACTORY_RESET, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_FACTORY_RESET, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    password = tf_leconvert_uint32_to(password); memcpy(_send_buf + 0, &password, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_button_press_boot_time(TF_EVSEV2 *evse_v2, bool reset, uint32_t *ret_button_press_boot_time) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_BUTTON_PRESS_BOOT_TIME, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = reset ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_button_press_boot_time != NULL) { *ret_button_press_boot_time = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_boost_mode(TF_EVSEV2 *evse_v2, bool boost_mode_enabled) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_BOOST_MODE, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_BOOST_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = boost_mode_enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_boost_mode(TF_EVSEV2 *evse_v2, bool *ret_boost_mode_enabled) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_BOOST_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_boost_mode_enabled != NULL) { *ret_boost_mode_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_trigger_dc_fault_test(TF_EVSEV2 *evse_v2, uint32_t password, bool *ret_started) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_TRIGGER_DC_FAULT_TEST, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    password = tf_leconvert_uint32_to(password); memcpy(_send_buf + 0, &password, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_started != NULL) { *ret_started = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_gp_output(TF_EVSEV2 *evse_v2, uint8_t gp_output) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_GP_OUTPUT, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_GP_OUTPUT, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)gp_output;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_temperature(TF_EVSEV2 *evse_v2, int16_t *ret_temperature) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_phase_control(TF_EVSEV2 *evse_v2, uint8_t phases) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_PHASE_CONTROL, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_PHASE_CONTROL, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)phases;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_phase_control(TF_EVSEV2 *evse_v2, uint8_t *ret_phases_current, uint8_t *ret_phases_requested, uint8_t *ret_phases_state, uint8_t *ret_phases_info) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_PHASE_CONTROL, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_phases_current != NULL) { *ret_phases_current = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_phases_requested != NULL) { *ret_phases_requested = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_phases_state != NULL) { *ret_phases_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_phases_info != NULL) { *ret_phases_info = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_phase_auto_switch(TF_EVSEV2 *evse_v2, bool phase_auto_switch_enabled) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_PHASE_AUTO_SWITCH, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_PHASE_AUTO_SWITCH, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = phase_auto_switch_enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_phase_auto_switch(TF_EVSEV2 *evse_v2, bool *ret_phase_auto_switch_enabled) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_PHASE_AUTO_SWITCH, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_phase_auto_switch_enabled != NULL) { *ret_phase_auto_switch_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_phases_connected(TF_EVSEV2 *evse_v2, uint8_t phases_connected) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_PHASES_CONNECTED, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_PHASES_CONNECTED, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)phases_connected;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_phases_connected(TF_EVSEV2 *evse_v2, uint8_t *ret_phases_connected) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_PHASES_CONNECTED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_phases_connected != NULL) { *ret_phases_connected = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_spitfp_error_count(TF_EVSEV2 *evse_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_bootloader_mode(TF_EVSEV2 *evse_v2, uint8_t mode, uint8_t *ret_status) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_bootloader_mode(TF_EVSEV2 *evse_v2, uint8_t *ret_mode) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_write_firmware_pointer(TF_EVSEV2 *evse_v2, uint32_t pointer) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_write_firmware(TF_EVSEV2 *evse_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_set_status_led_config(TF_EVSEV2 *evse_v2, uint8_t config) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_status_led_config(TF_EVSEV2 *evse_v2, uint8_t *ret_config) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_chip_temperature(TF_EVSEV2 *evse_v2, int16_t *ret_temperature) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_reset(TF_EVSEV2 *evse_v2) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_write_uid(TF_EVSEV2 *evse_v2, uint32_t uid) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_read_uid(TF_EVSEV2 *evse_v2, uint32_t *ret_uid) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_v2_get_identity(TF_EVSEV2 *evse_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse_v2->tfp);
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
        tf_tfp_packet_processed(evse_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

static int tf_evse_v2_get_all_energy_meter_values_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    (void)wrapper_data;
    uint16_t values_length = 88;
    uint16_t values_chunk_offset = 0;
    float *values_chunk_data = (float *) chunk_data;
    int ret = tf_evse_v2_get_all_energy_meter_values_low_level((TF_EVSEV2 *)device, &values_chunk_offset, values_chunk_data);

    if (values_chunk_offset == (1 << 16) - 1) { // maximum chunk offset -> stream has no data
        return TF_E_INTERNAL_STREAM_HAS_NO_DATA;
    }

    *ret_stream_length = (uint32_t)values_length;
    *ret_chunk_offset = (uint32_t)values_chunk_offset;
    return ret;
}

int tf_evse_v2_get_all_energy_meter_values(TF_EVSEV2 *evse_v2, float *ret_values, uint16_t *ret_values_length) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    uint32_t _values_length = 0;
    float _values_chunk_data[15];

    int ret = tf_stream_out(evse_v2, tf_evse_v2_get_all_energy_meter_values_ll_wrapper, NULL, ret_values, &_values_length, _values_chunk_data, 15, tf_copy_items_float);

    if (ret_values_length != NULL) {
        *ret_values_length = (uint16_t)_values_length;
    }
    return ret;
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_evse_v2_register_energy_meter_values_callback(TF_EVSEV2 *evse_v2, TF_EVSEV2_EnergyMeterValuesHandler handler, void *user_data) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    evse_v2->energy_meter_values_handler = handler;
    evse_v2->energy_meter_values_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_evse_v2_callback_tick(TF_EVSEV2 *evse_v2, uint32_t timeout_us) {
    if (evse_v2 == NULL) {
        return TF_E_NULL;
    }

    if (evse_v2->magic != 0x5446 || evse_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = evse_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(evse_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
