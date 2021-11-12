/* ***********************************************************
 * This file was automatically generated on 2021-11-08.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.0         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_multi_io.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


static bool tf_multi_io_callback_handler(void *dev, uint8_t fid, TF_Packetbuffer *payload) {
    (void)dev;
    (void)fid;
    (void)payload;
    return false;
}
int tf_multi_io_create(TF_MultiIO *multi_io, const char *uid, TF_HalContext *hal) {
    if (multi_io == NULL || uid == NULL || hal == NULL)
        return TF_E_NULL;

    memset(multi_io, 0, sizeof(TF_MultiIO));

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

    rc = tf_hal_get_tfp(hal, &multi_io->tfp, TF_MULTI_IO_DEVICE_IDENTIFIER, inventory_index);
    if (rc != TF_E_OK) {
        return rc;
    }
    multi_io->tfp->device = multi_io;
    multi_io->tfp->uid = numeric_uid;
    multi_io->tfp->cb_handler = tf_multi_io_callback_handler;
    multi_io->response_expected[0] = 0x00;
    return TF_E_OK;
}

int tf_multi_io_destroy(TF_MultiIO *multi_io) {
    if (multi_io == NULL)
        return TF_E_NULL;

    int result = tf_tfp_destroy(multi_io->tfp);
    multi_io->tfp = NULL;
    return result;
}

int tf_multi_io_get_response_expected(TF_MultiIO *multi_io, uint8_t function_id, bool *ret_response_expected) {
    if (multi_io == NULL)
        return TF_E_NULL;

    switch(function_id) {
        case TF_MULTI_IO_FUNCTION_SET_OUTPUTS:
            if(ret_response_expected != NULL)
                *ret_response_expected = (multi_io->response_expected[0] & (1 << 0)) != 0;
            break;
        case TF_MULTI_IO_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if(ret_response_expected != NULL)
                *ret_response_expected = (multi_io->response_expected[0] & (1 << 1)) != 0;
            break;
        case TF_MULTI_IO_FUNCTION_SET_STATUS_LED_CONFIG:
            if(ret_response_expected != NULL)
                *ret_response_expected = (multi_io->response_expected[0] & (1 << 2)) != 0;
            break;
        case TF_MULTI_IO_FUNCTION_RESET:
            if(ret_response_expected != NULL)
                *ret_response_expected = (multi_io->response_expected[0] & (1 << 3)) != 0;
            break;
        case TF_MULTI_IO_FUNCTION_WRITE_UID:
            if(ret_response_expected != NULL)
                *ret_response_expected = (multi_io->response_expected[0] & (1 << 4)) != 0;
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

int tf_multi_io_set_response_expected(TF_MultiIO *multi_io, uint8_t function_id, bool response_expected) {
    switch(function_id) {
        case TF_MULTI_IO_FUNCTION_SET_OUTPUTS:
            if (response_expected) {
                multi_io->response_expected[0] |= (1 << 0);
            } else {
                multi_io->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_MULTI_IO_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                multi_io->response_expected[0] |= (1 << 1);
            } else {
                multi_io->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_MULTI_IO_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                multi_io->response_expected[0] |= (1 << 2);
            } else {
                multi_io->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_MULTI_IO_FUNCTION_RESET:
            if (response_expected) {
                multi_io->response_expected[0] |= (1 << 3);
            } else {
                multi_io->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_MULTI_IO_FUNCTION_WRITE_UID:
            if (response_expected) {
                multi_io->response_expected[0] |= (1 << 4);
            } else {
                multi_io->response_expected[0] &= ~(1 << 4);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }
    return TF_E_OK;
}

void tf_multi_io_set_response_expected_all(TF_MultiIO *multi_io, bool response_expected) {
    memset(multi_io->response_expected, response_expected ? 0xFF : 0, 1);
}

int tf_multi_io_get_inputs(TF_MultiIO *multi_io, uint8_t ret_value[16]) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_GET_INPUTS, 0, 16, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_value != NULL) { for (i = 0; i < 16; ++i) ret_value[i] = tf_packetbuffer_read_uint8_t(&multi_io->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 16); }
        tf_tfp_packet_processed(multi_io->tfp);
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_set_outputs(TF_MultiIO *multi_io, const bool value[2]) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_multi_io_get_response_expected(multi_io, TF_MULTI_IO_FUNCTION_SET_OUTPUTS, &response_expected);
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_SET_OUTPUTS, 1, 0, response_expected);

    size_t i;
    uint8_t *buf = tf_tfp_get_payload_buffer(multi_io->tfp);

    memset(buf + 0, 0, 1); for (i = 0; i < 2; ++i) buf[0 + (i / 8)] |= (value[i] ? 1 : 0) << (i % 8);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_get_outputs(TF_MultiIO *multi_io, bool ret_value[2]) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_GET_OUTPUTS, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_value != NULL) { tf_packetbuffer_read_bool_array(&multi_io->tfp->spitfp->recv_buf, ret_value, 2);} else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(multi_io->tfp);
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_get_spitfp_error_count(TF_MultiIO *multi_io, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, 16, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packetbuffer_read_uint32_t(&multi_io->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packetbuffer_read_uint32_t(&multi_io->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packetbuffer_read_uint32_t(&multi_io->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 4); }
        if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packetbuffer_read_uint32_t(&multi_io->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(multi_io->tfp);
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_set_bootloader_mode(TF_MultiIO *multi_io, uint8_t mode, uint8_t *ret_status) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_SET_BOOTLOADER_MODE, 1, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(multi_io->tfp);

    buf[0] = (uint8_t)mode;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&multi_io->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(multi_io->tfp);
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_get_bootloader_mode(TF_MultiIO *multi_io, uint8_t *ret_mode) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_GET_BOOTLOADER_MODE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_mode != NULL) { *ret_mode = tf_packetbuffer_read_uint8_t(&multi_io->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(multi_io->tfp);
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_set_write_firmware_pointer(TF_MultiIO *multi_io, uint32_t pointer) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_multi_io_get_response_expected(multi_io, TF_MULTI_IO_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &response_expected);
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(multi_io->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(buf + 0, &pointer, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_write_firmware(TF_MultiIO *multi_io, const uint8_t data[64], uint8_t *ret_status) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_WRITE_FIRMWARE, 64, 1, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(multi_io->tfp);

    memcpy(buf + 0, data, 64);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_status != NULL) { *ret_status = tf_packetbuffer_read_uint8_t(&multi_io->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(multi_io->tfp);
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_set_status_led_config(TF_MultiIO *multi_io, uint8_t config) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_multi_io_get_response_expected(multi_io, TF_MULTI_IO_FUNCTION_SET_STATUS_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_SET_STATUS_LED_CONFIG, 1, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(multi_io->tfp);

    buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_get_status_led_config(TF_MultiIO *multi_io, uint8_t *ret_config) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_GET_STATUS_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_config != NULL) { *ret_config = tf_packetbuffer_read_uint8_t(&multi_io->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 1); }
        tf_tfp_packet_processed(multi_io->tfp);
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_get_chip_temperature(TF_MultiIO *multi_io, int16_t *ret_temperature) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_GET_CHIP_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_temperature != NULL) { *ret_temperature = tf_packetbuffer_read_int16_t(&multi_io->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 2); }
        tf_tfp_packet_processed(multi_io->tfp);
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_reset(TF_MultiIO *multi_io) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_multi_io_get_response_expected(multi_io, TF_MULTI_IO_FUNCTION_RESET, &response_expected);
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_RESET, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_write_uid(TF_MultiIO *multi_io, uint32_t uid) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_multi_io_get_response_expected(multi_io, TF_MULTI_IO_FUNCTION_WRITE_UID, &response_expected);
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_WRITE_UID, 4, 0, response_expected);

    uint8_t *buf = tf_tfp_get_payload_buffer(multi_io->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(buf + 0, &uid, 4);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_read_uid(TF_MultiIO *multi_io, uint32_t *ret_uid) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_READ_UID, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        if (ret_uid != NULL) { *ret_uid = tf_packetbuffer_read_uint32_t(&multi_io->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 4); }
        tf_tfp_packet_processed(multi_io->tfp);
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}

int tf_multi_io_get_identity(TF_MultiIO *multi_io, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (multi_io == NULL)
        return TF_E_NULL;

    if(tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(multi_io->tfp, TF_MULTI_IO_FUNCTION_GET_IDENTITY, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + tf_hal_get_common((TF_HalContext*)multi_io->tfp->hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_transmit_packet(multi_io->tfp, response_expected, deadline, &error_code);
    if(result < 0)
        return result;

    if (result & TF_TICK_TIMEOUT) {
        //return -result;
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        char tmp_connected_uid[8] = {0};
        if (ret_uid != NULL) { tf_packetbuffer_pop_n(&multi_io->tfp->spitfp->recv_buf, (uint8_t*)ret_uid, 8);} else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 8); }
        tf_packetbuffer_pop_n(&multi_io->tfp->spitfp->recv_buf, (uint8_t*)tmp_connected_uid, 8);
        if (ret_position != NULL) { *ret_position = tf_packetbuffer_read_char(&multi_io->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 1); }
        if (ret_hardware_version != NULL) { for (i = 0; i < 3; ++i) ret_hardware_version[i] = tf_packetbuffer_read_uint8_t(&multi_io->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 3); }
        if (ret_firmware_version != NULL) { for (i = 0; i < 3; ++i) ret_firmware_version[i] = tf_packetbuffer_read_uint8_t(&multi_io->tfp->spitfp->recv_buf);} else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 3); }
        if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packetbuffer_read_uint16_t(&multi_io->tfp->spitfp->recv_buf); } else { tf_packetbuffer_remove(&multi_io->tfp->spitfp->recv_buf, 2); }
        if (tmp_connected_uid[0] == 0 && ret_position != NULL) {
            *ret_position = tf_hal_get_port_name((TF_HalContext*)multi_io->tfp->hal, multi_io->tfp->spitfp->port_id);
        }
        if (ret_connected_uid != NULL) {
            memcpy(ret_connected_uid, tmp_connected_uid, 8);
        }
        tf_tfp_packet_processed(multi_io->tfp);
    }

    result = tf_tfp_finish_send(multi_io->tfp, result, deadline);
    if(result < 0)
        return result;

    return tf_tfp_get_error(error_code);
}


int tf_multi_io_callback_tick(TF_MultiIO *multi_io, uint32_t timeout_us) {
    if (multi_io == NULL)
        return TF_E_NULL;

    return tf_tfp_callback_tick(multi_io->tfp, tf_hal_current_time_us((TF_HalContext*)multi_io->tfp->hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
