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

#include <mqtt_client.h>

#include "module.h"
#include "config.h"
#include "modules/api/api.h"
#include "mqtt_connection_state.enum.h"
#include "module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

class Mqtt final : public IModule,
                   public IAPIBackend
#if MODULE_AUTOMATION_AVAILABLE()
                 , public IAutomationBackend
#endif
{
public:
    using SubscribeCallback = std::function<void(const char *, size_t, char *, size_t)>;

    enum class Retained : uint8_t {
        Accept       = 0,
        IgnoreWarn   = 1,
        IgnoreSilent = 2,
    };

    enum class CallbackInThread : uint8_t {
        Main = 0,
        Mqtt = 1,
    };

    enum class AddPrefix {
        No  = 0,
        Yes = 1,
    };

    Mqtt(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;
    void pre_reboot() override;

    // Retain messages by default because we only send on change.
    bool publish_with_prefix(const String &path, const String &payload, bool retain = true);
    bool publish(const String &topic, const String &payload, bool retain);

    void subscribe(const String &path, SubscribeCallback &&callback, Retained retained, CallbackInThread callback_in_thread = CallbackInThread::Main, AddPrefix add_prefix = AddPrefix::No);

    // IAPIBackend implementation
    void addCommand(size_t commandIdx, const CommandRegistration &reg) override;
    void addState(size_t stateIdx, const StateRegistration &reg) override;
    void addResponse(size_t responseIdx, const ResponseRegistration &reg) override;
    bool pushStateUpdate(size_t stateIdx, const String &payload, const String &path) override;
    bool pushRawStateUpdate(const String &payload, const String &path) override;
    WantsStateUpdate wantsStateUpdate(size_t stateIdx) override;

    void onMqttConnect();
    void onMqttMessage(char *topic, size_t topic_len, char *data, size_t data_len, bool retain);
    void onMqttDisconnect();

    void resubscribe();

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

    ConfigRoot config;
    ConfigRoot state;

    // Both strings are read by mqtt_auto_discovery.
    String client_name;
    // Copy prefix to not access config in MQTT thread.
    String global_topic_prefix;

private:
    struct MqttCommand {
        String topic;
        SubscribeCallback callback;
        Retained retained;
        CallbackInThread callback_in_thread;
        bool starts_with_global_topic_prefix;
        bool subscribed;
    };

    struct MqttState {
        uint32_t last_send_ms;
    };

    struct MqttMessage {
        String topic;
        String payload;
        bool retained;
    };

    std::vector<MqttCommand> commands;
    std::vector<MqttState, IRAMAlloc<MqttState>> states;

    size_t backend_idx;

    esp_mqtt_client_handle_t client = nullptr;
    uint32_t send_interval_ms;

    uint32_t last_connected_ms = 0;
    bool was_connected = false;
    bool global_topic_prefix_subscribed = false;
};
