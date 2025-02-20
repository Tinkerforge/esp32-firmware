/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#include "batteries.h"

#include <Arduino.h>

#include "options.h"
#include "battery_class_none.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"

static BatteryGeneratorNone battery_generator_none;

static const char *batteries_path_postfixes[] = {
    "",
    "config",
    "state",
    "errors",
    "permit_grid_charge",
    "revoke_grid_charge_override",
    "forbid_discharge",
    "revoke_discharge_override",
};

static_assert(ARRAY_SIZE(batteries_path_postfixes) == static_cast<uint32_t>(Batteries::PathType::_max) + 1, "Path postfix length mismatch");

void Batteries::pre_setup()
{
    config = Config::Object({
        {"block_discharge_during_fast_charge", Config::Bool(false)},
    });

    low_level_config = Config::Object({
        {"rewrite_period", Config::Uint(60, 5, 99999)}, // in seconds
    });

    state = Config::Object({
        {"grid_charge_allowed", Config::Bool(false)},
        {"discharge_blocked", Config::Bool(false)},
    });

    generators.reserve(BATTERY_CLASS_ID_COUNT);
    register_battery_generator(BatteryClassID::None, &battery_generator_none);

#if MODULE_AUTOMATION_AVAILABLE()
    for (uint32_t i = 0; i <= static_cast<uint32_t>(IBattery::Action::_max); ++i) {
        AutomationActionID automation_id = static_cast<AutomationActionID>(static_cast<uint32_t>(AutomationActionID::PermitGridCharge) + i);
        Batteries::PathType path_type = static_cast<Batteries::PathType>(static_cast<uint32_t>(Batteries::PathType::PermitGridCharge) + i);

        automation.register_action(
            automation_id,
            Config::Object({
                {"battery_slot", Config::Uint(0, 0, BATTERIES_SLOTS - 1)}
            }),
            [this, path_type](const Config *config) {
                uint32_t battery_slot = config->get("battery_slot")->asUint();
                const String path = get_path(battery_slot, path_type);
                const String errmsg = api.callCommand(path.c_str());

                if (!errmsg.isEmpty()) {
                    String action_name = batteries_path_postfixes[static_cast<uint32_t>(path_type)];
                    action_name.replace("_", " ");

                    logger.printfln("Automation couldn't %s for battery slot %u: %s", action_name.c_str(), battery_slot, errmsg.c_str());
                }
            }
        );
    }
#endif
}

void Batteries::setup()
{
    api.restorePersistentConfig("battery_control/config", &config);

    initialized = true;

    generators.shrink_to_fit();

    // Create config prototypes, depending on available generators.
    uint8_t class_count = static_cast<uint8_t>(generators.size());
    ConfUnionPrototype<BatteryClassID> *config_prototypes = new ConfUnionPrototype<BatteryClassID>[class_count];

    for (uint32_t i = 0; i < class_count; i++) {
        const auto &generator_tuple = generators[i];
        BatteryClassID battery_class = std::get<0>(generator_tuple);
        auto battery_generator = std::get<1>(generator_tuple);
        config_prototypes[i] = {battery_class, *battery_generator->get_config_prototype()};
    }

    bool have_battery = false;

    for (uint32_t slot = 0; slot < OPTIONS_BATTERIES_MAX_SLOTS(); slot++) {
        BatterySlot &battery_slot = battery_slots[slot];
        BatteryClassID battery_class = BatteryClassID::None;

        // Initialize config.
        battery_slot.config_union =
            Config::Union(*get_generator_for_class(battery_class)->get_config_prototype(), battery_class, config_prototypes, class_count);

        // Load config.
        api.restorePersistentConfig(get_path(slot, Batteries::PathType::Config), &battery_slot.config_union);

        BatteryClassID configured_battery_class = battery_slot.config_union.getTag<BatteryClassID>();

        // Generator might be a NONE class generator if the requested class is not available.
        IBatteryGenerator *generator = get_generator_for_class(configured_battery_class);

        // Initialize state and errors to match (loaded) config.
        battery_slot.state = *generator->get_state_prototype();
        battery_slot.errors = *generator->get_errors_prototype();

        // Create battery from config.
        Config *battery_state = &battery_slot.state;
        Config *battery_errors = &battery_slot.errors;

        IBattery *battery = new_battery_of_class(configured_battery_class, slot, battery_state, battery_errors);

        if (!battery) {
            logger.printfln("Failed to create battery of class %u in slot %u.", static_cast<uint32_t>(configured_battery_class), slot);
            battery = new_battery_of_class(BatteryClassID::None, slot, battery_state, battery_errors);
        }

        if (battery->get_class() != BatteryClassID::None) {
            have_battery = true;
        }

        battery->setup(*static_cast<const Config *>(battery_slot.config_union.get()));

        battery_slot.battery = battery;
    }

    const bool block_discharge = config.get("block_discharge_during_fast_charge")->asBool();

    if (block_discharge && have_battery && charge_manager.get_charger_count() > 0) {
        task_scheduler.scheduleWithFixedDelay([this]() {
            bool want_blocked = charge_manager.fast_charger_in_c;
            bool changed = this->discharge_blocked != static_cast<TristateBool>(want_blocked);

            if (changed || deadline_elapsed(next_blocked_update)) {
                if (want_blocked) {
                    // Block
                    if (changed) {
                        logger.printfln("Blocking discharge");
                    }
                    this->start_action_all(IBattery::Action::ForbidDischarge);
                    this->discharge_blocked = TristateBool::True;
                } else {
                    // Unblock
                    if (changed) {
                        logger.printfln("Unblocking discharge");
                    }
                    this->start_action_all(IBattery::Action::RevokeDischargeOverride);
                    this->discharge_blocked = TristateBool::False;
                }

                this->state.get("discharge_blocked")->updateBool(static_cast<bool>(this->discharge_blocked));

                next_blocked_update = now_us() + seconds_t{this->low_level_config.get("rewrite_period")->asUint()};
            }
        }, 3_s, 1_s);
    }
}

void Batteries::register_urls()
{
    api.addPersistentConfig("battery_control/config", &config);
    api.addPersistentConfig("battery_control/low_level_config", &low_level_config);
    api.addState("battery_control/state", &state);

    for (uint32_t slot = 0; slot < OPTIONS_BATTERIES_MAX_SLOTS(); slot++) {
        BatterySlot &battery_slot = battery_slots[slot];
        IBattery *battery = battery_slot.battery;

        api.addPersistentConfig(get_path(slot, Batteries::PathType::Config), &battery_slot.config_union);
        api.addState(get_path(slot, Batteries::PathType::State), &battery_slot.state);
        api.addState(get_path(slot, Batteries::PathType::Errors), &battery_slot.errors);

        for (uint32_t i = 0; i <= static_cast<uint32_t>(IBattery::Action::_max); ++i) {
            IBattery::Action action = static_cast<IBattery::Action>(i);
            Batteries::PathType path_type = static_cast<Batteries::PathType>(static_cast<uint32_t>(Batteries::PathType::PermitGridCharge) + i);

            if (battery->supports_action(action)) {
                api.addCommand(get_path(slot, path_type), Config::Null(), {}, [battery, action](String &errmsg) {
                    IBattery::Action current_action;

                    if (battery->get_current_action(&current_action)) {
                        errmsg = "Another action is already in progress";
                        return;
                    }

                    if (!battery->start_action(action)) {
                        errmsg = "Could not start action";
                        return;
                    }
                }, true);
            }

            battery->register_urls(get_path(slot, Batteries::PathType::Base));
        }
    }
}

void Batteries::register_events()
{
    for (uint32_t slot = 0; slot < OPTIONS_BATTERIES_MAX_SLOTS(); slot++) {
        battery_slots[slot].battery->register_events();
    }
}

void Batteries::pre_reboot()
{
    for (uint32_t slot = 0; slot < OPTIONS_BATTERIES_MAX_SLOTS(); slot++) {
        battery_slots[slot].battery->pre_reboot();
    }
}

void Batteries::register_battery_generator(BatteryClassID battery_class, IBatteryGenerator *generator)
{
    for (const auto &generator_tuple : generators) {
        BatteryClassID known_class = std::get<0>(generator_tuple);

        if (battery_class == known_class) {
            logger.printfln("Tried to register battery generator for already registered battery class %u.",
                            static_cast<uint32_t>(battery_class));
            return;
        }
    }

    generators.push_back({battery_class, generator});
}

IBatteryGenerator *Batteries::get_generator_for_class(BatteryClassID battery_class)
{
    for (auto generator_tuple : generators) {
        BatteryClassID known_class = std::get<0>(generator_tuple);

        if (battery_class == known_class) {
            return std::get<1>(generator_tuple);
        }
    }

    if (battery_class == BatteryClassID::None) {
        logger.printfln("No generator for dummy battery available. This is probably fatal.");
        return nullptr;
    }

    logger.printfln("No generator for battery class %u.", static_cast<uint32_t>(battery_class));
    return get_generator_for_class(BatteryClassID::None);
}

IBattery *Batteries::new_battery_of_class(BatteryClassID battery_class, uint32_t slot, Config *state, Config *errors)
{
    IBatteryGenerator *generator = get_generator_for_class(battery_class);

    if (!generator) {
        return nullptr;
    }

    return generator->new_battery(slot, state, errors);
}

IBattery *Batteries::get_battery(uint32_t slot)
{
    if (slot >= OPTIONS_BATTERIES_MAX_SLOTS()) {
        return nullptr;
    }

    return battery_slots[slot].battery;
}

uint32_t Batteries::get_batterys(BatteryClassID battery_class, IBattery **found_batterys, uint32_t found_batterys_capacity)
{
    uint32_t found_count = 0;

    for (uint32_t i = 0; i < OPTIONS_BATTERIES_MAX_SLOTS(); ++i) {
        IBattery *battery = battery_slots[i].battery;

        if (battery->get_class() == battery_class) {
            if (found_count < found_batterys_capacity) {
                found_batterys[found_count] = battery;
            }

            ++found_count;
        }
    }

    return found_count;
}

BatteryClassID Batteries::get_battery_class(uint32_t slot)
{
    if (slot >= OPTIONS_BATTERIES_MAX_SLOTS()) {
        return BatteryClassID::None;
    }

    return battery_slots[slot].battery->get_class();
}

String Batteries::get_path(uint32_t slot, Batteries::PathType path_type)
{
    String path = "batteries/";

    path.concat(slot);
    path.concat('/');
    path.concat(batteries_path_postfixes[static_cast<uint32_t>(path_type)]);

    return path;
}

void Batteries::start_action_all(IBattery::Action action)
{
    for (size_t i = 0; i < BATTERIES_SLOTS; i++) {
        IBattery *battery = battery_slots[i].battery;

        if (!battery->start_action(action)) {
            logger.printfln("Battery %zu failed to start action %u", i, static_cast<uint32_t>(action));
        }
    }
}
