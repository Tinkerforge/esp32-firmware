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

#include "mqtt_meter.h"
#include "module_dependencies.h"

#include <math.h>
#include <string.h>
#include "mqtt_client.h"

#include "api.h"

#include "gcc_warnings.h"

void MqttMeter::pre_setup()
{
    config = ConfigRoot(Config::Object({
        {"enable", Config::Bool(false)},
        {"has_all_values", Config::Bool(false)},
        {"source_meter_path", Config::Str("path/to/meter", 0, 128)},
    }),  [](Config &cfg, ConfigSource source) -> String {
        const String &global_topic_prefix = mqtt.config.get("global_topic_prefix")->asString();
        const String &meter_path = cfg.get("source_meter_path")->asString();

        if (meter_path.startsWith(global_topic_prefix))
            return "Cannot listen to itself: Source meter path cannot start with the topic prefix from the MQTT config.";

        return "";
    });
}

void MqttMeter::setup()
{
    // Module is always initialized, even if disabled.
    initialized = true;

    api.restorePersistentConfig("mqtt_meter/config", &config);

    bool enabled = config.get("enable")->asBool();
    const String &source_meter_path = config.get("source_meter_path")->asString();

    if (!enabled || source_meter_path.isEmpty())
        return;

    logger.printfln("mqtt_meter: Please ignore any log messages about ignored retained messages on values and all_values.");

    // Ignoring retained message. Need fresh data for the meter.
    mqtt.subscribe(source_meter_path + "/values",
        [this](const char *topic, size_t topic_len, char *data, size_t data_len) {
            this->onMessage(topic, topic_len, data, data_len, &MqttMeter::handle_mqtt_values);
    }, true);

    if (!config.get("has_all_values")->asBool()) {
        mqtt_meter_type = METER_TYPE_CUSTOM_BASIC;
        return;
    }

    mqtt_meter_type = METER_TYPE_CUSTOM_ALL_VALUES;

    // Ignoring retained message. Need fresh data for the meter.
    mqtt.subscribe(source_meter_path + "/all_values",
        [this](const char *topic, size_t topic_len, char *data, size_t data_len) {
            this->onMessage(topic, topic_len, data, data_len, &MqttMeter::handle_mqtt_all_values);
    }, true);
}

void MqttMeter::register_urls()
{
    api.addPersistentConfig("mqtt_meter/config", &config);
}

void MqttMeter::onMessage(const char *topic, size_t topic_len, char *data, size_t data_len, void (MqttMeter::*message_handler)(const JsonDocument &doc)) {
    // A document large enough to hold all_values is more than enough to hold values.
    StaticJsonDocument<JSON_ARRAY_SIZE(METER_ALL_VALUES_COUNT)> doc;

    DeserializationError error = deserializeJson(doc, data, data_len);
    if (error) {
        logger.printfln("mqtt_meter: Failed to deserialize payload to MQTT topic %*s: %s", static_cast<int>(topic_len), topic, error.c_str());
        return;
    }

    uint8_t meter_type = static_cast<uint8_t>(meter.state.get("type")->asUint());
    if (meter_type != mqtt_meter_type) {
        if (meter_type != METER_TYPE_NONE) {
            logger.printfln("mqtt_meter: Detected presence of a conflicting meter, type %u. The MQTT meter should be disabled.", meter_type);
            return;
        }
        logger.printfln("mqtt_meter: Setting meter type to %u.", mqtt_meter_type);
        meter.updateMeterState(2, mqtt_meter_type);
    }

    (this->*message_handler)(doc);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
static inline bool float_equals_zero(float v)
{
    return v == 0;
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

void MqttMeter::handle_mqtt_values(const JsonDocument &doc)
{
    float power      = doc["power"     ].as<float>();
    float energy_rel = doc["energy_rel"].as<float>();
    float energy_abs = doc["energy_abs"].as<float>();

    if (float_equals_zero(energy_rel)) energy_rel = NAN;
    if (float_equals_zero(energy_abs)) energy_abs = NAN;

#if MODULE_EM_PV_FAKER_AVAILABLE()
    power -= static_cast<float>(em_pv_faker.state.get("fake_power")->asInt());
#endif

    meter.updateMeterValues(power, energy_rel, energy_abs);
}

void MqttMeter::handle_mqtt_all_values(const JsonDocument &doc)
{
    JsonArrayConst array = doc.as<JsonArrayConst>();

    if (array.size() != METER_ALL_VALUES_COUNT) {
        logger.printfln("mqtt_meter: Unexpected amount of meter values: %u/%i", array.size(), METER_ALL_VALUES_COUNT);
        return;
    }

    float all_values[METER_ALL_VALUES_COUNT];

    int i = 0;
    // Use iterator because each getElement(index) call has a complexity of O(n).
    for (const JsonVariantConst v : array) {
        all_values[i] = v.as<float>();
        i++;
    }

#if MODULE_EM_PV_FAKER_AVAILABLE()
    float pv_power = static_cast<float>(em_pv_faker.state.get("fake_power")->asInt());
    float pv_phase_current = pv_power * (1.0f / (3.0f * 230.0f));

    all_values[METER_ALL_VALUES_CURRENT_L1_A] -= pv_phase_current; // METER_ALL_VALUES_CURRENT_DEMAND_L1_A ?
    all_values[METER_ALL_VALUES_CURRENT_L2_A] -= pv_phase_current;
    all_values[METER_ALL_VALUES_CURRENT_L3_A] -= pv_phase_current;
    all_values[METER_ALL_VALUES_TOTAL_SYSTEM_POWER_W] -= pv_power; // METER_ALL_VALUES_TOTAL_SYSTEM_POWER_DEMAND_W ?
#endif

    meter.updateMeterAllValues(all_values);
}
