/* esp32-firmware
 * Copyright (C) 2025 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "battery_control.h"
#include "module_dependencies.h"

#include "event_log_prefix.h"
#include "tools.h"

#include "gcc_warnings.h"

static constexpr const uint16_t MAX_RULES_PER_TYPE = 32;
static_assert(MAX_RULES_PER_TYPE < 256, "MAX_RULES_PER_TYPE must be below 256 to fit into an uint8_t");

void BatteryControl::pre_setup()
{
    config = Config::Object({
        {"block_discharge_during_fast_charge", Config::Bool(false)},
    });

    rule_prototype = Config::Object({
        {"name",          Config::Str ("", 0, 32)},
        {"soc_cond",      Config::Uint  (0, 0, 2)}, // 0 = ignore, 1 = true if below threshold, 2 = true if above threshold
        {"soc_th",        Config::Uint8 (0      )}, // in percent (0 to 100 %)
        {"price_cond",    Config::Uint  (0, 0, 2)},
        {"price_th",      Config::Int16 (0      )}, // in ct/10   (-32 to 32 EUR)
        {"forecast_cond", Config::Uint  (0, 0, 2)},
        {"forecast_th",   Config::Uint16(0      )}, // in kWh     (0 to 65 MWh)
    });

    rules_forbid_discharge   = Config::Array({}, &rule_prototype, 0, MAX_RULES_PER_TYPE, Config::type_id<Config::ConfObject>());
    rules_permit_grid_charge = Config::Array({}, &rule_prototype, 0, MAX_RULES_PER_TYPE, Config::type_id<Config::ConfObject>());

    low_level_config = Config::Object({
        {"rewrite_period", Config::Uint(60, 5, 99999)}, // in seconds
    });

    state = Config::Object({
        {"grid_charge_allowed", Config::Bool(false)},
        {"discharge_blocked", Config::Bool(false)},
    });
}

void BatteryControl::setup()
{
    api.restorePersistentConfig("battery_control/config",                   &config);
    api.restorePersistentConfig("battery_control/rules_forbid_discharge",   &rules_forbid_discharge);
    api.restorePersistentConfig("battery_control/rules_permit_grid_charge", &rules_permit_grid_charge);
    api.restorePersistentConfig("battery_control/low_level_config",         &low_level_config);

    bool have_battery = false;

    for (size_t i = 0; i < BATTERIES_SLOTS; i++) {
        if (batteries.get_battery_class(i) != BatteryClassID::None) {
            have_battery = true;
            break;
        }
    }

    if (have_battery) {
        for (size_t i = 0; i < METERS_SLOTS; i++) {
            if (meters.get_meter_location(i) == MeterLocation::Battery) {
                logger.printfln("Meter %zu is a battery", i); // TODO remove
            }
        }

        bool have_any_rule = config.get("block_discharge_during_fast_charge")->asBool() && charge_manager.get_charger_count() > 0;

        const size_t forbid_discharge_rules_cnt   = rules_forbid_discharge.count();
        const size_t permit_grid_charge_rules_cnt = rules_permit_grid_charge.count();

        const size_t total_rules_cnt = forbid_discharge_rules_count + permit_grid_charge_rules_count;
        if (total_rules_cnt > 0) {
            control_rule *rules = static_cast<decltype(rules)>(malloc(total_rules_cnt * sizeof(*rules)));
            if (!rules) {
                logger.printfln("Failed to allocate memory for rules");
                return;
            }

            if (forbid_discharge_rules_cnt > 0) {
                control_rule *fd_rules       = rules;
                forbid_discharge_rules       = fd_rules;
                forbid_discharge_rules_count = static_cast<uint8_t>(forbid_discharge_rules_cnt);

                preprocess_rules(&rules_forbid_discharge, fd_rules, forbid_discharge_rules_cnt);
            }

            if (permit_grid_charge_rules_cnt > 0) {
                control_rule *pgc_rules        = rules + forbid_discharge_rules_cnt;
                permit_grid_charge_rules       = pgc_rules;
                permit_grid_charge_rules_count = static_cast<uint8_t>(permit_grid_charge_rules_cnt);

                preprocess_rules(&rules_permit_grid_charge, pgc_rules, permit_grid_charge_rules_cnt);
            }

            have_any_rule = true;
        }

        if (have_any_rule) {
            task_scheduler.scheduleWithFixedDelay([this]() {
                bool want_blocked = charge_manager.fast_charger_in_c;
                bool changed = this->discharge_blocked != static_cast<TristateBool>(want_blocked);

                if (changed || deadline_elapsed(next_blocked_update)) {
                    if (want_blocked) {
                        // Block
                        if (changed) {
                            logger.printfln("Blocking discharge");
                        }
                        batteries.start_action_all(IBattery::Action::ForbidDischarge);
                        this->discharge_blocked = TristateBool::True;
                    } else {
                        // Unblock
                        if (changed) {
                            logger.printfln("Unblocking discharge");
                        }
                        batteries.start_action_all(IBattery::Action::RevokeDischargeOverride);
                        this->discharge_blocked = TristateBool::False;
                    }

                    this->state.get("discharge_blocked")->updateBool(static_cast<bool>(this->discharge_blocked));

                    next_blocked_update = now_us() + seconds_t{this->low_level_config.get("rewrite_period")->asUint()};
                }
            }, 5_s, 1_s);
        }
    }
}

void BatteryControl::register_urls()
{
    api.addPersistentConfig("battery_control/config",                   &config);
    api.addPersistentConfig("battery_control/rules_forbid_discharge",   &rules_forbid_discharge);
    api.addPersistentConfig("battery_control/rules_permit_grid_charge", &rules_permit_grid_charge);
    api.addPersistentConfig("battery_control/low_level_config",         &low_level_config);
    api.addState("battery_control/state", &state);
}

void BatteryControl::preprocess_rules(const Config *rules_config, control_rule *rules, size_t rules_count)
{
    for (size_t i = 0; i < rules_count; i++) {
        const auto rule_config = rules_config->get(i);
        control_rule *rule = rules + i;

        rule->soc_cond      =                       rule_config->get("soc_cond"     )->asEnum<RuleCondition>();
        rule->soc_th        = static_cast<uint8_t >(rule_config->get("soc_th"       )->asUint());
        rule->price_cond    =                       rule_config->get("price_cond"   )->asEnum<RuleCondition>();
        rule->price_th      =                       rule_config->get("price_th"     )->asInt()  *  100;         // ct/10 -> ct/1000
        rule->forecast_cond =                       rule_config->get("forecast_cond")->asEnum<RuleCondition>();
        rule->forecast_th   =                       rule_config->get("forecast_th"  )->asUint() * 1000;         // kWh -> Wh
    }
}
