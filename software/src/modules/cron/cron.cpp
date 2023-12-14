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

#include "cron.h"
#include "api.h"
#include "task_scheduler.h"

Cron::Cron() {
    trigger_vec.push_back({CronTriggerID::None, *Config::Null()});
    action_vec.push_back({CronActionID::None, *Config::Null()});
}

void Cron::pre_setup() {
    register_action(
        CronActionID::Print,
        Config::Object({
            {"message", Config::Str("", 0, 64)}
        }),
        [this](const Config *cfg) {
            logger.printfln("Got message: %s", cfg->get("message")->asString().c_str());
        }
    );
    register_trigger(
        CronTriggerID::Cron,
        Config::Object({
            {"mday", Config::Int(-1, -1, 32)},
            {"wday", Config::Int(-1, -1, 9)},
            {"hour", Config::Int(-1, -1, 23)},
            {"minute", Config::Int(-1, -1, 59)}
        })
    );
    Config trigger_prototype = Config::Union<CronTriggerID>(
                    *Config::Null(),
                    CronTriggerID::None,
                    trigger_vec.data(),
                    trigger_vec.size());

    Config action_prototype = Config::Union<CronActionID>(
                    *Config::Null(),
                    CronActionID::None,
                    action_vec.data(),
                    action_vec.size());

    config = Config::Array(
        {},
        new Config{
            Config::Object({
                {"trigger", trigger_prototype},
                {"action", action_prototype}
            })
        }, 0, 10, Config::type_id<Config::ConfObject>());

    config = ConfigRoot(Config::Object({
        {"tasks", config}
    }),
    [this](Config &cfg, ConfigSource source) -> String {
        for (auto &task : cfg.get("tasks")) {
            CronActionID action_id = task.get("action")->getTag<CronActionID>();
            if (action_id == CronActionID::None) {
                return "ActionID must not be 0!";
            }

            auto &action_validator = this->action_map[action_id].second;
            if (action_validator) {
                auto ret = action_validator((Config *)task.get("action")->get());
                if (ret != "") {
                    return ret;
                }
            }

            CronTriggerID trigger_id = task.get("trigger")->getTag<CronTriggerID>();
            if (trigger_id == CronTriggerID::None) {
                return "TriggerID must not be 0!";
            }

            auto &trigger_validator = this->trigger_map[trigger_id];
            if (trigger_validator) {
                auto ret = trigger_validator((Config *)task.get("trigger")->get());
                if (ret != "") {
                    return ret;
                }
            }
        }

        return "";
    });
    config_in_use = config;

    enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
}

void Cron::setup() {
    api.restorePersistentConfig("cron/config", &config);
    api.restorePersistentConfig("cron/enabled", &enabled);

    config_in_use = config;
    enabled_in_use = enabled;

    if (is_trigger_active(CronTriggerID::Cron)) {
        task_scheduler.scheduleWithFixedDelay([this]() {
            static int last_min = 0;
            static bool was_synced = false;
            auto func = [this](Config *cfg, void *data) -> bool {
                return action_triggered(cfg, data);
            };
            timeval tv;
            bool is_synced = clock_synced(&tv);

            tm time_struct;
            localtime_r(&tv.tv_sec, &time_struct);
            if (was_synced && time_struct.tm_min != last_min) {
                trigger_action(CronTriggerID::Cron, &time_struct, func);
            }

            last_min = time_struct.tm_min;
            was_synced = is_synced;
        }, 0, 1000);
    }


    initialized = true;
}

void Cron::register_urls() {
    api.addPersistentConfig("cron/config", &config, {}, 1000);
    api.addPersistentConfig("cron/timed_config", &enabled, {}, 1000);
}

void Cron::register_action(CronActionID id, Config cfg, ActionCb callback, ValidatorCb validator) {
    action_vec.push_back({id, cfg});
    action_map[id] = std::pair<ActionCb, ValidatorCb>(callback, validator);
}

void Cron::register_trigger(CronTriggerID id, Config cfg, ValidatorCb validator) {
    trigger_vec.push_back({id, cfg});
    trigger_map[id] = validator;
}

bool Cron::trigger_action(CronTriggerID number, void *data, std::function<bool(Config *, void *)> cb) {
    bool triggered = false;
    for (auto &conf: config_in_use.get("tasks")) {
        if (conf.get("trigger")->getTag<CronTriggerID>() == number && cb((Config *)conf.get("trigger"), data)) {
            triggered = true;
            auto action_ident = conf.get("action")->getTag<CronActionID>();
            if (action_map.find(action_ident) != action_map.end() && action_ident != CronActionID::None)
                action_map[action_ident].first((Config *)conf.get("action")->get());
            else
                logger.printfln("There is no action with ident-nr %u!", (uint8_t)action_ident);
        }
    }
    return triggered;
}

bool Cron::is_trigger_active(CronTriggerID number) {
    for (auto &conf: config_in_use.get("tasks")) {
        if (conf.get("trigger")->getTag<CronTriggerID>() == number) {
            return true;
        }
    }
    return false;
}

ConfigVec Cron::get_configured_triggers(CronTriggerID number) {
    ConfigVec vec;
    for (size_t idx = 0; idx < config_in_use.get("tasks")->count(); idx++) {
        auto trigger = config.get("tasks")->get(idx)->get("trigger");
        if (trigger->getTag<CronTriggerID>() == number) {
            vec.push_back({idx, (Config *)trigger->get()});
        }
    }
    return vec;
}
static bool is_last_day (struct tm time) {
    const int mon = time.tm_mon;
    time_t next_day = mktime(&time) + 86400;
    time = *localtime(&next_day);
    return time.tm_mon != mon;
}

bool Cron::action_triggered(Config *conf, void *data) {
    Config *cfg = (Config*)conf->get();
    tm *time_struct = (tm *)data;
    bool triggered = false;

    if (cfg->get("wday")->asInt() == -1) {
        triggered |= cfg->get("mday")->asInt() == time_struct->tm_mday || cfg->get("mday")->asInt() == -1 || cfg->get("mday")->asInt() == 0;
        triggered |= cfg->get("mday")->asInt() == 32 && is_last_day(*time_struct);
    } else if (cfg->get("wday")->asInt() > 7) {
        triggered |= cfg->get("wday")->asInt() == 8 && time_struct->tm_wday > 0 && time_struct->tm_wday < 6;
        triggered |= cfg->get("wday")->asInt() == 9 && (time_struct->tm_wday == 0 || time_struct->tm_wday >= 6);
    } else {
        triggered |= (cfg->get("wday")->asInt() % 7) == time_struct->tm_wday || cfg->get("wday")->asInt() == -1;
    }
    triggered = (cfg->get("hour")->asInt() == time_struct->tm_hour || cfg->get("hour")->asInt() == -1) && triggered;
    triggered = (cfg->get("minute")->asInt() == time_struct->tm_min || cfg->get("minute")->asInt() == -1) && triggered;

    switch (conf->getTag<CronTriggerID>()) {
        case CronTriggerID::Cron:
            if (triggered) {
                return true;
            }
            break;

        default:
            return false;
    }
    return false;
}
