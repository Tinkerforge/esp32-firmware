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

#include <map>
#include <vector>

#include "module.h"
#include "config.h"
#include "automation_trigger_id.enum.h"
#include "automation_action_id.enum.h"
#include "automation_backend.h"

class Automation : public IModule, public IAutomationBackend
{
public:
    typedef std::function<void(const Config *)>    ActionCb;
    typedef std::function<String(const Config *)> ValidatorCb;

    struct ActionValue {
        ActionCb callback;
        ValidatorCb validator;
        bool enable;
    };

    struct TriggerValue {
        ValidatorCb validator;
        bool enable;
    };

    typedef std::map<AutomationActionID, ActionValue>   ActionMap;
    typedef std::map<AutomationTriggerID, TriggerValue> TriggerMap;
    typedef std::vector<std::pair<size_t, Config *>>    ConfigVec;

    Automation();

    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void register_action(AutomationActionID id, Config cfg, ActionCb &&callback, ValidatorCb &&validator = nullptr, bool enable = true);
    void register_trigger(AutomationTriggerID id, Config cfg, ValidatorCb &&validator = nullptr, bool enable = true);

    void set_enabled(AutomationActionID id, bool enable);
    void set_enabled(AutomationTriggerID id, bool enable);

    bool trigger(AutomationTriggerID number, void *data, IAutomationBackend *backend);
    bool has_task_with_trigger(AutomationTriggerID number);
    bool has_triggered(const Config *conf, void *data) override;
    ConfigVec get_configured_triggers(AutomationTriggerID number);

private:
    ConfigRoot config;
    ConfigRoot config_in_use;
    ConfigRoot state;

    ActionMap   action_map;
    TriggerMap  trigger_map;
    std::vector<ConfUnionPrototype<AutomationTriggerID>>    trigger_prototypes;
    std::vector<ConfUnionPrototype<AutomationActionID>>     action_prototypes;
};
