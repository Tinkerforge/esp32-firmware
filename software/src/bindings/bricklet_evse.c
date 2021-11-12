/* ***********************************************************
 * This file was automatically generated on 2021-11-08.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.0         *
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


static bool tf_evse_callback_handler(void *dev, uint8_t fid, TF_Packetbuffer *payload) {
    (void)dev;
    (void)fid;
    (void)payload;
    return false;
}
int tf_evse_create(TF_EVSE *evse, const char *uid, TF_HalContext *hal) {
    if (evse == NULL || uid == NULL || hal == NULL)
        return TF_E_NULL;

    memset(evse, 0, sizeof(TF_EVSE));

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

    rc = tf_hal_get_tfp(hal, &evse->tfp, TF_EVSE_DEVICE_IDENTIFIER, inventory_index);
    if (rc != TF_E_OK) {
        return rc;
    }
    evse->tfp->device = evse;
    evse->tfp->uid = numeric_uid;
    evse->tfp->cb_handler = tf_evse_callback_handler;
    evse->response_expected[0] = 0x50;
    evse->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_evse_destroy(TF_EVSE *evse) {
    if (evse == NULL)
        return TF_E_NULL;

    int result = tf_tfp_destroy(evse->tfp);
    evse->tfp = NULL;
    return result;
}

int tf_evse_get_response_expected(TF_EVSE *evse, uint8_t function_id, bool *ret_response_expected) {
    if (evse == NULL)
        return TF_E_NULL;

    switch(function_id) {
        case TF_EVSE_FUNCTION_SET_MAX_CHARGING_CURRENT:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse->response_expected[0] & (1 << 0)) != 0;
            break;
        case TF_EVSE_FUNCTION_START_CHARGING:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse->response_expected[0] & (1 << 1)) != 0;
            break;
        case TF_EVSE_FUNCTION_STOP_CHARGING:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse->response_expected[0] & (1 << 2)) != 0;
            break;
        case TF_EVSE_FUNCTION_SET_CHARGING_AUTOSTART:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse->response_expected[0] & (1 << 3)) != 0;
            break;
        case TF_EVSE_FUNCTION_SET_MANAGED:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse->response_expected[0] & (1 << 4)) != 0;
            break;
        case TF_EVSE_FUNCTION_SET_MANAGED_CURRENT:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse->response_expected[0] & (1 << 5)) != 0;
            break;
        case TF_EVSE_FUNCTION_SET_USER_CALIBRATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse->response_expected[0] & (1 << 6)) != 0;
            break;
        case TF_EVSE_FUNCTION_SET_DATA_STORAGE:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse->response_expected[0] & (1 << 7)) != 0;
            break;
        case TF_EVSE_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse->response_expected[1] & (1 << 0)) != 0;
            break;
        case TF_EVSE_FUNCTION_SET_STATUS_LED_CONFIG:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse->response_expected[1] & (1 << 1)) != 0;
            break;
        case TF_EVSE_FUNCTION_RESET:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse->response_expected[1] & (1 << 2)) != 0;
            break;
        case TF_EVSE_FUNCTION_WRITE_UID:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse->response_expected[1] & (1 << 3)) != 0;
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

int tf_evse_set_response_expected(TF_EVSE *evse, uint8_t function_id, bool response_expected) {
    switch(function_id) {
        case TF_EVSE_FUNCTION_SET_MAX_CHARGING_CURRENT:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 0);
            } else {
                evse->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_EVSE_FUNCTION_START_CHARGING:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 1);
            } else {
                evse->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_EVSE_FUNCTION_STOP_CHARGING:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 2);
            } else {
                evse->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_EVSE_FUNCTION_SET_CHARGING_AUTOSTART:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 3);
            } else {
                evse->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_EVSE_FUNCTION_SET_MANAGED:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 4);
            } else {
                evse->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_EVSE_FUNCTION_SET_MANAGED_CURRENT:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 5);
            } else {
                evse->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_EVSE_FUNCTION_SET_USER_CALIBRATION:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 6);
            } else {
                evse->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_EVSE_FUNCTION_SET_DATA_STORAGE:
            if (response_expected) {
                evse->response_expected[0] |= (1 << 7);
            } else {
                evse->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_EVSE_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                evse->response_expected[1] |= (1 << 0);
            } else {
                evse->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_EVSE_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                evse->response_expected[1] |= (1 << 1);
            } else {
                evse->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_EVSE_FUNCTION_RESET:
            if (response_expected) {
                evse->response_expected[1] |= (1 << 2);
            } else {
                evse->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_EVSE_FUNCTION_WRITE_UID:
            if (response_expected) {
                evse->response_expected[1] |= (1 << 3);
            } else {
                evse->response_expected[1] &= ~(1 << 3);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

void tf_evse_set_response_expected_all(TF_EVSE *evse, bool response_expected) {
    memset(evse->response_expected, response_expected ? 0xFF : 0, 2);
}

int tf_evse_get_state(TF_EVSE *evse, uint8_t *ret_iec61851_state, uint8_t *ret_vehicle_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint8_t *ret_charge_release, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint32_t *ret_time_since_state_change, uint32_t *ret_uptime) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_STATE, 0, 17, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_iec61851_state != NULL) { *ret_iec61851_state = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_vehicle_state != NULL) { *ret_vehicle_state = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_contactor_state != NULL) { *ret_contactor_state = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_contactor_error != NULL) { *ret_contactor_error = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_charge_release != NULL) { *ret_charge_release = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_allowed_charging_current != NULL) { *ret_allowed_charging_current = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_error_state != NULL) { *ret_error_state = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_lock_state != NULL) { *ret_lock_state = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_time_since_state_change != NULL) { *ret_time_since_state_change = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_uptime != NULL) { *ret_uptime = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_hardware_configuration(TF_EVSE *evse, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_HARDWARE_CONFIGURATION, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_jumper_configuration != NULL) { *ret_jumper_configuration = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_has_lock_switch != NULL) { *ret_has_lock_switch = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_low_level_state(TF_EVSE *evse, bool *ret_low_level_mode_enabled, uint8_t *ret_led_state, uint16_t *ret_cp_pwm_duty_cycle, uint16_t ret_adc_values[2], int16_t ret_voltages[3], uint32_t ret_resistances[2], bool ret_gpio[5], uint8_t *ret_hardware_version, uint32_t *ret_charging_time) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_LOW_LEVEL_STATE, 0, 28, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_low_level_mode_enabled != NULL) { *ret_low_level_mode_enabled = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_led_state != NULL) { *ret_led_state = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_cp_pwm_duty_cycle != NULL) { *ret_cp_pwm_duty_cycle = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_adc_values != NULL) { for (i = 0; i < 2; ++i) ret_adc_values[i] = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_voltages != NULL) { for (i = 0; i < 3; ++i) ret_voltages[i] = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 6); }
        if (ret_resistances != NULL) { for (i = 0; i < 2; ++i) ret_resistances[i] = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 8); }
        if (ret_gpio != NULL) { tf_packetbuffer_read_bool_array(&evse->tfp->spitfp->recv_buf, ret_gpio, 5);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_hardware_version != NULL) { *ret_hardware_version = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_charging_time != NULL) { *ret_charging_time = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_set_max_charging_current(TF_EVSE *evse, uint16_t max_current) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_MAX_CHARGING_CURRENT, &response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_MAX_CHARGING_CURRENT, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    max_current = tf_leconvert_uint16_to(max_current); memcpy(buf + 0, &max_current, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_max_charging_current(TF_EVSE *evse, uint16_t *ret_max_current_configured, uint16_t *ret_max_current_incoming_cable, uint16_t *ret_max_current_outgoing_cable, uint16_t *ret_max_current_managed) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_MAX_CHARGING_CURRENT, 0, 8, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_max_current_configured != NULL) { *ret_max_current_configured = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_max_current_incoming_cable != NULL) { *ret_max_current_incoming_cable = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_max_current_outgoing_cable != NULL) { *ret_max_current_outgoing_cable = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_max_current_managed != NULL) { *ret_max_current_managed = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_calibrate(TF_EVSE *evse, uint8_t state, uint32_t password, int32_t value, bool *ret_success) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_CALIBRATE, 9, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    buf[0] = (uint8_t)state;
    password = tf_leconvert_uint32_to(password); memcpy(buf + 1, &password, 4);
    value = tf_leconvert_int32_to(value); memcpy(buf + 5, &value, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_success != NULL) { *ret_success = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_start_charging(TF_EVSE *evse) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_START_CHARGING, &response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_START_CHARGING, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_stop_charging(TF_EVSE *evse) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_STOP_CHARGING, &response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_STOP_CHARGING, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_set_charging_autostart(TF_EVSE *evse, bool autostart) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_CHARGING_AUTOSTART, &response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_CHARGING_AUTOSTART, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    buf[0] = autostart ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_charging_autostart(TF_EVSE *evse, bool *ret_autostart) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_CHARGING_AUTOSTART, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_autostart != NULL) { *ret_autostart = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_managed(TF_EVSE *evse, bool *ret_managed) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_MANAGED, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_managed != NULL) { *ret_managed = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_set_managed(TF_EVSE *evse, bool managed, uint32_t password) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_MANAGED, &response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_MANAGED, 5, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    buf[0] = managed ? 1 : 0;
    password = tf_leconvert_uint32_to(password); memcpy(buf + 1, &password, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_set_managed_current(TF_EVSE *evse, uint16_t current) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_MANAGED_CURRENT, &response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_MANAGED_CURRENT, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    current = tf_leconvert_uint16_to(current); memcpy(buf + 0, &current, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_user_calibration(TF_EVSE *evse, bool *ret_user_calibration_active, int16_t *ret_voltage_diff, int16_t *ret_voltage_mul, int16_t *ret_voltage_div, int16_t *ret_resistance_2700, int16_t ret_resistance_880[14]) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_USER_CALIBRATION, 0, 37, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_user_calibration_active != NULL) { *ret_user_calibration_active = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_voltage_diff != NULL) { *ret_voltage_diff = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_voltage_mul != NULL) { *ret_voltage_mul = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_voltage_div != NULL) { *ret_voltage_div = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_resistance_2700 != NULL) { *ret_resistance_2700 = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_resistance_880 != NULL) { for (i = 0; i < 14; ++i) ret_resistance_880[i] = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 28); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_set_user_calibration(TF_EVSE *evse, uint32_t password, bool user_calibration_active, int16_t voltage_diff, int16_t voltage_mul, int16_t voltage_div, int16_t resistance_2700, const int16_t resistance_880[14]) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_USER_CALIBRATION, &response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_USER_CALIBRATION, 41, 0, response_expected);

    size_t i;
    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    password = tf_leconvert_uint32_to(password); memcpy(buf + 0, &password, 4);
    buf[4] = user_calibration_active ? 1 : 0;
    voltage_diff = tf_leconvert_int16_to(voltage_diff); memcpy(buf + 5, &voltage_diff, 2);
    voltage_mul = tf_leconvert_int16_to(voltage_mul); memcpy(buf + 7, &voltage_mul, 2);
    voltage_div = tf_leconvert_int16_to(voltage_div); memcpy(buf + 9, &voltage_div, 2);
    resistance_2700 = tf_leconvert_int16_to(resistance_2700); memcpy(buf + 11, &resistance_2700, 2);
    for (i = 0; i < 14; i++) { int16_t tmp_resistance_880 = tf_leconvert_int16_to(resistance_880[i]); memcpy(buf + 13 + (i * sizeof(int16_t)), &tmp_resistance_880, sizeof(int16_t)); }

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_data_storage(TF_EVSE *evse, uint8_t page, uint8_t ret_data[63]) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_DATA_STORAGE, 1, 63, response_expected);

    size_t i;
    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    buf[0] = (uint8_t)page;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_data != NULL) { for (i = 0; i < 63; ++i) ret_data[i] = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 63); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_set_data_storage(TF_EVSE *evse, uint8_t page, const uint8_t data[63]) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_DATA_STORAGE, &response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_DATA_STORAGE, 64, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    buf[0] = (uint8_t)page;
    memcpy(buf + 1, data, 63);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_indicator_led(TF_EVSE *evse, int16_t *ret_indication, uint16_t *ret_duration) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_INDICATOR_LED, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_indication != NULL) { *ret_indication = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_duration != NULL) { *ret_duration = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_set_indicator_led(TF_EVSE *evse, int16_t indication, uint16_t duration, uint8_t *ret_status) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_INDICATOR_LED, 4, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    indication = tf_leconvert_int16_to(indication); memcpy(buf + 0, &indication, 2);
    duration = tf_leconvert_uint16_to(duration); memcpy(buf + 2, &duration, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_button_state(TF_EVSE *evse, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_BUTTON_STATE, 0, 9, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_button_press_time != NULL) { *ret_button_press_time = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_button_release_time != NULL) { *ret_button_release_time = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_button_pressed != NULL) { *ret_button_pressed = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_all_data_1(TF_EVSE *evse, uint8_t *ret_iec61851_state, uint8_t *ret_vehicle_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint8_t *ret_charge_release, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint32_t *ret_time_since_state_change, uint32_t *ret_uptime, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch, bool *ret_low_level_mode_enabled, uint8_t *ret_led_state, uint16_t *ret_cp_pwm_duty_cycle, uint16_t ret_adc_values[2], int16_t ret_voltages[3], uint32_t ret_resistances[2], bool ret_gpio[5], uint8_t *ret_hardware_version, uint32_t *ret_charging_time, uint16_t *ret_max_current_configured, uint16_t *ret_max_current_incoming_cable, uint16_t *ret_max_current_outgoing_cable, uint16_t *ret_max_current_managed, bool *ret_autostart, bool *ret_managed) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_ALL_DATA_1, 0, 57, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_iec61851_state != NULL) { *ret_iec61851_state = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_vehicle_state != NULL) { *ret_vehicle_state = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_contactor_state != NULL) { *ret_contactor_state = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_contactor_error != NULL) { *ret_contactor_error = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_charge_release != NULL) { *ret_charge_release = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_allowed_charging_current != NULL) { *ret_allowed_charging_current = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_error_state != NULL) { *ret_error_state = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_lock_state != NULL) { *ret_lock_state = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_time_since_state_change != NULL) { *ret_time_since_state_change = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_uptime != NULL) { *ret_uptime = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_jumper_configuration != NULL) { *ret_jumper_configuration = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_has_lock_switch != NULL) { *ret_has_lock_switch = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_low_level_mode_enabled != NULL) { *ret_low_level_mode_enabled = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_led_state != NULL) { *ret_led_state = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_cp_pwm_duty_cycle != NULL) { *ret_cp_pwm_duty_cycle = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_adc_values != NULL) { for (i = 0; i < 2; ++i) ret_adc_values[i] = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_voltages != NULL) { for (i = 0; i < 3; ++i) ret_voltages[i] = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 6); }
        if (ret_resistances != NULL) { for (i = 0; i < 2; ++i) ret_resistances[i] = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 8); }
        if (ret_gpio != NULL) { tf_packetbuffer_read_bool_array(&evse->tfp->spitfp->recv_buf, ret_gpio, 5);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_hardware_version != NULL) { *ret_hardware_version = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_charging_time != NULL) { *ret_charging_time = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_max_current_configured != NULL) { *ret_max_current_configured = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_max_current_incoming_cable != NULL) { *ret_max_current_incoming_cable = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_max_current_outgoing_cable != NULL) { *ret_max_current_outgoing_cable = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_max_current_managed != NULL) { *ret_max_current_managed = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_autostart != NULL) { *ret_autostart = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_managed != NULL) { *ret_managed = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_all_data_2(TF_EVSE *evse, bool *ret_user_calibration_active, int16_t *ret_voltage_diff, int16_t *ret_voltage_mul, int16_t *ret_voltage_div, int16_t *ret_resistance_2700, int16_t ret_resistance_880[14], int16_t *ret_indication, uint16_t *ret_duration, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_ALL_DATA_2, 0, 50, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_user_calibration_active != NULL) { *ret_user_calibration_active = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_voltage_diff != NULL) { *ret_voltage_diff = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_voltage_mul != NULL) { *ret_voltage_mul = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_voltage_div != NULL) { *ret_voltage_div = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_resistance_2700 != NULL) { *ret_resistance_2700 = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_resistance_880 != NULL) { for (i = 0; i < 14; ++i) ret_resistance_880[i] = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 28); }
        if (ret_indication != NULL) { *ret_indication = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_duration != NULL) { *ret_duration = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (ret_button_press_time != NULL) { *ret_button_press_time = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_button_release_time != NULL) { *ret_button_release_time = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_button_pressed != NULL) { *ret_button_pressed = tf_packetbuffer_read_bool(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_spitfp_error_count(TF_EVSE *evse, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, 16, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_set_bootloader_mode(TF_EVSE *evse, uint8_t mode, uint8_t *ret_status) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_BOOTLOADER_MODE, 1, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    buf[0] = (uint8_t)mode;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_bootloader_mode(TF_EVSE *evse, uint8_t *ret_mode) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_BOOTLOADER_MODE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_mode != NULL) { *ret_mode = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_set_write_firmware_pointer(TF_EVSE *evse, uint32_t pointer) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(buf + 0, &pointer, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_write_firmware(TF_EVSE *evse, const uint8_t data[64], uint8_t *ret_status) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_WRITE_FIRMWARE, 64, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    memcpy(buf + 0, data, 64);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_set_status_led_config(TF_EVSE *evse, uint8_t config) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_SET_STATUS_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_SET_STATUS_LED_CONFIG, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_status_led_config(TF_EVSE *evse, uint8_t *ret_config) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_STATUS_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_config != NULL) { *ret_config = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_chip_temperature(TF_EVSE *evse, int16_t *ret_temperature) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_CHIP_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_temperature != NULL) { *ret_temperature = tf_packetbuffer_read_int16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_reset(TF_EVSE *evse) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_RESET, &response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_RESET, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_write_uid(TF_EVSE *evse, uint32_t uid) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_get_response_expected(evse, TF_EVSE_FUNCTION_WRITE_UID, &response_expected);
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_WRITE_UID, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(buf + 0, &uid, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_read_uid(TF_EVSE *evse, uint32_t *ret_uid) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_READ_UID, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_uid != NULL) { *ret_uid = tf_packetbuffer_read_uint32_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_get_identity(TF_EVSE *evse, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (evse == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_IDENTITY, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        char tmp_connected_uid[8] = {0};
        if (ret_uid != NULL) { tf_packetbuffer_pop_n(&evse->tfp->spitfp->recv_buf, (uint8_t*)ret_uid, 8);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 8); }
        tf_packetbuffer_pop_n(&evse->tfp->spitfp->recv_buf, (uint8_t*)tmp_connected_uid, 8);
        if (ret_position != NULL) { *ret_position = tf_packetbuffer_read_char(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 1); }
        if (ret_hardware_version != NULL) { for (i = 0; i < 3; ++i) ret_hardware_version[i] = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 3); }
        if (ret_firmware_version != NULL) { for (i = 0; i < 3; ++i) ret_firmware_version[i] = tf_packetbuffer_read_uint8_t(&evse->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 3); }
        if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packetbuffer_read_uint16_t(&evse->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse->tfp->spitfp->recv_buf, 2); }
        if (tmp_connected_uid[0] == 0 && ret_position != NULL) {
            *ret_position = tf_hal_get_port_name((TF_HalContext*)evse->tfp->hal, evse->tfp->spitfp->port_id);
        }
        if (ret_connected_uid != NULL) {
            memcpy(ret_connected_uid, tmp_connected_uid, 8);
        }
        tf_tfp_packet_processed(evse->tfp);
    }

    result = tf_tfp_finish_send(evse->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}


int tf_evse_callback_tick(TF_EVSE *evse, uint32_t timeout_us) {
    if (evse == NULL)
        return TF_E_NULL;

    return tf_tfp_callback_tick(evse->tfp, tf_hal_current_time_us((TF_HalContext*)evse->tfp->hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
