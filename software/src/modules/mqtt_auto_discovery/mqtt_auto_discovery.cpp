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

#include <string.h>

#include "mqtt_client.h"

#include "api.h"
#include "build.h"
#include "modules.h"
#include "mqtt_auto_discovery.h"
#include "task_scheduler.h"

void MqttAutoDiscovery::pre_setup()
{
    config = ConfigRoot(Config::Object({
        {"auto_discovery_mode", Config::Uint(MQTT_AUTO_DISCOVERY_MODE_MIN, MQTT_AUTO_DISCOVERY_MODE_MIN, MQTT_AUTO_DISCOVERY_MODE_MAX)},
        {"auto_discovery_prefix", Config::Str("homeassistant", 1, 64)}
    }),  [](Config &cfg) -> String {
        const String &global_topic_prefix = mqtt.mqtt_config.get("global_topic_prefix")->asString();
        const String &auto_discovery_prefix = cfg.get("auto_discovery_prefix")->asString();

        if (global_topic_prefix == auto_discovery_prefix)
            return "Auto discovery topic prefix cannot be the same as the MQTT API topic prefix.";

        return "";
    });
}

void MqttAutoDiscovery::setup()
{
    api.restorePersistentConfig("mqtt/auto_discovery_config", &config);

    config_in_use = config;
    initialized = true;

    const String &global_topic_prefix = mqtt.mqtt_config_in_use.get("global_topic_prefix")->asString();
    const String &auto_discovery_prefix = config_in_use.get("auto_discovery_prefix")->asString();
    subscribed_topics_difference_at = 0;

    // Include zero-termination in comparison, in case global_topic_prefix is entirely a prefix of auto_discovery_prefix.
    for (size_t i = 0; i < global_topic_prefix.length() + 1; ++i) {
        if (global_topic_prefix[i] != auto_discovery_prefix[i]) {
            subscribed_topics_difference_at = i;
            subscribed_topics_difference_commands = global_topic_prefix[i];
            subscribed_topics_difference_discovery = auto_discovery_prefix[i];
            break;
        }
    }
    if (subscribed_topics_difference_commands == '\0')
        subscribed_topics_difference_commands = '/';
    if (subscribed_topics_difference_discovery == '\0')
        subscribed_topics_difference_discovery = '/';

    if (config.get("auto_discovery_mode")->asEnum<MqttAutoDiscoveryMode>() == MqttAutoDiscoveryMode::DISCOVERY_DISABLED)
        return;

    mqtt_auto_discovery.prepare_topics();

    task_scheduler.scheduleOnce([this](){
        this->announce_next_topic(0);
    }, 1000);
}

void MqttAutoDiscovery::register_urls()
{
    api.addPersistentConfig("mqtt/auto_discovery_config", &config, {}, 1000);
}

void MqttAutoDiscovery::onMqttConnect()
{
    // Always subscribe to own discovery topics. Clears all topics when auto discovery is not enabled.
    mqtt_auto_discovery.subscribe_to_own();
}

bool MqttAutoDiscovery::onMqttMessage(char *topic, size_t topic_len, char *data, size_t data_len, bool retain)
{
    if (topic_len <= subscribed_topics_difference_at) {
        String tp;
        tp.concat(topic, topic_len);
        logger.printfln("MQTT: Received invalid short topic: '%s'", topic);
        return true;
    }

    if (topic[subscribed_topics_difference_at] == subscribed_topics_difference_discovery) {
        mqtt_auto_discovery.check_discovery_topic(topic, topic_len, data_len);
        return true;
    }

    return false;
}

void MqttAutoDiscovery::prepare_topics()
{
    const String &auto_discovery_prefix = config_in_use.get("auto_discovery_prefix")->asString();
    const String &client_name = mqtt.mqtt_config_in_use.get("client_name")->asString();
    const MqttAutoDiscoveryMode mode = config_in_use.get("auto_discovery_mode")->asEnum<MqttAutoDiscoveryMode>();
    unsigned int topic_length;

    if (mode == MqttAutoDiscoveryMode::DISCOVERY_DISABLED)
        return;

    for (size_t i = 0; i < TOPIC_COUNT; ++i) {
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

    device_info.reserve(512);

    device_info.concat("\"device\":{\"identifiers\":\"");
    device_info.concat(client_name);
    device_info.concat("\",\"manufacturer\":\"Tinkerforge GmbH\",\"model\":\"" BUILD_DISPLAY_NAME "\",\"name\":\"" BUILD_DISPLAY_NAME " (");
    device_info.concat(client_name);
    device_info.concat(")\"}");

    device_info.shrinkToFit();
}

void MqttAutoDiscovery::subscribe_to_own()
{
    // <discovery_prefix>/+/<node_id>/+/config
    String topic;
    topic.reserve(256); // no need to be efficient here: esp_mqtt_client_subscribe copies this string

    topic.concat(config_in_use.get("auto_discovery_prefix")->asString());
    topic.concat("/+/");
    topic.concat(mqtt.mqtt_config_in_use.get("client_name")->asString());
    topic.concat("/+/config");

    esp_mqtt_client_subscribe(mqtt.client, topic.c_str(), 0);
}

void MqttAutoDiscovery::check_discovery_topic(const char *topic, size_t topic_len, size_t data_len)
{
    // auto discovery is disabled. remove all entities
    if (config_in_use.get("auto_discovery_mode")->asEnum<MqttAutoDiscoveryMode>() == MqttAutoDiscoveryMode::DISCOVERY_DISABLED) {
        if (data_len == 0) //already removed
            return;

        String tp(topic, topic_len);
        mqtt.publish(tp, String(), true);
        return;
    }

    for (size_t i = 0; i < TOPIC_COUNT; ++i) {
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
    logger.printfln("MQTT auto discovery: Removing unused topic '%s'.", tp.c_str());
    mqtt.publish(tp, String(), true);
}

void MqttAutoDiscovery::announce_next_topic(uint32_t topic_num)
{
    uint32_t delay_ms = 0;

    if (mqtt.mqtt_state.get("connection_state")->asInt() != (int)MqttConnectionState::CONNECTED) {
        topic_num = 0;
        delay_ms = 5 * 1000;
    } else {
        // deal with one topic
        if (api.hasFeature(mqtt_discovery_topic_infos[topic_num].feature)) {
            const char *static_info = mqtt_discovery_topic_infos[topic_num].static_infos[config_in_use.get("auto_discovery_mode")->asUint() - 1];
            if (static_info) { // No static info? Skip topic.
                const String &topic_prefix = mqtt.mqtt_config_in_use.get("global_topic_prefix")->asString();
                const char *name = mqtt_discovery_topic_infos[topic_num].name_de;

                String payload;
                // MAX_JSON_LEN: max length generated by prepare.py
                // 100: String literals
                // 3*64: topic_prefix (twice) and client name
                // 250: device_info
                payload.reserve(MAX_JSON_LEN + 100 + 64 + 64 + 64 + 250);

                payload.concat("{\"name\":\"");
                payload.concat(name);
                payload.concat("\",\"unique_id\":\"");
                payload.concat(mqtt.mqtt_config_in_use.get("client_name")->asString());
                payload.concat('-');
                payload.concat(mqtt_discovery_topic_infos[topic_num].object_id);
                payload.concat("\",");
                switch (mqtt_discovery_topic_infos[topic_num].type) {
                    case MqttDiscoveryType::STATE_AND_UPDATE:
                        payload.concat("\"command_topic\":\"");
                        payload.concat(topic_prefix);
                        payload.concat('/');
                        payload.concat(mqtt_discovery_topic_infos[topic_num].path);
                        payload.concat("_update\",");
                        /* FALLTHROUGH */
                    case MqttDiscoveryType::STATE_ONLY:
                        payload.concat("\"state_topic\":\"");
                        payload.concat(topic_prefix);
                        payload.concat('/');
                        payload.concat(mqtt_discovery_topic_infos[topic_num].path);
                        payload.concat("\",");
                        break;
                    case MqttDiscoveryType::COMMAND_ONLY:
                        payload.concat("\"command_topic\":\"");
                        payload.concat(topic_prefix);
                        payload.concat('/');
                        payload.concat(mqtt_discovery_topic_infos[topic_num].path);
                        payload.concat("\",");
                        break;
                }
                payload.concat(static_info);
                payload.concat(',');
                payload.concat(device_info);
                payload.concat('}');

                mqtt.publish(mqtt_discovery_topics[topic_num].full_path, payload, true);
            }
        }

        if (++topic_num >= TOPIC_COUNT) {
            topic_num = 0;
            delay_ms = 15 * 60 * 1000;
        }
    }

    task_scheduler.scheduleOnce([this, topic_num](){
        this->announce_next_topic(topic_num);
    }, delay_ms);
}

