/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_industrial_analog_out_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


static bool tf_industrial_analog_out_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    (void)device;
    (void)fid;
    (void)payload;

    return false;
}
int tf_industrial_analog_out_v2_create(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (industrial_analog_out_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(industrial_analog_out_v2, 0, sizeof(TF_IndustrialAnalogOutV2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_INDUSTRIAL_ANALOG_OUT_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    industrial_analog_out_v2->tfp = tfp;
    industrial_analog_out_v2->tfp->device = industrial_analog_out_v2;
    industrial_analog_out_v2->tfp->cb_handler = tf_industrial_analog_out_v2_callback_handler;
    industrial_analog_out_v2->magic = 0x5446;
    industrial_analog_out_v2->response_expected[0] = 0x00;
    industrial_analog_out_v2->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_industrial_analog_out_v2_destroy(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }
    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    industrial_analog_out_v2->tfp->cb_handler = NULL;
    industrial_analog_out_v2->tfp->device = NULL;
    industrial_analog_out_v2->tfp = NULL;
    industrial_analog_out_v2->magic = 0;

    return TF_E_OK;
}

int tf_industrial_analog_out_v2_get_response_expected(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint8_t function_id, bool *ret_response_expected) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_ENABLED:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_analog_out_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_VOLTAGE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_analog_out_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_CURRENT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_analog_out_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_analog_out_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_OUT_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_analog_out_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_OUT_LED_STATUS_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_analog_out_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_analog_out_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_analog_out_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_analog_out_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (industrial_analog_out_v2->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_industrial_analog_out_v2_set_response_expected(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint8_t function_id, bool response_expected) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_ENABLED:
            if (response_expected) {
                industrial_analog_out_v2->response_expected[0] |= (1 << 0);
            } else {
                industrial_analog_out_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_VOLTAGE:
            if (response_expected) {
                industrial_analog_out_v2->response_expected[0] |= (1 << 1);
            } else {
                industrial_analog_out_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_CURRENT:
            if (response_expected) {
                industrial_analog_out_v2->response_expected[0] |= (1 << 2);
            } else {
                industrial_analog_out_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_CONFIGURATION:
            if (response_expected) {
                industrial_analog_out_v2->response_expected[0] |= (1 << 3);
            } else {
                industrial_analog_out_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_OUT_LED_CONFIG:
            if (response_expected) {
                industrial_analog_out_v2->response_expected[0] |= (1 << 4);
            } else {
                industrial_analog_out_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_OUT_LED_STATUS_CONFIG:
            if (response_expected) {
                industrial_analog_out_v2->response_expected[0] |= (1 << 5);
            } else {
                industrial_analog_out_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                industrial_analog_out_v2->response_expected[0] |= (1 << 6);
            } else {
                industrial_analog_out_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                industrial_analog_out_v2->response_expected[0] |= (1 << 7);
            } else {
                industrial_analog_out_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_RESET:
            if (response_expected) {
                industrial_analog_out_v2->response_expected[1] |= (1 << 0);
            } else {
                industrial_analog_out_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                industrial_analog_out_v2->response_expected[1] |= (1 << 1);
            } else {
                industrial_analog_out_v2->response_expected[1] &= ~(1 << 1);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_industrial_analog_out_v2_set_response_expected_all(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, bool response_expected) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(industrial_analog_out_v2->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_industrial_analog_out_v2_set_enabled(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, bool enabled) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_analog_out_v2_get_response_expected(industrial_analog_out_v2, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_ENABLED, &_response_expected);
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_ENABLED, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_analog_out_v2->tfp);

    _send_buf[0] = enabled ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_get_enabled(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, bool *ret_enabled) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_GET_ENABLED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_set_voltage(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint16_t voltage) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_analog_out_v2_get_response_expected(industrial_analog_out_v2, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_VOLTAGE, &_response_expected);
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_VOLTAGE, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_analog_out_v2->tfp);

    voltage = tf_leconvert_uint16_to(voltage); memcpy(_send_buf + 0, &voltage, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_get_voltage(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint16_t *ret_voltage) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_GET_VOLTAGE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_voltage != NULL) { *ret_voltage = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_set_current(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint16_t current) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_analog_out_v2_get_response_expected(industrial_analog_out_v2, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_CURRENT, &_response_expected);
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_CURRENT, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_analog_out_v2->tfp);

    current = tf_leconvert_uint16_to(current); memcpy(_send_buf + 0, &current, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_get_current(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint16_t *ret_current) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_GET_CURRENT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_current != NULL) { *ret_current = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_set_configuration(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint8_t voltage_range, uint8_t current_range) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_analog_out_v2_get_response_expected(industrial_analog_out_v2, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_CONFIGURATION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_analog_out_v2->tfp);

    _send_buf[0] = (uint8_t)voltage_range;
    _send_buf[1] = (uint8_t)current_range;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_get_configuration(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint8_t *ret_voltage_range, uint8_t *ret_current_range) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_GET_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_voltage_range != NULL) { *ret_voltage_range = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_current_range != NULL) { *ret_current_range = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_set_out_led_config(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint8_t config) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_analog_out_v2_get_response_expected(industrial_analog_out_v2, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_OUT_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_OUT_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_analog_out_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_get_out_led_config(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint8_t *ret_config) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_GET_OUT_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_set_out_led_status_config(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint16_t min, uint16_t max, uint8_t config) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_analog_out_v2_get_response_expected(industrial_analog_out_v2, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_OUT_LED_STATUS_CONFIG, &_response_expected);
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_OUT_LED_STATUS_CONFIG, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_analog_out_v2->tfp);

    min = tf_leconvert_uint16_to(min); memcpy(_send_buf + 0, &min, 2);
    max = tf_leconvert_uint16_to(max); memcpy(_send_buf + 2, &max, 2);
    _send_buf[4] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_get_out_led_status_config(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint16_t *ret_min, uint16_t *ret_max, uint8_t *ret_config) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_GET_OUT_LED_STATUS_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_min != NULL) { *ret_min = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_max != NULL) { *ret_max = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_get_spitfp_error_count(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_set_bootloader_mode(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint8_t mode, uint8_t *ret_status) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_analog_out_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_get_bootloader_mode(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint8_t *ret_mode) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_set_write_firmware_pointer(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint32_t pointer) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_analog_out_v2_get_response_expected(industrial_analog_out_v2, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_analog_out_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_write_firmware(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_analog_out_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_set_status_led_config(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint8_t config) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_analog_out_v2_get_response_expected(industrial_analog_out_v2, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_analog_out_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_get_status_led_config(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint8_t *ret_config) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_get_chip_temperature(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, int16_t *ret_temperature) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_reset(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_analog_out_v2_get_response_expected(industrial_analog_out_v2, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_write_uid(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint32_t uid) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_industrial_analog_out_v2_get_response_expected(industrial_analog_out_v2, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_analog_out_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_read_uid(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint32_t *ret_uid) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_industrial_analog_out_v2_get_identity(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_analog_out_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(industrial_analog_out_v2->tfp, TF_INDUSTRIAL_ANALOG_OUT_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_analog_out_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(industrial_analog_out_v2->tfp);
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
        tf_tfp_packet_processed(industrial_analog_out_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_analog_out_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}


int tf_industrial_analog_out_v2_callback_tick(TF_IndustrialAnalogOutV2 *industrial_analog_out_v2, uint32_t timeout_us) {
    if (industrial_analog_out_v2 == NULL) {
        return TF_E_NULL;
    }

    if (industrial_analog_out_v2->magic != 0x5446 || industrial_analog_out_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = industrial_analog_out_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(industrial_analog_out_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
