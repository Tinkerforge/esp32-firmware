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

void Cron::pre_setup() {
    config = Config::Array(
        {},
        new Config{
            Config::Object({
                {"trigger", Config::Object({
                    {"number", Config::Uint(0)},
                    {"mday", Config::Int(-1, -1, 30)},
                    {"wday", Config::Int(-1, -1, 6)},
                    {"hour", Config::Int(-1, -1, 23)},
                    {"minute", Config::Int(-1, -1, 59)}
                })},
                {"action", Config::Uint(0)}
            })
        }, 0, 20, Config::type_id<Config::ConfObject>()
    );

    Config trigger_prototype = Config::Union(
                    *Config::Null(),
                    0,
                    new Config*[2] {
                        Config::Null(),
                        new Config(Config::Object({
                            {"ident-no", Config::Uint(0)},
                            {"mday", Config::Int(-1, -1, 30)},
                            {"wday", Config::Int(-1, -1, 6)},
                            {"hour", Config::Int(-1, -1, 23)},
                            {"minute", Config::Int(-1, -1, 59)}
                        }))
                    },
                    2);

    Config action_prototype = Config::Union(
                    *Config::Null(),
                    0,
                    new Config*[2] {
                        Config::Null(),
                        new Config(Config::Object({
                            {"message", Config::Str("", 0, 32)}
                        }))
                    },
                    2);

    config = Config::Array(
        {},
        new Config{
            Config::Object({
                {"trigger", trigger_prototype},
                {"action", action_prototype}
            })
        }, 0, 20, Config::type_id<Config::ConfObject>());

    enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
}

void Cron::setup() {
    api.restorePersistentConfig("cron/config", &config);
    api.restorePersistentConfig("cron/timed_config", &enabled);

    config_in_use = config;
    enabled_in_use = enabled;

    register_action(0, [this]() {
        logger.printfln("Action 0 triggered");
    });

    initialized = true;
}

void Cron::register_urls() {
    api.addPersistentConfig("cron/config", &config, {}, 1000);
    api.addPersistentConfig("cron/timed_config", &enabled, {}, 1000);
}

void Cron::register_action(uint32_t ident, ActionCb action) {
    action_map[ident] = action;
}

void Cron::register_trigger(uint32_t number) {

}

void Cron::trigger_action(ICronModule *module, uint32_t number) {
    for (auto it = config.begin(); it != config.end(); it++) {
        if ((*it).get("trigger")->get() == number && module->action_triggered((Config*)(*it).get("trigger"))) {
            uint32_t action_ident = (*it).get("action")->asUint();
            if (action_map.find(action_ident) != action_map.end())
                action_map[action_ident]();
            else
                logger.printfln("There is no action with ident-nr %u!", action_ident);
        }
    }
}