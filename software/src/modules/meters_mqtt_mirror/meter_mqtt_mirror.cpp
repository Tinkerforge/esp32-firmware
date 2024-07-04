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

#define EVENT_LOG_PREFIX "meters_mqtt_mirr"

#include "meter_mqtt_mirror.h"

#include <ArduinoJson.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modules/meters/meter_value_id.h"

#include "gcc_warnings.h"

MeterClassID MeterMqttMirror::get_class() const
{
    return MeterClassID::MqttMirror;
}

void MeterMqttMirror::setup(const Config &ephemeral_config)
{
    const String &meter_path = ephemeral_config.get("meter_path")->asString();
    bool automatic = ephemeral_config.get("auto")->asBool();

    if (meter_path.length() < 3) {
        logger.printfln("Meter path too short: '%s'", meter_path.c_str());
        return;
    }

    String values_topic = meter_path;

    if (automatic) {
        values_topic.concat("/values");

        mqtt.subscribe(meter_path + "/value_ids",
            [this](const char *topic, size_t topic_len, char *data, size_t data_len) {
                this->onMessage(topic, topic_len, data, data_len, &MeterMqttMirror::handle_mqtt_value_ids);
        }, Mqtt::Retained::Accept);
    } else {
        logger.printfln("Manual mode not yet implemented.");
        return;
    }

    // Ignoring retained message. Need fresh data for the meter.
    mqtt.subscribe(values_topic,
        [this](const char *topic, size_t topic_len, char *data, size_t data_len) {
            this->onMessage(topic, topic_len, data, data_len, &MeterMqttMirror::handle_mqtt_values);
    }, Mqtt::Retained::IgnoreSilent);
}

void MeterMqttMirror::onMessage(const char *topic, size_t topic_len, char *data, size_t data_len, void (MeterMqttMirror::*message_handler)(const JsonArrayConst &json_array)) {
    StaticJsonDocument<JSON_ARRAY_SIZE(METERS_MAX_VALUES_PER_METER)> doc;

    DeserializationError error = deserializeJson(doc, data, data_len);
    if (error) {
        logger.printfln("Failed to deserialize payload received on MQTT topic %*s: %s", static_cast<int>(topic_len), topic, error.c_str());
        return;
    }

    const JsonArrayConst array = doc.as<JsonArrayConst>();
    (this->*message_handler)(array);
}

void MeterMqttMirror::handle_mqtt_value_ids(const JsonArrayConst &array)
{
    size_t array_size = array.size();
    if (array_size > METERS_MAX_VALUES_PER_METER) {
        logger.printfln("Too many value IDs from mirrored meter: %u > %i", array_size, METERS_MAX_VALUES_PER_METER);
        return;
    }

    if (array_size == 0) {
        logger.printfln("Ignoring zero-length value IDs update.");
        return;
    }

    MeterValueID value_ids[METERS_MAX_VALUES_PER_METER];
    size_t index = 0;
    // Use iterator because each getElement(index) call has a complexity of O(n).
    for (const JsonVariantConst v : array) {
        value_ids[index] = static_cast<MeterValueID>(v.as<uint32_t>());
        index++;
    }

    if (declared_values_count == 0) {
        meters.declare_value_ids(slot, value_ids, array_size);
        declared_values_count = array_size;
        accepting_updates = true;
        return;
    }

    // Had already declared values, need to verify.
    String value_ids_path;
    value_ids_path.reserve(18);
    value_ids_path = "meters/";
    value_ids_path.concat(slot);
    value_ids_path.concat("/value_ids");
    const Config *old_value_ids = api.getState(value_ids_path);

    bool value_ids_match;

    if (old_value_ids->count() != declared_values_count) {
        value_ids_match = false;
    } else {
        value_ids_match = true;
        for (uint32_t i = 0; i < declared_values_count; i++) {
            if (old_value_ids->get(static_cast<uint16_t>(i))->asUint() != static_cast<uint32_t>(value_ids[i])) {
                value_ids_match = false;
                break;
            }
        }
    }

    if (value_ids_match) {
        if (accepting_updates) {
            //logger.printfln("Ignoring matching value IDs update.");
        } else {
            logger.printfln("Received matching value IDs update; resuming value updates.");
            accepting_updates = true;
        }
    } else { // non-matching
        if (accepting_updates) {
            logger.printfln("Received non-matching value IDs update; suspending value updates.");
            accepting_updates = false;
        } else {
            logger.printfln("Ignoring repeated non-matching value IDs update.");
        }
    }
}

void MeterMqttMirror::handle_mqtt_values(const JsonArrayConst &array)
{
    if (!accepting_updates)
        return;

    if (array.size() != declared_values_count) {
        logger.printfln("Unexpected amount of values from mirrored meter: %u, expected %u", array.size(), declared_values_count);
        return;
    }

    float values[METERS_MAX_VALUES_PER_METER];
    size_t i = 0;
    // Use iterator because each getElement(index) call has a complexity of O(n).
    for (const JsonVariantConst v : array) {
        values[i] = v.as<float>();
        i++;
    }

    meters.update_all_values(slot, values);
}
