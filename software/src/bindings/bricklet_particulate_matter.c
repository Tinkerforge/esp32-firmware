/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_particulate_matter.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_particulate_matter_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_ParticulateMatter *particulate_matter = (TF_ParticulateMatter *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(particulate_matter->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_PARTICULATE_MATTER_CALLBACK_PM_CONCENTRATION: {
            TF_ParticulateMatter_PMConcentrationHandler fn = particulate_matter->pm_concentration_handler;
            void *user_data = particulate_matter->pm_concentration_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t pm10 = tf_packet_buffer_read_uint16_t(payload);
            uint16_t pm25 = tf_packet_buffer_read_uint16_t(payload);
            uint16_t pm100 = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(particulate_matter, pm10, pm25, pm100, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_PARTICULATE_MATTER_CALLBACK_PM_COUNT: {
            TF_ParticulateMatter_PMCountHandler fn = particulate_matter->pm_count_handler;
            void *user_data = particulate_matter->pm_count_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t greater03um = tf_packet_buffer_read_uint16_t(payload);
            uint16_t greater05um = tf_packet_buffer_read_uint16_t(payload);
            uint16_t greater10um = tf_packet_buffer_read_uint16_t(payload);
            uint16_t greater25um = tf_packet_buffer_read_uint16_t(payload);
            uint16_t greater50um = tf_packet_buffer_read_uint16_t(payload);
            uint16_t greater100um = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(particulate_matter, greater03um, greater05um, greater10um, greater25um, greater50um, greater100um, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_particulate_matter_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_particulate_matter_create(TF_ParticulateMatter *particulate_matter, const char *uid_or_port_name, TF_HAL *hal) {
    if (particulate_matter == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(particulate_matter, 0, sizeof(TF_ParticulateMatter));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_PARTICULATE_MATTER_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    particulate_matter->tfp = tfp;
    particulate_matter->tfp->device = particulate_matter;
    particulate_matter->tfp->cb_handler = tf_particulate_matter_callback_handler;
    particulate_matter->magic = 0x5446;
    particulate_matter->response_expected[0] = 0x06;
    return TF_E_OK;
}

int tf_particulate_matter_destroy(TF_ParticulateMatter *particulate_matter) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }
    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    particulate_matter->tfp->cb_handler = NULL;
    particulate_matter->tfp->device = NULL;
    particulate_matter->tfp = NULL;
    particulate_matter->magic = 0;

    return TF_E_OK;
}

int tf_particulate_matter_get_response_expected(TF_ParticulateMatter *particulate_matter, uint8_t function_id, bool *ret_response_expected) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_PARTICULATE_MATTER_FUNCTION_SET_ENABLE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (particulate_matter->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_PARTICULATE_MATTER_FUNCTION_SET_PM_CONCENTRATION_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (particulate_matter->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_PARTICULATE_MATTER_FUNCTION_SET_PM_COUNT_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (particulate_matter->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_PARTICULATE_MATTER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (particulate_matter->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_PARTICULATE_MATTER_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (particulate_matter->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_PARTICULATE_MATTER_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (particulate_matter->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_PARTICULATE_MATTER_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (particulate_matter->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_particulate_matter_set_response_expected(TF_ParticulateMatter *particulate_matter, uint8_t function_id, bool response_expected) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_PARTICULATE_MATTER_FUNCTION_SET_ENABLE:
            if (response_expected) {
                particulate_matter->response_expected[0] |= (1 << 0);
            } else {
                particulate_matter->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_PARTICULATE_MATTER_FUNCTION_SET_PM_CONCENTRATION_CALLBACK_CONFIGURATION:
            if (response_expected) {
                particulate_matter->response_expected[0] |= (1 << 1);
            } else {
                particulate_matter->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_PARTICULATE_MATTER_FUNCTION_SET_PM_COUNT_CALLBACK_CONFIGURATION:
            if (response_expected) {
                particulate_matter->response_expected[0] |= (1 << 2);
            } else {
                particulate_matter->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_PARTICULATE_MATTER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                particulate_matter->response_expected[0] |= (1 << 3);
            } else {
                particulate_matter->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_PARTICULATE_MATTER_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                particulate_matter->response_expected[0] |= (1 << 4);
            } else {
                particulate_matter->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_PARTICULATE_MATTER_FUNCTION_RESET:
            if (response_expected) {
                particulate_matter->response_expected[0] |= (1 << 5);
            } else {
                particulate_matter->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_PARTICULATE_MATTER_FUNCTION_WRITE_UID:
            if (response_expected) {
                particulate_matter->response_expected[0] |= (1 << 6);
            } else {
                particulate_matter->response_expected[0] &= ~(1 << 6);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_particulate_matter_set_response_expected_all(TF_ParticulateMatter *particulate_matter, bool response_expected) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(particulate_matter->response_expected, response_expected ? 0xFF : 0, 1);

    return TF_E_OK;
}

int tf_particulate_matter_get_pm_concentration(TF_ParticulateMatter *particulate_matter, uint16_t *ret_pm10, uint16_t *ret_pm25, uint16_t *ret_pm100) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_GET_PM_CONCENTRATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_pm10 != NULL) { *ret_pm10 = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_pm25 != NULL) { *ret_pm25 = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_pm100 != NULL) { *ret_pm100 = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_get_pm_count(TF_ParticulateMatter *particulate_matter, uint16_t *ret_greater03um, uint16_t *ret_greater05um, uint16_t *ret_greater10um, uint16_t *ret_greater25um, uint16_t *ret_greater50um, uint16_t *ret_greater100um) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_GET_PM_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 12) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_greater03um != NULL) { *ret_greater03um = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_greater05um != NULL) { *ret_greater05um = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_greater10um != NULL) { *ret_greater10um = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_greater25um != NULL) { *ret_greater25um = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_greater50um != NULL) { *ret_greater50um = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_greater100um != NULL) { *ret_greater100um = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 12) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_set_enable(TF_ParticulateMatter *particulate_matter, bool enable) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_particulate_matter_get_response_expected(particulate_matter, TF_PARTICULATE_MATTER_FUNCTION_SET_ENABLE, &_response_expected);
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_SET_ENABLE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(particulate_matter->tfp);

    _send_buf[0] = enable ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_get_enable(TF_ParticulateMatter *particulate_matter, bool *ret_enable) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_GET_ENABLE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enable != NULL) { *ret_enable = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_get_sensor_info(TF_ParticulateMatter *particulate_matter, uint8_t *ret_sensor_version, uint8_t *ret_last_error_code, uint8_t *ret_framing_error_count, uint8_t *ret_checksum_error_count) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_GET_SENSOR_INFO, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_sensor_version != NULL) { *ret_sensor_version = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_last_error_code != NULL) { *ret_last_error_code = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_framing_error_count != NULL) { *ret_framing_error_count = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_checksum_error_count != NULL) { *ret_checksum_error_count = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_set_pm_concentration_callback_configuration(TF_ParticulateMatter *particulate_matter, uint32_t period, bool value_has_to_change) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_particulate_matter_get_response_expected(particulate_matter, TF_PARTICULATE_MATTER_FUNCTION_SET_PM_CONCENTRATION_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_SET_PM_CONCENTRATION_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(particulate_matter->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_get_pm_concentration_callback_configuration(TF_ParticulateMatter *particulate_matter, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_GET_PM_CONCENTRATION_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_set_pm_count_callback_configuration(TF_ParticulateMatter *particulate_matter, uint32_t period, bool value_has_to_change) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_particulate_matter_get_response_expected(particulate_matter, TF_PARTICULATE_MATTER_FUNCTION_SET_PM_COUNT_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_SET_PM_COUNT_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(particulate_matter->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_get_pm_count_callback_configuration(TF_ParticulateMatter *particulate_matter, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_GET_PM_COUNT_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_get_spitfp_error_count(TF_ParticulateMatter *particulate_matter, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_set_bootloader_mode(TF_ParticulateMatter *particulate_matter, uint8_t mode, uint8_t *ret_status) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(particulate_matter->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_get_bootloader_mode(TF_ParticulateMatter *particulate_matter, uint8_t *ret_mode) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_set_write_firmware_pointer(TF_ParticulateMatter *particulate_matter, uint32_t pointer) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_particulate_matter_get_response_expected(particulate_matter, TF_PARTICULATE_MATTER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(particulate_matter->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_write_firmware(TF_ParticulateMatter *particulate_matter, const uint8_t data[64], uint8_t *ret_status) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(particulate_matter->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_set_status_led_config(TF_ParticulateMatter *particulate_matter, uint8_t config) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_particulate_matter_get_response_expected(particulate_matter, TF_PARTICULATE_MATTER_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(particulate_matter->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_get_status_led_config(TF_ParticulateMatter *particulate_matter, uint8_t *ret_config) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_get_chip_temperature(TF_ParticulateMatter *particulate_matter, int16_t *ret_temperature) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_reset(TF_ParticulateMatter *particulate_matter) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_particulate_matter_get_response_expected(particulate_matter, TF_PARTICULATE_MATTER_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_write_uid(TF_ParticulateMatter *particulate_matter, uint32_t uid) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_particulate_matter_get_response_expected(particulate_matter, TF_PARTICULATE_MATTER_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(particulate_matter->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_read_uid(TF_ParticulateMatter *particulate_matter, uint32_t *ret_uid) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_particulate_matter_get_identity(TF_ParticulateMatter *particulate_matter, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = particulate_matter->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(particulate_matter->tfp, TF_PARTICULATE_MATTER_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(particulate_matter->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(particulate_matter->tfp);
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
        tf_tfp_packet_processed(particulate_matter->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(particulate_matter->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_particulate_matter_register_pm_concentration_callback(TF_ParticulateMatter *particulate_matter, TF_ParticulateMatter_PMConcentrationHandler handler, void *user_data) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    particulate_matter->pm_concentration_handler = handler;
    particulate_matter->pm_concentration_user_data = user_data;

    return TF_E_OK;
}


int tf_particulate_matter_register_pm_count_callback(TF_ParticulateMatter *particulate_matter, TF_ParticulateMatter_PMCountHandler handler, void *user_data) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    particulate_matter->pm_count_handler = handler;
    particulate_matter->pm_count_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_particulate_matter_callback_tick(TF_ParticulateMatter *particulate_matter, uint32_t timeout_us) {
    if (particulate_matter == NULL) {
        return TF_E_NULL;
    }

    if (particulate_matter->magic != 0x5446 || particulate_matter->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = particulate_matter->tfp->spitfp->hal;

    return tf_tfp_callback_tick(particulate_matter->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
