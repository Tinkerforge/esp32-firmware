/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "automation.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "options.h"
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
            logger.printfln_continue("%s", cfg->get("message")->asString().c_str());
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

    state_actions_prototype  = Config::Enum(AutomationActionID::None);
    state_triggers_prototype = Config::Enum(AutomationTriggerID::None);

    state = Config::Object({
        {"registered_triggers", Config::Array({}, &state_triggers_prototype, 0, AUTOMATION_TRIGGER_ID_COUNT, Config::type_id<Config::ConfUint>())},
        {"registered_actions",  Config::Array({}, &state_actions_prototype,  0, AUTOMATION_ACTION_ID_COUNT,  Config::type_id<Config::ConfUint>())},
        {"enabled_triggers",    Config::Array({}, &state_triggers_prototype, 0, AUTOMATION_TRIGGER_ID_COUNT, Config::type_id<Config::ConfUint>())},
        {"enabled_actions",     Config::Array({}, &state_actions_prototype,  0, AUTOMATION_ACTION_ID_COUNT,  Config::type_id<Config::ConfUint>())},
        {"last_run",            Config::Array({}, Config::get_prototype_timestamp_0(), 0, OPTIONS_AUTOMATION_MAX_RULES(), Config::type_id<Config::ConfInt52>())},
    });
}

void Automation::setup()
{
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

    // Create Config::Objects here and not during pre-setup so that other module can registertriggers and actions
    // but the automation config can still be loaded at the beginning of the setup stage.

    config_tasks_prototype = Config::Object({
        {"trigger", Config::Union<AutomationTriggerID>(
            *Config::Null(),
            AutomationTriggerID::None,
            trigger_prototypes.data(),
            trigger_prototypes.size()
        )},
        {"action", Config::Union<AutomationActionID>(
            *Config::Null(),
            AutomationActionID::None,
            action_prototypes.data(),
            action_prototypes.size()
        )},
        {"delay", Config::Uint(0, 0, 24*60*60)}
    });

    config = ConfigRoot{Config::Object({
            {"tasks", Config::Array(
                {},
                &config_tasks_prototype,
                0, OPTIONS_AUTOMATION_MAX_RULES(), Config::type_id<Config::ConfObject>())
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

            // Only apply the config live if the set of reboot-requiring trigger
            // types hasn't changed since boot. If the same types are present, boot-time
            // initialization already happened and live-apply is safe.
            auto new_types = get_reboot_types(cfg);
            if (new_types.triggers == this->boot_reboot_types.triggers) {
                task_scheduler.scheduleOnce([this]() {
                    this->apply_config();
                });
            }

            return "";
        }
    };

    api.restorePersistentConfig("automation/config", &config);
    boot_reboot_types = get_reboot_types(config);
    config_in_use = config;

    const size_t task_count = config.get("tasks")->count();

    this->state.get("last_run")->setCount(task_count);

    last_run = heap_alloc_array<micros_t>(task_count);

    handle_cron_task();

    initialized = true;
}

void Automation::register_urls()
{
    api.addPersistentConfig("automation/config", &config);
    api.addState("automation/state", &state);
}

void Automation::handle_cron_task()
{
    bool need_cron = has_task_with_trigger(AutomationTriggerID::Cron);

    if (need_cron && cron_task_id == 0) {
        cron_task_id = task_scheduler.scheduleWithFixedDelay([this]() {
            static int last_min = 0;
            static bool was_synced = false;
            timeval tv;
            bool is_synced = rtc.clock_synced(&tv);
            tm time_struct;

            localtime_r(&tv.tv_sec, &time_struct);

            if (was_synced && time_struct.tm_min != last_min) {
                trigger(AutomationTriggerID::Cron, &time_struct, this);
            }

            last_min = time_struct.tm_min;
            was_synced = is_synced;
        }, 1_s);
    } else if (!need_cron && cron_task_id != 0) {
        task_scheduler.cancel(cron_task_id);
        cron_task_id = 0;
    }
}

void Automation::apply_config()
{
    // Cancel any pending delayed actions before replacing config_in_use.
    // Their lambdas capture raw Config pointers into the old config tree,
    // which would become dangling after the assignment below.
    for (uint64_t task_id : pending_delayed_tasks) {
        task_scheduler.cancel(task_id);
    }
    pending_delayed_tasks.clear();

    config_in_use = config;

    const size_t task_count = config_in_use.get("tasks")->count();

    // Reallocate last_run array for the new task count.
    // We zero all timestamps, which means tasks may re-fire once.
    last_run = heap_alloc_array<micros_t>(task_count);

    this->state.get("last_run")->setCount(task_count);
    for (size_t i = 0; i < task_count; ++i) {
        static_cast<Config *>(state.get("last_run")->get(i))->updateUptime(micros_t{0});
    }

    handle_cron_task();

    for (const auto &callback : on_config_applied_callbacks) {
        callback();
    }
}

static bool trigger_needs_reboot(AutomationTriggerID id)
{
    switch (id) {
        case AutomationTriggerID::MQTT:
            return true;
        default:
            return false;
    }
}


Automation::RebootTypeSet Automation::get_reboot_types(const Config &cfg)
{
    RebootTypeSet set = {0};
    for (const Config &task : cfg.get("tasks")) {
        const Config *trigger = static_cast<const Config *>(task.get("trigger"));
        AutomationTriggerID tid = trigger->getTag<AutomationTriggerID>();
        if (trigger_needs_reboot(tid)) {
            set.triggers |= 1u << static_cast<uint8_t>(tid);
        }
    }
    return set;
}

void Automation::register_action(AutomationActionID id, const Config &cfg, ActionCb &&callback, ValidatorCb &&validator, bool enable)
{
    if (boot_stage > BootStage::PRE_SETUP) {
#ifdef DEBUG_FS_ENABLE
        esp_system_abort("Registering actions is only allowed during the pre-setup stage.");
#else
        logger.printfln("Registering actions is only allowed during the pre-setup stage. Ignoring action ID %lu.", static_cast<uint32_t>(id));
        return;
#endif
    }

    if (action_map.find(id) != action_map.end()) {
        logger.printfln("Action %u is already registered", static_cast<uint>(id));
        return;
    }

    action_prototypes.push_back({id, cfg});
    action_map[id] = ActionValue{std::move(callback), std::move(validator), enable};
}

void Automation::register_trigger(AutomationTriggerID id, const Config &cfg, ValidatorCb &&validator, bool enable)
{
    if (boot_stage > BootStage::PRE_SETUP) {
#ifdef DEBUG_FS_ENABLE
        esp_system_abort("Registering triggers is only allowed during the pre-setup stage.");
#else
        logger.printfln("Registering triggers is only allowed during the pre-setup stage. Ignoring trigger ID %lu.", static_cast<uint32_t>(id));
        return;
#endif
    }

    if (trigger_map.find(id) != trigger_map.end()) {
        logger.printfln("Trigger %u is already registered", static_cast<uint>(id));
        return;
    }

    trigger_prototypes.push_back({id, cfg});
    trigger_map[id] = TriggerValue{std::move(validator), enable};
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
        logger.printfln("Received trigger '%s' (%u) before loading config. Event lost.", get_automation_trigger_id_name(number), static_cast<unsigned>(number));
        return false;
    }
    bool triggered = false;
    int current_rule = 1;
    for (size_t i = 0; i < config_in_use.get("tasks")->count(); ++i) {
        micros_t *last_run_timestamp = &last_run[i];

        // If last_run is in the future, this rule's trigger has fired
        // but the configured delay is not elapsed yet.
        // Don't allow executing the rule again until the action is done
        if (!deadline_elapsed(*last_run_timestamp))
            continue;

        const Config *conf = static_cast<const Config *>(config_in_use.get("tasks")->get(i));
        const Config *trigger = static_cast<const Config *>(conf->get("trigger"));

        if (trigger->getTag<AutomationTriggerID>() == number && backend->has_triggered(trigger, data)) {
            auto delay = seconds_t{conf->get("delay")->asUint()};
            *last_run_timestamp = now_us() + delay;
            Config *last_run_cfg = static_cast<Config *>(state.get("last_run")->get(i));
            last_run_cfg->updateUptime(*last_run_timestamp);

            triggered = true;
            const Config *action = static_cast<const Config *>(conf->get("action"));
            AutomationActionID action_ident = action->getTag<AutomationActionID>();

            if (action_ident != AutomationActionID::None && action_map.find(action_ident) == action_map.end()) {
                logger.printfln("There is no action with ID %u!", static_cast<uint8_t>(action_ident));
                continue;
            }

            const ActionCb &cb = action_map[action_ident].callback;
            if (delay > 0_s) {
                logger.printfln("Running rule #%d in %lu seconds", current_rule, delay.as<uint32_t>());
                uint64_t task_id = task_scheduler.scheduleOnce([this, cb, action](){
                    cb(static_cast<const Config *>(action->get()));

                    // Remove this task from the pending list now that it is executed
                    uint64_t current_id = task_scheduler.currentTaskId();
                    for (size_t j = 0; j < pending_delayed_tasks.size(); ++j) {
                        if (pending_delayed_tasks[j] == current_id) {
                            pending_delayed_tasks[j] = pending_delayed_tasks.back();
                            pending_delayed_tasks.pop_back();
                            break;
                        }
                    }
                }, delay);
                pending_delayed_tasks.push_back(task_id);
            } else {
                logger.printfln("Running rule #%d", current_rule);
                cb(static_cast<const Config *>(action->get()));
            }
        }
        current_rule++;
    }
    return triggered;
}

bool Automation::has_task_with_trigger(AutomationTriggerID number)
{
    if (config_in_use.is_null()) {
#ifdef DEBUG_FS_ENABLE
        esp_system_abort("has_task_with_trigger failed because config is not loaded yet.");
#else
        logger.printfln("has_task_with_trigger failed because config is not loaded yet.");
        return false;
#endif
    }

    for (const Config &conf : config_in_use.get("tasks")) {
        if (conf.get("trigger")->getTag<AutomationTriggerID>() == number) {
            return true;
        }
    }
    return false;
}

bool Automation::has_task_with_action(AutomationActionID number)
{
    if (config_in_use.is_null()) {
#ifdef DEBUG_FS_ENABLE
        esp_system_abort("has_task_with_action failed because config is not loaded yet.");
#else
        logger.printfln("has_task_with_action failed because config is not loaded yet.");
        return false;
#endif
    }

    for (const Config &conf : config_in_use.get("tasks")) {
        if (conf.get("action")->getTag<AutomationActionID>() == number) {
            return true;
        }
    }
    return false;
}

void Automation::register_on_config_applied(std::function<void()> &&callback)
{
    on_config_applied_callbacks.push_back(std::move(callback));
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
    const time_t next_day = mktime(&time) + 24 * 60 * 60;
    tm tm_next_day;
    localtime_r(&next_day, &tm_next_day);
    return time.tm_mon != tm_next_day.tm_mon;
}

bool Automation::has_triggered(const Config *conf, void *data)
{
    const Config *cfg = static_cast<const Config *>(conf->get());
    tm *time_struct = static_cast<tm *>(data);
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
