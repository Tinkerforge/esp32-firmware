/* ***********************************************************
 * This file was automatically generated on 2021-11-26.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.0         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_one_wire.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


static bool tf_one_wire_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    (void)device;
    (void)fid;
    (void)payload;

    return false;
}
int tf_one_wire_create(TF_OneWire *one_wire, const char *uid, TF_HAL *hal) {
    if (one_wire == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    static uint16_t next_tfp_index = 0;

    memset(one_wire, 0, sizeof(TF_OneWire));

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

        if (tfp->device_id != TF_ONE_WIRE_DEVICE_IDENTIFIER) {
            return TF_E_WRONG_DEVICE_TYPE;
        }
    } else {
        uint16_t device_id = TF_ONE_WIRE_DEVICE_IDENTIFIER;

        tfp = tf_hal_get_tfp(hal, &next_tfp_index, NULL, NULL, &device_id);

        if (tfp == NULL) {
            return TF_E_DEVICE_NOT_FOUND;
        }
    }

    if (tfp->device != NULL) {
        return TF_E_DEVICE_ALREADY_IN_USE;
    }

    one_wire->tfp = tfp;
    one_wire->tfp->device = one_wire;
    one_wire->tfp->cb_handler = tf_one_wire_callback_handler;
    one_wire->response_expected[0] = 0x00;

    return TF_E_OK;
}

int tf_one_wire_destroy(TF_OneWire *one_wire) {
    if (one_wire == NULL || one_wire->tfp == NULL) {
        return TF_E_NULL;
    }

    one_wire->tfp->cb_handler = NULL;
    one_wire->tfp->device = NULL;
    one_wire->tfp = NULL;

    return TF_E_OK;
}

int tf_one_wire_get_response_expected(TF_OneWire *one_wire, uint8_t function_id, bool *ret_response_expected) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    switch (function_id) {
        case TF_ONE_WIRE_FUNCTION_SET_COMMUNICATION_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (one_wire->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_ONE_WIRE_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (one_wire->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_ONE_WIRE_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (one_wire->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_ONE_WIRE_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (one_wire->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_ONE_WIRE_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (one_wire->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_one_wire_set_response_expected(TF_OneWire *one_wire, uint8_t function_id, bool response_expected) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    switch (function_id) {
        case TF_ONE_WIRE_FUNCTION_SET_COMMUNICATION_LED_CONFIG:
            if (response_expected) {
                one_wire->response_expected[0] |= (1 << 0);
            } else {
                one_wire->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_ONE_WIRE_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                one_wire->response_expected[0] |= (1 << 1);
            } else {
                one_wire->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_ONE_WIRE_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                one_wire->response_expected[0] |= (1 << 2);
            } else {
                one_wire->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_ONE_WIRE_FUNCTION_RESET:
            if (response_expected) {
                one_wire->response_expected[0] |= (1 << 3);
            } else {
                one_wire->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_ONE_WIRE_FUNCTION_WRITE_UID:
            if (response_expected) {
                one_wire->response_expected[0] |= (1 << 4);
            } else {
                one_wire->response_expected[0] &= ~(1 << 4);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_one_wire_set_response_expected_all(TF_OneWire *one_wire, bool response_expected) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    memset(one_wire->response_expected, response_expected ? 0xFF : 0, 1);

    return TF_E_OK;
}

int tf_one_wire_search_bus_low_level(TF_OneWire *one_wire, uint16_t *ret_identifier_length, uint16_t *ret_identifier_chunk_offset, uint64_t ret_identifier_chunk_data[7], uint8_t *ret_status) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_SEARCH_BUS_LOW_LEVEL, 0, 61, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_identifier_length != NULL) { *ret_identifier_length = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        if (ret_identifier_chunk_offset != NULL) { *ret_identifier_chunk_offset = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        if (ret_identifier_chunk_data != NULL) { for (i = 0; i < 7; ++i) ret_identifier_chunk_data[i] = tf_packet_buffer_read_uint64_t(recv_buf);} else { tf_packet_buffer_remove(recv_buf, 56); }
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_reset_bus(TF_OneWire *one_wire, uint8_t *ret_status) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_RESET_BUS, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_write(TF_OneWire *one_wire, uint8_t data, uint8_t *ret_status) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_WRITE, 1, 1, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(one_wire->tfp);

    send_buf[0] = (uint8_t)data;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_read(TF_OneWire *one_wire, uint8_t *ret_data, uint8_t *ret_status) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_READ, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_data != NULL) { *ret_data = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_write_command(TF_OneWire *one_wire, uint64_t identifier, uint8_t command, uint8_t *ret_status) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_WRITE_COMMAND, 9, 1, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(one_wire->tfp);

    identifier = tf_leconvert_uint64_to(identifier); memcpy(send_buf + 0, &identifier, 8);
    send_buf[8] = (uint8_t)command;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_set_communication_led_config(TF_OneWire *one_wire, uint8_t config) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_one_wire_get_response_expected(one_wire, TF_ONE_WIRE_FUNCTION_SET_COMMUNICATION_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_SET_COMMUNICATION_LED_CONFIG, 1, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(one_wire->tfp);

    send_buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_get_communication_led_config(TF_OneWire *one_wire, uint8_t *ret_config) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_GET_COMMUNICATION_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_get_spitfp_error_count(TF_OneWire *one_wire, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, 16, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_set_bootloader_mode(TF_OneWire *one_wire, uint8_t mode, uint8_t *ret_status) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_SET_BOOTLOADER_MODE, 1, 1, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(one_wire->tfp);

    send_buf[0] = (uint8_t)mode;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_get_bootloader_mode(TF_OneWire *one_wire, uint8_t *ret_mode) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_GET_BOOTLOADER_MODE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_set_write_firmware_pointer(TF_OneWire *one_wire, uint32_t pointer) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_one_wire_get_response_expected(one_wire, TF_ONE_WIRE_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &response_expected);
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(one_wire->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(send_buf + 0, &pointer, 4);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_write_firmware(TF_OneWire *one_wire, const uint8_t data[64], uint8_t *ret_status) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_WRITE_FIRMWARE, 64, 1, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(one_wire->tfp);

    memcpy(send_buf + 0, data, 64);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_set_status_led_config(TF_OneWire *one_wire, uint8_t config) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_one_wire_get_response_expected(one_wire, TF_ONE_WIRE_FUNCTION_SET_STATUS_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_SET_STATUS_LED_CONFIG, 1, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(one_wire->tfp);

    send_buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_get_status_led_config(TF_OneWire *one_wire, uint8_t *ret_config) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_GET_STATUS_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_get_chip_temperature(TF_OneWire *one_wire, int16_t *ret_temperature) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_GET_CHIP_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_reset(TF_OneWire *one_wire) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_one_wire_get_response_expected(one_wire, TF_ONE_WIRE_FUNCTION_RESET, &response_expected);
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_RESET, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_write_uid(TF_OneWire *one_wire, uint32_t uid) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_one_wire_get_response_expected(one_wire, TF_ONE_WIRE_FUNCTION_WRITE_UID, &response_expected);
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_WRITE_UID, 4, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(one_wire->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(send_buf + 0, &uid, 4);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_read_uid(TF_OneWire *one_wire, uint32_t *ret_uid) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_READ_UID, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_one_wire_get_identity(TF_OneWire *one_wire, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(one_wire->tfp, TF_ONE_WIRE_FUNCTION_GET_IDENTITY, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(one_wire->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(one_wire->tfp);
        if (ret_uid != NULL) { tf_packet_buffer_pop_n(recv_buf, (uint8_t *)ret_uid, 8);} else { tf_packet_buffer_remove(recv_buf, 8); }
        if (ret_connected_uid != NULL) { tf_packet_buffer_pop_n(recv_buf, (uint8_t *)ret_connected_uid, 8);} else { tf_packet_buffer_remove(recv_buf, 8); }
        if (ret_position != NULL) { *ret_position = tf_packet_buffer_read_char(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_hardware_version != NULL) { for (i = 0; i < 3; ++i) ret_hardware_version[i] = tf_packet_buffer_read_uint8_t(recv_buf);} else { tf_packet_buffer_remove(recv_buf, 3); }
        if (ret_firmware_version != NULL) { for (i = 0; i < 3; ++i) ret_firmware_version[i] = tf_packet_buffer_read_uint8_t(recv_buf);} else { tf_packet_buffer_remove(recv_buf, 3); }
        if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(one_wire->tfp);
    }

    result = tf_tfp_finish_send(one_wire->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

typedef struct TF_OneWire_SearchBusLLWrapperData {
    uint8_t *ret_status;
} TF_OneWire_SearchBusLLWrapperData;


static int tf_one_wire_search_bus_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    TF_OneWire_SearchBusLLWrapperData *data = (TF_OneWire_SearchBusLLWrapperData *) wrapper_data;
    uint16_t identifier_length = 0;
    uint16_t identifier_chunk_offset = 0;
    uint64_t *identifier_chunk_data = (uint64_t *) chunk_data;
    int ret = tf_one_wire_search_bus_low_level((TF_OneWire *)device, &identifier_length, &identifier_chunk_offset, identifier_chunk_data, data->ret_status);

    *ret_stream_length = (uint32_t)identifier_length;
    *ret_chunk_offset = (uint32_t)identifier_chunk_offset;
    return ret;
}

int tf_one_wire_search_bus(TF_OneWire *one_wire, uint64_t *ret_identifier, uint16_t *ret_identifier_length, uint8_t *ret_status) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }
    
    TF_OneWire_SearchBusLLWrapperData wrapper_data;
    memset(&wrapper_data, 0, sizeof(wrapper_data));
    wrapper_data.ret_status = ret_status;
    uint32_t identifier_length = 0;
    uint64_t identifier_chunk_data[7];

    int ret = tf_stream_out(one_wire, tf_one_wire_search_bus_ll_wrapper, &wrapper_data, ret_identifier, &identifier_length, identifier_chunk_data, 7, tf_copy_items_uint64_t);

    if (ret_identifier_length != NULL) {
        *ret_identifier_length = (uint16_t)identifier_length;
    }
    return ret;
}


int tf_one_wire_callback_tick(TF_OneWire *one_wire, uint32_t timeout_us) {
    if (one_wire == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = one_wire->tfp->spitfp->hal;

    return tf_tfp_callback_tick(one_wire->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
