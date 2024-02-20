/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_lcd_128x64.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_lcd_128x64_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_LCD128x64 *lcd_128x64 = (TF_LCD128x64 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(lcd_128x64->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_LCD_128X64_CALLBACK_TOUCH_POSITION: {
            TF_LCD128x64_TouchPositionHandler fn = lcd_128x64->touch_position_handler;
            void *user_data = lcd_128x64->touch_position_user_data;
            if (fn == NULL) {
                return false;
            }

            uint16_t pressure = tf_packet_buffer_read_uint16_t(payload);
            uint16_t x = tf_packet_buffer_read_uint16_t(payload);
            uint16_t y = tf_packet_buffer_read_uint16_t(payload);
            uint32_t age = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(lcd_128x64, pressure, x, y, age, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_LCD_128X64_CALLBACK_TOUCH_GESTURE: {
            TF_LCD128x64_TouchGestureHandler fn = lcd_128x64->touch_gesture_handler;
            void *user_data = lcd_128x64->touch_gesture_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t gesture = tf_packet_buffer_read_uint8_t(payload);
            uint32_t duration = tf_packet_buffer_read_uint32_t(payload);
            uint16_t pressure_max = tf_packet_buffer_read_uint16_t(payload);
            uint16_t x_start = tf_packet_buffer_read_uint16_t(payload);
            uint16_t y_start = tf_packet_buffer_read_uint16_t(payload);
            uint16_t x_end = tf_packet_buffer_read_uint16_t(payload);
            uint16_t y_end = tf_packet_buffer_read_uint16_t(payload);
            uint32_t age = tf_packet_buffer_read_uint32_t(payload);
            hal_common->locked = true;
            fn(lcd_128x64, gesture, duration, pressure_max, x_start, y_start, x_end, y_end, age, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_LCD_128X64_CALLBACK_GUI_BUTTON_PRESSED: {
            TF_LCD128x64_GUIButtonPressedHandler fn = lcd_128x64->gui_button_pressed_handler;
            void *user_data = lcd_128x64->gui_button_pressed_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t index = tf_packet_buffer_read_uint8_t(payload);
            bool pressed = tf_packet_buffer_read_bool(payload);
            hal_common->locked = true;
            fn(lcd_128x64, index, pressed, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_LCD_128X64_CALLBACK_GUI_SLIDER_VALUE: {
            TF_LCD128x64_GUISliderValueHandler fn = lcd_128x64->gui_slider_value_handler;
            void *user_data = lcd_128x64->gui_slider_value_user_data;
            if (fn == NULL) {
                return false;
            }

            uint8_t index = tf_packet_buffer_read_uint8_t(payload);
            uint8_t value = tf_packet_buffer_read_uint8_t(payload);
            hal_common->locked = true;
            fn(lcd_128x64, index, value, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_LCD_128X64_CALLBACK_GUI_TAB_SELECTED: {
            TF_LCD128x64_GUITabSelectedHandler fn = lcd_128x64->gui_tab_selected_handler;
            void *user_data = lcd_128x64->gui_tab_selected_user_data;
            if (fn == NULL) {
                return false;
            }

            int8_t index = tf_packet_buffer_read_int8_t(payload);
            hal_common->locked = true;
            fn(lcd_128x64, index, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_lcd_128x64_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_lcd_128x64_create(TF_LCD128x64 *lcd_128x64, const char *uid_or_port_name, TF_HAL *hal) {
    if (lcd_128x64 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(lcd_128x64, 0, sizeof(TF_LCD128x64));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_LCD_128X64_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    lcd_128x64->tfp = tfp;
    lcd_128x64->tfp->device = lcd_128x64;
    lcd_128x64->tfp->cb_handler = tf_lcd_128x64_callback_handler;
    lcd_128x64->magic = 0x5446;
    lcd_128x64->response_expected[0] = 0x61;
    lcd_128x64->response_expected[1] = 0x90;
    lcd_128x64->response_expected[2] = 0xA0;
    lcd_128x64->response_expected[3] = 0x00;
    return TF_E_OK;
}

int tf_lcd_128x64_destroy(TF_LCD128x64 *lcd_128x64) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }
    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    lcd_128x64->tfp->cb_handler = NULL;
    lcd_128x64->tfp->device = NULL;
    lcd_128x64->tfp = NULL;
    lcd_128x64->magic = 0;

    return TF_E_OK;
}

int tf_lcd_128x64_get_response_expected(TF_LCD128x64 *lcd_128x64, uint8_t function_id, bool *ret_response_expected) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_LCD_128X64_FUNCTION_WRITE_PIXELS_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_CLEAR_DISPLAY:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_DISPLAY_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_WRITE_LINE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_DRAW_BUFFERED_FRAME:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_TOUCH_POSITION_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_TOUCH_GESTURE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_DRAW_LINE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_DRAW_BOX:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_DRAW_TEXT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_BUTTON:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_REMOVE_GUI_BUTTON:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_BUTTON_PRESSED_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[1] & (1 << 4)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_SLIDER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[1] & (1 << 5)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_REMOVE_GUI_SLIDER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[1] & (1 << 6)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_SLIDER_VALUE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[1] & (1 << 7)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_TAB_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[2] & (1 << 0)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_TAB_TEXT:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[2] & (1 << 1)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_TAB_ICON:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[2] & (1 << 2)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_REMOVE_GUI_TAB:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[2] & (1 << 3)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_TAB_SELECTED:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[2] & (1 << 4)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_TAB_SELECTED_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[2] & (1 << 5)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_GRAPH_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[2] & (1 << 6)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_GRAPH_DATA_LOW_LEVEL:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[2] & (1 << 7)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_REMOVE_GUI_GRAPH:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[3] & (1 << 0)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_REMOVE_ALL_GUI:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[3] & (1 << 1)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_TOUCH_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[3] & (1 << 2)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[3] & (1 << 3)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[3] & (1 << 4)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[3] & (1 << 5)) != 0;
            }
            break;
        case TF_LCD_128X64_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (lcd_128x64->response_expected[3] & (1 << 6)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_lcd_128x64_set_response_expected(TF_LCD128x64 *lcd_128x64, uint8_t function_id, bool response_expected) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_LCD_128X64_FUNCTION_WRITE_PIXELS_LOW_LEVEL:
            if (response_expected) {
                lcd_128x64->response_expected[0] |= (1 << 0);
            } else {
                lcd_128x64->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_LCD_128X64_FUNCTION_CLEAR_DISPLAY:
            if (response_expected) {
                lcd_128x64->response_expected[0] |= (1 << 1);
            } else {
                lcd_128x64->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_DISPLAY_CONFIGURATION:
            if (response_expected) {
                lcd_128x64->response_expected[0] |= (1 << 2);
            } else {
                lcd_128x64->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_LCD_128X64_FUNCTION_WRITE_LINE:
            if (response_expected) {
                lcd_128x64->response_expected[0] |= (1 << 3);
            } else {
                lcd_128x64->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_LCD_128X64_FUNCTION_DRAW_BUFFERED_FRAME:
            if (response_expected) {
                lcd_128x64->response_expected[0] |= (1 << 4);
            } else {
                lcd_128x64->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_TOUCH_POSITION_CALLBACK_CONFIGURATION:
            if (response_expected) {
                lcd_128x64->response_expected[0] |= (1 << 5);
            } else {
                lcd_128x64->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_TOUCH_GESTURE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                lcd_128x64->response_expected[0] |= (1 << 6);
            } else {
                lcd_128x64->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_LCD_128X64_FUNCTION_DRAW_LINE:
            if (response_expected) {
                lcd_128x64->response_expected[0] |= (1 << 7);
            } else {
                lcd_128x64->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_LCD_128X64_FUNCTION_DRAW_BOX:
            if (response_expected) {
                lcd_128x64->response_expected[1] |= (1 << 0);
            } else {
                lcd_128x64->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_LCD_128X64_FUNCTION_DRAW_TEXT:
            if (response_expected) {
                lcd_128x64->response_expected[1] |= (1 << 1);
            } else {
                lcd_128x64->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_BUTTON:
            if (response_expected) {
                lcd_128x64->response_expected[1] |= (1 << 2);
            } else {
                lcd_128x64->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_LCD_128X64_FUNCTION_REMOVE_GUI_BUTTON:
            if (response_expected) {
                lcd_128x64->response_expected[1] |= (1 << 3);
            } else {
                lcd_128x64->response_expected[1] &= ~(1 << 3);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_BUTTON_PRESSED_CALLBACK_CONFIGURATION:
            if (response_expected) {
                lcd_128x64->response_expected[1] |= (1 << 4);
            } else {
                lcd_128x64->response_expected[1] &= ~(1 << 4);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_SLIDER:
            if (response_expected) {
                lcd_128x64->response_expected[1] |= (1 << 5);
            } else {
                lcd_128x64->response_expected[1] &= ~(1 << 5);
            }
            break;
        case TF_LCD_128X64_FUNCTION_REMOVE_GUI_SLIDER:
            if (response_expected) {
                lcd_128x64->response_expected[1] |= (1 << 6);
            } else {
                lcd_128x64->response_expected[1] &= ~(1 << 6);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_SLIDER_VALUE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                lcd_128x64->response_expected[1] |= (1 << 7);
            } else {
                lcd_128x64->response_expected[1] &= ~(1 << 7);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_TAB_CONFIGURATION:
            if (response_expected) {
                lcd_128x64->response_expected[2] |= (1 << 0);
            } else {
                lcd_128x64->response_expected[2] &= ~(1 << 0);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_TAB_TEXT:
            if (response_expected) {
                lcd_128x64->response_expected[2] |= (1 << 1);
            } else {
                lcd_128x64->response_expected[2] &= ~(1 << 1);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_TAB_ICON:
            if (response_expected) {
                lcd_128x64->response_expected[2] |= (1 << 2);
            } else {
                lcd_128x64->response_expected[2] &= ~(1 << 2);
            }
            break;
        case TF_LCD_128X64_FUNCTION_REMOVE_GUI_TAB:
            if (response_expected) {
                lcd_128x64->response_expected[2] |= (1 << 3);
            } else {
                lcd_128x64->response_expected[2] &= ~(1 << 3);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_TAB_SELECTED:
            if (response_expected) {
                lcd_128x64->response_expected[2] |= (1 << 4);
            } else {
                lcd_128x64->response_expected[2] &= ~(1 << 4);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_TAB_SELECTED_CALLBACK_CONFIGURATION:
            if (response_expected) {
                lcd_128x64->response_expected[2] |= (1 << 5);
            } else {
                lcd_128x64->response_expected[2] &= ~(1 << 5);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_GRAPH_CONFIGURATION:
            if (response_expected) {
                lcd_128x64->response_expected[2] |= (1 << 6);
            } else {
                lcd_128x64->response_expected[2] &= ~(1 << 6);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_GUI_GRAPH_DATA_LOW_LEVEL:
            if (response_expected) {
                lcd_128x64->response_expected[2] |= (1 << 7);
            } else {
                lcd_128x64->response_expected[2] &= ~(1 << 7);
            }
            break;
        case TF_LCD_128X64_FUNCTION_REMOVE_GUI_GRAPH:
            if (response_expected) {
                lcd_128x64->response_expected[3] |= (1 << 0);
            } else {
                lcd_128x64->response_expected[3] &= ~(1 << 0);
            }
            break;
        case TF_LCD_128X64_FUNCTION_REMOVE_ALL_GUI:
            if (response_expected) {
                lcd_128x64->response_expected[3] |= (1 << 1);
            } else {
                lcd_128x64->response_expected[3] &= ~(1 << 1);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_TOUCH_LED_CONFIG:
            if (response_expected) {
                lcd_128x64->response_expected[3] |= (1 << 2);
            } else {
                lcd_128x64->response_expected[3] &= ~(1 << 2);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                lcd_128x64->response_expected[3] |= (1 << 3);
            } else {
                lcd_128x64->response_expected[3] &= ~(1 << 3);
            }
            break;
        case TF_LCD_128X64_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                lcd_128x64->response_expected[3] |= (1 << 4);
            } else {
                lcd_128x64->response_expected[3] &= ~(1 << 4);
            }
            break;
        case TF_LCD_128X64_FUNCTION_RESET:
            if (response_expected) {
                lcd_128x64->response_expected[3] |= (1 << 5);
            } else {
                lcd_128x64->response_expected[3] &= ~(1 << 5);
            }
            break;
        case TF_LCD_128X64_FUNCTION_WRITE_UID:
            if (response_expected) {
                lcd_128x64->response_expected[3] |= (1 << 6);
            } else {
                lcd_128x64->response_expected[3] &= ~(1 << 6);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_lcd_128x64_set_response_expected_all(TF_LCD128x64 *lcd_128x64, bool response_expected) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(lcd_128x64->response_expected, response_expected ? 0xFF : 0, 4);

    return TF_E_OK;
}

int tf_lcd_128x64_write_pixels_low_level(TF_LCD128x64 *lcd_128x64, uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end, uint16_t pixels_length, uint16_t pixels_chunk_offset, const bool pixels_chunk_data[448]) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_WRITE_PIXELS_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_WRITE_PIXELS_LOW_LEVEL, 64, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

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
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_read_pixels_low_level(TF_LCD128x64 *lcd_128x64, uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end, uint16_t *ret_pixels_length, uint16_t *ret_pixels_chunk_offset, bool ret_pixels_chunk_data[480]) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_READ_PIXELS_LOW_LEVEL, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)x_start;
    _send_buf[1] = (uint8_t)y_start;
    _send_buf[2] = (uint8_t)x_end;
    _send_buf[3] = (uint8_t)y_end;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 64) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_pixels_length != NULL) { *ret_pixels_length = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_pixels_chunk_offset != NULL) { *ret_pixels_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_pixels_chunk_data != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_pixels_chunk_data, 480);} else { tf_packet_buffer_remove(_recv_buf, 60); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 64) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_clear_display(TF_LCD128x64 *lcd_128x64) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_CLEAR_DISPLAY, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_CLEAR_DISPLAY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_display_configuration(TF_LCD128x64 *lcd_128x64, uint8_t contrast, uint8_t backlight, bool invert, bool automatic_draw) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_DISPLAY_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_DISPLAY_CONFIGURATION, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)contrast;
    _send_buf[1] = (uint8_t)backlight;
    _send_buf[2] = invert ? 1 : 0;
    _send_buf[3] = automatic_draw ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_display_configuration(TF_LCD128x64 *lcd_128x64, uint8_t *ret_contrast, uint8_t *ret_backlight, bool *ret_invert, bool *ret_automatic_draw) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_DISPLAY_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_contrast != NULL) { *ret_contrast = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_backlight != NULL) { *ret_backlight = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_invert != NULL) { *ret_invert = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_automatic_draw != NULL) { *ret_automatic_draw = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_write_line(TF_LCD128x64 *lcd_128x64, uint8_t line, uint8_t position, const char *text) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_WRITE_LINE, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_WRITE_LINE, 24, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)line;
    _send_buf[1] = (uint8_t)position;
    strncpy((char *)(_send_buf + 2), text, 22);


    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_draw_buffered_frame(TF_LCD128x64 *lcd_128x64, bool force_complete_redraw) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_DRAW_BUFFERED_FRAME, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_DRAW_BUFFERED_FRAME, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = force_complete_redraw ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_touch_position(TF_LCD128x64 *lcd_128x64, uint16_t *ret_pressure, uint16_t *ret_x, uint16_t *ret_y, uint32_t *ret_age) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_TOUCH_POSITION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 10) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_pressure != NULL) { *ret_pressure = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_x != NULL) { *ret_x = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_y != NULL) { *ret_y = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_age != NULL) { *ret_age = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 10) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_touch_position_callback_configuration(TF_LCD128x64 *lcd_128x64, uint32_t period, bool value_has_to_change) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_TOUCH_POSITION_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_TOUCH_POSITION_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_touch_position_callback_configuration(TF_LCD128x64 *lcd_128x64, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_TOUCH_POSITION_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_touch_gesture(TF_LCD128x64 *lcd_128x64, uint8_t *ret_gesture, uint32_t *ret_duration, uint16_t *ret_pressure_max, uint16_t *ret_x_start, uint16_t *ret_y_start, uint16_t *ret_x_end, uint16_t *ret_y_end, uint32_t *ret_age) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_TOUCH_GESTURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 19) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_gesture != NULL) { *ret_gesture = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_duration != NULL) { *ret_duration = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_pressure_max != NULL) { *ret_pressure_max = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_x_start != NULL) { *ret_x_start = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_y_start != NULL) { *ret_y_start = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_x_end != NULL) { *ret_x_end = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_y_end != NULL) { *ret_y_end = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_age != NULL) { *ret_age = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 19) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_touch_gesture_callback_configuration(TF_LCD128x64 *lcd_128x64, uint32_t period, bool value_has_to_change) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_TOUCH_GESTURE_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_TOUCH_GESTURE_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_touch_gesture_callback_configuration(TF_LCD128x64 *lcd_128x64, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_TOUCH_GESTURE_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_draw_line(TF_LCD128x64 *lcd_128x64, uint8_t position_x_start, uint8_t position_y_start, uint8_t position_x_end, uint8_t position_y_end, bool color) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_DRAW_LINE, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_DRAW_LINE, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)position_x_start;
    _send_buf[1] = (uint8_t)position_y_start;
    _send_buf[2] = (uint8_t)position_x_end;
    _send_buf[3] = (uint8_t)position_y_end;
    _send_buf[4] = color ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_draw_box(TF_LCD128x64 *lcd_128x64, uint8_t position_x_start, uint8_t position_y_start, uint8_t position_x_end, uint8_t position_y_end, bool fill, bool color) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_DRAW_BOX, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_DRAW_BOX, 6, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)position_x_start;
    _send_buf[1] = (uint8_t)position_y_start;
    _send_buf[2] = (uint8_t)position_x_end;
    _send_buf[3] = (uint8_t)position_y_end;
    _send_buf[4] = fill ? 1 : 0;
    _send_buf[5] = color ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_draw_text(TF_LCD128x64 *lcd_128x64, uint8_t position_x, uint8_t position_y, uint8_t font, bool color, const char *text) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_DRAW_TEXT, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_DRAW_TEXT, 26, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)position_x;
    _send_buf[1] = (uint8_t)position_y;
    _send_buf[2] = (uint8_t)font;
    _send_buf[3] = color ? 1 : 0;
    strncpy((char *)(_send_buf + 4), text, 22);


    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_gui_button(TF_LCD128x64 *lcd_128x64, uint8_t index, uint8_t position_x, uint8_t position_y, uint8_t width, uint8_t height, const char *text) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_GUI_BUTTON, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_GUI_BUTTON, 21, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;
    _send_buf[1] = (uint8_t)position_x;
    _send_buf[2] = (uint8_t)position_y;
    _send_buf[3] = (uint8_t)width;
    _send_buf[4] = (uint8_t)height;
    strncpy((char *)(_send_buf + 5), text, 16);


    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_button(TF_LCD128x64 *lcd_128x64, uint8_t index, bool *ret_active, uint8_t *ret_position_x, uint8_t *ret_position_y, uint8_t *ret_width, uint8_t *ret_height, char ret_text[16]) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_BUTTON, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 21) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_active != NULL) { *ret_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_position_x != NULL) { *ret_position_x = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_position_y != NULL) { *ret_position_y = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_width != NULL) { *ret_width = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_height != NULL) { *ret_height = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_text != NULL) { tf_packet_buffer_pop_n(_recv_buf, (uint8_t *)ret_text, 16);} else { tf_packet_buffer_remove(_recv_buf, 16); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 21) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_remove_gui_button(TF_LCD128x64 *lcd_128x64, uint8_t index) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_REMOVE_GUI_BUTTON, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_REMOVE_GUI_BUTTON, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_gui_button_pressed_callback_configuration(TF_LCD128x64 *lcd_128x64, uint32_t period, bool value_has_to_change) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_GUI_BUTTON_PRESSED_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_GUI_BUTTON_PRESSED_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_button_pressed_callback_configuration(TF_LCD128x64 *lcd_128x64, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_BUTTON_PRESSED_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_button_pressed(TF_LCD128x64 *lcd_128x64, uint8_t index, bool *ret_pressed) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_BUTTON_PRESSED, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_pressed != NULL) { *ret_pressed = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_gui_slider(TF_LCD128x64 *lcd_128x64, uint8_t index, uint8_t position_x, uint8_t position_y, uint8_t length, uint8_t direction, uint8_t value) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_GUI_SLIDER, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_GUI_SLIDER, 6, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;
    _send_buf[1] = (uint8_t)position_x;
    _send_buf[2] = (uint8_t)position_y;
    _send_buf[3] = (uint8_t)length;
    _send_buf[4] = (uint8_t)direction;
    _send_buf[5] = (uint8_t)value;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_slider(TF_LCD128x64 *lcd_128x64, uint8_t index, bool *ret_active, uint8_t *ret_position_x, uint8_t *ret_position_y, uint8_t *ret_length, uint8_t *ret_direction, uint8_t *ret_value) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_SLIDER, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_active != NULL) { *ret_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_position_x != NULL) { *ret_position_x = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_position_y != NULL) { *ret_position_y = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_length != NULL) { *ret_length = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_direction != NULL) { *ret_direction = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_value != NULL) { *ret_value = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_remove_gui_slider(TF_LCD128x64 *lcd_128x64, uint8_t index) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_REMOVE_GUI_SLIDER, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_REMOVE_GUI_SLIDER, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_gui_slider_value_callback_configuration(TF_LCD128x64 *lcd_128x64, uint32_t period, bool value_has_to_change) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_GUI_SLIDER_VALUE_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_GUI_SLIDER_VALUE_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_slider_value_callback_configuration(TF_LCD128x64 *lcd_128x64, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_SLIDER_VALUE_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_slider_value(TF_LCD128x64 *lcd_128x64, uint8_t index, uint8_t *ret_value) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_SLIDER_VALUE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_value != NULL) { *ret_value = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_gui_tab_configuration(TF_LCD128x64 *lcd_128x64, uint8_t change_tab_config, bool clear_gui) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_GUI_TAB_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_GUI_TAB_CONFIGURATION, 2, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)change_tab_config;
    _send_buf[1] = clear_gui ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_tab_configuration(TF_LCD128x64 *lcd_128x64, uint8_t *ret_change_tab_config, bool *ret_clear_gui) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_TAB_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_change_tab_config != NULL) { *ret_change_tab_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_clear_gui != NULL) { *ret_clear_gui = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_gui_tab_text(TF_LCD128x64 *lcd_128x64, uint8_t index, const char *text) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_GUI_TAB_TEXT, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_GUI_TAB_TEXT, 6, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;
    strncpy((char *)(_send_buf + 1), text, 5);


    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_tab_text(TF_LCD128x64 *lcd_128x64, uint8_t index, bool *ret_active, char ret_text[5]) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_TAB_TEXT, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_active != NULL) { *ret_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_text != NULL) { tf_packet_buffer_pop_n(_recv_buf, (uint8_t *)ret_text, 5);} else { tf_packet_buffer_remove(_recv_buf, 5); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_gui_tab_icon(TF_LCD128x64 *lcd_128x64, uint8_t index, const bool icon[168]) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_GUI_TAB_ICON, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_GUI_TAB_ICON, 22, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;
    memset(_send_buf + 1, 0, 21); for (_i = 0; _i < 168; ++_i) _send_buf[1 + (_i / 8)] |= (icon[_i] ? 1 : 0) << (_i % 8);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_tab_icon(TF_LCD128x64 *lcd_128x64, uint8_t index, bool *ret_active, bool ret_icon[168]) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_TAB_ICON, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 22) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_active != NULL) { *ret_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_icon != NULL) { tf_packet_buffer_read_bool_array(_recv_buf, ret_icon, 168);} else { tf_packet_buffer_remove(_recv_buf, 21); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 22) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_remove_gui_tab(TF_LCD128x64 *lcd_128x64, uint8_t index) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_REMOVE_GUI_TAB, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_REMOVE_GUI_TAB, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_gui_tab_selected(TF_LCD128x64 *lcd_128x64, uint8_t index) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_GUI_TAB_SELECTED, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_GUI_TAB_SELECTED, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_gui_tab_selected_callback_configuration(TF_LCD128x64 *lcd_128x64, uint32_t period, bool value_has_to_change) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_GUI_TAB_SELECTED_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_GUI_TAB_SELECTED_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_tab_selected_callback_configuration(TF_LCD128x64 *lcd_128x64, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_TAB_SELECTED_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_tab_selected(TF_LCD128x64 *lcd_128x64, int8_t *ret_index) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_TAB_SELECTED, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_index != NULL) { *ret_index = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_gui_graph_configuration(TF_LCD128x64 *lcd_128x64, uint8_t index, uint8_t graph_type, uint8_t position_x, uint8_t position_y, uint8_t width, uint8_t height, const char *text_x, const char *text_y) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_GUI_GRAPH_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_GUI_GRAPH_CONFIGURATION, 14, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;
    _send_buf[1] = (uint8_t)graph_type;
    _send_buf[2] = (uint8_t)position_x;
    _send_buf[3] = (uint8_t)position_y;
    _send_buf[4] = (uint8_t)width;
    _send_buf[5] = (uint8_t)height;
    strncpy((char *)(_send_buf + 6), text_x, 4);

    strncpy((char *)(_send_buf + 10), text_y, 4);


    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_graph_configuration(TF_LCD128x64 *lcd_128x64, uint8_t index, bool *ret_active, uint8_t *ret_graph_type, uint8_t *ret_position_x, uint8_t *ret_position_y, uint8_t *ret_width, uint8_t *ret_height, char ret_text_x[4], char ret_text_y[4]) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_GRAPH_CONFIGURATION, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 14) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_active != NULL) { *ret_active = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_graph_type != NULL) { *ret_graph_type = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_position_x != NULL) { *ret_position_x = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_position_y != NULL) { *ret_position_y = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_width != NULL) { *ret_width = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_height != NULL) { *ret_height = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_text_x != NULL) { tf_packet_buffer_pop_n(_recv_buf, (uint8_t *)ret_text_x, 4);} else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_text_y != NULL) { tf_packet_buffer_pop_n(_recv_buf, (uint8_t *)ret_text_y, 4);} else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 14) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_gui_graph_data_low_level(TF_LCD128x64 *lcd_128x64, uint8_t index, uint16_t data_length, uint16_t data_chunk_offset, const uint8_t data_chunk_data[59]) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_GUI_GRAPH_DATA_LOW_LEVEL, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_GUI_GRAPH_DATA_LOW_LEVEL, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;
    data_length = tf_leconvert_uint16_to(data_length); memcpy(_send_buf + 1, &data_length, 2);
    data_chunk_offset = tf_leconvert_uint16_to(data_chunk_offset); memcpy(_send_buf + 3, &data_chunk_offset, 2);
    memcpy(_send_buf + 5, data_chunk_data, 59);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_gui_graph_data_low_level(TF_LCD128x64 *lcd_128x64, uint8_t index, uint16_t *ret_data_length, uint16_t *ret_data_chunk_offset, uint8_t ret_data_chunk_data[59]) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_GUI_GRAPH_DATA_LOW_LEVEL, 1, _response_expected);

    size_t _i;
    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 63) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_data_length != NULL) { *ret_data_length = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_data_chunk_offset != NULL) { *ret_data_chunk_offset = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_data_chunk_data != NULL) { for (_i = 0; _i < 59; ++_i) ret_data_chunk_data[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 59); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 63) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_remove_gui_graph(TF_LCD128x64 *lcd_128x64, uint8_t index) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_REMOVE_GUI_GRAPH, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_REMOVE_GUI_GRAPH, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)index;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_remove_all_gui(TF_LCD128x64 *lcd_128x64) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_REMOVE_ALL_GUI, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_REMOVE_ALL_GUI, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_touch_led_config(TF_LCD128x64 *lcd_128x64, uint8_t config) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_TOUCH_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_TOUCH_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_touch_led_config(TF_LCD128x64 *lcd_128x64, uint8_t *ret_config) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_TOUCH_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_spitfp_error_count(TF_LCD128x64 *lcd_128x64, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_bootloader_mode(TF_LCD128x64 *lcd_128x64, uint8_t mode, uint8_t *ret_status) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_bootloader_mode(TF_LCD128x64 *lcd_128x64, uint8_t *ret_mode) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_write_firmware_pointer(TF_LCD128x64 *lcd_128x64, uint32_t pointer) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_write_firmware(TF_LCD128x64 *lcd_128x64, const uint8_t data[64], uint8_t *ret_status) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_set_status_led_config(TF_LCD128x64 *lcd_128x64, uint8_t config) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_status_led_config(TF_LCD128x64 *lcd_128x64, uint8_t *ret_config) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_chip_temperature(TF_LCD128x64 *lcd_128x64, int16_t *ret_temperature) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_reset(TF_LCD128x64 *lcd_128x64) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_write_uid(TF_LCD128x64 *lcd_128x64, uint32_t uid) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_lcd_128x64_get_response_expected(lcd_128x64, TF_LCD_128X64_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(lcd_128x64->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_read_uid(TF_LCD128x64 *lcd_128x64, uint32_t *ret_uid) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_lcd_128x64_get_identity(TF_LCD128x64 *lcd_128x64, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = lcd_128x64->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(lcd_128x64->tfp, TF_LCD_128X64_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(lcd_128x64->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(lcd_128x64->tfp);
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
        tf_tfp_packet_processed(lcd_128x64->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(lcd_128x64->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

typedef struct TF_LCD128x64_WritePixelsLLWrapperData {
    uint8_t x_start;
    uint8_t y_start;
    uint8_t x_end;
    uint8_t y_end;
} TF_LCD128x64_WritePixelsLLWrapperData;


static int tf_lcd_128x64_write_pixels_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    TF_LCD128x64_WritePixelsLLWrapperData *data = (TF_LCD128x64_WritePixelsLLWrapperData *) wrapper_data;
    uint16_t pixels_chunk_offset = (uint16_t)chunk_offset;
    uint16_t pixels_length = (uint16_t)stream_length;
    uint32_t pixels_chunk_written = 448;

    bool *pixels_chunk_data = (bool *) chunk_data;
    int ret = tf_lcd_128x64_write_pixels_low_level((TF_LCD128x64 *)device, data->x_start, data->y_start, data->x_end, data->y_end, pixels_length, pixels_chunk_offset, pixels_chunk_data);

    *ret_chunk_written = (uint32_t) pixels_chunk_written;
    return ret;
}

int tf_lcd_128x64_write_pixels(TF_LCD128x64 *lcd_128x64, uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end, const bool *pixels, uint16_t pixels_length) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_LCD128x64_WritePixelsLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.x_start = x_start;
    _wrapper_data.y_start = y_start;
    _wrapper_data.x_end = x_end;
    _wrapper_data.y_end = y_end;

    uint32_t _stream_length = pixels_length;
    uint32_t _pixels_written = 0;
    bool _chunk_data[448];

    int ret = tf_stream_in(lcd_128x64, tf_lcd_128x64_write_pixels_ll_wrapper, &_wrapper_data, pixels, _stream_length, _chunk_data, &_pixels_written, 448, tf_copy_items_bool);



    return ret;
}


typedef struct TF_LCD128x64_ReadPixelsLLWrapperData {
    uint8_t x_start;
    uint8_t y_start;
    uint8_t x_end;
    uint8_t y_end;
} TF_LCD128x64_ReadPixelsLLWrapperData;


static int tf_lcd_128x64_read_pixels_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    TF_LCD128x64_ReadPixelsLLWrapperData *data = (TF_LCD128x64_ReadPixelsLLWrapperData *) wrapper_data;
    uint16_t pixels_length = 0;
    uint16_t pixels_chunk_offset = 0;
    bool *pixels_chunk_data = (bool *) chunk_data;
    int ret = tf_lcd_128x64_read_pixels_low_level((TF_LCD128x64 *)device, data->x_start, data->y_start, data->x_end, data->y_end, &pixels_length, &pixels_chunk_offset, pixels_chunk_data);

    *ret_stream_length = (uint32_t)pixels_length;
    *ret_chunk_offset = (uint32_t)pixels_chunk_offset;
    return ret;
}

int tf_lcd_128x64_read_pixels(TF_LCD128x64 *lcd_128x64, uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end, bool *ret_pixels, uint16_t *ret_pixels_length) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_LCD128x64_ReadPixelsLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.x_start = x_start;
    _wrapper_data.y_start = y_start;
    _wrapper_data.x_end = x_end;
    _wrapper_data.y_end = y_end;
    uint32_t _pixels_length = 0;
    bool _pixels_chunk_data[480];

    int ret = tf_stream_out(lcd_128x64, tf_lcd_128x64_read_pixels_ll_wrapper, &_wrapper_data, ret_pixels, &_pixels_length, _pixels_chunk_data, 480, tf_copy_items_bool);

    if (ret_pixels_length != NULL) {
        *ret_pixels_length = (uint16_t)_pixels_length;
    }
    return ret;
}

typedef struct TF_LCD128x64_SetGUIGraphDataLLWrapperData {
    uint8_t index;
} TF_LCD128x64_SetGUIGraphDataLLWrapperData;


static int tf_lcd_128x64_set_gui_graph_data_ll_wrapper(void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written) {
    TF_LCD128x64_SetGUIGraphDataLLWrapperData *data = (TF_LCD128x64_SetGUIGraphDataLLWrapperData *) wrapper_data;
    uint16_t data_chunk_offset = (uint16_t)chunk_offset;
    uint16_t data_length = (uint16_t)stream_length;
    uint32_t data_chunk_written = 59;

    uint8_t *data_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_lcd_128x64_set_gui_graph_data_low_level((TF_LCD128x64 *)device, data->index, data_length, data_chunk_offset, data_chunk_data);

    *ret_chunk_written = (uint32_t) data_chunk_written;
    return ret;
}

int tf_lcd_128x64_set_gui_graph_data(TF_LCD128x64 *lcd_128x64, uint8_t index, const uint8_t *data, uint16_t data_length) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_LCD128x64_SetGUIGraphDataLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.index = index;

    uint32_t _stream_length = data_length;
    uint32_t _data_written = 0;
    uint8_t _chunk_data[59];

    int ret = tf_stream_in(lcd_128x64, tf_lcd_128x64_set_gui_graph_data_ll_wrapper, &_wrapper_data, data, _stream_length, _chunk_data, &_data_written, 59, tf_copy_items_uint8_t);



    return ret;
}


typedef struct TF_LCD128x64_GetGUIGraphDataLLWrapperData {
    uint8_t index;
} TF_LCD128x64_GetGUIGraphDataLLWrapperData;


static int tf_lcd_128x64_get_gui_graph_data_ll_wrapper(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *chunk_data) {
    TF_LCD128x64_GetGUIGraphDataLLWrapperData *data = (TF_LCD128x64_GetGUIGraphDataLLWrapperData *) wrapper_data;
    uint16_t data_length = 0;
    uint16_t data_chunk_offset = 0;
    uint8_t *data_chunk_data = (uint8_t *) chunk_data;
    int ret = tf_lcd_128x64_get_gui_graph_data_low_level((TF_LCD128x64 *)device, data->index, &data_length, &data_chunk_offset, data_chunk_data);

    *ret_stream_length = (uint32_t)data_length;
    *ret_chunk_offset = (uint32_t)data_chunk_offset;
    return ret;
}

int tf_lcd_128x64_get_gui_graph_data(TF_LCD128x64 *lcd_128x64, uint8_t index, uint8_t *ret_data, uint16_t *ret_data_length) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_LCD128x64_GetGUIGraphDataLLWrapperData _wrapper_data;
    memset(&_wrapper_data, 0, sizeof(_wrapper_data));
    _wrapper_data.index = index;
    uint32_t _data_length = 0;
    uint8_t _data_chunk_data[59];

    int ret = tf_stream_out(lcd_128x64, tf_lcd_128x64_get_gui_graph_data_ll_wrapper, &_wrapper_data, ret_data, &_data_length, _data_chunk_data, 59, tf_copy_items_uint8_t);

    if (ret_data_length != NULL) {
        *ret_data_length = (uint16_t)_data_length;
    }
    return ret;
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_lcd_128x64_register_touch_position_callback(TF_LCD128x64 *lcd_128x64, TF_LCD128x64_TouchPositionHandler handler, void *user_data) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    lcd_128x64->touch_position_handler = handler;
    lcd_128x64->touch_position_user_data = user_data;

    return TF_E_OK;
}


int tf_lcd_128x64_register_touch_gesture_callback(TF_LCD128x64 *lcd_128x64, TF_LCD128x64_TouchGestureHandler handler, void *user_data) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    lcd_128x64->touch_gesture_handler = handler;
    lcd_128x64->touch_gesture_user_data = user_data;

    return TF_E_OK;
}


int tf_lcd_128x64_register_gui_button_pressed_callback(TF_LCD128x64 *lcd_128x64, TF_LCD128x64_GUIButtonPressedHandler handler, void *user_data) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    lcd_128x64->gui_button_pressed_handler = handler;
    lcd_128x64->gui_button_pressed_user_data = user_data;

    return TF_E_OK;
}


int tf_lcd_128x64_register_gui_slider_value_callback(TF_LCD128x64 *lcd_128x64, TF_LCD128x64_GUISliderValueHandler handler, void *user_data) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    lcd_128x64->gui_slider_value_handler = handler;
    lcd_128x64->gui_slider_value_user_data = user_data;

    return TF_E_OK;
}


int tf_lcd_128x64_register_gui_tab_selected_callback(TF_LCD128x64 *lcd_128x64, TF_LCD128x64_GUITabSelectedHandler handler, void *user_data) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    lcd_128x64->gui_tab_selected_handler = handler;
    lcd_128x64->gui_tab_selected_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_lcd_128x64_callback_tick(TF_LCD128x64 *lcd_128x64, uint32_t timeout_us) {
    if (lcd_128x64 == NULL) {
        return TF_E_NULL;
    }

    if (lcd_128x64->magic != 0x5446 || lcd_128x64->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = lcd_128x64->tfp->spitfp->hal;

    return tf_tfp_callback_tick(lcd_128x64->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
