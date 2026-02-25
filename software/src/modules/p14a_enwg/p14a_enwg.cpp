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
    })};

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

    if (config.get("enable")->asBool()) {
        task_scheduler.scheduleUncancelable([this]() {
            this->update();
        }, 10_s, 1_s);
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

void P14aEnwg::update()
{
    const P14aEnwgSource source = config.get("source")->asEnum<P14aEnwgSource>();

    bool active = false;
    uint32_t limit_w = 0;

    switch (source) {
        case P14aEnwgSource::Input: {
            bool input_value = false;

#if MODULE_EVSE_COMMON_AVAILABLE()
            // On WARP: read shutdown input GPIO.
            // GPIO index 5 for WARP2 (evse_version < 30), GPIO index 18 for WARP3/4 (>= 30).
            uint32_t evse_version = evse_common.get_evse_version();
            uint32_t gpio_index = (evse_version >= 30) ? 18 : 5;
            input_value = evse_common.get_low_level_state().get("gpio")->get(gpio_index)->asBool();
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
    // Apply limit to this charger's P14A_ENWG slot (if enabled).
    if (config.get("this_charger")->asBool()) {
        if (!slot_enabled) {
            api.callCommand("evse/p14a_enwg_enabled_update", Config::ConfUpdateObject{{
                {"enabled", true}
            }});
            slot_enabled = true;
        }

        if (active) {
            uint32_t phases = evse_common.backend->get_phases();
            if (phases == 0) {
                phases = 3;
            }
            int limit_mA = static_cast<int>(limit_w) * 1000 / 230 / static_cast<int>(phases);
            limit_mA = std::min(limit_mA, 32000);
            limit_mA = std::max(limit_mA, 6000);
            evse_common.set_p14a_enwg_current(static_cast<uint16_t>(limit_mA));
        } else {
            evse_common.set_p14a_enwg_current(32000);
        }
    } else {
        // If_charger is disabled: disable the slot.
        if (slot_enabled) {
            api.callCommand("evse/p14a_enwg_enabled_update", Config::ConfUpdateObject{{
                {"enabled", false}
            }});
            slot_enabled = false;
        }
    }
#endif

    // TODO: Apply limit to managed chargers via charge manager .
    // TODO: Apply limit to heating via heating module.
}

void P14aEnwg::set_eebus_limit(bool active, uint32_t limit_w)
{
    eebus_active = active;
    eebus_limit_w = limit_w;

    if (config.get("source")->asEnum<P14aEnwgSource>() == P14aEnwgSource::EEBus) {
        this->update();
    }
}
