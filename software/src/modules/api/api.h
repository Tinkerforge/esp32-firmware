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

#include <WString.h>
#include <functional>
#include <vector>
#include <span>

#include "module.h"
#include "config.h"
#include "chunked_response.h"
#include "tools/allocator.h"

// Will be stored in IRAM -> store uint8_ts in one uint32_t
struct StateRegistration {
    const char *const path;
    const char *const *const keys_to_censor;
    const char *const *const keys_to_censor_in_debug_report;
    ConfigRoot *const config;

    // asm("" : : "r" (data)); forces 32 bit access
    [[gnu::always_inline]] uint8_t get_path_len()                           const { asm("" : : "r" (data)); return (data >> 24) & 0xFF; }
    [[gnu::always_inline]] uint8_t get_keys_to_censor_len()                 const { asm("" : : "r" (data)); return (data >> 16) & 0xFF; }
    [[gnu::always_inline]] uint8_t get_keys_to_censor_in_debug_report_len() const { asm("" : : "r" (data)); return (data >>  8) & 0xFF; }
    [[gnu::always_inline]] bool    get_low_latency()                        const { asm("" : : "r" (data)); return (data >>  0) & 0xFF; }

    /*
    const uint8_t path_len;
    const uint8_t keys_to_censor_len;
    const uint8_t keys_to_censor_in_debug_report_len;
    const bool low_latency;
    */
    const uint32_t data;
};

// Will be stored in IRAM -> store uint8_ts in one uint32_t
struct CommandRegistration {
    const char *const path;
    const char *const *const keys_to_censor_in_debug_report;
    ConfigRoot *const config;
    const std::function<void(String &)> callback;

    // asm("" : : "r" (data)); forces 32 bit access
    [[gnu::always_inline]] uint8_t get_path_len()                           const { asm("" : : "r" (data)); return (data >> 24) & 0xFF; }
    [[gnu::always_inline]] uint8_t get_keys_to_censor_in_debug_report_len() const { asm("" : : "r" (data)); return (data >> 16) & 0xFF; }
    [[gnu::always_inline]] bool    get_is_action()                          const { asm("" : : "r" (data)); return (data >>  8) & 0xFF; }

    /*
    const uint8_t path_len;
    const uint8_t keys_to_censor_in_debug_report_len;
    const bool is_action;
    */
    const uint32_t data;
};

struct ResponseRegistration {
    const char *const path;
    const char *const *const keys_to_censor_in_debug_report;
    ConfigRoot *const config;
    const std::function<void(IChunkedResponse *, Ownership *, uint32_t)> callback;

    const uint8_t path_len;
    const uint8_t keys_to_censor_in_debug_report_len;

    [[gnu::always_inline]] uint8_t get_path_len()                           const { return path_len; }
    [[gnu::always_inline]] uint8_t get_keys_to_censor_in_debug_report_len() const { return keys_to_censor_in_debug_report_len; }
};

class IAPIBackend
{
public:
    virtual void addCommand(size_t commandIdx, const CommandRegistration &reg) {};
    virtual void addState(size_t stateIdx, const StateRegistration &reg) {};
    virtual void addResponse(size_t responseIdx, const ResponseRegistration &reg) {};
    virtual bool pushStateUpdate(size_t stateIdx, const String &payload, const String &path) { return true; };
    virtual bool pushRawStateUpdate(const String &payload, const String &path) { return true; };
    enum class WantsStateUpdate {
        // This backend will not send this state update, for example because there is no connection. Clear updated flag.
        No,
        // This backend will not send this state update right now, but maybe later. Don't clear updated flag.
        Later,
        AsConfig,
        AsString
    };
    virtual WantsStateUpdate wantsStateUpdate(size_t stateIdx) { return WantsStateUpdate::No;}
};

#ifdef DEBUG_FS_ENABLE
class WebServerRequest;
#endif

class API final : public IModule
{
public:
    enum class SavedDefaultConfig {
        Remove,
        Keep,
    };

    struct SuffixPath {
        std::unique_ptr<char[]> suffix;
        CoolArray<Config::Key, Config::MAX_NESTING> path;
    };

    API() = default;

    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

    // Call this method only if you are a IAPIBackend and run in another FreeRTOS task!
    String callCommand(CommandRegistration &reg, char *payload, size_t len, const Config::Key *config_path = nullptr, size_t config_path_len = 0);

    // Call this method only if you are a IAPIBackend and run in another FreeRTOS task!
    void callCommandNonBlocking(CommandRegistration &reg, const char *payload, size_t len, const std::function<void(const String &errmsg)> &done_cb, SuffixPath &&suffix_path=SuffixPath{});

    String callCommand(const char *path, const Config::ConfUpdate &payload = {});

    void callResponse(ResponseRegistration &reg, char *payload, size_t len, IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id);

    const Config *getState(const char *path, bool log_if_not_found = true, size_t path_len = 0);
    const Config *getState(const String &path, bool log_if_not_found = true);

    void addFeature(const char *name);

    // Prefer this version of addCommand over the one below.
    void addCommand(const char * const path, ConfigRoot *config, const std::vector<const char *> &keys_to_censor_in_debug_report, std::function<void(String &errmsg)> &&callback, bool is_action);
    void addCommand(const String &path,      ConfigRoot *config, const std::vector<const char *> &keys_to_censor_in_debug_report, std::function<void(String &errmsg)> &&callback, bool is_action);

    void addState(const char * const path, ConfigRoot *config, const std::vector<const char *> &keys_to_censor = {}, const std::vector<const char *> &keys_to_censor_in_debug_report = {}, bool low_latency = false);
    void addState(const String &path, ConfigRoot *config, const std::vector<const char *> &keys_to_censor = {}, const std::vector<const char *> &keys_to_censor_in_debug_report = {}, bool low_latency = false);

    bool addPersistentConfig(const String &path, ConfigRoot *config, const std::vector<const char *> &keys_to_censor = {}, const std::vector<const char *> &keys_to_censor_in_debug_report = {});
    void addResponse(const char * const path, ConfigRoot *config, const std::vector<const char *> &keys_to_censor_in_debug_report, std::function<void(IChunkedResponse *, Ownership *, uint32_t)> &&callback);

    bool hasFeature(const char *name);

    static void writeConfig(const String &path, Config *config);
    static void removeConfig(const String &path);
    static void removeAllConfig();

    static bool restorePersistentConfig(const String &path, ConfigRoot *config, SavedDefaultConfig remove_saved_default = SavedDefaultConfig::Remove);

    static String getLittleFSConfigPath(const String &path, bool tmp = false);

    size_t registerBackend(IAPIBackend *backend);

    std::span<StateRegistration> states;
    std::span<CommandRegistration> commands;
    std::span<ResponseRegistration> responses;

    // There are currently only 4 implementors of IAPIBackend:
    // event, http, mqtt and ws
    CoolArray<IAPIBackend *, 4> backends;

    ConfigRoot features;
    ConfigRoot version;

    uint8_t state_update_counter = 0;

    template<typename T>
    static bool complete_or_prefix_match(const char *in_uri, size_t in_uri_len, const T& api_reg) {
        const char *path = api_reg.path;
        size_t path_len = api_reg.get_path_len();

        // Complete match
        if (path_len == in_uri_len && memcmp(path, in_uri, path_len) == 0)
            return true;

        // Prefix match with / in URI. For example evse/state/charger_state matches the API evse/state (and should return the charger_state value only)
        if (path_len < in_uri_len && in_uri[path_len] == '/' && memcmp(path, in_uri, path_len) == 0)
            return true;

        return false;
    }

    template<typename T>
    static bool complete_match(const char *in_uri, size_t in_uri_len, const T& api_reg) {
        const char *path = api_reg.path;
        size_t path_len = api_reg.get_path_len();

        // Complete match
        if (path_len == in_uri_len && memcmp(path, in_uri, path_len) == 0)
            return true;

        return false;
    }

    static const char *build_suffix_path(SuffixPath &suffix_path, const char *suffix, size_t suffix_len);

private:
    bool already_registered(const char *path, size_t path_len, const char *api_type);

    void executeCommand(const CommandRegistration &reg, Config::ConfUpdate payload);

#ifdef DEBUG_FS_ENABLE
    bool dump_all_registrations(WebServerRequest &request, StringWriter &sw, const std::function<bool(size_t i)> &dump_registration);
#endif

    Config features_prototype;
    Config modified_prototype;

    struct RegistrationCollector {
        std::vector<StateRegistration> states;
        std::vector<CommandRegistration> commands;
        std::vector<ResponseRegistration> responses;
    };
    RegistrationCollector *reg_collector;
};
