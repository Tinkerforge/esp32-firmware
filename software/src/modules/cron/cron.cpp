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
        }, 0, 20, Config::type_id<Config::ConfObject>());

    config = Config::Object({
        {"tasks", config}
    });


    enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
}

void Cron::setup() {
    api.restorePersistentConfig("cron/config", &config);
    api.restorePersistentConfig("cron/enabled", &enabled);

    config_in_use = config;
    enabled_in_use = enabled;

    initialized = true;
}

void Cron::register_urls() {
    api.addPersistentConfig("cron/config", &config, {}, 1000);
    api.addPersistentConfig("cron/timed_config", &enabled, {}, 1000);
}

void Cron::register_action(CronActionID id, Config cfg, ActionCb callback) {
    action_vec.push_back({id, cfg});
    action_map[id] = callback;
}

void Cron::register_trigger(CronTriggerID id, Config cfg) {
    trigger_vec.push_back({id, cfg});
}

bool Cron::trigger_action(CronTriggerID number, void *data, std::function<bool(Config *, void *)> cb) {
    bool triggered = false;
    for (auto &conf: config.get("tasks")) {
        if (conf.get("trigger")->getTag<CronTriggerID>() == number && cb((Config *)conf.get("trigger"), data)) {
            triggered = true;
            auto action_ident = conf.get("action")->getTag<CronActionID>();
            if (action_map.find(action_ident) != action_map.end())
                action_map[action_ident]((Config *)conf.get("action")->get());
            else
                logger.printfln("There is no action with ident-nr %u!", (uint8_t)action_ident);
        }
    }
    return triggered;
}

bool Cron::is_trigger_active(CronTriggerID number) {
    for (auto &conf: config.get("tasks")) {
        if (conf.get("trigger")->getTag<CronTriggerID>() == number) {
            return true;
        }
    }
    return false;
}

ConfigVec Cron::get_configured_triggers(CronTriggerID number) {
    ConfigVec vec;
    for (size_t idx = 0; idx < config.get("tasks")->count(); idx++) {
        auto trigger = config.get("tasks")->get(idx)->get("trigger");
        if (trigger->getTag<CronTriggerID>() == number) {
            vec.push_back({idx, (Config *)trigger->get()});
        }
    }
    return vec;
}
