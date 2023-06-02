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

#include "em_pv_faker.h"

#include "api.h"
#include "event_log.h"
#include "modules.h"
#include "task_scheduler.h"

#include "mqtt_client.h"

#include "gcc_warnings.h"

#if !MODULE_ENERGY_MANAGER_AVAILABLE()
#error Back-end module Energy Manager required
#endif
#if !MODULE_MQTT_AVAILABLE()
#error Back-end module MQTT required
#endif

void EmPvFaker::pre_setup()
{
    state = Config::Object({
        {"illuminance", Config::Uint32(0)},
        {"fake_power", Config::Int32(0)},
    });

    config = ConfigRoot{Config::Object({
        {"auto_fake",   Config::Bool(false)},
        {"topic",       Config::Str("", 0, 128)},
        {"peak_power",  Config::Uint32(30*1000)},  // watt
        {"zero_at_lux", Config::Uint32(100)},      // lux
        {"peak_at_lux", Config::Uint32(105*1000)}, // lux
        {"filter_time_constant", Config::Uint(0, 0, 600)}, // s
    }), [](Config &conf) -> String {
        if (conf.get("zero_at_lux")->asUint() >= conf.get("peak_at_lux")->asUint())
            return "Lux value for zero production must be less than lux value for peak production.";

        const String &global_topic_prefix = mqtt.mqtt_config.get("global_topic_prefix")->asString();
        const String &illuminance_topic = conf.get("topic")->asString();

        if (illuminance_topic.startsWith(global_topic_prefix))
            return "Cannot listen to itself: Illuminance topic cannot start with the topic prefix from the MQTT config.";

        return "";
    }};

    runtime_config = Config::Object({
        {"manual_power", Config::Int32(0)},
    });
    runtime_config_update = runtime_config;
}

void EmPvFaker::setup()
{
    api.restorePersistentConfig("em_pv_faker/config", &config);

    Config *conf = static_cast<Config *>(energy_manager.config_in_use.get("target_power_from_grid"));
    if (!conf) {
        logger.printfln("em_pv_faker: energy_manager config target_power_from_grid not available. Disabling em_pv_faker.");
        return;
    }

    int32_t target_power = conf->asInt();
    state.get("fake_power")->updateInt(target_power);
    runtime_config.get("manual_power")->updateInt(target_power);

    initialized = true;
}

void EmPvFaker::register_urls()
{
    api.addState("em_pv_faker/state",             &state,  {}, 1000);
    api.addPersistentConfig("em_pv_faker/config", &config, {}, 1000);

    api.addState("em_pv_faker/runtime_config", &runtime_config, {}, 1000);
    api.addCommand("em_pv_faker/runtime_config_update", &runtime_config_update, {}, [this](){
        int32_t new_power = runtime_config_update.get("manual_power")->asInt();

        runtime_config.get("manual_power")->updateInt(new_power);

        if (!config.get("auto_fake")->asBool())
            state.get("fake_power")->updateInt(new_power);
    }, false);
}

void EmPvFaker::onMqttConnect()
{
    // Always subscribe to illuminance topic even when auto_fake is not on
    // because it can be turned on at runtime.

    const String &topic = config.get("topic")->asString();

    if (topic.length() < 3)
        return;

    esp_mqtt_client_subscribe(mqtt.client, topic.c_str(), 0);
}

bool EmPvFaker::onMqttMessage(char *topic, size_t topic_len, char *data, size_t data_len, bool retain)
{
    const String &illuminance_topic = config.get("topic")->asString();
    if (illuminance_topic.length() != topic_len || memcmp(illuminance_topic.c_str(), topic, topic_len) != 0) {
        // Not our topic
        return false;
    }

    StaticJsonDocument<64> doc;
    DeserializationError error = deserializeJson(doc, data, data_len);
    if (error) {
        logger.printfln("em_pv_faker: Failed to deserialize MQTT payload: %s", error.c_str());
        return true;
    }

    uint32_t illuminance = doc["illuminance"].as<uint32_t>();
    state.get("illuminance")->updateUint(illuminance);

    if (!config.get("auto_fake")->asBool())
        return true;

    uint64_t peak_power          = config.get("peak_power" )->asUint();
    uint32_t zero_at_lux         = config.get("zero_at_lux")->asUint();
    uint32_t peak_at_lux         = config.get("peak_at_lux")->asUint();
    uint64_t peak_at_lux_shifted = peak_at_lux - zero_at_lux;

    if (illuminance < zero_at_lux) {
        illuminance = zero_at_lux;
    } else if (illuminance > peak_at_lux) {
        illuminance = peak_at_lux;
    }

    uint64_t illuminance_shifted = illuminance - zero_at_lux;

    int32_t fake_power_w = static_cast<int32_t>(peak_power * illuminance_shifted / peak_at_lux_shifted);
    state.get("fake_power")->updateInt(fake_power_w);

    return true;
}
