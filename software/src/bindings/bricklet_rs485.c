/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_rs485.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_rs485_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_RS485 *rs485 = (TF_RS485 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(rs485->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_RS485_CALLBACK_READ_LOW_LEVEL: {
            TF_RS485_ReadLowLevelHandler fn = rs485->read_low_level_handler;
            void *user_data = rs485->read_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint16_t message_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t message_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            char message_chunk_data[60]; for (_i = 0; _i < 60; ++_i) message_chunk_data[_i] = tf_packet_buffer_read_char(payload);
            hal_common->locked = true;
            fn(rs485, message_length, message_chunk_offset, message_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_ERROR_COUNT: {
            TF_RS485_ErrorCountHandler fn = rs485->error_count_handler;
            void *user_data = rs485->error_count_user_data;
            if (fn == NULL) {
                return false;
            }

            uint32_t overrun_error_count = tf_packet_buffer_read_uint32_t(payload);
            uint32_t parity_error_count = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(rs485, overrun_error_count, parity_error_count, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_SLAVE_READ_COILS_REQUEST: {
            TF_RS485_ModbusSlaveReadCoilsRequestHandler fn = rs485->modbus_slave_read_coils_request_handler;
            void *user_data = rs485->modbus_slave_read_coils_request_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            uint32_t starting_address = tf_packet_buffer_read_uint32_t(payload);
            uint16_t count = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(rs485, request_id, starting_address, count, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_MASTER_READ_COILS_RESPONSE_LOW_LEVEL: {
            TF_RS485_ModbusMasterReadCoilsResponseLowLevelHandler fn = rs485->modbus_master_read_coils_response_low_level_handler;
            void *user_data = rs485->modbus_master_read_coils_response_low_level_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            int8_t exception_code = tf_packet_buffer_read_int8_t(payload);
            uint16_t coils_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t coils_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            bool coils_chunk_data[464]; tf_packet_buffer_read_bool_array(payload, coils_chunk_data, 464);
            hal_common->locked = true;
            fn(rs485, request_id, exception_code, coils_length, coils_chunk_offset, coils_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_SLAVE_READ_HOLDING_REGISTERS_REQUEST: {
            TF_RS485_ModbusSlaveReadHoldingRegistersRequestHandler fn = rs485->modbus_slave_read_holding_registers_request_handler;
            void *user_data = rs485->modbus_slave_read_holding_registers_request_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            uint32_t starting_address = tf_packet_buffer_read_uint32_t(payload);
            uint16_t count = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(rs485, request_id, starting_address, count, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_MASTER_READ_HOLDING_REGISTERS_RESPONSE_LOW_LEVEL: {
            TF_RS485_ModbusMasterReadHoldingRegistersResponseLowLevelHandler fn = rs485->modbus_master_read_holding_registers_response_low_level_handler;
            void *user_data = rs485->modbus_master_read_holding_registers_response_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            int8_t exception_code = tf_packet_buffer_read_int8_t(payload);
            uint16_t holding_registers_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t holding_registers_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            uint16_t holding_registers_chunk_data[29]; for (_i = 0; _i < 29; ++_i) holding_registers_chunk_data[_i] = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(rs485, request_id, exception_code, holding_registers_length, holding_registers_chunk_offset, holding_registers_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_SLAVE_WRITE_SINGLE_COIL_REQUEST: {
            TF_RS485_ModbusSlaveWriteSingleCoilRequestHandler fn = rs485->modbus_slave_write_single_coil_request_handler;
            void *user_data = rs485->modbus_slave_write_single_coil_request_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            uint32_t coil_address = tf_packet_buffer_read_uint32_t(payload);
            bool coil_value = tf_packet_buffer_read_bool(payload);
            hal_common->locked = true;
            fn(rs485, request_id, coil_address, coil_value, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_MASTER_WRITE_SINGLE_COIL_RESPONSE: {
            TF_RS485_ModbusMasterWriteSingleCoilResponseHandler fn = rs485->modbus_master_write_single_coil_response_handler;
            void *user_data = rs485->modbus_master_write_single_coil_response_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            int8_t exception_code = tf_packet_buffer_read_int8_t(payload);
            hal_common->locked = true;
            fn(rs485, request_id, exception_code, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_SLAVE_WRITE_SINGLE_REGISTER_REQUEST: {
            TF_RS485_ModbusSlaveWriteSingleRegisterRequestHandler fn = rs485->modbus_slave_write_single_register_request_handler;
            void *user_data = rs485->modbus_slave_write_single_register_request_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            uint32_t register_address = tf_packet_buffer_read_uint32_t(payload);
            uint16_t register_value = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(rs485, request_id, register_address, register_value, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_MASTER_WRITE_SINGLE_REGISTER_RESPONSE: {
            TF_RS485_ModbusMasterWriteSingleRegisterResponseHandler fn = rs485->modbus_master_write_single_register_response_handler;
            void *user_data = rs485->modbus_master_write_single_register_response_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            int8_t exception_code = tf_packet_buffer_read_int8_t(payload);
            hal_common->locked = true;
            fn(rs485, request_id, exception_code, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_SLAVE_WRITE_MULTIPLE_COILS_REQUEST_LOW_LEVEL: {
            TF_RS485_ModbusSlaveWriteMultipleCoilsRequestLowLevelHandler fn = rs485->modbus_slave_write_multiple_coils_request_low_level_handler;
            void *user_data = rs485->modbus_slave_write_multiple_coils_request_low_level_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            uint32_t starting_address = tf_packet_buffer_read_uint32_t(payload);
            uint16_t coils_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t coils_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            bool coils_chunk_data[440]; tf_packet_buffer_read_bool_array(payload, coils_chunk_data, 440);
            hal_common->locked = true;
            fn(rs485, request_id, starting_address, coils_length, coils_chunk_offset, coils_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_MASTER_WRITE_MULTIPLE_COILS_RESPONSE: {
            TF_RS485_ModbusMasterWriteMultipleCoilsResponseHandler fn = rs485->modbus_master_write_multiple_coils_response_handler;
            void *user_data = rs485->modbus_master_write_multiple_coils_response_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            int8_t exception_code = tf_packet_buffer_read_int8_t(payload);
            hal_common->locked = true;
            fn(rs485, request_id, exception_code, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_SLAVE_WRITE_MULTIPLE_REGISTERS_REQUEST_LOW_LEVEL: {
            TF_RS485_ModbusSlaveWriteMultipleRegistersRequestLowLevelHandler fn = rs485->modbus_slave_write_multiple_registers_request_low_level_handler;
            void *user_data = rs485->modbus_slave_write_multiple_registers_request_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            uint32_t starting_address = tf_packet_buffer_read_uint32_t(payload);
            uint16_t registers_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t registers_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            uint16_t registers_chunk_data[27]; for (_i = 0; _i < 27; ++_i) registers_chunk_data[_i] = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(rs485, request_id, starting_address, registers_length, registers_chunk_offset, registers_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_MASTER_WRITE_MULTIPLE_REGISTERS_RESPONSE: {
            TF_RS485_ModbusMasterWriteMultipleRegistersResponseHandler fn = rs485->modbus_master_write_multiple_registers_response_handler;
            void *user_data = rs485->modbus_master_write_multiple_registers_response_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            int8_t exception_code = tf_packet_buffer_read_int8_t(payload);
            hal_common->locked = true;
            fn(rs485, request_id, exception_code, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_SLAVE_READ_DISCRETE_INPUTS_REQUEST: {
            TF_RS485_ModbusSlaveReadDiscreteInputsRequestHandler fn = rs485->modbus_slave_read_discrete_inputs_request_handler;
            void *user_data = rs485->modbus_slave_read_discrete_inputs_request_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            uint32_t starting_address = tf_packet_buffer_read_uint32_t(payload);
            uint16_t count = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(rs485, request_id, starting_address, count, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_MASTER_READ_DISCRETE_INPUTS_RESPONSE_LOW_LEVEL: {
            TF_RS485_ModbusMasterReadDiscreteInputsResponseLowLevelHandler fn = rs485->modbus_master_read_discrete_inputs_response_low_level_handler;
            void *user_data = rs485->modbus_master_read_discrete_inputs_response_low_level_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            int8_t exception_code = tf_packet_buffer_read_int8_t(payload);
            uint16_t discrete_inputs_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t discrete_inputs_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            bool discrete_inputs_chunk_data[464]; tf_packet_buffer_read_bool_array(payload, discrete_inputs_chunk_data, 464);
            hal_common->locked = true;
            fn(rs485, request_id, exception_code, discrete_inputs_length, discrete_inputs_chunk_offset, discrete_inputs_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_SLAVE_READ_INPUT_REGISTERS_REQUEST: {
            TF_RS485_ModbusSlaveReadInputRegistersRequestHandler fn = rs485->modbus_slave_read_input_registers_request_handler;
            void *user_data = rs485->modbus_slave_read_input_registers_request_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            uint32_t starting_address = tf_packet_buffer_read_uint32_t(payload);
            uint16_t count = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(rs485, request_id, starting_address, count, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_MODBUS_MASTER_READ_INPUT_REGISTERS_RESPONSE_LOW_LEVEL: {
            TF_RS485_ModbusMasterReadInputRegistersResponseLowLevelHandler fn = rs485->modbus_master_read_input_registers_response_low_level_handler;
            void *user_data = rs485->modbus_master_read_input_registers_response_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint8_t request_id = tf_packet_buffer_read_uint8_t(payload);
            int8_t exception_code = tf_packet_buffer_read_int8_t(payload);
            uint16_t input_registers_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t input_registers_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            uint16_t input_registers_chunk_data[29]; for (_i = 0; _i < 29; ++_i) input_registers_chunk_data[_i] = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(rs485, request_id, exception_code, input_registers_length, input_registers_chunk_offset, input_registers_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS485_CALLBACK_FRAME_READABLE: {
            TF_RS485_FrameReadableHandler fn = rs485->frame_readable_handler;
            void *user_data = rs485->frame_readable_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t frame_count = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(rs485, frame_count, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_rs485_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_rs485_create(TF_RS485 *rs485, const char *uid_or_port_name, TF_HAL *hal) {
    if (rs485 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(rs485, 0, sizeof(TF_RS485));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_RS485_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    rs485->tfp = tfp;
    rs485->tfp->device = rs485;
    rs485->tfp->cb_handler = tf_rs485_callback_handler;
    rs485->magic = 0x5446;
    rs485->response_expected[0] = 0x03;
    rs485->response_expected[1] = 0x1B;
    rs485->response_expected[2] = 0x0E;
    return TF_E_OK;
}

int tf_rs485_destroy(TF_RS485 *rs485) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }
    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->tfp->cb_handler = NULL;
    rs485->tfp->device = NULL;
    rs485->tfp = NULL;
    rs485->magic = 0;

    return TF_E_OK;
}

int tf_rs485_get_response_expected(TF_RS485 *rs485, uint8_t function_id, bool *ret_response_expected) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_RS485_FUNCTION_ENABLE_READ_CALLBACK:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_DISABLE_READ_CALLBACK:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_SET_RS485_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_SET_MODBUS_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_SET_MODE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_SET_COMMUNICATION_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_SET_ERROR_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_SET_BUFFER_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_ENABLE_ERROR_COUNT_CALLBACK:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_DISABLE_ERROR_COUNT_CALLBACK:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_REPORT_EXCEPTION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_COILS_REQUEST_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_HOLDING_REGISTERS_REQUEST_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[1] & (1 << 4)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_COIL_REQUEST:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[1] & (1 << 5)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_REGISTER_REQUEST:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[1] & (1 << 6)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_COILS_REQUEST:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[1] & (1 << 7)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_REGISTERS_REQUEST:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[2] & (1 << 0)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_DISCRETE_INPUTS_REQUEST_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[2] & (1 << 1)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_INPUT_REGISTERS_REQUEST_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[2] & (1 << 2)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[2] & (1 << 3)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[2] & (1 << 4)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[2] & (1 << 5)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[2] & (1 << 6)) != 0;
            }
            break;
        case TF_RS485_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs485->response_expected[2] & (1 << 7)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_rs485_set_response_expected(TF_RS485 *rs485, uint8_t function_id, bool response_expected) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_RS485_FUNCTION_ENABLE_READ_CALLBACK:
            if (response_expected) {
                rs485->response_expected[0] |= (1 << 0);
            } else {
                rs485->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_RS485_FUNCTION_DISABLE_READ_CALLBACK:
            if (response_expected) {
                rs485->response_expected[0] |= (1 << 1);
            } else {
                rs485->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_RS485_FUNCTION_SET_RS485_CONFIGURATION:
            if (response_expected) {
                rs485->response_expected[0] |= (1 << 2);
            } else {
                rs485->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_RS485_FUNCTION_SET_MODBUS_CONFIGURATION:
            if (response_expected) {
                rs485->response_expected[0] |= (1 << 3);
            } else {
                rs485->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_RS485_FUNCTION_SET_MODE:
            if (response_expected) {
                rs485->response_expected[0] |= (1 << 4);
            } else {
                rs485->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_RS485_FUNCTION_SET_COMMUNICATION_LED_CONFIG:
            if (response_expected) {
                rs485->response_expected[0] |= (1 << 5);
            } else {
                rs485->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_RS485_FUNCTION_SET_ERROR_LED_CONFIG:
            if (response_expected) {
                rs485->response_expected[0] |= (1 << 6);
            } else {
                rs485->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_RS485_FUNCTION_SET_BUFFER_CONFIG:
            if (response_expected) {
                rs485->response_expected[0] |= (1 << 7);
            } else {
                rs485->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_RS485_FUNCTION_ENABLE_ERROR_COUNT_CALLBACK:
            if (response_expected) {
                rs485->response_expected[1] |= (1 << 0);
            } else {
                rs485->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_RS485_FUNCTION_DISABLE_ERROR_COUNT_CALLBACK:
            if (response_expected) {
                rs485->response_expected[1] |= (1 << 1);
            } else {
                rs485->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_REPORT_EXCEPTION:
            if (response_expected) {
                rs485->response_expected[1] |= (1 << 2);
            } else {
                rs485->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_COILS_REQUEST_LOW_LEVEL:
            if (response_expected) {
                rs485->response_expected[1] |= (1 << 3);
            } else {
                rs485->response_expected[1] &= ~(1 << 3);
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_HOLDING_REGISTERS_REQUEST_LOW_LEVEL:
            if (response_expected) {
                rs485->response_expected[1] |= (1 << 4);
            } else {
                rs485->response_expected[1] &= ~(1 << 4);
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_COIL_REQUEST:
            if (response_expected) {
                rs485->response_expected[1] |= (1 << 5);
            } else {
                rs485->response_expected[1] &= ~(1 << 5);
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_REGISTER_REQUEST:
            if (response_expected) {
                rs485->response_expected[1] |= (1 << 6);
            } else {
                rs485->response_expected[1] &= ~(1 << 6);
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_COILS_REQUEST:
            if (response_expected) {
                rs485->response_expected[1] |= (1 << 7);
            } else {
                rs485->response_expected[1] &= ~(1 << 7);
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_REGISTERS_REQUEST:
            if (response_expected) {
                rs485->response_expected[2] |= (1 << 0);
            } else {
                rs485->response_expected[2] &= ~(1 << 0);
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_DISCRETE_INPUTS_REQUEST_LOW_LEVEL:
            if (response_expected) {
                rs485->response_expected[2] |= (1 << 1);
            } else {
                rs485->response_expected[2] &= ~(1 << 1);
            }
            break;
        case TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_INPUT_REGISTERS_REQUEST_LOW_LEVEL:
            if (response_expected) {
                rs485->response_expected[2] |= (1 << 2);
            } else {
                rs485->response_expected[2] &= ~(1 << 2);
            }
            break;
        case TF_RS485_FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                rs485->response_expected[2] |= (1 << 3);
            } else {
                rs485->response_expected[2] &= ~(1 << 3);
            }
            break;
        case TF_RS485_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                rs485->response_expected[2] |= (1 << 4);
            } else {
                rs485->response_expected[2] &= ~(1 << 4);
            }
            break;
        case TF_RS485_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                rs485->response_expected[2] |= (1 << 5);
            } else {
                rs485->response_expected[2] &= ~(1 << 5);
            }
            break;
        case TF_RS485_FUNCTION_RESET:
            if (response_expected) {
                rs485->response_expected[2] |= (1 << 6);
            } else {
                rs485->response_expected[2] &= ~(1 << 6);
            }
            break;
        case TF_RS485_FUNCTION_WRITE_UID:
            if (response_expected) {
                rs485->response_expected[2] |= (1 << 7);
            } else {
                rs485->response_expected[2] &= ~(1 << 7);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_rs485_set_response_expected_all(TF_RS485 *rs485, bool response_expected) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(rs485->response_expected, response_expected ? 0xFF : 0, 3);

    return TF_E_OK;
}

int tf_rs485_write_low_level(TF_RS485 *rs485, uint16_t message_length, uint16_t message_chunk_offset, const char message_chunk_data[60], uint8_t *ret_message_chunk_written) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_WRITE_LOW_LEVEL, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    message_length = tf_leconvert_uint16_to(message_length); memcpy(_send_buf + 0, &message_length, 2);
    message_chunk_offset = tf_leconvert_uint16_to(message_chunk_offset); memcpy(_send_buf + 2, &message_chunk_offset, 2);
    memcpy(_send_buf + 4, message_chunk_data, 60);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_message_chunk_written != NULL) { *ret_message_chunk_written = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_read_low_level(TF_RS485 *rs485, uint16_t length, uint16_t *ret_message_length, uint16_t *ret_message_chunk_offset, char ret_message_chunk_data[60]) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_READ_LOW_LEVEL, 2, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    length = tf_leconvert_uint16_to(length); memcpy(_send_buf + 0, &length, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_message_length != NULL) { *ret_message_length = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_message_chunk_offset != NULL) { *ret_message_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_message_chunk_data != NULL) { for (_i = 0; _i < 60; ++_i) ret_message_chunk_data[_i] = tf_packet_buffer_read_char(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_enable_read_callback(TF_RS485 *rs485) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_ENABLE_READ_CALLBACK, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_ENABLE_READ_CALLBACK, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_disable_read_callback(TF_RS485 *rs485) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_DISABLE_READ_CALLBACK, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_DISABLE_READ_CALLBACK, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_is_read_callback_enabled(TF_RS485 *rs485, bool *ret_enabled) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_IS_READ_CALLBACK_ENABLED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_set_rs485_configuration(TF_RS485 *rs485, uint32_t baudrate, uint8_t parity, uint8_t stopbits, uint8_t wordlength, uint8_t duplex) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_SET_RS485_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_SET_RS485_CONFIGURATION, 8, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    baudrate = tf_leconvert_uint32_to(baudrate); memcpy(_send_buf + 0, &baudrate, 4);
    _send_buf[4] = (uint8_t)parity;
    _send_buf[5] = (uint8_t)stopbits;
    _send_buf[6] = (uint8_t)wordlength;
    _send_buf[7] = (uint8_t)duplex;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_rs485_configuration(TF_RS485 *rs485, uint32_t *ret_baudrate, uint8_t *ret_parity, uint8_t *ret_stopbits, uint8_t *ret_wordlength, uint8_t *ret_duplex) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_RS485_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_baudrate != NULL) { *ret_baudrate = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_parity != NULL) { *ret_parity = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_stopbits != NULL) { *ret_stopbits = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_wordlength != NULL) { *ret_wordlength = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_duplex != NULL) { *ret_duplex = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_set_modbus_configuration(TF_RS485 *rs485, uint8_t slave_address, uint32_t master_request_timeout) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_SET_MODBUS_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_SET_MODBUS_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)slave_address;
    master_request_timeout = tf_leconvert_uint32_to(master_request_timeout); memcpy(_send_buf + 1, &master_request_timeout, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_modbus_configuration(TF_RS485 *rs485, uint8_t *ret_slave_address, uint32_t *ret_master_request_timeout) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_MODBUS_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_slave_address != NULL) { *ret_slave_address = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_master_request_timeout != NULL) { *ret_master_request_timeout = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_set_mode(TF_RS485 *rs485, uint8_t mode) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_SET_MODE, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_SET_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_mode(TF_RS485 *rs485, uint8_t *ret_mode) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_set_communication_led_config(TF_RS485 *rs485, uint8_t config) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_SET_COMMUNICATION_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_SET_COMMUNICATION_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_communication_led_config(TF_RS485 *rs485, uint8_t *ret_config) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_COMMUNICATION_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_set_error_led_config(TF_RS485 *rs485, uint8_t config) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_SET_ERROR_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_SET_ERROR_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_error_led_config(TF_RS485 *rs485, uint8_t *ret_config) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_ERROR_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_set_buffer_config(TF_RS485 *rs485, uint16_t send_buffer_size, uint16_t receive_buffer_size) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_SET_BUFFER_CONFIG, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_SET_BUFFER_CONFIG, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    send_buffer_size = tf_leconvert_uint16_to(send_buffer_size); memcpy(_send_buf + 0, &send_buffer_size, 2);
    receive_buffer_size = tf_leconvert_uint16_to(receive_buffer_size); memcpy(_send_buf + 2, &receive_buffer_size, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_buffer_config(TF_RS485 *rs485, uint16_t *ret_send_buffer_size, uint16_t *ret_receive_buffer_size) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_BUFFER_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_send_buffer_size != NULL) { *ret_send_buffer_size = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_receive_buffer_size != NULL) { *ret_receive_buffer_size = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_buffer_status(TF_RS485 *rs485, uint16_t *ret_send_buffer_used, uint16_t *ret_receive_buffer_used) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_BUFFER_STATUS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_send_buffer_used != NULL) { *ret_send_buffer_used = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_receive_buffer_used != NULL) { *ret_receive_buffer_used = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_enable_error_count_callback(TF_RS485 *rs485) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_ENABLE_ERROR_COUNT_CALLBACK, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_ENABLE_ERROR_COUNT_CALLBACK, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_disable_error_count_callback(TF_RS485 *rs485) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_DISABLE_ERROR_COUNT_CALLBACK, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_DISABLE_ERROR_COUNT_CALLBACK, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_is_error_count_callback_enabled(TF_RS485 *rs485, bool *ret_enabled) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_IS_ERROR_COUNT_CALLBACK_ENABLED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_error_count(TF_RS485 *rs485, uint32_t *ret_overrun_error_count, uint32_t *ret_parity_error_count) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_overrun_error_count != NULL) { *ret_overrun_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_parity_error_count != NULL) { *ret_parity_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_modbus_common_error_count(TF_RS485 *rs485, uint32_t *ret_timeout_error_count, uint32_t *ret_checksum_error_count, uint32_t *ret_frame_too_big_error_count, uint32_t *ret_illegal_function_error_count, uint32_t *ret_illegal_data_address_error_count, uint32_t *ret_illegal_data_value_error_count, uint32_t *ret_slave_device_failure_error_count) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_MODBUS_COMMON_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 28) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_timeout_error_count != NULL) { *ret_timeout_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_checksum_error_count != NULL) { *ret_checksum_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_frame_too_big_error_count != NULL) { *ret_frame_too_big_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_illegal_function_error_count != NULL) { *ret_illegal_function_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_illegal_data_address_error_count != NULL) { *ret_illegal_data_address_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_illegal_data_value_error_count != NULL) { *ret_illegal_data_value_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_slave_device_failure_error_count != NULL) { *ret_slave_device_failure_error_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 28) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_slave_report_exception(TF_RS485 *rs485, uint8_t request_id, int8_t exception_code) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_MODBUS_SLAVE_REPORT_EXCEPTION, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_SLAVE_REPORT_EXCEPTION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)request_id;
    _send_buf[1] = (uint8_t)exception_code;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_slave_answer_read_coils_request_low_level(TF_RS485 *rs485, uint8_t request_id, uint16_t coils_length, uint16_t coils_chunk_offset, const bool coils_chunk_data[472]) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_COILS_REQUEST_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_COILS_REQUEST_LOW_LEVEL, 64, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)request_id;
    coils_length = tf_leconvert_uint16_to(coils_length); memcpy(_send_buf + 1, &coils_length, 2);
    coils_chunk_offset = tf_leconvert_uint16_to(coils_chunk_offset); memcpy(_send_buf + 3, &coils_chunk_offset, 2);
    memset(_send_buf + 5, 0, 59); for (_i = 0; _i < 472; ++_i) _send_buf[5 + (_i / 8)] |= (coils_chunk_data[_i] ? 1 : 0) << (_i % 8);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_master_read_coils(TF_RS485 *rs485, uint8_t slave_address, uint32_t starting_address, uint16_t count, uint8_t *ret_request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_MASTER_READ_COILS, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)slave_address;
    starting_address = tf_leconvert_uint32_to(starting_address); memcpy(_send_buf + 1, &starting_address, 4);
    count = tf_leconvert_uint16_to(count); memcpy(_send_buf + 5, &count, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_request_id != NULL) { *ret_request_id = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_slave_answer_read_holding_registers_request_low_level(TF_RS485 *rs485, uint8_t request_id, uint16_t holding_registers_length, uint16_t holding_registers_chunk_offset, const uint16_t holding_registers_chunk_data[29]) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_HOLDING_REGISTERS_REQUEST_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_HOLDING_REGISTERS_REQUEST_LOW_LEVEL, 63, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)request_id;
    holding_registers_length = tf_leconvert_uint16_to(holding_registers_length); memcpy(_send_buf + 1, &holding_registers_length, 2);
    holding_registers_chunk_offset = tf_leconvert_uint16_to(holding_registers_chunk_offset); memcpy(_send_buf + 3, &holding_registers_chunk_offset, 2);
    for (_i = 0; _i < 29; _i++) { uint16_t tmp_holding_registers_chunk_data = tf_leconvert_uint16_to(holding_registers_chunk_data[_i]); memcpy(_send_buf + 5 + (_i * sizeof(uint16_t)), &tmp_holding_registers_chunk_data, sizeof(uint16_t)); }

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_master_read_holding_registers(TF_RS485 *rs485, uint8_t slave_address, uint32_t starting_address, uint16_t count, uint8_t *ret_request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_MASTER_READ_HOLDING_REGISTERS, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)slave_address;
    starting_address = tf_leconvert_uint32_to(starting_address); memcpy(_send_buf + 1, &starting_address, 4);
    count = tf_leconvert_uint16_to(count); memcpy(_send_buf + 5, &count, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_request_id != NULL) { *ret_request_id = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_slave_answer_write_single_coil_request(TF_RS485 *rs485, uint8_t request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_COIL_REQUEST, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_COIL_REQUEST, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)request_id;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_master_write_single_coil(TF_RS485 *rs485, uint8_t slave_address, uint32_t coil_address, bool coil_value, uint8_t *ret_request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_MASTER_WRITE_SINGLE_COIL, 6, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)slave_address;
    coil_address = tf_leconvert_uint32_to(coil_address); memcpy(_send_buf + 1, &coil_address, 4);
    _send_buf[5] = coil_value ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_request_id != NULL) { *ret_request_id = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_slave_answer_write_single_register_request(TF_RS485 *rs485, uint8_t request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_REGISTER_REQUEST, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_REGISTER_REQUEST, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)request_id;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_master_write_single_register(TF_RS485 *rs485, uint8_t slave_address, uint32_t register_address, uint16_t register_value, uint8_t *ret_request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_MASTER_WRITE_SINGLE_REGISTER, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)slave_address;
    register_address = tf_leconvert_uint32_to(register_address); memcpy(_send_buf + 1, &register_address, 4);
    register_value = tf_leconvert_uint16_to(register_value); memcpy(_send_buf + 5, &register_value, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_request_id != NULL) { *ret_request_id = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_slave_answer_write_multiple_coils_request(TF_RS485 *rs485, uint8_t request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_COILS_REQUEST, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_COILS_REQUEST, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)request_id;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_master_write_multiple_coils_low_level(TF_RS485 *rs485, uint8_t slave_address, uint32_t starting_address, uint16_t coils_length, uint16_t coils_chunk_offset, const bool coils_chunk_data[440], uint8_t *ret_request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_MASTER_WRITE_MULTIPLE_COILS_LOW_LEVEL, 64, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)slave_address;
    starting_address = tf_leconvert_uint32_to(starting_address); memcpy(_send_buf + 1, &starting_address, 4);
    coils_length = tf_leconvert_uint16_to(coils_length); memcpy(_send_buf + 5, &coils_length, 2);
    coils_chunk_offset = tf_leconvert_uint16_to(coils_chunk_offset); memcpy(_send_buf + 7, &coils_chunk_offset, 2);
    memset(_send_buf + 9, 0, 55); for (_i = 0; _i < 440; ++_i) _send_buf[9 + (_i / 8)] |= (coils_chunk_data[_i] ? 1 : 0) << (_i % 8);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_request_id != NULL) { *ret_request_id = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_slave_answer_write_multiple_registers_request(TF_RS485 *rs485, uint8_t request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_REGISTERS_REQUEST, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_REGISTERS_REQUEST, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)request_id;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_master_write_multiple_registers_low_level(TF_RS485 *rs485, uint8_t slave_address, uint32_t starting_address, uint16_t registers_length, uint16_t registers_chunk_offset, const uint16_t registers_chunk_data[27], uint8_t *ret_request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_MASTER_WRITE_MULTIPLE_REGISTERS_LOW_LEVEL, 63, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)slave_address;
    starting_address = tf_leconvert_uint32_to(starting_address); memcpy(_send_buf + 1, &starting_address, 4);
    registers_length = tf_leconvert_uint16_to(registers_length); memcpy(_send_buf + 5, &registers_length, 2);
    registers_chunk_offset = tf_leconvert_uint16_to(registers_chunk_offset); memcpy(_send_buf + 7, &registers_chunk_offset, 2);
    for (_i = 0; _i < 27; _i++) { uint16_t tmp_registers_chunk_data = tf_leconvert_uint16_to(registers_chunk_data[_i]); memcpy(_send_buf + 9 + (_i * sizeof(uint16_t)), &tmp_registers_chunk_data, sizeof(uint16_t)); }

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_request_id != NULL) { *ret_request_id = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_slave_answer_read_discrete_inputs_request_low_level(TF_RS485 *rs485, uint8_t request_id, uint16_t discrete_inputs_length, uint16_t discrete_inputs_chunk_offset, const bool discrete_inputs_chunk_data[472]) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_DISCRETE_INPUTS_REQUEST_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_DISCRETE_INPUTS_REQUEST_LOW_LEVEL, 64, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)request_id;
    discrete_inputs_length = tf_leconvert_uint16_to(discrete_inputs_length); memcpy(_send_buf + 1, &discrete_inputs_length, 2);
    discrete_inputs_chunk_offset = tf_leconvert_uint16_to(discrete_inputs_chunk_offset); memcpy(_send_buf + 3, &discrete_inputs_chunk_offset, 2);
    memset(_send_buf + 5, 0, 59); for (_i = 0; _i < 472; ++_i) _send_buf[5 + (_i / 8)] |= (discrete_inputs_chunk_data[_i] ? 1 : 0) << (_i % 8);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_master_read_discrete_inputs(TF_RS485 *rs485, uint8_t slave_address, uint32_t starting_address, uint16_t count, uint8_t *ret_request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_MASTER_READ_DISCRETE_INPUTS, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)slave_address;
    starting_address = tf_leconvert_uint32_to(starting_address); memcpy(_send_buf + 1, &starting_address, 4);
    count = tf_leconvert_uint16_to(count); memcpy(_send_buf + 5, &count, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_request_id != NULL) { *ret_request_id = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_slave_answer_read_input_registers_request_low_level(TF_RS485 *rs485, uint8_t request_id, uint16_t input_registers_length, uint16_t input_registers_chunk_offset, const uint16_t input_registers_chunk_data[29]) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_INPUT_REGISTERS_REQUEST_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_SLAVE_ANSWER_READ_INPUT_REGISTERS_REQUEST_LOW_LEVEL, 63, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)request_id;
    input_registers_length = tf_leconvert_uint16_to(input_registers_length); memcpy(_send_buf + 1, &input_registers_length, 2);
    input_registers_chunk_offset = tf_leconvert_uint16_to(input_registers_chunk_offset); memcpy(_send_buf + 3, &input_registers_chunk_offset, 2);
    for (_i = 0; _i < 29; _i++) { uint16_t tmp_input_registers_chunk_data = tf_leconvert_uint16_to(input_registers_chunk_data[_i]); memcpy(_send_buf + 5 + (_i * sizeof(uint16_t)), &tmp_input_registers_chunk_data, sizeof(uint16_t)); }

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_modbus_master_read_input_registers(TF_RS485 *rs485, uint8_t slave_address, uint32_t starting_address, uint16_t count, uint8_t *ret_request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_MODBUS_MASTER_READ_INPUT_REGISTERS, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)slave_address;
    starting_address = tf_leconvert_uint32_to(starting_address); memcpy(_send_buf + 1, &starting_address, 4);
    count = tf_leconvert_uint16_to(count); memcpy(_send_buf + 5, &count, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_request_id != NULL) { *ret_request_id = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_set_frame_readable_callback_configuration(TF_RS485 *rs485, uint16_t frame_size) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    frame_size = tf_leconvert_uint16_to(frame_size); memcpy(_send_buf + 0, &frame_size, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_frame_readable_callback_configuration(TF_RS485 *rs485, uint16_t *ret_frame_size) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_FRAME_READABLE_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_frame_size != NULL) { *ret_frame_size = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_spitfp_error_count(TF_RS485 *rs485, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_set_bootloader_mode(TF_RS485 *rs485, uint8_t mode, uint8_t *ret_status) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_bootloader_mode(TF_RS485 *rs485, uint8_t *ret_mode) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_set_write_firmware_pointer(TF_RS485 *rs485, uint32_t pointer) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_write_firmware(TF_RS485 *rs485, const uint8_t data[64], uint8_t *ret_status) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_set_status_led_config(TF_RS485 *rs485, uint8_t config) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_status_led_config(TF_RS485 *rs485, uint8_t *ret_config) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_chip_temperature(TF_RS485 *rs485, int16_t *ret_temperature) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_reset(TF_RS485 *rs485) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_write_uid(TF_RS485 *rs485, uint32_t uid) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs485_get_response_expected(rs485, TF_RS485_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs485->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_read_uid(TF_RS485 *rs485, uint32_t *ret_uid) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs485_get_identity(TF_RS485 *rs485, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs485->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs485->tfp, TF_RS485_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs485->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs485->tfp);
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
        tf_tfp_packet_processed(rs485->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs485->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

static int tf_rs485_write_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    (void)wrapper_data;
    uint16_t message_chunk_offset = (uint16_t)chunk_offset;
    uint16_t message_length = (uint16_t)stream_length;
    uint8_t message_chunk_written = 60;

    char *message_chunk_data = (char *) chunk_data;
    int ret = tf_rs485_write_low_level((TF_RS485 *)device, message_length, message_chunk_offset, message_chunk_data, &message_chunk_written);

    *ret_chunk_written = (uint32_t) message_chunk_written;
    return ret;
}

int tf_rs485_write(TF_RS485 *rs485, const char *message, uint16_t message_length, uint16_t *ret_message_written) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }


    uint32_t _stream_length = message_length;
    uint32_t _message_written = 0;
    char _chunk_data[60];

    int ret = tf_stream_in(rs485, tf_rs485_write_ll_wrapper, NULL, message, _stream_length, _chunk_data, &_message_written, 60, tf_copy_items_char);

    if (ret_message_written != NULL) {
        *ret_message_written = (uint16_t) _message_written;
    }

    return ret;
}


typedef struct TF_RS485_ReadLLWrapperData {
    uint16_t length;
} TF_RS485_ReadLLWrapperData;


static int tf_rs485_read_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    TF_RS485_ReadLLWrapperData *data = (TF_RS485_ReadLLWrapperData *) wrapper_data;
    uint16_t message_length = 0;
    uint16_t message_chunk_offset = 0;
    char *message_chunk_data = (char *) chunk_data;
    int ret = tf_rs485_read_low_level((TF_RS485 *)device, data->length, &message_length, &message_chunk_offset, message_chunk_data);

    *ret_stream_length = (uint32_t)message_length;
    *ret_chunk_offset = (uint32_t)message_chunk_offset;
    return ret;
}

int tf_rs485_read(TF_RS485 *rs485, uint16_t length, char *ret_message, uint16_t *ret_message_length) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_RS485_ReadLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.length = length;
    uint32_t _message_length = 0;
    char _message_chunk_data[60];

    int ret = tf_stream_out(rs485, tf_rs485_read_ll_wrapper, &_wrapper_data, ret_message, &_message_length, _message_chunk_data, 60, tf_copy_items_char);

    if (ret_message_length != NULL) {
        *ret_message_length = (uint16_t)_message_length;
    }
    return ret;
}

typedef struct TF_RS485_ModbusSlaveAnswerReadCoilsRequestLLWrapperData {
    uint8_t request_id;
} TF_RS485_ModbusSlaveAnswerReadCoilsRequestLLWrapperData;


static int tf_rs485_modbus_slave_answer_read_coils_request_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    TF_RS485_ModbusSlaveAnswerReadCoilsRequestLLWrapperData *data = (TF_RS485_ModbusSlaveAnswerReadCoilsRequestLLWrapperData *) wrapper_data;
    uint16_t coils_chunk_offset = (uint16_t)chunk_offset;
    uint16_t coils_length = (uint16_t)stream_length;
    uint32_t coils_chunk_written = 472;

    bool *coils_chunk_data = (bool *) chunk_data;
    int ret = tf_rs485_modbus_slave_answer_read_coils_request_low_level((TF_RS485 *)device, data->request_id, coils_length, coils_chunk_offset, coils_chunk_data);

    *ret_chunk_written = (uint32_t) coils_chunk_written;
    return ret;
}

int tf_rs485_modbus_slave_answer_read_coils_request(TF_RS485 *rs485, uint8_t request_id, const bool *coils, uint16_t coils_length) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_RS485_ModbusSlaveAnswerReadCoilsRequestLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.request_id = request_id;

    uint32_t _stream_length = coils_length;
    uint32_t _coils_written = 0;
    bool _chunk_data[472];

    int ret = tf_stream_in(rs485, tf_rs485_modbus_slave_answer_read_coils_request_ll_wrapper, &_wrapper_data, coils, _stream_length, _chunk_data, &_coils_written, 472, tf_copy_items_bool);



    return ret;
}


typedef struct TF_RS485_ModbusSlaveAnswerReadHoldingRegistersRequestLLWrapperData {
    uint8_t request_id;
} TF_RS485_ModbusSlaveAnswerReadHoldingRegistersRequestLLWrapperData;


static int tf_rs485_modbus_slave_answer_read_holding_registers_request_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    TF_RS485_ModbusSlaveAnswerReadHoldingRegistersRequestLLWrapperData *data = (TF_RS485_ModbusSlaveAnswerReadHoldingRegistersRequestLLWrapperData *) wrapper_data;
    uint16_t holding_registers_chunk_offset = (uint16_t)chunk_offset;
    uint16_t holding_registers_length = (uint16_t)stream_length;
    uint32_t holding_registers_chunk_written = 29;

    uint16_t *holding_registers_chunk_data = (uint16_t *) chunk_data;
    int ret = tf_rs485_modbus_slave_answer_read_holding_registers_request_low_level((TF_RS485 *)device, data->request_id, holding_registers_length, holding_registers_chunk_offset, holding_registers_chunk_data);

    *ret_chunk_written = (uint32_t) holding_registers_chunk_written;
    return ret;
}

int tf_rs485_modbus_slave_answer_read_holding_registers_request(TF_RS485 *rs485, uint8_t request_id, const uint16_t *holding_registers, uint16_t holding_registers_length) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_RS485_ModbusSlaveAnswerReadHoldingRegistersRequestLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.request_id = request_id;

    uint32_t _stream_length = holding_registers_length;
    uint32_t _holding_registers_written = 0;
    uint16_t _chunk_data[29];

    int ret = tf_stream_in(rs485, tf_rs485_modbus_slave_answer_read_holding_registers_request_ll_wrapper, &_wrapper_data, holding_registers, _stream_length, _chunk_data, &_holding_registers_written, 29, tf_copy_items_uint16_t);



    return ret;
}


typedef struct TF_RS485_ModbusMasterWriteMultipleCoilsLLWrapperData {
    uint8_t slave_address;
    uint32_t starting_address;
    uint8_t *ret_request_id;
} TF_RS485_ModbusMasterWriteMultipleCoilsLLWrapperData;


static int tf_rs485_modbus_master_write_multiple_coils_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    TF_RS485_ModbusMasterWriteMultipleCoilsLLWrapperData *data = (TF_RS485_ModbusMasterWriteMultipleCoilsLLWrapperData *) wrapper_data;
    uint16_t coils_chunk_offset = (uint16_t)chunk_offset;
    uint16_t coils_length = (uint16_t)stream_length;
    uint32_t coils_chunk_written = 440;

    bool *coils_chunk_data = (bool *) chunk_data;
    int ret = tf_rs485_modbus_master_write_multiple_coils_low_level((TF_RS485 *)device, data->slave_address, data->starting_address, coils_length, coils_chunk_offset, coils_chunk_data, data->ret_request_id);

    *ret_chunk_written = (uint32_t) coils_chunk_written;
    return ret;
}

int tf_rs485_modbus_master_write_multiple_coils(TF_RS485 *rs485, uint8_t slave_address, uint32_t starting_address, const bool *coils, uint16_t coils_length, uint8_t *ret_request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_RS485_ModbusMasterWriteMultipleCoilsLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.slave_address = slave_address;
    _wrapper_data.starting_address = starting_address;
    _wrapper_data.ret_request_id = ret_request_id;

    uint32_t _stream_length = coils_length;
    uint32_t _coils_written = 0;
    bool _chunk_data[440];

    int ret = tf_stream_in(rs485, tf_rs485_modbus_master_write_multiple_coils_ll_wrapper, &_wrapper_data, coils, _stream_length, _chunk_data, &_coils_written, 440, tf_copy_items_bool);



    return ret;
}


typedef struct TF_RS485_ModbusMasterWriteMultipleRegistersLLWrapperData {
    uint8_t slave_address;
    uint32_t starting_address;
    uint8_t *ret_request_id;
} TF_RS485_ModbusMasterWriteMultipleRegistersLLWrapperData;


static int tf_rs485_modbus_master_write_multiple_registers_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    TF_RS485_ModbusMasterWriteMultipleRegistersLLWrapperData *data = (TF_RS485_ModbusMasterWriteMultipleRegistersLLWrapperData *) wrapper_data;
    uint16_t registers_chunk_offset = (uint16_t)chunk_offset;
    uint16_t registers_length = (uint16_t)stream_length;
    uint32_t registers_chunk_written = 27;

    uint16_t *registers_chunk_data = (uint16_t *) chunk_data;
    int ret = tf_rs485_modbus_master_write_multiple_registers_low_level((TF_RS485 *)device, data->slave_address, data->starting_address, registers_length, registers_chunk_offset, registers_chunk_data, data->ret_request_id);

    *ret_chunk_written = (uint32_t) registers_chunk_written;
    return ret;
}

int tf_rs485_modbus_master_write_multiple_registers(TF_RS485 *rs485, uint8_t slave_address, uint32_t starting_address, const uint16_t *registers, uint16_t registers_length, uint8_t *ret_request_id) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_RS485_ModbusMasterWriteMultipleRegistersLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.slave_address = slave_address;
    _wrapper_data.starting_address = starting_address;
    _wrapper_data.ret_request_id = ret_request_id;

    uint32_t _stream_length = registers_length;
    uint32_t _registers_written = 0;
    uint16_t _chunk_data[27];

    int ret = tf_stream_in(rs485, tf_rs485_modbus_master_write_multiple_registers_ll_wrapper, &_wrapper_data, registers, _stream_length, _chunk_data, &_registers_written, 27, tf_copy_items_uint16_t);



    return ret;
}


typedef struct TF_RS485_ModbusSlaveAnswerReadDiscreteInputsRequestLLWrapperData {
    uint8_t request_id;
} TF_RS485_ModbusSlaveAnswerReadDiscreteInputsRequestLLWrapperData;


static int tf_rs485_modbus_slave_answer_read_discrete_inputs_request_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    TF_RS485_ModbusSlaveAnswerReadDiscreteInputsRequestLLWrapperData *data = (TF_RS485_ModbusSlaveAnswerReadDiscreteInputsRequestLLWrapperData *) wrapper_data;
    uint16_t discrete_inputs_chunk_offset = (uint16_t)chunk_offset;
    uint16_t discrete_inputs_length = (uint16_t)stream_length;
    uint32_t discrete_inputs_chunk_written = 472;

    bool *discrete_inputs_chunk_data = (bool *) chunk_data;
    int ret = tf_rs485_modbus_slave_answer_read_discrete_inputs_request_low_level((TF_RS485 *)device, data->request_id, discrete_inputs_length, discrete_inputs_chunk_offset, discrete_inputs_chunk_data);

    *ret_chunk_written = (uint32_t) discrete_inputs_chunk_written;
    return ret;
}

int tf_rs485_modbus_slave_answer_read_discrete_inputs_request(TF_RS485 *rs485, uint8_t request_id, const bool *discrete_inputs, uint16_t discrete_inputs_length) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_RS485_ModbusSlaveAnswerReadDiscreteInputsRequestLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.request_id = request_id;

    uint32_t _stream_length = discrete_inputs_length;
    uint32_t _discrete_inputs_written = 0;
    bool _chunk_data[472];

    int ret = tf_stream_in(rs485, tf_rs485_modbus_slave_answer_read_discrete_inputs_request_ll_wrapper, &_wrapper_data, discrete_inputs, _stream_length, _chunk_data, &_discrete_inputs_written, 472, tf_copy_items_bool);



    return ret;
}


typedef struct TF_RS485_ModbusSlaveAnswerReadInputRegistersRequestLLWrapperData {
    uint8_t request_id;
} TF_RS485_ModbusSlaveAnswerReadInputRegistersRequestLLWrapperData;


static int tf_rs485_modbus_slave_answer_read_input_registers_request_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    TF_RS485_ModbusSlaveAnswerReadInputRegistersRequestLLWrapperData *data = (TF_RS485_ModbusSlaveAnswerReadInputRegistersRequestLLWrapperData *) wrapper_data;
    uint16_t input_registers_chunk_offset = (uint16_t)chunk_offset;
    uint16_t input_registers_length = (uint16_t)stream_length;
    uint32_t input_registers_chunk_written = 29;

    uint16_t *input_registers_chunk_data = (uint16_t *) chunk_data;
    int ret = tf_rs485_modbus_slave_answer_read_input_registers_request_low_level((TF_RS485 *)device, data->request_id, input_registers_length, input_registers_chunk_offset, input_registers_chunk_data);

    *ret_chunk_written = (uint32_t) input_registers_chunk_written;
    return ret;
}

int tf_rs485_modbus_slave_answer_read_input_registers_request(TF_RS485 *rs485, uint8_t request_id, const uint16_t *input_registers, uint16_t input_registers_length) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_RS485_ModbusSlaveAnswerReadInputRegistersRequestLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.request_id = request_id;

    uint32_t _stream_length = input_registers_length;
    uint32_t _input_registers_written = 0;
    uint16_t _chunk_data[29];

    int ret = tf_stream_in(rs485, tf_rs485_modbus_slave_answer_read_input_registers_request_ll_wrapper, &_wrapper_data, input_registers, _stream_length, _chunk_data, &_input_registers_written, 29, tf_copy_items_uint16_t);



    return ret;
}

#if TF_IMPLEMENT_CALLBACKS != 0
int tf_rs485_register_read_low_level_callback(TF_RS485 *rs485, TF_RS485_ReadLowLevelHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->read_low_level_handler = handler;
    rs485->read_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_rs485_read_wrapper(TF_RS485 *rs485, uint16_t message_length, uint16_t message_chunk_offset, char message_chunk_data[60], void *user_data) {
    uint32_t stream_length = (uint32_t) message_length;
    uint32_t chunk_offset = (uint32_t) message_chunk_offset;
    if (!tf_stream_out_callback(&rs485->read_hlc, stream_length, chunk_offset, message_chunk_data, 60, tf_copy_items_char)) {
        return;
    }

    // Stream is either complete or out of sync
    char *message = (char *) (rs485->read_hlc.length == 0 ? NULL : rs485->read_hlc.data);
    rs485->read_handler(rs485, message, message_length, user_data);

    rs485->read_hlc.stream_in_progress = false;
    rs485->read_hlc.length = 0;
}

int tf_rs485_register_read_callback(TF_RS485 *rs485, TF_RS485_ReadHandler handler, char *message_buffer, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->read_handler = handler;

    rs485->read_hlc.data = message_buffer;
    rs485->read_hlc.length = 0;
    rs485->read_hlc.stream_in_progress = false;

    return tf_rs485_register_read_low_level_callback(rs485, handler == NULL ? NULL : tf_rs485_read_wrapper, user_data);
}


int tf_rs485_register_error_count_callback(TF_RS485 *rs485, TF_RS485_ErrorCountHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->error_count_handler = handler;
    rs485->error_count_user_data = user_data;

    return TF_E_OK;
}


int tf_rs485_register_modbus_slave_read_coils_request_callback(TF_RS485 *rs485, TF_RS485_ModbusSlaveReadCoilsRequestHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_slave_read_coils_request_handler = handler;
    rs485->modbus_slave_read_coils_request_user_data = user_data;

    return TF_E_OK;
}


int tf_rs485_register_modbus_master_read_coils_response_low_level_callback(TF_RS485 *rs485, TF_RS485_ModbusMasterReadCoilsResponseLowLevelHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_master_read_coils_response_low_level_handler = handler;
    rs485->modbus_master_read_coils_response_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_rs485_modbus_master_read_coils_response_wrapper(TF_RS485 *rs485, uint8_t request_id, int8_t exception_code, uint16_t coils_length, uint16_t coils_chunk_offset, bool coils_chunk_data[464], void *user_data) {
    uint32_t stream_length = (uint32_t) coils_length;
    uint32_t chunk_offset = (uint32_t) coils_chunk_offset;
    if (!tf_stream_out_callback(&rs485->modbus_master_read_coils_response_hlc, stream_length, chunk_offset, coils_chunk_data, 464, tf_copy_items_bool)) {
        return;
    }

    // Stream is either complete or out of sync
    bool *coils = (bool *) (rs485->modbus_master_read_coils_response_hlc.length == 0 ? NULL : rs485->modbus_master_read_coils_response_hlc.data);
    rs485->modbus_master_read_coils_response_handler(rs485, request_id, exception_code, coils, coils_length, user_data);

    rs485->modbus_master_read_coils_response_hlc.stream_in_progress = false;
    rs485->modbus_master_read_coils_response_hlc.length = 0;
}

int tf_rs485_register_modbus_master_read_coils_response_callback(TF_RS485 *rs485, TF_RS485_ModbusMasterReadCoilsResponseHandler handler, bool *coils_buffer, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_master_read_coils_response_handler = handler;

    rs485->modbus_master_read_coils_response_hlc.data = coils_buffer;
    rs485->modbus_master_read_coils_response_hlc.length = 0;
    rs485->modbus_master_read_coils_response_hlc.stream_in_progress = false;

    return tf_rs485_register_modbus_master_read_coils_response_low_level_callback(rs485, handler == NULL ? NULL : tf_rs485_modbus_master_read_coils_response_wrapper, user_data);
}


int tf_rs485_register_modbus_slave_read_holding_registers_request_callback(TF_RS485 *rs485, TF_RS485_ModbusSlaveReadHoldingRegistersRequestHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_slave_read_holding_registers_request_handler = handler;
    rs485->modbus_slave_read_holding_registers_request_user_data = user_data;

    return TF_E_OK;
}


int tf_rs485_register_modbus_master_read_holding_registers_response_low_level_callback(TF_RS485 *rs485, TF_RS485_ModbusMasterReadHoldingRegistersResponseLowLevelHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_master_read_holding_registers_response_low_level_handler = handler;
    rs485->modbus_master_read_holding_registers_response_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_rs485_modbus_master_read_holding_registers_response_wrapper(TF_RS485 *rs485, uint8_t request_id, int8_t exception_code, uint16_t holding_registers_length, uint16_t holding_registers_chunk_offset, uint16_t holding_registers_chunk_data[29], void *user_data) {
    uint32_t stream_length = (uint32_t) holding_registers_length;
    uint32_t chunk_offset = (uint32_t) holding_registers_chunk_offset;
    if (!tf_stream_out_callback(&rs485->modbus_master_read_holding_registers_response_hlc, stream_length, chunk_offset, holding_registers_chunk_data, 29, tf_copy_items_uint16_t)) {
        return;
    }

    // Stream is either complete or out of sync
    uint16_t *holding_registers = (uint16_t *) (rs485->modbus_master_read_holding_registers_response_hlc.length == 0 ? NULL : rs485->modbus_master_read_holding_registers_response_hlc.data);
    rs485->modbus_master_read_holding_registers_response_handler(rs485, request_id, exception_code, holding_registers, holding_registers_length, user_data);

    rs485->modbus_master_read_holding_registers_response_hlc.stream_in_progress = false;
    rs485->modbus_master_read_holding_registers_response_hlc.length = 0;
}

int tf_rs485_register_modbus_master_read_holding_registers_response_callback(TF_RS485 *rs485, TF_RS485_ModbusMasterReadHoldingRegistersResponseHandler handler, uint16_t *holding_registers_buffer, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_master_read_holding_registers_response_handler = handler;

    rs485->modbus_master_read_holding_registers_response_hlc.data = holding_registers_buffer;
    rs485->modbus_master_read_holding_registers_response_hlc.length = 0;
    rs485->modbus_master_read_holding_registers_response_hlc.stream_in_progress = false;

    return tf_rs485_register_modbus_master_read_holding_registers_response_low_level_callback(rs485, handler == NULL ? NULL : tf_rs485_modbus_master_read_holding_registers_response_wrapper, user_data);
}


int tf_rs485_register_modbus_slave_write_single_coil_request_callback(TF_RS485 *rs485, TF_RS485_ModbusSlaveWriteSingleCoilRequestHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_slave_write_single_coil_request_handler = handler;
    rs485->modbus_slave_write_single_coil_request_user_data = user_data;

    return TF_E_OK;
}


int tf_rs485_register_modbus_master_write_single_coil_response_callback(TF_RS485 *rs485, TF_RS485_ModbusMasterWriteSingleCoilResponseHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_master_write_single_coil_response_handler = handler;
    rs485->modbus_master_write_single_coil_response_user_data = user_data;

    return TF_E_OK;
}


int tf_rs485_register_modbus_slave_write_single_register_request_callback(TF_RS485 *rs485, TF_RS485_ModbusSlaveWriteSingleRegisterRequestHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_slave_write_single_register_request_handler = handler;
    rs485->modbus_slave_write_single_register_request_user_data = user_data;

    return TF_E_OK;
}


int tf_rs485_register_modbus_master_write_single_register_response_callback(TF_RS485 *rs485, TF_RS485_ModbusMasterWriteSingleRegisterResponseHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_master_write_single_register_response_handler = handler;
    rs485->modbus_master_write_single_register_response_user_data = user_data;

    return TF_E_OK;
}


int tf_rs485_register_modbus_slave_write_multiple_coils_request_low_level_callback(TF_RS485 *rs485, TF_RS485_ModbusSlaveWriteMultipleCoilsRequestLowLevelHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_slave_write_multiple_coils_request_low_level_handler = handler;
    rs485->modbus_slave_write_multiple_coils_request_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_rs485_modbus_slave_write_multiple_coils_request_wrapper(TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t coils_length, uint16_t coils_chunk_offset, bool coils_chunk_data[440], void *user_data) {
    uint32_t stream_length = (uint32_t) coils_length;
    uint32_t chunk_offset = (uint32_t) coils_chunk_offset;
    if (!tf_stream_out_callback(&rs485->modbus_slave_write_multiple_coils_request_hlc, stream_length, chunk_offset, coils_chunk_data, 440, tf_copy_items_bool)) {
        return;
    }

    // Stream is either complete or out of sync
    bool *coils = (bool *) (rs485->modbus_slave_write_multiple_coils_request_hlc.length == 0 ? NULL : rs485->modbus_slave_write_multiple_coils_request_hlc.data);
    rs485->modbus_slave_write_multiple_coils_request_handler(rs485, request_id, starting_address, coils, coils_length, user_data);

    rs485->modbus_slave_write_multiple_coils_request_hlc.stream_in_progress = false;
    rs485->modbus_slave_write_multiple_coils_request_hlc.length = 0;
}

int tf_rs485_register_modbus_slave_write_multiple_coils_request_callback(TF_RS485 *rs485, TF_RS485_ModbusSlaveWriteMultipleCoilsRequestHandler handler, bool *coils_buffer, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_slave_write_multiple_coils_request_handler = handler;

    rs485->modbus_slave_write_multiple_coils_request_hlc.data = coils_buffer;
    rs485->modbus_slave_write_multiple_coils_request_hlc.length = 0;
    rs485->modbus_slave_write_multiple_coils_request_hlc.stream_in_progress = false;

    return tf_rs485_register_modbus_slave_write_multiple_coils_request_low_level_callback(rs485, handler == NULL ? NULL : tf_rs485_modbus_slave_write_multiple_coils_request_wrapper, user_data);
}


int tf_rs485_register_modbus_master_write_multiple_coils_response_callback(TF_RS485 *rs485, TF_RS485_ModbusMasterWriteMultipleCoilsResponseHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_master_write_multiple_coils_response_handler = handler;
    rs485->modbus_master_write_multiple_coils_response_user_data = user_data;

    return TF_E_OK;
}


int tf_rs485_register_modbus_slave_write_multiple_registers_request_low_level_callback(TF_RS485 *rs485, TF_RS485_ModbusSlaveWriteMultipleRegistersRequestLowLevelHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_slave_write_multiple_registers_request_low_level_handler = handler;
    rs485->modbus_slave_write_multiple_registers_request_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_rs485_modbus_slave_write_multiple_registers_request_wrapper(TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t registers_length, uint16_t registers_chunk_offset, uint16_t registers_chunk_data[27], void *user_data) {
    uint32_t stream_length = (uint32_t) registers_length;
    uint32_t chunk_offset = (uint32_t) registers_chunk_offset;
    if (!tf_stream_out_callback(&rs485->modbus_slave_write_multiple_registers_request_hlc, stream_length, chunk_offset, registers_chunk_data, 27, tf_copy_items_uint16_t)) {
        return;
    }

    // Stream is either complete or out of sync
    uint16_t *registers = (uint16_t *) (rs485->modbus_slave_write_multiple_registers_request_hlc.length == 0 ? NULL : rs485->modbus_slave_write_multiple_registers_request_hlc.data);
    rs485->modbus_slave_write_multiple_registers_request_handler(rs485, request_id, starting_address, registers, registers_length, user_data);

    rs485->modbus_slave_write_multiple_registers_request_hlc.stream_in_progress = false;
    rs485->modbus_slave_write_multiple_registers_request_hlc.length = 0;
}

int tf_rs485_register_modbus_slave_write_multiple_registers_request_callback(TF_RS485 *rs485, TF_RS485_ModbusSlaveWriteMultipleRegistersRequestHandler handler, uint16_t *registers_buffer, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_slave_write_multiple_registers_request_handler = handler;

    rs485->modbus_slave_write_multiple_registers_request_hlc.data = registers_buffer;
    rs485->modbus_slave_write_multiple_registers_request_hlc.length = 0;
    rs485->modbus_slave_write_multiple_registers_request_hlc.stream_in_progress = false;

    return tf_rs485_register_modbus_slave_write_multiple_registers_request_low_level_callback(rs485, handler == NULL ? NULL : tf_rs485_modbus_slave_write_multiple_registers_request_wrapper, user_data);
}


int tf_rs485_register_modbus_master_write_multiple_registers_response_callback(TF_RS485 *rs485, TF_RS485_ModbusMasterWriteMultipleRegistersResponseHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_master_write_multiple_registers_response_handler = handler;
    rs485->modbus_master_write_multiple_registers_response_user_data = user_data;

    return TF_E_OK;
}


int tf_rs485_register_modbus_slave_read_discrete_inputs_request_callback(TF_RS485 *rs485, TF_RS485_ModbusSlaveReadDiscreteInputsRequestHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_slave_read_discrete_inputs_request_handler = handler;
    rs485->modbus_slave_read_discrete_inputs_request_user_data = user_data;

    return TF_E_OK;
}


int tf_rs485_register_modbus_master_read_discrete_inputs_response_low_level_callback(TF_RS485 *rs485, TF_RS485_ModbusMasterReadDiscreteInputsResponseLowLevelHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_master_read_discrete_inputs_response_low_level_handler = handler;
    rs485->modbus_master_read_discrete_inputs_response_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_rs485_modbus_master_read_discrete_inputs_response_wrapper(TF_RS485 *rs485, uint8_t request_id, int8_t exception_code, uint16_t discrete_inputs_length, uint16_t discrete_inputs_chunk_offset, bool discrete_inputs_chunk_data[464], void *user_data) {
    uint32_t stream_length = (uint32_t) discrete_inputs_length;
    uint32_t chunk_offset = (uint32_t) discrete_inputs_chunk_offset;
    if (!tf_stream_out_callback(&rs485->modbus_master_read_discrete_inputs_response_hlc, stream_length, chunk_offset, discrete_inputs_chunk_data, 464, tf_copy_items_bool)) {
        return;
    }

    // Stream is either complete or out of sync
    bool *discrete_inputs = (bool *) (rs485->modbus_master_read_discrete_inputs_response_hlc.length == 0 ? NULL : rs485->modbus_master_read_discrete_inputs_response_hlc.data);
    rs485->modbus_master_read_discrete_inputs_response_handler(rs485, request_id, exception_code, discrete_inputs, discrete_inputs_length, user_data);

    rs485->modbus_master_read_discrete_inputs_response_hlc.stream_in_progress = false;
    rs485->modbus_master_read_discrete_inputs_response_hlc.length = 0;
}

int tf_rs485_register_modbus_master_read_discrete_inputs_response_callback(TF_RS485 *rs485, TF_RS485_ModbusMasterReadDiscreteInputsResponseHandler handler, bool *discrete_inputs_buffer, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_master_read_discrete_inputs_response_handler = handler;

    rs485->modbus_master_read_discrete_inputs_response_hlc.data = discrete_inputs_buffer;
    rs485->modbus_master_read_discrete_inputs_response_hlc.length = 0;
    rs485->modbus_master_read_discrete_inputs_response_hlc.stream_in_progress = false;

    return tf_rs485_register_modbus_master_read_discrete_inputs_response_low_level_callback(rs485, handler == NULL ? NULL : tf_rs485_modbus_master_read_discrete_inputs_response_wrapper, user_data);
}


int tf_rs485_register_modbus_slave_read_input_registers_request_callback(TF_RS485 *rs485, TF_RS485_ModbusSlaveReadInputRegistersRequestHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_slave_read_input_registers_request_handler = handler;
    rs485->modbus_slave_read_input_registers_request_user_data = user_data;

    return TF_E_OK;
}


int tf_rs485_register_modbus_master_read_input_registers_response_low_level_callback(TF_RS485 *rs485, TF_RS485_ModbusMasterReadInputRegistersResponseLowLevelHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_master_read_input_registers_response_low_level_handler = handler;
    rs485->modbus_master_read_input_registers_response_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_rs485_modbus_master_read_input_registers_response_wrapper(TF_RS485 *rs485, uint8_t request_id, int8_t exception_code, uint16_t input_registers_length, uint16_t input_registers_chunk_offset, uint16_t input_registers_chunk_data[29], void *user_data) {
    uint32_t stream_length = (uint32_t) input_registers_length;
    uint32_t chunk_offset = (uint32_t) input_registers_chunk_offset;
    if (!tf_stream_out_callback(&rs485->modbus_master_read_input_registers_response_hlc, stream_length, chunk_offset, input_registers_chunk_data, 29, tf_copy_items_uint16_t)) {
        return;
    }

    // Stream is either complete or out of sync
    uint16_t *input_registers = (uint16_t *) (rs485->modbus_master_read_input_registers_response_hlc.length == 0 ? NULL : rs485->modbus_master_read_input_registers_response_hlc.data);
    rs485->modbus_master_read_input_registers_response_handler(rs485, request_id, exception_code, input_registers, input_registers_length, user_data);

    rs485->modbus_master_read_input_registers_response_hlc.stream_in_progress = false;
    rs485->modbus_master_read_input_registers_response_hlc.length = 0;
}

int tf_rs485_register_modbus_master_read_input_registers_response_callback(TF_RS485 *rs485, TF_RS485_ModbusMasterReadInputRegistersResponseHandler handler, uint16_t *input_registers_buffer, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->modbus_master_read_input_registers_response_handler = handler;

    rs485->modbus_master_read_input_registers_response_hlc.data = input_registers_buffer;
    rs485->modbus_master_read_input_registers_response_hlc.length = 0;
    rs485->modbus_master_read_input_registers_response_hlc.stream_in_progress = false;

    return tf_rs485_register_modbus_master_read_input_registers_response_low_level_callback(rs485, handler == NULL ? NULL : tf_rs485_modbus_master_read_input_registers_response_wrapper, user_data);
}


int tf_rs485_register_frame_readable_callback(TF_RS485 *rs485, TF_RS485_FrameReadableHandler handler, void *user_data) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs485->frame_readable_handler = handler;
    rs485->frame_readable_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_rs485_callback_tick(TF_RS485 *rs485, uint32_t timeout_us) {
    if (rs485 == NULL) {
        return TF_E_NULL;
    }

    if (rs485->magic != 0x5446 || rs485->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = rs485->tfp->spitfp->hal;

    return tf_tfp_callback_tick(rs485->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
