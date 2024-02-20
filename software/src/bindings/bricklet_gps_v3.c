/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_gps_v3.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_gps_v3_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_GPSV3 *gps_v3 = (TF_GPSV3 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(gps_v3->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_GPS_V3_CALLBACK_PULSE_PER_SECOND: {
            TF_GPSV3_PulsePerSecondHandler fn = gps_v3->pulse_per_second_handler;
            void *user_data = gps_v3->pulse_per_second_user_data;
            if (fn == NULL) {
                return false;
            }


            hal_common->locked = true;
            fn(gps_v3, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_GPS_V3_CALLBACK_COORDINATES: {
            TF_GPSV3_CoordinatesHandler fn = gps_v3->coordinates_handler;
            void *user_data = gps_v3->coordinates_user_data;
            if (fn == NULL) {
                return false;
            }

            uint32_t latitude = tf_packet_buffer_read_uint32_t(payload);
            char ns = tf_packet_buffer_read_char(payload);
            uint32_t longitude = tf_packet_buffer_read_uint32_t(payload);
            char ew = tf_packet_buffer_read_char(payload);
            hal_common->locked = true;
            fn(gps_v3, latitude, ns, longitude, ew, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_GPS_V3_CALLBACK_STATUS: {
            TF_GPSV3_StatusHandler fn = gps_v3->status_handler;
            void *user_data = gps_v3->status_user_data;
            if (fn == NULL) {
                return false;
            }

            bool has_fix = tf_packet_buffer_read_bool(payload);
            uint8_t satellites_view = tf_packet_buffer_read_uint8_t(payload);
            hal_common->locked = true;
            fn(gps_v3, has_fix, satellites_view, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_GPS_V3_CALLBACK_ALTITUDE: {
            TF_GPSV3_AltitudeHandler fn = gps_v3->altitude_handler;
            void *user_data = gps_v3->altitude_user_data;
            if (fn == NULL) {
                return false;
            }

            int32_t altitude = tf_packet_buffer_read_int32_t(payload);
            int32_t geoidal_separation = tf_packet_buffer_read_int32_t(payload);
            hal_common->locked = true;
            fn(gps_v3, altitude, geoidal_separation, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_GPS_V3_CALLBACK_MOTION: {
            TF_GPSV3_MotionHandler fn = gps_v3->motion_handler;
            void *user_data = gps_v3->motion_user_data;
            if (fn == NULL) {
                return false;
            }

            uint32_t course = tf_packet_buffer_read_uint32_t(payload);
            uint32_t speed = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(gps_v3, course, speed, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_GPS_V3_CALLBACK_DATE_TIME: {
            TF_GPSV3_DateTimeHandler fn = gps_v3->date_time_handler;
            void *user_data = gps_v3->date_time_user_data;
            if (fn == NULL) {
                return false;
            }

            uint32_t date = tf_packet_buffer_read_uint32_t(payload);
            uint32_t time = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(gps_v3, date, time, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_gps_v3_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_gps_v3_create(TF_GPSV3 *gps_v3, const char *uid_or_port_name, TF_HAL *hal) {
    if (gps_v3 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(gps_v3, 0, sizeof(TF_GPSV3));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_GPS_V3_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    gps_v3->tfp = tfp;
    gps_v3->tfp->device = gps_v3;
    gps_v3->tfp->cb_handler = tf_gps_v3_callback_handler;
    gps_v3->magic = 0x5446;
    gps_v3->response_expected[0] = 0x7C;
    gps_v3->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_gps_v3_destroy(TF_GPSV3 *gps_v3) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }
    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    gps_v3->tfp->cb_handler = NULL;
    gps_v3->tfp->device = NULL;
    gps_v3->tfp = NULL;
    gps_v3->magic = 0;

    return TF_E_OK;
}

int tf_gps_v3_get_response_expected(TF_GPSV3 *gps_v3, uint8_t function_id, bool *ret_response_expected) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_GPS_V3_FUNCTION_RESTART:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (gps_v3->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_FIX_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (gps_v3->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_COORDINATES_CALLBACK_PERIOD:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (gps_v3->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_STATUS_CALLBACK_PERIOD:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (gps_v3->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (gps_v3->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_MOTION_CALLBACK_PERIOD:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (gps_v3->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (gps_v3->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_SBAS_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (gps_v3->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (gps_v3->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (gps_v3->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_GPS_V3_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (gps_v3->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_GPS_V3_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (gps_v3->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_gps_v3_set_response_expected(TF_GPSV3 *gps_v3, uint8_t function_id, bool response_expected) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_GPS_V3_FUNCTION_RESTART:
            if (response_expected) {
                gps_v3->response_expected[0] |= (1 << 0);
            } else {
                gps_v3->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_FIX_LED_CONFIG:
            if (response_expected) {
                gps_v3->response_expected[0] |= (1 << 1);
            } else {
                gps_v3->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_COORDINATES_CALLBACK_PERIOD:
            if (response_expected) {
                gps_v3->response_expected[0] |= (1 << 2);
            } else {
                gps_v3->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_STATUS_CALLBACK_PERIOD:
            if (response_expected) {
                gps_v3->response_expected[0] |= (1 << 3);
            } else {
                gps_v3->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD:
            if (response_expected) {
                gps_v3->response_expected[0] |= (1 << 4);
            } else {
                gps_v3->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_MOTION_CALLBACK_PERIOD:
            if (response_expected) {
                gps_v3->response_expected[0] |= (1 << 5);
            } else {
                gps_v3->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD:
            if (response_expected) {
                gps_v3->response_expected[0] |= (1 << 6);
            } else {
                gps_v3->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_SBAS_CONFIG:
            if (response_expected) {
                gps_v3->response_expected[0] |= (1 << 7);
            } else {
                gps_v3->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                gps_v3->response_expected[1] |= (1 << 0);
            } else {
                gps_v3->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_GPS_V3_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                gps_v3->response_expected[1] |= (1 << 1);
            } else {
                gps_v3->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_GPS_V3_FUNCTION_RESET:
            if (response_expected) {
                gps_v3->response_expected[1] |= (1 << 2);
            } else {
                gps_v3->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_GPS_V3_FUNCTION_WRITE_UID:
            if (response_expected) {
                gps_v3->response_expected[1] |= (1 << 3);
            } else {
                gps_v3->response_expected[1] &= ~(1 << 3);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_gps_v3_set_response_expected_all(TF_GPSV3 *gps_v3, bool response_expected) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(gps_v3->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_gps_v3_get_coordinates(TF_GPSV3 *gps_v3, uint32_t *ret_latitude, char *ret_ns, uint32_t *ret_longitude, char *ret_ew) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_COORDINATES, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 10) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_latitude != NULL) { *ret_latitude = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_ns != NULL) { *ret_ns = tf_packet_buffer_read_char(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_longitude != NULL) { *ret_longitude = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_ew != NULL) { *ret_ew = tf_packet_buffer_read_char(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 10) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_status(TF_GPSV3 *gps_v3, bool *ret_has_fix, uint8_t *ret_satellites_view) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_STATUS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_has_fix != NULL) { *ret_has_fix = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_satellites_view != NULL) { *ret_satellites_view = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_altitude(TF_GPSV3 *gps_v3, int32_t *ret_altitude, int32_t *ret_geoidal_separation) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_ALTITUDE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_altitude != NULL) { *ret_altitude = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_geoidal_separation != NULL) { *ret_geoidal_separation = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_motion(TF_GPSV3 *gps_v3, uint32_t *ret_course, uint32_t *ret_speed) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_MOTION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_course != NULL) { *ret_course = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_speed != NULL) { *ret_speed = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_date_time(TF_GPSV3 *gps_v3, uint32_t *ret_date, uint32_t *ret_time) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_DATE_TIME, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_date != NULL) { *ret_date = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_time != NULL) { *ret_time = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_restart(TF_GPSV3 *gps_v3, uint8_t restart_type) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_gps_v3_get_response_expected(gps_v3, TF_GPS_V3_FUNCTION_RESTART, &_response_expected);
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_RESTART, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    _send_buf[0] = (uint8_t)restart_type;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_satellite_system_status_low_level(TF_GPSV3 *gps_v3, uint8_t satellite_system, uint8_t *ret_satellite_numbers_length, uint8_t ret_satellite_numbers_data[12], uint8_t *ret_fix, uint16_t *ret_pdop, uint16_t *ret_hdop, uint16_t *ret_vdop) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_SATELLITE_SYSTEM_STATUS_LOW_LEVEL, 1, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    _send_buf[0] = (uint8_t)satellite_system;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 20) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_satellite_numbers_length != NULL) { *ret_satellite_numbers_length = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_satellite_numbers_data != NULL) { for (_i = 0; _i < 12; ++_i) ret_satellite_numbers_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 12); }
            if (ret_fix != NULL) { *ret_fix = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_pdop != NULL) { *ret_pdop = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_hdop != NULL) { *ret_hdop = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_vdop != NULL) { *ret_vdop = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 20) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_satellite_status(TF_GPSV3 *gps_v3, uint8_t satellite_system, uint8_t satellite_number, int16_t *ret_elevation, int16_t *ret_azimuth, int16_t *ret_snr) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_SATELLITE_STATUS, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    _send_buf[0] = (uint8_t)satellite_system;
    _send_buf[1] = (uint8_t)satellite_number;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_elevation != NULL) { *ret_elevation = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_azimuth != NULL) { *ret_azimuth = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_snr != NULL) { *ret_snr = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_set_fix_led_config(TF_GPSV3 *gps_v3, uint8_t config) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_gps_v3_get_response_expected(gps_v3, TF_GPS_V3_FUNCTION_SET_FIX_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_SET_FIX_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_fix_led_config(TF_GPSV3 *gps_v3, uint8_t *ret_config) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_FIX_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_set_coordinates_callback_period(TF_GPSV3 *gps_v3, uint32_t period) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_gps_v3_get_response_expected(gps_v3, TF_GPS_V3_FUNCTION_SET_COORDINATES_CALLBACK_PERIOD, &_response_expected);
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_SET_COORDINATES_CALLBACK_PERIOD, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_coordinates_callback_period(TF_GPSV3 *gps_v3, uint32_t *ret_period) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_COORDINATES_CALLBACK_PERIOD, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_set_status_callback_period(TF_GPSV3 *gps_v3, uint32_t period) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_gps_v3_get_response_expected(gps_v3, TF_GPS_V3_FUNCTION_SET_STATUS_CALLBACK_PERIOD, &_response_expected);
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_SET_STATUS_CALLBACK_PERIOD, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_status_callback_period(TF_GPSV3 *gps_v3, uint32_t *ret_period) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_STATUS_CALLBACK_PERIOD, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_set_altitude_callback_period(TF_GPSV3 *gps_v3, uint32_t period) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_gps_v3_get_response_expected(gps_v3, TF_GPS_V3_FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD, &_response_expected);
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_altitude_callback_period(TF_GPSV3 *gps_v3, uint32_t *ret_period) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_ALTITUDE_CALLBACK_PERIOD, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_set_motion_callback_period(TF_GPSV3 *gps_v3, uint32_t period) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_gps_v3_get_response_expected(gps_v3, TF_GPS_V3_FUNCTION_SET_MOTION_CALLBACK_PERIOD, &_response_expected);
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_SET_MOTION_CALLBACK_PERIOD, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_motion_callback_period(TF_GPSV3 *gps_v3, uint32_t *ret_period) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_MOTION_CALLBACK_PERIOD, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_set_date_time_callback_period(TF_GPSV3 *gps_v3, uint32_t period) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_gps_v3_get_response_expected(gps_v3, TF_GPS_V3_FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD, &_response_expected);
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_date_time_callback_period(TF_GPSV3 *gps_v3, uint32_t *ret_period) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_DATE_TIME_CALLBACK_PERIOD, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_set_sbas_config(TF_GPSV3 *gps_v3, uint8_t sbas_config) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_gps_v3_get_response_expected(gps_v3, TF_GPS_V3_FUNCTION_SET_SBAS_CONFIG, &_response_expected);
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_SET_SBAS_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    _send_buf[0] = (uint8_t)sbas_config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_sbas_config(TF_GPSV3 *gps_v3, uint8_t *ret_sbas_config) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_SBAS_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_sbas_config != NULL) { *ret_sbas_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_spitfp_error_count(TF_GPSV3 *gps_v3, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_set_bootloader_mode(TF_GPSV3 *gps_v3, uint8_t mode, uint8_t *ret_status) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_bootloader_mode(TF_GPSV3 *gps_v3, uint8_t *ret_mode) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_set_write_firmware_pointer(TF_GPSV3 *gps_v3, uint32_t pointer) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_gps_v3_get_response_expected(gps_v3, TF_GPS_V3_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_write_firmware(TF_GPSV3 *gps_v3, const uint8_t data[64], uint8_t *ret_status) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_set_status_led_config(TF_GPSV3 *gps_v3, uint8_t config) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_gps_v3_get_response_expected(gps_v3, TF_GPS_V3_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_status_led_config(TF_GPSV3 *gps_v3, uint8_t *ret_config) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_chip_temperature(TF_GPSV3 *gps_v3, int16_t *ret_temperature) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_reset(TF_GPSV3 *gps_v3) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_gps_v3_get_response_expected(gps_v3, TF_GPS_V3_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_write_uid(TF_GPSV3 *gps_v3, uint32_t uid) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_gps_v3_get_response_expected(gps_v3, TF_GPS_V3_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(gps_v3->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_read_uid(TF_GPSV3 *gps_v3, uint32_t *ret_uid) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_identity(TF_GPSV3 *gps_v3, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = gps_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(gps_v3->tfp, TF_GPS_V3_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(gps_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(gps_v3->tfp);
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
        tf_tfp_packet_processed(gps_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(gps_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_gps_v3_get_satellite_system_status(TF_GPSV3 *gps_v3, uint8_t satellite_system, uint8_t *ret_satellite_numbers, uint8_t *ret_satellite_numbers_length, uint8_t *ret_fix, uint16_t *ret_pdop, uint16_t *ret_hdop, uint16_t *ret_vdop) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    int ret = TF_E_OK;
    uint8_t satellite_numbers_length = 0;
    uint8_t satellite_numbers_data[12];

    if (ret_satellite_numbers_length != NULL) {
        *ret_satellite_numbers_length = 0;
    }

    ret = tf_gps_v3_get_satellite_system_status_low_level(gps_v3, satellite_system, &satellite_numbers_length, satellite_numbers_data, ret_fix, ret_pdop, ret_hdop, ret_vdop);

    if (ret != TF_E_OK) {
        return ret;
    }

    if (ret_satellite_numbers != NULL) {
        memcpy(ret_satellite_numbers, satellite_numbers_data, sizeof(uint8_t) * satellite_numbers_length);
        memset(&ret_satellite_numbers[satellite_numbers_length], 0, sizeof(uint8_t) * (12 - satellite_numbers_length));
    }

    if (ret_satellite_numbers_length != NULL) {
        *ret_satellite_numbers_length = satellite_numbers_length;
    }

    return ret;
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_gps_v3_register_pulse_per_second_callback(TF_GPSV3 *gps_v3, TF_GPSV3_PulsePerSecondHandler handler, void *user_data) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    gps_v3->pulse_per_second_handler = handler;
    gps_v3->pulse_per_second_user_data = user_data;

    return TF_E_OK;
}


int tf_gps_v3_register_coordinates_callback(TF_GPSV3 *gps_v3, TF_GPSV3_CoordinatesHandler handler, void *user_data) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    gps_v3->coordinates_handler = handler;
    gps_v3->coordinates_user_data = user_data;

    return TF_E_OK;
}


int tf_gps_v3_register_status_callback(TF_GPSV3 *gps_v3, TF_GPSV3_StatusHandler handler, void *user_data) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    gps_v3->status_handler = handler;
    gps_v3->status_user_data = user_data;

    return TF_E_OK;
}


int tf_gps_v3_register_altitude_callback(TF_GPSV3 *gps_v3, TF_GPSV3_AltitudeHandler handler, void *user_data) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    gps_v3->altitude_handler = handler;
    gps_v3->altitude_user_data = user_data;

    return TF_E_OK;
}


int tf_gps_v3_register_motion_callback(TF_GPSV3 *gps_v3, TF_GPSV3_MotionHandler handler, void *user_data) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    gps_v3->motion_handler = handler;
    gps_v3->motion_user_data = user_data;

    return TF_E_OK;
}


int tf_gps_v3_register_date_time_callback(TF_GPSV3 *gps_v3, TF_GPSV3_DateTimeHandler handler, void *user_data) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    gps_v3->date_time_handler = handler;
    gps_v3->date_time_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_gps_v3_callback_tick(TF_GPSV3 *gps_v3, uint32_t timeout_us) {
    if (gps_v3 == NULL) {
        return TF_E_NULL;
    }

    if (gps_v3->magic != 0x5446 || gps_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = gps_v3->tfp->spitfp->hal;

    return tf_tfp_callback_tick(gps_v3->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
