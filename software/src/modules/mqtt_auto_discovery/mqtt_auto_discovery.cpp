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
#include <format>

#include "build.h"
#include "event_log_prefix.h"
#include "modules/meters/generated/meter_class_id.enum.h"
#include "generated/module_dependencies.h"
#include "language.h"
#include "options.h"
#include "tools/string_builder.h"

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
        {"auto_discovery_prefix", Config::Str("homeassistant", 1, 64)}
    }),  [this](Config &cfg, ConfigSource source) -> String {
        const String &global_topic_prefix = mqtt.global_topic_prefix;
        const String &auto_discovery_prefix = cfg.get("auto_discovery_prefix")->asString();

        if (global_topic_prefix == auto_discovery_prefix)
            return "Auto discovery topic prefix cannot be the same as the MQTT API topic prefix.";

        if (source == ConfigSource::File)
            return "";

        // Stop with old config, start with new one.
        this->stop();
        task_scheduler.scheduleOnce([this](){ this->start(); });

        return "";
    }};
}

void MqttAutoDiscovery::setup()
{
    api.restorePersistentConfig("mqtt/auto_discovery_config", &config);

    initialized = true;
    this->start();
}

void MqttAutoDiscovery::stop()
{
    if (this->mode == MqttAutoDiscoveryMode::Disabled)
        return;

    task_scheduler.cancel(this->task_id);
    this->task_id = 0;

    // <discovery_prefix>/+/<node_id>/+/config
    String discovery_topic;
    discovery_topic.reserve(256); // no need to be efficient here: esp_mqtt_client_subscribe copies this string

    discovery_topic.concat(this->prefix);
    discovery_topic.concat("/+/");
    discovery_topic.concat(mqtt.client_name);
    discovery_topic.concat("/+/config");
    mqtt.unsubscribe(discovery_topic);

    for (size_t topic_num = 0; topic_num < MQTT_DISCOVERY_TOPIC_COUNT; ++topic_num) {
        CoolString topic;
        size_t topic_len = this->mqtt_discovery_topic_lengths[topic_num];
        topic.reserve(topic_len + 1);
        this->get_discovery_topic(topic_num, topic.begin(), topic_len + 1);
        topic.setLength(topic_len);

        mqtt.publish(topic, String(), true);
    }

    mqtt_discovery_topic_lengths = nullptr;
}

void MqttAutoDiscovery::start()
{
    this->mode = config.get("auto_discovery_mode")->asEnum<MqttAutoDiscoveryMode>();
    this->prefix = config.get("auto_discovery_prefix")->asString();

    if (this->mode == MqttAutoDiscoveryMode::Disabled)
        return;

    prepare_topic_lengths();

    // <discovery_prefix>/+/<node_id>/+/config
    String discovery_topic;
    discovery_topic.reserve(256); // no need to be efficient here: esp_mqtt_client_subscribe copies this string

    discovery_topic.concat(this->prefix);
    discovery_topic.concat("/+/");
    discovery_topic.concat(mqtt.client_name);
    discovery_topic.concat("/+/config");

    mqtt.subscribe(discovery_topic, [this](const char *topic, size_t topic_len, char *data, size_t data_len) {
        check_discovery_topic(topic, topic_len, data_len);
    }, Mqtt::Retained::Accept);

    task_id = task_scheduler.scheduleWithFixedDelay([this](){
        this->announce_next_topic();
    }, 1_s);
}

void MqttAutoDiscovery::register_urls()
{
    api.addPersistentConfig("mqtt/auto_discovery_config", &config);
}

void MqttAutoDiscovery::register_events()
{
#if MODULE_SYSTEM_AVAILABLE()
    event.registerEvent("system/i18n_config", {"language"}, [this](const Config */*language*/) {
        task_scheduler.rescheduleNow(this->task_id);
        return EventResult::OK;
    });
#endif
}

size_t MqttAutoDiscovery::get_discovery_topic(size_t topic_idx, char *buf, size_t buf_len)
{
    if (size_t topic_len = this->mqtt_discovery_topic_lengths[topic_idx]; topic_len >= buf_len)
        esp_system_abortf<96>("topic length mismatch; expected %zu, got %zu", topic_len, buf_len);

    buf[0] = '\0';

    const auto &info = mqtt_discovery_topic_infos[topic_idx];

    const char *static_info = info.static_infos[(size_t)this->mode - 1];
    if (!static_info) // No static info? Skip topic.
        return 0;

    StringWriter sw{buf, buf_len};

    sw.puts(this->prefix);
    sw.putc('/');
    sw.puts(info.component);
    sw.putc('/');
    sw.puts(mqtt.client_name);
    sw.putc('/');
    sw.puts(info.object_id);
    sw.puts("/config");
    return sw.getLength();
}

void MqttAutoDiscovery::prepare_topic_lengths()
{
    const String &client_name = mqtt.client_name;

    mqtt_discovery_topic_lengths = heap_alloc_array<uint8_t>(MQTT_DISCOVERY_TOPIC_COUNT);

    for (size_t i = 0; i < MQTT_DISCOVERY_TOPIC_COUNT; ++i) {
        const char *static_info = mqtt_discovery_topic_infos[i].static_infos[(size_t)this->mode - 1];
        if (!static_info) // No static info? Skip topic.
            continue;

        // <discovery_prefix>/<component>/<node_id>/<object_id>/config
        size_t topic_length = this->prefix.length() + strlen(mqtt_discovery_topic_infos[i].component)
            + client_name.length() + strlen(mqtt_discovery_topic_infos[i].object_id) + 10; // "config" + 4*'/' = 10

        if (size_t max_len = std::numeric_limits<decltype(mqtt_discovery_topic_lengths)::element_type>::max(); topic_length > max_len)
            esp_system_abortf<96>("Topic length too long: is %zu, max allowed %zu", topic_length, max_len);

        mqtt_discovery_topic_lengths[i] = topic_length;
    }
}

void MqttAutoDiscovery::check_discovery_topic(const char *topic, size_t topic_len, size_t data_len)
{
    for (size_t i = 0; i < MQTT_DISCOVERY_TOPIC_COUNT; ++i) {
        if (mqtt_discovery_topic_lengths[i] != topic_len)
            continue;

        char buf[255];
        this->get_discovery_topic(i, buf, 255);

        if (memcmp(buf, topic, topic_len) == 0) {
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
    mqtt.publish(tp, String(), true);
}

void MqttAutoDiscovery::announce_next_topic()
{
    if (mqtt.state.get("connection_state")->asEnum<MqttConnectionState>() != MqttConnectionState::Connected) {
        this->next_topic = 0;
        task_scheduler.updateCurrentTaskDelay(5_s);
        return;
    }

    auto topic_idx = this->next_topic;
    if (++this->next_topic >= MQTT_DISCOVERY_TOPIC_COUNT) {
        this->next_topic = 0;
        task_scheduler.updateCurrentTaskDelay(15_min);
    } else {
        task_scheduler.updateCurrentTaskDelay(0_us);
    }

    // Determine if this topic should be announced based on its check type.
    const auto &info = mqtt_discovery_topic_infos[topic_idx];
    bool entity_enabled = false;
    int resolved_meter_index = -1;

    switch (info.check_type) {
        case MqttDiscoveryCheckType::Feature:
            entity_enabled = api.hasFeature(info.feature);
            break;

        case MqttDiscoveryCheckType::ApiBool: {
            const Config *cfg = api.getState(info.api_check_path, false);
            if (cfg == nullptr)
                break;

            if (info.api_check_key == nullptr) {
                // if we just want to check if the path exists -> assume its active
                entity_enabled = true;
            } else {
                // if we check the path and a key, get the keys value
                entity_enabled = cfg->get(info.api_check_key)->asBool();
            }
            break;
        }

        case MqttDiscoveryCheckType::MeterValue: {
            // TODO Use meters.get_path instead of codifying API assumptions here.

            const Config *cfg = api.getState(info.api_check_path, false);
            if (cfg == nullptr || !cfg->is<Config::ConfUnion>() || cfg->getTag<MeterClassID>() == MeterClassID::None)
                break;

            // Meter is enabled. Now find the value_id index in the value_ids array.
            String value_ids_path = info.api_check_path;
            int last_slash = value_ids_path.lastIndexOf('/');
            if (last_slash >= 0) {
                value_ids_path = value_ids_path.substring(0, static_cast<size_t>(last_slash + 1)) + "value_ids";
            }
            const Config *value_ids_cfg = api.getState(value_ids_path.c_str(), false);
            if (value_ids_cfg == nullptr)
                break;

            size_t count = value_ids_cfg->count();
            for (size_t idx = 0; idx < count; idx++) {
                if (value_ids_cfg->get(idx)->asUint() == static_cast<uint32_t>(info.meter_value_id)) {
                    entity_enabled = true;
                    resolved_meter_index = static_cast<int>(idx);
                    break;
                }
            }
            break;
        }
        default:
            esp_system_abortf<96>("Unknown MqttDiscoveryCheckType %d", static_cast<int>(info.check_type));
    }

    CoolString topic;
    size_t topic_len = this->mqtt_discovery_topic_lengths[topic_idx];
    topic.reserve(topic_len + 1);
    this->get_discovery_topic(topic_idx, topic.begin(), topic_len + 1);
    topic.setLength(topic_len);

    if (!entity_enabled) {
        if (topic.length() > 0) {
            // Entity is not enabled; send empty payload to remove it from HA.
            mqtt.publish(topic, String(), true);
        }
        return;
    }

    size_t mode_idx = static_cast<size_t>(this->mode) - 1;

    // Pick language-specific static_info if available, otherwise fall back to the default (German).
    const char *static_info = info.static_infos[mode_idx];
    if (default_language == Language::English) {
        const char *en = info.static_infos_en[mode_idx];
        if (en != nullptr) {
            static_info = en;
        }
    }

    if (!static_info) {
        // No static info? Skip topic.
        return;
    }

    const String &client_name = mqtt.client_name;
    const String &topic_prefix = mqtt.global_topic_prefix;

    String name = "";
    if (info.check_type == MqttDiscoveryCheckType::MeterValue) {
        String meter_name = api.getState(info.availability[0].topic)->get(1)->get("display_name")->asString();
        if (default_language == Language::English) {
            name = "Meter " + String(meter_name) + " " + String(info.name_en);
        }else {
            name = "Zähler " + String(meter_name) + " " + String(info.name_de);
        }
    } else {
        name = default_language == Language::English ? info.name_en : info.name_de;
    }
    // MQTT_DISCOVERY_MAX_JSON_LENGTH: max length generated by prepare.py
    // 265: String literals
    // 7*64: topic_prefix (four times) and client name (thrice)
    // 13: component (max length is "binary_sensor")
    // 250: device_info
    constexpr size_t json_doc_size = MQTT_DISCOVERY_MAX_JSON_LENGTH + 265 + 7 * 64 + 13 + 250;

    // TODO: can we afford a 2k stack buffer here?

    char *buf = static_cast<char *>(malloc(json_doc_size));
    memset(buf, 0, json_doc_size);
    TFJsonSerializer json(buf, json_doc_size);

    json.addObject();

    json.addMemberString("name", name.c_str());

    json.addMemberStringF("unique_id", "%s-%s", client_name.c_str(), info.object_id);
    json.addMemberStringF("default_entity_id", "%s.%s-%s", info.component, client_name.c_str(), info.object_id);
    json.addMemberStringF("object_id", "%s-%s", client_name.c_str(), info.object_id);

    switch (info.type) {
        case MqttDiscoveryType::StateAndUpdate:
            json.addMemberStringF("command_topic", "%s/%s_update", topic_prefix.c_str(), info.path);
        [[fallthrough]];
        case MqttDiscoveryType::StateOnly:
            json.addMemberStringF("state_topic", "%s/%s", topic_prefix.c_str(), info.path);
            break;
        case MqttDiscoveryType::CommandOnly:
            json.addMemberStringF("command_topic", "%s/%s", topic_prefix.c_str(), info.path);
            break;
        default:
            esp_system_abortf<96>("Unknown MqttDiscoveryType %d", static_cast<int>(info.type));
    }

    if (info.availability_count > 0) {
        json.addMemberArray("availability");
        for (uint8_t i = 0; i < info.availability_count; i++) {
            const auto &entry = info.availability[i];
            json.addObject();
            json.addMemberStringF("topic", "%s/%s", topic_prefix.c_str(), entry.topic);
            json.addMemberString("value_template", entry.value_template);
            json.endObject();
        }
        json.endArray();
        json.addMemberString("availability_mode", "all");
    }


    if (strlen(info.json_attributes_topic) > 0) {
        json.addMemberStringF("json_attributes_topic", "%s/%s", topic_prefix.c_str(), info.json_attributes_topic);
        json_write_raw(json, info.json_attributes_info, strlen(info.json_attributes_info));
    }

    // Inject pre-formatted static_info as raw JSON object members
    json_write_raw(json, static_info, strlen(static_info));

    // For MeterValue entities, inject dynamically-resolved value_template
    if (info.check_type == MqttDiscoveryCheckType::MeterValue && resolved_meter_index >= 0) {
        assert(info.value_fractional_digits >= 0);
        json.addMemberStringF("value_template", "{{value_json[%d] | round(%d)}}", resolved_meter_index, info.value_fractional_digits);
    }

    json.addMemberObject("device");
    json.addMemberString("identifiers", mqtt.client_name.c_str());
    json.addMemberString("manufacturer", OPTIONS_MANUFACTURER_FULL());
    json.addMemberString("model", OPTIONS_PRODUCT_NAME());
    json.addMemberStringF("name", "%s (%s)", OPTIONS_PRODUCT_NAME(), mqtt.client_name.c_str());
    json.endObject();

    json.endObject();

    String json_str = buf;
    json_str.trim();

    mqtt.publish(topic, json_str, true);
    free(buf);
}
