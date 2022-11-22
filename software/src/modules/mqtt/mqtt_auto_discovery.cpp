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

extern API api;
extern TaskScheduler task_scheduler;
extern char local_uid_str[32];

#define TOPIC_COUNT (sizeof(mqtt_discovery_topic_infos)/sizeof(mqtt_discovery_topic_infos[0]))

enum class MqttDiscoveryType {
    STATE_ONLY,
    STATE_AND_UPDATE,
    COMMAND_ONLY
};

struct DiscoveryTopicInfo {
    const char * const feature;
    const char * const path;
    const char * const component;
    const char * const object_id;
    const char * const name_de;
    const char * const name_en;
    const char * const static_info;
    const MqttDiscoveryType type;
};

struct DiscoveryTopic {
    String full_path;
};

static const struct DiscoveryTopicInfo mqtt_discovery_topic_infos[] = {
    {
        .feature = "evse",
        .path = "evse/state",
        .component = "binary_sensor",
        .object_id = "cable",
        .name_de = "Wallbox Ladekabel verbunden",
        .name_en = "Wallbox cable connected",
        .static_info = "\"value_template\":\"{{value_json.charger_state in [1, 2, 3]}}\"," "\"payload_on\":\"True\","   "\"payload_off\":\"False\","    "\"device_class\":\"plug\"",
        .type = MqttDiscoveryType::STATE_ONLY,
    },
    {
        .feature = "evse",
        .path = "evse/state",
        .component = "binary_sensor",
        .object_id = "ready",
        .name_de = "Wallbox ladebereit",
        .name_en = "Wallbox ready to charge",
        .static_info = "\"value_template\":\"{{value_json.charger_state in [2, 3]}}\","     "\"payload_on\":\"True\","  "\"payload_off\":\"False\","    "\"device_class\":\"power\"",
        .type = MqttDiscoveryType::STATE_ONLY,
    },
    {
        .feature = "evse",
        .path = "evse/state",
        .component = "binary_sensor",
        .object_id = "charging",
        .name_de = "Wallbox Ladevorgang",
        .name_en = "Wallbox charging",
        .static_info = "\"value_template\":\"{{value_json.charger_state in [3]}}\","        "\"payload_on\":\"True\","  "\"payload_off\":\"False\","    "\"device_class\":\"battery_charging\"",
        .type = MqttDiscoveryType::STATE_ONLY,
    },
    {
        .feature = "evse",
        .path = "evse/state",
        .component = "binary_sensor",
        .object_id = "error",
        .name_de = "Wallbox Fehler",
        .name_en = "Wallbox error",
        .static_info = "\"value_template\":\"{{value_json.charger_state in [4]}}\","		"\"payload_on\":\"True\","	"\"payload_off\":\"False\","    "\"device_class\":\"problem\"",
        .type = MqttDiscoveryType::STATE_ONLY,
    },
    {
        .feature = "evse",
        .path = "evse/low_level_state",
        .component = "binary_sensor",
        .object_id = "online",
        .name_de = "Wallbox verfügbar",
        .name_en = "Wallbox online",
        .static_info = "\"value_template\":\"{{value_json.uptime>0}}\","			        "\"payload_on\":\"True\","	"\"payload_off\":\"False\","	"\"device_class\":\"connectivity\","     "\"expire_after\":\"30\"",
        .type = MqttDiscoveryType::STATE_ONLY,
    },
/*    {
        .feature = "evse",
        .path = "evse/low_level_state",
        .component = "sensor",
        .object_id = "p12vrail",
        .name_de = "+12V Rail",
        .name_en = "+12V rail",
        .static_info = "\"value_template\":\"{{value_json.voltages[5]}}\","                 "\"unit_of_measurement\":\"mV\","   "\"entity_category\":\"diagnostic\","   "\"icon\":\"mdi:speedometer\"",
        .type = MqttDiscoveryType::STATE_ONLY,
    },
*/    {
        .feature = "meter",
        .path = "meter/values",
        .component = "sensor",
        .object_id = "powernow",
        .name_de = "Wallbox aktuelle Ladeleistung",
        .name_en = "Wallbox current charge power",
        .static_info = "\"value_template\":\"{{value_json.power}}\","                       "\"unit_of_measurement\":\"W\","    "\"device_class\":\"power\","           "\"state_class\":\"measurement\"",
        .type = MqttDiscoveryType::STATE_ONLY,
    },
    {
        .feature = "meter",
        .path = "meter/values",
        .component = "sensor",
        .object_id = "energyabs",
        .name_de = "Wallbox Zählerstand absolut",
        .name_en = "Wallbox meter value (absolute)",
        .static_info = "\"value_template\":\"{{value_json.energy_abs}}\","                  "\"unit_of_measurement\":\"kWh\","  "\"device_class\":\"energy\","          "\"state_class\":\"total\"",
        .type = MqttDiscoveryType::STATE_ONLY,
    },
    {
        .feature = "meter",
        .path = "meter/values",
        .component = "sensor",
        .object_id = "energyrel",
        .name_de = "Wallbox Zählerstand relativ",
        .name_en = "Wallbox meter value (relative)",
        .static_info = "\"value_template\":\"{{value_json.energy_rel}}\","                  "\"unit_of_measurement\":\"kWh\","  "\"device_class\":\"energy\","          "\"state_class\":\"total\"",
        .type = MqttDiscoveryType::STATE_ONLY,
    },
    {
        .feature = "evse",
        .path = "evse/auto_start_charging",
        .component = "switch",
        .object_id = "autocharge",
        .name_de = "Wallbox automatisches Laden",
        .name_en = "Wallbox automatic charging",
        .static_info = "\"value_template\":\"{{value_json.auto_start_charging}}\","     "\"state_on\":\"True\","    "\"state_off\":\"False\","  "\"payload_on\":\"{\\\"auto_start_charging\\\":true}\","    "\"payload_off\":\"{\\\"auto_start_charging\\\":false}\","  "\"icon\":\"mdi:ev-plug-type2\"",
        .type = MqttDiscoveryType::STATE_AND_UPDATE,
    },
    /* Ladestrom? */
    {
        .feature = "evse",
        .path = "evse/start_charging",
        .component = "button",
        .object_id = "startcharge",
        .name_de = "Wallbox Ladevorgang freigeben",
        .name_en = "Wallbox start charging",
        .static_info = "\"payload_press\":\"null\","    "\"icon\":\"mdi:flash\"",
        .type = MqttDiscoveryType::COMMAND_ONLY,
    },
    {
        .feature = "evse",
        .path = "evse/stop_charging",
        .component = "button",
        .object_id = "stopcharge",
        .name_de = "Wallbox Ladevorgang beenden",
        .name_en = "Wallbox stop charging",
        .static_info = "\"payload_press\":\"null\","    "\"icon\":\"mdi:flash-off\"",
        .type = MqttDiscoveryType::COMMAND_ONLY,
    }
};

static struct DiscoveryTopic mqtt_discovery_topics[TOPIC_COUNT];
static String device_info;

MqttAutoDiscovery mqtt_auto_discovery;

void MqttAutoDiscovery::prepare_topics(const ConfigRoot &mqtt_config_in_use)
{
    const String auto_discovery_prefix = mqtt_config_in_use.get("auto_discovery_prefix")->asString();
    const String client_name = mqtt_config_in_use.get("client_name")->asString();
    unsigned int topic_length;

    for (size_t i = 0; i < TOPIC_COUNT; ++i) {
        // <discovery_prefix>/<component>/<node_id>/<object_id>/config
        topic_length = auto_discovery_prefix.length() + strlen(mqtt_discovery_topic_infos[i].component)
            + client_name.length() + strlen(mqtt_discovery_topic_infos[i].object_id) + 10; // "config" + 4*'/' = 10

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

    device_info.reserve(127);

    device_info.concat("\"device\":{\"identifiers\":\"");
    device_info.concat(client_name);
    device_info.concat("\",\"manufacturer\":\"Tinkerforge GmbH\",\"model\":\"" BUILD_DISPLAY_NAME "\",\"name\":\"" BUILD_DISPLAY_NAME " (");
    device_info.concat(client_name);
    device_info.concat(")\"}");
}

void MqttAutoDiscovery::subscribe_to_own()
{
    // <discovery_prefix>/+/<node_id>/+/config
    String topic;
    topic.reserve(47);

    topic.concat(mqtt.mqtt_config_in_use.get("auto_discovery_prefix")->asString());
    topic.concat("/+/");
    topic.concat(mqtt.mqtt_config_in_use.get("client_name")->asString());
    topic.concat("/+/config");

    esp_mqtt_client_subscribe(mqtt.client, topic.c_str(), 0);
}

void MqttAutoDiscovery::check_discovery_topic(const char *topic, size_t topic_len, size_t data_len)
{
    for (size_t i = 0; i < TOPIC_COUNT; ++i) {
        if (mqtt_discovery_topics[i].full_path.length() != topic_len)
            continue;

        if (memcmp(mqtt_discovery_topics[i].full_path.c_str(), topic, topic_len) == 0) {
            // Discovery topic is known; nothing to do.
            return;
        }
    }

    // topic is not null-terminated and needs to be copied to terminate properly.
    String tp;
    tp.concat(topic, topic_len);

    // Unknown discovery topic with zero-length data probably caused by us removing it. Catch it to avoid an infinite loop.
    if (data_len ==  0) {
        logger.printfln("MQTT auto discovery: Topic '%s' was removed.", tp.c_str());
        return;
    }

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
    }
    // deal with one topic
    else if (api.hasFeature(mqtt_discovery_topic_infos[topic_num].feature)) {
        const String topic_prefix = mqtt.mqtt_config_in_use.get("global_topic_prefix")->asString();
        const char *name = mqtt_discovery_topic_infos[topic_num].name_de;

        String payload;
        payload.reserve(639);

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
        payload.concat(mqtt_discovery_topic_infos[topic_num].static_info);
        payload.concat(',');
        payload.concat(device_info);
        payload.concat('}');

        mqtt.publish(mqtt_discovery_topics[topic_num].full_path, payload, true);

        if (++topic_num >= TOPIC_COUNT) {
            topic_num = 0;
            delay_ms = 15 * 60 * 1000;
        }
    }

    task_scheduler.scheduleOnce([this, topic_num](){
        this->announce_next_topic(topic_num);
    }, delay_ms);
}

void MqttAutoDiscovery::start_announcing()
{
    task_scheduler.scheduleOnce([this](){
        this->announce_next_topic(0);
    }, 1000);
}
