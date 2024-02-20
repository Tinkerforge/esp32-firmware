/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_oled_128x64_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


static bool tf_oled_128x64_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    (void)device;
    (void)fid;
    (void)payload;

    return false;
}
int tf_oled_128x64_v2_create(TF_OLED128x64V2 *oled_128x64_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (oled_128x64_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(oled_128x64_v2, 0, sizeof(TF_OLED128x64V2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_OLED_128X64_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    oled_128x64_v2->tfp = tfp;
    oled_128x64_v2->tfp->device = oled_128x64_v2;
    oled_128x64_v2->tfp->cb_handler = tf_oled_128x64_v2_callback_handler;
    oled_128x64_v2->magic = 0x5446;
    oled_128x64_v2->response_expected[0] = 0x01;
    oled_128x64_v2->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_oled_128x64_v2_destroy(TF_OLED128x64V2 *oled_128x64_v2) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }
    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    oled_128x64_v2->tfp->cb_handler = NULL;
    oled_128x64_v2->tfp->device = NULL;
    oled_128x64_v2->tfp = NULL;
    oled_128x64_v2->magic = 0;

    return TF_E_OK;
}

int tf_oled_128x64_v2_get_response_expected(TF_OLED128x64V2 *oled_128x64_v2, uint8_t function_id, bool *ret_response_expected) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_OLED_128X64_V2_FUNCTION_WRITE_PIXELS_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (oled_128x64_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_CLEAR_DISPLAY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (oled_128x64_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_SET_DISPLAY_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (oled_128x64_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_WRITE_LINE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (oled_128x64_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_DRAW_BUFFERED_FRAME:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (oled_128x64_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (oled_128x64_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (oled_128x64_v2->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (oled_128x64_v2->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (oled_128x64_v2->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_oled_128x64_v2_set_response_expected(TF_OLED128x64V2 *oled_128x64_v2, uint8_t function_id, bool response_expected) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_OLED_128X64_V2_FUNCTION_WRITE_PIXELS_LOW_LEVEL:
            if (response_expected) {
                oled_128x64_v2->response_expected[0] |= (1 << 0);
            } else {
                oled_128x64_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_CLEAR_DISPLAY:
            if (response_expected) {
                oled_128x64_v2->response_expected[0] |= (1 << 1);
            } else {
                oled_128x64_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_SET_DISPLAY_CONFIGURATION:
            if (response_expected) {
                oled_128x64_v2->response_expected[0] |= (1 << 2);
            } else {
                oled_128x64_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_WRITE_LINE:
            if (response_expected) {
                oled_128x64_v2->response_expected[0] |= (1 << 3);
            } else {
                oled_128x64_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_DRAW_BUFFERED_FRAME:
            if (response_expected) {
                oled_128x64_v2->response_expected[0] |= (1 << 4);
            } else {
                oled_128x64_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                oled_128x64_v2->response_expected[0] |= (1 << 5);
            } else {
                oled_128x64_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                oled_128x64_v2->response_expected[0] |= (1 << 6);
            } else {
                oled_128x64_v2->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_RESET:
            if (response_expected) {
                oled_128x64_v2->response_expected[0] |= (1 << 7);
            } else {
                oled_128x64_v2->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_OLED_128X64_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                oled_128x64_v2->response_expected[1] |= (1 << 0);
            } else {
                oled_128x64_v2->response_expected[1] &= ~(1 << 0);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_oled_128x64_v2_set_response_expected_all(TF_OLED128x64V2 *oled_128x64_v2, bool response_expected) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(oled_128x64_v2->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_oled_128x64_v2_write_pixels_low_level(TF_OLED128x64V2 *oled_128x64_v2, uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end, uint16_t pixels_length, uint16_t pixels_chunk_offset, const bool pixels_chunk_data[448]) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_oled_128x64_v2_get_response_expected(oled_128x64_v2, TF_OLED_128X64_V2_FUNCTION_WRITE_PIXELS_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_WRITE_PIXELS_LOW_LEVEL, 64, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(oled_128x64_v2->tfp);

    _send_buf[0] = (uint8_t)x_start;
    _send_buf[1] = (uint8_t)y_start;
    _send_buf[2] = (uint8_t)x_end;
    _send_buf[3] = (uint8_t)y_end;
    pixels_length = tf_leconvert_uint16_to(pixels_length); memcpy(_send_buf + 4, &pixels_length, 2);
    pixels_chunk_offset = tf_leconvert_uint16_to(pixels_chunk_offset); memcpy(_send_buf + 6, &pixels_chunk_offset, 2);
    memset(_send_buf + 8, 0, 56); for (_i = 0; _i < 448; ++_i) _send_buf[8 + (_i / 8)] |= (pixels_chunk_data[_i] ? 1 : 0) << (_i % 8);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_read_pixels_low_level(TF_OLED128x64V2 *oled_128x64_v2, uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end, uint16_t *ret_pixels_length, uint16_t *ret_pixels_chunk_offset, bool ret_pixels_chunk_data[480]) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_READ_PIXELS_LOW_LEVEL, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(oled_128x64_v2->tfp);

    _send_buf[0] = (uint8_t)x_start;
    _send_buf[1] = (uint8_t)y_start;
    _send_buf[2] = (uint8_t)x_end;
    _send_buf[3] = (uint8_t)y_end;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(oled_128x64_v2->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_pixels_length != NULL) { *ret_pixels_length = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_pixels_chunk_offset != NULL) { *ret_pixels_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_pixels_chunk_data != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_pixels_chunk_data, 480);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_clear_display(TF_OLED128x64V2 *oled_128x64_v2) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_oled_128x64_v2_get_response_expected(oled_128x64_v2, TF_OLED_128X64_V2_FUNCTION_CLEAR_DISPLAY, &_response_expected);
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_CLEAR_DISPLAY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_set_display_configuration(TF_OLED128x64V2 *oled_128x64_v2, uint8_t contrast, bool invert, bool automatic_draw) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_oled_128x64_v2_get_response_expected(oled_128x64_v2, TF_OLED_128X64_V2_FUNCTION_SET_DISPLAY_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_SET_DISPLAY_CONFIGURATION, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(oled_128x64_v2->tfp);

    _send_buf[0] = (uint8_t)contrast;
    _send_buf[1] = invert ? 1 : 0;
    _send_buf[2] = automatic_draw ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_get_display_configuration(TF_OLED128x64V2 *oled_128x64_v2, uint8_t *ret_contrast, bool *ret_invert, bool *ret_automatic_draw) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_GET_DISPLAY_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(oled_128x64_v2->tfp);
        if (_error_code != 0 || _length != 3) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_contrast != NULL) { *ret_contrast = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_invert != NULL) { *ret_invert = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_automatic_draw != NULL) { *ret_automatic_draw = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 3) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_write_line(TF_OLED128x64V2 *oled_128x64_v2, uint8_t line, uint8_t position, const char *text) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_oled_128x64_v2_get_response_expected(oled_128x64_v2, TF_OLED_128X64_V2_FUNCTION_WRITE_LINE, &_response_expected);
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_WRITE_LINE, 24, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(oled_128x64_v2->tfp);

    _send_buf[0] = (uint8_t)line;
    _send_buf[1] = (uint8_t)position;
    strncpy((char *)(_send_buf + 2), text, 22);


    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_draw_buffered_frame(TF_OLED128x64V2 *oled_128x64_v2, bool force_complete_redraw) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_oled_128x64_v2_get_response_expected(oled_128x64_v2, TF_OLED_128X64_V2_FUNCTION_DRAW_BUFFERED_FRAME, &_response_expected);
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_DRAW_BUFFERED_FRAME, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(oled_128x64_v2->tfp);

    _send_buf[0] = force_complete_redraw ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_get_spitfp_error_count(TF_OLED128x64V2 *oled_128x64_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(oled_128x64_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_set_bootloader_mode(TF_OLED128x64V2 *oled_128x64_v2, uint8_t mode, uint8_t *ret_status) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(oled_128x64_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(oled_128x64_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_get_bootloader_mode(TF_OLED128x64V2 *oled_128x64_v2, uint8_t *ret_mode) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(oled_128x64_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_set_write_firmware_pointer(TF_OLED128x64V2 *oled_128x64_v2, uint32_t pointer) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_oled_128x64_v2_get_response_expected(oled_128x64_v2, TF_OLED_128X64_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(oled_128x64_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_write_firmware(TF_OLED128x64V2 *oled_128x64_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(oled_128x64_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(oled_128x64_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_set_status_led_config(TF_OLED128x64V2 *oled_128x64_v2, uint8_t config) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_oled_128x64_v2_get_response_expected(oled_128x64_v2, TF_OLED_128X64_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(oled_128x64_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_get_status_led_config(TF_OLED128x64V2 *oled_128x64_v2, uint8_t *ret_config) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(oled_128x64_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_get_chip_temperature(TF_OLED128x64V2 *oled_128x64_v2, int16_t *ret_temperature) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(oled_128x64_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_reset(TF_OLED128x64V2 *oled_128x64_v2) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_oled_128x64_v2_get_response_expected(oled_128x64_v2, TF_OLED_128X64_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_write_uid(TF_OLED128x64V2 *oled_128x64_v2, uint32_t uid) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_oled_128x64_v2_get_response_expected(oled_128x64_v2, TF_OLED_128X64_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(oled_128x64_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_read_uid(TF_OLED128x64V2 *oled_128x64_v2, uint32_t *ret_uid) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(oled_128x64_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_oled_128x64_v2_get_identity(TF_OLED128x64V2 *oled_128x64_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = oled_128x64_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(oled_128x64_v2->tfp, TF_OLED_128X64_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(oled_128x64_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(oled_128x64_v2->tfp);
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
        tf_tfp_packet_processed(oled_128x64_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(oled_128x64_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

typedef struct TF_OLED128x64V2_WritePixelsLLWrapperData {
    uint8_t x_start;
    uint8_t y_start;
    uint8_t x_end;
    uint8_t y_end;
} TF_OLED128x64V2_WritePixelsLLWrapperData;


static int tf_oled_128x64_v2_write_pixels_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    TF_OLED128x64V2_WritePixelsLLWrapperData *data = (TF_OLED128x64V2_WritePixelsLLWrapperData *) wrapper_data;
    uint16_t pixels_chunk_offset = (uint16_t)chunk_offset;
    uint16_t pixels_length = (uint16_t)stream_length;
    uint32_t pixels_chunk_written = 448;

    bool *pixels_chunk_data = (bool *) chunk_data;
    int ret = tf_oled_128x64_v2_write_pixels_low_level((TF_OLED128x64V2 *)device, data->x_start, data->y_start, data->x_end, data->y_end, pixels_length, pixels_chunk_offset, pixels_chunk_data);

    *ret_chunk_written = (uint32_t) pixels_chunk_written;
    return ret;
}

int tf_oled_128x64_v2_write_pixels(TF_OLED128x64V2 *oled_128x64_v2, uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end, const bool *pixels, uint16_t pixels_length) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_OLED128x64V2_WritePixelsLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.x_start = x_start;
    _wrapper_data.y_start = y_start;
    _wrapper_data.x_end = x_end;
    _wrapper_data.y_end = y_end;

    uint32_t _stream_length = pixels_length;
    uint32_t _pixels_written = 0;
    bool _chunk_data[448];

    int ret = tf_stream_in(oled_128x64_v2, tf_oled_128x64_v2_write_pixels_ll_wrapper, &_wrapper_data, pixels, _stream_length, _chunk_data, &_pixels_written, 448, tf_copy_items_bool);



    return ret;
}


typedef struct TF_OLED128x64V2_ReadPixelsLLWrapperData {
    uint8_t x_start;
    uint8_t y_start;
    uint8_t x_end;
    uint8_t y_end;
} TF_OLED128x64V2_ReadPixelsLLWrapperData;


static int tf_oled_128x64_v2_read_pixels_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    TF_OLED128x64V2_ReadPixelsLLWrapperData *data = (TF_OLED128x64V2_ReadPixelsLLWrapperData *) wrapper_data;
    uint16_t pixels_length = 0;
    uint16_t pixels_chunk_offset = 0;
    bool *pixels_chunk_data = (bool *) chunk_data;
    int ret = tf_oled_128x64_v2_read_pixels_low_level((TF_OLED128x64V2 *)device, data->x_start, data->y_start, data->x_end, data->y_end, &pixels_length, &pixels_chunk_offset, pixels_chunk_data);

    *ret_stream_length = (uint32_t)pixels_length;
    *ret_chunk_offset = (uint32_t)pixels_chunk_offset;
    return ret;
}

int tf_oled_128x64_v2_read_pixels(TF_OLED128x64V2 *oled_128x64_v2, uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end, bool *ret_pixels, uint16_t *ret_pixels_length) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_OLED128x64V2_ReadPixelsLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.x_start = x_start;
    _wrapper_data.y_start = y_start;
    _wrapper_data.x_end = x_end;
    _wrapper_data.y_end = y_end;
    uint32_t _pixels_length = 0;
    bool _pixels_chunk_data[480];

    int ret = tf_stream_out(oled_128x64_v2, tf_oled_128x64_v2_read_pixels_ll_wrapper, &_wrapper_data, ret_pixels, &_pixels_length, _pixels_chunk_data, 480, tf_copy_items_bool);

    if (ret_pixels_length != NULL) {
        *ret_pixels_length = (uint16_t)_pixels_length;
    }
    return ret;
}


int tf_oled_128x64_v2_callback_tick(TF_OLED128x64V2 *oled_128x64_v2, uint32_t timeout_us) {
    if (oled_128x64_v2 == NULL) {
        return TF_E_NULL;
    }

    if (oled_128x64_v2->magic != 0x5446 || oled_128x64_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = oled_128x64_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(oled_128x64_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
