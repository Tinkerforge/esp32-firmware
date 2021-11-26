/* ***********************************************************
 * This file was automatically generated on 2021-11-26.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.0         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_sound_pressure_level.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_sound_pressure_level_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_SoundPressureLevel *sound_pressure_level = (TF_SoundPressureLevel *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(sound_pressure_level->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_SOUND_PRESSURE_LEVEL_CALLBACK_DECIBEL: {
            TF_SoundPressureLevel_DecibelHandler fn = sound_pressure_level->decibel_handler;
            void *user_data = sound_pressure_level->decibel_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t decibel = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(sound_pressure_level, decibel, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_SOUND_PRESSURE_LEVEL_CALLBACK_SPECTRUM_LOW_LEVEL: {
            TF_SoundPressureLevel_SpectrumLowLevelHandler fn = sound_pressure_level->spectrum_low_level_handler;
            void *user_data = sound_pressure_level->spectrum_low_level_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t i;
            uint16_t spectrum_length = tf_packet_buffer_read_uint16_t(payload);
            uint16_t spectrum_chunk_offset = tf_packet_buffer_read_uint16_t(payload);
            uint16_t spectrum_chunk_data[30]; for (i = 0; i < 30; ++i) spectrum_chunk_data[i] = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(sound_pressure_level, spectrum_length, spectrum_chunk_offset, spectrum_chunk_data, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_sound_pressure_level_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_sound_pressure_level_create(TF_SoundPressureLevel *sound_pressure_level, const char *uid, TF_HAL *hal) {
    if (sound_pressure_level == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    static uint16_t next_tfp_index = 0;

    memset(sound_pressure_level, 0, sizeof(TF_SoundPressureLevel));

    TF_TFP *tfp;

    if (uid != NULL && *uid != '\0') {
        uint32_t uid_num = 0;
        int rc = tf_base58_decode(uid, &uid_num);

        if (rc != TF_E_OK) {
            return rc;
        }

        tfp = tf_hal_get_tfp(hal, &next_tfp_index, &uid_num, NULL, NULL);

        if (tfp == NULL) {
            return TF_E_DEVICE_NOT_FOUND;
        }

        if (tfp->device_id != TF_SOUND_PRESSURE_LEVEL_DEVICE_IDENTIFIER) {
            return TF_E_WRONG_DEVICE_TYPE;
        }
    } else {
        uint16_t device_id = TF_SOUND_PRESSURE_LEVEL_DEVICE_IDENTIFIER;

        tfp = tf_hal_get_tfp(hal, &next_tfp_index, NULL, NULL, &device_id);

        if (tfp == NULL) {
            return TF_E_DEVICE_NOT_FOUND;
        }
    }

    if (tfp->device != NULL) {
        return TF_E_DEVICE_ALREADY_IN_USE;
    }

    sound_pressure_level->tfp = tfp;
    sound_pressure_level->tfp->device = sound_pressure_level;
    sound_pressure_level->tfp->cb_handler = tf_sound_pressure_level_callback_handler;
    sound_pressure_level->response_expected[0] = 0x03;

    return TF_E_OK;
}

int tf_sound_pressure_level_destroy(TF_SoundPressureLevel *sound_pressure_level) {
    if (sound_pressure_level == NULL || sound_pressure_level->tfp == NULL) {
        return TF_E_NULL;
    }

    sound_pressure_level->tfp->cb_handler = NULL;
    sound_pressure_level->tfp->device = NULL;
    sound_pressure_level->tfp = NULL;

    return TF_E_OK;
}

int tf_sound_pressure_level_get_response_expected(TF_SoundPressureLevel *sound_pressure_level, uint8_t function_id, bool *ret_response_expected) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    switch (function_id) {
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_DECIBEL_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (sound_pressure_level->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_SPECTRUM_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (sound_pressure_level->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (sound_pressure_level->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (sound_pressure_level->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (sound_pressure_level->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (sound_pressure_level->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (sound_pressure_level->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_sound_pressure_level_set_response_expected(TF_SoundPressureLevel *sound_pressure_level, uint8_t function_id, bool response_expected) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    switch (function_id) {
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_DECIBEL_CALLBACK_CONFIGURATION:
            if (response_expected) {
                sound_pressure_level->response_expected[0] |= (1 << 0);
            } else {
                sound_pressure_level->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_SPECTRUM_CALLBACK_CONFIGURATION:
            if (response_expected) {
                sound_pressure_level->response_expected[0] |= (1 << 1);
            } else {
                sound_pressure_level->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_CONFIGURATION:
            if (response_expected) {
                sound_pressure_level->response_expected[0] |= (1 << 2);
            } else {
                sound_pressure_level->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                sound_pressure_level->response_expected[0] |= (1 << 3);
            } else {
                sound_pressure_level->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                sound_pressure_level->response_expected[0] |= (1 << 4);
            } else {
                sound_pressure_level->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_RESET:
            if (response_expected) {
                sound_pressure_level->response_expected[0] |= (1 << 5);
            } else {
                sound_pressure_level->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_SOUND_PRESSURE_LEVEL_FUNCTION_WRITE_UID:
            if (response_expected) {
                sound_pressure_level->response_expected[0] |= (1 << 6);
            } else {
                sound_pressure_level->response_expected[0] &= ~(1 << 6);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_sound_pressure_level_set_response_expected_all(TF_SoundPressureLevel *sound_pressure_level, bool response_expected) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    memset(sound_pressure_level->response_expected, response_expected ? 0xFF : 0, 1);

    return TF_E_OK;
}

int tf_sound_pressure_level_get_decibel(TF_SoundPressureLevel *sound_pressure_level, uint16_t *ret_decibel) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_DECIBEL, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_decibel != NULL) { *ret_decibel = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_set_decibel_callback_configuration(TF_SoundPressureLevel *sound_pressure_level, uint32_t period, bool value_has_to_change, char option, uint16_t min, uint16_t max) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_sound_pressure_level_get_response_expected(sound_pressure_level, TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_DECIBEL_CALLBACK_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_DECIBEL_CALLBACK_CONFIGURATION, 10, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(sound_pressure_level->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(send_buf + 0, &period, 4);
    send_buf[4] = value_has_to_change ? 1 : 0;
    send_buf[5] = (uint8_t)option;
    min = tf_leconvert_uint16_to(min); memcpy(send_buf + 6, &min, 2);
    max = tf_leconvert_uint16_to(max); memcpy(send_buf + 8, &max, 2);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_get_decibel_callback_configuration(TF_SoundPressureLevel *sound_pressure_level, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, uint16_t *ret_min, uint16_t *ret_max) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_DECIBEL_CALLBACK_CONFIGURATION, 0, 10, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_option != NULL) { *ret_option = tf_packet_buffer_read_char(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_min != NULL) { *ret_min = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        if (ret_max != NULL) { *ret_max = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_get_spectrum_low_level(TF_SoundPressureLevel *sound_pressure_level, uint16_t *ret_spectrum_length, uint16_t *ret_spectrum_chunk_offset, uint16_t ret_spectrum_chunk_data[30]) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_SPECTRUM_LOW_LEVEL, 0, 64, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_spectrum_length != NULL) { *ret_spectrum_length = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        if (ret_spectrum_chunk_offset != NULL) { *ret_spectrum_chunk_offset = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        if (ret_spectrum_chunk_data != NULL) { for (i = 0; i < 30; ++i) ret_spectrum_chunk_data[i] = tf_packet_buffer_read_uint16_t(recv_buf);} else { tf_packet_buffer_remove(recv_buf, 60); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_set_spectrum_callback_configuration(TF_SoundPressureLevel *sound_pressure_level, uint32_t period) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_sound_pressure_level_get_response_expected(sound_pressure_level, TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_SPECTRUM_CALLBACK_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_SPECTRUM_CALLBACK_CONFIGURATION, 4, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(sound_pressure_level->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(send_buf + 0, &period, 4);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_get_spectrum_callback_configuration(TF_SoundPressureLevel *sound_pressure_level, uint32_t *ret_period) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_SPECTRUM_CALLBACK_CONFIGURATION, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_set_configuration(TF_SoundPressureLevel *sound_pressure_level, uint8_t fft_size, uint8_t weighting) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_sound_pressure_level_get_response_expected(sound_pressure_level, TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_CONFIGURATION, 2, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(sound_pressure_level->tfp);

    send_buf[0] = (uint8_t)fft_size;
    send_buf[1] = (uint8_t)weighting;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_get_configuration(TF_SoundPressureLevel *sound_pressure_level, uint8_t *ret_fft_size, uint8_t *ret_weighting) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_CONFIGURATION, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_fft_size != NULL) { *ret_fft_size = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_weighting != NULL) { *ret_weighting = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_get_spitfp_error_count(TF_SoundPressureLevel *sound_pressure_level, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, 16, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_set_bootloader_mode(TF_SoundPressureLevel *sound_pressure_level, uint8_t mode, uint8_t *ret_status) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_BOOTLOADER_MODE, 1, 1, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(sound_pressure_level->tfp);

    send_buf[0] = (uint8_t)mode;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_get_bootloader_mode(TF_SoundPressureLevel *sound_pressure_level, uint8_t *ret_mode) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_BOOTLOADER_MODE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_set_write_firmware_pointer(TF_SoundPressureLevel *sound_pressure_level, uint32_t pointer) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_sound_pressure_level_get_response_expected(sound_pressure_level, TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &response_expected);
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(sound_pressure_level->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(send_buf + 0, &pointer, 4);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_write_firmware(TF_SoundPressureLevel *sound_pressure_level, const uint8_t data[64], uint8_t *ret_status) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_WRITE_FIRMWARE, 64, 1, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(sound_pressure_level->tfp);

    memcpy(send_buf + 0, data, 64);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_set_status_led_config(TF_SoundPressureLevel *sound_pressure_level, uint8_t config) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_sound_pressure_level_get_response_expected(sound_pressure_level, TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_STATUS_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_SET_STATUS_LED_CONFIG, 1, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(sound_pressure_level->tfp);

    send_buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_get_status_led_config(TF_SoundPressureLevel *sound_pressure_level, uint8_t *ret_config) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_STATUS_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_get_chip_temperature(TF_SoundPressureLevel *sound_pressure_level, int16_t *ret_temperature) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_CHIP_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_reset(TF_SoundPressureLevel *sound_pressure_level) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_sound_pressure_level_get_response_expected(sound_pressure_level, TF_SOUND_PRESSURE_LEVEL_FUNCTION_RESET, &response_expected);
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_RESET, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_write_uid(TF_SoundPressureLevel *sound_pressure_level, uint32_t uid) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_sound_pressure_level_get_response_expected(sound_pressure_level, TF_SOUND_PRESSURE_LEVEL_FUNCTION_WRITE_UID, &response_expected);
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_WRITE_UID, 4, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(sound_pressure_level->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(send_buf + 0, &uid, 4);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_read_uid(TF_SoundPressureLevel *sound_pressure_level, uint32_t *ret_uid) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_READ_UID, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_sound_pressure_level_get_identity(TF_SoundPressureLevel *sound_pressure_level, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(sound_pressure_level->tfp, TF_SOUND_PRESSURE_LEVEL_FUNCTION_GET_IDENTITY, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(sound_pressure_level->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(sound_pressure_level->tfp);
        if (ret_uid != NULL) { tf_packet_buffer_pop_n(recv_buf, (uint8_t *)ret_uid, 8);} else { tf_packet_buffer_remove(recv_buf, 8); }
        if (ret_connected_uid != NULL) { tf_packet_buffer_pop_n(recv_buf, (uint8_t *)ret_connected_uid, 8);} else { tf_packet_buffer_remove(recv_buf, 8); }
        if (ret_position != NULL) { *ret_position = tf_packet_buffer_read_char(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_hardware_version != NULL) { for (i = 0; i < 3; ++i) ret_hardware_version[i] = tf_packet_buffer_read_uint8_t(recv_buf);} else { tf_packet_buffer_remove(recv_buf, 3); }
        if (ret_firmware_version != NULL) { for (i = 0; i < 3; ++i) ret_firmware_version[i] = tf_packet_buffer_read_uint8_t(recv_buf);} else { tf_packet_buffer_remove(recv_buf, 3); }
        if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(sound_pressure_level->tfp);
    }

    result = tf_tfp_finish_send(sound_pressure_level->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

static int tf_sound_pressure_level_get_spectrum_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    (void)wrapper_data;
    uint16_t spectrum_length = 0;
    uint16_t spectrum_chunk_offset = 0;
    uint16_t *spectrum_chunk_data = (uint16_t *) chunk_data;
    int ret = tf_sound_pressure_level_get_spectrum_low_level((TF_SoundPressureLevel *)device, &spectrum_length, &spectrum_chunk_offset, spectrum_chunk_data);

    *ret_stream_length = (uint32_t)spectrum_length;
    *ret_chunk_offset = (uint32_t)spectrum_chunk_offset;
    return ret;
}

int tf_sound_pressure_level_get_spectrum(TF_SoundPressureLevel *sound_pressure_level, uint16_t *ret_spectrum, uint16_t *ret_spectrum_length) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }
    
    uint32_t spectrum_length = 0;
    uint16_t spectrum_chunk_data[30];

    int ret = tf_stream_out(sound_pressure_level, tf_sound_pressure_level_get_spectrum_ll_wrapper, NULL, ret_spectrum, &spectrum_length, spectrum_chunk_data, 30, tf_copy_items_uint16_t);

    if (ret_spectrum_length != NULL) {
        *ret_spectrum_length = (uint16_t)spectrum_length;
    }
    return ret;
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_sound_pressure_level_register_decibel_callback(TF_SoundPressureLevel *sound_pressure_level, TF_SoundPressureLevel_DecibelHandler handler, void *user_data) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    if (handler == NULL) {
        sound_pressure_level->tfp->needs_callback_tick = false;
        sound_pressure_level->tfp->needs_callback_tick |= sound_pressure_level->spectrum_low_level_handler != NULL;
    } else {
        sound_pressure_level->tfp->needs_callback_tick = true;
    }

    sound_pressure_level->decibel_handler = handler;
    sound_pressure_level->decibel_user_data = user_data;

    return TF_E_OK;
}


int tf_sound_pressure_level_register_spectrum_low_level_callback(TF_SoundPressureLevel *sound_pressure_level, TF_SoundPressureLevel_SpectrumLowLevelHandler handler, void *user_data) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    if (handler == NULL) {
        sound_pressure_level->tfp->needs_callback_tick = false;
        sound_pressure_level->tfp->needs_callback_tick |= sound_pressure_level->decibel_handler != NULL;
    } else {
        sound_pressure_level->tfp->needs_callback_tick = true;
    }

    sound_pressure_level->spectrum_low_level_handler = handler;
    sound_pressure_level->spectrum_low_level_user_data = user_data;

    return TF_E_OK;
}


static void tf_sound_pressure_level_spectrum_wrapper(TF_SoundPressureLevel *sound_pressure_level, uint16_t spectrum_length, uint16_t spectrum_chunk_offset, uint16_t spectrum_chunk_data[30], void *user_data) {
    uint32_t stream_length = (uint32_t) spectrum_length;
    uint32_t chunk_offset = (uint32_t) spectrum_chunk_offset;
    if (!tf_stream_out_callback(&sound_pressure_level->spectrum_hlc, stream_length, chunk_offset, spectrum_chunk_data, 30, tf_copy_items_uint16_t)) {
        return;
    }

    // Stream is either complete or out of sync
    uint16_t *spectrum = (uint16_t *) (sound_pressure_level->spectrum_hlc.length == 0 ? NULL : sound_pressure_level->spectrum_hlc.data);
    sound_pressure_level->spectrum_handler(sound_pressure_level, spectrum, spectrum_length, user_data);

    sound_pressure_level->spectrum_hlc.stream_in_progress = false;
    sound_pressure_level->spectrum_hlc.length = 0;
}

int tf_sound_pressure_level_register_spectrum_callback(TF_SoundPressureLevel *sound_pressure_level, TF_SoundPressureLevel_SpectrumHandler handler, uint16_t *spectrum_buffer, void *user_data) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    sound_pressure_level->spectrum_handler = handler;

    sound_pressure_level->spectrum_hlc.data = spectrum_buffer;
    sound_pressure_level->spectrum_hlc.length = 0;
    sound_pressure_level->spectrum_hlc.stream_in_progress = false;

    return tf_sound_pressure_level_register_spectrum_low_level_callback(sound_pressure_level, handler == NULL ? NULL : tf_sound_pressure_level_spectrum_wrapper, user_data);
}
#endif
int tf_sound_pressure_level_callback_tick(TF_SoundPressureLevel *sound_pressure_level, uint32_t timeout_us) {
    if (sound_pressure_level == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = sound_pressure_level->tfp->spitfp->hal;

    return tf_tfp_callback_tick(sound_pressure_level->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
