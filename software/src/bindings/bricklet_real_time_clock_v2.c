/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_real_time_clock_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_real_time_clock_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_RealTimeClockV2 *real_time_clock_v2 = (TF_RealTimeClockV2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(real_time_clock_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_REAL_TIME_CLOCK_V2_CALLBACK_DATE_TIME: {
            TF_RealTimeClockV2_DateTimeHandler fn = real_time_clock_v2->date_time_handler;
            void *user_data = real_time_clock_v2->date_time_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t year = tf_packet_buffer_read_uint16_t(payload);
            uint8_t month = tf_packet_buffer_read_uint8_t(payload);
            uint8_t day = tf_packet_buffer_read_uint8_t(payload);
            uint8_t hour = tf_packet_buffer_read_uint8_t(payload);
            uint8_t minute = tf_packet_buffer_read_uint8_t(payload);
            uint8_t second = tf_packet_buffer_read_uint8_t(payload);
            uint8_t centisecond = tf_packet_buffer_read_uint8_t(payload);
            uint8_t weekday = tf_packet_buffer_read_uint8_t(payload);
            int64_t timestamp = tf_packet_buffer_read_int64_t(payload);
            hal_common->locked = true;
            fn(real_time_clock_v2, year, month, day, hour, minute, second, centisecond, weekday, timestamp, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_REAL_TIME_CLOCK_V2_CALLBACK_ALARM: {
            TF_RealTimeClockV2_AlarmHandler fn = real_time_clock_v2->alarm_handler;
            void *user_data = real_time_clock_v2->alarm_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t year = tf_packet_buffer_read_uint16_t(payload);
            uint8_t month = tf_packet_buffer_read_uint8_t(payload);
            uint8_t day = tf_packet_buffer_read_uint8_t(payload);
            uint8_t hour = tf_packet_buffer_read_uint8_t(payload);
            uint8_t minute = tf_packet_buffer_read_uint8_t(payload);
            uint8_t second = tf_packet_buffer_read_uint8_t(payload);
            uint8_t centisecond = tf_packet_buffer_read_uint8_t(payload);
            uint8_t weekday = tf_packet_buffer_read_uint8_t(payload);
            int64_t timestamp = tf_packet_buffer_read_int64_t(payload);
            hal_common->locked = true;
            fn(real_time_clock_v2, year, month, day, hour, minute, second, centisecond, weekday, timestamp, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_real_time_clock_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_real_time_clock_v2_create(TF_RealTimeClockV2 *real_time_clock_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (real_time_clock_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(real_time_clock_v2, 0, sizeof(TF_RealTimeClockV2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_REAL_TIME_CLOCK_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    real_time_clock_v2->tfp = tfp;
    real_time_clock_v2->tfp->device = real_time_clock_v2;
    real_time_clock_v2->tfp->cb_handler = tf_real_time_clock_v2_callback_handler;
    real_time_clock_v2->magic = 0x5446;
    real_time_clock_v2->response_expected[0] = 0x0C;
    return TF_E_OK;
}

int tf_real_time_clock_v2_destroy(TF_RealTimeClockV2 *real_time_clock_v2) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }
    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    real_time_clock_v2->tfp->cb_handler = NULL;
    real_time_clock_v2->tfp->device = NULL;
    real_time_clock_v2->tfp = NULL;
    real_time_clock_v2->magic = 0;

    return TF_E_OK;
}

int tf_real_time_clock_v2_get_response_expected(TF_RealTimeClockV2 *real_time_clock_v2, uint8_t function_id, bool *ret_response_expected) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_DATE_TIME:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (real_time_clock_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_OFFSET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (real_time_clock_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_DATE_TIME_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (real_time_clock_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_ALARM:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (real_time_clock_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (real_time_clock_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (real_time_clock_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (real_time_clock_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (real_time_clock_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_real_time_clock_v2_set_response_expected(TF_RealTimeClockV2 *real_time_clock_v2, uint8_t function_id, bool response_expected) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_DATE_TIME:
            if (response_expected) {
                real_time_clock_v2->response_expected[0] |= (1 << 0);
            } else {
                real_time_clock_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_OFFSET:
            if (response_expected) {
                real_time_clock_v2->response_expected[0] |= (1 << 1);
            } else {
                real_time_clock_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_DATE_TIME_CALLBACK_CONFIGURATION:
            if (response_expected) {
                real_time_clock_v2->response_expected[0] |= (1 << 2);
            } else {
                real_time_clock_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_ALARM:
            if (response_expected) {
                real_time_clock_v2->response_expected[0] |= (1 << 3);
            } else {
                real_time_clock_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                real_time_clock_v2->response_expected[0] |= (1 << 4);
            } else {
                real_time_clock_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                real_time_clock_v2->response_expected[0] |= (1 << 5);
            } else {
                real_time_clock_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_RESET:
            if (response_expected) {
                real_time_clock_v2->response_expected[0] |= (1 << 6);
            } else {
                real_time_clock_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_REAL_TIME_CLOCK_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                real_time_clock_v2->response_expected[0] |= (1 << 7);
            } else {
                real_time_clock_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_real_time_clock_v2_set_response_expected_all(TF_RealTimeClockV2 *real_time_clock_v2, bool response_expected) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(real_time_clock_v2->response_expected, response_expected ? 0xFF : 0, 1);

    return TF_E_OK;
}

int tf_real_time_clock_v2_set_date_time(TF_RealTimeClockV2 *real_time_clock_v2, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint8_t centisecond, uint8_t weekday) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_real_time_clock_v2_get_response_expected(real_time_clock_v2, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_DATE_TIME, &_response_expected);
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_DATE_TIME, 9, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(real_time_clock_v2->tfp);

    year = tf_leconvert_uint16_to(year); memcpy(_send_buf + 0, &year, 2);
    _send_buf[2] = (uint8_t)month;
    _send_buf[3] = (uint8_t)day;
    _send_buf[4] = (uint8_t)hour;
    _send_buf[5] = (uint8_t)minute;
    _send_buf[6] = (uint8_t)second;
    _send_buf[7] = (uint8_t)centisecond;
    _send_buf[8] = (uint8_t)weekday;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_get_date_time(TF_RealTimeClockV2 *real_time_clock_v2, uint16_t *ret_year, uint8_t *ret_month, uint8_t *ret_day, uint8_t *ret_hour, uint8_t *ret_minute, uint8_t *ret_second, uint8_t *ret_centisecond, uint8_t *ret_weekday, int64_t *ret_timestamp) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_GET_DATE_TIME, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
        if (_error_code != 0 || _length != 17) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_year != NULL) { *ret_year = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_month != NULL) { *ret_month = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_day != NULL) { *ret_day = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_hour != NULL) { *ret_hour = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_minute != NULL) { *ret_minute = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_second != NULL) { *ret_second = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_centisecond != NULL) { *ret_centisecond = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_weekday != NULL) { *ret_weekday = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_timestamp != NULL) { *ret_timestamp = tf_packet_buffer_read_int64_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 8); }
        }
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 17) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_get_timestamp(TF_RealTimeClockV2 *real_time_clock_v2, int64_t *ret_timestamp) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_GET_TIMESTAMP, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_timestamp != NULL) { *ret_timestamp = tf_packet_buffer_read_int64_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 8); }
        }
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_set_offset(TF_RealTimeClockV2 *real_time_clock_v2, int8_t offset) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_real_time_clock_v2_get_response_expected(real_time_clock_v2, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_OFFSET, &_response_expected);
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_OFFSET, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(real_time_clock_v2->tfp);

    _send_buf[0] = (uint8_t)offset;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_get_offset(TF_RealTimeClockV2 *real_time_clock_v2, int8_t *ret_offset) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_GET_OFFSET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_offset != NULL) { *ret_offset = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_set_date_time_callback_configuration(TF_RealTimeClockV2 *real_time_clock_v2, uint32_t period) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_real_time_clock_v2_get_response_expected(real_time_clock_v2, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_DATE_TIME_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_DATE_TIME_CALLBACK_CONFIGURATION, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(real_time_clock_v2->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_get_date_time_callback_configuration(TF_RealTimeClockV2 *real_time_clock_v2, uint32_t *ret_period) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_GET_DATE_TIME_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_set_alarm(TF_RealTimeClockV2 *real_time_clock_v2, int8_t month, int8_t day, int8_t hour, int8_t minute, int8_t second, int8_t weekday, int32_t interval) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_real_time_clock_v2_get_response_expected(real_time_clock_v2, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_ALARM, &_response_expected);
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_ALARM, 10, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(real_time_clock_v2->tfp);

    _send_buf[0] = (uint8_t)month;
    _send_buf[1] = (uint8_t)day;
    _send_buf[2] = (uint8_t)hour;
    _send_buf[3] = (uint8_t)minute;
    _send_buf[4] = (uint8_t)second;
    _send_buf[5] = (uint8_t)weekday;
    interval = tf_leconvert_int32_to(interval); memcpy(_send_buf + 6, &interval, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_get_alarm(TF_RealTimeClockV2 *real_time_clock_v2, int8_t *ret_month, int8_t *ret_day, int8_t *ret_hour, int8_t *ret_minute, int8_t *ret_second, int8_t *ret_weekday, int32_t *ret_interval) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_GET_ALARM, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
        if (_error_code != 0 || _length != 10) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_month != NULL) { *ret_month = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_day != NULL) { *ret_day = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_hour != NULL) { *ret_hour = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_minute != NULL) { *ret_minute = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_second != NULL) { *ret_second = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_weekday != NULL) { *ret_weekday = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_interval != NULL) { *ret_interval = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 10) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_get_spitfp_error_count(TF_RealTimeClockV2 *real_time_clock_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_set_bootloader_mode(TF_RealTimeClockV2 *real_time_clock_v2, uint8_t mode, uint8_t *ret_status) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(real_time_clock_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_get_bootloader_mode(TF_RealTimeClockV2 *real_time_clock_v2, uint8_t *ret_mode) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_set_write_firmware_pointer(TF_RealTimeClockV2 *real_time_clock_v2, uint32_t pointer) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_real_time_clock_v2_get_response_expected(real_time_clock_v2, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(real_time_clock_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_write_firmware(TF_RealTimeClockV2 *real_time_clock_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(real_time_clock_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_set_status_led_config(TF_RealTimeClockV2 *real_time_clock_v2, uint8_t config) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_real_time_clock_v2_get_response_expected(real_time_clock_v2, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(real_time_clock_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_get_status_led_config(TF_RealTimeClockV2 *real_time_clock_v2, uint8_t *ret_config) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_get_chip_temperature(TF_RealTimeClockV2 *real_time_clock_v2, int16_t *ret_temperature) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_reset(TF_RealTimeClockV2 *real_time_clock_v2) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_real_time_clock_v2_get_response_expected(real_time_clock_v2, TF_REAL_TIME_CLOCK_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_write_uid(TF_RealTimeClockV2 *real_time_clock_v2, uint32_t uid) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_real_time_clock_v2_get_response_expected(real_time_clock_v2, TF_REAL_TIME_CLOCK_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(real_time_clock_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_read_uid(TF_RealTimeClockV2 *real_time_clock_v2, uint32_t *ret_uid) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_real_time_clock_v2_get_identity(TF_RealTimeClockV2 *real_time_clock_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = real_time_clock_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(real_time_clock_v2->tfp, TF_REAL_TIME_CLOCK_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(real_time_clock_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(real_time_clock_v2->tfp);
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
        tf_tfp_packet_processed(real_time_clock_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(real_time_clock_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_real_time_clock_v2_register_date_time_callback(TF_RealTimeClockV2 *real_time_clock_v2, TF_RealTimeClockV2_DateTimeHandler handler, void *user_data) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    real_time_clock_v2->date_time_handler = handler;
    real_time_clock_v2->date_time_user_data = user_data;

    return TF_E_OK;
}


int tf_real_time_clock_v2_register_alarm_callback(TF_RealTimeClockV2 *real_time_clock_v2, TF_RealTimeClockV2_AlarmHandler handler, void *user_data) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    real_time_clock_v2->alarm_handler = handler;
    real_time_clock_v2->alarm_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_real_time_clock_v2_callback_tick(TF_RealTimeClockV2 *real_time_clock_v2, uint32_t timeout_us) {
    if (real_time_clock_v2 == NULL) {
        return TF_E_NULL;
    }

    if (real_time_clock_v2->magic != 0x5446 || real_time_clock_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = real_time_clock_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(real_time_clock_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
