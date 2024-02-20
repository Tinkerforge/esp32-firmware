/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_thermal_imaging.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_thermal_imaging_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_ThermalImaging *thermal_imaging = (TF_ThermalImaging *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(thermal_imaging->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_THERMAL_IMAGING_CALLBACK_HIGH_CONTRAST_IMAGE_LOW_LEVEL: {
            TF_ThermalImaging_HighContrastImageLowLevelHandler fn = thermal_imaging->high_contrast_image_low_level_handler;
            void *user_data = thermal_imaging->high_contrast_image_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint16_t image_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            uint8_t image_chunk_data[62]; for (_i = 0; _i < 62; ++_i) image_chunk_data[_i] = tf_packet_buffer_read_uint8_t(payload);
            hal_common->locked = true;
            fn(thermal_imaging, image_chunk_offset, image_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_THERMAL_IMAGING_CALLBACK_TEMPERATURE_IMAGE_LOW_LEVEL: {
            TF_ThermalImaging_TemperatureImageLowLevelHandler fn = thermal_imaging->temperature_image_low_level_handler;
            void *user_data = thermal_imaging->temperature_image_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint16_t image_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            uint16_t image_chunk_data[31]; for (_i = 0; _i < 31; ++_i) image_chunk_data[_i] = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(thermal_imaging, image_chunk_offset, image_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_thermal_imaging_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_thermal_imaging_create(TF_ThermalImaging *thermal_imaging, const char *uid_or_port_name, TF_HAL *hal) {
    if (thermal_imaging == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(thermal_imaging, 0, sizeof(TF_ThermalImaging));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_THERMAL_IMAGING_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    thermal_imaging->tfp = tfp;
    thermal_imaging->tfp->device = thermal_imaging;
    thermal_imaging->tfp->cb_handler = tf_thermal_imaging_callback_handler;
    thermal_imaging->magic = 0x5446;
    thermal_imaging->response_expected[0] = 0x08;
    thermal_imaging->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_thermal_imaging_destroy(TF_ThermalImaging *thermal_imaging) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }
    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    thermal_imaging->tfp->cb_handler = NULL;
    thermal_imaging->tfp->device = NULL;
    thermal_imaging->tfp = NULL;
    thermal_imaging->magic = 0;

    return TF_E_OK;
}

int tf_thermal_imaging_get_response_expected(TF_ThermalImaging *thermal_imaging, uint8_t function_id, bool *ret_response_expected) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_THERMAL_IMAGING_FUNCTION_SET_RESOLUTION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (thermal_imaging->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_SPOTMETER_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (thermal_imaging->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_HIGH_CONTRAST_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (thermal_imaging->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_IMAGE_TRANSFER_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (thermal_imaging->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_FLUX_LINEAR_PARAMETERS:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (thermal_imaging->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_FFC_SHUTTER_MODE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (thermal_imaging->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_RUN_FFC_NORMALIZATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (thermal_imaging->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (thermal_imaging->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (thermal_imaging->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (thermal_imaging->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (thermal_imaging->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_thermal_imaging_set_response_expected(TF_ThermalImaging *thermal_imaging, uint8_t function_id, bool response_expected) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_THERMAL_IMAGING_FUNCTION_SET_RESOLUTION:
            if (response_expected) {
                thermal_imaging->response_expected[0] |= (1 << 0);
            } else {
                thermal_imaging->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_SPOTMETER_CONFIG:
            if (response_expected) {
                thermal_imaging->response_expected[0] |= (1 << 1);
            } else {
                thermal_imaging->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_HIGH_CONTRAST_CONFIG:
            if (response_expected) {
                thermal_imaging->response_expected[0] |= (1 << 2);
            } else {
                thermal_imaging->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_IMAGE_TRANSFER_CONFIG:
            if (response_expected) {
                thermal_imaging->response_expected[0] |= (1 << 3);
            } else {
                thermal_imaging->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_FLUX_LINEAR_PARAMETERS:
            if (response_expected) {
                thermal_imaging->response_expected[0] |= (1 << 4);
            } else {
                thermal_imaging->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_FFC_SHUTTER_MODE:
            if (response_expected) {
                thermal_imaging->response_expected[0] |= (1 << 5);
            } else {
                thermal_imaging->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_RUN_FFC_NORMALIZATION:
            if (response_expected) {
                thermal_imaging->response_expected[0] |= (1 << 6);
            } else {
                thermal_imaging->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                thermal_imaging->response_expected[0] |= (1 << 7);
            } else {
                thermal_imaging->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                thermal_imaging->response_expected[1] |= (1 << 0);
            } else {
                thermal_imaging->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_RESET:
            if (response_expected) {
                thermal_imaging->response_expected[1] |= (1 << 1);
            } else {
                thermal_imaging->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_THERMAL_IMAGING_FUNCTION_WRITE_UID:
            if (response_expected) {
                thermal_imaging->response_expected[1] |= (1 << 2);
            } else {
                thermal_imaging->response_expected[1] &= ~(1 << 2);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_thermal_imaging_set_response_expected_all(TF_ThermalImaging *thermal_imaging, bool response_expected) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(thermal_imaging->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_thermal_imaging_get_high_contrast_image_low_level(TF_ThermalImaging *thermal_imaging, uint16_t *ret_image_chunk_offset, uint8_t ret_image_chunk_data[62]) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_HIGH_CONTRAST_IMAGE_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_image_chunk_offset != NULL) { *ret_image_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_image_chunk_data != NULL) { for (_i = 0; _i < 62; ++_i) ret_image_chunk_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 62); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_temperature_image_low_level(TF_ThermalImaging *thermal_imaging, uint16_t *ret_image_chunk_offset, uint16_t ret_image_chunk_data[31]) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_TEMPERATURE_IMAGE_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_image_chunk_offset != NULL) { *ret_image_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_image_chunk_data != NULL) { for (_i = 0; _i < 31; ++_i) ret_image_chunk_data[_i] = tf_packet_buffer_read_uint16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 62); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_statistics(TF_ThermalImaging *thermal_imaging, uint16_t ret_spotmeter_statistics[4], uint16_t ret_temperatures[4], uint8_t *ret_resolution, uint8_t *ret_ffc_status, bool ret_temperature_warning[2]) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_STATISTICS, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 19) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_spotmeter_statistics != NULL) { for (_i = 0; _i < 4; ++_i) ret_spotmeter_statistics[_i] = tf_packet_buffer_read_uint16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 8); }
            if (ret_temperatures != NULL) { for (_i = 0; _i < 4; ++_i) ret_temperatures[_i] = tf_packet_buffer_read_uint16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 8); }
            if (ret_resolution != NULL) { *ret_resolution = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_ffc_status != NULL) { *ret_ffc_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_temperature_warning != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_temperature_warning, 2);} else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 19) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_set_resolution(TF_ThermalImaging *thermal_imaging, uint8_t resolution) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_thermal_imaging_get_response_expected(thermal_imaging, TF_THERMAL_IMAGING_FUNCTION_SET_RESOLUTION, &_response_expected);
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_SET_RESOLUTION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(thermal_imaging->tfp);

    _send_buf[0] = (uint8_t)resolution;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_resolution(TF_ThermalImaging *thermal_imaging, uint8_t *ret_resolution) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_RESOLUTION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_resolution != NULL) { *ret_resolution = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_set_spotmeter_config(TF_ThermalImaging *thermal_imaging, const uint8_t region_of_interest[4]) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_thermal_imaging_get_response_expected(thermal_imaging, TF_THERMAL_IMAGING_FUNCTION_SET_SPOTMETER_CONFIG, &_response_expected);
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_SET_SPOTMETER_CONFIG, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(thermal_imaging->tfp);

    memcpy(_send_buf + 0, region_of_interest, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_spotmeter_config(TF_ThermalImaging *thermal_imaging, uint8_t ret_region_of_interest[4]) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_SPOTMETER_CONFIG, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_region_of_interest != NULL) { for (_i = 0; _i < 4; ++_i) ret_region_of_interest[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_set_high_contrast_config(TF_ThermalImaging *thermal_imaging, const uint8_t region_of_interest[4], uint16_t dampening_factor, const uint16_t clip_limit[2], uint16_t empty_counts) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_thermal_imaging_get_response_expected(thermal_imaging, TF_THERMAL_IMAGING_FUNCTION_SET_HIGH_CONTRAST_CONFIG, &_response_expected);
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_SET_HIGH_CONTRAST_CONFIG, 12, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(thermal_imaging->tfp);

    memcpy(_send_buf + 0, region_of_interest, 4);
    dampening_factor = tf_leconvert_uint16_to(dampening_factor); memcpy(_send_buf + 4, &dampening_factor, 2);
    for (_i = 0; _i < 2; _i++) { uint16_t tmp_clip_limit = tf_leconvert_uint16_to(clip_limit[_i]); memcpy(_send_buf + 6 + (_i * sizeof(uint16_t)), &tmp_clip_limit, sizeof(uint16_t)); }
    empty_counts = tf_leconvert_uint16_to(empty_counts); memcpy(_send_buf + 10, &empty_counts, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_high_contrast_config(TF_ThermalImaging *thermal_imaging, uint8_t ret_region_of_interest[4], uint16_t *ret_dampening_factor, uint16_t ret_clip_limit[2], uint16_t *ret_empty_counts) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_HIGH_CONTRAST_CONFIG, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 12) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_region_of_interest != NULL) { for (_i = 0; _i < 4; ++_i) ret_region_of_interest[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_dampening_factor != NULL) { *ret_dampening_factor = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_clip_limit != NULL) { for (_i = 0; _i < 2; ++_i) ret_clip_limit[_i] = tf_packet_buffer_read_uint16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_empty_counts != NULL) { *ret_empty_counts = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 12) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_set_image_transfer_config(TF_ThermalImaging *thermal_imaging, uint8_t config) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_thermal_imaging_get_response_expected(thermal_imaging, TF_THERMAL_IMAGING_FUNCTION_SET_IMAGE_TRANSFER_CONFIG, &_response_expected);
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_SET_IMAGE_TRANSFER_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(thermal_imaging->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_image_transfer_config(TF_ThermalImaging *thermal_imaging, uint8_t *ret_config) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_IMAGE_TRANSFER_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_set_flux_linear_parameters(TF_ThermalImaging *thermal_imaging, uint16_t scene_emissivity, uint16_t temperature_background, uint16_t tau_window, uint16_t temperatur_window, uint16_t tau_atmosphere, uint16_t temperature_atmosphere, uint16_t reflection_window, uint16_t temperature_reflection) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_thermal_imaging_get_response_expected(thermal_imaging, TF_THERMAL_IMAGING_FUNCTION_SET_FLUX_LINEAR_PARAMETERS, &_response_expected);
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_SET_FLUX_LINEAR_PARAMETERS, 16, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(thermal_imaging->tfp);

    scene_emissivity = tf_leconvert_uint16_to(scene_emissivity); memcpy(_send_buf + 0, &scene_emissivity, 2);
    temperature_background = tf_leconvert_uint16_to(temperature_background); memcpy(_send_buf + 2, &temperature_background, 2);
    tau_window = tf_leconvert_uint16_to(tau_window); memcpy(_send_buf + 4, &tau_window, 2);
    temperatur_window = tf_leconvert_uint16_to(temperatur_window); memcpy(_send_buf + 6, &temperatur_window, 2);
    tau_atmosphere = tf_leconvert_uint16_to(tau_atmosphere); memcpy(_send_buf + 8, &tau_atmosphere, 2);
    temperature_atmosphere = tf_leconvert_uint16_to(temperature_atmosphere); memcpy(_send_buf + 10, &temperature_atmosphere, 2);
    reflection_window = tf_leconvert_uint16_to(reflection_window); memcpy(_send_buf + 12, &reflection_window, 2);
    temperature_reflection = tf_leconvert_uint16_to(temperature_reflection); memcpy(_send_buf + 14, &temperature_reflection, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_flux_linear_parameters(TF_ThermalImaging *thermal_imaging, uint16_t *ret_scene_emissivity, uint16_t *ret_temperature_background, uint16_t *ret_tau_window, uint16_t *ret_temperatur_window, uint16_t *ret_tau_atmosphere, uint16_t *ret_temperature_atmosphere, uint16_t *ret_reflection_window, uint16_t *ret_temperature_reflection) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_FLUX_LINEAR_PARAMETERS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_scene_emissivity != NULL) { *ret_scene_emissivity = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_temperature_background != NULL) { *ret_temperature_background = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_tau_window != NULL) { *ret_tau_window = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_temperatur_window != NULL) { *ret_temperatur_window = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_tau_atmosphere != NULL) { *ret_tau_atmosphere = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_temperature_atmosphere != NULL) { *ret_temperature_atmosphere = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_reflection_window != NULL) { *ret_reflection_window = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_temperature_reflection != NULL) { *ret_temperature_reflection = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_set_ffc_shutter_mode(TF_ThermalImaging *thermal_imaging, uint8_t shutter_mode, uint8_t temp_lockout_state, bool video_freeze_during_ffc, bool ffc_desired, uint32_t elapsed_time_since_last_ffc, uint32_t desired_ffc_period, bool explicit_cmd_to_open, uint16_t desired_ffc_temp_delta, uint16_t imminent_delay) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_thermal_imaging_get_response_expected(thermal_imaging, TF_THERMAL_IMAGING_FUNCTION_SET_FFC_SHUTTER_MODE, &_response_expected);
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_SET_FFC_SHUTTER_MODE, 17, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(thermal_imaging->tfp);

    _send_buf[0] = (uint8_t)shutter_mode;
    _send_buf[1] = (uint8_t)temp_lockout_state;
    _send_buf[2] = video_freeze_during_ffc ? 1 : 0;
    _send_buf[3] = ffc_desired ? 1 : 0;
    elapsed_time_since_last_ffc = tf_leconvert_uint32_to(elapsed_time_since_last_ffc); memcpy(_send_buf + 4, &elapsed_time_since_last_ffc, 4);
    desired_ffc_period = tf_leconvert_uint32_to(desired_ffc_period); memcpy(_send_buf + 8, &desired_ffc_period, 4);
    _send_buf[12] = explicit_cmd_to_open ? 1 : 0;
    desired_ffc_temp_delta = tf_leconvert_uint16_to(desired_ffc_temp_delta); memcpy(_send_buf + 13, &desired_ffc_temp_delta, 2);
    imminent_delay = tf_leconvert_uint16_to(imminent_delay); memcpy(_send_buf + 15, &imminent_delay, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_ffc_shutter_mode(TF_ThermalImaging *thermal_imaging, uint8_t *ret_shutter_mode, uint8_t *ret_temp_lockout_state, bool *ret_video_freeze_during_ffc, bool *ret_ffc_desired, uint32_t *ret_elapsed_time_since_last_ffc, uint32_t *ret_desired_ffc_period, bool *ret_explicit_cmd_to_open, uint16_t *ret_desired_ffc_temp_delta, uint16_t *ret_imminent_delay) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_FFC_SHUTTER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 17) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_shutter_mode != NULL) { *ret_shutter_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_temp_lockout_state != NULL) { *ret_temp_lockout_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_video_freeze_during_ffc != NULL) { *ret_video_freeze_during_ffc = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_ffc_desired != NULL) { *ret_ffc_desired = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_elapsed_time_since_last_ffc != NULL) { *ret_elapsed_time_since_last_ffc = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_desired_ffc_period != NULL) { *ret_desired_ffc_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_explicit_cmd_to_open != NULL) { *ret_explicit_cmd_to_open = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_desired_ffc_temp_delta != NULL) { *ret_desired_ffc_temp_delta = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_imminent_delay != NULL) { *ret_imminent_delay = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 17) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_run_ffc_normalization(TF_ThermalImaging *thermal_imaging) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_thermal_imaging_get_response_expected(thermal_imaging, TF_THERMAL_IMAGING_FUNCTION_RUN_FFC_NORMALIZATION, &_response_expected);
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_RUN_FFC_NORMALIZATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_spitfp_error_count(TF_ThermalImaging *thermal_imaging, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_set_bootloader_mode(TF_ThermalImaging *thermal_imaging, uint8_t mode, uint8_t *ret_status) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(thermal_imaging->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_bootloader_mode(TF_ThermalImaging *thermal_imaging, uint8_t *ret_mode) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_set_write_firmware_pointer(TF_ThermalImaging *thermal_imaging, uint32_t pointer) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_thermal_imaging_get_response_expected(thermal_imaging, TF_THERMAL_IMAGING_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(thermal_imaging->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_write_firmware(TF_ThermalImaging *thermal_imaging, const uint8_t data[64], uint8_t *ret_status) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(thermal_imaging->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_set_status_led_config(TF_ThermalImaging *thermal_imaging, uint8_t config) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_thermal_imaging_get_response_expected(thermal_imaging, TF_THERMAL_IMAGING_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(thermal_imaging->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_status_led_config(TF_ThermalImaging *thermal_imaging, uint8_t *ret_config) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_chip_temperature(TF_ThermalImaging *thermal_imaging, int16_t *ret_temperature) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_reset(TF_ThermalImaging *thermal_imaging) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_thermal_imaging_get_response_expected(thermal_imaging, TF_THERMAL_IMAGING_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_write_uid(TF_ThermalImaging *thermal_imaging, uint32_t uid) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_thermal_imaging_get_response_expected(thermal_imaging, TF_THERMAL_IMAGING_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(thermal_imaging->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_read_uid(TF_ThermalImaging *thermal_imaging, uint32_t *ret_uid) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_thermal_imaging_get_identity(TF_ThermalImaging *thermal_imaging, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = thermal_imaging->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(thermal_imaging->tfp, TF_THERMAL_IMAGING_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(thermal_imaging->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(thermal_imaging->tfp);
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
        tf_tfp_packet_processed(thermal_imaging->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(thermal_imaging->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

static int tf_thermal_imaging_get_high_contrast_image_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    (void)wrapper_data;
    uint16_t image_length = 4800;
    uint16_t image_chunk_offset = 0;
    uint8_t *image_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_thermal_imaging_get_high_contrast_image_low_level((TF_ThermalImaging *)device, &image_chunk_offset, image_chunk_data);

    if (image_chunk_offset == (1 << 16) - 1) { // maximum chunk offset -> stream has no data
        return TF_E_INTERNAL_STREAM_HAS_NO_DATA;
    }

    *ret_stream_length = (uint32_t)image_length;
    *ret_chunk_offset = (uint32_t)image_chunk_offset;
    return ret;
}

int tf_thermal_imaging_get_high_contrast_image(TF_ThermalImaging *thermal_imaging, uint8_t *ret_image, uint16_t *ret_image_length) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    uint32_t _image_length = 0;
    uint8_t _image_chunk_data[62];

    int ret = tf_stream_out(thermal_imaging, tf_thermal_imaging_get_high_contrast_image_ll_wrapper, NULL, ret_image, &_image_length, _image_chunk_data, 62, tf_copy_items_uint8_t);

    if (ret_image_length != NULL) {
        *ret_image_length = (uint16_t)_image_length;
    }
    return ret;
}

static int tf_thermal_imaging_get_temperature_image_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    (void)wrapper_data;
    uint16_t image_length = 4800;
    uint16_t image_chunk_offset = 0;
    uint16_t *image_chunk_data = (uint16_t *) chunk_data;
    int ret = tf_thermal_imaging_get_temperature_image_low_level((TF_ThermalImaging *)device, &image_chunk_offset, image_chunk_data);

    if (image_chunk_offset == (1 << 16) - 1) { // maximum chunk offset -> stream has no data
        return TF_E_INTERNAL_STREAM_HAS_NO_DATA;
    }

    *ret_stream_length = (uint32_t)image_length;
    *ret_chunk_offset = (uint32_t)image_chunk_offset;
    return ret;
}

int tf_thermal_imaging_get_temperature_image(TF_ThermalImaging *thermal_imaging, uint16_t *ret_image, uint16_t *ret_image_length) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    uint32_t _image_length = 0;
    uint16_t _image_chunk_data[31];

    int ret = tf_stream_out(thermal_imaging, tf_thermal_imaging_get_temperature_image_ll_wrapper, NULL, ret_image, &_image_length, _image_chunk_data, 31, tf_copy_items_uint16_t);

    if (ret_image_length != NULL) {
        *ret_image_length = (uint16_t)_image_length;
    }
    return ret;
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_thermal_imaging_register_high_contrast_image_low_level_callback(TF_ThermalImaging *thermal_imaging, TF_ThermalImaging_HighContrastImageLowLevelHandler handler, void *user_data) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    thermal_imaging->high_contrast_image_low_level_handler = handler;
    thermal_imaging->high_contrast_image_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_thermal_imaging_high_contrast_image_wrapper(TF_ThermalImaging *thermal_imaging, uint16_t image_chunk_offset, uint8_t image_chunk_data[62], void *user_data) {
    uint16_t image_length = 4800;
    uint32_t stream_length = (uint32_t) image_length;
    uint32_t chunk_offset = (uint32_t) image_chunk_offset;
    if (!tf_stream_out_callback(&thermal_imaging->high_contrast_image_hlc, stream_length, chunk_offset, image_chunk_data, 62, tf_copy_items_uint8_t)) {
        return;
    }

    // Stream is either complete or out of sync
    uint8_t *image = (uint8_t *) (thermal_imaging->high_contrast_image_hlc.length == 0 ? NULL : thermal_imaging->high_contrast_image_hlc.data);
    thermal_imaging->high_contrast_image_handler(thermal_imaging, image, image_length, user_data);

    thermal_imaging->high_contrast_image_hlc.stream_in_progress = false;
    thermal_imaging->high_contrast_image_hlc.length = 0;
}

int tf_thermal_imaging_register_high_contrast_image_callback(TF_ThermalImaging *thermal_imaging, TF_ThermalImaging_HighContrastImageHandler handler, uint8_t *image_buffer, void *user_data) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    thermal_imaging->high_contrast_image_handler = handler;

    thermal_imaging->high_contrast_image_hlc.data = image_buffer;
    thermal_imaging->high_contrast_image_hlc.length = 0;
    thermal_imaging->high_contrast_image_hlc.stream_in_progress = false;

    return tf_thermal_imaging_register_high_contrast_image_low_level_callback(thermal_imaging, handler == NULL ? NULL : tf_thermal_imaging_high_contrast_image_wrapper, user_data);
}


int tf_thermal_imaging_register_temperature_image_low_level_callback(TF_ThermalImaging *thermal_imaging, TF_ThermalImaging_TemperatureImageLowLevelHandler handler, void *user_data) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    thermal_imaging->temperature_image_low_level_handler = handler;
    thermal_imaging->temperature_image_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_thermal_imaging_temperature_image_wrapper(TF_ThermalImaging *thermal_imaging, uint16_t image_chunk_offset, uint16_t image_chunk_data[31], void *user_data) {
    uint16_t image_length = 4800;
    uint32_t stream_length = (uint32_t) image_length;
    uint32_t chunk_offset = (uint32_t) image_chunk_offset;
    if (!tf_stream_out_callback(&thermal_imaging->temperature_image_hlc, stream_length, chunk_offset, image_chunk_data, 31, tf_copy_items_uint16_t)) {
        return;
    }

    // Stream is either complete or out of sync
    uint16_t *image = (uint16_t *) (thermal_imaging->temperature_image_hlc.length == 0 ? NULL : thermal_imaging->temperature_image_hlc.data);
    thermal_imaging->temperature_image_handler(thermal_imaging, image, image_length, user_data);

    thermal_imaging->temperature_image_hlc.stream_in_progress = false;
    thermal_imaging->temperature_image_hlc.length = 0;
}

int tf_thermal_imaging_register_temperature_image_callback(TF_ThermalImaging *thermal_imaging, TF_ThermalImaging_TemperatureImageHandler handler, uint16_t *image_buffer, void *user_data) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    thermal_imaging->temperature_image_handler = handler;

    thermal_imaging->temperature_image_hlc.data = image_buffer;
    thermal_imaging->temperature_image_hlc.length = 0;
    thermal_imaging->temperature_image_hlc.stream_in_progress = false;

    return tf_thermal_imaging_register_temperature_image_low_level_callback(thermal_imaging, handler == NULL ? NULL : tf_thermal_imaging_temperature_image_wrapper, user_data);
}
#endif
int tf_thermal_imaging_callback_tick(TF_ThermalImaging *thermal_imaging, uint32_t timeout_us) {
    if (thermal_imaging == NULL) {
        return TF_E_NULL;
    }

    if (thermal_imaging->magic != 0x5446 || thermal_imaging->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = thermal_imaging->tfp->spitfp->hal;

    return tf_tfp_callback_tick(thermal_imaging->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
