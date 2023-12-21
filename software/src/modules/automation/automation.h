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
#include "automation_defs.h"

typedef std::function<void(const Config *)>                             ActionCb;
typedef std::function<String (const Config *)>                          ValidatorCb;
typedef std::map<AutomationActionID, std::pair<ActionCb, ValidatorCb>>        ActionMap;
typedef std::map<AutomationTriggerID, ValidatorCb>                            TriggerMap;
typedef std::vector<std::pair<size_t, Config *>>                        ConfigVec;

class Automation : public IModule {
    ConfigRoot config;
    ConfigRoot config_in_use;
    ConfigRoot enabled;
    ConfigRoot enabled_in_use;

    ActionMap   action_map;
    TriggerMap  trigger_map;
    std::vector<ConfUnionPrototype<AutomationTriggerID>>    trigger_vec;
    std::vector<ConfUnionPrototype<AutomationActionID>>     action_vec;

public:
    Automation();

    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void register_action(AutomationActionID id, Config cfg, ActionCb callback, ValidatorCb validator = nullptr);
    void register_trigger(AutomationTriggerID id, Config cfg, ValidatorCb validator = nullptr);

    bool trigger_action(AutomationTriggerID number, void *data, std::function<bool(Config *, void *)> cb);
    bool is_trigger_active(AutomationTriggerID number);

    bool action_triggered(Config *conf, void *data);

    ConfigVec get_configured_triggers(AutomationTriggerID number);
};
