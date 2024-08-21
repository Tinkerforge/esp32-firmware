/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "automation.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"

Automation::Automation()
{
    trigger_prototypes.push_back({AutomationTriggerID::None, *Config::Null()});
    action_prototypes.push_back({AutomationActionID::None, *Config::Null()});
}

void Automation::pre_setup()
{
    register_action(
        AutomationActionID::Print,
        Config::Object({
            {"message", Config::Str("", 0, 64)}
        }),
        [this](const Config *cfg) {
            logger.printfln_plain("    %s", cfg->get("message")->asString().c_str());
        }
    );

    // TODO: disable here and enable whenever settimeofday is called.
    register_trigger(
        AutomationTriggerID::Cron,
        Config::Object({
            {"mday", Config::Int(-1, -1, 32)},
            {"wday", Config::Int(-1, -1, 9)},
            {"hour", Config::Int(-1, -1, 23)},
            {"minute", Config::Int(-1, -1, 59)}
        })
    );

    Config trigger_union = Config::Union<AutomationTriggerID>(
                    *Config::Null(),
                    AutomationTriggerID::None,
                    trigger_prototypes.data(),
                    trigger_prototypes.size());

    Config action_union = Config::Union<AutomationActionID>(
                    *Config::Null(),
                    AutomationActionID::None,
                    action_prototypes.data(),
                    action_prototypes.size());

    config = ConfigRoot{Config::Object({
            {"tasks", Config::Array(
                {},
                new Config{
                    Config::Object({
                        {"trigger", trigger_union},
                        {"action", action_union}
                    })
                }, 0, 14, Config::type_id<Config::ConfObject>())
            }
        }),
        [this](const Config &cfg, ConfigSource source) -> String {
            for (const Config &task : cfg.get("tasks")) {
                const Config *action = static_cast<const Config *>(task.get("action"));
                AutomationActionID action_id = action->getTag<AutomationActionID>();
                if (action_id == AutomationActionID::None) {
                    return "ActionID must not be 0!";
                }

                ValidatorCb &action_validator = this->action_map[action_id].validator;
                if (action_validator) {
                    String ret = action_validator(static_cast<const Config *>(action->get()));
                    if (!ret.isEmpty()) {
                        return ret;
                    }
                }

                const Config *trigger = static_cast<const Config *>(task.get("trigger"));
                AutomationTriggerID trigger_id = trigger->getTag<AutomationTriggerID>();
                if (trigger_id == AutomationTriggerID::None) {
                    return "TriggerID must not be 0!";
                }

                ValidatorCb &trigger_validator = this->trigger_map[trigger_id].validator;
                if (trigger_validator) {
                    String ret = trigger_validator(static_cast<const Config *>(trigger->get()));
                    if (!ret.isEmpty()) {
                        return ret;
                    }
                }
            }

            return "";
        }
    };

    const Config *conf_uint8_prototype = new Config{Config::Uint8(0)};

    state = Config::Object({
        {"registered_triggers", Config::Array({}, conf_uint8_prototype, 0, AUTOMATION_TRIGGER_ID_COUNT, Config::type_id<Config::ConfUint>())},
        {"registered_actions",  Config::Array({}, conf_uint8_prototype, 0, AUTOMATION_ACTION_ID_COUNT,  Config::type_id<Config::ConfUint>())},
        {"enabled_triggers",    Config::Array({}, conf_uint8_prototype, 0, AUTOMATION_TRIGGER_ID_COUNT, Config::type_id<Config::ConfUint>())},
        {"enabled_actions",     Config::Array({}, conf_uint8_prototype, 0, AUTOMATION_ACTION_ID_COUNT,  Config::type_id<Config::ConfUint>())},
    });

    for (auto const &trigger : trigger_map) {
        state.get("registered_triggers")->add()->updateEnum(trigger.first);

        if (trigger.second.enable) {
            state.get("enabled_triggers")->add()->updateEnum(trigger.first);
        }
    }

    for (auto const &action : action_map) {
        state.get("registered_actions")->add()->updateEnum(action.first);

        if (action.second.enable) {
            state.get("enabled_actions")->add()->updateEnum(action.first);
        }
    }
}

void Automation::setup()
{
    api.restorePersistentConfig("automation/config", &config);

    config_in_use = config;

    if (has_task_with_trigger(AutomationTriggerID::Cron)) {
        task_scheduler.scheduleWithFixedDelay([this]() {
            static int last_min = 0;
            static bool was_synced = false;
            timeval tv;
            bool is_synced = clock_synced(&tv);
            tm time_struct;

            localtime_r(&tv.tv_sec, &time_struct);

            if (was_synced && time_struct.tm_min != last_min) {
                trigger(AutomationTriggerID::Cron, &time_struct, this);
            }

            last_min = time_struct.tm_min;
            was_synced = is_synced;
        }, 0, 1000);
    }

    initialized = true;
}

void Automation::register_urls()
{
    api.addPersistentConfig("automation/config", &config);
    api.addState("automation/state", &state);
}

void Automation::register_action(AutomationActionID id, Config cfg, ActionCb &&callback, ValidatorCb &&validator, bool enable)
{
    if (action_map.find(id) != action_map.end()) {
        logger.printfln("Action %u is already registered", static_cast<uint>(id));
        return;
    }

    action_prototypes.push_back({id, cfg});
    action_map[id] = ActionValue{std::forward<ActionCb>(callback), std::forward<ValidatorCb>(validator), enable};
}

void Automation::register_trigger(AutomationTriggerID id, Config cfg, ValidatorCb &&validator, bool enable)
{
    if (trigger_map.find(id) != trigger_map.end()) {
        logger.printfln("Trigger %u is already registered", static_cast<uint>(id));
        return;
    }

    trigger_prototypes.push_back({id, cfg});
    trigger_map[id] = TriggerValue{std::forward<ValidatorCb>(validator), enable};
}

void Automation::set_enabled(AutomationActionID id, bool enable)
{
    if (action_map.find(id) == action_map.end()) {
        logger.printfln("Action %u is not registered", static_cast<uint>(id));
        return;
    }

    auto enabled_actions = state.get("enabled_actions");

    for (size_t i = 0; i < enabled_actions->count(); ++i) {
        if (enabled_actions->get(i)->asEnum<AutomationActionID>() == id) {
            if (!enable) {
                enabled_actions->remove(i);
            }

            return;
        }
    }

    if (enable) {
        enabled_actions->add()->updateEnum(id);
    }
}

void Automation::set_enabled(AutomationTriggerID id, bool enable)
{
    if (trigger_map.find(id) == trigger_map.end()) {
        logger.printfln("Trigger %u is not registered", static_cast<uint>(id));
        return;
    }

    auto enabled_triggers = state.get("enabled_triggers");

    for (size_t i = 0; i < enabled_triggers->count(); ++i) {
        if (enabled_triggers->get(i)->asEnum<AutomationTriggerID>() == id) {
            if (!enable) {
                enabled_triggers->remove(i);
            }

            return;
        }
    }

    if (enable) {
        enabled_triggers->add()->updateEnum(id);
    }
}

bool Automation::trigger(AutomationTriggerID number, void *data, IAutomationBackend *backend)
{
    if (config_in_use.is_null()) {
        logger.printfln("Received trigger ID %u before loading config. Event lost.", static_cast<uint32_t>(number));
        return false;
    }
    bool triggered = false;
    int current_rule = 1;
    for (Config &conf : config_in_use.get("tasks")) {
        Config *trigger = static_cast<Config *>(conf.get("trigger"));
        if (trigger->getTag<AutomationTriggerID>() == number && backend->has_triggered(trigger, data)) {
            triggered = true;
            logger.printfln("Running rule #%d", current_rule);
            const Config *action = static_cast<const Config *>(conf.get("action"));
            AutomationActionID action_ident = action->getTag<AutomationActionID>();
            if (action_ident != AutomationActionID::None && action_map.find(action_ident) != action_map.end()) {
                action_map[action_ident].callback(static_cast<const Config *>(action->get()));
            } else {
                logger.printfln("There is no action with ID %u!", (uint8_t)action_ident);
            }
        }
        current_rule++;
    }
    return triggered;
}

bool Automation::has_task_with_trigger(AutomationTriggerID number)
{
    for (const Config &conf : config_in_use.get("tasks")) {
        if (conf.get("trigger")->getTag<AutomationTriggerID>() == number) {
            return true;
        }
    }
    return false;
}

Automation::ConfigVec Automation::get_configured_triggers(AutomationTriggerID number)
{
    ConfigVec vec;
    Config *tasks = static_cast<Config *>(config_in_use.get("tasks"));
    size_t task_count = tasks->count();
    for (size_t idx = 0; idx < task_count; idx++) {
        auto trigger = tasks->get(idx)->get("trigger");
        if (trigger->getTag<AutomationTriggerID>() == number) {
            vec.push_back({idx, static_cast<Config *>(trigger->get())});
        }
    }
    return vec;
}

static bool is_last_day(struct tm time)
{
    const int mon = time.tm_mon;
    time_t next_day = mktime(&time) + 86400;
    time = *localtime(&next_day);
    return time.tm_mon != mon;
}

bool Automation::has_triggered(const Config *conf, void *data)
{
    const Config *cfg = static_cast<const Config *>(conf->get());
    tm *time_struct = (tm *)data;
    bool triggered = false;

    int32_t wday = cfg->get("wday")->asInt();
    if (wday == -1) {
        int32_t mday = cfg->get("mday")->asInt();
        triggered |= mday == time_struct->tm_mday || mday == -1 || mday == 0;
        triggered |= mday == 32 && is_last_day(*time_struct);
    } else if (wday > 7) {
        triggered |= wday == 8 && time_struct->tm_wday > 0 && time_struct->tm_wday < 6;
        triggered |= wday == 9 && (time_struct->tm_wday == 0 || time_struct->tm_wday >= 6);
    } else {
        triggered |= (wday % 7) == time_struct->tm_wday;
    }

    int32_t hour = cfg->get("hour")->asInt();
    triggered = (hour == time_struct->tm_hour || hour == -1) && triggered;
    int32_t minute = cfg->get("minute")->asInt();
    triggered = (minute == time_struct->tm_min || minute == -1) && triggered;

    switch (conf->getTag<AutomationTriggerID>()) {
        case AutomationTriggerID::Cron:
            if (triggered) {
                return true;
            }
            break;

        default:
            return false;
    }
    return false;
}
