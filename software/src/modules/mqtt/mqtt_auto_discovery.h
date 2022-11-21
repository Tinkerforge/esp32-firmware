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

#include "config.h"
#include "mqtt.h"

class MqttAutoDiscovery
{
public:
    MqttAutoDiscovery(){}
    void prepare_topics(const ConfigRoot &mqtt_config_in_use);
    void subscribe_to_own();
    void check_discovery_topic(const char *topic, size_t topic_len, size_t data_len);
    void start_announcing();

private:
    void announce_next_topic(uint32_t next_topic);
};

extern MqttAutoDiscovery mqtt_auto_discovery;
