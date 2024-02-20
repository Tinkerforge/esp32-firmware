/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_energy_monitor.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_energy_monitor_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_EnergyMonitor *energy_monitor = (TF_EnergyMonitor *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(energy_monitor->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_ENERGY_MONITOR_CALLBACK_ENERGY_DATA: {
            TF_EnergyMonitor_EnergyDataHandler fn = energy_monitor->energy_data_handler;
            void *user_data = energy_monitor->energy_data_user_data;
            if (fn == NULL) {
                return false;
            }

            int32_t voltage = tf_packet_buffer_read_int32_t(payload);
            int32_t current = tf_packet_buffer_read_int32_t(payload);
            int32_t energy = tf_packet_buffer_read_int32_t(payload);
            int32_t real_power = tf_packet_buffer_read_int32_t(payload);
            int32_t apparent_power = tf_packet_buffer_read_int32_t(payload);
            int32_t reactive_power = tf_packet_buffer_read_int32_t(payload);
            uint16_t power_factor = tf_packet_buffer_read_uint16_t(payload);
            uint16_t frequency = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(energy_monitor, voltage, current, energy, real_power, apparent_power, reactive_power, power_factor, frequency, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_energy_monitor_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_energy_monitor_create(TF_EnergyMonitor *energy_monitor, const char *uid_or_port_name, TF_HAL *hal) {
    if (energy_monitor == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(energy_monitor, 0, sizeof(TF_EnergyMonitor));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_ENERGY_MONITOR_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    energy_monitor->tfp = tfp;
    energy_monitor->tfp->device = energy_monitor;
    energy_monitor->tfp->cb_handler = tf_energy_monitor_callback_handler;
    energy_monitor->magic = 0x5446;
    energy_monitor->response_expected[0] = 0x08;
    return TF_E_OK;
}

int tf_energy_monitor_destroy(TF_EnergyMonitor *energy_monitor) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }
    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    energy_monitor->tfp->cb_handler = NULL;
    energy_monitor->tfp->device = NULL;
    energy_monitor->tfp = NULL;
    energy_monitor->magic = 0;

    return TF_E_OK;
}

int tf_energy_monitor_get_response_expected(TF_EnergyMonitor *energy_monitor, uint8_t function_id, bool *ret_response_expected) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_ENERGY_MONITOR_FUNCTION_RESET_ENERGY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (energy_monitor->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_SET_TRANSFORMER_CALIBRATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (energy_monitor->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_CALIBRATE_OFFSET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (energy_monitor->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_SET_ENERGY_DATA_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (energy_monitor->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (energy_monitor->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (energy_monitor->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (energy_monitor->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (energy_monitor->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_energy_monitor_set_response_expected(TF_EnergyMonitor *energy_monitor, uint8_t function_id, bool response_expected) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_ENERGY_MONITOR_FUNCTION_RESET_ENERGY:
            if (response_expected) {
                energy_monitor->response_expected[0] |= (1 << 0);
            } else {
                energy_monitor->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_SET_TRANSFORMER_CALIBRATION:
            if (response_expected) {
                energy_monitor->response_expected[0] |= (1 << 1);
            } else {
                energy_monitor->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_CALIBRATE_OFFSET:
            if (response_expected) {
                energy_monitor->response_expected[0] |= (1 << 2);
            } else {
                energy_monitor->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_SET_ENERGY_DATA_CALLBACK_CONFIGURATION:
            if (response_expected) {
                energy_monitor->response_expected[0] |= (1 << 3);
            } else {
                energy_monitor->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                energy_monitor->response_expected[0] |= (1 << 4);
            } else {
                energy_monitor->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                energy_monitor->response_expected[0] |= (1 << 5);
            } else {
                energy_monitor->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_RESET:
            if (response_expected) {
                energy_monitor->response_expected[0] |= (1 << 6);
            } else {
                energy_monitor->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_ENERGY_MONITOR_FUNCTION_WRITE_UID:
            if (response_expected) {
                energy_monitor->response_expected[0] |= (1 << 7);
            } else {
                energy_monitor->response_expected[0] &= ~(1 << 7);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_energy_monitor_set_response_expected_all(TF_EnergyMonitor *energy_monitor, bool response_expected) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(energy_monitor->response_expected, response_expected ? 0xFF : 0, 1);

    return TF_E_OK;
}

int tf_energy_monitor_get_energy_data(TF_EnergyMonitor *energy_monitor, int32_t *ret_voltage, int32_t *ret_current, int32_t *ret_energy, int32_t *ret_real_power, int32_t *ret_apparent_power, int32_t *ret_reactive_power, uint16_t *ret_power_factor, uint16_t *ret_frequency) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_GET_ENERGY_DATA, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
        if (_error_code != 0 || _length != 28) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_voltage != NULL) { *ret_voltage = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_current != NULL) { *ret_current = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_energy != NULL) { *ret_energy = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_real_power != NULL) { *ret_real_power = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_apparent_power != NULL) { *ret_apparent_power = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_reactive_power != NULL) { *ret_reactive_power = tf_packet_buffer_read_int32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_power_factor != NULL) { *ret_power_factor = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_frequency != NULL) { *ret_frequency = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 28) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_reset_energy(TF_EnergyMonitor *energy_monitor) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_energy_monitor_get_response_expected(energy_monitor, TF_ENERGY_MONITOR_FUNCTION_RESET_ENERGY, &_response_expected);
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_RESET_ENERGY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_get_waveform_low_level(TF_EnergyMonitor *energy_monitor, uint16_t *ret_waveform_chunk_offset, int16_t ret_waveform_chunk_data[30]) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_GET_WAVEFORM_LOW_LEVEL, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
        if (_error_code != 0 || _length != 62) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_waveform_chunk_offset != NULL) { *ret_waveform_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_waveform_chunk_data != NULL) { for (_i = 0; _i < 30; ++_i) ret_waveform_chunk_data[_i] = tf_packet_buffer_read_int16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 62) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_get_transformer_status(TF_EnergyMonitor *energy_monitor, bool *ret_voltage_transformer_connected, bool *ret_current_transformer_connected) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_GET_TRANSFORMER_STATUS, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_voltage_transformer_connected != NULL) { *ret_voltage_transformer_connected = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_current_transformer_connected != NULL) { *ret_current_transformer_connected = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_set_transformer_calibration(TF_EnergyMonitor *energy_monitor, uint16_t voltage_ratio, uint16_t current_ratio, int16_t phase_shift) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_energy_monitor_get_response_expected(energy_monitor, TF_ENERGY_MONITOR_FUNCTION_SET_TRANSFORMER_CALIBRATION, &_response_expected);
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_SET_TRANSFORMER_CALIBRATION, 6, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(energy_monitor->tfp);

    voltage_ratio = tf_leconvert_uint16_to(voltage_ratio); memcpy(_send_buf + 0, &voltage_ratio, 2);
    current_ratio = tf_leconvert_uint16_to(current_ratio); memcpy(_send_buf + 2, &current_ratio, 2);
    phase_shift = tf_leconvert_int16_to(phase_shift); memcpy(_send_buf + 4, &phase_shift, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_get_transformer_calibration(TF_EnergyMonitor *energy_monitor, uint16_t *ret_voltage_ratio, uint16_t *ret_current_ratio, int16_t *ret_phase_shift) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_GET_TRANSFORMER_CALIBRATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_voltage_ratio != NULL) { *ret_voltage_ratio = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_current_ratio != NULL) { *ret_current_ratio = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_phase_shift != NULL) { *ret_phase_shift = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_calibrate_offset(TF_EnergyMonitor *energy_monitor) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_energy_monitor_get_response_expected(energy_monitor, TF_ENERGY_MONITOR_FUNCTION_CALIBRATE_OFFSET, &_response_expected);
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_CALIBRATE_OFFSET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_set_energy_data_callback_configuration(TF_EnergyMonitor *energy_monitor, uint32_t period, bool value_has_to_change) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_energy_monitor_get_response_expected(energy_monitor, TF_ENERGY_MONITOR_FUNCTION_SET_ENERGY_DATA_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_SET_ENERGY_DATA_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(energy_monitor->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_get_energy_data_callback_configuration(TF_EnergyMonitor *energy_monitor, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_GET_ENERGY_DATA_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_get_spitfp_error_count(TF_EnergyMonitor *energy_monitor, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_set_bootloader_mode(TF_EnergyMonitor *energy_monitor, uint8_t mode, uint8_t *ret_status) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(energy_monitor->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_get_bootloader_mode(TF_EnergyMonitor *energy_monitor, uint8_t *ret_mode) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_set_write_firmware_pointer(TF_EnergyMonitor *energy_monitor, uint32_t pointer) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_energy_monitor_get_response_expected(energy_monitor, TF_ENERGY_MONITOR_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(energy_monitor->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_write_firmware(TF_EnergyMonitor *energy_monitor, const uint8_t data[64], uint8_t *ret_status) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(energy_monitor->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_set_status_led_config(TF_EnergyMonitor *energy_monitor, uint8_t config) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_energy_monitor_get_response_expected(energy_monitor, TF_ENERGY_MONITOR_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(energy_monitor->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_get_status_led_config(TF_EnergyMonitor *energy_monitor, uint8_t *ret_config) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_get_chip_temperature(TF_EnergyMonitor *energy_monitor, int16_t *ret_temperature) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_reset(TF_EnergyMonitor *energy_monitor) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_energy_monitor_get_response_expected(energy_monitor, TF_ENERGY_MONITOR_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_write_uid(TF_EnergyMonitor *energy_monitor, uint32_t uid) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_energy_monitor_get_response_expected(energy_monitor, TF_ENERGY_MONITOR_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(energy_monitor->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_read_uid(TF_EnergyMonitor *energy_monitor, uint32_t *ret_uid) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_energy_monitor_get_identity(TF_EnergyMonitor *energy_monitor, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = energy_monitor->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(energy_monitor->tfp, TF_ENERGY_MONITOR_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(energy_monitor->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(energy_monitor->tfp);
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
        tf_tfp_packet_processed(energy_monitor->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(energy_monitor->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

static int tf_energy_monitor_get_waveform_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    (void)wrapper_data;
    uint16_t waveform_length = 1536;
    uint16_t waveform_chunk_offset = 0;
    int16_t *waveform_chunk_data = (int16_t *) chunk_data;
    int ret = tf_energy_monitor_get_waveform_low_level((TF_EnergyMonitor *)device, &waveform_chunk_offset, waveform_chunk_data);

    if (waveform_chunk_offset == (1 << 16) - 1) { // maximum chunk offset -> stream has no data
        return TF_E_INTERNAL_STREAM_HAS_NO_DATA;
    }

    *ret_stream_length = (uint32_t)waveform_length;
    *ret_chunk_offset = (uint32_t)waveform_chunk_offset;
    return ret;
}

int tf_energy_monitor_get_waveform(TF_EnergyMonitor *energy_monitor, int16_t *ret_waveform, uint16_t *ret_waveform_length) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    uint32_t _waveform_length = 0;
    int16_t _waveform_chunk_data[30];

    int ret = tf_stream_out(energy_monitor, tf_energy_monitor_get_waveform_ll_wrapper, NULL, ret_waveform, &_waveform_length, _waveform_chunk_data, 30, tf_copy_items_int16_t);

    if (ret_waveform_length != NULL) {
        *ret_waveform_length = (uint16_t)_waveform_length;
    }
    return ret;
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_energy_monitor_register_energy_data_callback(TF_EnergyMonitor *energy_monitor, TF_EnergyMonitor_EnergyDataHandler handler, void *user_data) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    energy_monitor->energy_data_handler = handler;
    energy_monitor->energy_data_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_energy_monitor_callback_tick(TF_EnergyMonitor *energy_monitor, uint32_t timeout_us) {
    if (energy_monitor == NULL) {
        return TF_E_NULL;
    }

    if (energy_monitor->magic != 0x5446 || energy_monitor->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = energy_monitor->tfp->spitfp->hal;

    return tf_tfp_callback_tick(energy_monitor->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
