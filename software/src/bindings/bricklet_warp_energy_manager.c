/* ***********************************************************
 * This file was automatically generated on 2023-01-25.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.3         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_warp_energy_manager.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


static bool tf_warp_energy_manager_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    (void)device;
    (void)fid;
    (void)payload;

    return false;
}
int tf_warp_energy_manager_create(TF_WARPEnergyManager *warp_energy_manager, const char *uid_or_port_name, TF_HAL *hal) {
    if (warp_energy_manager == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(warp_energy_manager, 0, sizeof(TF_WARPEnergyManager));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_WARP_ENERGY_MANAGER_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    warp_energy_manager->tfp = tfp;
    warp_energy_manager->tfp->device = warp_energy_manager;
    warp_energy_manager->tfp->cb_handler = tf_warp_energy_manager_callback_handler;
    warp_energy_manager->magic = 0x5446;
    warp_energy_manager->response_expected[0] = 0x00;
    return TF_E_OK;
}

int tf_warp_energy_manager_destroy(TF_WARPEnergyManager *warp_energy_manager) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }
    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_energy_manager->tfp->cb_handler = NULL;
    warp_energy_manager->tfp->device = NULL;
    warp_energy_manager->tfp = NULL;
    warp_energy_manager->magic = 0;

    return TF_E_OK;
}

int tf_warp_energy_manager_get_response_expected(TF_WARPEnergyManager *warp_energy_manager, uint8_t function_id, bool *ret_response_expected) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_CONTACTOR:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_RGB_VALUE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_OUTPUT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_SD_WALLBOX_DATA_POINT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_energy_manager->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_warp_energy_manager_set_response_expected(TF_WARPEnergyManager *warp_energy_manager, uint8_t function_id, bool response_expected) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_CONTACTOR:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 0);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_RGB_VALUE:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 1);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_OUTPUT:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 2);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_SD_WALLBOX_DATA_POINT:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 3);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 4);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 5);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_RESET:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 6);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_WARP_ENERGY_MANAGER_FUNCTION_WRITE_UID:
            if (response_expected) {
                warp_energy_manager->response_expected[0] |= (1 << 7);
            } else {
                warp_energy_manager->response_expected[0] &= ~(1 << 7);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_warp_energy_manager_set_response_expected_all(TF_WARPEnergyManager *warp_energy_manager, bool response_expected) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(warp_energy_manager->response_expected, response_expected ? 0xFF : 0, 1);

    return TF_E_OK;
}

int tf_warp_energy_manager_set_contactor(TF_WARPEnergyManager *warp_energy_manager, bool contactor_value) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_CONTACTOR, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_CONTACTOR, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager->tfp);

    _send_buf[0] = contactor_value ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_contactor(TF_WARPEnergyManager *warp_energy_manager, bool *ret_contactor_value) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_CONTACTOR, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_contactor_value != NULL) { *ret_contactor_value = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_set_rgb_value(TF_WARPEnergyManager *warp_energy_manager, uint8_t r, uint8_t g, uint8_t b) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_RGB_VALUE, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_RGB_VALUE, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager->tfp);

    _send_buf[0] = (uint8_t)r;
    _send_buf[1] = (uint8_t)g;
    _send_buf[2] = (uint8_t)b;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_rgb_value(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_r, uint8_t *ret_g, uint8_t *ret_b) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_RGB_VALUE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 3) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_r != NULL) { *ret_r = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_g != NULL) { *ret_g = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_b != NULL) { *ret_b = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 3) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_energy_meter_values(TF_WARPEnergyManager *warp_energy_manager, float *ret_power, float *ret_energy_import, float *ret_energy_export) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_ENERGY_METER_VALUES, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 12) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_power != NULL) { *ret_power = tf_packet_buffer_read_float(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_energy_import != NULL) { *ret_energy_import = tf_packet_buffer_read_float(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_energy_export != NULL) { *ret_energy_export = tf_packet_buffer_read_float(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 12) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_energy_meter_detailed_values_low_level(TF_WARPEnergyManager *warp_energy_manager, uint16_t *ret_values_chunk_offset, float ret_values_chunk_data[15]) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_ENERGY_METER_DETAILED_VALUES_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 62) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_values_chunk_offset != NULL) { *ret_values_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_values_chunk_data != NULL) { for (_i = 0; _i < 15; ++_i) ret_values_chunk_data[_i] = tf_packet_buffer_read_float(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 62) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_energy_meter_state(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_energy_meter_type, uint32_t ret_error_count[6]) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_ENERGY_METER_STATE, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 25) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_energy_meter_type != NULL) { *ret_energy_meter_type = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_error_count != NULL) { for (_i = 0; _i < 6; ++_i) ret_error_count[_i] = tf_packet_buffer_read_uint32_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 24); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_input(TF_WARPEnergyManager *warp_energy_manager, bool ret_input[2]) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_INPUT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_input != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_input, 2);} else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_set_output(TF_WARPEnergyManager *warp_energy_manager, bool output) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_OUTPUT, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_OUTPUT, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager->tfp);

    _send_buf[0] = output ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_output(TF_WARPEnergyManager *warp_energy_manager, bool *ret_output) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_OUTPUT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_output != NULL) { *ret_output = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_input_voltage(TF_WARPEnergyManager *warp_energy_manager, uint16_t *ret_voltage) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_INPUT_VOLTAGE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_voltage != NULL) { *ret_voltage = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_state(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_contactor_check_state) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_contactor_check_state != NULL) { *ret_contactor_check_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_all_data_1(TF_WARPEnergyManager *warp_energy_manager, bool *ret_contactor_value, uint8_t *ret_r, uint8_t *ret_g, uint8_t *ret_b, float *ret_power, float *ret_energy_import, float *ret_energy_export, uint8_t *ret_energy_meter_type, uint32_t ret_error_count[6], bool ret_input[2], bool *ret_output, uint16_t *ret_voltage, uint8_t *ret_contactor_check_state) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_ALL_DATA_1, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 46) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_contactor_value != NULL) { *ret_contactor_value = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_r != NULL) { *ret_r = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_g != NULL) { *ret_g = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_b != NULL) { *ret_b = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_power != NULL) { *ret_power = tf_packet_buffer_read_float(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_energy_import != NULL) { *ret_energy_import = tf_packet_buffer_read_float(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_energy_export != NULL) { *ret_energy_export = tf_packet_buffer_read_float(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_energy_meter_type != NULL) { *ret_energy_meter_type = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_error_count != NULL) { for (_i = 0; _i < 6; ++_i) ret_error_count[_i] = tf_packet_buffer_read_uint32_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 24); }
            if (ret_input != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_input, 2);} else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_output != NULL) { *ret_output = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_voltage != NULL) { *ret_voltage = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_contactor_check_state != NULL) { *ret_contactor_check_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 46) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_sd_information(TF_WARPEnergyManager *warp_energy_manager, uint32_t *ret_sd_status, uint32_t *ret_lfs_status, uint16_t *ret_sector_size, uint32_t *ret_sector_count, uint32_t *ret_card_type, uint8_t *ret_product_rev, char ret_product_name[5], uint8_t *ret_manufacturer_id) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_SD_INFORMATION, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
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
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_set_sd_wallbox_data_point(TF_WARPEnergyManager *warp_energy_manager, uint8_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t flags, uint16_t power) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_SD_WALLBOX_DATA_POINT, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_SD_WALLBOX_DATA_POINT, 9, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager->tfp);

    _send_buf[0] = (uint8_t)wallbox_id;
    _send_buf[1] = (uint8_t)year;
    _send_buf[2] = (uint8_t)month;
    _send_buf[3] = (uint8_t)day;
    _send_buf[4] = (uint8_t)hour;
    _send_buf[5] = (uint8_t)minute;
    _send_buf[6] = (uint8_t)flags;
    power = tf_leconvert_uint16_to(power); memcpy(_send_buf + 7, &power, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_sd_wallbox_data_point(TF_WARPEnergyManager *warp_energy_manager, uint8_t wallbox_id, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t *ret_flags, uint16_t *ret_power) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_SD_WALLBOX_DATA_POINT, 6, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager->tfp);

    _send_buf[0] = (uint8_t)wallbox_id;
    _send_buf[1] = (uint8_t)year;
    _send_buf[2] = (uint8_t)month;
    _send_buf[3] = (uint8_t)day;
    _send_buf[4] = (uint8_t)hour;
    _send_buf[5] = (uint8_t)minute;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 3) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_flags != NULL) { *ret_flags = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_power != NULL) { *ret_power = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 3) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_spitfp_error_count(TF_WARPEnergyManager *warp_energy_manager, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_set_bootloader_mode(TF_WARPEnergyManager *warp_energy_manager, uint8_t mode, uint8_t *ret_status) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_bootloader_mode(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_mode) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_set_write_firmware_pointer(TF_WARPEnergyManager *warp_energy_manager, uint32_t pointer) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_write_firmware(TF_WARPEnergyManager *warp_energy_manager, const uint8_t data[64], uint8_t *ret_status) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_set_status_led_config(TF_WARPEnergyManager *warp_energy_manager, uint8_t config) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_status_led_config(TF_WARPEnergyManager *warp_energy_manager, uint8_t *ret_config) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_chip_temperature(TF_WARPEnergyManager *warp_energy_manager, int16_t *ret_temperature) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_reset(TF_WARPEnergyManager *warp_energy_manager) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_write_uid(TF_WARPEnergyManager *warp_energy_manager, uint32_t uid) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_energy_manager_get_response_expected(warp_energy_manager, TF_WARP_ENERGY_MANAGER_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_energy_manager->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_read_uid(TF_WARPEnergyManager *warp_energy_manager, uint32_t *ret_uid) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_energy_manager_get_identity(TF_WARPEnergyManager *warp_energy_manager, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_energy_manager->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_energy_manager->tfp, TF_WARP_ENERGY_MANAGER_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_energy_manager->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_energy_manager->tfp);
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
        tf_tfp_packet_processed(warp_energy_manager->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_energy_manager->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

static int tf_warp_energy_manager_get_energy_meter_detailed_values_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    (void)wrapper_data;
    uint16_t values_length = 85;
    uint16_t values_chunk_offset = 0;
    float *values_chunk_data = (float *) chunk_data;
    int ret = tf_warp_energy_manager_get_energy_meter_detailed_values_low_level((TF_WARPEnergyManager *)device, &values_chunk_offset, values_chunk_data);

    if (values_chunk_offset == (1 << 16) - 1) { // maximum chunk offset -> stream has no data
        return TF_E_INTERNAL_STREAM_HAS_NO_DATA;
    }

    *ret_stream_length = (uint32_t)values_length;
    *ret_chunk_offset = (uint32_t)values_chunk_offset;
    return ret;
}

int tf_warp_energy_manager_get_energy_meter_detailed_values(TF_WARPEnergyManager *warp_energy_manager, float *ret_values, uint16_t *ret_values_length) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    uint32_t _values_length = 0;
    float _values_chunk_data[15];

    int ret = tf_stream_out(warp_energy_manager, tf_warp_energy_manager_get_energy_meter_detailed_values_ll_wrapper, NULL, ret_values, &_values_length, _values_chunk_data, 15, tf_copy_items_float);

    if (ret_values_length != NULL) {
        *ret_values_length = (uint16_t)_values_length;
    }
    return ret;
}


int tf_warp_energy_manager_callback_tick(TF_WARPEnergyManager *warp_energy_manager, uint32_t timeout_us) {
    if (warp_energy_manager == NULL) {
        return TF_E_NULL;
    }

    if (warp_energy_manager->magic != 0x5446 || warp_energy_manager->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = warp_energy_manager->tfp->spitfp->hal;

    return tf_tfp_callback_tick(warp_energy_manager->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
