/* esp32-firmware
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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

#include "evse_led.h"

#include "module_dependencies.h"

void EvseLed::pre_setup()
{
    config = Config::Object({
        {"enable_api", Config::Bool(false)}
    });

    led = Config::Object({
        {"indication", Config::Int(-1, -1, Blink::ErrorEnd)},
        {"duration", Config::Uint16(0)},
        {"color_h", Config::Uint(0, 0, 360)},
        {"color_s", Config::Uint8(0)},
        {"color_v", Config::Uint8(0)},
    });

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_action(
        AutomationActionID::LED,
        Config::Object({
            {"indication", Config::Int(-1)},
            {"duration", Config::Uint16(0)},
            {"color_h", Config::Uint(0, 0, 360)},
            {"color_s", Config::Uint8(0)},
            {"color_v", Config::Uint8(0)},
        }),
        [this](const Config *config) {
            set_module(config->get("indication")->asEnum<Blink>(),
                       config->get("duration")->asUint(),
                       config->get("color_h")->asUint(),
                       config->get("color_s")->asUint(),
                       config->get("color_v")->asUint());
        }
    );
#endif
}

void EvseLed::setup()
{
    api.restorePersistentConfig("evse/led_configuration", &config);
    enable_api = config.get("enable_api")->asBool();

    if (evse_common.get_evse_version() >= 30)
        api.addFeature("rgb_led");

    initialized = true;
}

void EvseLed::register_urls()
{
    api.addPersistentConfig("evse/led_configuration", &config);

    // Has to be named this way, because evse/indicator_led is the corresponding read API.
    api.addCommand("evse/indicator_led_update", &led, {}, [this](){
        this->set_api(
            led.get("indication")->asEnum<EvseLed::Blink>(),
            led.get("duration")->asUint(),
            led.get("color_h")->asUint(),
            led.get("color_s")->asUint(),
            led.get("color_v")->asUint());
    }, true);
}

static bool is_error(EvseLed::Blink state)
{
    return state >= EvseLed::Blink::ErrorStart && state <= EvseLed::Blink::ErrorEnd;
}

static bool is_pwm_or_evse(EvseLed::Blink state)
{
    return state >= EvseLed::Blink::None && state <= EvseLed::Blink::On;
}

static bool is_nfc(EvseLed::Blink state)
{
    return state >= EvseLed::Blink::Ack && state <= EvseLed::Blink::Nag;
}

bool EvseLed::accepts_new_state(Blink new_state)
{
    // Always accept new state if old state duration is elapsed
    if (this->current_duration_end_us == 0_us || deadline_elapsed(this->current_duration_end_us))
        return true;

    /*
              new
          pwm nfc err
    o pwm  1   1   1
    l nfc  0   ?   1
    d err  0   0   1
    */

    // Accept new errors (last column)
    if (is_error(new_state))
        return true;

    // Accept any if we currently show PWM or the EVSE controls the LED (first row)
    if (is_pwm_or_evse(this->current_state))
        return true;

    // Don't accept non-error states if we currently show an error (last row)
    if (is_error(this->current_state))
        return false;

    // Don't accept new PWM or EVSE controlled if we currently show an NFC animation. (first column middle row)
    if (is_nfc(this->current_state) && is_pwm_or_evse(new_state))
        return false;

    // Old and new state are both NFC animations. Prioritize Ack over Nack over Nag.
    switch (new_state) {
        case Blink::Ack:
            return true;
        case Blink::Nack:
            return this->current_state != Blink::Ack;
        case Blink::Nag:
            return this->current_state != Blink::Ack && this->current_state != Blink::Nack;
        // Should be unreachable. But to be sure, accept the new state.
        default:
            return true;
    }
}

bool EvseLed::set(Blink state, uint16_t duration_ms, uint16_t h, uint8_t s, uint8_t v, bool via_api)
{
    if (!accepts_new_state(state))
        return false;

    micros_t new_duration_end_us = now_us() + micros_t{duration_ms * 1000};

    uint8_t error_code = 1;

    evse_common.set_indicator_led(state, duration_ms, h, s, v, &error_code); //Don't set colors for now, let the EVSE choose.

    if (error_code == 0) {
        current_state = state;
        current_duration_end_us = new_duration_end_us;
        current_state_via_api = via_api;
    }
    return error_code == 0;
}

bool EvseLed::set_module(Blink state, uint16_t duration_ms, uint16_t h, uint8_t s, uint8_t v)
{
    // Allow modules to override API only if module reports an error and API does not.
    // Don't allow modules to set non-error states.
    // Don't allow modules to override error states with other error states.
    if (!enable_api || (is_error(state) && !is_error(current_state)))
        return set(state, duration_ms, h, s, v, false);

    return false;
}

bool EvseLed::set_api(Blink state, uint16_t duration_ms, uint16_t h, uint8_t s, uint8_t v)
{
    if (!enable_api)
        return false;

    // Don't allow API to override non-API error states.
    if (!current_state_via_api && is_error(current_state))
        return false;

    return set(state, duration_ms, h, s, v, true);
}
