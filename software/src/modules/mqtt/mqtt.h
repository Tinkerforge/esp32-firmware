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

#pragma once

#include "mqtt_client.h"

#include "api.h"
#include "config.h"

enum class MqttConnectionState {
    NOT_CONFIGURED,
    NOT_CONNECTED,
    CONNECTED,
    ERROR
};

struct MqttCommand {
    String topic;
    std::function<void(const char *, size_t, char *, size_t)> callback;
    bool forbid_retained;
    bool starts_with_global_topic_prefix;
};

struct MqttState {
    String topic;
    uint32_t last_send_ms;
};

class Mqtt final : public IAPIBackend
{
public:
    Mqtt(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;
    void connect();

    // Retain messages by default because we only send on change.
    void publish_with_prefix(const String &path, const String &payload, bool retain=true);
    void subscribe_with_prefix(const String &path, std::function<void(const char *, size_t, char *, size_t)> callback, bool forbid_retained);
    void publish(const String &topic, const String &payload, bool retain);
    void subscribe(const String &topic, std::function<void(const char *, size_t, char *, size_t)> callback, bool forbid_retained);

    // IAPIBackend implementation
    void addCommand(size_t commandIdx, const CommandRegistration &reg) override;
    void addState(size_t stateIdx, const StateRegistration &reg) override;
    void addRawCommand(size_t rawCommandIdx, const RawCommandRegistration &reg) override;
    void addResponse(size_t responseIdx, const ResponseRegistration &reg) override;
    bool pushStateUpdate(size_t stateIdx, const String &payload, const String &path) override;
    void pushRawStateUpdate(const String &payload, const String &path) override;

    void onMqttConnect();
    void onMqttMessage(char *topic, size_t topic_len, char *data, size_t data_len, bool retain);
    void onMqttDisconnect();

    ConfigRoot config;
    ConfigRoot state;

    ConfigRoot config_in_use;

    std::vector<MqttCommand> commands;
    std::vector<MqttState> states;
private:
    esp_mqtt_client_handle_t client;

    uint32_t last_connected_ms = 0;
    bool was_connected = false;
};
