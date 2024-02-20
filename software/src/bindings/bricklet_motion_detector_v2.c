/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_motion_detector_v2.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_motion_detector_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_MotionDetectorV2 *motion_detector_v2 = (TF_MotionDetectorV2 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(motion_detector_v2->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_MOTION_DETECTOR_V2_CALLBACK_MOTION_DETECTED: {
            TF_MotionDetectorV2_MotionDetectedHandler fn = motion_detector_v2->motion_detected_handler;
            void *user_data = motion_detector_v2->motion_detected_user_data;
            if (fn == NULL) {
                return false;
            }


            hal_common->locked = true;
            fn(motion_detector_v2, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_MOTION_DETECTOR_V2_CALLBACK_DETECTION_CYCLE_ENDED: {
            TF_MotionDetectorV2_DetectionCycleEndedHandler fn = motion_detector_v2->detection_cycle_ended_handler;
            void *user_data = motion_detector_v2->detection_cycle_ended_user_data;
            if (fn == NULL) {
                return false;
            }


            hal_common->locked = true;
            fn(motion_detector_v2, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_motion_detector_v2_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_motion_detector_v2_create(TF_MotionDetectorV2 *motion_detector_v2, const char *uid_or_port_name, TF_HAL *hal) {
    if (motion_detector_v2 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(motion_detector_v2, 0, sizeof(TF_MotionDetectorV2));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_MOTION_DETECTOR_V2_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    motion_detector_v2->tfp = tfp;
    motion_detector_v2->tfp->device = motion_detector_v2;
    motion_detector_v2->tfp->cb_handler = tf_motion_detector_v2_callback_handler;
    motion_detector_v2->magic = 0x5446;
    motion_detector_v2->response_expected[0] = 0x00;
    return TF_E_OK;
}

int tf_motion_detector_v2_destroy(TF_MotionDetectorV2 *motion_detector_v2) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }
    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    motion_detector_v2->tfp->cb_handler = NULL;
    motion_detector_v2->tfp->device = NULL;
    motion_detector_v2->tfp = NULL;
    motion_detector_v2->magic = 0;

    return TF_E_OK;
}

int tf_motion_detector_v2_get_response_expected(TF_MotionDetectorV2 *motion_detector_v2, uint8_t function_id, bool *ret_response_expected) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_MOTION_DETECTOR_V2_FUNCTION_SET_SENSITIVITY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motion_detector_v2->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_MOTION_DETECTOR_V2_FUNCTION_SET_INDICATOR:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motion_detector_v2->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_MOTION_DETECTOR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motion_detector_v2->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_MOTION_DETECTOR_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motion_detector_v2->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_MOTION_DETECTOR_V2_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motion_detector_v2->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_MOTION_DETECTOR_V2_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (motion_detector_v2->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_motion_detector_v2_set_response_expected(TF_MotionDetectorV2 *motion_detector_v2, uint8_t function_id, bool response_expected) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_MOTION_DETECTOR_V2_FUNCTION_SET_SENSITIVITY:
            if (response_expected) {
                motion_detector_v2->response_expected[0] |= (1 << 0);
            } else {
                motion_detector_v2->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_MOTION_DETECTOR_V2_FUNCTION_SET_INDICATOR:
            if (response_expected) {
                motion_detector_v2->response_expected[0] |= (1 << 1);
            } else {
                motion_detector_v2->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_MOTION_DETECTOR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                motion_detector_v2->response_expected[0] |= (1 << 2);
            } else {
                motion_detector_v2->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_MOTION_DETECTOR_V2_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                motion_detector_v2->response_expected[0] |= (1 << 3);
            } else {
                motion_detector_v2->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_MOTION_DETECTOR_V2_FUNCTION_RESET:
            if (response_expected) {
                motion_detector_v2->response_expected[0] |= (1 << 4);
            } else {
                motion_detector_v2->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_MOTION_DETECTOR_V2_FUNCTION_WRITE_UID:
            if (response_expected) {
                motion_detector_v2->response_expected[0] |= (1 << 5);
            } else {
                motion_detector_v2->response_expected[0] &= ~(1 << 5);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_motion_detector_v2_set_response_expected_all(TF_MotionDetectorV2 *motion_detector_v2, bool response_expected) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(motion_detector_v2->response_expected, response_expected ? 0xFF : 0, 1);

    return TF_E_OK;
}

int tf_motion_detector_v2_get_motion_detected(TF_MotionDetectorV2 *motion_detector_v2, uint8_t *ret_motion) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_GET_MOTION_DETECTED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(motion_detector_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_motion != NULL) { *ret_motion = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_set_sensitivity(TF_MotionDetectorV2 *motion_detector_v2, uint8_t sensitivity) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_motion_detector_v2_get_response_expected(motion_detector_v2, TF_MOTION_DETECTOR_V2_FUNCTION_SET_SENSITIVITY, &_response_expected);
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_SET_SENSITIVITY, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(motion_detector_v2->tfp);

    _send_buf[0] = (uint8_t)sensitivity;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_get_sensitivity(TF_MotionDetectorV2 *motion_detector_v2, uint8_t *ret_sensitivity) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_GET_SENSITIVITY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(motion_detector_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_sensitivity != NULL) { *ret_sensitivity = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_set_indicator(TF_MotionDetectorV2 *motion_detector_v2, uint8_t top_left, uint8_t top_right, uint8_t bottom) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_motion_detector_v2_get_response_expected(motion_detector_v2, TF_MOTION_DETECTOR_V2_FUNCTION_SET_INDICATOR, &_response_expected);
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_SET_INDICATOR, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(motion_detector_v2->tfp);

    _send_buf[0] = (uint8_t)top_left;
    _send_buf[1] = (uint8_t)top_right;
    _send_buf[2] = (uint8_t)bottom;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_get_indicator(TF_MotionDetectorV2 *motion_detector_v2, uint8_t *ret_top_left, uint8_t *ret_top_right, uint8_t *ret_bottom) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_GET_INDICATOR, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(motion_detector_v2->tfp);
        if (_error_code != 0 || _length != 3) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_top_left != NULL) { *ret_top_left = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_top_right != NULL) { *ret_top_right = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_bottom != NULL) { *ret_bottom = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 3) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_get_spitfp_error_count(TF_MotionDetectorV2 *motion_detector_v2, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(motion_detector_v2->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_set_bootloader_mode(TF_MotionDetectorV2 *motion_detector_v2, uint8_t mode, uint8_t *ret_status) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(motion_detector_v2->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(motion_detector_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_get_bootloader_mode(TF_MotionDetectorV2 *motion_detector_v2, uint8_t *ret_mode) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(motion_detector_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_set_write_firmware_pointer(TF_MotionDetectorV2 *motion_detector_v2, uint32_t pointer) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_motion_detector_v2_get_response_expected(motion_detector_v2, TF_MOTION_DETECTOR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(motion_detector_v2->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_write_firmware(TF_MotionDetectorV2 *motion_detector_v2, const uint8_t data[64], uint8_t *ret_status) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(motion_detector_v2->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(motion_detector_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_set_status_led_config(TF_MotionDetectorV2 *motion_detector_v2, uint8_t config) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_motion_detector_v2_get_response_expected(motion_detector_v2, TF_MOTION_DETECTOR_V2_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(motion_detector_v2->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_get_status_led_config(TF_MotionDetectorV2 *motion_detector_v2, uint8_t *ret_config) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(motion_detector_v2->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_get_chip_temperature(TF_MotionDetectorV2 *motion_detector_v2, int16_t *ret_temperature) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(motion_detector_v2->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_reset(TF_MotionDetectorV2 *motion_detector_v2) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_motion_detector_v2_get_response_expected(motion_detector_v2, TF_MOTION_DETECTOR_V2_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_write_uid(TF_MotionDetectorV2 *motion_detector_v2, uint32_t uid) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_motion_detector_v2_get_response_expected(motion_detector_v2, TF_MOTION_DETECTOR_V2_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(motion_detector_v2->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_read_uid(TF_MotionDetectorV2 *motion_detector_v2, uint32_t *ret_uid) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(motion_detector_v2->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_motion_detector_v2_get_identity(TF_MotionDetectorV2 *motion_detector_v2, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = motion_detector_v2->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(motion_detector_v2->tfp, TF_MOTION_DETECTOR_V2_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(motion_detector_v2->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(motion_detector_v2->tfp);
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
        tf_tfp_packet_processed(motion_detector_v2->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(motion_detector_v2->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_motion_detector_v2_register_motion_detected_callback(TF_MotionDetectorV2 *motion_detector_v2, TF_MotionDetectorV2_MotionDetectedHandler handler, void *user_data) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    motion_detector_v2->motion_detected_handler = handler;
    motion_detector_v2->motion_detected_user_data = user_data;

    return TF_E_OK;
}


int tf_motion_detector_v2_register_detection_cycle_ended_callback(TF_MotionDetectorV2 *motion_detector_v2, TF_MotionDetectorV2_DetectionCycleEndedHandler handler, void *user_data) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    motion_detector_v2->detection_cycle_ended_handler = handler;
    motion_detector_v2->detection_cycle_ended_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_motion_detector_v2_callback_tick(TF_MotionDetectorV2 *motion_detector_v2, uint32_t timeout_us) {
    if (motion_detector_v2 == NULL) {
        return TF_E_NULL;
    }

    if (motion_detector_v2->magic != 0x5446 || motion_detector_v2->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = motion_detector_v2->tfp->spitfp->hal;

    return tf_tfp_callback_tick(motion_detector_v2->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
