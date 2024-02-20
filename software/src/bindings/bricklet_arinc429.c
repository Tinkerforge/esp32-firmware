/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_arinc429.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_arinc429_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_ARINC429 *arinc429 = (TF_ARINC429 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(arinc429->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_ARINC429_CALLBACK_HEARTBEAT_MESSAGE: {
            TF_ARINC429_HeartbeatMessageHandler fn = arinc429->heartbeat_message_handler;
            void *user_data = arinc429->heartbeat_message_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t channel = tf_packet_buffer_read_uint8_t(payload);
            uint8_t status = tf_packet_buffer_read_uint8_t(payload);
            uint8_t seq_number = tf_packet_buffer_read_uint8_t(payload);
            uint16_t timestamp = tf_packet_buffer_read_uint16_t(payload);
            uint16_t frames_processed = tf_packet_buffer_read_uint16_t(payload);
            uint16_t frames_lost = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(arinc429, channel, status, seq_number, timestamp, frames_processed, frames_lost, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_ARINC429_CALLBACK_FRAME_MESSAGE: {
            TF_ARINC429_FrameMessageHandler fn = arinc429->frame_message_handler;
            void *user_data = arinc429->frame_message_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t channel = tf_packet_buffer_read_uint8_t(payload);
            uint8_t status = tf_packet_buffer_read_uint8_t(payload);
            uint8_t seq_number = tf_packet_buffer_read_uint8_t(payload);
            uint16_t timestamp = tf_packet_buffer_read_uint16_t(payload);
            uint32_t frame = tf_packet_buffer_read_uint32_t(payload);
            uint16_t age = tf_packet_buffer_read_uint16_t(payload);
            hal_common->locked = true;
            fn(arinc429, channel, status, seq_number, timestamp, frame, age, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_ARINC429_CALLBACK_SCHEDULER_MESSAGE: {
            TF_ARINC429_SchedulerMessageHandler fn = arinc429->scheduler_message_handler;
            void *user_data = arinc429->scheduler_message_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t channel = tf_packet_buffer_read_uint8_t(payload);
            uint8_t status = tf_packet_buffer_read_uint8_t(payload);
            uint8_t seq_number = tf_packet_buffer_read_uint8_t(payload);
            uint16_t timestamp = tf_packet_buffer_read_uint16_t(payload);
            uint8_t userdata = tf_packet_buffer_read_uint8_t(payload);
            hal_common->locked = true;
            fn(arinc429, channel, status, seq_number, timestamp, userdata, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_arinc429_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_arinc429_create(TF_ARINC429 *arinc429, const char *uid_or_port_name, TF_HAL *hal) {
    if (arinc429 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(arinc429, 0, sizeof(TF_ARINC429));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_ARINC429_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    arinc429->tfp = tfp;
    arinc429->tfp->device = arinc429;
    arinc429->tfp->cb_handler = tf_arinc429_callback_handler;
    arinc429->magic = 0x5446;
    arinc429->response_expected[0] = 0x21;
    arinc429->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_arinc429_destroy(TF_ARINC429 *arinc429) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }
    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    arinc429->tfp->cb_handler = NULL;
    arinc429->tfp->device = NULL;
    arinc429->tfp = NULL;
    arinc429->magic = 0;

    return TF_E_OK;
}

int tf_arinc429_get_response_expected(TF_ARINC429 *arinc429, uint8_t function_id, bool *ret_response_expected) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_ARINC429_FUNCTION_SET_HEARTBEAT_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_SET_CHANNEL_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_SET_CHANNEL_MODE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_CLEAR_ALL_RX_FILTERS:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_SET_RX_STANDARD_FILTERS:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_SET_RX_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_WRITE_FRAME_DIRECT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_WRITE_FRAME_SCHEDULED:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_CLEAR_SCHEDULE_ENTRIES:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_SET_SCHEDULE_ENTRY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_RESTART:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_SET_FRAME_MODE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[1] & (1 << 4)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[1] & (1 << 5)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[1] & (1 << 6)) != 0;
            }
            break;
        case TF_ARINC429_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (arinc429->response_expected[1] & (1 << 7)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_arinc429_set_response_expected(TF_ARINC429 *arinc429, uint8_t function_id, bool response_expected) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_ARINC429_FUNCTION_SET_HEARTBEAT_CALLBACK_CONFIGURATION:
            if (response_expected) {
                arinc429->response_expected[0] |= (1 << 0);
            } else {
                arinc429->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_ARINC429_FUNCTION_SET_CHANNEL_CONFIGURATION:
            if (response_expected) {
                arinc429->response_expected[0] |= (1 << 1);
            } else {
                arinc429->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_ARINC429_FUNCTION_SET_CHANNEL_MODE:
            if (response_expected) {
                arinc429->response_expected[0] |= (1 << 2);
            } else {
                arinc429->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_ARINC429_FUNCTION_CLEAR_ALL_RX_FILTERS:
            if (response_expected) {
                arinc429->response_expected[0] |= (1 << 3);
            } else {
                arinc429->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_ARINC429_FUNCTION_SET_RX_STANDARD_FILTERS:
            if (response_expected) {
                arinc429->response_expected[0] |= (1 << 4);
            } else {
                arinc429->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_ARINC429_FUNCTION_SET_RX_CALLBACK_CONFIGURATION:
            if (response_expected) {
                arinc429->response_expected[0] |= (1 << 5);
            } else {
                arinc429->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_ARINC429_FUNCTION_WRITE_FRAME_DIRECT:
            if (response_expected) {
                arinc429->response_expected[0] |= (1 << 6);
            } else {
                arinc429->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_ARINC429_FUNCTION_WRITE_FRAME_SCHEDULED:
            if (response_expected) {
                arinc429->response_expected[0] |= (1 << 7);
            } else {
                arinc429->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_ARINC429_FUNCTION_CLEAR_SCHEDULE_ENTRIES:
            if (response_expected) {
                arinc429->response_expected[1] |= (1 << 0);
            } else {
                arinc429->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_ARINC429_FUNCTION_SET_SCHEDULE_ENTRY:
            if (response_expected) {
                arinc429->response_expected[1] |= (1 << 1);
            } else {
                arinc429->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_ARINC429_FUNCTION_RESTART:
            if (response_expected) {
                arinc429->response_expected[1] |= (1 << 2);
            } else {
                arinc429->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_ARINC429_FUNCTION_SET_FRAME_MODE:
            if (response_expected) {
                arinc429->response_expected[1] |= (1 << 3);
            } else {
                arinc429->response_expected[1] &= ~(1 << 3);
            }
            break;
        case TF_ARINC429_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                arinc429->response_expected[1] |= (1 << 4);
            } else {
                arinc429->response_expected[1] &= ~(1 << 4);
            }
            break;
        case TF_ARINC429_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                arinc429->response_expected[1] |= (1 << 5);
            } else {
                arinc429->response_expected[1] &= ~(1 << 5);
            }
            break;
        case TF_ARINC429_FUNCTION_RESET:
            if (response_expected) {
                arinc429->response_expected[1] |= (1 << 6);
            } else {
                arinc429->response_expected[1] &= ~(1 << 6);
            }
            break;
        case TF_ARINC429_FUNCTION_WRITE_UID:
            if (response_expected) {
                arinc429->response_expected[1] |= (1 << 7);
            } else {
                arinc429->response_expected[1] &= ~(1 << 7);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_arinc429_set_response_expected_all(TF_ARINC429 *arinc429, bool response_expected) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(arinc429->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_arinc429_get_capabilities(TF_ARINC429 *arinc429, uint16_t *ret_tx_total_scheduler_jobs, uint16_t *ret_tx_used_scheduler_jobs, uint16_t *ret_rx_total_frame_filters, uint16_t ret_rx_used_frame_filters[2]) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_GET_CAPABILITIES, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 10) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_tx_total_scheduler_jobs != NULL) { *ret_tx_total_scheduler_jobs = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_tx_used_scheduler_jobs != NULL) { *ret_tx_used_scheduler_jobs = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_rx_total_frame_filters != NULL) { *ret_rx_total_frame_filters = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_rx_used_frame_filters != NULL) { for (_i = 0; _i < 2; ++_i) ret_rx_used_frame_filters[_i] = tf_packet_buffer_read_uint16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 10) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_set_heartbeat_callback_configuration(TF_ARINC429 *arinc429, uint8_t channel, bool enabled, bool value_has_to_change, uint16_t period) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_SET_HEARTBEAT_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_SET_HEARTBEAT_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = enabled ? 1 : 0;
    _send_buf[2] = value_has_to_change ? 1 : 0;
    period = tf_leconvert_uint16_to(period); memcpy(_send_buf + 3, &period, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_get_heartbeat_callback_configuration(TF_ARINC429 *arinc429, uint8_t channel, bool *ret_enabled, bool *ret_value_has_to_change, uint16_t *ret_period) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_GET_HEARTBEAT_CALLBACK_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_set_channel_configuration(TF_ARINC429 *arinc429, uint8_t channel, uint8_t parity, uint8_t speed) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_SET_CHANNEL_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_SET_CHANNEL_CONFIGURATION, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)parity;
    _send_buf[2] = (uint8_t)speed;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_get_channel_configuration(TF_ARINC429 *arinc429, uint8_t channel, uint8_t *ret_parity, uint8_t *ret_speed) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_GET_CHANNEL_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_parity != NULL) { *ret_parity = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_speed != NULL) { *ret_speed = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_set_channel_mode(TF_ARINC429 *arinc429, uint8_t channel, uint8_t mode) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_SET_CHANNEL_MODE, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_SET_CHANNEL_MODE, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_get_channel_mode(TF_ARINC429 *arinc429, uint8_t channel, uint8_t *ret_mode) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_GET_CHANNEL_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_clear_all_rx_filters(TF_ARINC429 *arinc429, uint8_t channel) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_CLEAR_ALL_RX_FILTERS, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_CLEAR_ALL_RX_FILTERS, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_clear_rx_filter(TF_ARINC429 *arinc429, uint8_t channel, uint8_t label, uint8_t sdi, bool *ret_success) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_CLEAR_RX_FILTER, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)label;
    _send_buf[2] = (uint8_t)sdi;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_success != NULL) { *ret_success = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_set_rx_standard_filters(TF_ARINC429 *arinc429, uint8_t channel) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_SET_RX_STANDARD_FILTERS, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_SET_RX_STANDARD_FILTERS, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_set_rx_filter(TF_ARINC429 *arinc429, uint8_t channel, uint8_t label, uint8_t sdi, bool *ret_success) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_SET_RX_FILTER, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)label;
    _send_buf[2] = (uint8_t)sdi;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_success != NULL) { *ret_success = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_get_rx_filter(TF_ARINC429 *arinc429, uint8_t channel, uint8_t label, uint8_t sdi, bool *ret_configured) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_GET_RX_FILTER, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)label;
    _send_buf[2] = (uint8_t)sdi;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_configured != NULL) { *ret_configured = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_read_frame(TF_ARINC429 *arinc429, uint8_t channel, uint8_t label, uint8_t sdi, bool *ret_status, uint32_t *ret_frame, uint16_t *ret_age) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_READ_FRAME, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = (uint8_t)label;
    _send_buf[2] = (uint8_t)sdi;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 7) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_frame != NULL) { *ret_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_age != NULL) { *ret_age = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 7) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_set_rx_callback_configuration(TF_ARINC429 *arinc429, uint8_t channel, bool enabled, bool value_has_to_change, uint16_t timeout) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_SET_RX_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_SET_RX_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = enabled ? 1 : 0;
    _send_buf[2] = value_has_to_change ? 1 : 0;
    timeout = tf_leconvert_uint16_to(timeout); memcpy(_send_buf + 3, &timeout, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_get_rx_callback_configuration(TF_ARINC429 *arinc429, uint8_t channel, bool *ret_enabled, bool *ret_value_has_to_change, uint16_t *ret_timeout) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_GET_RX_CALLBACK_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_enabled != NULL) { *ret_enabled = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_timeout != NULL) { *ret_timeout = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_write_frame_direct(TF_ARINC429 *arinc429, uint8_t channel, uint32_t frame) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_WRITE_FRAME_DIRECT, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_WRITE_FRAME_DIRECT, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    frame = tf_leconvert_uint32_to(frame); memcpy(_send_buf + 1, &frame, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_write_frame_scheduled(TF_ARINC429 *arinc429, uint8_t channel, uint16_t frame_index, uint32_t frame) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_WRITE_FRAME_SCHEDULED, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_WRITE_FRAME_SCHEDULED, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    frame_index = tf_leconvert_uint16_to(frame_index); memcpy(_send_buf + 1, &frame_index, 2);
    frame = tf_leconvert_uint32_to(frame); memcpy(_send_buf + 3, &frame, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_clear_schedule_entries(TF_ARINC429 *arinc429, uint8_t channel, uint16_t job_index_first, uint16_t job_index_last) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_CLEAR_SCHEDULE_ENTRIES, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_CLEAR_SCHEDULE_ENTRIES, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    job_index_first = tf_leconvert_uint16_to(job_index_first); memcpy(_send_buf + 1, &job_index_first, 2);
    job_index_last = tf_leconvert_uint16_to(job_index_last); memcpy(_send_buf + 3, &job_index_last, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_set_schedule_entry(TF_ARINC429 *arinc429, uint8_t channel, uint16_t job_index, uint8_t job, uint16_t frame_index, uint8_t dwell_time) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_SET_SCHEDULE_ENTRY, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_SET_SCHEDULE_ENTRY, 7, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    job_index = tf_leconvert_uint16_to(job_index); memcpy(_send_buf + 1, &job_index, 2);
    _send_buf[3] = (uint8_t)job;
    frame_index = tf_leconvert_uint16_to(frame_index); memcpy(_send_buf + 4, &frame_index, 2);
    _send_buf[6] = (uint8_t)dwell_time;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_get_schedule_entry(TF_ARINC429 *arinc429, uint8_t channel, uint16_t job_index, uint8_t *ret_job, uint16_t *ret_frame_index, uint32_t *ret_frame, uint8_t *ret_dwell_time) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_GET_SCHEDULE_ENTRY, 3, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    job_index = tf_leconvert_uint16_to(job_index); memcpy(_send_buf + 1, &job_index, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_job != NULL) { *ret_job = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_frame_index != NULL) { *ret_frame_index = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_frame != NULL) { *ret_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_dwell_time != NULL) { *ret_dwell_time = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_restart(TF_ARINC429 *arinc429) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_RESTART, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_RESTART, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_set_frame_mode(TF_ARINC429 *arinc429, uint8_t channel, uint16_t frame_index, uint8_t mode) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_SET_FRAME_MODE, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_SET_FRAME_MODE, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)channel;
    frame_index = tf_leconvert_uint16_to(frame_index); memcpy(_send_buf + 1, &frame_index, 2);
    _send_buf[3] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_get_spitfp_error_count(TF_ARINC429 *arinc429, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_set_bootloader_mode(TF_ARINC429 *arinc429, uint8_t mode, uint8_t *ret_status) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_get_bootloader_mode(TF_ARINC429 *arinc429, uint8_t *ret_mode) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_set_write_firmware_pointer(TF_ARINC429 *arinc429, uint32_t pointer) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_write_firmware(TF_ARINC429 *arinc429, const uint8_t data[64], uint8_t *ret_status) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_set_status_led_config(TF_ARINC429 *arinc429, uint8_t config) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_get_status_led_config(TF_ARINC429 *arinc429, uint8_t *ret_config) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_get_chip_temperature(TF_ARINC429 *arinc429, int16_t *ret_temperature) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_reset(TF_ARINC429 *arinc429) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_write_uid(TF_ARINC429 *arinc429, uint32_t uid) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_arinc429_get_response_expected(arinc429, TF_ARINC429_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(arinc429->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_read_uid(TF_ARINC429 *arinc429, uint32_t *ret_uid) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_arinc429_get_identity(TF_ARINC429 *arinc429, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = arinc429->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(arinc429->tfp, TF_ARINC429_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(arinc429->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(arinc429->tfp);
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
        tf_tfp_packet_processed(arinc429->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(arinc429->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_arinc429_register_heartbeat_message_callback(TF_ARINC429 *arinc429, TF_ARINC429_HeartbeatMessageHandler handler, void *user_data) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    arinc429->heartbeat_message_handler = handler;
    arinc429->heartbeat_message_user_data = user_data;

    return TF_E_OK;
}


int tf_arinc429_register_frame_message_callback(TF_ARINC429 *arinc429, TF_ARINC429_FrameMessageHandler handler, void *user_data) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    arinc429->frame_message_handler = handler;
    arinc429->frame_message_user_data = user_data;

    return TF_E_OK;
}


int tf_arinc429_register_scheduler_message_callback(TF_ARINC429 *arinc429, TF_ARINC429_SchedulerMessageHandler handler, void *user_data) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    arinc429->scheduler_message_handler = handler;
    arinc429->scheduler_message_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_arinc429_callback_tick(TF_ARINC429 *arinc429, uint32_t timeout_us) {
    if (arinc429 == NULL) {
        return TF_E_NULL;
    }

    if (arinc429->magic != 0x5446 || arinc429->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = arinc429->tfp->spitfp->hal;

    return tf_tfp_callback_tick(arinc429->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
