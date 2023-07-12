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
    config = Config::Object({
        {"enabled", Config::Bool(false)}
    });

    timed_config = Config::Array(
        {},
        new Config{Config::Object({
            {"h", Config::Uint(0, 0, 24)},
            {"m", Config::Uint(0, 0, 60)},
            {"s", Config::Uint(0, 0, 60)}
        })}, 0, 20, Config::type_id<Config::ConfObject>());
}

void Cron::setup() {
    api.restorePersistentConfig("cron/config", &config);
    api.restorePersistentConfig("cron/timed_config", &timed_config);

    config_in_use = config;
    timed_config_in_use = timed_config;
}

void Cron::register_urls() {
    api.addPersistentConfig("cron/config", &config, {}, 1000);
    api.addPersistentConfig("cron/timed_config", &timed_config, {}, 1000);
}