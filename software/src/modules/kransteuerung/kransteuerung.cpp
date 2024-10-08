/* warp-charger
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "kransteuerung.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "bindings/errors.h"
#include "tools.h"

extern TF_HAL hal;

void Kransteuerung::button_pressed_handler(bool left, uint8_t button_l, uint8_t button_r, uint8_t led_l, uint8_t led_r)
{
    bool value[4] = {false, false, false, false};
    tf_industrial_quad_relay_v2_get_value(&relay, value);

    if (button_l != last_button_states[left ? 0 : 2] && button_l == TF_DUAL_BUTTON_V2_BUTTON_STATE_PRESSED) {
        tf_industrial_quad_relay_v2_set_selected_value(&relay, (left ? 0 : 2), !value[(left ? 0 : 2)]);
    }

    if (button_r != last_button_states[left ? 1 : 3] && button_r == TF_DUAL_BUTTON_V2_BUTTON_STATE_PRESSED) {
        tf_industrial_quad_relay_v2_set_selected_value(&relay, (left ? 1 : 3), !value[(left ? 1 : 3)]);
    }

    last_button_states[left ? 0 : 2] = button_l;
    last_button_states[left ? 1 : 3] = button_r;
}

void Kransteuerung::setup()
{
    int result = tf_dual_button_v2_create(&left, "D", &hal);
    if (result != TF_E_OK) {
        logger.printfln("Failed to initialize left Dual Button 2.0 Bricklet.");
        return;
    }

    result = tf_dual_button_v2_create(&right, "C", &hal);
    if (result != TF_E_OK) {
        logger.printfln("Failed to initialize right Dual Button 2.0 Bricklet.");
        return;
    }

    result = tf_industrial_quad_relay_v2_create(&relay, "E", &hal);
    if (result != TF_E_OK) {
        logger.printfln("Failed to initialize Industrial Quad Relay 2.0 Bricklet.");
        return;
    }

    bool setup_value[4] = {false, false, false, false};
    tf_industrial_quad_relay_v2_get_value(&relay, setup_value);

    tf_dual_button_v2_set_led_state(&left,  TF_DUAL_BUTTON_V2_LED_STATE_ON, setup_value[1] ? TF_DUAL_BUTTON_V2_LED_STATE_ON : TF_DUAL_BUTTON_V2_LED_STATE_OFF);
    tf_dual_button_v2_set_led_state(&right, setup_value[2] ? TF_DUAL_BUTTON_V2_LED_STATE_ON : TF_DUAL_BUTTON_V2_LED_STATE_OFF, setup_value[3] ? TF_DUAL_BUTTON_V2_LED_STATE_ON : TF_DUAL_BUTTON_V2_LED_STATE_OFF);


    tf_dual_button_v2_register_state_changed_callback(&left, [](struct TF_DualButtonV2 *device, uint8_t button_l, uint8_t button_r, uint8_t led_l, uint8_t led_r, void *user_data){
        task_scheduler.scheduleOnce([button_l, button_r, led_l, led_r, user_data](){
            ((Kransteuerung*)user_data)->button_pressed_handler(true, button_l, button_r, led_l, led_r);
        });
    }, this);

    tf_dual_button_v2_register_state_changed_callback(&right, [](struct TF_DualButtonV2 *device, uint8_t button_l, uint8_t button_r, uint8_t led_l, uint8_t led_r, void *user_data){
        task_scheduler.scheduleOnce([button_l, button_r, led_l, led_r, user_data](){
            ((Kransteuerung*)user_data)->button_pressed_handler(false, button_l, button_r, led_l, led_r);
        });
    }, this);

    tf_dual_button_v2_set_state_changed_callback_configuration(&left, true);
    tf_dual_button_v2_set_state_changed_callback_configuration(&right, true);

    task_scheduler.scheduleWithFixedDelay([this](){
        bool task_value[4] = {false, false, false, false};
        tf_industrial_quad_relay_v2_get_value(&relay, task_value);
        tf_dual_button_v2_set_led_state(&left,  TF_DUAL_BUTTON_V2_LED_STATE_ON, task_value[1] ? TF_DUAL_BUTTON_V2_LED_STATE_ON : TF_DUAL_BUTTON_V2_LED_STATE_OFF);
        tf_dual_button_v2_set_led_state(&right, task_value[2] ? TF_DUAL_BUTTON_V2_LED_STATE_ON : TF_DUAL_BUTTON_V2_LED_STATE_OFF, task_value[3] ? TF_DUAL_BUTTON_V2_LED_STATE_ON : TF_DUAL_BUTTON_V2_LED_STATE_OFF);
    }, 100_ms, 100_ms);

    initialized = true;
}
