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

#include "p9_eeg.h"

#include <algorithm>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"

#include "gcc_warnings.h"

static bool is_valid_relay_level(uint8_t level)
{
    return level == 0 || level == 30 || level == 60 || level == P9_EEG_RELAY_UNUSED;
}

void P9Eeg::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
        {"source", Config::Enum(P9EegSource::EEBus)},
        {"inverter_power_w", Config::Uint32(0)}, // Rated power of the inverter
        {"relays", Config::Tuple(P9_EEG_RELAY_COUNT, Config::Uint8(P9_EEG_RELAY_UNUSED))}, // Level in % that the inverter applies while the relay is closed. No relay closed is always 100 %.
        {"default_level_pct", Config::Uint8(100)}, // Used as long as the source has not provided a limit, for example after a reboot
    }), [this](Config &update, ConfigSource /*source*/) -> String {
        Config *relays = static_cast<Config *>(update.get("relays"));
        bool any_relay_used = false;

        for (size_t i = 0; i < relays->count(); i++) {
            const uint8_t level = relays->get(i)->asUint8();
            if (!is_valid_relay_level(level)) {
                return "Relay level must be 0, 30, 60 or 255 (not used).";
            }
            if (level == P9_EEG_RELAY_UNUSED) {
                continue;
            }
            any_relay_used = true;
            for (size_t j = 0; j < i; j++) {
                if (relays->get(j)->asUint8() == level) {
                    return "Two relays cannot be connected to the same level.";
                }
            }
        }

        const uint8_t default_level = update.get("default_level_pct")->asUint8();
        bool default_level_available = default_level == 100;
        for (size_t i = 0; i < relays->count(); i++) {
            if (relays->get(i)->asUint8() == default_level) {
                default_level_available = true;
            }
        }
        if (!default_level_available) {
            return "Default level must be 100 % or the level of a used relay.";
        }

        const bool will_enable = update.get("enable")->asBool();

        if (will_enable) {
            if (update.get("inverter_power_w")->asUint() == 0) {
                return "Inverter rated power must be set.";
            }
            if (!any_relay_used) {
                return "At least one relay must be used.";
            }
        }

        if (!config.get("enable")->asBool() && will_enable) {
            // Report the initial state to the automation module once the module gets enabled, same as on boot.
            last_trigger_state = TriggerState::Unknown;
        }

#if MODULE_AUTOMATION_AVAILABLE()
        automation.set_enabled(AutomationTriggerID::P9Eeg, will_enable);
#endif

        task_scheduler.scheduleOnce([this]() {
            this->update();
        });

        return "";
    }};

    state = Config::Object({
        {"limit_active", Config::Bool(false)},   // The source requests a production limit
        {"limit_w", Config::Uint32(0)},          // Requested production limit (0 if not limited)
        {"level_pct", Config::Uint8(100)},       // Level that is signalled to the inverter
        {"default_active", Config::Bool(false)}, // The source has not provided a limit yet, the default level is used
        {"limit_reachable", Config::Bool(true)}, // false: The requested limit is below the lowest available level
    });

    control = Config::Object({
        {"active", Config::Bool(false)},
        {"limit_w", Config::Uint32(0)},
    });

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_trigger(
        AutomationTriggerID::P9Eeg,
        Config::Object({
            {"limited", Config::Bool(true)} // true = §9 EEG limits the power production (level below 100 %), false = not limited
        }),
        nullptr,
        false);
#endif
}

void P9Eeg::setup()
{
    api.restorePersistentConfig("p9_eeg/config", &config);
    update_lpp_failsafe();

#if MODULE_AUTOMATION_AVAILABLE()
    automation.set_enabled(AutomationTriggerID::P9Eeg, is_enabled());
#endif

    task_scheduler.scheduleOnce([this]() {
        this->update();
    });

    // Corrects the relays if they were switched otherwise, for example by a reset of the Energy Manager Bricklet
    task_scheduler.scheduleUncancelable([this]() {
        this->apply_relays(false);
    }, 1_s, 1_s);

    initialized = true;
}

void P9Eeg::register_urls()
{
    api.addPersistentConfig("p9_eeg/config", &config);
    api.addState("p9_eeg/state", &state);

    // Volatile, the default level is used after a reboot until the first control update
    api.addState("p9_eeg/control", &control);
    api.addCommand(
        "p9_eeg/control_update",
        &control,
        {},
        [this](Language /*language*/, String & /*errmsg*/) {
            api_control_written = true;
            if (config.get("source")->asEnum<P9EegSource>() == P9EegSource::API) {
                this->update();
            }
        },
        true);
}

bool P9Eeg::is_relay_in_use(uint32_t index)
{
    if (!is_enabled() || index >= P9_EEG_RELAY_COUNT) {
        return false;
    }

    return config.get("relays")->get(index)->asUint8() != P9_EEG_RELAY_UNUSED;
}

void P9Eeg::set_eebus_limit(bool available, bool limited, uint32_t limit_w)
{
    if (available == eebus_available && limited == eebus_limited && limit_w == eebus_limit_w) {
        return;
    }

    eebus_available = available;
    eebus_limited = limited;
    eebus_limit_w = limit_w;

    if (config.get("source")->asEnum<P9EegSource>() == P9EegSource::EEBus) {
        update();
    }
}

void P9Eeg::update_lpp_failsafe()
{
#if MODULE_EEBUS_AVAILABLE()
    // LPP-021/1: The default level is the pre-configured failsafe limit. 100 % means no limit, LPP then uses its nominal maximum power.
    int power_w = -1;
    const uint8_t default_level = config.get("default_level_pct")->asUint8();
    if (is_enabled() && config.get("source")->asEnum<P9EegSource>() == P9EegSource::EEBus && default_level < 100) {
        // Rounded up, so that level_for_limit() maps the failsafe limit back to the default level
        const uint64_t inverter_power_w = config.get("inverter_power_w")->asUint();
        power_w = static_cast<int>((inverter_power_w * default_level + 99) / 100);
    }
    eebus.set_lpp_preconfigured_failsafe(power_w);
#endif
}

uint8_t P9Eeg::level_for_limit(uint32_t limit_w, bool *reachable)
{
    const uint64_t inverter_power_w = config.get("inverter_power_w")->asUint();
    const uint64_t limit_w_x100 = static_cast<uint64_t>(limit_w) * 100;

    uint8_t best = P9_EEG_RELAY_UNUSED;
    uint8_t lowest = 100;

    const auto consider = [&](uint8_t level) {
        lowest = std::min(lowest, level);
        if (level * inverter_power_w <= limit_w_x100 && (best == P9_EEG_RELAY_UNUSED || level > best)) {
            best = level;
        }
    };

    consider(100);
    for (size_t i = 0; i < P9_EEG_RELAY_COUNT; i++) {
        const uint8_t level = config.get("relays")->get(i)->asUint8();
        if (level != P9_EEG_RELAY_UNUSED) {
            consider(level);
        }
    }

    if (best == P9_EEG_RELAY_UNUSED) {
        *reachable = false;
        return lowest;
    }

    *reachable = true;
    return best;
}

void P9Eeg::update()
{
    update_lpp_failsafe();

    if (!is_enabled()) {
        state.get("limit_active")->updateBool(false);
        state.get("limit_w")->updateUint(0);
        state.get("level_pct")->updateUint(100);
        state.get("default_active")->updateBool(false);
        state.get("limit_reachable")->updateBool(true);
        apply_relays(true);
        update_trigger(false);
        return;
    }

    bool available = false;
    bool limited = false;
    uint32_t limit_w = 0;

    switch (config.get("source")->asEnum<P9EegSource>()) {
        case P9EegSource::EEBus:
            available = eebus_available;
            limited = eebus_limited;
            limit_w = eebus_limit_w;
            break;

        case P9EegSource::API:
            available = api_control_written;
            limited = control.get("active")->asBool();
            limit_w = control.get("limit_w")->asUint();
            break;

        default:
            break;
    }

    uint8_t level = 100;
    bool reachable = true;

    if (!available) {
        level = config.get("default_level_pct")->asUint8();
        limited = false;
    } else if (limited) {
        level = level_for_limit(limit_w, &reachable);
    }

    state.get("limit_active")->updateBool(limited);
    state.get("limit_w")->updateUint(limited ? limit_w : 0);
    bool changed = state.get("level_pct")->updateUint(level);
    changed |= state.get("default_active")->updateBool(!available);
    changed |= state.get("limit_reachable")->updateBool(reachable);

    if (changed) {
        if (!available) {
            logger.printfln("No limit from the source yet. Using default level of %hhu %%", level);
        } else if (!limited) {
            logger.printfln("Power production not limited");
        } else if (!reachable) {
            logger.printfln("Limit of %lu W is below the lowest available level. Limiting power production to %hhu %%", limit_w, level);
        } else {
            logger.printfln("Limit of %lu W requested. Limiting power production to %hhu %%", limit_w, level);
        }
    }

    apply_relays(true);
    update_trigger(level < 100);
}

void P9Eeg::update_trigger(bool limited)
{
    const TriggerState new_state = limited ? TriggerState::Limited : TriggerState::NotLimited;
    if (new_state == last_trigger_state) {
        return;
    }

    const bool was_unknown = last_trigger_state == TriggerState::Unknown;
    last_trigger_state = new_state;

    if (was_unknown && !is_enabled()) {
        return;
    }

#if MODULE_AUTOMATION_AVAILABLE()
    task_scheduler.scheduleOnce([this, limited]() {
        bool limited_ = limited;
        automation.trigger(AutomationTriggerID::P9Eeg, &limited_, this);
    });
#endif
}

#if MODULE_AUTOMATION_AVAILABLE()
bool P9Eeg::has_triggered(const Config *conf, void *data)
{
    const Config *cfg = static_cast<const Config *>(conf->get());

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"

    switch (conf->getTag<AutomationTriggerID>()) {
        case AutomationTriggerID::P9Eeg:
            return *static_cast<bool *>(data) == cfg->get("limited")->asBool();

        default:
            break;
    }

#pragma GCC diagnostic pop

    return false;
}
#endif

void P9Eeg::apply_relays(bool force)
{
    const EMAllDataCommon *em_data = em_common.get_all_data_common();
    if (em_data == nullptr || !em_data->is_valid || deadline_elapsed(em_data->last_update + 5_s)) {
        return;
    }

    const bool enabled = is_enabled();
    const uint8_t level = state.get("level_pct")->asUint8();

    bool own[P9_EEG_RELAY_COUNT];
    bool closed[P9_EEG_RELAY_COUNT];
    bool mismatch = force;

    for (size_t i = 0; i < P9_EEG_RELAY_COUNT; i++) {
        const uint8_t relay_level = config.get("relays")->get(i)->asUint8();
        own[i] = enabled && relay_level != P9_EEG_RELAY_UNUSED;
        closed[i] = own[i] && relay_level == level;

        if ((own[i] || relay_owned[i]) && get_relay(i) != closed[i]) {
            mismatch = true;
        }
    }

    if (mismatch) {
        // Open first, so that the inverter never sees two closed contacts
        for (size_t i = 0; i < P9_EEG_RELAY_COUNT; i++) {
            if ((own[i] || relay_owned[i]) && !closed[i]) {
                set_relay(i, false);
            }
        }
        for (size_t i = 0; i < P9_EEG_RELAY_COUNT; i++) {
            if (closed[i]) {
                set_relay(i, true);
            }
        }
    }

    for (size_t i = 0; i < P9_EEG_RELAY_COUNT; i++) {
        relay_owned[i] = own[i];
    }
}

bool P9Eeg::get_relay(size_t index)
{
    bool inputs[4];
    size_t inputs_len = ARRAY_SIZE(inputs);
    bool outputs[4];
    size_t outputs_len = ARRAY_SIZE(outputs);
    em_common.get_input_output_states(inputs, &inputs_len, outputs, &outputs_len);

#if MODULE_EM_V1_AVAILABLE()
    const size_t offset = 0;
#else
    const size_t offset = 2; // EM V2: SG Ready outputs first, then the relays
#endif

    return index + offset < outputs_len && outputs[index + offset];
}

void P9Eeg::set_relay(size_t index, bool closed)
{
#if MODULE_EM_V1_AVAILABLE()
    (void)index;
    em_v1.set_output(closed);
#elif MODULE_EM_V2_AVAILABLE()
    em_v2.set_relay_output(static_cast<uint32_t>(index), closed);
#else
    (void)index;
    (void)closed;
#endif
}
