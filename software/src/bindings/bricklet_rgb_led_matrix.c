/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_rgb_led_matrix.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_rgb_led_matrix_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_RGBLEDMatrix *rgb_led_matrix = (TF_RGBLEDMatrix *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(rgb_led_matrix->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_RGB_LED_MATRIX_CALLBACK_FRAME_STARTED: {
            TF_RGBLEDMatrix_FrameStartedHandler fn = rgb_led_matrix->frame_started_handler;
            void *user_data = rgb_led_matrix->frame_started_user_data;
            if (fn == NULL) {
                return false;
            }

            uint32_t frame_number = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(rgb_led_matrix, frame_number, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_rgb_led_matrix_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_rgb_led_matrix_create(TF_RGBLEDMatrix *rgb_led_matrix, const char *uid_or_port_name, TF_HAL *hal) {
    if (rgb_led_matrix == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(rgb_led_matrix, 0, sizeof(TF_RGBLEDMatrix));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_RGB_LED_MATRIX_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    rgb_led_matrix->tfp = tfp;
    rgb_led_matrix->tfp->device = rgb_led_matrix;
    rgb_led_matrix->tfp->cb_handler = tf_rgb_led_matrix_callback_handler;
    rgb_led_matrix->magic = 0x5446;
    rgb_led_matrix->response_expected[0] = 0x00;
    rgb_led_matrix->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_rgb_led_matrix_destroy(TF_RGBLEDMatrix *rgb_led_matrix) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }
    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rgb_led_matrix->tfp->cb_handler = NULL;
    rgb_led_matrix->tfp->device = NULL;
    rgb_led_matrix->tfp = NULL;
    rgb_led_matrix->magic = 0;

    return TF_E_OK;
}

int tf_rgb_led_matrix_get_response_expected(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t function_id, bool *ret_response_expected) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_RGB_LED_MATRIX_FUNCTION_SET_RED:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_matrix->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_SET_GREEN:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_matrix->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_SET_BLUE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_matrix->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_SET_FRAME_DURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_matrix->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_DRAW_FRAME:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_matrix->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_matrix->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_matrix->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_matrix->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (rgb_led_matrix->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_rgb_led_matrix_set_response_expected(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t function_id, bool response_expected) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_RGB_LED_MATRIX_FUNCTION_SET_RED:
            if (response_expected) {
                rgb_led_matrix->response_expected[0] |= (1 << 0);
            } else {
                rgb_led_matrix->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_SET_GREEN:
            if (response_expected) {
                rgb_led_matrix->response_expected[0] |= (1 << 1);
            } else {
                rgb_led_matrix->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_SET_BLUE:
            if (response_expected) {
                rgb_led_matrix->response_expected[0] |= (1 << 2);
            } else {
                rgb_led_matrix->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_SET_FRAME_DURATION:
            if (response_expected) {
                rgb_led_matrix->response_expected[0] |= (1 << 3);
            } else {
                rgb_led_matrix->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_DRAW_FRAME:
            if (response_expected) {
                rgb_led_matrix->response_expected[0] |= (1 << 4);
            } else {
                rgb_led_matrix->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                rgb_led_matrix->response_expected[0] |= (1 << 5);
            } else {
                rgb_led_matrix->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                rgb_led_matrix->response_expected[0] |= (1 << 6);
            } else {
                rgb_led_matrix->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_RESET:
            if (response_expected) {
                rgb_led_matrix->response_expected[0] |= (1 << 7);
            } else {
                rgb_led_matrix->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_RGB_LED_MATRIX_FUNCTION_WRITE_UID:
            if (response_expected) {
                rgb_led_matrix->response_expected[1] |= (1 << 0);
            } else {
                rgb_led_matrix->response_expected[1] &= ~(1 << 0);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_rgb_led_matrix_set_response_expected_all(TF_RGBLEDMatrix *rgb_led_matrix, bool response_expected) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(rgb_led_matrix->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_rgb_led_matrix_set_red(TF_RGBLEDMatrix *rgb_led_matrix, const uint8_t red[64]) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_matrix_get_response_expected(rgb_led_matrix, TF_RGB_LED_MATRIX_FUNCTION_SET_RED, &_response_expected);
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_SET_RED, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_matrix->tfp);

    memcpy(_send_buf + 0, red, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_get_red(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t ret_red[64]) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_GET_RED, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_red != NULL) { for (_i = 0; _i < 64; ++_i) ret_red[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 64); }
        }
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_set_green(TF_RGBLEDMatrix *rgb_led_matrix, const uint8_t green[64]) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_matrix_get_response_expected(rgb_led_matrix, TF_RGB_LED_MATRIX_FUNCTION_SET_GREEN, &_response_expected);
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_SET_GREEN, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_matrix->tfp);

    memcpy(_send_buf + 0, green, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_get_green(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t ret_green[64]) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_GET_GREEN, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_green != NULL) { for (_i = 0; _i < 64; ++_i) ret_green[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 64); }
        }
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_set_blue(TF_RGBLEDMatrix *rgb_led_matrix, const uint8_t blue[64]) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_matrix_get_response_expected(rgb_led_matrix, TF_RGB_LED_MATRIX_FUNCTION_SET_BLUE, &_response_expected);
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_SET_BLUE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_matrix->tfp);

    memcpy(_send_buf + 0, blue, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_get_blue(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t ret_blue[64]) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_GET_BLUE, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_blue != NULL) { for (_i = 0; _i < 64; ++_i) ret_blue[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 64); }
        }
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_set_frame_duration(TF_RGBLEDMatrix *rgb_led_matrix, uint16_t frame_duration) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_matrix_get_response_expected(rgb_led_matrix, TF_RGB_LED_MATRIX_FUNCTION_SET_FRAME_DURATION, &_response_expected);
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_SET_FRAME_DURATION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_matrix->tfp);

    frame_duration = tf_leconvert_uint16_to(frame_duration); memcpy(_send_buf + 0, &frame_duration, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_get_frame_duration(TF_RGBLEDMatrix *rgb_led_matrix, uint16_t *ret_frame_duration) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_GET_FRAME_DURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_frame_duration != NULL) { *ret_frame_duration = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_draw_frame(TF_RGBLEDMatrix *rgb_led_matrix) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_matrix_get_response_expected(rgb_led_matrix, TF_RGB_LED_MATRIX_FUNCTION_DRAW_FRAME, &_response_expected);
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_DRAW_FRAME, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_get_supply_voltage(TF_RGBLEDMatrix *rgb_led_matrix, uint16_t *ret_voltage) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_GET_SUPPLY_VOLTAGE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_voltage != NULL) { *ret_voltage = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_get_spitfp_error_count(TF_RGBLEDMatrix *rgb_led_matrix, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_set_bootloader_mode(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t mode, uint8_t *ret_status) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_matrix->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_get_bootloader_mode(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t *ret_mode) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_set_write_firmware_pointer(TF_RGBLEDMatrix *rgb_led_matrix, uint32_t pointer) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_matrix_get_response_expected(rgb_led_matrix, TF_RGB_LED_MATRIX_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_matrix->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_write_firmware(TF_RGBLEDMatrix *rgb_led_matrix, const uint8_t data[64], uint8_t *ret_status) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_matrix->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_set_status_led_config(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t config) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_matrix_get_response_expected(rgb_led_matrix, TF_RGB_LED_MATRIX_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_matrix->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_get_status_led_config(TF_RGBLEDMatrix *rgb_led_matrix, uint8_t *ret_config) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_get_chip_temperature(TF_RGBLEDMatrix *rgb_led_matrix, int16_t *ret_temperature) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_reset(TF_RGBLEDMatrix *rgb_led_matrix) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_matrix_get_response_expected(rgb_led_matrix, TF_RGB_LED_MATRIX_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_write_uid(TF_RGBLEDMatrix *rgb_led_matrix, uint32_t uid) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_rgb_led_matrix_get_response_expected(rgb_led_matrix, TF_RGB_LED_MATRIX_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(rgb_led_matrix->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_read_uid(TF_RGBLEDMatrix *rgb_led_matrix, uint32_t *ret_uid) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_rgb_led_matrix_get_identity(TF_RGBLEDMatrix *rgb_led_matrix, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = rgb_led_matrix->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(rgb_led_matrix->tfp, TF_RGB_LED_MATRIX_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(rgb_led_matrix->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(rgb_led_matrix->tfp);
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
        tf_tfp_packet_processed(rgb_led_matrix->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(rgb_led_matrix->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_rgb_led_matrix_register_frame_started_callback(TF_RGBLEDMatrix *rgb_led_matrix, TF_RGBLEDMatrix_FrameStartedHandler handler, void *user_data) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    rgb_led_matrix->frame_started_handler = handler;
    rgb_led_matrix->frame_started_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_rgb_led_matrix_callback_tick(TF_RGBLEDMatrix *rgb_led_matrix, uint32_t timeout_us) {
    if (rgb_led_matrix == NULL) {
        return TF_E_NULL;
    }

    if (rgb_led_matrix->magic != 0x5446 || rgb_led_matrix->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = rgb_led_matrix->tfp->spitfp->hal;

    return tf_tfp_callback_tick(rgb_led_matrix->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
