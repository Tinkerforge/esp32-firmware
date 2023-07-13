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

#pragma once

#include "module.h"
#include "config.h"
#include <map>
#include <vector>

class ICronModule {
public:
    virtual bool action_triggered(Config *config) = 0;
};


typedef std::function<void()>       ActionCb;
typedef std::map<uint32_t, ActionCb>  ActionMap;
typedef std::vector<String>         EventVec;

class Cron : public IModule {
    ConfigRoot config;
    ConfigRoot config_in_use;
    ConfigRoot enabled;
    ConfigRoot enabled_in_use;

    ActionMap action_map;

public:
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void register_action(uint32_t ident, ActionCb callback);
    void register_trigger(uint32_t number);

    void trigger_action(ICronModule *module, uint32_t number);
};
