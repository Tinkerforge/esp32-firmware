/* ***********************************************************
 * This file was automatically generated on 2024-12-04.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_evse.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


static bool tf_evse_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    (void)device;
    (void)fid;
    (void)payload;

    return false;
}
int tf_evse_create(TF_EVSE *evse, const char *uid_or_port_name, TF_HAL *hal) {
    if (evse == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(evse, 0, sizeof(TF_EVSE));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_EVSE_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    evse->tfp = tfp;
    evse->tfp->device = evse;
    evse->tfp->cb_handler = tf_evse_callback_handler;
    evse->magic = 0x5446;
    evse->response_expected[0] = 0xA0;
    evse->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_evse_destroy(TF_EVSE *evse) {
    if (evse == NULL) {
        return TF_E_NULL;
    }
    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    evse->tfp->cb_handler = NULL;
    evse->tfp->device = NULL;
    evse->tfp = NULL;
    evse->magic = 0;

    return TF_E_OK;
}

int tf_evse_get_response_expected(TF_EVSE *evse, uint8_t function_id, bool *ret_response_expected) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_EVSE_FUNCTION_SET_CHARGING_SLOT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_EVSE_FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_EVSE_FUNCTION_SET_CHARGING_SLOT_ACTIVE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_EVSE_FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_EVSE_FUNCTION_SET_CHARGING_SLOT_DEFAULT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_EVSE_FUNCTION_SET_USER_CALIBRATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_EVSE_FUNCTION_SET_DATA_STORAGE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_EVSE_FUNCTION_FACTORY_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_EVSE_FUNCTION_SET_BOOST_MODE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_EVSE_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_EVSE_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_EVSE_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        case TF_EVSE_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (evse->response_expected[1] & (1 << 4)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_evse_set_response_expected(TF_EVSE *evse, uint8_t function_id, bool response_expected) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_EVSE_FUNCTION_SET_CHARGING_SLOT:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 0);
            } else {
                evse->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_EVSE_FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 1);
            } else {
                evse->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_EVSE_FUNCTION_SET_CHARGING_SLOT_ACTIVE:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 2);
            } else {
                evse->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_EVSE_FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 3);
            } else {
                evse->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_EVSE_FUNCTION_SET_CHARGING_SLOT_DEFAULT:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 4);
            } else {
                evse->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_EVSE_FUNCTION_SET_USER_CALIBRATION:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 5);
            } else {
                evse->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_EVSE_FUNCTION_SET_DATA_STORAGE:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 6);
            } else {
                evse->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_EVSE_FUNCTION_FACTORY_RESET:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 7);
            } else {
                evse->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_EVSE_FUNCTION_SET_BOOST_MODE:
            if (response_expected) {
                evse->response_expected[1] |= (1 << 0);
            } else {
                evse->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_EVSE_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                evse->response_expected[1] |= (1 << 1);
            } else {
                evse->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_EVSE_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                evse->response_expected[1] |= (1 << 2);
            } else {
                evse->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_EVSE_FUNCTION_RESET:
            if (response_expected) {
                evse->response_expected[1] |= (1 << 3);
            } else {
                evse->response_expected[1] &= ~(1 << 3);
            }
            break;
        case TF_EVSE_FUNCTION_WRITE_UID:
            if (response_expected) {
                evse->response_expected[1] |= (1 << 4);
            } else {
                evse->response_expected[1] &= ~(1 << 4);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_evse_set_response_expected_all(TF_EVSE *evse, bool response_expected) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(evse->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_evse_get_state(TF_EVSE *evse, uint8_t *ret_iec61851_state, uint8_t *ret_charger_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_iec61851_state != NULL) { *ret_iec61851_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_charger_state != NULL) { *ret_charger_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_contactor_state != NULL) { *ret_contactor_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_contactor_error != NULL) { *ret_contactor_error = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_allowed_charging_current != NULL) { *ret_allowed_charging_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_error_state != NULL) { *ret_error_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_lock_state != NULL) { *ret_lock_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_hardware_configuration(TF_EVSE *evse, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch, uint8_t *ret_evse_version) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_HARDWARE_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 3) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_jumper_configuration != NULL) { *ret_jumper_configuration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_has_lock_switch != NULL) { *ret_has_lock_switch = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_evse_version != NULL) { *ret_evse_version = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 3) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_low_level_state(TF_EVSE *evse, uint8_t *ret_led_state, uint16_t *ret_cp_pwm_duty_cycle, uint16_t ret_adc_values[2], int16_t ret_voltages[3], uint32_t ret_resistances[2], bool ret_gpio[5], bool *ret_car_stopped_charging, uint32_t *ret_time_since_state_change, uint32_t *ret_uptime) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_LOW_LEVEL_STATE, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 31) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_led_state != NULL) { *ret_led_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_cp_pwm_duty_cycle != NULL) { *ret_cp_pwm_duty_cycle = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_adc_values != NULL) { for (_i = 0; _i < 2; ++_i) ret_adc_values[_i] = tf_packet_buffer_read_uint16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_voltages != NULL) { for (_i = 0; _i < 3; ++_i) ret_voltages[_i] = tf_packet_buffer_read_int16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 6); }
            if (ret_resistances != NULL) { for (_i = 0; _i < 2; ++_i) ret_resistances[_i] = tf_packet_buffer_read_uint32_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 8); }
            if (ret_gpio != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_gpio, 5);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_car_stopped_charging != NULL) { *ret_car_stopped_charging = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_time_since_state_change != NULL) { *ret_time_since_state_change = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_uptime != NULL) { *ret_uptime = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 31) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_set_charging_slot(TF_EVSE *evse, uint8_t slot, uint16_t max_current, bool active, bool clear_on_disconnect) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_CHARGING_SLOT, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_CHARGING_SLOT, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = (uint8_t)slot;
    max_current = tf_leconvert_uint16_to(max_current); memcpy(_send_buf + 1, &max_current, 2);
    _send_buf[3] = active ? 1 : 0;
    _send_buf[4] = clear_on_disconnect ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_set_charging_slot_max_current(TF_EVSE *evse, uint8_t slot, uint16_t max_current) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = (uint8_t)slot;
    max_current = tf_leconvert_uint16_to(max_current); memcpy(_send_buf + 1, &max_current, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_set_charging_slot_active(TF_EVSE *evse, uint8_t slot, bool active) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_CHARGING_SLOT_ACTIVE, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_CHARGING_SLOT_ACTIVE, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = (uint8_t)slot;
    _send_buf[1] = active ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_set_charging_slot_clear_on_disconnect(TF_EVSE *evse, uint8_t slot, bool clear_on_disconnect) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = (uint8_t)slot;
    _send_buf[1] = clear_on_disconnect ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_charging_slot(TF_EVSE *evse, uint8_t slot, uint16_t *ret_max_current, bool *ret_active, bool *ret_clear_on_disconnect) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_CHARGING_SLOT, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = (uint8_t)slot;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_max_current != NULL) { *ret_max_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_active != NULL) { *ret_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_clear_on_disconnect != NULL) { *ret_clear_on_disconnect = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_all_charging_slots(TF_EVSE *evse, uint16_t ret_max_current[20], uint8_t ret_active_and_clear_on_disconnect[20]) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_ALL_CHARGING_SLOTS, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 60) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_max_current != NULL) { for (_i = 0; _i < 20; ++_i) ret_max_current[_i] = tf_packet_buffer_read_uint16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 40); }
            if (ret_active_and_clear_on_disconnect != NULL) { for (_i = 0; _i < 20; ++_i) ret_active_and_clear_on_disconnect[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 20); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 60) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_set_charging_slot_default(TF_EVSE *evse, uint8_t slot, uint16_t max_current, bool active, bool clear_on_disconnect) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_CHARGING_SLOT_DEFAULT, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_CHARGING_SLOT_DEFAULT, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = (uint8_t)slot;
    max_current = tf_leconvert_uint16_to(max_current); memcpy(_send_buf + 1, &max_current, 2);
    _send_buf[3] = active ? 1 : 0;
    _send_buf[4] = clear_on_disconnect ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_charging_slot_default(TF_EVSE *evse, uint8_t slot, uint16_t *ret_max_current, bool *ret_active, bool *ret_clear_on_disconnect) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_CHARGING_SLOT_DEFAULT, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = (uint8_t)slot;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_max_current != NULL) { *ret_max_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_active != NULL) { *ret_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_clear_on_disconnect != NULL) { *ret_clear_on_disconnect = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_calibrate(TF_EVSE *evse, uint8_t state, uint32_t password, int32_t value, bool *ret_success) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_CALIBRATE, 9, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = (uint8_t)state;
    password = tf_leconvert_uint32_to(password); memcpy(_send_buf + 1, &password, 4);
    value = tf_leconvert_int32_to(value); memcpy(_send_buf + 5, &value, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_success != NULL) { *ret_success = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_user_calibration(TF_EVSE *evse, bool *ret_user_calibration_active, int16_t *ret_voltage_diff, int16_t *ret_voltage_mul, int16_t *ret_voltage_div, int16_t *ret_resistance_2700, int16_t ret_resistance_880[14]) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_USER_CALIBRATION, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 37) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_user_calibration_active != NULL) { *ret_user_calibration_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_voltage_diff != NULL) { *ret_voltage_diff = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_voltage_mul != NULL) { *ret_voltage_mul = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_voltage_div != NULL) { *ret_voltage_div = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_resistance_2700 != NULL) { *ret_resistance_2700 = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_resistance_880 != NULL) { for (_i = 0; _i < 14; ++_i) ret_resistance_880[_i] = tf_packet_buffer_read_int16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 28); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 37) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_set_user_calibration(TF_EVSE *evse, uint32_t password, bool user_calibration_active, int16_t voltage_diff, int16_t voltage_mul, int16_t voltage_div, int16_t resistance_2700, const int16_t resistance_880[14]) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_USER_CALIBRATION, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_USER_CALIBRATION, 41, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    password = tf_leconvert_uint32_to(password); memcpy(_send_buf + 0, &password, 4);
    _send_buf[4] = user_calibration_active ? 1 : 0;
    voltage_diff = tf_leconvert_int16_to(voltage_diff); memcpy(_send_buf + 5, &voltage_diff, 2);
    voltage_mul = tf_leconvert_int16_to(voltage_mul); memcpy(_send_buf + 7, &voltage_mul, 2);
    voltage_div = tf_leconvert_int16_to(voltage_div); memcpy(_send_buf + 9, &voltage_div, 2);
    resistance_2700 = tf_leconvert_int16_to(resistance_2700); memcpy(_send_buf + 11, &resistance_2700, 2);
    for (_i = 0; _i < 14; _i++) { int16_t tmp_resistance_880 = tf_leconvert_int16_to(resistance_880[_i]); memcpy(_send_buf + 13 + (_i * sizeof(int16_t)), &tmp_resistance_880, sizeof(int16_t)); }

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_data_storage(TF_EVSE *evse, uint8_t page, uint8_t ret_data[63]) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_DATA_STORAGE, 1, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = (uint8_t)page;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 63) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_data != NULL) { for (_i = 0; _i < 63; ++_i) ret_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 63); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 63) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_set_data_storage(TF_EVSE *evse, uint8_t page, const uint8_t data[63]) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_DATA_STORAGE, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_DATA_STORAGE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = (uint8_t)page;
    memcpy(_send_buf + 1, data, 63);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_indicator_led(TF_EVSE *evse, int16_t *ret_indication, uint16_t *ret_duration) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_INDICATOR_LED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_indication != NULL) { *ret_indication = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_duration != NULL) { *ret_duration = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_set_indicator_led(TF_EVSE *evse, int16_t indication, uint16_t duration, uint8_t *ret_status) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_INDICATOR_LED, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    indication = tf_leconvert_int16_to(indication); memcpy(_send_buf + 0, &indication, 2);
    duration = tf_leconvert_uint16_to(duration); memcpy(_send_buf + 2, &duration, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_button_state(TF_EVSE *evse, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_BUTTON_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 9) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_button_press_time != NULL) { *ret_button_press_time = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_button_release_time != NULL) { *ret_button_release_time = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_button_pressed != NULL) { *ret_button_pressed = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 9) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_all_data_1(TF_EVSE *evse, uint8_t *ret_iec61851_state, uint8_t *ret_charger_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch, uint8_t *ret_evse_version, uint8_t *ret_led_state, uint16_t *ret_cp_pwm_duty_cycle, uint16_t ret_adc_values[2], int16_t ret_voltages[3], uint32_t ret_resistances[2], bool ret_gpio[5], bool *ret_car_stopped_charging, uint32_t *ret_time_since_state_change, uint32_t *ret_uptime, int16_t *ret_indication, uint16_t *ret_duration, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed, bool *ret_boost_mode_enabled) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_ALL_DATA_1, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 56) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_iec61851_state != NULL) { *ret_iec61851_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_charger_state != NULL) { *ret_charger_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_contactor_state != NULL) { *ret_contactor_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_contactor_error != NULL) { *ret_contactor_error = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_allowed_charging_current != NULL) { *ret_allowed_charging_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_error_state != NULL) { *ret_error_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_lock_state != NULL) { *ret_lock_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_jumper_configuration != NULL) { *ret_jumper_configuration = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_has_lock_switch != NULL) { *ret_has_lock_switch = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_evse_version != NULL) { *ret_evse_version = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_led_state != NULL) { *ret_led_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_cp_pwm_duty_cycle != NULL) { *ret_cp_pwm_duty_cycle = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_adc_values != NULL) { for (_i = 0; _i < 2; ++_i) ret_adc_values[_i] = tf_packet_buffer_read_uint16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_voltages != NULL) { for (_i = 0; _i < 3; ++_i) ret_voltages[_i] = tf_packet_buffer_read_int16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 6); }
            if (ret_resistances != NULL) { for (_i = 0; _i < 2; ++_i) ret_resistances[_i] = tf_packet_buffer_read_uint32_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 8); }
            if (ret_gpio != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_gpio, 5);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_car_stopped_charging != NULL) { *ret_car_stopped_charging = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_time_since_state_change != NULL) { *ret_time_since_state_change = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_uptime != NULL) { *ret_uptime = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_indication != NULL) { *ret_indication = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_duration != NULL) { *ret_duration = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_button_press_time != NULL) { *ret_button_press_time = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_button_release_time != NULL) { *ret_button_release_time = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_button_pressed != NULL) { *ret_button_pressed = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_boost_mode_enabled != NULL) { *ret_boost_mode_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 56) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_factory_reset(TF_EVSE *evse, uint32_t password) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_FACTORY_RESET, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_FACTORY_RESET, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    password = tf_leconvert_uint32_to(password); memcpy(_send_buf + 0, &password, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_set_boost_mode(TF_EVSE *evse, bool boost_mode_enabled) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_BOOST_MODE, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_BOOST_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = boost_mode_enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_boost_mode(TF_EVSE *evse, bool *ret_boost_mode_enabled) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_BOOST_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_boost_mode_enabled != NULL) { *ret_boost_mode_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_spitfp_error_count(TF_EVSE *evse, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_set_bootloader_mode(TF_EVSE *evse, uint8_t mode, uint8_t *ret_status) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_bootloader_mode(TF_EVSE *evse, uint8_t *ret_mode) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_set_write_firmware_pointer(TF_EVSE *evse, uint32_t pointer) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_write_firmware(TF_EVSE *evse, const uint8_t data[64], uint8_t *ret_status) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_set_status_led_config(TF_EVSE *evse, uint8_t config) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_status_led_config(TF_EVSE *evse, uint8_t *ret_config) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_chip_temperature(TF_EVSE *evse, int16_t *ret_temperature) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_reset(TF_EVSE *evse) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_write_uid(TF_EVSE *evse, uint32_t uid) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(evse->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_read_uid(TF_EVSE *evse, uint32_t *ret_uid) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_evse_get_identity(TF_EVSE *evse, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = evse->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(evse->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(evse->tfp);
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
        tf_tfp_packet_processed(evse->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(evse->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}


int tf_evse_callback_tick(TF_EVSE *evse, uint32_t timeout_us) {
    if (evse == NULL) {
        return TF_E_NULL;
    }

    if (evse->magic != 0x5446 || evse->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = evse->tfp->spitfp->hal;

    return tf_tfp_callback_tick(evse->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
