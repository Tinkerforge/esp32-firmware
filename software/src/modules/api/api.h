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

#include <Arduino.h>
#include <functional>
#include <initializer_list>
#include <vector>

#include "module.h"
#include "config.h"
#include "chunked_response.h"
#include "tools.h"
#include "modules/web_server/web_server.h"

// Will be stored in IRAM -> use 32 bit integers even if a bool would be sufficient
struct StateRegistration {
    const char *const path;
    const char *const *const keys_to_censor;
    const char *const *const keys_to_censor_in_debug_report;
    ConfigRoot *const config;

    const size_t path_len;
    const size_t keys_to_censor_len;
    const size_t keys_to_censor_in_debug_report_len;
    const uint32_t low_latency;
};

// Will be stored in IRAM -> use 32 bit integers even if a bool would be sufficient
struct CommandRegistration {
    const char *const path;
    const char *const *const keys_to_censor_in_debug_report;
    ConfigRoot *const config;
    const std::function<void(String &)> callback;

    const size_t path_len;
    const size_t keys_to_censor_in_debug_report_len;
    const uint32_t is_action;
};

struct ResponseRegistration {
    const char *const path;
    const char *const *const keys_to_censor_in_debug_report;
    ConfigRoot *config;
    std::function<void(IChunkedResponse *, Ownership *, uint32_t)> callback;

    const size_t path_len;
    const size_t keys_to_censor_in_debug_report_len;
};

class IAPIBackend
{
public:
    virtual void addCommand(size_t commandIdx, const CommandRegistration &reg) = 0;
    virtual void addState(size_t stateIdx, const StateRegistration &reg) = 0;
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

class API final : public IModule
{
public:
    API();

    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    // Call this method only if you are a IAPIBackend and run in another FreeRTOS task!
    String callCommand(CommandRegistration &reg, char *payload, size_t len);

    // Call this method only if you are a IAPIBackend and run in another FreeRTOS task!
    void callCommandNonBlocking(CommandRegistration &reg, char *payload, size_t len, const std::function<void(const String &errmsg)> &done_cb);

    String callCommand(const char *path, Config::ConfUpdate payload);

    void callResponse(ResponseRegistration &reg, char *payload, size_t len, IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);

    const Config *getState(const char *path, bool log_if_not_found = true, size_t path_len = 0);
    const Config *getState(const String &path, bool log_if_not_found = true);

    void addFeature(const char *name);

    // Prefer this version of addCommand over the one below.
    void addCommand(const char * const path, ConfigRoot *config, std::initializer_list<const char *> keys_to_censor_in_debug_report, std::function<void(String &errmsg)> &&callback, bool is_action);
    void addCommand(const String &path,      ConfigRoot *config, std::initializer_list<const char *> keys_to_censor_in_debug_report, std::function<void(String &errmsg)> &&callback, bool is_action);

    void addState(const char * const path, ConfigRoot *config, std::initializer_list<const char *> keys_to_censor = {}, std::initializer_list<const char *> keys_to_censor_in_debug_report = {}, bool low_latency = false);
    void addState(const String &path, ConfigRoot *config, std::initializer_list<const char *> keys_to_censor = {}, std::initializer_list<const char *> keys_to_censor_in_debug_report = {}, bool low_latency = false);

    bool addPersistentConfig(const String &path, ConfigRoot *config, std::initializer_list<const char *> keys_to_censor = {});
    void addResponse(const char * const path, ConfigRoot *config, std::initializer_list<const char *> keys_to_censor_in_debug_report, std::function<void(IChunkedResponse *, Ownership *, uint32_t)> &&callback);

    // TODO Remove deprecated functions. Marked as deprecated on 2024-10-04.
    [[gnu::deprecated("Please add a 'String &' parameter to the callback lambda. It can be unused or unnamed.")]]
    void addCommand(const char * const path, ConfigRoot *config, std::initializer_list<const char *> keys_to_censor_in_debug_report, std::function<void()> &&callback, bool is_action);
    [[gnu::deprecated("Please add a 'String &' parameter to the callback lambda. It can be unused or unnamed.")]]
    void addCommand(const String &path,      ConfigRoot *config, std::initializer_list<const char *> keys_to_censor_in_debug_report, std::function<void()> &&callback, bool is_action);

    bool hasFeature(const char *name);

    static void writeConfig(const String &path, Config *config);
    static void removeConfig(const String &path);
    static void removeAllConfig();

    static bool restorePersistentConfig(const String &path, ConfigRoot *config);

    static String getLittleFSConfigPath(const String &path, bool tmp = false);

    size_t registerBackend(IAPIBackend *backend);

    std::vector<StateRegistration, IRAMAlloc<StateRegistration>> states;
    std::vector<CommandRegistration, IRAMAlloc<CommandRegistration>> commands;
    std::vector<ResponseRegistration, IRAMAlloc<ResponseRegistration>> responses;

    std::vector<IAPIBackend *> backends;

    ConfigRoot features;
    ConfigRoot version;

    uint8_t state_update_counter = 0;

private:
    bool already_registered(const char *path, size_t path_len, const char *api_type);

    void executeCommand(const CommandRegistration &reg, Config::ConfUpdate payload);

    Config features_prototype;
    Config modified_prototype;
};
