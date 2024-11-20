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
#include <esp_crt_bundle.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "build.h"
#include "matchTopicFilter.h"

extern char local_uid_str[32];

// MQTT over WSS takes ~ 3.4k only for the connection
// + ~ 1.2k for publishing/subscribing.
// 6144 byte is the default value.
#define MQTT_TASK_STACK_SIZE  6144U

// Also change ws.cpp WS_SEND_BUFFER_SIZE when changing MQTT_RECV_BUFFER_SIZE here!
#if defined(BOARD_HAS_PSRAM)
#define MQTT_RECV_BUFFER_SIZE 10240U
#define MQTT_SEND_BUFFER_SIZE 32768U
#else
#define MQTT_RECV_BUFFER_SIZE 4096U
#define MQTT_SEND_BUFFER_SIZE 4096U
#endif

#define MQTT_RECV_BUFFER_HEADROOM (MQTT_RECV_BUFFER_SIZE / 6)

extern "C" esp_err_t esp_crt_bundle_attach(void *conf);

#if !MODULE_CERTS_AVAILABLE()
#define MAX_CERT_ID -1
#endif

void Mqtt::pre_setup()
{
    // The real UID will be patched in later
    config = ConfigRoot{Config::Object({
        {"enable_mqtt", Config::Bool(false)},
        {"broker_host", Config::Str("", 0, 128)},
        {"broker_port", Config::Uint16(1883)},
        {"broker_username", Config::Str("", 0, 64)},
        {"broker_password", Config::Str("", 0, 64)},
        {"global_topic_prefix", Config::Str(String(BUILD_HOST_PREFIX) + "/" + "ABC", 0, 64)},
        {"client_name", Config::Str(String(BUILD_HOST_PREFIX) + "-" + "ABC", 1, 64)},
        {"interval", Config::Uint(1, 0, 24 * 60 * 60)},
        // esp_mqtt_transport_t. -1 because we don't allow MQTT_TRANSPORT_UNKNOWN.
        {"protocol", Config::Uint(MQTT_TRANSPORT_OVER_TCP - 1, MQTT_TRANSPORT_OVER_TCP - 1, MQTT_TRANSPORT_OVER_WSS - 1)},
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
        {"client_cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
        {"client_key_id", Config::Int(-1, -1, MAX_CERT_ID)},
        {"path", Config::Str("", 0, 64)}
    }), [](Config &cfg, ConfigSource source) -> String {
#if MODULE_MQTT_AUTO_DISCOVERY_AVAILABLE()
        const String &global_topic_prefix = cfg.get("global_topic_prefix")->asString();
        const String &auto_discovery_prefix = mqtt_auto_discovery.config.get("auto_discovery_prefix")->asString();

        if (global_topic_prefix == auto_discovery_prefix)
            return "Global topic prefix cannot be the same as the MQTT auto discovery topic prefix.";
#endif
        return "";
    }};

    state = Config::Object({
        {"connection_state", Config::Enum(MqttConnectionState::NotConfigured, MqttConnectionState::NotConfigured, MqttConnectionState::Error)},
        {"connection_start", Config::Uint(0)},
        {"connection_end", Config::Uint(0)},
        {"last_error", Config::Int(0)}
    });

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_trigger(
        AutomationTriggerID::MQTT,
        Config::Object({
            {"topic_filter", Config::Str("", 0, 32)},
            {"payload", Config::Str("", 0, 32)},
            {"retain", Config::Bool(false)},
            {"use_prefix", Config::Bool(false)}
        }),
        [this](const Config *cfg) {
            const CoolString &topic = cfg->get("topic_filter")->asString();
            if (topic.startsWith(this->config.get("global_topic_prefix")->asString())) {
                return String("MQTT topic must not contain the global prefix.");
            }
            bool valid = true;
            int pos = topic.indexOf('#');
            if ((pos != -1 && pos != topic.length() - 1) || (pos > 1 && topic[pos - 1] != '/')) {
                valid = false;
            }

            pos = topic.indexOf('+');
            while (pos != -1) {
                if (pos != 0 && topic[pos - 1] != '/')
                    valid = false;
                if (pos != topic.length() - 1 && topic[pos + 1] != '/')
                    valid = false;
                pos = topic.indexOf('+', pos + 1);
            }
            if (!valid) {
                return String("Invalid use of wildcards in topic.");
            }
            return String("");
        },
        false
    );

    automation.register_action(
        AutomationActionID::MQTT,
        Config::Object({
            {"topic", Config::Str("", 0, 32)},
            {"payload", Config::Str("", 0, 32)},
            {"retain", Config::Bool(false)},
            {"use_prefix", Config::Bool(false)}
        }),
        [this](const Config *cfg) {
            String topic = cfg->get("topic")->asString();
            if (cfg->get("use_prefix")->asBool()) {
                topic = config.get("global_topic_prefix")->asString() + "/automation_action/" + topic;
            }
            publish(topic, cfg->get("payload")->asString(), cfg->get("retain")->asBool());
        },
        [this](const Config *cfg) {
            const CoolString &topic = cfg->get("topic")->asString();
            if (topic.startsWith(this->config.get("global_topic_prefix")->asString())) {
                return String("MQTT topic must not contain the global prefix.");
            }

            if (topic.indexOf('#') != -1 || topic.indexOf('+') != -1) {
                return String("MQTT topic must not contain wildcards.");
            }
            return String("");
        },
        false
    );
#endif
}

void Mqtt::subscribe(const String &path, SubscribeCallback &&callback, Retained retained, CallbackInThread callback_in_thread, AddPrefix add_prefix)
{
    if (client == nullptr) {
        return;
    }

    const String *topic;
    bool starts_with_global_topic_prefix;
    String local_topic(static_cast<const char *>(nullptr));

    if (add_prefix == AddPrefix::No) {
        topic = &path;
        starts_with_global_topic_prefix = path.startsWith(global_topic_prefix);
    } else {
        local_topic.reserve(128);
        local_topic.concat(global_topic_prefix);
        local_topic.concat('/');
        local_topic.concat(path);

        topic = &local_topic;
        starts_with_global_topic_prefix = true;
    }

    bool subscribed = esp_mqtt_client_subscribe(client, topic->c_str(), 0) >= 0;

    this->commands.push_back({*topic, std::move(callback), retained, callback_in_thread, starts_with_global_topic_prefix, subscribed});
}

void Mqtt::addCommand(size_t commandIdx, const CommandRegistration &reg)
{
    auto req_size = reg.config->max_string_length();
    if (req_size > MQTT_RECV_BUFFER_SIZE) {
        logger.printfln("Recv buf is %u bytes. %s requires %u. Bump MQTT_RECV_BUFFER_SIZE! Updates on this topic might break the MQTT connection!", MQTT_RECV_BUFFER_SIZE, reg.path, req_size);
        return;
    }
#if MODULE_DEBUG_AVAILABLE()
    if (req_size > (MQTT_RECV_BUFFER_SIZE - MQTT_RECV_BUFFER_HEADROOM))
        logger.printfln("Recv buf is %u bytes. %s requires %u. Maybe bump MQTT_RECV_BUFFER_SIZE?", MQTT_RECV_BUFFER_SIZE, reg.path, req_size);
#endif
}

void Mqtt::addState(size_t stateIdx, const StateRegistration &reg)
{
    this->states.push_back({0});
}

void Mqtt::addResponse(size_t responseIdx, const ResponseRegistration &reg)
{
}

bool Mqtt::publish_with_prefix(const String &path, const String &payload, bool retain)
{
    String topic = global_topic_prefix + "/" + path;
    return publish(topic, payload, retain);
}

bool Mqtt::publish(const String &topic, const String &payload, bool retain)
{
    // ESP-MQTT does this check but we only want to allow publishing after
    // onMqttConnect was called (in the main thread!)
    // ESP-MQTT's check can asynchronously flip to connected.
    if (client == nullptr || this->state.get("connection_state")->asEnum<MqttConnectionState>() != MqttConnectionState::Connected)
        return false;

// enqueue uses an unbounded queue! IDF 5.3 adds a limit for the queue. Until then use publish even though it blocks the main thread.
//#if defined(BOARD_HAS_PSRAM)
//    return esp_mqtt_client_enqueue(this->client, topic.c_str(), payload.c_str(), payload.length(), 0, retain, true) >= 0;
//#else
    return esp_mqtt_client_publish(this->client, topic.c_str(), payload.c_str(), payload.length(), 0, retain) >= 0;
//#endif
}

bool Mqtt::pushStateUpdate(size_t stateIdx, const String &payload, const String &path)
{
    auto &state = this->states[stateIdx];

    if (!deadline_elapsed(state.last_send_ms + this->send_interval_ms))
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

IAPIBackend::WantsStateUpdate Mqtt::wantsStateUpdate(size_t stateIdx) {
    return this->state.get("connection_state")->asEnum<MqttConnectionState>() == MqttConnectionState::Connected ?
           IAPIBackend::WantsStateUpdate::AsString :
           IAPIBackend::WantsStateUpdate::No;
}

void Mqtt::resubscribe()
{
    if (client == nullptr || this->state.get("connection_state")->asEnum<MqttConnectionState>() != MqttConnectionState::Connected)
        return;

    if (!global_topic_prefix_subscribed) {
        String topic = global_topic_prefix + "/#";
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

    const char *schema = "";
    bool print_path = false;
    // + 1 to undo the -1 in the config's definition.
    switch (this->config.get("protocol")->asUint() + 1) {
        case MQTT_TRANSPORT_OVER_TCP:
            schema = "mqtt://";
            break;
        case MQTT_TRANSPORT_OVER_SSL:
            schema = "mqtts://";
            break;
        case MQTT_TRANSPORT_OVER_WS:
            schema = "ws://";
            print_path = true;
            break;
        case MQTT_TRANSPORT_OVER_WSS:
            schema = "wss://";
            print_path = true;
            break;
    }
    logger.printfln("Connected to broker at %s%s:%u%s.", schema, this->config.get("broker_host")->asEphemeralCStr(), this->config.get("broker_port")->asUint(), print_path ? this->config.get("broker_path")->asEphemeralCStr() : "");

    this->state.get("connection_state")->updateEnum(MqttConnectionState::Connected);

    for (size_t i = 0; i < api.commands.size(); ++i) {
        auto &reg = api.commands[i];
        this->addCommand(i, reg);
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
    if (this->state.get("connection_state")->asEnum<MqttConnectionState>() == MqttConnectionState::NotConnected)
        logger.printfln("Failed to connect to broker.");
    else
        logger.printfln("Disconnected from broker.");

    this->state.get("connection_state")->updateEnum(MqttConnectionState::NotConnected);
    if (was_connected) {
        was_connected = false;
        uint32_t now = millis();
        uint32_t connected_for = now - last_connected_ms;
        state.get("connection_end")->updateUint(now);
        if (connected_for < 0x7FFFFFFF) {
            logger.printfln("Was connected for %u seconds.", connected_for / 1000);
        } else {
            logger.printfln("Was connected for a long time.");
        }
    }
}

static bool filter_mqtt_log(const char *topic, size_t topic_len)
{
#if MODULE_AUTOMATION_AVAILABLE()
    if (topic_len >= 12 && strncmp(topic, "automation_action/", 12) == 0)
        return false;
#endif

    return true;
}

void Mqtt::onMqttMessage(char *topic, size_t topic_len, char *data, size_t data_len, bool retain)
{
    if (client == nullptr) {
        return;
    }

    for (auto &c : commands) {
        if (!matchTopicFilter(topic, topic_len, c.topic.c_str(), c.topic.length()))
            continue;

        if (retain && c.retained != Retained::Accept) {
            if (c.retained == Retained::IgnoreWarn) {
                logger.printfln("Retained messages on topic %s are forbidden. Ignoring retained message (data_len=%u).", c.topic.c_str(), data_len);
            }
            return;
        }

        if (c.callback_in_thread == CallbackInThread::Main) {
            esp_mqtt_client_disable_receive(client, 100);
            char *copy_buf = (char *)malloc(topic_len + data_len);
            if (copy_buf == nullptr) {
                logger.printfln("Failed to run command %s: Failed to allocate copy_buf", c.topic.c_str());
                return;
            }

            memcpy(copy_buf, topic, topic_len);
            memcpy(copy_buf + topic_len, data, data_len);

            task_scheduler.scheduleOnce([this, c, copy_buf, topic_len, data_len](){
                c.callback(copy_buf, topic_len, copy_buf + topic_len, data_len);
                free(copy_buf);
                esp_mqtt_client_enable_receive(client);
            });
        } else
            c.callback(topic, topic_len, data, data_len);

        return;
    }

    if (topic_len < global_topic_prefix.length() + 1) // + 1 because we will check for the / between the prefix and the topic.
        return;
    if (memcmp(topic, global_topic_prefix.c_str(), global_topic_prefix.length()) != 0)
        return;
    if (topic[global_topic_prefix.length()] != '/')
        return;

    topic += global_topic_prefix.length() + 1;
    topic_len -= global_topic_prefix.length() + 1;

    for (auto &reg : api.commands) {
        if (topic_len != reg.path_len || memcmp(topic, reg.path, topic_len) != 0)
            continue;

        if (retain && reg.is_action) {
            logger.printfln("Topic %s is an action. Ignoring retained message (data_len=%u).", reg.path, data_len);
            return;
        }

        if (reg.is_action && data_len == 0) {
            logger.printfln("Topic %s is an action. Ignoring empty message.", reg.path);
            return;
        }

        esp_mqtt_client_disable_receive(client, 100);
        api.callCommandNonBlocking(reg, data, data_len, [this, reg](const String &error) {
            if (!error.isEmpty())
                logger.printfln("On %s: %s", reg.path, error.c_str());
            esp_mqtt_client_enable_receive(this->client);
        });

        return;
    }

    // Don't print error message on state topics, this could be one of our own messages.
    for (auto &reg : api.states) {
        if (topic_len != reg.path_len || memcmp(topic, reg.path, topic_len) != 0)
            continue;
        return;
    }

    // Don't print error message if this packet was received because it was retained (as opposed to a newly published message)
    // The spec says:
    // It MUST set the RETAIN flag to 0 when a PUBLISH Packet is sent to a Client
    // because it matches an established subscription regardless of how the flag was set in the message it received [MQTT-3.3.1-9].
    if (!retain && filter_mqtt_log(topic, topic_len))
        logger.printfln("Received message on unknown topic '%.*s' (data_len=%u)", static_cast<int>(topic_len), topic, data_len);
}

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
            });
            break;
        case MQTT_EVENT_DISCONNECTED:
            task_scheduler.scheduleOnce([mqtt](){
                mqtt->onMqttDisconnect();
            });
            break;
        case MQTT_EVENT_DATA:
            if (event->current_data_offset != 0)
                return;
            if (event->total_data_len != event->data_len) {
                logger.printfln("Ignoring message with payload length %d for topic %.*s. Maximum length allowed is %u.", event->total_data_len, event->topic_len, event->topic, MQTT_RECV_BUFFER_SIZE);
                return;
            }
            mqtt->onMqttMessage(event->topic, event->topic_len, event->data, event->data_len, event->retain);
            break;
        case MQTT_EVENT_ERROR: {
                auto eh = *event->error_handle;
                task_scheduler.scheduleOnce([mqtt, eh](){
                    bool was_connected = mqtt->state.get("connection_state")->asEnum<MqttConnectionState>() != MqttConnectionState::NotConnected;

                    if (eh.error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                        if (was_connected) {
                            if (eh.esp_tls_last_esp_err != ESP_OK) {
                                char err_buf[64];
                                const char *e = esp_err_to_name_r(eh.esp_tls_last_esp_err, err_buf, ARRAY_SIZE(err_buf));
                                logger.printfln("Transport error: %s (esp_tls_last_esp_err)", e);
                                mqtt->state.get("last_error")->updateInt(eh.esp_tls_last_esp_err);
                            } else if (eh.esp_tls_stack_err != 0) {
                                char err_buf[64];
                                const char *e = esp_err_to_name_r(eh.esp_tls_stack_err, err_buf, ARRAY_SIZE(err_buf));
                                logger.printfln("Transport error: %s (esp_tls_stack_err)", e);
                                mqtt->state.get("last_error")->updateInt(eh.esp_tls_stack_err);
                            } else {
                                logger.printfln("Unknown transport error after initial connect");
                                mqtt->state.get("last_error")->updateInt(0xFFFFFFFD);
                            }
                        } else if (eh.esp_transport_sock_errno != 0) {
                            const char *e = strerror(eh.esp_transport_sock_errno);
                            logger.printfln("Transport error: %s", e);
                            mqtt->state.get("last_error")->updateInt(eh.esp_transport_sock_errno);
                        } else {
                            logger.printfln("Unknown transport error");
                            mqtt->state.get("last_error")->updateInt(0xFFFFFFFE);
                        }
                    } else if (eh.error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                        logger.printfln("Connection refused: %s", get_mqtt_error(eh.connect_return_code));
                        // Minus to indicate this is a connection error
                        mqtt->state.get("last_error")->updateInt(-eh.connect_return_code);
                    } else {
                        logger.printfln("Unknown error");
                        mqtt->state.get("last_error")->updateInt(0xFFFFFFFF);
                    }
                });
                break;
            }
        default:
            break;
    }
}

void Mqtt::setup()
{
    initialized = true;

    if (!api.restorePersistentConfig("mqtt/config", &config)) {
        config.get("global_topic_prefix")->updateString(String(BUILD_HOST_PREFIX) + "/" + local_uid_str);
        config.get("client_name")->updateString(String(BUILD_HOST_PREFIX) + "-" + local_uid_str);

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

    global_topic_prefix = this->config.get("global_topic_prefix")->asString();
    send_interval_ms = this->config.get("interval")->asUint() * 1000;
    client_name = this->config.get("client_name")->asString();
    global_topic_prefix = this->config.get("global_topic_prefix")->asString();

    if (!config.get("enable_mqtt")->asBool()) {
        return;
    }

#if MODULE_AUTOMATION_AVAILABLE()
    automation.set_enabled(AutomationTriggerID::MQTT, true);
    automation.set_enabled(AutomationActionID::MQTT, true);
#endif

    this->backend_idx = api.registerBackend(this);

    esp_log_level_set("MQTT_CLIENT", ESP_LOG_NONE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_NONE);
    esp_log_level_set("esp-tls", ESP_LOG_NONE);
    esp_log_level_set("TRANSPORT", ESP_LOG_NONE);
    esp_log_level_set("OUTBOX", ESP_LOG_NONE);

    esp_mqtt_client_config_t mqtt_cfg = {};

    mqtt_cfg.host = config.get("broker_host")->asEphemeralCStr();
    mqtt_cfg.port = config.get("broker_port")->asUint();
    mqtt_cfg.client_id = config.get("client_name")->asEphemeralCStr();
    mqtt_cfg.username = config.get("broker_username")->asEphemeralCStr();
    mqtt_cfg.password = config.get("broker_password")->asEphemeralCStr();
    mqtt_cfg.task_stack = MQTT_TASK_STACK_SIZE;
    mqtt_cfg.buffer_size = MQTT_RECV_BUFFER_SIZE;
    mqtt_cfg.out_buffer_size = MQTT_SEND_BUFFER_SIZE;
    mqtt_cfg.network_timeout_ms = 3000;
    mqtt_cfg.message_retransmit_timeout = 800;
    // + 1 to undo the -1 in the config's definition.
    mqtt_cfg.transport = (esp_mqtt_transport_t)(config.get("protocol")->asUint() + 1);
    bool encrypted = mqtt_cfg.transport == MQTT_TRANSPORT_OVER_SSL || mqtt_cfg.transport == MQTT_TRANSPORT_OVER_WSS;
    int cert_id = config.get("cert_id")->asInt();

    if (encrypted && cert_id != -1) {
#if MODULE_CERTS_AVAILABLE()
        size_t cert_len = 0;
        auto cert = certs.get_cert((uint8_t)cert_id, &cert_len);
        if (cert == nullptr) {
            logger.printfln("Certificate with ID %d is not available", cert_id);
            return;
        }
        // Leak cert here: MQTT requires the buffer to live forever.
        mqtt_cfg.cert_pem = (const char *)cert.release();
#else
        // defense in depth: it should not be possible to arrive here because in case
        // that the certs module is not available the cert_id should always be -1
        logger.printfln("Can't use custom certificate: certs module is not built into this firmware!");
        return;
#endif
    }
    else if (encrypted) {
        mqtt_cfg.crt_bundle_attach = esp_crt_bundle_attach;
    }

    int client_cert_id = config.get("client_cert_id")->asInt();

    if (encrypted && client_cert_id != -1) {
#if MODULE_CERTS_AVAILABLE()
        size_t cert_len = 0;
        auto cert = certs.get_cert((uint8_t)client_cert_id, &cert_len);
        if (cert == nullptr) {
            logger.printfln("Client certificate with ID %d is not available", client_cert_id);
            return;
        }
        // Leak cert here: MQTT requires the buffer to live forever.
        mqtt_cfg.client_cert_pem = (const char *)cert.release();
#else
        // defense in depth: it should not be possible to arrive here because in case
        // that the certs module is not available the cert_id should always be -1
        logger.printfln("Can't use custom client certificate: certs module is not built into this firmware!");
        return;
#endif
    }

    int client_key_id = config.get("client_key_id")->asInt();

    if (encrypted && client_key_id != -1) {
#if MODULE_CERTS_AVAILABLE()
        size_t cert_len = 0;
        auto cert = certs.get_cert((uint8_t)client_key_id, &cert_len);
        if (cert == nullptr) {
            logger.printfln("Client key with ID %d is not available", client_key_id);
            return;
        }
        // Leak cert here: MQTT requires the buffer to live forever.
        mqtt_cfg.client_key_pem = (const char *)cert.release();
#else
        // defense in depth: it should not be possible to arrive here because in case
        // that the certs module is not available the cert_id should always be -1
        logger.printfln("Can't use custom client key: certs module is not built into this firmware!");
        return;
#endif
    }

    if ((mqtt_cfg.transport == MQTT_TRANSPORT_OVER_WS || mqtt_cfg.transport == MQTT_TRANSPORT_OVER_WSS) && config.get("path")->asString().length() > 0) {
        mqtt_cfg.path = config.get("path")->asEphemeralCStr();
        logger.printfln("Using path %s", mqtt_cfg.path);
    }

    // Set connection state here. Otherwise, it will stay stay "not configured" until the first connection attempt.
    state.get("connection_state")->updateEnum(MqttConnectionState::NotConnected);

    client = esp_mqtt_client_init(&mqtt_cfg);

    if (client == nullptr) {
        logger.printfln("Could not create MQTT client");
        return;
    }

    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, this);

    task_scheduler.scheduleWithFixedDelay([this](){
        this->resubscribe();
    }, 1_s, 1_s);
}

void Mqtt::register_urls()
{
    api.addPersistentConfig("mqtt/config", &config, {"broker_password"});
    api.addState("mqtt/state", &state);

#if MODULE_AUTOMATION_AVAILABLE()
    if (automation.has_task_with_trigger(AutomationTriggerID::MQTT) && config.get("enable_mqtt")->asBool()) {
        Automation::ConfigVec trigger_config = automation.get_configured_triggers(AutomationTriggerID::MQTT);
        std::vector<String> subscribed_topics;
        for (auto &conf : trigger_config) {
            bool already_subscribed = false;
            for (auto &new_topic : subscribed_topics) {
                if (conf.second->get("topic_filter")->asString() == new_topic)
                    already_subscribed = true;
            }
            const size_t idx = conf.first;
            if (!already_subscribed) {
                String topic = conf.second->get("topic_filter")->asString();
                if (conf.second->get("use_prefix")->asBool()) {
                    topic = config.get("global_topic_prefix")->asString() + "/automation_trigger/" + topic;
                }
                subscribe(topic, [this, idx](const char *tpic, size_t tpic_len, char * data, size_t data_len) {
                    MqttMessage msg;
                    msg.topic = String(tpic).substring(0, tpic_len);
                    msg.payload = String(data).substring(0, data_len);
                    msg.retained = false;
                    if (automation.trigger(AutomationTriggerID::MQTT, &msg, this))
                        return;
                }, conf.second->get("retain")->asBool() ? Retained::Accept : Retained::IgnoreWarn);
                subscribed_topics.push_back(topic);
            }
        }
    }
#endif
}

void Mqtt::register_events()
{
    if (client == nullptr || !config.get("enable_mqtt")->asBool()) {
        return;
    }

    // Start MQTT client here to make sure all handlers are already registered.
    event.registerEvent("network/state", {"connected"}, [this](const Config *connected) {
        if (connected->asBool()) {
            esp_mqtt_client_start(client);
#if MODULE_DEBUG_AVAILABLE()
            debug.register_task("mqtt_task", MQTT_TASK_STACK_SIZE);
#endif
        }
        else {
#if MODULE_DEBUG_AVAILABLE()
            // This event will trigger during start-up and may send a 'disconnected' event in an already disconnected state.
            // The following esp_mqtt_client_stop doesn't care, but deregistering a task that doesn't exist will log a warning.
            // Only handle 'disconnected' events if they’re not the very first event.
            if (this->initial_network_event_seen) {
                debug.deregister_task("mqtt_task");
            }
#endif
            esp_mqtt_client_stop(client);
        }

        this->initial_network_event_seen = true;

        return EventResult::OK;
    });
}

void Mqtt::pre_reboot()
{
    if (client != nullptr) {
#if MODULE_DEBUG_AVAILABLE()
        debug.deregister_task("mqtt_task");
#endif
        esp_mqtt_client_stop(client);
    }
}

#if MODULE_AUTOMATION_AVAILABLE()
bool Mqtt::has_triggered(const Config *conf, void *data)
{
    const Config *cfg = static_cast<const Config *>(conf->get());
    const MqttMessage *msg = (const MqttMessage *)data;
    const String &payload = cfg->get("payload")->asString();

    String topic(static_cast<const char *>(nullptr));
    if (cfg->get("use_prefix")->asBool()) {
        topic.reserve(64);
        topic = this->config.get("global_topic_prefix")->asString();
        topic += "/automation_trigger/";
    }
    topic += cfg->get("topic_filter")->asString();

    switch (conf->getTag<AutomationTriggerID>())
    {
    case AutomationTriggerID::MQTT:
        if (msg->topic == topic && (payload.length() == 0 || msg->payload == payload))
            return true;
        break;

    default:
        break;
    }
    return false;
}
#endif
