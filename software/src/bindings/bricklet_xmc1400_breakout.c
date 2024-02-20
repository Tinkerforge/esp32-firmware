/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_xmc1400_breakout.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_xmc1400_breakout_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_XMC1400Breakout *xmc1400_breakout = (TF_XMC1400Breakout *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(xmc1400_breakout->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_XMC1400_BREAKOUT_CALLBACK_ADC_VALUES: {
            TF_XMC1400Breakout_ADCValuesHandler fn = xmc1400_breakout->adc_values_handler;
            void *user_data = xmc1400_breakout->adc_values_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint16_t values[8]; for (_i = 0; _i < 8; ++_i) values[_i] = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(xmc1400_breakout, values, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_XMC1400_BREAKOUT_CALLBACK_COUNT: {
            TF_XMC1400Breakout_CountHandler fn = xmc1400_breakout->count_handler;
            void *user_data = xmc1400_breakout->count_user_data;
            if (fn == NULL) {
                return false;
            }

            uint32_t count = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(xmc1400_breakout, count, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_xmc1400_breakout_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_xmc1400_breakout_create(TF_XMC1400Breakout *xmc1400_breakout, const char *uid_or_port_name, TF_HAL *hal) {
    if (xmc1400_breakout == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(xmc1400_breakout, 0, sizeof(TF_XMC1400Breakout));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_XMC1400_BREAKOUT_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    xmc1400_breakout->tfp = tfp;
    xmc1400_breakout->tfp->device = xmc1400_breakout;
    xmc1400_breakout->tfp->cb_handler = tf_xmc1400_breakout_callback_handler;
    xmc1400_breakout->magic = 0x5446;
    xmc1400_breakout->response_expected[0] = 0x0C;
    return TF_E_OK;
}

int tf_xmc1400_breakout_destroy(TF_XMC1400Breakout *xmc1400_breakout) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }
    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    xmc1400_breakout->tfp->cb_handler = NULL;
    xmc1400_breakout->tfp->device = NULL;
    xmc1400_breakout->tfp = NULL;
    xmc1400_breakout->magic = 0;

    return TF_E_OK;
}

int tf_xmc1400_breakout_get_response_expected(TF_XMC1400Breakout *xmc1400_breakout, uint8_t function_id, bool *ret_response_expected) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_XMC1400_BREAKOUT_FUNCTION_SET_GPIO_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (xmc1400_breakout->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_SET_ADC_CHANNEL_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (xmc1400_breakout->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_SET_ADC_VALUES_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (xmc1400_breakout->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_SET_COUNT_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (xmc1400_breakout->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (xmc1400_breakout->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (xmc1400_breakout->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (xmc1400_breakout->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (xmc1400_breakout->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_xmc1400_breakout_set_response_expected(TF_XMC1400Breakout *xmc1400_breakout, uint8_t function_id, bool response_expected) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_XMC1400_BREAKOUT_FUNCTION_SET_GPIO_CONFIG:
            if (response_expected) {
                xmc1400_breakout->response_expected[0] |= (1 << 0);
            } else {
                xmc1400_breakout->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_SET_ADC_CHANNEL_CONFIG:
            if (response_expected) {
                xmc1400_breakout->response_expected[0] |= (1 << 1);
            } else {
                xmc1400_breakout->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_SET_ADC_VALUES_CALLBACK_CONFIGURATION:
            if (response_expected) {
                xmc1400_breakout->response_expected[0] |= (1 << 2);
            } else {
                xmc1400_breakout->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_SET_COUNT_CALLBACK_CONFIGURATION:
            if (response_expected) {
                xmc1400_breakout->response_expected[0] |= (1 << 3);
            } else {
                xmc1400_breakout->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                xmc1400_breakout->response_expected[0] |= (1 << 4);
            } else {
                xmc1400_breakout->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                xmc1400_breakout->response_expected[0] |= (1 << 5);
            } else {
                xmc1400_breakout->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_RESET:
            if (response_expected) {
                xmc1400_breakout->response_expected[0] |= (1 << 6);
            } else {
                xmc1400_breakout->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_XMC1400_BREAKOUT_FUNCTION_WRITE_UID:
            if (response_expected) {
                xmc1400_breakout->response_expected[0] |= (1 << 7);
            } else {
                xmc1400_breakout->response_expected[0] &= ~(1 << 7);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_xmc1400_breakout_set_response_expected_all(TF_XMC1400Breakout *xmc1400_breakout, bool response_expected) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(xmc1400_breakout->response_expected, response_expected ? 0xFF : 0, 1);

    return TF_E_OK;
}

int tf_xmc1400_breakout_set_gpio_config(TF_XMC1400Breakout *xmc1400_breakout, uint8_t port, uint8_t pin, uint8_t mode, uint8_t input_hysteresis, bool output_level) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_xmc1400_breakout_get_response_expected(xmc1400_breakout, TF_XMC1400_BREAKOUT_FUNCTION_SET_GPIO_CONFIG, &_response_expected);
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_SET_GPIO_CONFIG, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(xmc1400_breakout->tfp);

    _send_buf[0] = (uint8_t)port;
    _send_buf[1] = (uint8_t)pin;
    _send_buf[2] = (uint8_t)mode;
    _send_buf[3] = (uint8_t)input_hysteresis;
    _send_buf[4] = output_level ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_get_gpio_input(TF_XMC1400Breakout *xmc1400_breakout, uint8_t port, uint8_t pin, bool *ret_value) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_GET_GPIO_INPUT, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(xmc1400_breakout->tfp);

    _send_buf[0] = (uint8_t)port;
    _send_buf[1] = (uint8_t)pin;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_value != NULL) { *ret_value = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_set_adc_channel_config(TF_XMC1400Breakout *xmc1400_breakout, uint8_t channel, bool enable) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_xmc1400_breakout_get_response_expected(xmc1400_breakout, TF_XMC1400_BREAKOUT_FUNCTION_SET_ADC_CHANNEL_CONFIG, &_response_expected);
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_SET_ADC_CHANNEL_CONFIG, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(xmc1400_breakout->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = enable ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_get_adc_channel_config(TF_XMC1400Breakout *xmc1400_breakout, uint8_t channel, bool *ret_enable) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_GET_ADC_CHANNEL_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(xmc1400_breakout->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enable != NULL) { *ret_enable = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_get_adc_channel_value(TF_XMC1400Breakout *xmc1400_breakout, uint8_t channel, uint16_t *ret_value) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_GET_ADC_CHANNEL_VALUE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(xmc1400_breakout->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_value != NULL) { *ret_value = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_get_adc_values(TF_XMC1400Breakout *xmc1400_breakout, uint16_t ret_values[8]) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_GET_ADC_VALUES, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_values != NULL) { for (_i = 0; _i < 8; ++_i) ret_values[_i] = tf_packet_buffer_read_uint16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 16); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_set_adc_values_callback_configuration(TF_XMC1400Breakout *xmc1400_breakout, uint32_t period, bool value_has_to_change) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_xmc1400_breakout_get_response_expected(xmc1400_breakout, TF_XMC1400_BREAKOUT_FUNCTION_SET_ADC_VALUES_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_SET_ADC_VALUES_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(xmc1400_breakout->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_get_adc_values_callback_configuration(TF_XMC1400Breakout *xmc1400_breakout, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_GET_ADC_VALUES_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_get_count(TF_XMC1400Breakout *xmc1400_breakout, uint32_t *ret_count) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_GET_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_count != NULL) { *ret_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_set_count_callback_configuration(TF_XMC1400Breakout *xmc1400_breakout, uint32_t period, bool value_has_to_change, char option, uint32_t min, uint32_t max) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_xmc1400_breakout_get_response_expected(xmc1400_breakout, TF_XMC1400_BREAKOUT_FUNCTION_SET_COUNT_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_SET_COUNT_CALLBACK_CONFIGURATION, 14, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(xmc1400_breakout->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;
    _send_buf[5] = (uint8_t)option;
    min = tf_leconvert_uint32_to(min); memcpy(_send_buf + 6, &min, 4);
    max = tf_leconvert_uint32_to(max); memcpy(_send_buf + 10, &max, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_get_count_callback_configuration(TF_XMC1400Breakout *xmc1400_breakout, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, uint32_t *ret_min, uint32_t *ret_max) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_GET_COUNT_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 14) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_option != NULL) { *ret_option = tf_packet_buffer_read_char(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_min != NULL) { *ret_min = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_max != NULL) { *ret_max = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 14) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_get_spitfp_error_count(TF_XMC1400Breakout *xmc1400_breakout, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_set_bootloader_mode(TF_XMC1400Breakout *xmc1400_breakout, uint8_t mode, uint8_t *ret_status) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(xmc1400_breakout->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_get_bootloader_mode(TF_XMC1400Breakout *xmc1400_breakout, uint8_t *ret_mode) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_set_write_firmware_pointer(TF_XMC1400Breakout *xmc1400_breakout, uint32_t pointer) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_xmc1400_breakout_get_response_expected(xmc1400_breakout, TF_XMC1400_BREAKOUT_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(xmc1400_breakout->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_write_firmware(TF_XMC1400Breakout *xmc1400_breakout, const uint8_t data[64], uint8_t *ret_status) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(xmc1400_breakout->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_set_status_led_config(TF_XMC1400Breakout *xmc1400_breakout, uint8_t config) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_xmc1400_breakout_get_response_expected(xmc1400_breakout, TF_XMC1400_BREAKOUT_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(xmc1400_breakout->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_get_status_led_config(TF_XMC1400Breakout *xmc1400_breakout, uint8_t *ret_config) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_get_chip_temperature(TF_XMC1400Breakout *xmc1400_breakout, int16_t *ret_temperature) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_reset(TF_XMC1400Breakout *xmc1400_breakout) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_xmc1400_breakout_get_response_expected(xmc1400_breakout, TF_XMC1400_BREAKOUT_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_write_uid(TF_XMC1400Breakout *xmc1400_breakout, uint32_t uid) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_xmc1400_breakout_get_response_expected(xmc1400_breakout, TF_XMC1400_BREAKOUT_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(xmc1400_breakout->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_read_uid(TF_XMC1400Breakout *xmc1400_breakout, uint32_t *ret_uid) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_xmc1400_breakout_get_identity(TF_XMC1400Breakout *xmc1400_breakout, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = xmc1400_breakout->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(xmc1400_breakout->tfp, TF_XMC1400_BREAKOUT_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(xmc1400_breakout->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(xmc1400_breakout->tfp);
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
        tf_tfp_packet_processed(xmc1400_breakout->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(xmc1400_breakout->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_xmc1400_breakout_register_adc_values_callback(TF_XMC1400Breakout *xmc1400_breakout, TF_XMC1400Breakout_ADCValuesHandler handler, void *user_data) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    xmc1400_breakout->adc_values_handler = handler;
    xmc1400_breakout->adc_values_user_data = user_data;

    return TF_E_OK;
}


int tf_xmc1400_breakout_register_count_callback(TF_XMC1400Breakout *xmc1400_breakout, TF_XMC1400Breakout_CountHandler handler, void *user_data) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    xmc1400_breakout->count_handler = handler;
    xmc1400_breakout->count_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_xmc1400_breakout_callback_tick(TF_XMC1400Breakout *xmc1400_breakout, uint32_t timeout_us) {
    if (xmc1400_breakout == NULL) {
        return TF_E_NULL;
    }

    if (xmc1400_breakout->magic != 0x5446 || xmc1400_breakout->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = xmc1400_breakout->tfp->spitfp->hal;

    return tf_tfp_callback_tick(xmc1400_breakout->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
