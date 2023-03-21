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

#include "mqtt.h"

#include <Arduino.h>

#include "task_scheduler.h"
#include "tools.h"
#include "api.h"
#include "event_log.h"
#include "build.h"
#include "modules.h"

extern char local_uid_str[32];

#if MODULE_ESP32_ETHERNET_BRICK_AVAILABLE()
#define MQTT_RECV_BUFFER_SIZE 4096
#else
#define MQTT_RECV_BUFFER_SIZE 2048
#endif

#define MQTT_RECV_BUFFER_HEADROOM (MQTT_RECV_BUFFER_SIZE / 4)

void Mqtt::pre_setup()
{
    // The real UID will be patched in later
    mqtt_config = ConfigRoot(Config::Object({
        {"enable_mqtt", Config::Bool(false)},
        {"broker_host", Config::Str("", 0, 128)},
        {"broker_port", Config::Uint16(1883)},
        {"broker_username", Config::Str("", 0, 64)},
        {"broker_password", Config::Str("", 0, 64)},
        {"global_topic_prefix", Config::Str(String(BUILD_HOST_PREFIX) + String("/") + String("ABC"), 0, 64)},
        {"client_name", Config::Str(String(BUILD_HOST_PREFIX) + String("-") + String("ABC"), 1, 64)},
        {"interval", Config::Uint32(1)}
    }), [](Config &cfg) -> String {
#if MODULE_MQTT_AUTO_DISCOVERY_AVAILABLE()
        const String &global_topic_prefix = cfg.get("global_topic_prefix")->asString();
        const String &auto_discovery_prefix = mqtt_auto_discovery.config.get("auto_discovery_prefix")->asString();

        if (global_topic_prefix == auto_discovery_prefix)
            return "Global topic prefix cannot be the same as the MQTT auto discovery topic prefix.";
#endif
        return "";
    });

    mqtt_state = Config::Object({
        {"connection_state", Config::Int(0)},
        {"last_error", Config::Int(0)}
    });
}

void Mqtt::subscribe_with_prefix(const String &path, std::function<void(char *, size_t)> callback, bool forbid_retained)
{
    const String &prefix = mqtt_config_in_use.get("global_topic_prefix")->asString();
    String topic = prefix + "/" + path;
    subscribe(topic, callback, forbid_retained);
}

void Mqtt::subscribe(const String &topic, std::function<void(char *, size_t)> callback, bool forbid_retained)
{
    this->commands.push_back({topic, callback, forbid_retained});

    esp_mqtt_client_unsubscribe(client, topic.c_str());
    esp_mqtt_client_subscribe(client, topic.c_str(), 0);
}

void Mqtt::addCommand(size_t commandIdx, const CommandRegistration &reg)
{
    auto req_size = reg.config->max_string_length();
    if (req_size > MQTT_RECV_BUFFER_SIZE) {
        logger.printfln("MQTT: Recv buf is %u bytes. %s requires %u. Bump MQTT_RECV_BUFFER_SIZE! Not subscribing!", MQTT_RECV_BUFFER_SIZE, reg.path.c_str(), req_size);
        return;
    }
    if (req_size > (MQTT_RECV_BUFFER_SIZE - MQTT_RECV_BUFFER_HEADROOM))
        logger.printfln("MQTT: Recv buf is %u bytes. %s requires %u. Maybe bump MQTT_RECV_BUFFER_SIZE?", MQTT_RECV_BUFFER_SIZE, reg.path.c_str(), req_size);
}

void Mqtt::addState(size_t stateIdx, const StateRegistration &reg)
{
    this->states.push_back({reg.path, 0});
}

void Mqtt::addRawCommand(size_t rawCommandIdx, const RawCommandRegistration &reg)
{

}

void Mqtt::addResponse(size_t responseIdx, const ResponseRegistration &reg)
{
}

void Mqtt::publish_with_prefix(const String &path, const String &payload)
{
    const String &prefix = mqtt_config_in_use.get("global_topic_prefix")->asString();
    String topic = prefix + "/" + path;
    // Retain messages because we only send on change.
    publish(topic, payload, true);
}

void Mqtt::publish(const String &topic, const String &payload, bool retain)
{
    esp_mqtt_client_publish(this->client, topic.c_str(), payload.c_str(), payload.length(), 0, retain);
}

bool Mqtt::pushStateUpdate(size_t stateIdx, const String &payload, const String &path)
{
    auto &state = this->states[stateIdx];

    if (!deadline_elapsed(state.last_send_ms + mqtt_config_in_use.get("interval")->asUint() * 1000))
        return false;

    this->publish_with_prefix(path, payload);
    state.last_send_ms = millis();
    return true;
}

void Mqtt::pushRawStateUpdate(const String &payload, const String &path)
{
    this->publish_with_prefix(path, payload);
}

void Mqtt::wifiAvailable()
{
    static bool started = false;
    if (!started)
        esp_mqtt_client_start(client);
    started = true;
}

void Mqtt::onMqttConnect()
{
    logger.printfln("MQTT: Connected to broker.");
    this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::CONNECTED);

    this->commands.clear();
    for (size_t i = 0; i < api.commands.size(); ++i) {
        auto &reg = api.commands[i];
        this->addCommand(i, reg);
    }
    for (size_t i = 0; i < api.raw_commands.size(); ++i) {
        auto &reg = api.raw_commands[i];
        this->addRawCommand(i, reg);
    }
    for (auto &reg : api.states) {
        publish_with_prefix(reg.path, reg.config->to_string_except(reg.keys_to_censor));
    }

#if MODULE_MQTT_METER_AVAILABLE()
    mqtt_meter.onMqttConnect();
#endif
#if MODULE_MQTT_AUTO_DISCOVERY_AVAILABLE()
    mqtt_auto_discovery.onMqttConnect();
#endif

    const String &prefix = mqtt_config_in_use.get("global_topic_prefix")->asString();
    String topic = prefix + "/#";
    esp_mqtt_client_unsubscribe(client, topic.c_str());
    esp_mqtt_client_subscribe(client, topic.c_str(), 0);
}

void Mqtt::onMqttDisconnect()
{
    this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::NOT_CONNECTED);
    logger.printfln("MQTT: Disconnected from broker.");
}

void Mqtt::onMqttMessage(char *topic, size_t topic_len, char *data, size_t data_len, bool retain)
{
#if MODULE_MQTT_METER_AVAILABLE()
    if (mqtt_meter.onMqttMessage(topic, topic_len, data, data_len, retain))
        return;
#endif
#if MODULE_MQTT_AUTO_DISCOVERY_AVAILABLE()
    if (mqtt_auto_discovery.onMqttMessage(topic, topic_len, data, data_len, retain))
        return;
#endif

    for (auto &c : commands) {
        if (c.topic.length() != topic_len)
            continue;
        if (memcmp(c.topic.c_str(), topic, topic_len) != 0)
            continue;

        if (retain && c.forbid_retained) {
            logger.printfln("MQTT: Topic %s is an action. Ignoring retained message.", c.topic.c_str());
            return;
        }

        c.callback(data, data_len);
        return;
    }

    const String &prefix = this->mqtt_config_in_use.get("global_topic_prefix")->asString();
    if (topic_len < prefix.length() + 1) // + 1 because we will check for the / between the prefix and the topic.
        return;
    if (memcmp(topic, prefix.c_str(), prefix.length()) != 0)
        return;
    if (topic[prefix.length()] != '/')
        return;

    topic += prefix.length() + 1;
    topic_len -= prefix.length() + 1;


    for (auto &reg : api.commands) {
        if (topic_len != reg.path.length() || memcmp(topic, reg.path.c_str(), topic_len) != 0)
            continue;

        if (retain && reg.is_action) {
            logger.printfln("MQTT: Topic %s is an action. Ignoring retained message.", reg.path.c_str());
            return;
        }

        String error = reg.config->update_from_cstr(data, data_len);
        if(error == "") {
            task_scheduler.scheduleOnce([reg](){reg.callback();}, 0);
            return;
        }

        logger.printfln("MQTT: Failed to update %s from MQTT payload: %s", reg.path.c_str(), error.c_str());
        return;
    }

    for (auto &reg : api.raw_commands) {
        if (topic_len != reg.path.length() || memcmp(topic, reg.path.c_str(), topic_len) != 0)
            continue;

        if (retain && reg.is_action) {
            logger.printfln("MQTT: Topic %s is an action. Ignoring retained message.", reg.path.c_str());
            return;
        }

        String error = reg.callback(data, data_len);
        if(error == "") {
            return;
        }

        logger.printfln("MQTT: Failed to update %s from MQTT payload: %s", reg.path.c_str(), error.c_str());
        return;
    }

    // Don't print error message on state topics, this could be one of our own messages.
    for (auto &reg : api.states) {
        if (topic_len != reg.path.length() || memcmp(topic, reg.path.c_str(), topic_len) != 0)
            continue;
        return;
    }

    // Don't print error message if this packet was received because it was retained (as opposed to a newly published message)
    // The spec says:
    // It MUST set the RETAIN flag to 0 when a PUBLISH Packet is sent to a Client
    // because it matches an established subscription regardless of how the flag was set in the message it received [MQTT-3.3.1-9].
    if (!retain)
        logger.printfln("MQTT: Received message on unknown topic '%.*s'. data_len=%i", topic_len, topic, data_len);
}

static char err_buf[64] = {0};

static const char *get_mqtt_error(esp_mqtt_connect_return_code_t rc)
{
    switch (rc) {
        case MQTT_CONNECTION_ACCEPTED:
            return "Connection accepted";
        case MQTT_CONNECTION_REFUSE_PROTOCOL:
            return "Wrong protocol";

        case MQTT_CONNECTION_REFUSE_ID_REJECTED:
            return "ID rejected";

        case MQTT_CONNECTION_REFUSE_SERVER_UNAVAILABLE:
            return "Server unavailable";

        case MQTT_CONNECTION_REFUSE_BAD_USERNAME:
            return "Wrong user";

        case MQTT_CONNECTION_REFUSE_NOT_AUTHORIZED:
            return "Wrong username or password";
    }
    return "Unknown error";
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    Mqtt *mqtt = (Mqtt *)handler_args;
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            task_scheduler.scheduleOnce([mqtt](){
                mqtt->onMqttConnect();
            }, 0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            task_scheduler.scheduleOnce([mqtt](){
                mqtt->onMqttDisconnect();
            }, 0);
            break;
        case MQTT_EVENT_DATA:
            if (event->current_data_offset != 0)
                return;
            if (event->total_data_len != event->data_len) {
                logger.printfln("MQTT: Ignoring message with payload length %d for topic %.*s. Maximum length allowed is %u.", event->total_data_len, event->topic_len, event->topic, MQTT_RECV_BUFFER_SIZE);
                return;
            }
            mqtt->onMqttMessage(event->topic, event->topic_len, event->data, event->data_len, event->retain);
            break;
        case MQTT_EVENT_ERROR: {
                auto eh = event->error_handle;

                if (eh->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                    if (eh->esp_tls_last_esp_err != ESP_OK) {
                        const char *e = esp_err_to_name_r(eh->esp_tls_last_esp_err, err_buf, sizeof(err_buf) / sizeof(err_buf[0]));
                        logger.printfln("MQTT: Transport error: %s (esp_tls_last_esp_err)", e);
                        mqtt->mqtt_state.get("last_error")->updateInt(eh->esp_tls_last_esp_err);
                    }
                    if (eh->esp_tls_stack_err != 0) {
                        const char *e = esp_err_to_name_r(eh->esp_tls_stack_err, err_buf, sizeof(err_buf) / sizeof(err_buf[0]));
                        logger.printfln("MQTT: Transport error: %s (esp_tls_stack_err)", e);
                        mqtt->mqtt_state.get("last_error")->updateInt(eh->esp_tls_stack_err);
                    }
                    if (eh->esp_transport_sock_errno != 0) {
                        const char *e = strerror(eh->esp_transport_sock_errno);
                        logger.printfln("MQTT: Transport error: %s", e);
                        mqtt->mqtt_state.get("last_error")->updateInt(eh->esp_transport_sock_errno);
                    }
                } else if (eh->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                    logger.printfln("MQTT: Connection refused: %s", get_mqtt_error(eh->connect_return_code));
                    // Minus to indicate this is a connection error
                    mqtt->mqtt_state.get("last_error")->updateInt(-eh->connect_return_code);
                } else {
                    logger.printfln("MQTT: Unknown error");
                    mqtt->mqtt_state.get("last_error")->updateInt(0xFFFFFFFF);
                }
                break;
            }
        default:
            break;
    }
}

void Mqtt::setup()
{
    if (!api.restorePersistentConfig("mqtt/config", &mqtt_config)) {
        mqtt_config.get("global_topic_prefix")->updateString(String(BUILD_HOST_PREFIX) + String("/") + String(local_uid_str));
        mqtt_config.get("client_name")->updateString(String(BUILD_HOST_PREFIX) + String("-") + String(local_uid_str));

#ifdef DEFAULT_MQTT_ENABLE
        mqtt_config.get("enable_mqtt")->updateBool(DEFAULT_MQTT_ENABLE);
#endif
#ifdef DEFAULT_MQTT_BROKER_HOST
        mqtt_config.get("broker_host")->updateString(DEFAULT_MQTT_BROKER_HOST);
#endif
#ifdef DEFAULT_MQTT_BROKER_PORT
        mqtt_config.get("broker_port")->updateUint(DEFAULT_MQTT_BROKER_PORT);
#endif
#ifdef DEFAULT_MQTT_BROKER_USERNAME
        mqtt_config.get("broker_username")->updateString(DEFAULT_MQTT_BROKER_USERNAME);
#endif
#ifdef DEFAULT_MQTT_BROKER_PASSWORD
        mqtt_config.get("broker_password")->updateString(DEFAULT_MQTT_BROKER_PASSWORD);
#endif
    }

    mqtt_config_in_use = mqtt_config;

    if (!mqtt_config.get("enable_mqtt")->asBool()) {
        initialized = true;
        return;
    }

    api.registerBackend(this);

    esp_log_level_set("MQTT_CLIENT", ESP_LOG_NONE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_NONE);
    esp_log_level_set("esp-tls", ESP_LOG_NONE);
    esp_log_level_set("TRANSPORT", ESP_LOG_NONE);
    esp_log_level_set("OUTBOX", ESP_LOG_NONE);



    esp_mqtt_client_config_t mqtt_cfg = {};

    mqtt_cfg.host = mqtt_config_in_use.get("broker_host")->asEphemeralCStr();
    mqtt_cfg.port = mqtt_config_in_use.get("broker_port")->asUint();
    mqtt_cfg.client_id = mqtt_config_in_use.get("client_name")->asEphemeralCStr();
    mqtt_cfg.username = mqtt_config_in_use.get("broker_username")->asEphemeralCStr();
    mqtt_cfg.password = mqtt_config_in_use.get("broker_password")->asEphemeralCStr();
    mqtt_cfg.buffer_size = MQTT_RECV_BUFFER_SIZE;
    mqtt_cfg.network_timeout_ms = 100;

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, this);


    initialized = true;
}

void Mqtt::register_urls()
{
    api.addPersistentConfig("mqtt/config", &mqtt_config, {"broker_password"}, 1000);
    api.addState("mqtt/state", &mqtt_state, {}, 1000);
}

void Mqtt::loop()
{
}
