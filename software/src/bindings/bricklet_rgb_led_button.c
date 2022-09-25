/* ***********************************************************
 * This file was automatically generated on 2022-07-12.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.3         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_rgb_led_button.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_rgb_led_button_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_RGBLEDButton *rgb_led_button = (TF_RGBLEDButton *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(rgb_led_button->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_RGB_LED_BUTTON_CALLBACK_BUTTON_STATE_CHANGED: {
            TF_RGBLEDButton_ButtonStateChangedHandler fn = rgb_led_button->button_state_changed_handler;
            void *user_data = rgb_led_button->button_state_changed_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t state = tf_packet_buffer_read_uint8_t(payload);
            hal_common->locked = true;
            fn(rgb_led_button, state, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_rgb_led_button_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_rgb_led_button_create(TF_RGBLEDButton *rgb_led_button, const char *uid_or_port_name, TF_HAL *hal) {
    if (rgb_led_button == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(rgb_led_button, 0, sizeof(TF_RGBLEDButton));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_RGB_LED_BUTTON_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    rgb_led_button->tfp = tfp;
    rgb_led_button->tfp->device = rgb_led_button;
    rgb_led_button->tfp->cb_handler = tf_rgb_led_button_callback_handler;
    rgb_led_button->magic = 0x5446;
    rgb_led_button->response_expected[0] = 0x00;
    return TF_E_OK;
}

int tf_rgb_led_button_destroy(TF_RGBLEDButton *rgb_led_button) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }
    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rgb_led_button->tfp->cb_handler = NULL;
    rgb_led_button->tfp->device = NULL;
    rgb_led_button->tfp = NULL;
    rgb_led_button->magic = 0;

    return TF_E_OK;
}

int tf_rgb_led_button_get_response_expected(TF_RGBLEDButton *rgb_led_button, uint8_t function_id, bool *ret_response_expected) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_RGB_LED_BUTTON_FUNCTION_SET_COLOR:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_button->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_RGB_LED_BUTTON_FUNCTION_SET_COLOR_CALIBRATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_button->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_RGB_LED_BUTTON_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_button->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_RGB_LED_BUTTON_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_button->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_RGB_LED_BUTTON_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_button->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_RGB_LED_BUTTON_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_button->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_rgb_led_button_set_response_expected(TF_RGBLEDButton *rgb_led_button, uint8_t function_id, bool response_expected) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_RGB_LED_BUTTON_FUNCTION_SET_COLOR:
            if (response_expected) {
                rgb_led_button->response_expected[0] |= (1 << 0);
            } else {
                rgb_led_button->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_RGB_LED_BUTTON_FUNCTION_SET_COLOR_CALIBRATION:
            if (response_expected) {
                rgb_led_button->response_expected[0] |= (1 << 1);
            } else {
                rgb_led_button->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_RGB_LED_BUTTON_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                rgb_led_button->response_expected[0] |= (1 << 2);
            } else {
                rgb_led_button->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_RGB_LED_BUTTON_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                rgb_led_button->response_expected[0] |= (1 << 3);
            } else {
                rgb_led_button->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_RGB_LED_BUTTON_FUNCTION_RESET:
            if (response_expected) {
                rgb_led_button->response_expected[0] |= (1 << 4);
            } else {
                rgb_led_button->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_RGB_LED_BUTTON_FUNCTION_WRITE_UID:
            if (response_expected) {
                rgb_led_button->response_expected[0] |= (1 << 5);
            } else {
                rgb_led_button->response_expected[0] &= ~(1 << 5);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_rgb_led_button_set_response_expected_all(TF_RGBLEDButton *rgb_led_button, bool response_expected) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(rgb_led_button->response_expected, response_expected ? 0xFF : 0, 1);

    return TF_E_OK;
}

int tf_rgb_led_button_set_color(TF_RGBLEDButton *rgb_led_button, uint8_t red, uint8_t green, uint8_t blue) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_button_get_response_expected(rgb_led_button, TF_RGB_LED_BUTTON_FUNCTION_SET_COLOR, &_response_expected);
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_SET_COLOR, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_button->tfp);

    _send_buf[0] = (uint8_t)red;
    _send_buf[1] = (uint8_t)green;
    _send_buf[2] = (uint8_t)blue;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_get_color(TF_RGBLEDButton *rgb_led_button, uint8_t *ret_red, uint8_t *ret_green, uint8_t *ret_blue) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_GET_COLOR, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_button->tfp);
        if (_error_code != 0 || _length != 3) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_red != NULL) { *ret_red = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_green != NULL) { *ret_green = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_blue != NULL) { *ret_blue = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rgb_led_button->tfp);
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 3) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_get_button_state(TF_RGBLEDButton *rgb_led_button, uint8_t *ret_state) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_GET_BUTTON_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_button->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_state != NULL) { *ret_state = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rgb_led_button->tfp);
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_set_color_calibration(TF_RGBLEDButton *rgb_led_button, uint8_t red, uint8_t green, uint8_t blue) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_button_get_response_expected(rgb_led_button, TF_RGB_LED_BUTTON_FUNCTION_SET_COLOR_CALIBRATION, &_response_expected);
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_SET_COLOR_CALIBRATION, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_button->tfp);

    _send_buf[0] = (uint8_t)red;
    _send_buf[1] = (uint8_t)green;
    _send_buf[2] = (uint8_t)blue;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_get_color_calibration(TF_RGBLEDButton *rgb_led_button, uint8_t *ret_red, uint8_t *ret_green, uint8_t *ret_blue) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_GET_COLOR_CALIBRATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_button->tfp);
        if (_error_code != 0 || _length != 3) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_red != NULL) { *ret_red = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_green != NULL) { *ret_green = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_blue != NULL) { *ret_blue = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rgb_led_button->tfp);
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 3) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_get_spitfp_error_count(TF_RGBLEDButton *rgb_led_button, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_button->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(rgb_led_button->tfp);
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_set_bootloader_mode(TF_RGBLEDButton *rgb_led_button, uint8_t mode, uint8_t *ret_status) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_button->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_button->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rgb_led_button->tfp);
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_get_bootloader_mode(TF_RGBLEDButton *rgb_led_button, uint8_t *ret_mode) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_button->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rgb_led_button->tfp);
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_set_write_firmware_pointer(TF_RGBLEDButton *rgb_led_button, uint32_t pointer) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_button_get_response_expected(rgb_led_button, TF_RGB_LED_BUTTON_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_button->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_write_firmware(TF_RGBLEDButton *rgb_led_button, const uint8_t data[64], uint8_t *ret_status) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_button->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_button->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rgb_led_button->tfp);
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_set_status_led_config(TF_RGBLEDButton *rgb_led_button, uint8_t config) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_button_get_response_expected(rgb_led_button, TF_RGB_LED_BUTTON_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_button->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_get_status_led_config(TF_RGBLEDButton *rgb_led_button, uint8_t *ret_config) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_button->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rgb_led_button->tfp);
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_get_chip_temperature(TF_RGBLEDButton *rgb_led_button, int16_t *ret_temperature) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_button->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(rgb_led_button->tfp);
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_reset(TF_RGBLEDButton *rgb_led_button) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_button_get_response_expected(rgb_led_button, TF_RGB_LED_BUTTON_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_write_uid(TF_RGBLEDButton *rgb_led_button, uint32_t uid) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_button_get_response_expected(rgb_led_button, TF_RGB_LED_BUTTON_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_button->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_read_uid(TF_RGBLEDButton *rgb_led_button, uint32_t *ret_uid) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_button->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(rgb_led_button->tfp);
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_button_get_identity(TF_RGBLEDButton *rgb_led_button, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_button->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_button->tfp, TF_RGB_LED_BUTTON_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_button->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_button->tfp);
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
        tf_tfp_packet_processed(rgb_led_button->tfp);
    }

    _result = tf_tfp_finish_send(rgb_led_button->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_rgb_led_button_register_button_state_changed_callback(TF_RGBLEDButton *rgb_led_button, TF_RGBLEDButton_ButtonStateChangedHandler handler, void *user_data) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rgb_led_button->button_state_changed_handler = handler;
    rgb_led_button->button_state_changed_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_rgb_led_button_callback_tick(TF_RGBLEDButton *rgb_led_button, uint32_t timeout_us) {
    if (rgb_led_button == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_button->magic != 0x5446 || rgb_led_button->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = rgb_led_button->tfp->spitfp->hal;

    return tf_tfp_callback_tick(rgb_led_button->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
