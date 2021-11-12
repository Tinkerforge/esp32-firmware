/* ***********************************************************
 * This file was automatically generated on 2021-11-08.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.0         *
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


static bool tf_evse_v2_callback_handler(void *dev, uint8_t fid, TF_Packetbuffer *payload) {
    (void)dev;
    (void)fid;
    (void)payload;
    return false;
}
int tf_evse_v2_create(TF_EVSEV2 *evse_v2, const char *uid, TF_HalContext *hal) {
    if (evse_v2 == NULL || uid == NULL || hal == NULL)
        return TF_E_NULL;

    memset(evse_v2, 0, sizeof(TF_EVSEV2));

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

    rc = tf_hal_get_tfp(hal, &evse_v2->tfp, TF_EVSE_V2_DEVICE_IDENTIFIER, inventory_index);
    if (rc != TF_E_OK) {
        return rc;
    }
    evse_v2->tfp->device = evse_v2;
    evse_v2->tfp->uid = numeric_uid;
    evse_v2->tfp->cb_handler = tf_evse_v2_callback_handler;
    evse_v2->response_expected[0] = 0xA0;
    evse_v2->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_evse_v2_destroy(TF_EVSEV2 *evse_v2) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    int result = tf_tfp_destroy(evse_v2->tfp);
    evse_v2->tfp = NULL;
    return result;
}

int tf_evse_v2_get_response_expected(TF_EVSEV2 *evse_v2, uint8_t function_id, bool *ret_response_expected) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    switch(function_id) {
        case TF_EVSE_V2_FUNCTION_SET_MAX_CHARGING_CURRENT:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 0)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_START_CHARGING:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 1)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_STOP_CHARGING:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 2)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_SET_CHARGING_AUTOSTART:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 3)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_RESET_ENERGY_METER:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 4)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_RESET_DC_FAULT_CURRENT:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 5)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_SET_GPIO_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 6)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_SET_MANAGED:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[0] & (1 << 7)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_SET_MANAGED_CURRENT:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 0)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_SET_DATA_STORAGE:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 1)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_SET_BUTTON_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 2)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_SET_CONTROL_PILOT_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 3)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 4)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 5)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_RESET:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 6)) != 0;
            break;
        case TF_EVSE_V2_FUNCTION_WRITE_UID:
            if(ret_response_expected != NULL)
                *ret_response_expected = (evse_v2->response_expected[1] & (1 << 7)) != 0;
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

int tf_evse_v2_set_response_expected(TF_EVSEV2 *evse_v2, uint8_t function_id, bool response_expected) {
    switch(function_id) {
        case TF_EVSE_V2_FUNCTION_SET_MAX_CHARGING_CURRENT:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 0);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_EVSE_V2_FUNCTION_START_CHARGING:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 1);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_EVSE_V2_FUNCTION_STOP_CHARGING:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 2);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_CHARGING_AUTOSTART:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 3);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_EVSE_V2_FUNCTION_RESET_ENERGY_METER:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 4);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_EVSE_V2_FUNCTION_RESET_DC_FAULT_CURRENT:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 5);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_GPIO_CONFIGURATION:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 6);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_MANAGED:
            if (response_expected) {
                evse_v2->response_expected[0] |= (1 << 7);
            } else {
                evse_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_MANAGED_CURRENT:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 0);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_DATA_STORAGE:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 1);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_BUTTON_CONFIGURATION:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 2);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_CONTROL_PILOT_CONFIGURATION:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 3);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 3);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 4);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 4);
            }
            break;
        case TF_EVSE_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 5);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 5);
            }
            break;
        case TF_EVSE_V2_FUNCTION_RESET:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 6);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 6);
            }
            break;
        case TF_EVSE_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                evse_v2->response_expected[1] |= (1 << 7);
            } else {
                evse_v2->response_expected[1] &= ~(1 << 7);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

void tf_evse_v2_set_response_expected_all(TF_EVSEV2 *evse_v2, bool response_expected) {
    memset(evse_v2->response_expected, response_expected ? 0xFF : 0, 2);
}

int tf_evse_v2_get_state(TF_EVSEV2 *evse_v2, uint8_t *ret_iec61851_state, uint8_t *ret_vehicle_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint8_t *ret_charge_release, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint32_t *ret_time_since_state_change, uint32_t *ret_uptime) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_STATE, 0, 17, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_iec61851_state != NULL) { *ret_iec61851_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_vehicle_state != NULL) { *ret_vehicle_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_contactor_state != NULL) { *ret_contactor_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_contactor_error != NULL) { *ret_contactor_error = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_charge_release != NULL) { *ret_charge_release = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_allowed_charging_current != NULL) { *ret_allowed_charging_current = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_error_state != NULL) { *ret_error_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_lock_state != NULL) { *ret_lock_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_time_since_state_change != NULL) { *ret_time_since_state_change = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_uptime != NULL) { *ret_uptime = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_hardware_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_HARDWARE_CONFIGURATION, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_jumper_configuration != NULL) { *ret_jumper_configuration = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_has_lock_switch != NULL) { *ret_has_lock_switch = tf_packetbuffer_read_bool(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_low_level_state(TF_EVSEV2 *evse_v2, uint8_t *ret_led_state, uint16_t *ret_cp_pwm_duty_cycle, uint16_t ret_adc_values[7], int16_t ret_voltages[7], uint32_t ret_resistances[2], bool ret_gpio[24], uint32_t *ret_charging_time) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_LOW_LEVEL_STATE, 0, 46, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_led_state != NULL) { *ret_led_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_cp_pwm_duty_cycle != NULL) { *ret_cp_pwm_duty_cycle = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_adc_values != NULL) { for (i = 0; i < 7; ++i) ret_adc_values[i] = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 14); }
        if (ret_voltages != NULL) { for (i = 0; i < 7; ++i) ret_voltages[i] = tf_packetbuffer_read_int16_t(&evse_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 14); }
        if (ret_resistances != NULL) { for (i = 0; i < 2; ++i) ret_resistances[i] = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 8); }
        if (ret_gpio != NULL) { tf_packetbuffer_read_bool_array(&evse_v2->tfp->spitfp->recv_buf, ret_gpio, 24);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 3); }
        if (ret_charging_time != NULL) { *ret_charging_time = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_set_max_charging_current(TF_EVSEV2 *evse_v2, uint16_t max_current) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_MAX_CHARGING_CURRENT, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_MAX_CHARGING_CURRENT, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    max_current = tf_leconvert_uint16_to(max_current); memcpy(buf + 0, &max_current, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_max_charging_current(TF_EVSEV2 *evse_v2, uint16_t *ret_max_current_configured, uint16_t *ret_max_current_incoming_cable, uint16_t *ret_max_current_outgoing_cable, uint16_t *ret_max_current_managed) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_MAX_CHARGING_CURRENT, 0, 8, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_max_current_configured != NULL) { *ret_max_current_configured = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_max_current_incoming_cable != NULL) { *ret_max_current_incoming_cable = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_max_current_outgoing_cable != NULL) { *ret_max_current_outgoing_cable = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_max_current_managed != NULL) { *ret_max_current_managed = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_start_charging(TF_EVSEV2 *evse_v2) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_START_CHARGING, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_START_CHARGING, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_stop_charging(TF_EVSEV2 *evse_v2) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_STOP_CHARGING, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_STOP_CHARGING, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_set_charging_autostart(TF_EVSEV2 *evse_v2, bool autostart) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_CHARGING_AUTOSTART, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_CHARGING_AUTOSTART, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    buf[0] = autostart ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_charging_autostart(TF_EVSEV2 *evse_v2, bool *ret_autostart) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_CHARGING_AUTOSTART, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_autostart != NULL) { *ret_autostart = tf_packetbuffer_read_bool(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_energy_meter_values(TF_EVSEV2 *evse_v2, float *ret_power, float *ret_energy_relative, float *ret_energy_absolute, bool ret_phases_active[3], bool ret_phases_connected[3]) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_ENERGY_METER_VALUES, 0, 14, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_power != NULL) { *ret_power = tf_packetbuffer_read_float(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_energy_relative != NULL) { *ret_energy_relative = tf_packetbuffer_read_float(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_energy_absolute != NULL) { *ret_energy_absolute = tf_packetbuffer_read_float(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_phases_active != NULL) { tf_packetbuffer_read_bool_array(&evse_v2->tfp->spitfp->recv_buf, ret_phases_active, 3);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_phases_connected != NULL) { tf_packetbuffer_read_bool_array(&evse_v2->tfp->spitfp->recv_buf, ret_phases_connected, 3);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_energy_meter_detailed_values_low_level(TF_EVSEV2 *evse_v2, uint16_t *ret_values_chunk_offset, float ret_values_chunk_data[15]) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_ENERGY_METER_DETAILED_VALUES_LOW_LEVEL, 0, 62, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_values_chunk_offset != NULL) { *ret_values_chunk_offset = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_values_chunk_data != NULL) { for (i = 0; i < 15; ++i) ret_values_chunk_data[i] = tf_packetbuffer_read_float(&evse_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 60); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_energy_meter_state(TF_EVSEV2 *evse_v2, bool *ret_available, uint32_t ret_error_count[6]) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_ENERGY_METER_STATE, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_available != NULL) { *ret_available = tf_packetbuffer_read_bool(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_error_count != NULL) { for (i = 0; i < 6; ++i) ret_error_count[i] = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 24); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_reset_energy_meter(TF_EVSEV2 *evse_v2) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_RESET_ENERGY_METER, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_RESET_ENERGY_METER, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_dc_fault_current_state(TF_EVSEV2 *evse_v2, uint8_t *ret_dc_fault_current_state) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_DC_FAULT_CURRENT_STATE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_dc_fault_current_state != NULL) { *ret_dc_fault_current_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_reset_dc_fault_current(TF_EVSEV2 *evse_v2, uint32_t password) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_RESET_DC_FAULT_CURRENT, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_RESET_DC_FAULT_CURRENT, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    password = tf_leconvert_uint32_to(password); memcpy(buf + 0, &password, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_set_gpio_configuration(TF_EVSEV2 *evse_v2, uint8_t shutdown_input_configuration, uint8_t input_configuration, uint8_t output_configuration) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_GPIO_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_GPIO_CONFIGURATION, 3, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    buf[0] = (uint8_t)shutdown_input_configuration;
    buf[1] = (uint8_t)input_configuration;
    buf[2] = (uint8_t)output_configuration;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_gpio_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_shutdown_input_configuration, uint8_t *ret_input_configuration, uint8_t *ret_output_configuration) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_GPIO_CONFIGURATION, 0, 3, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_shutdown_input_configuration != NULL) { *ret_shutdown_input_configuration = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_input_configuration != NULL) { *ret_input_configuration = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_output_configuration != NULL) { *ret_output_configuration = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_managed(TF_EVSEV2 *evse_v2, bool *ret_managed) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_MANAGED, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_managed != NULL) { *ret_managed = tf_packetbuffer_read_bool(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_set_managed(TF_EVSEV2 *evse_v2, bool managed, uint32_t password) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_MANAGED, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_MANAGED, 5, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    buf[0] = managed ? 1 : 0;
    password = tf_leconvert_uint32_to(password); memcpy(buf + 1, &password, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_set_managed_current(TF_EVSEV2 *evse_v2, uint16_t current) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_MANAGED_CURRENT, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_MANAGED_CURRENT, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    current = tf_leconvert_uint16_to(current); memcpy(buf + 0, &current, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_data_storage(TF_EVSEV2 *evse_v2, uint8_t page, uint8_t ret_data[63]) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_DATA_STORAGE, 1, 63, response_expected);

    size_t i;
    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    buf[0] = (uint8_t)page;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_data != NULL) { for (i = 0; i < 63; ++i) ret_data[i] = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 63); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_set_data_storage(TF_EVSEV2 *evse_v2, uint8_t page, const uint8_t data[63]) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_DATA_STORAGE, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_DATA_STORAGE, 64, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    buf[0] = (uint8_t)page;
    memcpy(buf + 1, data, 63);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_indicator_led(TF_EVSEV2 *evse_v2, int16_t *ret_indication, uint16_t *ret_duration) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_INDICATOR_LED, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_indication != NULL) { *ret_indication = tf_packetbuffer_read_int16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_duration != NULL) { *ret_duration = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_set_indicator_led(TF_EVSEV2 *evse_v2, int16_t indication, uint16_t duration, uint8_t *ret_status) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_INDICATOR_LED, 4, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    indication = tf_leconvert_int16_to(indication); memcpy(buf + 0, &indication, 2);
    duration = tf_leconvert_uint16_to(duration); memcpy(buf + 2, &duration, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_set_button_configuration(TF_EVSEV2 *evse_v2, uint8_t button_configuration) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_BUTTON_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_BUTTON_CONFIGURATION, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    buf[0] = (uint8_t)button_configuration;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_button_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_button_configuration) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_BUTTON_CONFIGURATION, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_button_configuration != NULL) { *ret_button_configuration = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_button_state(TF_EVSEV2 *evse_v2, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_BUTTON_STATE, 0, 9, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_button_press_time != NULL) { *ret_button_press_time = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_button_release_time != NULL) { *ret_button_release_time = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_button_pressed != NULL) { *ret_button_pressed = tf_packetbuffer_read_bool(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_set_control_pilot_configuration(TF_EVSEV2 *evse_v2, uint8_t control_pilot) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_CONTROL_PILOT_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_CONTROL_PILOT_CONFIGURATION, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    buf[0] = (uint8_t)control_pilot;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_control_pilot_configuration(TF_EVSEV2 *evse_v2, uint8_t *ret_control_pilot) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_CONTROL_PILOT_CONFIGURATION, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_control_pilot != NULL) { *ret_control_pilot = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_all_data_1(TF_EVSEV2 *evse_v2, uint8_t *ret_iec61851_state, uint8_t *ret_vehicle_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint8_t *ret_charge_release, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint32_t *ret_time_since_state_change, uint32_t *ret_uptime, uint8_t *ret_jumper_configuration, bool *ret_has_lock_switch) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_ALL_DATA_1, 0, 19, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_iec61851_state != NULL) { *ret_iec61851_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_vehicle_state != NULL) { *ret_vehicle_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_contactor_state != NULL) { *ret_contactor_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_contactor_error != NULL) { *ret_contactor_error = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_charge_release != NULL) { *ret_charge_release = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_allowed_charging_current != NULL) { *ret_allowed_charging_current = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_error_state != NULL) { *ret_error_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_lock_state != NULL) { *ret_lock_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_time_since_state_change != NULL) { *ret_time_since_state_change = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_uptime != NULL) { *ret_uptime = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_jumper_configuration != NULL) { *ret_jumper_configuration = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_has_lock_switch != NULL) { *ret_has_lock_switch = tf_packetbuffer_read_bool(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_all_data_2(TF_EVSEV2 *evse_v2, uint8_t *ret_led_state, uint16_t *ret_cp_pwm_duty_cycle, uint16_t ret_adc_values[7], int16_t ret_voltages[7], uint32_t ret_resistances[2], bool ret_gpio[24], uint32_t *ret_charging_time, uint16_t *ret_max_current_configured, uint16_t *ret_max_current_incoming_cable, uint16_t *ret_max_current_outgoing_cable, uint16_t *ret_max_current_managed, bool *ret_autostart) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_ALL_DATA_2, 0, 55, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_led_state != NULL) { *ret_led_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_cp_pwm_duty_cycle != NULL) { *ret_cp_pwm_duty_cycle = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_adc_values != NULL) { for (i = 0; i < 7; ++i) ret_adc_values[i] = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 14); }
        if (ret_voltages != NULL) { for (i = 0; i < 7; ++i) ret_voltages[i] = tf_packetbuffer_read_int16_t(&evse_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 14); }
        if (ret_resistances != NULL) { for (i = 0; i < 2; ++i) ret_resistances[i] = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 8); }
        if (ret_gpio != NULL) { tf_packetbuffer_read_bool_array(&evse_v2->tfp->spitfp->recv_buf, ret_gpio, 24);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 3); }
        if (ret_charging_time != NULL) { *ret_charging_time = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_max_current_configured != NULL) { *ret_max_current_configured = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_max_current_incoming_cable != NULL) { *ret_max_current_incoming_cable = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_max_current_outgoing_cable != NULL) { *ret_max_current_outgoing_cable = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_max_current_managed != NULL) { *ret_max_current_managed = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_autostart != NULL) { *ret_autostart = tf_packetbuffer_read_bool(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_all_data_3(TF_EVSEV2 *evse_v2, float *ret_power, float *ret_energy_relative, float *ret_energy_absolute, bool ret_phases_active[3], bool ret_phases_connected[3], bool *ret_available, uint32_t ret_error_count[6], uint8_t *ret_dc_fault_current_state, uint8_t *ret_shutdown_input_configuration, uint8_t *ret_input_configuration, uint8_t *ret_output_configuration, bool *ret_managed, int16_t *ret_indication, uint16_t *ret_duration, uint8_t *ret_button_configuration, uint32_t *ret_button_press_time, uint32_t *ret_button_release_time, bool *ret_button_pressed, uint8_t *ret_control_pilot) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_ALL_DATA_3, 0, 59, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_power != NULL) { *ret_power = tf_packetbuffer_read_float(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_energy_relative != NULL) { *ret_energy_relative = tf_packetbuffer_read_float(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_energy_absolute != NULL) { *ret_energy_absolute = tf_packetbuffer_read_float(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_phases_active != NULL) { tf_packetbuffer_read_bool_array(&evse_v2->tfp->spitfp->recv_buf, ret_phases_active, 3);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_phases_connected != NULL) { tf_packetbuffer_read_bool_array(&evse_v2->tfp->spitfp->recv_buf, ret_phases_connected, 3);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_available != NULL) { *ret_available = tf_packetbuffer_read_bool(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_error_count != NULL) { for (i = 0; i < 6; ++i) ret_error_count[i] = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 24); }
        if (ret_dc_fault_current_state != NULL) { *ret_dc_fault_current_state = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_shutdown_input_configuration != NULL) { *ret_shutdown_input_configuration = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_input_configuration != NULL) { *ret_input_configuration = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_output_configuration != NULL) { *ret_output_configuration = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_managed != NULL) { *ret_managed = tf_packetbuffer_read_bool(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_indication != NULL) { *ret_indication = tf_packetbuffer_read_int16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_duration != NULL) { *ret_duration = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (ret_button_configuration != NULL) { *ret_button_configuration = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_button_press_time != NULL) { *ret_button_press_time = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_button_release_time != NULL) { *ret_button_release_time = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_button_pressed != NULL) { *ret_button_pressed = tf_packetbuffer_read_bool(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_control_pilot != NULL) { *ret_control_pilot = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_spitfp_error_count(TF_EVSEV2 *evse_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, 16, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_set_bootloader_mode(TF_EVSEV2 *evse_v2, uint8_t mode, uint8_t *ret_status) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    buf[0] = (uint8_t)mode;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_bootloader_mode(TF_EVSEV2 *evse_v2, uint8_t *ret_mode) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_mode != NULL) { *ret_mode = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_set_write_firmware_pointer(TF_EVSEV2 *evse_v2, uint32_t pointer) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(buf + 0, &pointer, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_write_firmware(TF_EVSEV2 *evse_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_WRITE_FIRMWARE, 64, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    memcpy(buf + 0, data, 64);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_set_status_led_config(TF_EVSEV2 *evse_v2, uint8_t config) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_SET_STATUS_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_status_led_config(TF_EVSEV2 *evse_v2, uint8_t *ret_config) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_config != NULL) { *ret_config = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_chip_temperature(TF_EVSEV2 *evse_v2, int16_t *ret_temperature) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_temperature != NULL) { *ret_temperature = tf_packetbuffer_read_int16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_reset(TF_EVSEV2 *evse_v2) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_RESET, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_RESET, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_write_uid(TF_EVSEV2 *evse_v2, uint32_t uid) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_evse_v2_get_response_expected(evse_v2, TF_EVSE_V2_FUNCTION_WRITE_UID, &response_expected);
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_WRITE_UID, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(evse_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(buf + 0, &uid, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_read_uid(TF_EVSEV2 *evse_v2, uint32_t *ret_uid) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_READ_UID, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_uid != NULL) { *ret_uid = tf_packetbuffer_read_uint32_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_identity(TF_EVSEV2 *evse_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(evse_v2->tfp, TF_EVSE_V2_FUNCTION_GET_IDENTITY, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + tf_hal_get_common((TF_HalContext*)evse_v2->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(evse_v2->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        char tmp_connected_uid[8] = {0};
        if (ret_uid != NULL) { tf_packetbuffer_pop_n(&evse_v2->tfp->spitfp->recv_buf, (uint8_t*)ret_uid, 8);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 8); }
        tf_packetbuffer_pop_n(&evse_v2->tfp->spitfp->recv_buf, (uint8_t*)tmp_connected_uid, 8);
        if (ret_position != NULL) { *ret_position = tf_packetbuffer_read_char(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 1); }
        if (ret_hardware_version != NULL) { for (i = 0; i < 3; ++i) ret_hardware_version[i] = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 3); }
        if (ret_firmware_version != NULL) { for (i = 0; i < 3; ++i) ret_firmware_version[i] = tf_packetbuffer_read_uint8_t(&evse_v2->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 3); }
        if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packetbuffer_read_uint16_t(&evse_v2->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&evse_v2->tfp->spitfp->recv_buf, 2); }
        if (tmp_connected_uid[0] == 0 && ret_position != NULL) {
            *ret_position = tf_hal_get_port_name((TF_HalContext*)evse_v2->tfp->hal, evse_v2->tfp->spitfp->port_id);
        }
        if (ret_connected_uid != NULL) {
            memcpy(ret_connected_uid, tmp_connected_uid, 8);
        }
        tf_tfp_packet_processed(evse_v2->tfp);
    }

    result = tf_tfp_finish_send(evse_v2->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_evse_v2_get_energy_meter_detailed_values(TF_EVSEV2 *evse_v2, float *ret_values, uint16_t *ret_values_length) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    int ret = TF_E_OK;
    uint16_t max_values_length = 85;
    uint16_t values_length = 0;
    uint16_t values_chunk_offset = 0;
    float values_chunk_data[15];
    bool values_out_of_sync;
    uint16_t values_chunk_length = 0;

    ret = tf_evse_v2_get_energy_meter_detailed_values_low_level(evse_v2, &values_chunk_offset, values_chunk_data);

    if (ret != TF_E_OK) {
        if (ret_values_length != NULL) {
            *ret_values_length = values_length;
        }
        return ret;
    }

    if (values_chunk_offset == (1 << 16) - 1) { // maximum chunk offset -> stream has no data
        return ret;
    }

    values_out_of_sync = values_chunk_offset != 0;

    if (!values_out_of_sync) {
        values_chunk_length = max_values_length - values_chunk_offset;

        if (values_chunk_length > 15) {
            values_chunk_length = 15;
        }

        if (ret_values != NULL) {
            memcpy(ret_values, values_chunk_data, sizeof(float) * values_chunk_length);
        }

        values_length = values_chunk_length;

        while (values_length < max_values_length) {
            ret = tf_evse_v2_get_energy_meter_detailed_values_low_level(evse_v2, &values_chunk_offset, values_chunk_data);

            if (ret != TF_E_OK) {
                if (ret_values_length != NULL) {
                    *ret_values_length = values_length;
                }
                return ret;
            }

            values_out_of_sync = values_chunk_offset != values_length;

            if (values_out_of_sync) {
                break;
            }

            values_chunk_length = max_values_length - values_chunk_offset;

            if (values_chunk_length > 15) {
                values_chunk_length = 15;
            }

            if (ret_values != NULL) {
                memcpy(&ret_values[values_length], values_chunk_data, sizeof(float) * values_chunk_length);
            }
            values_length += values_chunk_length;
        }
    }

    if (values_out_of_sync) {
        if (ret_values_length != NULL) {
            *ret_values_length = 0; // return empty array
        }

        // discard remaining stream to bring it back in-sync
        while (values_chunk_offset + 15 < max_values_length) {
            ret = tf_evse_v2_get_energy_meter_detailed_values_low_level(evse_v2, &values_chunk_offset, values_chunk_data);

            if (ret != TF_E_OK) {
                return ret;
            }
        }

        ret = TF_E_STREAM_OUT_OF_SYNC;
    }

    return ret;
}


int tf_evse_v2_callback_tick(TF_EVSEV2 *evse_v2, uint32_t timeout_us) {
    if (evse_v2 == NULL)
        return TF_E_NULL;

    return tf_tfp_callback_tick(evse_v2->tfp, tf_hal_current_time_us((TF_HalContext*)evse_v2->tfp->hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
