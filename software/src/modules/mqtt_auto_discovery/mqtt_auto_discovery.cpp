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

#include <TFJson.h>
#include <mqtt_client.h>
#include <string.h>

#include "build.h"
#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "language.h"
#include "options.h"

// Inject RAW, preformatted Json into the serializer. Must be valid JSON otherwise things might break
static void json_write_raw(TFJsonSerializer &json, const char *raw, size_t len)
{
    // Write comma separator if not the first element in the container
    if (!json.in_empty_container) {
        ++json.buf_required;
        if (json.buf_size > 0 && (size_t)(json.head - json.buf) <= (json.buf_size - 1)) {
            *json.head = ',';
            ++json.head;
        }
    }
    json.in_empty_container = false;

    json.buf_required += len;
    if (json.buf_size >= len && (size_t)(json.head - json.buf) <= (json.buf_size - len)) {
        memcpy(json.head, raw, len);
        json.head += len;
    }
}

void MqttAutoDiscovery::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"auto_discovery_mode", Config::Enum(MqttAutoDiscoveryMode::Disabled)},
        {"auto_discovery_prefix", Config::Str("homeassistant", 1, 64)},
        {"broadcast_empty", Config::Bool(false)}
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
    broadcast_empty = config_in_use.get("broadcast_empty")->asBool();

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
            size_t mode_idx = config_in_use.get("auto_discovery_mode")->asUint() - 1;

            // Pick language-specific static_info if available, otherwise fall back to the default (German).
            const char *static_info = mqtt_discovery_topic_infos[topic_num].static_infos[mode_idx];
            if (default_language == Language::English) {
                const char *en = mqtt_discovery_topic_infos[topic_num].static_infos_en[mode_idx];
                if (en != nullptr) {
                    static_info = en;
                }
            }

            if (static_info) { // No static info? Skip topic.
                const String &client_name = mqtt.client_name;
                const String &topic_prefix = mqtt.global_topic_prefix;
                const char *name = default_language == Language::English ? mqtt_discovery_topic_infos[topic_num].name_en : mqtt_discovery_topic_infos[topic_num].name_de;

                constexpr size_t json_doc_size = MQTT_DISCOVERY_MAX_JSON_LENGTH + 265 + 7 * 64 + 13 + 250;

                char *buf = static_cast<char *>(malloc(json_doc_size));
                memset(buf, 0, json_doc_size);
                TFJsonSerializer json(buf, json_doc_size);

                json.addObject();

                json.addMemberString("name", name);

                json.addMemberStringF("unique_id", "%s-%s", client_name.c_str(), mqtt_discovery_topic_infos[topic_num].object_id);
                json.addMemberStringF("default_entity_id", "%s.%s-%s", mqtt_discovery_topic_infos[topic_num].component, client_name.c_str(), mqtt_discovery_topic_infos[topic_num].object_id);
                json.addMemberStringF("object_id", "%s-%s", client_name.c_str(), mqtt_discovery_topic_infos[topic_num].object_id);

                switch (mqtt_discovery_topic_infos[topic_num].type) {
                    case MqttDiscoveryType::StateAndUpdate:
                        json.addMemberStringF("command_topic", "%s/%s_update", topic_prefix.c_str(), mqtt_discovery_topic_infos[topic_num].path);
                        /* FALLTHROUGH */
                    case MqttDiscoveryType::StateOnly:
                        json.addMemberStringF("state_topic", "%s/%s", topic_prefix.c_str(), mqtt_discovery_topic_infos[topic_num].path);
                        break;
                    case MqttDiscoveryType::CommandOnly:
                        json.addMemberStringF("command_topic", "%s/%s", topic_prefix.c_str(), mqtt_discovery_topic_infos[topic_num].path);
                        break;
                }

                if (mqtt_discovery_topic_infos[topic_num].availability_count > 0) {
                    json.addMemberArray("availability");
                    for (uint8_t i = 0; i < mqtt_discovery_topic_infos[topic_num].availability_count; i++) {
                        const auto &entry = mqtt_discovery_topic_infos[topic_num].availability[i];
                        json.addObject();
                        json.addMemberStringF("topic", "%s/%s", topic_prefix.c_str(), entry.topic);
                        json.addMemberString("value_template", entry.value_template);
                        json.endObject();
                    }
                    json.endArray();
                    json.addMemberString("availability_mode", "all");
                }

                if (mqtt_discovery_topic_infos[topic_num].create_disabled) {
                    json.addMemberBoolean("enabled_by_default", false);
                }

                if (strlen(mqtt_discovery_topic_infos[topic_num].json_attributes_topic) > 0) {
                    json.addMemberStringF("json_attributes_topic", "%s/%s", topic_prefix.c_str(), mqtt_discovery_topic_infos[topic_num].json_attributes_topic);
                    json_write_raw(json, mqtt_discovery_topic_infos[topic_num].json_attributes_info, strlen(mqtt_discovery_topic_infos[topic_num].json_attributes_info));
                }

                // Inject pre-formatted static_info as raw JSON object members
                json_write_raw(json, static_info, strlen(static_info));

                json.addMemberObject("device");
                json.addMemberString("identifiers", mqtt.client_name.c_str());
                json.addMemberString("manufacturer", OPTIONS_MANUFACTURER_FULL());
                json.addMemberString("model", OPTIONS_PRODUCT_NAME());
                json.addMemberStringF("name", "%s (%s)", OPTIONS_PRODUCT_NAME(), mqtt.client_name.c_str());
                json.endObject();

                json.endObject();

                String json_str = buf;
                json_str.trim();
                mqtt.publish(mqtt_discovery_topics[topic_num].full_path, json_str, true);
                free(buf);

            }
        } else if (broadcast_empty && mqtt_discovery_topics[topic_num].full_path.length() > 0) {
            // Entity is not enabled; send empty payload to remove it from HA.
            mqtt.publish(mqtt_discovery_topics[topic_num].full_path, String(), true);
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
