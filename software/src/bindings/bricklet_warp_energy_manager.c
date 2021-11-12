/* ***********************************************************
 * This file was automatically generated on 2021-11-08.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.0         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_warp_energy_manager.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


static bool tf_warp_energy_manager_callback_handler(void *dev, uint8_t fid, TF_Packetbuffer *payload) {
    (void)dev;
    (void)fid;
    (void)payload;
    return false;
}
int tf_warp_energy_manager_create(TF_WARPEnergyManager *warp_energy_manager, const char *uid, TF_HalContext *hal) {
    if (warp_energy_manager == NULL || uid == NULL || hal == NULL)
        return TF_E_NULL;

    memset(warp_energy_manager, 0, sizeof(TF_WARPEnergyManager));

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

    rc = tf_hal_get_tfp(hal, &warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_DEVICE_IDENTIFIER, inventory_index);
    if (rc != TF_E_OK) {
        return rc;
    }
    warp_energy_manager->tfp->device = warp_energy_manager;
    warp_energy_manager->tfp->uid = numeric_uid;
    warp_energy_manager->tfp->cb_handler = tf_warp_energy_manager_callback_handler;
    warp_energy_manager->response_expected[0] = 0x00;
    warp_energy_manager->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_warp_energy_manager_destroy(TF_WARPEnergyManager *warp_energy_manager) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    int result = tf_tfp_destroy(warp_energy_manager->tfp);
    warp_energy_manager->tfp = NULL;
    return result;
}

int tf_warp_energy_manager_get_response_expected(TF_WARPEnergyManager *warp_energy_manager, uint8_t function_id, bool *ret_response_expected) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    switch(function_id) {
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_CONTACTOR:
            if(ret_response_expected != NULL)
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 0)) != 0;
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_RGB_VALUE:
            if(ret_response_expected != NULL)
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 1)) != 0;
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_RESET_ENERGY_METER:
            if(ret_response_expected != NULL)
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 2)) != 0;
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_OUTPUT:
            if(ret_response_expected != NULL)
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 3)) != 0;
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_INPUT_CONFIGURATION:
            if(ret_response_expected != NULL)
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 4)) != 0;
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if(ret_response_expected != NULL)
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 5)) != 0;
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_STATUS_LED_CONFIG:
            if(ret_response_expected != NULL)
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 6)) != 0;
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_RESET:
            if(ret_response_expected != NULL)
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 7)) != 0;
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_WRITE_UID:
            if(ret_response_expected != NULL)
                *ret_response_expected = (warp_energy_manager->response_expected[1] & (1 << 0)) != 0;
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

int tf_warp_energy_manager_set_response_expected(TF_WARPEnergyManager *warp_energy_manager, uint8_t function_id, bool response_expected) {
    switch(function_id) {
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_CONTACTOR:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 0);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_RGB_VALUE:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 1);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_RESET_ENERGY_METER:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 2);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_OUTPUT:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 3);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_INPUT_CONFIGURATION:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 4);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 5);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 6);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_RESET:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 7);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_WRITE_UID:
            if (response_expected) {
                warp_energy_manager->response_expected[1] |= (1 << 0);
            } else {
                warp_energy_manager->response_expected[1] &= ~(1 << 0);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

void tf_warp_energy_manager_set_response_expected_all(TF_WARPEnergyManager *warp_energy_manager, bool response_expected) {
    memset(warp_energy_manager->response_expected, response_expected ? 0xFF : 0, 2);
}

int tf_warp_energy_manager_set_contactor(TF_WARPEnergyManager *warp_energy_manager, bool value) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_CONTACTOR, &response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_CONTACTOR, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(warp_energy_manager->tfp);

    buf[0] = value ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_contactor(TF_WARPEnergyManager *warp_energy_manager, bool *ret_value) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_CONTACTOR, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_value != NULL) { *ret_value = tf_packetbuffer_read_bool(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_set_rgb_value(TF_WARPEnergyManager *warp_energy_manager, uint8_t r, uint8_t g, uint8_t b) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_RGB_VALUE, &response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_RGB_VALUE, 3, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(warp_energy_manager->tfp);

    buf[0] = (uint8_t)r;
    buf[1] = (uint8_t)g;
    buf[2] = (uint8_t)b;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_rgb_value(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_r, uint8_t *ret_g, uint8_t *ret_b) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_RGB_VALUE, 0, 3, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_r != NULL) { *ret_r = tf_packetbuffer_read_uint8_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        if (ret_g != NULL) { *ret_g = tf_packetbuffer_read_uint8_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        if (ret_b != NULL) { *ret_b = tf_packetbuffer_read_uint8_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_energy_meter_values(TF_WARPEnergyManager *warp_energy_manager, float *ret_power, float *ret_energy_relative, float *ret_energy_absolute, bool ret_phases_active[3], bool ret_phases_connected[3]) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_ENERGY_METER_VALUES, 0, 14, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_power != NULL) { *ret_power = tf_packetbuffer_read_float(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 4); }
        if (ret_energy_relative != NULL) { *ret_energy_relative = tf_packetbuffer_read_float(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 4); }
        if (ret_energy_absolute != NULL) { *ret_energy_absolute = tf_packetbuffer_read_float(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 4); }
        if (ret_phases_active != NULL) { tf_packetbuffer_read_bool_array(&warp_energy_manager->tfp->spitfp->recv_buf, ret_phases_active, 3);} else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        if (ret_phases_connected != NULL) { tf_packetbuffer_read_bool_array(&warp_energy_manager->tfp->spitfp->recv_buf, ret_phases_connected, 3);} else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_energy_meter_detailed_values_low_level(TF_WARPEnergyManager *warp_energy_manager, uint16_t *ret_values_chunk_offset, float ret_values_chunk_data[15]) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_ENERGY_METER_DETAILED_VALUES_LOW_LEVEL, 0, 62, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_values_chunk_offset != NULL) { *ret_values_chunk_offset = tf_packetbuffer_read_uint16_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 2); }
        if (ret_values_chunk_data != NULL) { for (i = 0; i < 15; ++i) ret_values_chunk_data[i] = tf_packetbuffer_read_float(&warp_energy_manager->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 60); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_energy_meter_state(TF_WARPEnergyManager *warp_energy_manager, bool *ret_available, uint32_t ret_error_count[6]) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_ENERGY_METER_STATE, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_available != NULL) { *ret_available = tf_packetbuffer_read_bool(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        if (ret_error_count != NULL) { for (i = 0; i < 6; ++i) ret_error_count[i] = tf_packetbuffer_read_uint32_t(&warp_energy_manager->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 24); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_reset_energy_meter(TF_WARPEnergyManager *warp_energy_manager) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_RESET_ENERGY_METER, &response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_RESET_ENERGY_METER, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_input(TF_WARPEnergyManager *warp_energy_manager, bool ret_input[2]) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_INPUT, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_input != NULL) { tf_packetbuffer_read_bool_array(&warp_energy_manager->tfp->spitfp->recv_buf, ret_input, 2);} else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_set_output(TF_WARPEnergyManager *warp_energy_manager, bool output) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_OUTPUT, &response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_OUTPUT, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(warp_energy_manager->tfp);

    buf[0] = output ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_output(TF_WARPEnergyManager *warp_energy_manager, bool *ret_output) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_OUTPUT, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_output != NULL) { *ret_output = tf_packetbuffer_read_bool(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_set_input_configuration(TF_WARPEnergyManager *warp_energy_manager, const uint8_t input_configuration[2]) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_INPUT_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_INPUT_CONFIGURATION, 2, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(warp_energy_manager->tfp);

    memcpy(buf + 0, input_configuration, 2);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_input_configuration(TF_WARPEnergyManager *warp_energy_manager, uint8_t ret_input_configuration[2]) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_INPUT_CONFIGURATION, 0, 2, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_input_configuration != NULL) { for (i = 0; i < 2; ++i) ret_input_configuration[i] = tf_packetbuffer_read_uint8_t(&warp_energy_manager->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_input_voltage(TF_WARPEnergyManager *warp_energy_manager, uint16_t *ret_voltage) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_INPUT_VOLTAGE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_voltage != NULL) { *ret_voltage = tf_packetbuffer_read_uint16_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_state(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_contactor_check_state) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_STATE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_contactor_check_state != NULL) { *ret_contactor_check_state = tf_packetbuffer_read_uint8_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_spitfp_error_count(TF_WARPEnergyManager *warp_energy_manager, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, 16, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packetbuffer_read_uint32_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packetbuffer_read_uint32_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packetbuffer_read_uint32_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packetbuffer_read_uint32_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_set_bootloader_mode(TF_WARPEnergyManager *warp_energy_manager, uint8_t mode, uint8_t *ret_status) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_BOOTLOADER_MODE, 1, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(warp_energy_manager->tfp);

    buf[0] = (uint8_t)mode;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_bootloader_mode(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_mode) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_BOOTLOADER_MODE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_mode != NULL) { *ret_mode = tf_packetbuffer_read_uint8_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_set_write_firmware_pointer(TF_WARPEnergyManager *warp_energy_manager, uint32_t pointer) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(warp_energy_manager->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(buf + 0, &pointer, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_write_firmware(TF_WARPEnergyManager *warp_energy_manager, const uint8_t data[64], uint8_t *ret_status) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_WRITE_FIRMWARE, 64, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(warp_energy_manager->tfp);

    memcpy(buf + 0, data, 64);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_set_status_led_config(TF_WARPEnergyManager *warp_energy_manager, uint8_t config) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_STATUS_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_STATUS_LED_CONFIG, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(warp_energy_manager->tfp);

    buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_status_led_config(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_config) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_STATUS_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_config != NULL) { *ret_config = tf_packetbuffer_read_uint8_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_chip_temperature(TF_WARPEnergyManager *warp_energy_manager, int16_t *ret_temperature) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_CHIP_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_temperature != NULL) { *ret_temperature = tf_packetbuffer_read_int16_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_reset(TF_WARPEnergyManager *warp_energy_manager) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_RESET, &response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_RESET, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_write_uid(TF_WARPEnergyManager *warp_energy_manager, uint32_t uid) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_WRITE_UID, &response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_WRITE_UID, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(warp_energy_manager->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(buf + 0, &uid, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_read_uid(TF_WARPEnergyManager *warp_energy_manager, uint32_t *ret_uid) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_READ_UID, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_uid != NULL) { *ret_uid = tf_packetbuffer_read_uint32_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_identity(TF_WARPEnergyManager *warp_energy_manager, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_IDENTITY, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + tf_hal_get_common((TF_HalContext*)warp_energy_manager->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(warp_energy_manager->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        char tmp_connected_uid[8] = {0};
        if (ret_uid != NULL) { tf_packetbuffer_pop_n(&warp_energy_manager->tfp->spitfp->recv_buf, (uint8_t*)ret_uid, 8);} else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 8); }
        tf_packetbuffer_pop_n(&warp_energy_manager->tfp->spitfp->recv_buf, (uint8_t*)tmp_connected_uid, 8);
        if (ret_position != NULL) { *ret_position = tf_packetbuffer_read_char(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 1); }
        if (ret_hardware_version != NULL) { for (i = 0; i < 3; ++i) ret_hardware_version[i] = tf_packetbuffer_read_uint8_t(&warp_energy_manager->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 3); }
        if (ret_firmware_version != NULL) { for (i = 0; i < 3; ++i) ret_firmware_version[i] = tf_packetbuffer_read_uint8_t(&warp_energy_manager->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 3); }
        if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packetbuffer_read_uint16_t(&warp_energy_manager->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&warp_energy_manager->tfp->spitfp->recv_buf, 2); }
        if (tmp_connected_uid[0] == 0 && ret_position != NULL) {
            *ret_position = tf_hal_get_port_name((TF_HalContext*)warp_energy_manager->tfp->hal, warp_energy_manager->tfp->spitfp->port_id);
        }
        if (ret_connected_uid != NULL) {
            memcpy(ret_connected_uid, tmp_connected_uid, 8);
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }

    result = tf_tfp_finish_send(warp_energy_manager->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_warp_energy_manager_get_energy_meter_detailed_values(TF_WARPEnergyManager *warp_energy_manager, float *ret_values, uint16_t *ret_values_length) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    int ret = TF_E_OK;
    uint16_t max_values_length = 85;
    uint16_t values_length = 0;
    uint16_t values_chunk_offset = 0;
    float values_chunk_data[15];
    bool values_out_of_sync;
    uint16_t values_chunk_length = 0;

    ret = tf_warp_energy_manager_get_energy_meter_detailed_values_low_level(warp_energy_manager, &values_chunk_offset, values_chunk_data);

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
            ret = tf_warp_energy_manager_get_energy_meter_detailed_values_low_level(warp_energy_manager, &values_chunk_offset, values_chunk_data);

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
            ret = tf_warp_energy_manager_get_energy_meter_detailed_values_low_level(warp_energy_manager, &values_chunk_offset, values_chunk_data);

            if (ret != TF_E_OK) {
                return ret;
            }
        }

        ret = TF_E_STREAM_OUT_OF_SYNC;
    }

    return ret;
}


int tf_warp_energy_manager_callback_tick(TF_WARPEnergyManager *warp_energy_manager, uint32_t timeout_us) {
    if (warp_energy_manager == NULL)
        return TF_E_NULL;

    return tf_tfp_callback_tick(warp_energy_manager->tfp, tf_hal_current_time_us((TF_HalContext*)warp_energy_manager->tfp->hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
