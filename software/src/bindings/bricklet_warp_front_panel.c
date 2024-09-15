/* ***********************************************************
 * This file was automatically generated on 2024-09-15.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_warp_front_panel.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


static bool tf_warp_front_panel_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    (void)device;
    (void)fid;
    (void)payload;

    return false;
}
int tf_warp_front_panel_create(TF_WARPFrontPanel *warp_front_panel, const char *uid_or_port_name, TF_HAL *hal) {
    if (warp_front_panel == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(warp_front_panel, 0, sizeof(TF_WARPFrontPanel));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_WARP_FRONT_PANEL_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    warp_front_panel->tfp = tfp;
    warp_front_panel->tfp->device = warp_front_panel;
    warp_front_panel->tfp->cb_handler = tf_warp_front_panel_callback_handler;
    warp_front_panel->magic = 0x5446;
    warp_front_panel->response_expected[0] = 0x00;
    warp_front_panel->response_expected[1] = 0x00;
    return TF_E_OK;
}

int tf_warp_front_panel_destroy(TF_WARPFrontPanel *warp_front_panel) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }
    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    warp_front_panel->tfp->cb_handler = NULL;
    warp_front_panel->tfp->device = NULL;
    warp_front_panel->tfp = NULL;
    warp_front_panel->magic = 0;

    return TF_E_OK;
}

int tf_warp_front_panel_get_response_expected(TF_WARPFrontPanel *warp_front_panel, uint8_t function_id, bool *ret_response_expected) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_FLASH_INDEX:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_front_panel->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_STATUS_BAR:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_front_panel->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_PAGE_INDEX:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_front_panel->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_FRONT_PAGE_ICON:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_front_panel->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_WIFI_SETUP_1:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_front_panel->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_WIFI_SETUP_2:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_front_panel->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_LED_STATE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_front_panel->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_front_panel->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_front_panel->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_front_panel->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_front_panel->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (warp_front_panel->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_warp_front_panel_set_response_expected(TF_WARPFrontPanel *warp_front_panel, uint8_t function_id, bool response_expected) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_FLASH_INDEX:
            if (response_expected) {
                warp_front_panel->response_expected[0] |= (1 << 0);
            } else {
                warp_front_panel->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_STATUS_BAR:
            if (response_expected) {
                warp_front_panel->response_expected[0] |= (1 << 1);
            } else {
                warp_front_panel->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_PAGE_INDEX:
            if (response_expected) {
                warp_front_panel->response_expected[0] |= (1 << 2);
            } else {
                warp_front_panel->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_FRONT_PAGE_ICON:
            if (response_expected) {
                warp_front_panel->response_expected[0] |= (1 << 3);
            } else {
                warp_front_panel->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_WIFI_SETUP_1:
            if (response_expected) {
                warp_front_panel->response_expected[0] |= (1 << 4);
            } else {
                warp_front_panel->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_WIFI_SETUP_2:
            if (response_expected) {
                warp_front_panel->response_expected[0] |= (1 << 5);
            } else {
                warp_front_panel->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_LED_STATE:
            if (response_expected) {
                warp_front_panel->response_expected[0] |= (1 << 6);
            } else {
                warp_front_panel->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY:
            if (response_expected) {
                warp_front_panel->response_expected[0] |= (1 << 7);
            } else {
                warp_front_panel->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                warp_front_panel->response_expected[1] |= (1 << 0);
            } else {
                warp_front_panel->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                warp_front_panel->response_expected[1] |= (1 << 1);
            } else {
                warp_front_panel->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_RESET:
            if (response_expected) {
                warp_front_panel->response_expected[1] |= (1 << 2);
            } else {
                warp_front_panel->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_WARP_FRONT_PANEL_FUNCTION_WRITE_UID:
            if (response_expected) {
                warp_front_panel->response_expected[1] |= (1 << 3);
            } else {
                warp_front_panel->response_expected[1] &= ~(1 << 3);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_warp_front_panel_set_response_expected_all(TF_WARPFrontPanel *warp_front_panel, bool response_expected) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(warp_front_panel->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_warp_front_panel_set_flash_index(TF_WARPFrontPanel *warp_front_panel, uint32_t page_index, uint8_t sub_page_index) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_front_panel_get_response_expected(warp_front_panel, TF_WARP_FRONT_PANEL_FUNCTION_SET_FLASH_INDEX, &_response_expected);
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_SET_FLASH_INDEX, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    page_index = tf_leconvert_uint32_to(page_index); memcpy(_send_buf + 0, &page_index, 4);
    _send_buf[4] = (uint8_t)sub_page_index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_flash_index(TF_WARPFrontPanel *warp_front_panel, uint32_t *ret_page_index, uint8_t *ret_sub_page_index) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_FLASH_INDEX, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_page_index != NULL) { *ret_page_index = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_sub_page_index != NULL) { *ret_sub_page_index = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_set_flash_data(TF_WARPFrontPanel *warp_front_panel, const uint8_t data[64], uint32_t *ret_next_page_index, uint8_t *ret_next_sub_page_index, uint8_t *ret_status) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_SET_FLASH_DATA, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_next_page_index != NULL) { *ret_next_page_index = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_next_sub_page_index != NULL) { *ret_next_sub_page_index = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_erase_flash_sector(TF_WARPFrontPanel *warp_front_panel, uint16_t sector_index, uint8_t *ret_status) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_ERASE_FLASH_SECTOR, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    sector_index = tf_leconvert_uint16_to(sector_index); memcpy(_send_buf + 0, &sector_index, 2);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_erase_flash(TF_WARPFrontPanel *warp_front_panel, uint8_t *ret_status) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_ERASE_FLASH, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_set_status_bar(TF_WARPFrontPanel *warp_front_panel, uint32_t ethernet_status, uint32_t wifi_status, uint8_t hours, uint8_t minutes, uint8_t seconds) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_front_panel_get_response_expected(warp_front_panel, TF_WARP_FRONT_PANEL_FUNCTION_SET_STATUS_BAR, &_response_expected);
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_SET_STATUS_BAR, 11, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    ethernet_status = tf_leconvert_uint32_to(ethernet_status); memcpy(_send_buf + 0, &ethernet_status, 4);
    wifi_status = tf_leconvert_uint32_to(wifi_status); memcpy(_send_buf + 4, &wifi_status, 4);
    _send_buf[8] = (uint8_t)hours;
    _send_buf[9] = (uint8_t)minutes;
    _send_buf[10] = (uint8_t)seconds;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_status_bar(TF_WARPFrontPanel *warp_front_panel, uint32_t *ret_ethernet_status, uint32_t *ret_wifi_status, uint8_t *ret_hours, uint8_t *ret_minutes, uint8_t *ret_seconds) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_STATUS_BAR, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 11) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_ethernet_status != NULL) { *ret_ethernet_status = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_wifi_status != NULL) { *ret_wifi_status = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_hours != NULL) { *ret_hours = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_minutes != NULL) { *ret_minutes = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_seconds != NULL) { *ret_seconds = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 11) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_set_display_page_index(TF_WARPFrontPanel *warp_front_panel, uint32_t page_index) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_front_panel_get_response_expected(warp_front_panel, TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_PAGE_INDEX, &_response_expected);
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_PAGE_INDEX, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    page_index = tf_leconvert_uint32_to(page_index); memcpy(_send_buf + 0, &page_index, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_display_page_index(TF_WARPFrontPanel *warp_front_panel, uint32_t *ret_page_index) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_DISPLAY_PAGE_INDEX, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_page_index != NULL) { *ret_page_index = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_set_display_front_page_icon(TF_WARPFrontPanel *warp_front_panel, uint32_t icon_index, bool active, uint32_t sprite_index, const char text_1[6], uint8_t font_index_1, const char text_2[6], uint8_t font_index_2) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_front_panel_get_response_expected(warp_front_panel, TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_FRONT_PAGE_ICON, &_response_expected);
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_FRONT_PAGE_ICON, 23, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    icon_index = tf_leconvert_uint32_to(icon_index); memcpy(_send_buf + 0, &icon_index, 4);
    _send_buf[4] = active ? 1 : 0;
    sprite_index = tf_leconvert_uint32_to(sprite_index); memcpy(_send_buf + 5, &sprite_index, 4);
    memcpy(_send_buf + 9, text_1, 6);
    _send_buf[15] = (uint8_t)font_index_1;
    memcpy(_send_buf + 16, text_2, 6);
    _send_buf[22] = (uint8_t)font_index_2;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_display_front_page_icon(TF_WARPFrontPanel *warp_front_panel, uint32_t icon_index, bool *ret_active, uint32_t *ret_sprite_index, char ret_text_1[10], uint8_t *ret_font_index_1, char ret_text_2[10], uint8_t *ret_font_index_2) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_DISPLAY_FRONT_PAGE_ICON, 4, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    icon_index = tf_leconvert_uint32_to(icon_index); memcpy(_send_buf + 0, &icon_index, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 27) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_active != NULL) { *ret_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_sprite_index != NULL) { *ret_sprite_index = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_text_1 != NULL) { for (_i = 0; _i < 10; ++_i) ret_text_1[_i] = tf_packet_buffer_read_char(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 10); }
            if (ret_font_index_1 != NULL) { *ret_font_index_1 = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_text_2 != NULL) { for (_i = 0; _i < 10; ++_i) ret_text_2[_i] = tf_packet_buffer_read_char(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 10); }
            if (ret_font_index_2 != NULL) { *ret_font_index_2 = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 27) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_set_display_wifi_setup_1(TF_WARPFrontPanel *warp_front_panel, const char ip_address[15], const char ssid[49]) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_front_panel_get_response_expected(warp_front_panel, TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_WIFI_SETUP_1, &_response_expected);
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_WIFI_SETUP_1, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    memcpy(_send_buf + 0, ip_address, 15);
    memcpy(_send_buf + 15, ssid, 49);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_display_wifi_setup_1(TF_WARPFrontPanel *warp_front_panel, char ret_ip_address[15], char ret_ssid[49]) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_DISPLAY_WIFI_SETUP_1, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_ip_address != NULL) { for (_i = 0; _i < 15; ++_i) ret_ip_address[_i] = tf_packet_buffer_read_char(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 15); }
            if (ret_ssid != NULL) { for (_i = 0; _i < 49; ++_i) ret_ssid[_i] = tf_packet_buffer_read_char(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 49); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_set_display_wifi_setup_2(TF_WARPFrontPanel *warp_front_panel, const char password[64]) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_front_panel_get_response_expected(warp_front_panel, TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_WIFI_SETUP_2, &_response_expected);
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY_WIFI_SETUP_2, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    memcpy(_send_buf + 0, password, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_display_wifi_setup_2(TF_WARPFrontPanel *warp_front_panel, char ret_password[64]) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_DISPLAY_WIFI_SETUP_2, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_password != NULL) { for (_i = 0; _i < 64; ++_i) ret_password[_i] = tf_packet_buffer_read_char(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 64); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_set_led_state(TF_WARPFrontPanel *warp_front_panel, uint8_t pattern, uint8_t color) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_front_panel_get_response_expected(warp_front_panel, TF_WARP_FRONT_PANEL_FUNCTION_SET_LED_STATE, &_response_expected);
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_SET_LED_STATE, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    _send_buf[0] = (uint8_t)pattern;
    _send_buf[1] = (uint8_t)color;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_led_state(TF_WARPFrontPanel *warp_front_panel, uint8_t *ret_pattern, uint8_t *ret_color) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_LED_STATE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_pattern != NULL) { *ret_pattern = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_color != NULL) { *ret_color = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_set_display(TF_WARPFrontPanel *warp_front_panel, uint8_t display) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_front_panel_get_response_expected(warp_front_panel, TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY, &_response_expected);
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_SET_DISPLAY, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    _send_buf[0] = (uint8_t)display;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_display(TF_WARPFrontPanel *warp_front_panel, uint8_t *ret_display, uint32_t *ret_countdown) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_DISPLAY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_display != NULL) { *ret_display = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_countdown != NULL) { *ret_countdown = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_flash_metadata(TF_WARPFrontPanel *warp_front_panel, uint32_t *ret_version_flash, uint32_t *ret_version_expected, uint32_t *ret_length_flash, uint32_t *ret_length_expected, uint32_t *ret_checksum_flash, uint32_t *ret_checksum_expected) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_FLASH_METADATA, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 24) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_version_flash != NULL) { *ret_version_flash = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_version_expected != NULL) { *ret_version_expected = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_length_flash != NULL) { *ret_length_flash = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_length_expected != NULL) { *ret_length_expected = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_checksum_flash != NULL) { *ret_checksum_flash = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_checksum_expected != NULL) { *ret_checksum_expected = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 24) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_spitfp_error_count(TF_WARPFrontPanel *warp_front_panel, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_set_bootloader_mode(TF_WARPFrontPanel *warp_front_panel, uint8_t mode, uint8_t *ret_status) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_bootloader_mode(TF_WARPFrontPanel *warp_front_panel, uint8_t *ret_mode) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_set_write_firmware_pointer(TF_WARPFrontPanel *warp_front_panel, uint32_t pointer) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_front_panel_get_response_expected(warp_front_panel, TF_WARP_FRONT_PANEL_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_write_firmware(TF_WARPFrontPanel *warp_front_panel, const uint8_t data[64], uint8_t *ret_status) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_set_status_led_config(TF_WARPFrontPanel *warp_front_panel, uint8_t config) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_front_panel_get_response_expected(warp_front_panel, TF_WARP_FRONT_PANEL_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_status_led_config(TF_WARPFrontPanel *warp_front_panel, uint8_t *ret_config) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_chip_temperature(TF_WARPFrontPanel *warp_front_panel, int16_t *ret_temperature) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_reset(TF_WARPFrontPanel *warp_front_panel) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_front_panel_get_response_expected(warp_front_panel, TF_WARP_FRONT_PANEL_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_write_uid(TF_WARPFrontPanel *warp_front_panel, uint32_t uid) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_warp_front_panel_get_response_expected(warp_front_panel, TF_WARP_FRONT_PANEL_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(warp_front_panel->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_read_uid(TF_WARPFrontPanel *warp_front_panel, uint32_t *ret_uid) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_warp_front_panel_get_identity(TF_WARPFrontPanel *warp_front_panel, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = warp_front_panel->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(warp_front_panel->tfp, TF_WARP_FRONT_PANEL_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(warp_front_panel->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(warp_front_panel->tfp);
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
        tf_tfp_packet_processed(warp_front_panel->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(warp_front_panel->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}


int tf_warp_front_panel_callback_tick(TF_WARPFrontPanel *warp_front_panel, uint32_t timeout_us) {
    if (warp_front_panel == NULL) {
        return TF_E_NULL;
    }

    if (warp_front_panel->magic != 0x5446 || warp_front_panel->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = warp_front_panel->tfp->spitfp->hal;

    return tf_tfp_callback_tick(warp_front_panel->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
