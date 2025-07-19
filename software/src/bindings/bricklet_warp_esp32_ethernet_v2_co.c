/* ***********************************************************
 * This file was automatically generated on 2025-07-10.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_warp_esp32_ethernet_v2_co.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_warp_esp32_ethernet_v2_co_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co = (TF_WARPESP32EthernetV2Co *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(warp_esp32_ethernet_v2_co->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_WARP_ESP32_ETHERNET_V2_CO_CALLBACK_RMMI_INTERRUPT: {
            TF_WARPESP32EthernetV2Co_RMMIInterruptHandler fn = warp_esp32_ethernet_v2_co->rmmi_interrupt_handler;
            void *user_data = warp_esp32_ethernet_v2_co->rmmi_interrupt_user_data;
            if (fn == NULL) {
                return false;
            }


            hal_common->locked = true;
            fn(warp_esp32_ethernet_v2_co, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_warp_esp32_ethernet_v2_co_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_warp_esp32_ethernet_v2_co_create(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, const char *uid_or_port_name, TF_HAL *hal) {
    if (warp_esp32_ethernet_v2_co == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(warp_esp32_ethernet_v2_co, 0, sizeof(TF_WARPESP32EthernetV2Co));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_WARP_ESP32_ETHERNET_V2_CO_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    warp_esp32_ethernet_v2_co->tfp = tfp;
    warp_esp32_ethernet_v2_co->tfp->device = warp_esp32_ethernet_v2_co;
    warp_esp32_ethernet_v2_co->tfp->cb_handler = tf_warp_esp32_ethernet_v2_co_callback_handler;
    warp_esp32_ethernet_v2_co->magic = 0x5446;
    warp_esp32_ethernet_v2_co->response_expected[0] = 0x00;
    return TF_E_OK;
}

int tf_warp_esp32_ethernet_v2_co_destroy(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }
    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_esp32_ethernet_v2_co->tfp->cb_handler = NULL;
    warp_esp32_ethernet_v2_co->tfp->device = NULL;
    warp_esp32_ethernet_v2_co->tfp = NULL;
    warp_esp32_ethernet_v2_co->magic = 0;

    return TF_E_OK;
}

int tf_warp_esp32_ethernet_v2_co_get_response_expected(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t function_id, bool *ret_response_expected) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_LED:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_esp32_ethernet_v2_co->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_DATE_TIME:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_esp32_ethernet_v2_co->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_esp32_ethernet_v2_co->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_esp32_ethernet_v2_co->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_esp32_ethernet_v2_co->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_esp32_ethernet_v2_co->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_warp_esp32_ethernet_v2_co_set_response_expected(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t function_id, bool response_expected) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_LED:
            if (response_expected) {
                warp_esp32_ethernet_v2_co->response_expected[0] |= (1 << 0);
            } else {
                warp_esp32_ethernet_v2_co->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_DATE_TIME:
            if (response_expected) {
                warp_esp32_ethernet_v2_co->response_expected[0] |= (1 << 1);
            } else {
                warp_esp32_ethernet_v2_co->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                warp_esp32_ethernet_v2_co->response_expected[0] |= (1 << 2);
            } else {
                warp_esp32_ethernet_v2_co->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                warp_esp32_ethernet_v2_co->response_expected[0] |= (1 << 3);
            } else {
                warp_esp32_ethernet_v2_co->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_RESET:
            if (response_expected) {
                warp_esp32_ethernet_v2_co->response_expected[0] |= (1 << 4);
            } else {
                warp_esp32_ethernet_v2_co->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_WRITE_UID:
            if (response_expected) {
                warp_esp32_ethernet_v2_co->response_expected[0] |= (1 << 5);
            } else {
                warp_esp32_ethernet_v2_co->response_expected[0] &= ~(1 << 5);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_warp_esp32_ethernet_v2_co_set_response_expected_all(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, bool response_expected) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(warp_esp32_ethernet_v2_co->response_expected, response_expected ? 0xFF : 0, 1);

    return TF_E_OK;
}

int tf_warp_esp32_ethernet_v2_co_set_led(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t state) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_esp32_ethernet_v2_co_get_response_expected(warp_esp32_ethernet_v2_co, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_LED, &_response_expected);
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_LED, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_esp32_ethernet_v2_co->tfp);

    _send_buf[0] = (uint8_t)state;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_get_led(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t *ret_state) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_LED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_state != NULL) { *ret_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_get_temperature(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, int16_t *ret_temperature) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_set_date_time(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t days, uint8_t days_of_week, uint8_t month, uint16_t year) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_esp32_ethernet_v2_co_get_response_expected(warp_esp32_ethernet_v2_co, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_DATE_TIME, &_response_expected);
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_DATE_TIME, 8, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_esp32_ethernet_v2_co->tfp);

    _send_buf[0] = (uint8_t)seconds;
    _send_buf[1] = (uint8_t)minutes;
    _send_buf[2] = (uint8_t)hours;
    _send_buf[3] = (uint8_t)days;
    _send_buf[4] = (uint8_t)days_of_week;
    _send_buf[5] = (uint8_t)month;
    year = tf_leconvert_uint16_to(year); memcpy(_send_buf + 6, &year, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_get_date_time(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t *ret_seconds, uint8_t *ret_minutes, uint8_t *ret_hours, uint8_t *ret_days, uint8_t *ret_days_of_week, uint8_t *ret_month, uint16_t *ret_year) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_DATE_TIME, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_seconds != NULL) { *ret_seconds = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_minutes != NULL) { *ret_minutes = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_hours != NULL) { *ret_hours = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_days != NULL) { *ret_days = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_days_of_week != NULL) { *ret_days_of_week = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_month != NULL) { *ret_month = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_year != NULL) { *ret_year = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_get_uptime(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t *ret_uptime) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_UPTIME, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uptime != NULL) { *ret_uptime = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_format_sd(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t password, uint8_t *ret_format_status) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_FORMAT_SD, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_esp32_ethernet_v2_co->tfp);

    password = tf_leconvert_uint32_to(password); memcpy(_send_buf + 0, &password, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_format_status != NULL) { *ret_format_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_get_sd_information(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t *ret_sd_status, uint32_t *ret_lfs_status, uint16_t *ret_sector_size, uint32_t *ret_sector_count, uint32_t *ret_card_type, uint8_t *ret_product_rev, char ret_product_name[5], uint8_t *ret_manufacturer_id) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_SD_INFORMATION, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 25) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_sd_status != NULL) { *ret_sd_status = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_lfs_status != NULL) { *ret_lfs_status = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_sector_size != NULL) { *ret_sector_size = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_sector_count != NULL) { *ret_sector_count = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_card_type != NULL) { *ret_card_type = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_product_rev != NULL) { *ret_product_rev = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_product_name != NULL) { for (_i = 0; _i < 5; ++_i) ret_product_name[_i] = tf_packet_buffer_read_char(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 5); }
            if (ret_manufacturer_id != NULL) { *ret_manufacturer_id = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_get_spitfp_error_count(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_set_bootloader_mode(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t mode, uint8_t *ret_status) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_esp32_ethernet_v2_co->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_get_bootloader_mode(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t *ret_mode) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_set_write_firmware_pointer(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t pointer) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_esp32_ethernet_v2_co_get_response_expected(warp_esp32_ethernet_v2_co, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_esp32_ethernet_v2_co->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_write_firmware(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, const uint8_t data[64], uint8_t *ret_status) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_esp32_ethernet_v2_co->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_set_status_led_config(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t config) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_esp32_ethernet_v2_co_get_response_expected(warp_esp32_ethernet_v2_co, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_esp32_ethernet_v2_co->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_get_status_led_config(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint8_t *ret_config) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_get_chip_temperature(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, int16_t *ret_temperature) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_reset(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_esp32_ethernet_v2_co_get_response_expected(warp_esp32_ethernet_v2_co, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_write_uid(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t uid) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_esp32_ethernet_v2_co_get_response_expected(warp_esp32_ethernet_v2_co, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_esp32_ethernet_v2_co->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_read_uid(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t *ret_uid) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_esp32_ethernet_v2_co_get_identity(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_esp32_ethernet_v2_co->tfp, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_esp32_ethernet_v2_co->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_esp32_ethernet_v2_co->tfp);
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
        tf_tfp_packet_processed(warp_esp32_ethernet_v2_co->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_esp32_ethernet_v2_co->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_warp_esp32_ethernet_v2_co_register_rmmi_interrupt_callback(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, TF_WARPESP32EthernetV2Co_RMMIInterruptHandler handler, void *user_data) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_esp32_ethernet_v2_co->rmmi_interrupt_handler = handler;
    warp_esp32_ethernet_v2_co->rmmi_interrupt_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_warp_esp32_ethernet_v2_co_callback_tick(TF_WARPESP32EthernetV2Co *warp_esp32_ethernet_v2_co, uint32_t timeout_us) {
    if (warp_esp32_ethernet_v2_co == NULL) {
        return TF_E_NULL;
    }

    if (warp_esp32_ethernet_v2_co->magic != 0x5446 || warp_esp32_ethernet_v2_co->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = warp_esp32_ethernet_v2_co->tfp->spitfp->hal;

    return tf_tfp_callback_tick(warp_esp32_ethernet_v2_co->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
