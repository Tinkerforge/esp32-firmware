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

#include <string.h>

#include "mqtt_client.h"

#include "api.h"
#include "modules.h"
#include "mqtt_meter.h"

void MqttMeter::pre_setup()
{
    config = ConfigRoot(Config::Object({
        {"enable", Config::Bool(false)},
        {"topic", Config::Str("warp2/source/meter/values", 1, 128)}
    }),  [](Config &cfg) -> String {
        const String &global_topic_prefix = mqtt.mqtt_config.get("global_topic_prefix")->asString();
        const String &meter_topic = cfg.get("topic")->asString();

        if (meter_topic.startsWith(global_topic_prefix))
            return "Cannot listen to itself: Meter topic cannot start with the topic prefix.";

        return "";
    });
}

void MqttMeter::setup()
{
    api.restorePersistentConfig("mqtt/meter_config", &config);
    enabled = config.get("enable")->asBool();
    source_meter_topic = config.get("topic")->asString();
    initialized = true;
}

void MqttMeter::register_urls()
{
    api.addPersistentConfig("mqtt/meter_config", &config, {}, 1000);
}

void MqttMeter::loop()
{
}

void MqttMeter::onMqttConnect()
{
    if (!enabled)
        return;

    esp_mqtt_client_subscribe(mqtt.client, source_meter_topic.c_str(), 0);
}

bool MqttMeter::onMqttMessage(char *topic, size_t topic_len, char *data, size_t data_len, bool retain)
{
    if (!enabled)
        return false;

    if (source_meter_topic.length() != topic_len || memcmp(source_meter_topic.c_str(), topic, topic_len) != 0) {
        // Not our topic
        return false;
    }

    if (retain) {
        logger.printfln("mqtt_meter: Ignoring retained message.");
        // Even if we ignore the data, return true because we consumed the message.
        return true;
    }

    StaticJsonDocument<64> doc;
    DeserializationError error = deserializeJson(doc, data, data_len);
    if (error) {
        logger.printfln("mqtt_meter: Failed to deserialize MQTT payload: %s", error.c_str());
        return true;
    }

    uint32_t meter_type = meter.state.get("type")->asUint();
    if (meter_type != METER_TYPE_MQTT) {
        if (meter_type != METER_TYPE_NONE) {
            logger.printfln("mqtt_meter: Detected presence of a conflicting meter, type %u. The MQTT meter should be disabled.", meter_type);
            return true;
        }
        meter.updateMeterState(2, METER_TYPE_MQTT);
    }

    float power      = doc["power"     ].as<float>();
    float energy_rel = doc["energy_rel"].as<float>();
    float energy_abs = doc["energy_abs"].as<float>();
    meter.updateMeterValues(power, energy_rel, energy_abs);

    return true;
}
