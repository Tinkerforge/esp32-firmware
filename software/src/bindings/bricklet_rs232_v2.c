/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_rs232_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_rs232_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_RS232V2 *rs232_v2 = (TF_RS232V2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(rs232_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_RS232_V2_CALLBACK_READ_LOW_LEVEL: {
            TF_RS232V2_ReadLowLevelHandler fn = rs232_v2->read_low_level_handler;
            void *user_data = rs232_v2->read_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint16_t message_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t message_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            char message_chunk_data[60]; for (_i = 0; _i < 60; ++_i) message_chunk_data[_i] = tf_packet_buffer_read_char(payload);
            hal_common->locked = true;
            fn(rs232_v2, message_length, message_chunk_offset, message_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS232_V2_CALLBACK_ERROR_COUNT: {
            TF_RS232V2_ErrorCountHandler fn = rs232_v2->error_count_handler;
            void *user_data = rs232_v2->error_count_user_data;
            if (fn == NULL) {
                return false;
            }

            uint32_t error_count_overrun = tf_packet_buffer_read_uint32_t(payload);
            uint32_t error_count_parity = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(rs232_v2, error_count_overrun, error_count_parity, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_RS232_V2_CALLBACK_FRAME_READABLE: {
            TF_RS232V2_FrameReadableHandler fn = rs232_v2->frame_readable_handler;
            void *user_data = rs232_v2->frame_readable_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t frame_count = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(rs232_v2, frame_count, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_rs232_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_rs232_v2_create(TF_RS232V2 *rs232_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (rs232_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(rs232_v2, 0, sizeof(TF_RS232V2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_RS232_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    rs232_v2->tfp = tfp;
    rs232_v2->tfp->device = rs232_v2;
    rs232_v2->tfp->cb_handler = tf_rs232_v2_callback_handler;
    rs232_v2->magic = 0x5446;
    rs232_v2->response_expected[0] = 0x13;
    rs232_v2->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_rs232_v2_destroy(TF_RS232V2 *rs232_v2) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }
    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs232_v2->tfp->cb_handler = NULL;
    rs232_v2->tfp->device = NULL;
    rs232_v2->tfp = NULL;
    rs232_v2->magic = 0;

    return TF_E_OK;
}

int tf_rs232_v2_get_response_expected(TF_RS232V2 *rs232_v2, uint8_t function_id, bool *ret_response_expected) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_RS232_V2_FUNCTION_ENABLE_READ_CALLBACK:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs232_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_RS232_V2_FUNCTION_DISABLE_READ_CALLBACK:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs232_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_RS232_V2_FUNCTION_SET_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs232_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_RS232_V2_FUNCTION_SET_BUFFER_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs232_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_RS232_V2_FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs232_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_RS232_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs232_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_RS232_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs232_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_RS232_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs232_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_RS232_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rs232_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_rs232_v2_set_response_expected(TF_RS232V2 *rs232_v2, uint8_t function_id, bool response_expected) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_RS232_V2_FUNCTION_ENABLE_READ_CALLBACK:
            if (response_expected) {
                rs232_v2->response_expected[0] |= (1 << 0);
            } else {
                rs232_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_RS232_V2_FUNCTION_DISABLE_READ_CALLBACK:
            if (response_expected) {
                rs232_v2->response_expected[0] |= (1 << 1);
            } else {
                rs232_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_RS232_V2_FUNCTION_SET_CONFIGURATION:
            if (response_expected) {
                rs232_v2->response_expected[0] |= (1 << 2);
            } else {
                rs232_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_RS232_V2_FUNCTION_SET_BUFFER_CONFIG:
            if (response_expected) {
                rs232_v2->response_expected[0] |= (1 << 3);
            } else {
                rs232_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_RS232_V2_FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                rs232_v2->response_expected[0] |= (1 << 4);
            } else {
                rs232_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_RS232_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                rs232_v2->response_expected[0] |= (1 << 5);
            } else {
                rs232_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_RS232_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                rs232_v2->response_expected[0] |= (1 << 6);
            } else {
                rs232_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_RS232_V2_FUNCTION_RESET:
            if (response_expected) {
                rs232_v2->response_expected[0] |= (1 << 7);
            } else {
                rs232_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_RS232_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                rs232_v2->response_expected[1] |= (1 << 0);
            } else {
                rs232_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_rs232_v2_set_response_expected_all(TF_RS232V2 *rs232_v2, bool response_expected) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(rs232_v2->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_rs232_v2_write_low_level(TF_RS232V2 *rs232_v2, uint16_t message_length, uint16_t message_chunk_offset, const char message_chunk_data[60], uint8_t *ret_message_chunk_written) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_WRITE_LOW_LEVEL, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs232_v2->tfp);

    message_length = tf_leconvert_uint16_to(message_length); memcpy(_send_buf + 0, &message_length, 2);
    message_chunk_offset = tf_leconvert_uint16_to(message_chunk_offset); memcpy(_send_buf + 2, &message_chunk_offset, 2);
    memcpy(_send_buf + 4, message_chunk_data, 60);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_message_chunk_written != NULL) { *ret_message_chunk_written = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_read_low_level(TF_RS232V2 *rs232_v2, uint16_t length, uint16_t *ret_message_length, uint16_t *ret_message_chunk_offset, char ret_message_chunk_data[60]) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_READ_LOW_LEVEL, 2, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs232_v2->tfp);

    length = tf_leconvert_uint16_to(length); memcpy(_send_buf + 0, &length, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_message_length != NULL) { *ret_message_length = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_message_chunk_offset != NULL) { *ret_message_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_message_chunk_data != NULL) { for (_i = 0; _i < 60; ++_i) ret_message_chunk_data[_i] = tf_packet_buffer_read_char(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_enable_read_callback(TF_RS232V2 *rs232_v2) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs232_v2_get_response_expected(rs232_v2, TF_RS232_V2_FUNCTION_ENABLE_READ_CALLBACK, &_response_expected);
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_ENABLE_READ_CALLBACK, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_disable_read_callback(TF_RS232V2 *rs232_v2) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs232_v2_get_response_expected(rs232_v2, TF_RS232_V2_FUNCTION_DISABLE_READ_CALLBACK, &_response_expected);
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_DISABLE_READ_CALLBACK, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_is_read_callback_enabled(TF_RS232V2 *rs232_v2, bool *ret_enabled) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_IS_READ_CALLBACK_ENABLED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_set_configuration(TF_RS232V2 *rs232_v2, uint32_t baudrate, uint8_t parity, uint8_t stopbits, uint8_t wordlength, uint8_t flowcontrol) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs232_v2_get_response_expected(rs232_v2, TF_RS232_V2_FUNCTION_SET_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_SET_CONFIGURATION, 8, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs232_v2->tfp);

    baudrate = tf_leconvert_uint32_to(baudrate); memcpy(_send_buf + 0, &baudrate, 4);
    _send_buf[4] = (uint8_t)parity;
    _send_buf[5] = (uint8_t)stopbits;
    _send_buf[6] = (uint8_t)wordlength;
    _send_buf[7] = (uint8_t)flowcontrol;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_get_configuration(TF_RS232V2 *rs232_v2, uint32_t *ret_baudrate, uint8_t *ret_parity, uint8_t *ret_stopbits, uint8_t *ret_wordlength, uint8_t *ret_flowcontrol) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_GET_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_baudrate != NULL) { *ret_baudrate = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_parity != NULL) { *ret_parity = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_stopbits != NULL) { *ret_stopbits = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_wordlength != NULL) { *ret_wordlength = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_flowcontrol != NULL) { *ret_flowcontrol = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_set_buffer_config(TF_RS232V2 *rs232_v2, uint16_t send_buffer_size, uint16_t receive_buffer_size) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs232_v2_get_response_expected(rs232_v2, TF_RS232_V2_FUNCTION_SET_BUFFER_CONFIG, &_response_expected);
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_SET_BUFFER_CONFIG, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs232_v2->tfp);

    send_buffer_size = tf_leconvert_uint16_to(send_buffer_size); memcpy(_send_buf + 0, &send_buffer_size, 2);
    receive_buffer_size = tf_leconvert_uint16_to(receive_buffer_size); memcpy(_send_buf + 2, &receive_buffer_size, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_get_buffer_config(TF_RS232V2 *rs232_v2, uint16_t *ret_send_buffer_size, uint16_t *ret_receive_buffer_size) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_GET_BUFFER_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_send_buffer_size != NULL) { *ret_send_buffer_size = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_receive_buffer_size != NULL) { *ret_receive_buffer_size = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_get_buffer_status(TF_RS232V2 *rs232_v2, uint16_t *ret_send_buffer_used, uint16_t *ret_receive_buffer_used) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_GET_BUFFER_STATUS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_send_buffer_used != NULL) { *ret_send_buffer_used = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_receive_buffer_used != NULL) { *ret_receive_buffer_used = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_get_error_count(TF_RS232V2 *rs232_v2, uint32_t *ret_error_count_overrun, uint32_t *ret_error_count_parity) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_GET_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_overrun != NULL) { *ret_error_count_overrun = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_parity != NULL) { *ret_error_count_parity = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_set_frame_readable_callback_configuration(TF_RS232V2 *rs232_v2, uint16_t frame_size) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs232_v2_get_response_expected(rs232_v2, TF_RS232_V2_FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs232_v2->tfp);

    frame_size = tf_leconvert_uint16_to(frame_size); memcpy(_send_buf + 0, &frame_size, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_get_frame_readable_callback_configuration(TF_RS232V2 *rs232_v2, uint16_t *ret_frame_size) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_GET_FRAME_READABLE_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_frame_size != NULL) { *ret_frame_size = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_get_spitfp_error_count(TF_RS232V2 *rs232_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_set_bootloader_mode(TF_RS232V2 *rs232_v2, uint8_t mode, uint8_t *ret_status) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs232_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_get_bootloader_mode(TF_RS232V2 *rs232_v2, uint8_t *ret_mode) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_set_write_firmware_pointer(TF_RS232V2 *rs232_v2, uint32_t pointer) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs232_v2_get_response_expected(rs232_v2, TF_RS232_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs232_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_write_firmware(TF_RS232V2 *rs232_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs232_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_set_status_led_config(TF_RS232V2 *rs232_v2, uint8_t config) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs232_v2_get_response_expected(rs232_v2, TF_RS232_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs232_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_get_status_led_config(TF_RS232V2 *rs232_v2, uint8_t *ret_config) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_get_chip_temperature(TF_RS232V2 *rs232_v2, int16_t *ret_temperature) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_reset(TF_RS232V2 *rs232_v2) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs232_v2_get_response_expected(rs232_v2, TF_RS232_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_write_uid(TF_RS232V2 *rs232_v2, uint32_t uid) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rs232_v2_get_response_expected(rs232_v2, TF_RS232_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rs232_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_read_uid(TF_RS232V2 *rs232_v2, uint32_t *ret_uid) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rs232_v2_get_identity(TF_RS232V2 *rs232_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rs232_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rs232_v2->tfp, TF_RS232_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rs232_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rs232_v2->tfp);
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
        tf_tfp_packet_processed(rs232_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rs232_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

static int tf_rs232_v2_write_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    (void)wrapper_data;
    uint16_t message_chunk_offset = (uint16_t)chunk_offset;
    uint16_t message_length = (uint16_t)stream_length;
    uint8_t message_chunk_written = 60;

    char *message_chunk_data = (char *) chunk_data;
    int ret = tf_rs232_v2_write_low_level((TF_RS232V2 *)device, message_length, message_chunk_offset, message_chunk_data, &message_chunk_written);

    *ret_chunk_written = (uint32_t) message_chunk_written;
    return ret;
}

int tf_rs232_v2_write(TF_RS232V2 *rs232_v2, const char *message, uint16_t message_length, uint16_t *ret_message_written) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }


    uint32_t _stream_length = message_length;
    uint32_t _message_written = 0;
    char _chunk_data[60];

    int ret = tf_stream_in(rs232_v2, tf_rs232_v2_write_ll_wrapper, NULL, message, _stream_length, _chunk_data, &_message_written, 60, tf_copy_items_char);

    if (ret_message_written != NULL) {
        *ret_message_written = (uint16_t) _message_written;
    }

    return ret;
}


typedef struct TF_RS232V2_ReadLLWrapperData {
    uint16_t length;
} TF_RS232V2_ReadLLWrapperData;


static int tf_rs232_v2_read_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    TF_RS232V2_ReadLLWrapperData *data = (TF_RS232V2_ReadLLWrapperData *) wrapper_data;
    uint16_t message_length = 0;
    uint16_t message_chunk_offset = 0;
    char *message_chunk_data = (char *) chunk_data;
    int ret = tf_rs232_v2_read_low_level((TF_RS232V2 *)device, data->length, &message_length, &message_chunk_offset, message_chunk_data);

    *ret_stream_length = (uint32_t)message_length;
    *ret_chunk_offset = (uint32_t)message_chunk_offset;
    return ret;
}

int tf_rs232_v2_read(TF_RS232V2 *rs232_v2, uint16_t length, char *ret_message, uint16_t *ret_message_length) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_RS232V2_ReadLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.length = length;
    uint32_t _message_length = 0;
    char _message_chunk_data[60];

    int ret = tf_stream_out(rs232_v2, tf_rs232_v2_read_ll_wrapper, &_wrapper_data, ret_message, &_message_length, _message_chunk_data, 60, tf_copy_items_char);

    if (ret_message_length != NULL) {
        *ret_message_length = (uint16_t)_message_length;
    }
    return ret;
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_rs232_v2_register_read_low_level_callback(TF_RS232V2 *rs232_v2, TF_RS232V2_ReadLowLevelHandler handler, void *user_data) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs232_v2->read_low_level_handler = handler;
    rs232_v2->read_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_rs232_v2_read_wrapper(TF_RS232V2 *rs232_v2, uint16_t message_length, uint16_t message_chunk_offset, char message_chunk_data[60], void *user_data) {
    uint32_t stream_length = (uint32_t) message_length;
    uint32_t chunk_offset = (uint32_t) message_chunk_offset;
    if (!tf_stream_out_callback(&rs232_v2->read_hlc, stream_length, chunk_offset, message_chunk_data, 60, tf_copy_items_char)) {
        return;
    }

    // Stream is either complete or out of sync
    char *message = (char *) (rs232_v2->read_hlc.length == 0 ? NULL : rs232_v2->read_hlc.data);
    rs232_v2->read_handler(rs232_v2, message, message_length, user_data);

    rs232_v2->read_hlc.stream_in_progress = false;
    rs232_v2->read_hlc.length = 0;
}

int tf_rs232_v2_register_read_callback(TF_RS232V2 *rs232_v2, TF_RS232V2_ReadHandler handler, char *message_buffer, void *user_data) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs232_v2->read_handler = handler;

    rs232_v2->read_hlc.data = message_buffer;
    rs232_v2->read_hlc.length = 0;
    rs232_v2->read_hlc.stream_in_progress = false;

    return tf_rs232_v2_register_read_low_level_callback(rs232_v2, handler == NULL ? NULL : tf_rs232_v2_read_wrapper, user_data);
}


int tf_rs232_v2_register_error_count_callback(TF_RS232V2 *rs232_v2, TF_RS232V2_ErrorCountHandler handler, void *user_data) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs232_v2->error_count_handler = handler;
    rs232_v2->error_count_user_data = user_data;

    return TF_E_OK;
}


int tf_rs232_v2_register_frame_readable_callback(TF_RS232V2 *rs232_v2, TF_RS232V2_FrameReadableHandler handler, void *user_data) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rs232_v2->frame_readable_handler = handler;
    rs232_v2->frame_readable_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_rs232_v2_callback_tick(TF_RS232V2 *rs232_v2, uint32_t timeout_us) {
    if (rs232_v2 == NULL) {
        return TF_E_NULL;
    }

    if (rs232_v2->magic != 0x5446 || rs232_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = rs232_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(rs232_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
