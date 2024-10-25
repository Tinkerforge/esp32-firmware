/* ***********************************************************
 * This file was automatically generated on 2024-10-25.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_warp_energy_manager_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_warp_energy_manager_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_WARPEnergyManagerV2 *warp_energy_manager_v2 = (TF_WARPEnergyManagerV2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(warp_energy_manager_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_WARP_ENERGY_MANAGER_V2_CALLBACK_SD_WALLBOX_DATA_POINTS_LOW_LEVEL: {
            TF_WARPEnergyManagerV2_SDWallboxDataPointsLowLevelHandler fn = warp_energy_manager_v2->sd_wallbox_data_points_low_level_handler;
            void *user_data = warp_energy_manager_v2->sd_wallbox_data_points_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint16_t data_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t data_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            uint8_t data_chunk_data[60]; for (_i = 0; _i < 60; ++_i) data_chunk_data[_i] = tf_packet_buffer_read_uint8_t(payload);
            hal_common->locked = true;
            fn(warp_energy_manager_v2, data_length, data_chunk_offset, data_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_WARP_ENERGY_MANAGER_V2_CALLBACK_SD_WALLBOX_DAILY_DATA_POINTS_LOW_LEVEL: {
            TF_WARPEnergyManagerV2_SDWallboxDailyDataPointsLowLevelHandler fn = warp_energy_manager_v2->sd_wallbox_daily_data_points_low_level_handler;
            void *user_data = warp_energy_manager_v2->sd_wallbox_daily_data_points_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint16_t data_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t data_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            uint32_t data_chunk_data[15]; for (_i = 0; _i < 15; ++_i) data_chunk_data[_i] = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(warp_energy_manager_v2, data_length, data_chunk_offset, data_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_WARP_ENERGY_MANAGER_V2_CALLBACK_SD_ENERGY_MANAGER_DATA_POINTS_LOW_LEVEL: {
            TF_WARPEnergyManagerV2_SDEnergyManagerDataPointsLowLevelHandler fn = warp_energy_manager_v2->sd_energy_manager_data_points_low_level_handler;
            void *user_data = warp_energy_manager_v2->sd_energy_manager_data_points_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint16_t data_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t data_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            uint8_t data_chunk_data[34]; for (_i = 0; _i < 34; ++_i) data_chunk_data[_i] = tf_packet_buffer_read_uint8_t(payload);
            hal_common->locked = true;
            fn(warp_energy_manager_v2, data_length, data_chunk_offset, data_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_WARP_ENERGY_MANAGER_V2_CALLBACK_SD_ENERGY_MANAGER_DAILY_DATA_POINTS_LOW_LEVEL: {
            TF_WARPEnergyManagerV2_SDEnergyManagerDailyDataPointsLowLevelHandler fn = warp_energy_manager_v2->sd_energy_manager_daily_data_points_low_level_handler;
            void *user_data = warp_energy_manager_v2->sd_energy_manager_daily_data_points_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            uint16_t data_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t data_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            uint32_t data_chunk_data[15]; for (_i = 0; _i < 15; ++_i) data_chunk_data[_i] = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(warp_energy_manager_v2, data_length, data_chunk_offset, data_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_warp_energy_manager_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_warp_energy_manager_v2_create(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (warp_energy_manager_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(warp_energy_manager_v2, 0, sizeof(TF_WARPEnergyManagerV2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_WARP_ENERGY_MANAGER_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    warp_energy_manager_v2->tfp = tfp;
    warp_energy_manager_v2->tfp->device = warp_energy_manager_v2;
    warp_energy_manager_v2->tfp->cb_handler = tf_warp_energy_manager_v2_callback_handler;
    warp_energy_manager_v2->magic = 0x5446;
    warp_energy_manager_v2->response_expected[0] = 0x00;
    warp_energy_manager_v2->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_warp_energy_manager_v2_destroy(TF_WARPEnergyManagerV2 *warp_energy_manager_v2) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }
    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_energy_manager_v2->tfp->cb_handler = NULL;
    warp_energy_manager_v2->tfp->device = NULL;
    warp_energy_manager_v2->tfp = NULL;
    warp_energy_manager_v2->magic = 0;

    return TF_E_OK;
}

int tf_warp_energy_manager_v2_get_response_expected(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t function_id, bool *ret_response_expected) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_SG_READY_OUTPUT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_RELAY_OUTPUT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_DATE_TIME:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_DATA_STORAGE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_warp_energy_manager_v2_set_response_expected(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t function_id, bool response_expected) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_SG_READY_OUTPUT:
            if (response_expected) {
                warp_energy_manager_v2->response_expected[0] |= (1 << 0);
            } else {
                warp_energy_manager_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_RELAY_OUTPUT:
            if (response_expected) {
                warp_energy_manager_v2->response_expected[0] |= (1 << 1);
            } else {
                warp_energy_manager_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_DATE_TIME:
            if (response_expected) {
                warp_energy_manager_v2->response_expected[0] |= (1 << 2);
            } else {
                warp_energy_manager_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_DATA_STORAGE:
            if (response_expected) {
                warp_energy_manager_v2->response_expected[0] |= (1 << 3);
            } else {
                warp_energy_manager_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY:
            if (response_expected) {
                warp_energy_manager_v2->response_expected[0] |= (1 << 4);
            } else {
                warp_energy_manager_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                warp_energy_manager_v2->response_expected[0] |= (1 << 5);
            } else {
                warp_energy_manager_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                warp_energy_manager_v2->response_expected[0] |= (1 << 6);
            } else {
                warp_energy_manager_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_RESET:
            if (response_expected) {
                warp_energy_manager_v2->response_expected[0] |= (1 << 7);
            } else {
                warp_energy_manager_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                warp_energy_manager_v2->response_expected[1] |= (1 << 0);
            } else {
                warp_energy_manager_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_warp_energy_manager_v2_set_response_expected_all(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, bool response_expected) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(warp_energy_manager_v2->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_warp_energy_manager_v2_get_energy_meter_values(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, float *ret_power, float ret_current[3]) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_ENERGY_METER_VALUES, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_power != NULL) { *ret_power = tf_packet_buffer_read_float(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_current != NULL) { for (_i = 0; _i < 3; ++_i) ret_current[_i] = tf_packet_buffer_read_float(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 12); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_energy_meter_detailed_values_low_level(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint16_t *ret_values_chunk_offset, float ret_values_chunk_data[15]) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_ENERGY_METER_DETAILED_VALUES_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 62) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_values_chunk_offset != NULL) { *ret_values_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_values_chunk_data != NULL) { for (_i = 0; _i < 15; ++_i) ret_values_chunk_data[_i] = tf_packet_buffer_read_float(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 62) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_energy_meter_state(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t *ret_energy_meter_type, uint32_t ret_error_count[6]) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_ENERGY_METER_STATE, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 25) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_energy_meter_type != NULL) { *ret_energy_meter_type = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_error_count != NULL) { for (_i = 0; _i < 6; ++_i) ret_error_count[_i] = tf_packet_buffer_read_uint32_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 24); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_input(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, bool ret_input[4]) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_INPUT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_input != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_input, 4);} else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_set_sg_ready_output(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t index, bool output) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_v2_get_response_expected(warp_energy_manager_v2, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_SG_READY_OUTPUT, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_SG_READY_OUTPUT, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    _send_buf[0] = (uint8_t)index;
    _send_buf[1] = output ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_sg_ready_output(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, bool ret_output[2]) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_SG_READY_OUTPUT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_output != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_output, 2);} else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_set_relay_output(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t index, bool output) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_v2_get_response_expected(warp_energy_manager_v2, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_RELAY_OUTPUT, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_RELAY_OUTPUT, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    _send_buf[0] = (uint8_t)index;
    _send_buf[1] = output ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_relay_output(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, bool ret_output[2]) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_RELAY_OUTPUT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_output != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_output, 2);} else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_input_voltage(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint16_t *ret_voltage) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_INPUT_VOLTAGE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_voltage != NULL) { *ret_voltage = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_uptime(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint32_t *ret_uptime) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_UPTIME, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uptime != NULL) { *ret_uptime = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_all_data_1(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, float *ret_power, float ret_current[3], uint8_t *ret_energy_meter_type, uint32_t ret_error_count[6], bool ret_input[4], bool ret_output_sg_ready[2], bool ret_output_relay[2], uint16_t *ret_voltage, uint32_t *ret_uptime) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_ALL_DATA_1, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 50) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_power != NULL) { *ret_power = tf_packet_buffer_read_float(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_current != NULL) { for (_i = 0; _i < 3; ++_i) ret_current[_i] = tf_packet_buffer_read_float(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 12); }
            if (ret_energy_meter_type != NULL) { *ret_energy_meter_type = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_error_count != NULL) { for (_i = 0; _i < 6; ++_i) ret_error_count[_i] = tf_packet_buffer_read_uint32_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 24); }
            if (ret_input != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_input, 4);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_output_sg_ready != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_output_sg_ready, 2);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_output_relay != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_output_relay, 2);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_voltage != NULL) { *ret_voltage = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_uptime != NULL) { *ret_uptime = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 50) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_sd_information(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint32_t *ret_sd_status, uint32_t *ret_lfs_status, uint16_t *ret_sector_size, uint32_t *ret_sector_count, uint32_t *ret_card_type, uint8_t *ret_product_rev, char ret_product_name[5], uint8_t *ret_manufacturer_id) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_SD_INFORMATION, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
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
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_set_sd_wallbox_data_point(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t flags, uint16_t power, uint8_t *ret_status) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_SD_WALLBOX_DATA_POINT, 13, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    wallbox_id = tf_leconvert_uint32_to(wallbox_id); memcpy(_send_buf + 0, &wallbox_id, 4);
    _send_buf[4] = (uint8_t)year;
    _send_buf[5] = (uint8_t)month;
    _send_buf[6] = (uint8_t)day;
    _send_buf[7] = (uint8_t)hour;
    _send_buf[8] = (uint8_t)minute;
    flags = tf_leconvert_uint16_to(flags); memcpy(_send_buf + 9, &flags, 2);
    power = tf_leconvert_uint16_to(power); memcpy(_send_buf + 11, &power, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_sd_wallbox_data_points(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_SD_WALLBOX_DATA_POINTS, 11, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    wallbox_id = tf_leconvert_uint32_to(wallbox_id); memcpy(_send_buf + 0, &wallbox_id, 4);
    _send_buf[4] = (uint8_t)year;
    _send_buf[5] = (uint8_t)month;
    _send_buf[6] = (uint8_t)day;
    _send_buf[7] = (uint8_t)hour;
    _send_buf[8] = (uint8_t)minute;
    amount = tf_leconvert_uint16_to(amount); memcpy(_send_buf + 9, &amount, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_set_sd_wallbox_daily_data_point(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint32_t energy, uint8_t *ret_status) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_SD_WALLBOX_DAILY_DATA_POINT, 11, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    wallbox_id = tf_leconvert_uint32_to(wallbox_id); memcpy(_send_buf + 0, &wallbox_id, 4);
    _send_buf[4] = (uint8_t)year;
    _send_buf[5] = (uint8_t)month;
    _send_buf[6] = (uint8_t)day;
    energy = tf_leconvert_uint32_to(energy); memcpy(_send_buf + 7, &energy, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_sd_wallbox_daily_data_points(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint32_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_SD_WALLBOX_DAILY_DATA_POINTS, 8, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    wallbox_id = tf_leconvert_uint32_to(wallbox_id); memcpy(_send_buf + 0, &wallbox_id, 4);
    _send_buf[4] = (uint8_t)year;
    _send_buf[5] = (uint8_t)month;
    _send_buf[6] = (uint8_t)day;
    _send_buf[7] = (uint8_t)amount;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_set_sd_energy_manager_data_point(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t flags, int32_t power_grid, const int32_t power_general[6], uint32_t price, uint8_t *ret_status) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_SD_ENERGY_MANAGER_DATA_POINT, 39, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    _send_buf[0] = (uint8_t)year;
    _send_buf[1] = (uint8_t)month;
    _send_buf[2] = (uint8_t)day;
    _send_buf[3] = (uint8_t)hour;
    _send_buf[4] = (uint8_t)minute;
    flags = tf_leconvert_uint16_to(flags); memcpy(_send_buf + 5, &flags, 2);
    power_grid = tf_leconvert_int32_to(power_grid); memcpy(_send_buf + 7, &power_grid, 4);
    for (_i = 0; _i < 6; _i++) { int32_t tmp_power_general = tf_leconvert_int32_to(power_general[_i]); memcpy(_send_buf + 11 + (_i * sizeof(int32_t)), &tmp_power_general, sizeof(int32_t)); }
    price = tf_leconvert_uint32_to(price); memcpy(_send_buf + 35, &price, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_sd_energy_manager_data_points(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t amount, uint8_t *ret_status) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_SD_ENERGY_MANAGER_DATA_POINTS, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    _send_buf[0] = (uint8_t)year;
    _send_buf[1] = (uint8_t)month;
    _send_buf[2] = (uint8_t)day;
    _send_buf[3] = (uint8_t)hour;
    _send_buf[4] = (uint8_t)minute;
    amount = tf_leconvert_uint16_to(amount); memcpy(_send_buf + 5, &amount, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_set_sd_energy_manager_daily_data_point(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t year, uint8_t month, uint8_t day, uint32_t energy_grid_in, uint32_t energy_grid_out, const uint32_t energy_general_in[6], const uint32_t energy_general_out[6], uint32_t price, uint8_t *ret_status) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_SD_ENERGY_MANAGER_DAILY_DATA_POINT, 63, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    _send_buf[0] = (uint8_t)year;
    _send_buf[1] = (uint8_t)month;
    _send_buf[2] = (uint8_t)day;
    energy_grid_in = tf_leconvert_uint32_to(energy_grid_in); memcpy(_send_buf + 3, &energy_grid_in, 4);
    energy_grid_out = tf_leconvert_uint32_to(energy_grid_out); memcpy(_send_buf + 7, &energy_grid_out, 4);
    for (_i = 0; _i < 6; _i++) { uint32_t tmp_energy_general_in = tf_leconvert_uint32_to(energy_general_in[_i]); memcpy(_send_buf + 11 + (_i * sizeof(uint32_t)), &tmp_energy_general_in, sizeof(uint32_t)); }
    for (_i = 0; _i < 6; _i++) { uint32_t tmp_energy_general_out = tf_leconvert_uint32_to(energy_general_out[_i]); memcpy(_send_buf + 35 + (_i * sizeof(uint32_t)), &tmp_energy_general_out, sizeof(uint32_t)); }
    price = tf_leconvert_uint32_to(price); memcpy(_send_buf + 59, &price, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_sd_energy_manager_daily_data_points(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t year, uint8_t month, uint8_t day, uint8_t amount, uint8_t *ret_status) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_SD_ENERGY_MANAGER_DAILY_DATA_POINTS, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    _send_buf[0] = (uint8_t)year;
    _send_buf[1] = (uint8_t)month;
    _send_buf[2] = (uint8_t)day;
    _send_buf[3] = (uint8_t)amount;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_format_sd(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint32_t password, uint8_t *ret_format_status) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_FORMAT_SD, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    password = tf_leconvert_uint32_to(password); memcpy(_send_buf + 0, &password, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_format_status != NULL) { *ret_format_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_set_date_time(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t days, uint8_t days_of_week, uint8_t month, uint16_t year) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_v2_get_response_expected(warp_energy_manager_v2, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_DATE_TIME, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_DATE_TIME, 8, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

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
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_date_time(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t *ret_seconds, uint8_t *ret_minutes, uint8_t *ret_hours, uint8_t *ret_days, uint8_t *ret_days_of_week, uint8_t *ret_month, uint16_t *ret_year) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_DATE_TIME, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
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
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_data_storage(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t page, uint8_t *ret_status, uint8_t ret_data[63]) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_DATA_STORAGE, 1, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    _send_buf[0] = (uint8_t)page;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_data != NULL) { for (_i = 0; _i < 63; ++_i) ret_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 63); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_set_data_storage(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t page, const uint8_t data[63]) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_v2_get_response_expected(warp_energy_manager_v2, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_DATA_STORAGE, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_DATA_STORAGE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    _send_buf[0] = (uint8_t)page;
    memcpy(_send_buf + 1, data, 63);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_reset_energy_meter_relative_energy(TF_WARPEnergyManagerV2 *warp_energy_manager_v2) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_v2_get_response_expected(warp_energy_manager_v2, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_spitfp_error_count(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_set_bootloader_mode(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t mode, uint8_t *ret_status) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_bootloader_mode(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t *ret_mode) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_set_write_firmware_pointer(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint32_t pointer) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_v2_get_response_expected(warp_energy_manager_v2, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_write_firmware(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_set_status_led_config(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t config) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_v2_get_response_expected(warp_energy_manager_v2, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_status_led_config(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint8_t *ret_config) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_chip_temperature(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, int16_t *ret_temperature) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_reset(TF_WARPEnergyManagerV2 *warp_energy_manager_v2) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_v2_get_response_expected(warp_energy_manager_v2, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_write_uid(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint32_t uid) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_v2_get_response_expected(warp_energy_manager_v2, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_read_uid(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint32_t *ret_uid) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_v2_get_identity(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager_v2->tfp, TF_WARP_ENERGY_MANAGER_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager_v2->tfp);
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
        tf_tfp_packet_processed(warp_energy_manager_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

static int tf_warp_energy_manager_v2_get_energy_meter_detailed_values_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    (void)wrapper_data;
    uint16_t values_length = 88;
    uint16_t values_chunk_offset = 0;
    float *values_chunk_data = (float *) chunk_data;
    int ret = tf_warp_energy_manager_v2_get_energy_meter_detailed_values_low_level((TF_WARPEnergyManagerV2 *)device, &values_chunk_offset, values_chunk_data);

    if (values_chunk_offset == (1 << 16) - 1) { // maximum chunk offset -> stream has no data
        return TF_E_INTERNAL_STREAM_HAS_NO_DATA;
    }

    *ret_stream_length = (uint32_t)values_length;
    *ret_chunk_offset = (uint32_t)values_chunk_offset;
    return ret;
}

int tf_warp_energy_manager_v2_get_energy_meter_detailed_values(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, float *ret_values, uint16_t *ret_values_length) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    uint32_t _values_length = 0;
    float _values_chunk_data[15];

    int ret = tf_stream_out(warp_energy_manager_v2, tf_warp_energy_manager_v2_get_energy_meter_detailed_values_ll_wrapper, NULL, ret_values, &_values_length, _values_chunk_data, 15, tf_copy_items_float);

    if (ret_values_length != NULL) {
        *ret_values_length = (uint16_t)_values_length;
    }
    return ret;
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_warp_energy_manager_v2_register_sd_wallbox_data_points_low_level_callback(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, TF_WARPEnergyManagerV2_SDWallboxDataPointsLowLevelHandler handler, void *user_data) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_energy_manager_v2->sd_wallbox_data_points_low_level_handler = handler;
    warp_energy_manager_v2->sd_wallbox_data_points_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_warp_energy_manager_v2_sd_wallbox_data_points_wrapper(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint16_t data_length, uint16_t data_chunk_offset, uint8_t data_chunk_data[60], void *user_data) {
    uint32_t stream_length = (uint32_t) data_length;
    uint32_t chunk_offset = (uint32_t) data_chunk_offset;
    if (!tf_stream_out_callback(&warp_energy_manager_v2->sd_wallbox_data_points_hlc, stream_length, chunk_offset, data_chunk_data, 60, tf_copy_items_uint8_t)) {
        return;
    }

    // Stream is either complete or out of sync
    uint8_t *data = (uint8_t *) (warp_energy_manager_v2->sd_wallbox_data_points_hlc.length == 0 ? NULL : warp_energy_manager_v2->sd_wallbox_data_points_hlc.data);
    warp_energy_manager_v2->sd_wallbox_data_points_handler(warp_energy_manager_v2, data, data_length, user_data);

    warp_energy_manager_v2->sd_wallbox_data_points_hlc.stream_in_progress = false;
    warp_energy_manager_v2->sd_wallbox_data_points_hlc.length = 0;
}

int tf_warp_energy_manager_v2_register_sd_wallbox_data_points_callback(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, TF_WARPEnergyManagerV2_SDWallboxDataPointsHandler handler, uint8_t *data_buffer, void *user_data) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_energy_manager_v2->sd_wallbox_data_points_handler = handler;

    warp_energy_manager_v2->sd_wallbox_data_points_hlc.data = data_buffer;
    warp_energy_manager_v2->sd_wallbox_data_points_hlc.length = 0;
    warp_energy_manager_v2->sd_wallbox_data_points_hlc.stream_in_progress = false;

    return tf_warp_energy_manager_v2_register_sd_wallbox_data_points_low_level_callback(warp_energy_manager_v2, handler == NULL ? NULL : tf_warp_energy_manager_v2_sd_wallbox_data_points_wrapper, user_data);
}


int tf_warp_energy_manager_v2_register_sd_wallbox_daily_data_points_low_level_callback(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, TF_WARPEnergyManagerV2_SDWallboxDailyDataPointsLowLevelHandler handler, void *user_data) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_energy_manager_v2->sd_wallbox_daily_data_points_low_level_handler = handler;
    warp_energy_manager_v2->sd_wallbox_daily_data_points_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_warp_energy_manager_v2_sd_wallbox_daily_data_points_wrapper(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint16_t data_length, uint16_t data_chunk_offset, uint32_t data_chunk_data[15], void *user_data) {
    uint32_t stream_length = (uint32_t) data_length;
    uint32_t chunk_offset = (uint32_t) data_chunk_offset;
    if (!tf_stream_out_callback(&warp_energy_manager_v2->sd_wallbox_daily_data_points_hlc, stream_length, chunk_offset, data_chunk_data, 15, tf_copy_items_uint32_t)) {
        return;
    }

    // Stream is either complete or out of sync
    uint32_t *data = (uint32_t *) (warp_energy_manager_v2->sd_wallbox_daily_data_points_hlc.length == 0 ? NULL : warp_energy_manager_v2->sd_wallbox_daily_data_points_hlc.data);
    warp_energy_manager_v2->sd_wallbox_daily_data_points_handler(warp_energy_manager_v2, data, data_length, user_data);

    warp_energy_manager_v2->sd_wallbox_daily_data_points_hlc.stream_in_progress = false;
    warp_energy_manager_v2->sd_wallbox_daily_data_points_hlc.length = 0;
}

int tf_warp_energy_manager_v2_register_sd_wallbox_daily_data_points_callback(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, TF_WARPEnergyManagerV2_SDWallboxDailyDataPointsHandler handler, uint32_t *data_buffer, void *user_data) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_energy_manager_v2->sd_wallbox_daily_data_points_handler = handler;

    warp_energy_manager_v2->sd_wallbox_daily_data_points_hlc.data = data_buffer;
    warp_energy_manager_v2->sd_wallbox_daily_data_points_hlc.length = 0;
    warp_energy_manager_v2->sd_wallbox_daily_data_points_hlc.stream_in_progress = false;

    return tf_warp_energy_manager_v2_register_sd_wallbox_daily_data_points_low_level_callback(warp_energy_manager_v2, handler == NULL ? NULL : tf_warp_energy_manager_v2_sd_wallbox_daily_data_points_wrapper, user_data);
}


int tf_warp_energy_manager_v2_register_sd_energy_manager_data_points_low_level_callback(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, TF_WARPEnergyManagerV2_SDEnergyManagerDataPointsLowLevelHandler handler, void *user_data) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_energy_manager_v2->sd_energy_manager_data_points_low_level_handler = handler;
    warp_energy_manager_v2->sd_energy_manager_data_points_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_warp_energy_manager_v2_sd_energy_manager_data_points_wrapper(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint16_t data_length, uint16_t data_chunk_offset, uint8_t data_chunk_data[34], void *user_data) {
    uint32_t stream_length = (uint32_t) data_length;
    uint32_t chunk_offset = (uint32_t) data_chunk_offset;
    if (!tf_stream_out_callback(&warp_energy_manager_v2->sd_energy_manager_data_points_hlc, stream_length, chunk_offset, data_chunk_data, 34, tf_copy_items_uint8_t)) {
        return;
    }

    // Stream is either complete or out of sync
    uint8_t *data = (uint8_t *) (warp_energy_manager_v2->sd_energy_manager_data_points_hlc.length == 0 ? NULL : warp_energy_manager_v2->sd_energy_manager_data_points_hlc.data);
    warp_energy_manager_v2->sd_energy_manager_data_points_handler(warp_energy_manager_v2, data, data_length, user_data);

    warp_energy_manager_v2->sd_energy_manager_data_points_hlc.stream_in_progress = false;
    warp_energy_manager_v2->sd_energy_manager_data_points_hlc.length = 0;
}

int tf_warp_energy_manager_v2_register_sd_energy_manager_data_points_callback(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, TF_WARPEnergyManagerV2_SDEnergyManagerDataPointsHandler handler, uint8_t *data_buffer, void *user_data) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_energy_manager_v2->sd_energy_manager_data_points_handler = handler;

    warp_energy_manager_v2->sd_energy_manager_data_points_hlc.data = data_buffer;
    warp_energy_manager_v2->sd_energy_manager_data_points_hlc.length = 0;
    warp_energy_manager_v2->sd_energy_manager_data_points_hlc.stream_in_progress = false;

    return tf_warp_energy_manager_v2_register_sd_energy_manager_data_points_low_level_callback(warp_energy_manager_v2, handler == NULL ? NULL : tf_warp_energy_manager_v2_sd_energy_manager_data_points_wrapper, user_data);
}


int tf_warp_energy_manager_v2_register_sd_energy_manager_daily_data_points_low_level_callback(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, TF_WARPEnergyManagerV2_SDEnergyManagerDailyDataPointsLowLevelHandler handler, void *user_data) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_energy_manager_v2->sd_energy_manager_daily_data_points_low_level_handler = handler;
    warp_energy_manager_v2->sd_energy_manager_daily_data_points_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_warp_energy_manager_v2_sd_energy_manager_daily_data_points_wrapper(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint16_t data_length, uint16_t data_chunk_offset, uint32_t data_chunk_data[15], void *user_data) {
    uint32_t stream_length = (uint32_t) data_length;
    uint32_t chunk_offset = (uint32_t) data_chunk_offset;
    if (!tf_stream_out_callback(&warp_energy_manager_v2->sd_energy_manager_daily_data_points_hlc, stream_length, chunk_offset, data_chunk_data, 15, tf_copy_items_uint32_t)) {
        return;
    }

    // Stream is either complete or out of sync
    uint32_t *data = (uint32_t *) (warp_energy_manager_v2->sd_energy_manager_daily_data_points_hlc.length == 0 ? NULL : warp_energy_manager_v2->sd_energy_manager_daily_data_points_hlc.data);
    warp_energy_manager_v2->sd_energy_manager_daily_data_points_handler(warp_energy_manager_v2, data, data_length, user_data);

    warp_energy_manager_v2->sd_energy_manager_daily_data_points_hlc.stream_in_progress = false;
    warp_energy_manager_v2->sd_energy_manager_daily_data_points_hlc.length = 0;
}

int tf_warp_energy_manager_v2_register_sd_energy_manager_daily_data_points_callback(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, TF_WARPEnergyManagerV2_SDEnergyManagerDailyDataPointsHandler handler, uint32_t *data_buffer, void *user_data) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_energy_manager_v2->sd_energy_manager_daily_data_points_handler = handler;

    warp_energy_manager_v2->sd_energy_manager_daily_data_points_hlc.data = data_buffer;
    warp_energy_manager_v2->sd_energy_manager_daily_data_points_hlc.length = 0;
    warp_energy_manager_v2->sd_energy_manager_daily_data_points_hlc.stream_in_progress = false;

    return tf_warp_energy_manager_v2_register_sd_energy_manager_daily_data_points_low_level_callback(warp_energy_manager_v2, handler == NULL ? NULL : tf_warp_energy_manager_v2_sd_energy_manager_daily_data_points_wrapper, user_data);
}
#endif
int tf_warp_energy_manager_v2_callback_tick(TF_WARPEnergyManagerV2 *warp_energy_manager_v2, uint32_t timeout_us) {
    if (warp_energy_manager_v2 == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager_v2->magic != 0x5446 || warp_energy_manager_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = warp_energy_manager_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(warp_energy_manager_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
