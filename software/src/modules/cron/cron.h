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

#define CRON_TRIGGER_CRON 1
#define CRON_TRIGGER_IEC_CHANGE 2
#define CRON_TRIGGER_MQTT 3
#define CRON_TRIGGER_EVSE_BUTTON 4
#define CRON_TRIGGER_NFC 5
#define CRON_TRIGGER_CHARGE_LIMITS 6
#define CRON_TRIGGER_EVSE_SHUTDOWN_INPUT 7
#define CRON_TRIGGER_EVSE_GP_INPUT 8

#define CRON_ACTION_PRINT 1
#define CRON_ACTION_MQTT 2
#define CRON_ACTION_SET_CURRENT 3
#define CRON_ACTION_LED 4
#define CRON_ACTION_METER_RESET 5
#define CRON_ACTION_SET_MANAGER_CURRENT 6
#define CRON_ACTION_NFC_INJECT_TAG 7
#define CRON_ACTION_CHARGE_LIMITS 8
#define CRON_ACTION_EVSE_GP_OUTPUT 9

typedef std::function<void(const Config *)>               ActionCb;
typedef std::map<uint32_t, ActionCb>                ActionMap;
typedef std::vector<std::pair<size_t, Config *>>                       ConfigVec;

class Cron : public IModule {
    ConfigRoot config;
    ConfigRoot config_in_use;
    ConfigRoot enabled;
    ConfigRoot enabled_in_use;

    ActionMap   action_map;
    std::vector<ConfUnionPrototype>    trigger_vec;
    std::vector<ConfUnionPrototype>    action_vec;

public:
    Cron();

    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void register_action(const ConfUnionPrototype &proto, ActionCb callback);
    void register_trigger(const ConfUnionPrototype &proto);

    bool trigger_action(uint8_t number, void *data, bool (*cb)(Config *, void *));
    bool is_trigger_active(uint8_t number);

    ConfigVec get_configured_triggers(uint8_t number);
};
