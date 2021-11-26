/* ***********************************************************
 * This file was automatically generated on 2021-11-26.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.0         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_color_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_color_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_ColorV2 *color_v2 = (TF_ColorV2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(color_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_COLOR_V2_CALLBACK_COLOR: {
            TF_ColorV2_ColorHandler fn = color_v2->color_handler;
            void *user_data = color_v2->color_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t r = tf_packet_buffer_read_uint16_t(payload);
            uint16_t g = tf_packet_buffer_read_uint16_t(payload);
            uint16_t b = tf_packet_buffer_read_uint16_t(payload);
            uint16_t c = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(color_v2, r, g, b, c, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_COLOR_V2_CALLBACK_ILLUMINANCE: {
            TF_ColorV2_IlluminanceHandler fn = color_v2->illuminance_handler;
            void *user_data = color_v2->illuminance_user_data;
            if (fn == NULL) {
                return false;
            }

            uint32_t illuminance = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(color_v2, illuminance, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_COLOR_V2_CALLBACK_COLOR_TEMPERATURE: {
            TF_ColorV2_ColorTemperatureHandler fn = color_v2->color_temperature_handler;
            void *user_data = color_v2->color_temperature_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t color_temperature = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(color_v2, color_temperature, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_color_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_color_v2_create(TF_ColorV2 *color_v2, const char *uid, TF_HAL *hal) {
    if (color_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    static uint16_t next_tfp_index = 0;

    memset(color_v2, 0, sizeof(TF_ColorV2));

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

        if (tfp->device_id != TF_COLOR_V2_DEVICE_IDENTIFIER) {
            return TF_E_WRONG_DEVICE_TYPE;
        }
    } else {
        uint16_t device_id = TF_COLOR_V2_DEVICE_IDENTIFIER;

        tfp = tf_hal_get_tfp(hal, &next_tfp_index, NULL, NULL, &device_id);

        if (tfp == NULL) {
            return TF_E_DEVICE_NOT_FOUND;
        }
    }

    if (tfp->device != NULL) {
        return TF_E_DEVICE_ALREADY_IN_USE;
    }

    color_v2->tfp = tfp;
    color_v2->tfp->device = color_v2;
    color_v2->tfp->cb_handler = tf_color_v2_callback_handler;
    color_v2->response_expected[0] = 0x07;
    color_v2->response_expected[1] = 0x00;

    return TF_E_OK;
}

int tf_color_v2_destroy(TF_ColorV2 *color_v2) {
    if (color_v2 == NULL || color_v2->tfp == NULL) {
        return TF_E_NULL;
    }

    color_v2->tfp->cb_handler = NULL;
    color_v2->tfp->device = NULL;
    color_v2->tfp = NULL;

    return TF_E_OK;
}

int tf_color_v2_get_response_expected(TF_ColorV2 *color_v2, uint8_t function_id, bool *ret_response_expected) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    switch (function_id) {
        case TF_COLOR_V2_FUNCTION_SET_COLOR_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (color_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_COLOR_V2_FUNCTION_SET_ILLUMINANCE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (color_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_COLOR_V2_FUNCTION_SET_COLOR_TEMPERATURE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (color_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_COLOR_V2_FUNCTION_SET_LIGHT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (color_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_COLOR_V2_FUNCTION_SET_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (color_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_COLOR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (color_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_COLOR_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (color_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_COLOR_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (color_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_COLOR_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (color_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_color_v2_set_response_expected(TF_ColorV2 *color_v2, uint8_t function_id, bool response_expected) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    switch (function_id) {
        case TF_COLOR_V2_FUNCTION_SET_COLOR_CALLBACK_CONFIGURATION:
            if (response_expected) {
                color_v2->response_expected[0] |= (1 << 0);
            } else {
                color_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_COLOR_V2_FUNCTION_SET_ILLUMINANCE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                color_v2->response_expected[0] |= (1 << 1);
            } else {
                color_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_COLOR_V2_FUNCTION_SET_COLOR_TEMPERATURE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                color_v2->response_expected[0] |= (1 << 2);
            } else {
                color_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_COLOR_V2_FUNCTION_SET_LIGHT:
            if (response_expected) {
                color_v2->response_expected[0] |= (1 << 3);
            } else {
                color_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_COLOR_V2_FUNCTION_SET_CONFIGURATION:
            if (response_expected) {
                color_v2->response_expected[0] |= (1 << 4);
            } else {
                color_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_COLOR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                color_v2->response_expected[0] |= (1 << 5);
            } else {
                color_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_COLOR_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                color_v2->response_expected[0] |= (1 << 6);
            } else {
                color_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_COLOR_V2_FUNCTION_RESET:
            if (response_expected) {
                color_v2->response_expected[0] |= (1 << 7);
            } else {
                color_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_COLOR_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                color_v2->response_expected[1] |= (1 << 0);
            } else {
                color_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_color_v2_set_response_expected_all(TF_ColorV2 *color_v2, bool response_expected) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    memset(color_v2->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_color_v2_get_color(TF_ColorV2 *color_v2, uint16_t *ret_r, uint16_t *ret_g, uint16_t *ret_b, uint16_t *ret_c) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_COLOR, 0, 8, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_r != NULL) { *ret_r = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        if (ret_g != NULL) { *ret_g = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        if (ret_b != NULL) { *ret_b = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        if (ret_c != NULL) { *ret_c = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_set_color_callback_configuration(TF_ColorV2 *color_v2, uint32_t period, bool value_has_to_change) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_color_v2_get_response_expected(color_v2, TF_COLOR_V2_FUNCTION_SET_COLOR_CALLBACK_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_SET_COLOR_CALLBACK_CONFIGURATION, 5, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(color_v2->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(send_buf + 0, &period, 4);
    send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_get_color_callback_configuration(TF_ColorV2 *color_v2, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_COLOR_CALLBACK_CONFIGURATION, 0, 5, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_get_illuminance(TF_ColorV2 *color_v2, uint32_t *ret_illuminance) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_ILLUMINANCE, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_illuminance != NULL) { *ret_illuminance = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_set_illuminance_callback_configuration(TF_ColorV2 *color_v2, uint32_t period, bool value_has_to_change, char option, uint32_t min, uint32_t max) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_color_v2_get_response_expected(color_v2, TF_COLOR_V2_FUNCTION_SET_ILLUMINANCE_CALLBACK_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_SET_ILLUMINANCE_CALLBACK_CONFIGURATION, 14, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(color_v2->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(send_buf + 0, &period, 4);
    send_buf[4] = value_has_to_change ? 1 : 0;
    send_buf[5] = (uint8_t)option;
    min = tf_leconvert_uint32_to(min); memcpy(send_buf + 6, &min, 4);
    max = tf_leconvert_uint32_to(max); memcpy(send_buf + 10, &max, 4);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_get_illuminance_callback_configuration(TF_ColorV2 *color_v2, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, uint32_t *ret_min, uint32_t *ret_max) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_ILLUMINANCE_CALLBACK_CONFIGURATION, 0, 14, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_option != NULL) { *ret_option = tf_packet_buffer_read_char(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_min != NULL) { *ret_min = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_max != NULL) { *ret_max = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_get_color_temperature(TF_ColorV2 *color_v2, uint16_t *ret_color_temperature) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_COLOR_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_color_temperature != NULL) { *ret_color_temperature = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_set_color_temperature_callback_configuration(TF_ColorV2 *color_v2, uint32_t period, bool value_has_to_change, char option, uint16_t min, uint16_t max) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_color_v2_get_response_expected(color_v2, TF_COLOR_V2_FUNCTION_SET_COLOR_TEMPERATURE_CALLBACK_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_SET_COLOR_TEMPERATURE_CALLBACK_CONFIGURATION, 10, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(color_v2->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(send_buf + 0, &period, 4);
    send_buf[4] = value_has_to_change ? 1 : 0;
    send_buf[5] = (uint8_t)option;
    min = tf_leconvert_uint16_to(min); memcpy(send_buf + 6, &min, 2);
    max = tf_leconvert_uint16_to(max); memcpy(send_buf + 8, &max, 2);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_get_color_temperature_callback_configuration(TF_ColorV2 *color_v2, uint32_t *ret_period, bool *ret_value_has_to_change, char *ret_option, uint16_t *ret_min, uint16_t *ret_max) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_COLOR_TEMPERATURE_CALLBACK_CONFIGURATION, 0, 10, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_option != NULL) { *ret_option = tf_packet_buffer_read_char(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_min != NULL) { *ret_min = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        if (ret_max != NULL) { *ret_max = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_set_light(TF_ColorV2 *color_v2, bool enable) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_color_v2_get_response_expected(color_v2, TF_COLOR_V2_FUNCTION_SET_LIGHT, &response_expected);
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_SET_LIGHT, 1, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(color_v2->tfp);

    send_buf[0] = enable ? 1 : 0;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_get_light(TF_ColorV2 *color_v2, bool *ret_enable) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_LIGHT, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_enable != NULL) { *ret_enable = tf_packet_buffer_read_bool(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_set_configuration(TF_ColorV2 *color_v2, uint8_t gain, uint8_t integration_time) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_color_v2_get_response_expected(color_v2, TF_COLOR_V2_FUNCTION_SET_CONFIGURATION, &response_expected);
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_SET_CONFIGURATION, 2, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(color_v2->tfp);

    send_buf[0] = (uint8_t)gain;
    send_buf[1] = (uint8_t)integration_time;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_get_configuration(TF_ColorV2 *color_v2, uint8_t *ret_gain, uint8_t *ret_integration_time) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_CONFIGURATION, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_gain != NULL) { *ret_gain = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_integration_time != NULL) { *ret_integration_time = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_get_spitfp_error_count(TF_ColorV2 *color_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, 16, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_set_bootloader_mode(TF_ColorV2 *color_v2, uint8_t mode, uint8_t *ret_status) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, 1, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(color_v2->tfp);

    send_buf[0] = (uint8_t)mode;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_get_bootloader_mode(TF_ColorV2 *color_v2, uint8_t *ret_mode) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_set_write_firmware_pointer(TF_ColorV2 *color_v2, uint32_t pointer) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_color_v2_get_response_expected(color_v2, TF_COLOR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &response_expected);
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(color_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(send_buf + 0, &pointer, 4);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_write_firmware(TF_ColorV2 *color_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_WRITE_FIRMWARE, 64, 1, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(color_v2->tfp);

    memcpy(send_buf + 0, data, 64);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_set_status_led_config(TF_ColorV2 *color_v2, uint8_t config) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_color_v2_get_response_expected(color_v2, TF_COLOR_V2_FUNCTION_SET_STATUS_LED_CONFIG, &response_expected);
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(color_v2->tfp);

    send_buf[0] = (uint8_t)config;

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_get_status_led_config(TF_ColorV2 *color_v2, uint8_t *ret_config) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, 1, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_get_chip_temperature(TF_ColorV2 *color_v2, int16_t *ret_temperature) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, 2, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_reset(TF_ColorV2 *color_v2) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_color_v2_get_response_expected(color_v2, TF_COLOR_V2_FUNCTION_RESET, &response_expected);
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_RESET, 0, 0, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_write_uid(TF_ColorV2 *color_v2, uint32_t uid) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_color_v2_get_response_expected(color_v2, TF_COLOR_V2_FUNCTION_WRITE_UID, &response_expected);
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_WRITE_UID, 4, 0, response_expected);

    uint8_t *send_buf = tf_tfp_get_send_payload_buffer(color_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(send_buf + 0, &uid, 4);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_read_uid(TF_ColorV2 *color_v2, uint32_t *ret_uid) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_READ_UID, 0, 4, response_expected);

    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 4); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}

int tf_color_v2_get_identity(TF_ColorV2 *color_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(hal)->locked) {
        return TF_E_LOCKED;
    }

    bool response_expected = true;
    tf_tfp_prepare_send(color_v2->tfp, TF_COLOR_V2_FUNCTION_GET_IDENTITY, 0, 25, response_expected);

    size_t i;
    uint32_t deadline = tf_hal_current_time_us(hal) + tf_hal_get_common(hal)->timeout;

    uint8_t error_code = 0;
    int result = tf_tfp_send_packet(color_v2->tfp, response_expected, deadline, &error_code);

    if (result < 0) {
        return result;
    }

    if (result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (result & TF_TICK_PACKET_RECEIVED && error_code == 0) {
        TF_PacketBuffer *recv_buf = tf_tfp_get_receive_buffer(color_v2->tfp);
        if (ret_uid != NULL) { tf_packet_buffer_pop_n(recv_buf, (uint8_t *)ret_uid, 8);} else { tf_packet_buffer_remove(recv_buf, 8); }
        if (ret_connected_uid != NULL) { tf_packet_buffer_pop_n(recv_buf, (uint8_t *)ret_connected_uid, 8);} else { tf_packet_buffer_remove(recv_buf, 8); }
        if (ret_position != NULL) { *ret_position = tf_packet_buffer_read_char(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 1); }
        if (ret_hardware_version != NULL) { for (i = 0; i < 3; ++i) ret_hardware_version[i] = tf_packet_buffer_read_uint8_t(recv_buf);} else { tf_packet_buffer_remove(recv_buf, 3); }
        if (ret_firmware_version != NULL) { for (i = 0; i < 3; ++i) ret_firmware_version[i] = tf_packet_buffer_read_uint8_t(recv_buf);} else { tf_packet_buffer_remove(recv_buf, 3); }
        if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packet_buffer_read_uint16_t(recv_buf); } else { tf_packet_buffer_remove(recv_buf, 2); }
        tf_tfp_packet_processed(color_v2->tfp);
    }

    result = tf_tfp_finish_send(color_v2->tfp, result, deadline);

    if (result < 0) {
        return result;
    }

    return tf_tfp_get_error(error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_color_v2_register_color_callback(TF_ColorV2 *color_v2, TF_ColorV2_ColorHandler handler, void *user_data) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    if (handler == NULL) {
        color_v2->tfp->needs_callback_tick = false;
        color_v2->tfp->needs_callback_tick |= color_v2->illuminance_handler != NULL;
        color_v2->tfp->needs_callback_tick |= color_v2->color_temperature_handler != NULL;
    } else {
        color_v2->tfp->needs_callback_tick = true;
    }

    color_v2->color_handler = handler;
    color_v2->color_user_data = user_data;

    return TF_E_OK;
}


int tf_color_v2_register_illuminance_callback(TF_ColorV2 *color_v2, TF_ColorV2_IlluminanceHandler handler, void *user_data) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    if (handler == NULL) {
        color_v2->tfp->needs_callback_tick = false;
        color_v2->tfp->needs_callback_tick |= color_v2->color_handler != NULL;
        color_v2->tfp->needs_callback_tick |= color_v2->color_temperature_handler != NULL;
    } else {
        color_v2->tfp->needs_callback_tick = true;
    }

    color_v2->illuminance_handler = handler;
    color_v2->illuminance_user_data = user_data;

    return TF_E_OK;
}


int tf_color_v2_register_color_temperature_callback(TF_ColorV2 *color_v2, TF_ColorV2_ColorTemperatureHandler handler, void *user_data) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    if (handler == NULL) {
        color_v2->tfp->needs_callback_tick = false;
        color_v2->tfp->needs_callback_tick |= color_v2->color_handler != NULL;
        color_v2->tfp->needs_callback_tick |= color_v2->illuminance_handler != NULL;
    } else {
        color_v2->tfp->needs_callback_tick = true;
    }

    color_v2->color_temperature_handler = handler;
    color_v2->color_temperature_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_color_v2_callback_tick(TF_ColorV2 *color_v2, uint32_t timeout_us) {
    if (color_v2 == NULL) {
        return TF_E_NULL;
    }

    TF_HAL *hal = color_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(color_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
