/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_outdoor_weather.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_outdoor_weather_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_OutdoorWeather *outdoor_weather = (TF_OutdoorWeather *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(outdoor_weather->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_OUTDOOR_WEATHER_CALLBACK_STATION_DATA: {
            TF_OutdoorWeather_StationDataHandler fn = outdoor_weather->station_data_handler;
            void *user_data = outdoor_weather->station_data_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t identifier = tf_packet_buffer_read_uint8_t(payload);
            int16_t temperature = tf_packet_buffer_read_int16_t(payload);
            uint8_t humidity = tf_packet_buffer_read_uint8_t(payload);
            uint32_t wind_speed = tf_packet_buffer_read_uint32_t(payload);
            uint32_t gust_speed = tf_packet_buffer_read_uint32_t(payload);
            uint32_t rain = tf_packet_buffer_read_uint32_t(payload);
            uint8_t wind_direction = tf_packet_buffer_read_uint8_t(payload);
            bool battery_low = tf_packet_buffer_read_bool(payload);
            hal_common->locked = true;
            fn(outdoor_weather, identifier, temperature, humidity, wind_speed, gust_speed, rain, wind_direction, battery_low, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_OUTDOOR_WEATHER_CALLBACK_SENSOR_DATA: {
            TF_OutdoorWeather_SensorDataHandler fn = outdoor_weather->sensor_data_handler;
            void *user_data = outdoor_weather->sensor_data_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t identifier = tf_packet_buffer_read_uint8_t(payload);
            int16_t temperature = tf_packet_buffer_read_int16_t(payload);
            uint8_t humidity = tf_packet_buffer_read_uint8_t(payload);
            hal_common->locked = true;
            fn(outdoor_weather, identifier, temperature, humidity, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_outdoor_weather_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_outdoor_weather_create(TF_OutdoorWeather *outdoor_weather, const char *uid_or_port_name, TF_HAL *hal) {
    if (outdoor_weather == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(outdoor_weather, 0, sizeof(TF_OutdoorWeather));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_OUTDOOR_WEATHER_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    outdoor_weather->tfp = tfp;
    outdoor_weather->tfp->device = outdoor_weather;
    outdoor_weather->tfp->cb_handler = tf_outdoor_weather_callback_handler;
    outdoor_weather->magic = 0x5446;
    outdoor_weather->response_expected[0] = 0x03;
    return TF_E_OK;
}

int tf_outdoor_weather_destroy(TF_OutdoorWeather *outdoor_weather) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }
    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    outdoor_weather->tfp->cb_handler = NULL;
    outdoor_weather->tfp->device = NULL;
    outdoor_weather->tfp = NULL;
    outdoor_weather->magic = 0;

    return TF_E_OK;
}

int tf_outdoor_weather_get_response_expected(TF_OutdoorWeather *outdoor_weather, uint8_t function_id, bool *ret_response_expected) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_OUTDOOR_WEATHER_FUNCTION_SET_STATION_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (outdoor_weather->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_OUTDOOR_WEATHER_FUNCTION_SET_SENSOR_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (outdoor_weather->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_OUTDOOR_WEATHER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (outdoor_weather->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_OUTDOOR_WEATHER_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (outdoor_weather->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_OUTDOOR_WEATHER_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (outdoor_weather->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_OUTDOOR_WEATHER_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (outdoor_weather->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_outdoor_weather_set_response_expected(TF_OutdoorWeather *outdoor_weather, uint8_t function_id, bool response_expected) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_OUTDOOR_WEATHER_FUNCTION_SET_STATION_CALLBACK_CONFIGURATION:
            if (response_expected) {
                outdoor_weather->response_expected[0] |= (1 << 0);
            } else {
                outdoor_weather->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_OUTDOOR_WEATHER_FUNCTION_SET_SENSOR_CALLBACK_CONFIGURATION:
            if (response_expected) {
                outdoor_weather->response_expected[0] |= (1 << 1);
            } else {
                outdoor_weather->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_OUTDOOR_WEATHER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                outdoor_weather->response_expected[0] |= (1 << 2);
            } else {
                outdoor_weather->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_OUTDOOR_WEATHER_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                outdoor_weather->response_expected[0] |= (1 << 3);
            } else {
                outdoor_weather->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_OUTDOOR_WEATHER_FUNCTION_RESET:
            if (response_expected) {
                outdoor_weather->response_expected[0] |= (1 << 4);
            } else {
                outdoor_weather->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_OUTDOOR_WEATHER_FUNCTION_WRITE_UID:
            if (response_expected) {
                outdoor_weather->response_expected[0] |= (1 << 5);
            } else {
                outdoor_weather->response_expected[0] &= ~(1 << 5);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_outdoor_weather_set_response_expected_all(TF_OutdoorWeather *outdoor_weather, bool response_expected) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(outdoor_weather->response_expected, response_expected ? 0xFF : 0, 1);

    return TF_E_OK;
}

int tf_outdoor_weather_get_station_identifiers_low_level(TF_OutdoorWeather *outdoor_weather, uint16_t *ret_identifiers_length, uint16_t *ret_identifiers_chunk_offset, uint8_t ret_identifiers_chunk_data[60]) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_GET_STATION_IDENTIFIERS_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_identifiers_length != NULL) { *ret_identifiers_length = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_identifiers_chunk_offset != NULL) { *ret_identifiers_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_identifiers_chunk_data != NULL) { for (_i = 0; _i < 60; ++_i) ret_identifiers_chunk_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_get_sensor_identifiers_low_level(TF_OutdoorWeather *outdoor_weather, uint16_t *ret_identifiers_length, uint16_t *ret_identifiers_chunk_offset, uint8_t ret_identifiers_chunk_data[60]) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_GET_SENSOR_IDENTIFIERS_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_identifiers_length != NULL) { *ret_identifiers_length = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_identifiers_chunk_offset != NULL) { *ret_identifiers_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_identifiers_chunk_data != NULL) { for (_i = 0; _i < 60; ++_i) ret_identifiers_chunk_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_get_station_data(TF_OutdoorWeather *outdoor_weather, uint8_t identifier, int16_t *ret_temperature, uint8_t *ret_humidity, uint32_t *ret_wind_speed, uint32_t *ret_gust_speed, uint32_t *ret_rain, uint8_t *ret_wind_direction, bool *ret_battery_low, uint16_t *ret_last_change) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_GET_STATION_DATA, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(outdoor_weather->tfp);

    _send_buf[0] = (uint8_t)identifier;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 19) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_humidity != NULL) { *ret_humidity = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_wind_speed != NULL) { *ret_wind_speed = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_gust_speed != NULL) { *ret_gust_speed = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_rain != NULL) { *ret_rain = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_wind_direction != NULL) { *ret_wind_direction = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_battery_low != NULL) { *ret_battery_low = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_last_change != NULL) { *ret_last_change = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 19) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_get_sensor_data(TF_OutdoorWeather *outdoor_weather, uint8_t identifier, int16_t *ret_temperature, uint8_t *ret_humidity, uint16_t *ret_last_change) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_GET_SENSOR_DATA, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(outdoor_weather->tfp);

    _send_buf[0] = (uint8_t)identifier;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_humidity != NULL) { *ret_humidity = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_last_change != NULL) { *ret_last_change = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_set_station_callback_configuration(TF_OutdoorWeather *outdoor_weather, bool enable_callback) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_outdoor_weather_get_response_expected(outdoor_weather, TF_OUTDOOR_WEATHER_FUNCTION_SET_STATION_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_SET_STATION_CALLBACK_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(outdoor_weather->tfp);

    _send_buf[0] = enable_callback ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_get_station_callback_configuration(TF_OutdoorWeather *outdoor_weather, bool *ret_enable_callback) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_GET_STATION_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enable_callback != NULL) { *ret_enable_callback = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_set_sensor_callback_configuration(TF_OutdoorWeather *outdoor_weather, bool enable_callback) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_outdoor_weather_get_response_expected(outdoor_weather, TF_OUTDOOR_WEATHER_FUNCTION_SET_SENSOR_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_SET_SENSOR_CALLBACK_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(outdoor_weather->tfp);

    _send_buf[0] = enable_callback ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_get_sensor_callback_configuration(TF_OutdoorWeather *outdoor_weather, bool *ret_enable_callback) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_GET_SENSOR_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enable_callback != NULL) { *ret_enable_callback = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_get_spitfp_error_count(TF_OutdoorWeather *outdoor_weather, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_set_bootloader_mode(TF_OutdoorWeather *outdoor_weather, uint8_t mode, uint8_t *ret_status) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(outdoor_weather->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_get_bootloader_mode(TF_OutdoorWeather *outdoor_weather, uint8_t *ret_mode) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_set_write_firmware_pointer(TF_OutdoorWeather *outdoor_weather, uint32_t pointer) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_outdoor_weather_get_response_expected(outdoor_weather, TF_OUTDOOR_WEATHER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(outdoor_weather->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_write_firmware(TF_OutdoorWeather *outdoor_weather, const uint8_t data[64], uint8_t *ret_status) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(outdoor_weather->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_set_status_led_config(TF_OutdoorWeather *outdoor_weather, uint8_t config) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_outdoor_weather_get_response_expected(outdoor_weather, TF_OUTDOOR_WEATHER_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(outdoor_weather->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_get_status_led_config(TF_OutdoorWeather *outdoor_weather, uint8_t *ret_config) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_get_chip_temperature(TF_OutdoorWeather *outdoor_weather, int16_t *ret_temperature) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_reset(TF_OutdoorWeather *outdoor_weather) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_outdoor_weather_get_response_expected(outdoor_weather, TF_OUTDOOR_WEATHER_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_write_uid(TF_OutdoorWeather *outdoor_weather, uint32_t uid) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_outdoor_weather_get_response_expected(outdoor_weather, TF_OUTDOOR_WEATHER_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(outdoor_weather->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_read_uid(TF_OutdoorWeather *outdoor_weather, uint32_t *ret_uid) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_outdoor_weather_get_identity(TF_OutdoorWeather *outdoor_weather, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = outdoor_weather->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(outdoor_weather->tfp, TF_OUTDOOR_WEATHER_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(outdoor_weather->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(outdoor_weather->tfp);
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
        tf_tfp_packet_processed(outdoor_weather->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(outdoor_weather->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

static int tf_outdoor_weather_get_station_identifiers_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    (void)wrapper_data;
    uint16_t identifiers_length = 0;
    uint16_t identifiers_chunk_offset = 0;
    uint8_t *identifiers_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_outdoor_weather_get_station_identifiers_low_level((TF_OutdoorWeather *)device, &identifiers_length, &identifiers_chunk_offset, identifiers_chunk_data);

    *ret_stream_length = (uint32_t)identifiers_length;
    *ret_chunk_offset = (uint32_t)identifiers_chunk_offset;
    return ret;
}

int tf_outdoor_weather_get_station_identifiers(TF_OutdoorWeather *outdoor_weather, uint8_t *ret_identifiers, uint16_t *ret_identifiers_length) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    uint32_t _identifiers_length = 0;
    uint8_t _identifiers_chunk_data[60];

    int ret = tf_stream_out(outdoor_weather, tf_outdoor_weather_get_station_identifiers_ll_wrapper, NULL, ret_identifiers, &_identifiers_length, _identifiers_chunk_data, 60, tf_copy_items_uint8_t);

    if (ret_identifiers_length != NULL) {
        *ret_identifiers_length = (uint16_t)_identifiers_length;
    }
    return ret;
}

static int tf_outdoor_weather_get_sensor_identifiers_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    (void)wrapper_data;
    uint16_t identifiers_length = 0;
    uint16_t identifiers_chunk_offset = 0;
    uint8_t *identifiers_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_outdoor_weather_get_sensor_identifiers_low_level((TF_OutdoorWeather *)device, &identifiers_length, &identifiers_chunk_offset, identifiers_chunk_data);

    *ret_stream_length = (uint32_t)identifiers_length;
    *ret_chunk_offset = (uint32_t)identifiers_chunk_offset;
    return ret;
}

int tf_outdoor_weather_get_sensor_identifiers(TF_OutdoorWeather *outdoor_weather, uint8_t *ret_identifiers, uint16_t *ret_identifiers_length) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    uint32_t _identifiers_length = 0;
    uint8_t _identifiers_chunk_data[60];

    int ret = tf_stream_out(outdoor_weather, tf_outdoor_weather_get_sensor_identifiers_ll_wrapper, NULL, ret_identifiers, &_identifiers_length, _identifiers_chunk_data, 60, tf_copy_items_uint8_t);

    if (ret_identifiers_length != NULL) {
        *ret_identifiers_length = (uint16_t)_identifiers_length;
    }
    return ret;
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_outdoor_weather_register_station_data_callback(TF_OutdoorWeather *outdoor_weather, TF_OutdoorWeather_StationDataHandler handler, void *user_data) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    outdoor_weather->station_data_handler = handler;
    outdoor_weather->station_data_user_data = user_data;

    return TF_E_OK;
}


int tf_outdoor_weather_register_sensor_data_callback(TF_OutdoorWeather *outdoor_weather, TF_OutdoorWeather_SensorDataHandler handler, void *user_data) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    outdoor_weather->sensor_data_handler = handler;
    outdoor_weather->sensor_data_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_outdoor_weather_callback_tick(TF_OutdoorWeather *outdoor_weather, uint32_t timeout_us) {
    if (outdoor_weather == NULL) {
        return TF_E_NULL;
    }

    if (outdoor_weather->magic != 0x5446 || outdoor_weather->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = outdoor_weather->tfp->spitfp->hal;

    return tf_tfp_callback_tick(outdoor_weather->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
