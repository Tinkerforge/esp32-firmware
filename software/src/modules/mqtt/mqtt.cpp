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
#include "module_dependencies.h"

#include <Arduino.h>

#include "task_scheduler.h"
#include "tools.h"
#include "api.h"
#include "event_log.h"
#include "build.h"

#include "matchTopicFilter.h"

extern Mqtt mqtt;
extern char local_uid_str[32];

#if MODULE_ESP32_ETHERNET_BRICK_AVAILABLE()
#define MQTT_RECV_BUFFER_SIZE 4096U
#else
#define MQTT_RECV_BUFFER_SIZE 2048U
#endif

#define MQTT_RECV_BUFFER_HEADROOM (MQTT_RECV_BUFFER_SIZE / 4)

void Mqtt::pre_setup()
{
    // The real UID will be patched in later
    config = ConfigRoot(Config::Object({
        {"enable_mqtt", Config::Bool(false)},
        {"broker_host", Config::Str("", 0, 128)},
        {"broker_port", Config::Uint16(1883)},
        {"broker_username", Config::Str("", 0, 64)},
        {"broker_password", Config::Str("", 0, 64)},
        {"global_topic_prefix", Config::Str(String(BUILD_HOST_PREFIX) + "/" + "ABC", 0, 64)},
        {"client_name", Config::Str(String(BUILD_HOST_PREFIX) + "-" + "ABC", 1, 64)},
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

    state = Config::Object({
        {"connection_state", Config::Int(0)},
        {"connection_start", Config::Uint(0)},
        {"connection_end", Config::Uint32(0)},
        {"last_error", Config::Int(0)}
    });

#if MODULE_CRON_AVAILABLE()
    cron.register_trigger(
        CronTriggerID::MQTT,
        Config::Object({
            {"topic", Config::Str("", 0, 64)},
            {"payload", Config::Str("", 0, 64)},
            {"retain", Config::Bool(false)}
        })
    );

    cron.register_action(
        CronActionID::MQTT,
        Config::Object({
            {"topic", Config::Str("", 0, 64)},
            {"payload", Config::Str("", 0, 64)},
            {"retain", Config::Bool(false)}
        }),
        [this](const Config *cfg) {
            publish(cfg->get("topic")->asString(), cfg->get("payload")->asString(), cfg->get("retain")->asBool());
        }
    );
#endif
}

void Mqtt::subscribe_with_prefix(const String &path, std::function<void(const char *, size_t, char *, size_t)> callback, bool forbid_retained)
{
    String topic = prefix + "/" + path;
    subscribe(topic, callback, forbid_retained);
}

void Mqtt::subscribe(const String &topic, std::function<void(const char *, size_t, char *, size_t)> callback, bool forbid_retained)
{
    bool subscribed = esp_mqtt_client_subscribe(client, topic.c_str(), 0) >= 0;

    this->commands.push_back({topic, callback, forbid_retained, topic.startsWith(prefix), subscribed});
}

void Mqtt::addCommand(size_t commandIdx, const CommandRegistration &reg)
{
    auto req_size = reg.config->max_string_length();
    if (req_size > MQTT_RECV_BUFFER_SIZE) {
        logger.printfln("MQTT: Recv buf is %u bytes. %s requires %u. Bump MQTT_RECV_BUFFER_SIZE! Updates on this topic might break the MQTT connection!", MQTT_RECV_BUFFER_SIZE, reg.path.c_str(), req_size);
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

bool Mqtt::publish_with_prefix(const String &path, const String &payload, bool retain)
{
    String topic = prefix + "/" + path;
    return publish(topic, payload, retain);
}

bool Mqtt::publish(const String &topic, const String &payload, bool retain)
{
    // ESP-MQTT does this check but we only want to allow publishing after
    // onMqttConnect was called (in the main thread!)
    // ESP-MQTT's check can asynchronously flip to connected.
    if (this->state.get("connection_state")->asInt() != (int)MqttConnectionState::CONNECTED)
        return false;

    return esp_mqtt_client_publish(this->client, topic.c_str(), payload.c_str(), payload.length(), 0, retain) >= 0;
}

bool Mqtt::pushStateUpdate(size_t stateIdx, const String &payload, const String &path)
{
    auto &state = this->states[stateIdx];

    if (!deadline_elapsed(state.last_send_ms + config_in_use.get("interval")->asUint() * 1000))
        return false;

    bool success = this->publish_with_prefix(path, payload);

    if (success) {
        state.last_send_ms = millis();
    }

    return success;
}

bool Mqtt::pushRawStateUpdate(const String &payload, const String &path)
{
    return this->publish_with_prefix(path, payload);
}

void Mqtt::resubscribe() {
    if (this->state.get("connection_state")->asInt() != (int)MqttConnectionState::CONNECTED)
        return;

    if (!global_topic_prefix_subscribed) {
        String topic = prefix + "/#";
        global_topic_prefix_subscribed = esp_mqtt_client_subscribe(client, topic.c_str(), 0) >= 0;
    }

    for (auto &cmd : this->commands) {
        if (cmd.starts_with_global_topic_prefix)
            continue;

        if (cmd.subscribed)
            continue;

        cmd.subscribed = esp_mqtt_client_subscribe(client, cmd.topic.c_str(), 0) >= 0;
    }
}

void Mqtt::onMqttConnect()
{
    last_connected_ms = millis();
    state.get("connection_start")->updateUint(last_connected_ms);
    was_connected = true;
    logger.printfln("MQTT: Connected to broker.");
    this->state.get("connection_state")->updateInt((int)MqttConnectionState::CONNECTED);

    for (size_t i = 0; i < api.commands.size(); ++i) {
        auto &reg = api.commands[i];
        this->addCommand(i, reg);
    }
    for (size_t i = 0; i < api.raw_commands.size(); ++i) {
        auto &reg = api.raw_commands[i];
        this->addRawCommand(i, reg);
    }
    for (auto &reg : api.states) {
        reg.config->set_updated(1 << this->backend_idx);
    }

    this->global_topic_prefix_subscribed = false;
    for (auto &cmd : this->commands) {
        cmd.subscribed = false;
    }

    // Resubscribe now to prioritize re-subscription
    // of the first topics (for example the global topic prefix)
    // over sending state updates.
    this->resubscribe();
}

void Mqtt::onMqttDisconnect()
{
    if (this->state.get("connection_state")->asEnum<MqttConnectionState>() == MqttConnectionState::NOT_CONNECTED)
        logger.printfln("MQTT: Failed to connect to broker.");
    else
        logger.printfln("MQTT: Disconnected from broker.");

    this->state.get("connection_state")->updateInt((int)MqttConnectionState::NOT_CONNECTED);
    if (was_connected) {
        was_connected = false;
        uint32_t now = millis();
        uint32_t connected_for = now - last_connected_ms;
        state.get("connection_end")->updateUint(now);
        if (connected_for < 0x7FFFFFFF) {
            logger.printfln("MQTT: Was connected for %u seconds.", connected_for / 1000);
        } else {
            logger.printfln("MQTT: Was connected for a long time.");
        }
    }
}

#if MODULE_CRON_AVAILABLE()
static bool trigger_action(Config *cfg, void *data) {
    return mqtt.action_triggered(cfg, data);
}
#endif

void Mqtt::onMqttMessage(char *topic, size_t topic_len, char *data, size_t data_len, bool retain)
{
    for (auto &c : commands) {
        if (!matchTopicFilter(topic, topic_len, c.topic.c_str(), c.topic.length()))
            continue;

        if (retain && c.forbid_retained) {
            logger.printfln("MQTT: Retained messages on topic %s are forbidden. Ignoring retained message (data_len=%u).", c.topic.c_str(), data_len);
            return;
        }

        c.callback(topic, topic_len, data, data_len);
        return;
    }

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
            logger.printfln("MQTT: Topic %s is an action. Ignoring retained message (data_len=%u).", reg.path.c_str(), data_len);
            return;
        }

        esp_mqtt_client_disable_receive(client, 100);
        api.callCommandNonBlocking(reg, data, data_len, [this](String error){
            if (error != "")
                logger.printfln("MQTT: %s", error.c_str());
            esp_mqtt_client_enable_receive(this->client);
        });

        return;
    }

    for (auto &reg : api.raw_commands) {
        if (topic_len != reg.path.length() || memcmp(topic, reg.path.c_str(), topic_len) != 0)
            continue;

        if (retain && reg.is_action) {
            logger.printfln("MQTT: Topic %s is an action. Ignoring retained message (data_len=%u).", reg.path.c_str(), data_len);
            return;
        }

        esp_mqtt_client_disable_receive(client, 100);
        task_scheduler.scheduleOnce([this, reg, data, data_len](){
            String error = reg.callback(data, data_len);
            if (error != "")
                logger.printfln("MQTT: %s", error.c_str());

            esp_mqtt_client_enable_receive(this->client);
        }, 0);

        return;
    }

    // Don't print error message on state topics, this could be one of our own messages.
    for (auto &reg : api.states) {
        if (topic_len != reg.path.length() || memcmp(topic, reg.path.c_str(), topic_len) != 0)
            continue;
        return;
    }

#if MODULE_CRON_AVAILABLE()
    MqttMessage msg;
    msg.topic = String(topic).substring(0, topic_len);
    msg.payload = String(data).substring(0, data_len);
    msg.retained = retain;
    if (cron.trigger_action(CronTriggerID::MQTT, &msg, &trigger_action))
        return;
#endif

    // Don't print error message if this packet was received because it was retained (as opposed to a newly published message)
    // The spec says:
    // It MUST set the RETAIN flag to 0 when a PUBLISH Packet is sent to a Client
    // because it matches an established subscription regardless of how the flag was set in the message it received [MQTT-3.3.1-9].
    if (!retain)
        logger.printfln("MQTT: Received message on unknown topic '%.*s' (data_len=%u)", static_cast<int>(topic_len), topic, data_len);
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
                bool was_connected = mqtt->state.get("connection_state")->asEnum<MqttConnectionState>() != MqttConnectionState::NOT_CONNECTED;

                if (eh->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                    if (was_connected && eh->esp_tls_last_esp_err != ESP_OK) {
                        const char *e = esp_err_to_name_r(eh->esp_tls_last_esp_err, err_buf, sizeof(err_buf) / sizeof(err_buf[0]));
                        logger.printfln("MQTT: Transport error: %s (esp_tls_last_esp_err)", e);
                        mqtt->state.get("last_error")->updateInt(eh->esp_tls_last_esp_err);
                    }
                    if (was_connected && eh->esp_tls_stack_err != 0) {
                        const char *e = esp_err_to_name_r(eh->esp_tls_stack_err, err_buf, sizeof(err_buf) / sizeof(err_buf[0]));
                        logger.printfln("MQTT: Transport error: %s (esp_tls_stack_err)", e);
                        mqtt->state.get("last_error")->updateInt(eh->esp_tls_stack_err);
                    }
                    if (eh->esp_transport_sock_errno != 0) {
                        const char *e = strerror(eh->esp_transport_sock_errno);
                        logger.printfln("MQTT: Transport error: %s", e);
                        mqtt->state.get("last_error")->updateInt(eh->esp_transport_sock_errno);
                    }
                } else if (eh->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                    logger.printfln("MQTT: Connection refused: %s", get_mqtt_error(eh->connect_return_code));
                    // Minus to indicate this is a connection error
                    mqtt->state.get("last_error")->updateInt(-eh->connect_return_code);
                } else {
                    logger.printfln("MQTT: Unknown error");
                    mqtt->state.get("last_error")->updateInt(0xFFFFFFFF);
                }
                break;
            }
        default:
            break;
    }
}

void Mqtt::setup()
{
    if (!api.restorePersistentConfig("mqtt/config", &config)) {
        config.get("global_topic_prefix")->updateString(String(BUILD_HOST_PREFIX) + String("/") + String(local_uid_str));
        config.get("client_name")->updateString(String(BUILD_HOST_PREFIX) + String("-") + String(local_uid_str));

#ifdef DEFAULT_MQTT_ENABLE
        config.get("enable_mqtt")->updateBool(DEFAULT_MQTT_ENABLE);
#endif
#ifdef DEFAULT_MQTT_BROKER_HOST
        config.get("broker_host")->updateString(DEFAULT_MQTT_BROKER_HOST);
#endif
#ifdef DEFAULT_MQTT_BROKER_PORT
        config.get("broker_port")->updateUint(DEFAULT_MQTT_BROKER_PORT);
#endif
#ifdef DEFAULT_MQTT_BROKER_USERNAME
        config.get("broker_username")->updateString(DEFAULT_MQTT_BROKER_USERNAME);
#endif
#ifdef DEFAULT_MQTT_BROKER_PASSWORD
        config.get("broker_password")->updateString(DEFAULT_MQTT_BROKER_PASSWORD);
#endif
    }

    config_in_use = config;
    prefix = this->config_in_use.get("global_topic_prefix")->asString();

    if (!config.get("enable_mqtt")->asBool()) {
        initialized = true;
        return;
    }

    this->backend_idx = api.registerBackend(this);

    esp_log_level_set("MQTT_CLIENT", ESP_LOG_NONE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_NONE);
    esp_log_level_set("esp-tls", ESP_LOG_NONE);
    esp_log_level_set("TRANSPORT", ESP_LOG_NONE);
    esp_log_level_set("OUTBOX", ESP_LOG_NONE);

    esp_mqtt_client_config_t mqtt_cfg = {};

    mqtt_cfg.host = config_in_use.get("broker_host")->asEphemeralCStr();
    mqtt_cfg.port = config_in_use.get("broker_port")->asUint();
    mqtt_cfg.client_id = config_in_use.get("client_name")->asEphemeralCStr();
    mqtt_cfg.username = config_in_use.get("broker_username")->asEphemeralCStr();
    mqtt_cfg.password = config_in_use.get("broker_password")->asEphemeralCStr();
    mqtt_cfg.buffer_size = MQTT_RECV_BUFFER_SIZE;
    mqtt_cfg.network_timeout_ms = 1000;
    mqtt_cfg.message_retransmit_timeout = 400;

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, this);

    task_scheduler.scheduleWithFixedDelay([this](){
        this->resubscribe();
    }, 1000, 1000);

    initialized = true;
}

void Mqtt::register_urls()
{
    api.addPersistentConfig("mqtt/config", &config, {"broker_password"});
    api.addState("mqtt/state", &state);

#if MODULE_CRON_AVAILABLE()
    if (cron.is_trigger_active(CronTriggerID::MQTT)) {
        ConfigVec trigger_config = cron.get_configured_triggers(CronTriggerID::MQTT);
        std::vector<String> subscribed_topics;
        for (auto &conf: trigger_config) {
            bool already_subscribed = false;
            for (auto &new_topic: subscribed_topics) {
                if (conf.second->get("topic")->asString() == new_topic)
                    already_subscribed = true;
            }
            const size_t idx = conf.first;
            if (!already_subscribed) {
                subscribe(conf.second->get("topic")->asString(), [this, idx](const char *tpic, size_t tpic_len, char * data, size_t data_len) {
                    MqttMessage msg;
                    msg.topic = String(tpic).substring(0, tpic_len);
                    msg.payload = String(data).substring(0, data_len);
                    msg.retained = false;
                    if (cron.trigger_action(CronTriggerID::MQTT, &msg, &trigger_action))
                        return;
                }, false);
                subscribed_topics.push_back(conf.second->get("topic")->asString());
            }
        }
    }
#endif
}

void Mqtt::register_events() {
    if (!config.get("enable_mqtt")->asBool()) {
        return;
    }

    // Start MQTT client here to make sure all handlers are already registered.

    // Start immediately if we already have a working ethernet connection. WiFi takes a bit longer.
    // Wait 20 secs to not spam the event log with a failed connection attempt.
    bool start_immediately = false;
#if MODULE_ETHERNET_AVAILABLE()
    start_immediately = ethernet.get_connection_state() == EthernetState::CONNECTED;
#endif
    if (start_immediately)
        esp_mqtt_client_start(client);
    else
        task_scheduler.scheduleOnce([this]() {
            esp_mqtt_client_start(client);
        }, 20000);
}

#if MODULE_CRON_AVAILABLE()
bool Mqtt::action_triggered(Config *config, void *data) {
    Config *cfg = (Config*)config->get();
    MqttMessage *msg = (MqttMessage *)data;
    switch (config->getTag<CronTriggerID>())
    {
        case CronTriggerID::MQTT:
        if (cfg->get("payload")->asString() == msg->payload)
            return true;
        break;

    default:
        break;
    }
    return false;
}
#endif
