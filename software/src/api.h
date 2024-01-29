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

#include "config.h"

#include <Arduino.h>

#include <functional>
#include <initializer_list>
#include <vector>

#include "module.h"
#include "web_server.h"
#include "chunked_response.h"
#include "tools.h"

struct StateRegistration {
    String path;
    ConfigRoot *config;
    std::vector<String> keys_to_censor;
    bool low_latency;
};

struct CommandRegistration {
    String path;
    ConfigRoot *config;
    std::function<void(String &)> callback;
    std::vector<String> keys_to_censor_in_debug_report;
    bool is_action;
    uint64_t task_id;
    Config *config_in_flight;
};

struct RawCommandRegistration {
    String path;
    std::function<String(char *, size_t)> callback;
    bool is_action;
};

struct ResponseRegistration {
    String path;
    ConfigRoot *config;
    std::function<void(IChunkedResponse *, Ownership *, uint32_t)> callback;
    std::vector<String> keys_to_censor_in_debug_report;
};

class IAPIBackend : public IModule
{
public:
    virtual void addCommand(size_t commandIdx, const CommandRegistration &reg) = 0;
    virtual void addState(size_t stateIdx, const StateRegistration &reg) = 0;
    virtual void addRawCommand(size_t rawCommandIdx, const RawCommandRegistration &reg) = 0;
    virtual void addResponse(size_t responseIdx, const ResponseRegistration &reg) = 0;
    virtual bool pushStateUpdate(size_t stateIdx, const String &payload, const String &path) = 0;
    virtual bool pushRawStateUpdate(const String &payload, const String &path) = 0;
    enum class WantsStateUpdate {
        No,
        AsConfig,
        AsString
    };
    virtual WantsStateUpdate wantsStateUpdate(size_t stateIdx);
};

class API
{
public:
    API();

    void pre_setup();
    void setup();

    // Call this method only if you are a IAPIBackend and run in another FreeRTOS task!
    String callCommand(CommandRegistration &reg, char *payload, size_t len);

    // Call this method only if you are a IAPIBackend and run in another FreeRTOS task!
    void callCommandNonBlocking(CommandRegistration &reg, char *payload, size_t len, std::function<void(String)> done_cb);

    String callCommand(const char *path, Config::ConfUpdate payload);

    void callResponse(ResponseRegistration &reg, char *payload, size_t len, IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);

    const Config *getState(const String &path, bool log_if_not_found = true);

    void addFeature(const char *name);
    void addCommand(const String &path, ConfigRoot *config, std::initializer_list<String> keys_to_censor_in_debug_report, std::function<void()> callback, bool is_action);
    void addCommand(const String &path, ConfigRoot *config, std::initializer_list<String> keys_to_censor_in_debug_report, std::function<void(String &)> callback, bool is_action);

    void addState(const String &path, ConfigRoot *config, std::initializer_list<String> keys_to_censor = {}, bool low_latency = false);
    bool addPersistentConfig(const String &path, ConfigRoot *config, std::initializer_list<String> keys_to_censor = {});
    //void addTemporaryConfig(const String &path, Config *config, std::initializer_list<String> keys_to_censor, std::function<void(void)> callback);
    void addRawCommand(const String &path, std::function<String(char *, size_t)> callback, bool is_action);
    void addResponse(const String &path, ConfigRoot *config, std::initializer_list<String> keys_to_censor_in_debug_report, std::function<void(IChunkedResponse *, Ownership *, uint32_t)> callback);

    // TODO Remove deprecated functions. Marked as deprecated on 2024-01-29.
    template<typename T>
    [[gnu::deprecated("Pass bool low_latecy instead of interval_ms. Use 'false' or default for a 1000ms interval or 'true' for a 250ms interval.")]]
    void addState(const String &path, ConfigRoot *config, std::initializer_list<String> keys_to_censor, T interval_ms) {
        addState(path, config, keys_to_censor, interval_ms < 1000);
    }
    template<typename T>
    [[gnu::deprecated("Please remove the interval_ms parameter.")]]
    bool addPersistentConfig(const String &path, ConfigRoot *config, std::initializer_list<String> keys_to_censor, T interval_ms) {
        (void)interval_ms;
        return addPersistentConfig(path, config, keys_to_censor);
    }

    bool hasFeature(const char *name);

    static void writeConfig(const String &path, Config *config);
    static void removeConfig(const String &path);
    static void removeAllConfig();

    static bool restorePersistentConfig(const String &path, ConfigRoot *config);

    void registerDebugUrl();

    size_t registerBackend(IAPIBackend *backend);

    std::vector<StateRegistration> states;
    std::vector<CommandRegistration> commands;
    std::vector<RawCommandRegistration> raw_commands;
    std::vector<ResponseRegistration> responses;

    std::vector<IAPIBackend *> backends;

    ConfigRoot features;
    ConfigRoot version;

    uint8_t state_update_counter = 0;

private:
    bool already_registered(const String &path, const char *api_type);

    void executeCommand(const CommandRegistration &reg, Config::ConfUpdate payload);
};

// Make global variable available everywhere because it is not declared in modules.h.
// Definition is in api.cpp.
extern API api;
