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
                    {"h", Config::Uint(0, 0, 2)},
                    {"m", Config::Uint(0, 1, 60)}
                })},
                {"action", Config::Str("", 0, 128)}
            })
        }, 0, 20, Config::type_id<Config::ConfObject>()
    );

    enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
}

void Cron::setup() {
    api.restorePersistentConfig("cron/config", &config);
    api.restorePersistentConfig("cron/timed_config", &enabled);

    config_in_use = config;
    enabled_in_use = enabled;
}

void Cron::register_urls() {
    api.addPersistentConfig("cron/config", &config, {}, 1000);
    api.addPersistentConfig("cron/timed_config", &enabled, {}, 1000);
}

void Cron::register_action(String &name, ActionCb &action) {
    action_map[name] = action;
}

void Cron::register_trigger(uint32_t number) {

}

void Cron::trigger_action(ICronModule *module, uint32_t number) {
    for (auto it = config.begin(); it != config.end(); it++) {
        if ((*it).get("trigger")->get("number")->asUint() == number && module->action_triggered((Config*)(*it).get("trigger")))
            action_map[(*it).get("action")->asString()]();
    }
}