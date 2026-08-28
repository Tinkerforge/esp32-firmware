/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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
#include "generated/mqtt_discovery_topics.h"
#include "cool_string.h"
#include "generated/mqtt_auto_discovery_mode.enum.h"

class MqttAutoDiscovery final : public IModule
{
public:
    MqttAutoDiscovery(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

    ConfigRoot config;
    void reschedule_announce_next_topic();

private:
    MqttAutoDiscoveryMode mode;
    String prefix;

    std::unique_ptr<uint8_t[]> mqtt_discovery_topic_lengths = nullptr;

    uint64_t task_id = 0;
    uint32_t next_topic = 0;
    void announce_next_topic();

    void prepare_topic_lengths();
    void check_discovery_topic(const char *topic, size_t topic_len, size_t data_len);
    size_t get_discovery_topic(size_t topic_idx, char *buf, size_t buf_len);
};
