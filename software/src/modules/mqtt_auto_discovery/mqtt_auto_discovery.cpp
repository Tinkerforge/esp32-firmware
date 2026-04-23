/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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

#define EVENT_LOG_PREFIX "mqtt_disco"

#include "mqtt_auto_discovery.h"

#include <string.h>
#include <mqtt_client.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "build.h"
#include "options.h"
#include "language.h"

void MqttAutoDiscovery::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"auto_discovery_mode", Config::Enum(MqttAutoDiscoveryMode::Disabled)},
        {"auto_discovery_prefix", Config::Str("homeassistant", 1, 64)}
    }),  [](Config &cfg, ConfigSource source) -> String {
        const String &global_topic_prefix = mqtt.global_topic_prefix;
        const String &auto_discovery_prefix = cfg.get("auto_discovery_prefix")->asString();

        if (global_topic_prefix == auto_discovery_prefix)
            return "Auto discovery topic prefix cannot be the same as the MQTT API topic prefix.";

        return "";
    }};
}

void MqttAutoDiscovery::setup()
{
    api.restorePersistentConfig("mqtt/auto_discovery_config", &config);

    config_in_use = config;
    mode = config_in_use.get("auto_discovery_mode")->asEnum<MqttAutoDiscoveryMode>();

    initialized = true;

    if (config_in_use.get("auto_discovery_mode")->asEnum<MqttAutoDiscoveryMode>() == MqttAutoDiscoveryMode::Disabled)
        return;

    prepare_topics();

    task_id = task_scheduler.scheduleOnce([this](){
        this->announce_next_topic(0);
    }, 1_s);

    // <discovery_prefix>/+/<node_id>/+/config
    String discovery_topic;
    discovery_topic.reserve(256); // no need to be efficient here: esp_mqtt_client_subscribe copies this string

    discovery_topic.concat(config_in_use.get("auto_discovery_prefix")->asString());
    discovery_topic.concat("/+/");
    discovery_topic.concat(mqtt.client_name);
    discovery_topic.concat("/+/config");

    mqtt.subscribe(discovery_topic, [this](const char *topic, size_t topic_len, char *data, size_t data_len) {
        check_discovery_topic(topic, topic_len, data_len);
    }, Mqtt::Retained::Accept);
}

void MqttAutoDiscovery::register_urls()
{
    api.addPersistentConfig("mqtt/auto_discovery_config", &config);
}

void MqttAutoDiscovery::register_events()
{
#if MODULE_SYSTEM_AVAILABLE()
    event.registerEvent("system/i18n_config", {"language"}, [this](const Config */*language*/) {
        reschedule_announce_next_topic();
        return EventResult::OK;
    });
#endif
}

void MqttAutoDiscovery::prepare_topics()
{
    const String &auto_discovery_prefix = config_in_use.get("auto_discovery_prefix")->asString();
    const String &client_name = mqtt.client_name;
    const MqttAutoDiscoveryMode mode = config_in_use.get("auto_discovery_mode")->asEnum<MqttAutoDiscoveryMode>();
    unsigned int topic_length;

    if (mode == MqttAutoDiscoveryMode::Disabled)
        return;

    for (size_t i = 0; i < MQTT_DISCOVERY_TOPIC_COUNT; ++i) {
        const char *static_info = mqtt_discovery_topic_infos[i].static_infos[(size_t)mode - 1];
        if (!static_info) // No static info? Skip topic.
            continue;

        // <discovery_prefix>/<component>/<node_id>/<object_id>/config
        topic_length = auto_discovery_prefix.length() + strlen(mqtt_discovery_topic_infos[i].component)
            + client_name.length() + strlen(mqtt_discovery_topic_infos[i].object_id) + 10; // "config" + 4*'/' = 10

        mqtt_discovery_topics[i].full_path.reserve(topic_length);

        mqtt_discovery_topics[i].full_path.concat(auto_discovery_prefix);
        mqtt_discovery_topics[i].full_path.concat('/');
        mqtt_discovery_topics[i].full_path.concat(mqtt_discovery_topic_infos[i].component);
        mqtt_discovery_topics[i].full_path.concat('/');
        mqtt_discovery_topics[i].full_path.concat(client_name);
        mqtt_discovery_topics[i].full_path.concat('/');
        mqtt_discovery_topics[i].full_path.concat(mqtt_discovery_topic_infos[i].object_id);
        mqtt_discovery_topics[i].full_path.concat("/config");
    }
}

void MqttAutoDiscovery::subscribe_to_own()
{
}

void MqttAutoDiscovery::check_discovery_topic(const char *topic, size_t topic_len, size_t data_len)
{
    // auto discovery is disabled. remove all entities
    if (this->mode == MqttAutoDiscoveryMode::Disabled) {
        if (data_len == 0) //already removed
            return;

        String tp(topic, topic_len);
        mqtt.publish(tp, String(), true);
        return;
    }

    for (size_t i = 0; i < MQTT_DISCOVERY_TOPIC_COUNT; ++i) {
        if (mqtt_discovery_topics[i].full_path.length() != topic_len)
            continue;

        if (memcmp(mqtt_discovery_topics[i].full_path.c_str(), topic, topic_len) == 0) {
            // Discovery topic is known; nothing to do.
            return;
        }
    }

    // Unknown discovery topic with zero-length data probably caused by us removing it. Catch it to avoid an infinite loop.
    if (data_len == 0)
        return;

    // topic is not null-terminated and needs to be copied to terminate properly.
    String tp(topic, topic_len);

    // Unknown discovery topic with data; needs to be removed by sending a retained empty payload.
    logger.printfln("Removing unused topic '%s'.", tp.c_str());
    mqtt.publish(tp, String(), true);
}

void MqttAutoDiscovery::reschedule_announce_next_topic()
{
    if (config_in_use.get("auto_discovery_mode")->asEnum<MqttAutoDiscoveryMode>() == MqttAutoDiscoveryMode::Disabled)
        return;

    task_scheduler.cancel(task_id);
    task_id = task_scheduler.scheduleOnce([this](){
        this->announce_next_topic(0);
    }, 1_s);
}

void MqttAutoDiscovery::announce_next_topic(uint32_t topic_num)
{
    seconds_t delay = 0_s;

    if (mqtt.state.get("connection_state")->asEnum<MqttConnectionState>() != MqttConnectionState::Connected) {
        topic_num = 0;
        delay = 5_s;
    } else {
        // deal with one topic
        if (api.hasFeature(mqtt_discovery_topic_infos[topic_num].feature)) {
            const char *static_info = mqtt_discovery_topic_infos[topic_num].static_infos[config_in_use.get("auto_discovery_mode")->asUint() - 1];
            if (static_info) { // No static info? Skip topic.
                const String &client_name = mqtt.client_name;
                const String &topic_prefix = mqtt.global_topic_prefix;
                const char *name = default_language == Language::English ? mqtt_discovery_topic_infos[topic_num].name_en : mqtt_discovery_topic_infos[topic_num].name_de;

                auto concat_string = [](const auto&... args) -> String {
                    String out;
                    (out.concat(args), ...);
                    return out;
                };
                // MQTT_DISCOVERY_MAX_JSON_LENGTH: max length generated by prepare.py
                // 265: String literals
                // 7*64: topic_prefix (four times) and client name (thrice)
                // 13: component (max length is "binary_sensor")
                // 250: device_info
                StaticJsonDocument<MQTT_DISCOVERY_MAX_JSON_LENGTH + 265 + 7 * 64 + 13 + 250> doc; // TODO: Maybe move this to psram similar to EEBUS

                doc["name"] = name;
                doc["unique_id"] = concat_string(client_name, "-", mqtt_discovery_topic_infos[topic_num].object_id);
                doc["default_entity_id"] = concat_string(mqtt_discovery_topic_infos[topic_num].component, ".", client_name, "-", mqtt_discovery_topic_infos[topic_num].object_id);
                doc["object_id"] = concat_string(client_name, "-", mqtt_discovery_topic_infos[topic_num].object_id);
                switch (mqtt_discovery_topic_infos[topic_num].type) {
                    case MqttDiscoveryType::StateAndUpdate:
                        doc["command_topic"] = concat_string(topic_prefix, '/', mqtt_discovery_topic_infos[topic_num].path, "_update");
                        /* FALLTHROUGH */
                    case MqttDiscoveryType::StateOnly:
                        doc["state_topic"] = concat_string(topic_prefix, '/', mqtt_discovery_topic_infos[topic_num].path);
                        break;
                    case MqttDiscoveryType::CommandOnly:
                        doc["command_topic"] = concat_string(topic_prefix, '/', mqtt_discovery_topic_infos[topic_num].path);
                        break;
                }
                if (mqtt_discovery_topic_infos[topic_num].availability_path[0] != 0) {
                    JsonObject availability = doc.createNestedObject("availability");
                    availability["topic"] = concat_string(topic_prefix, '/', mqtt_discovery_topic_infos[topic_num].availability_path);
                    availability["payload_available"] = mqtt_discovery_topic_infos[topic_num].availability_yes;
                    availability["payload_not_available"] = mqtt_discovery_topic_infos[topic_num].availability_no;
                }

                String wrapped_static_info = concat_string("{", static_info, "}");

                StaticJsonDocument<MQTT_DISCOVERY_MAX_JSON_LENGTH + 265> static_json_doc;
                DeserializationError err = deserializeJson(static_json_doc, wrapped_static_info);

                if (err) {
                    logger.printfln("Failed to parse static_info: %s", err.c_str()); // TODO: move to tracelog
                } else {
                    JsonObject src = static_json_doc.as<JsonObject>();
                    for (JsonPair kv : src) {
                        doc[kv.key()] = kv.value();
                    }
                }

                JsonObject device = doc.createNestedObject("device");
                device["identifiers"] = mqtt.client_name;
                device["manufacturer"] = OPTIONS_MANUFACTURER_FULL();
                device["model"] = OPTIONS_PRODUCT_NAME();
                device["name"] = concat_string(OPTIONS_PRODUCT_NAME(), " (", mqtt.client_name, ")");

                String payload;
                payload.reserve(MQTT_DISCOVERY_MAX_JSON_LENGTH + 265 + 7 * 64 + 13 + 250);
                serializeJson(doc, payload);

                mqtt.publish(mqtt_discovery_topics[topic_num].full_path, payload, true);
            }
        }

        if (++topic_num >= MQTT_DISCOVERY_TOPIC_COUNT) {
            topic_num = 0;
            delay = 15_min;
        }
    }

    task_id = task_scheduler.scheduleOnce([this, topic_num](){
        this->announce_next_topic(topic_num);
    }, delay);
}
