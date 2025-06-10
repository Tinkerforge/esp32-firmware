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
    "forbid_charge",
    "revoke_charge_override",
};

static_assert(ARRAY_SIZE(batteries_path_postfixes) == static_cast<uint32_t>(Batteries::PathType::_max) + 1, "Path postfix length mismatch");

void Batteries::pre_setup()
{
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
                uint32_t slot = config->get("battery_slot")->asUint();
                const String path = get_path(slot, path_type);
                const String errmsg = api.callCommand(path.c_str());

                if (!errmsg.isEmpty()) {
                    String action_name = batteries_path_postfixes[static_cast<uint32_t>(path_type)];
                    action_name.replace("_", " ");

                    logger.printfln_battery("Automation couldn't %s: %s", action_name.c_str(), errmsg.c_str());
                }
            }
        );
    }
#endif
}

void Batteries::setup()
{
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

        if (battery == nullptr) {
            logger.printfln_battery("Failed to create battery of class %lu", static_cast<uint32_t>(configured_battery_class));
            battery = new_battery_of_class(BatteryClassID::None, slot, battery_state, battery_errors);
        }

        battery->setup(*static_cast<const Config *>(battery_slot.config_union.get()));

        battery_slot.battery = battery;
    }

    initialized = true;
}

void Batteries::register_urls()
{
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
                api.addCommand(get_path(slot, path_type), Config::Null(), {}, [slot, battery, action](String &/*errmsg*/) {
                    battery->start_action(action, [slot, action](bool success) {
                        if (!success) {
                         logger.printfln_battery("Failed to start action %lu", static_cast<uint32_t>(action));
                        }
                    });
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
            logger.printfln("Tried to register battery generator for already registered battery class %lu.",
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

    logger.printfln("No generator for battery class %lu.", static_cast<uint32_t>(battery_class));
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

uint32_t Batteries::get_batteries(BatteryClassID battery_class, IBattery **found_batteries, uint32_t found_batteries_capacity)
{
    uint32_t found_count = 0;

    for (uint32_t i = 0; i < OPTIONS_BATTERIES_MAX_SLOTS(); ++i) {
        IBattery *battery = battery_slots[i].battery;

        if (battery->get_class() == battery_class) {
            if (found_count < found_batteries_capacity) {
                found_batteries[found_count] = battery;
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
    for (uint32_t slot = 0; slot < BATTERIES_SLOTS; slot++) {
        IBattery *battery = battery_slots[slot].battery;

        battery->start_action(action, [slot, action](bool success) {
            if (!success) {
                logger.printfln_battery("Failed to start action %lu", static_cast<uint32_t>(action));
            }
        });
    }
}

char *format_battery_slot(uint32_t slot)
{
    char *result;

    if (asprintf(&result, "Battery %lu: ", slot) < 0) {
        result = strdup("Battery ?: ");
    }

    return result;
}
