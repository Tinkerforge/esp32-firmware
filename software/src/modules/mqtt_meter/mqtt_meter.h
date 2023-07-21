/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include <stdint.h>
#include "ArduinoJson.h"

#include "config.h"
#include "module.h"

class MqttMeter final : public IModule
{
public:
    MqttMeter(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    ConfigRoot config;

private:
    void onMessage(const char *topic, size_t topic_len, char *data, size_t data_len, void (MqttMeter::*message_handler)(const JsonDocument &doc));

    void handle_mqtt_values(const JsonDocument &doc);
    void handle_mqtt_all_values(const JsonDocument &doc);

    bool enabled = false;
    uint8_t mqtt_meter_type = 0;
    String source_meter_values_topic;
    String source_meter_all_values_topic;
};
