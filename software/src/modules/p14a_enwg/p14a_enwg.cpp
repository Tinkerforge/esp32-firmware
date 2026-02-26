/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include "p14a_enwg.h"

#include <algorithm>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "p14a_enwg_source.enum.h"

void P14aEnwg::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
        {"source", Config::Enum(P14aEnwgSource::Input)},
        {"limit", Config::Uint32(4200)},           // Limit in W when triggered via input
        {"active_on_close", Config::Bool(true)},   // true = active when input closed, false = active when open
        {"input_index", Config::Uint(0, 0, 3)},    // EM only: which of the 4 inputs (0-3)
        {"this_charger", Config::Bool(false)},     // WARP only: apply limit to this charger
        {"managed_chargers", Config::Bool(false)}, // Apply limit to managed chargers
        {"heating", Config::Bool(false)},          // EM only: apply limit to heating
        {"heating_max_power", Config::Uint32(0)},  // EM only: maximum power draw of heating in W
    }), [this](Config &update, ConfigSource source) -> String {
        bool was_enabled = config.get("enable")->asBool();
        bool will_enable = update.get("enable")->asBool();

        if (!was_enabled && will_enable) {
            start_input_check();
        } else if (was_enabled && !will_enable) {
            stop_input_check();
        }

        task_scheduler.scheduleOnce([this]() {
            this->update();
        }, 0_s);

        return "";
    }};

    state = Config::Object({
        {"active", Config::Bool(false)}, // Is §14a currently triggered
        {"limit", Config::Uint32(0)},    // Current effective limit in W (0 when not active)
    });

    control = Config::Object({
        {"active", Config::Bool(false)}, // Set §14a active via API
        {"limit", Config::Uint32(0)},    // Power limit in W when set via API
    });
}

void P14aEnwg::setup()
{
    api.restorePersistentConfig("p14a_enwg/config", &config);

    if (is_enabled()) {
        start_input_check();
    }

    initialized = true;
}

void P14aEnwg::register_urls()
{
    api.addPersistentConfig("p14a_enwg/config", &config);
    api.addState("p14a_enwg/state", &state);

    // "Volatile" control endpoint for API source.
    // Resets to inactive on reboot.
    api.addState("p14a_enwg/control", &control);
    api.addCommand(
        "p14a_enwg/control_update",
        &control,
        {},
        [this](Language /*language*/, String & /*errmsg*/) {
            if (config.get("source")->asEnum<P14aEnwgSource>() == P14aEnwgSource::API) {
                this->update();
            }
        },
        true);
}

void P14aEnwg::register_events()
{
#if MODULE_EVSE_COMMON_AVAILABLE()
    event.registerEvent("evse/gpio_configuration", {}, [this](const Config *) {
        this->check_evse_shutdown_input();
        return EventResult::Deregister;
    });
#endif
}

void P14aEnwg::check_evse_shutdown_input()
{
#if MODULE_EVSE_COMMON_AVAILABLE()
    const Config *gpio_cfg = api.getState("evse/gpio_configuration", false);
    if (gpio_cfg == nullptr) {
        return;
    }

    uint8_t shutdown_input = gpio_cfg->get("shutdown_input")->asUint();

    // Old values 3 and 4 are the §14a 4200W limit options:
    // 3 = limit to 4200W on open, 4 = limit to 4200W on close.
    if ((shutdown_input != 3) && (shutdown_input != 4)) {
        return;
    }

    // Migrate: if p14a_enwg is not yet enabled, auto-configure from EVSE settings.
    if (!config.get("enable")->asBool()) {
        logger.printfln("Migrating EVSE shutdown input §14a configuration to p14a_enwg module.");
        config.get("enable")->updateBool(true);
        config.get("source")->updateUint(static_cast<uint8_t>(P14aEnwgSource::Input));
        config.get("this_charger")->updateBool(true);
        config.get("active_on_close")->updateBool(shutdown_input == 4);
        config.get("limit")->updateUint(4200);
        API::writeConfig("p14a_enwg/config", &config);
        api.callCommand("evse/p14a_enwg_enabled_update", Config::ConfUpdateObject{{
            {"enabled", true}
        }});
        start_input_check();
    }

    // Reset the EVSE shutdown input to "not configured" since p14a_enwg now handles it.
    uint8_t input_val  = gpio_cfg->get("input")->asUint();
    uint8_t output_val = gpio_cfg->get("output")->asUint();
    api.callCommand("evse/gpio_configuration_update", Config::ConfUpdateObject{{
        {"shutdown_input", (uint8_t)0},
        {"input",          input_val},
        {"output",         output_val}
    }});
    logger.printfln("Reset EVSE shutdown input from §14a mode (%u) to not configured.", shutdown_input);
#endif
}

void P14aEnwg::check_inputs()
{
    if (!is_enabled()) {
        return;
    }

    bool input_value = false;
    uint32_t phases = 0;

#if MODULE_EVSE_V2_AVAILABLE()
    input_value = evse_v2.is_shutdown_input_closed();
    phases = evse_common.backend->get_phases();
#endif

#if MODULE_EM_V2_AVAILABLE()
    uint32_t input_index = config.get("input_index")->asUint();
    input_value = em_v2.get_input(input_index);
#endif

    if (input_value != last_input_value || phases != last_phases) {
        last_input_value = input_value;
        last_phases = phases;
        update();
    }
}

void P14aEnwg::update()
{
    if (!is_enabled()) {
        state.get("active")->updateBool(false);
        state.get("limit")->updateUint(0);
#if MODULE_EVSE_COMMON_AVAILABLE()
        if (last_current_mA != 32000) {
            evse_common.set_p14a_enwg_current(32000);
            last_current_mA = 32000;
        }
#endif
        return;
    }

    const P14aEnwgSource source = config.get("source")->asEnum<P14aEnwgSource>();

    bool active = false;
    uint32_t limit_w = 0;

    switch (source) {
        case P14aEnwgSource::Input: {
            bool input_value = false;

#if MODULE_EVSE_V2_AVAILABLE()
            input_value = evse_v2.is_shutdown_input_closed();
#endif

#if MODULE_EM_V2_AVAILABLE()
            uint32_t input_index = config.get("input_index")->asUint();
            input_value = em_v2.get_input(input_index);
#endif

            bool active_on_close = config.get("active_on_close")->asBool();
            // input_value == true means input is closed.
            active = active_on_close ? input_value : !input_value;
            limit_w = config.get("limit")->asUint();
            break;
        }

        case P14aEnwgSource::EEBus:
            active = eebus_active;
            limit_w = eebus_limit_w;
            break;

        case P14aEnwgSource::API:
            active = control.get("active")->asBool();
            limit_w = control.get("limit")->asUint();
            break;
    }

    state.get("active")->updateBool(active);
    state.get("limit")->updateUint(active ? limit_w : 0);

#if MODULE_EVSE_COMMON_AVAILABLE()
    uint16_t new_current_mA = 32000;

    if (config.get("this_charger")->asBool() && active) {
        uint32_t phases = evse_common.backend->get_phases();
        if (phases == 0) {
            phases = 3;
        }
        int limit_mA = static_cast<int>(limit_w) * 1000 / 230 / static_cast<int>(phases);
        limit_mA = std::min(limit_mA, 32000);
        limit_mA = std::max(limit_mA, 6000);
        new_current_mA = static_cast<uint16_t>(limit_mA);
    }

    if (new_current_mA != last_current_mA) {
        evse_common.set_p14a_enwg_current(new_current_mA);
        last_current_mA = new_current_mA;
    }
#endif
}

void P14aEnwg::start_input_check()
{
    if (input_check_task_id != 0) {
        return;
    }

    input_check_task_id = task_scheduler.scheduleWithFixedDelay([this]() {
        this->check_inputs();
    }, 1_s, 1_s);
}

void P14aEnwg::stop_input_check()
{
    if (input_check_task_id == 0) {
        return;
    }

    task_scheduler.cancel(input_check_task_id);
    input_check_task_id = 0;

    // Reset state when stopping
    state.get("active")->updateBool(false);
    state.get("limit")->updateUint(0);

#if MODULE_EVSE_COMMON_AVAILABLE()
    if (last_current_mA != 32000) {
        evse_common.set_p14a_enwg_current(32000);
        last_current_mA = 32000;
    }
#endif
}

void P14aEnwg::set_eebus_limit(bool active, uint32_t limit_w)
{
    eebus_active = active;
    eebus_limit_w = limit_w;

    if (config.get("source")->asEnum<P14aEnwgSource>() == P14aEnwgSource::EEBus) {
        this->update();
    }
}

// Currently heating always gets blocked when §14a is active.
// Later on we will integrate this into the load management,
// then it may stay active if the power limit allows it.
bool P14aEnwg::is_heating_active()
{
    return is_enabled() && state.get("active")->asBool();
}

uint32_t P14aEnwg::get_managed_chargers_limit()
{
    if (!is_enabled() || !config.get("managed_chargers")->asBool() || !state.get("active")->asBool()) {
        return 0;
    }

    return config.get("limit")->asUint();
}
