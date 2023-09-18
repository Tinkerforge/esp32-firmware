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

#include "api.h"

#include "LittleFS.h"
#include "bindings/hal_common.h"
#include "bindings/errors.h"

#include "build.h"
#include "config_migrations.h"
#include "event_log.h"
#include "task_scheduler.h"

extern TF_HAL hal;

// Global definition here to match the declaration in api.h.
API api;

API::API() {

}

void API::pre_setup()
{
    mainTaskHandle = xTaskGetCurrentTaskHandle();

    features = Config::Array(
        {},
        new Config{Config::Str("", 0, 32)},
        0, 20, Config::type_id<Config::ConfString>()
    );

    version = Config::Object({
        {"firmware", Config::Str(build_version_full_str(), 0, strlen(build_version_full_str()))},
        {"config", Config::Str("", 0, 12)},
        {"config_type", Config::Str("", 0, 32)},
    });
}

void API::setup()
{
    migrate_config();

    String config_version;
    String config_type;
    if (LittleFS.exists("/config/version")) {
        StaticJsonDocument<JSON_OBJECT_SIZE(2) + 60> doc;
        File file = LittleFS.open("/config/version", "r");

        deserializeJson(doc, file);
        file.close();

        config_version = doc["spiffs"].as<String>();
        config_type    = doc["config_type"].as<String>();
    } else {
        logger.printfln("Failed to read config version!");
        config_version = BUILD_VERSION_STRING;
        config_type    = BUILD_CONFIG_TYPE;
    }
    logger.printfln("%s config version: %s (%s)", BUILD_DISPLAY_NAME, config_version.c_str(), config_type.c_str());
    version.get("config")->updateString(config_version);
    version.get("config_type")->updateString(config_type);

    task_scheduler.scheduleWithFixedDelay([this]() {
        bool skip_high_latency_states = state_update_counter % 4 != 0;
        ++state_update_counter;

        for (size_t state_idx = 0; state_idx < states.size(); ++state_idx) {
            auto &reg = states[state_idx];

            if (skip_high_latency_states && !reg.low_latency)
                continue;

            size_t backend_count = this->backends.size();

            uint8_t to_send = reg.config->was_updated((1 << backend_count) - 1);
            // If the config was not updated for any API, we don't have to serialize the payload.
            if (to_send == 0) {
                continue;
            }

            String payload = reg.config->to_string_except(reg.keys_to_censor);

            uint8_t sent = 0;

            for (size_t backend_idx = 0; backend_idx < this->backends.size(); ++backend_idx) {
                if ((to_send & (1 << backend_idx)) == 0)
                    continue;

                if (this->backends[backend_idx]->pushStateUpdate(state_idx, payload, reg.path))
                    sent |= 1 << backend_idx;
            }

            reg.config->clear_updated(sent);
        }
    }, 250, 250);
}

void API::addCommand(const String &path, ConfigRoot *config, std::initializer_list<String> keys_to_censor_in_debug_report, std::function<void(void)> callback, bool is_action) {
    this->addCommand(path, config, keys_to_censor_in_debug_report, [callback](String &){callback();}, is_action);
}

void API::addCommand(const String &path, ConfigRoot *config, std::initializer_list<String> keys_to_censor_in_debug_report, std::function<void(String &)> callback, bool is_action)
{
    if (already_registered(path, "command"))
        return;

    commands.push_back({path, config, callback, keys_to_censor_in_debug_report, is_action, 0, nullptr});
    auto commandIdx = commands.size() - 1;

    for (auto *backend : this->backends) {
        backend->addCommand(commandIdx, commands[commandIdx]);
    }
}

void API::addState(const String &path, ConfigRoot *config, std::initializer_list<String> keys_to_censor, bool low_latency)
{
    if (already_registered(path, "state"))
        return;

    states.push_back({path, config, keys_to_censor, low_latency});
    auto stateIdx = states.size() - 1;

    for (auto *backend : this->backends) {
        backend->addState(stateIdx, states[stateIdx]);
    }
}

bool API::addPersistentConfig(const String &path, ConfigRoot *config, std::initializer_list<String> keys_to_censor)
{
    if (path.length() > 63) {
        logger.printfln("The maximum allowed config path length is 63 bytes. Got %u bytes instead.", path.length());
        return false;
    }

    if (path[0] == '.') {
        logger.printfln("A config path may not start with a '.' as it is used to mark temporary files.");
        return false;
    }

    // It is okay to leak this: Configs cannot be deregistered.
    // The [path]_modified config has to live forever
    ConfigRoot *conf_modified = new ConfigRoot(Config::Object({
        // 0 - Config not modified since boot, config has default values (i.e. does not exist in flash)
        // 1 - Config modified since boot,     config has default values (i.e. does not exist in flash)
        // 2 - Config not modified since boot, config is changed from defaults (i.e. exists in flash)
        // 3 - Config modified since boot,     config is changed from defaults (i.e. exists in flash)
        {"modified", Config::Uint8(0)}
    }));

    {
        // If the config is written to flash, we assume that it is not the default configuration.
        // This does not have to be the case, however then we allow resetting the config once
        // before reporting that it how has the default values. This is good enough (tm).
        auto path_copy = path;
        path_copy.replace('/', '_');
        String filename = String("/config/") + path_copy;

        if (LittleFS.exists(filename)) {
            conf_modified->get("modified")->updateUint(2);
        }

        String conf_modified_path = path + "_modified";
        addState(conf_modified_path, conf_modified);

        addState(path, config, keys_to_censor);
    }

    addCommand(path + "_update", config, keys_to_censor, [path, config, conf_modified]() {
        API::writeConfig(path, config);
        conf_modified->get("modified")->updateUint(3);
    }, false);

    addCommand(path + "_reset", Config::Null(), {}, [path, conf_modified]() {
        API::removeConfig(path);
        conf_modified->get("modified")->updateUint(1);
    }, true);

    return true;
}

void API::addRawCommand(const String &path, std::function<String(char *, size_t)> callback, bool is_action)
{
    if (already_registered(path, "raw command"))
        return;

    raw_commands.push_back({path, callback, is_action});
    auto rawCommandIdx = raw_commands.size() - 1;

    for (auto *backend : this->backends) {
        backend->addRawCommand(rawCommandIdx, raw_commands[rawCommandIdx]);
    }
}

void API::callResponse(ResponseRegistration &reg, char *payload, size_t len, IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id) {
    if (this->mainTaskHandle != xTaskGetCurrentTaskHandle()) {
        logger.printfln("Don't use API::callResponse in non-main thread!");
        return;
    }

    if (!(len == 0 && reg.config->is_null())) {
        String message = reg.config->update_from_cstr(payload, len);
        if (message != "") {
            response->begin(false);
            response->write(message.c_str(), message.length());
            response->flush();
            response->end("");
            return;
        }
    }

    reg.callback(response, response_ownership, response_owner_id);
}

void API::addResponse(const String &path, ConfigRoot *config, std::initializer_list<String> keys_to_censor_in_debug_report, std::function<void(IChunkedResponse *, Ownership *, uint32_t)> callback)
{
    if (already_registered(path, "response"))
        return;

    responses.push_back({path, config, callback, keys_to_censor_in_debug_report});
    auto responseIdx = responses.size() - 1;

    for (auto *backend : this->backends) {
        backend->addResponse(responseIdx, responses[responseIdx]);
    }
}

bool API::hasFeature(const char *name)
{
    for (int i = 0; i < features.count(); ++i)
        if (features.get(i)->asString() == name)
            return true;
    return false;
}

void API::writeConfig(const String &path, ConfigRoot *config)
{
    String path_copy = path;
    path_copy.replace('/', '_');
    String cfg_path = String("/config/") + path_copy;
    String tmp_path = String("/config/.") + path_copy;

    if (LittleFS.exists(tmp_path)) {
        LittleFS.remove(tmp_path);
    }

    File file = LittleFS.open(tmp_path, "w");

    config->save_to_file(file);
    file.close();

    if (LittleFS.exists(cfg_path)) {
        LittleFS.remove(cfg_path);
    }

    LittleFS.rename(tmp_path, cfg_path);
}

void API::removeConfig(const String &path) {
    String path_copy = path;
    path_copy.replace('/', '_');
    String cfg_path = String("/config/") + path_copy;
    String tmp_path = String("/config/.") + path_copy;

    if (LittleFS.exists(tmp_path)) {
        LittleFS.remove(tmp_path);
    }

    if (LittleFS.exists(cfg_path)) {
        LittleFS.remove(cfg_path);
    }
}

void API::removeAllConfig() {
    remove_directory("/config");
}

/*
void API::addTemporaryConfig(String path, Config *config, std::initializer_list<String> keys_to_censor, std::function<void(void)> callback)
{
    addState(path, config, keys_to_censor);
    addCommand(path + "_update", config, callback);
}
*/

bool API::restorePersistentConfig(const String &path, ConfigRoot *config)
{
    String path_copy = path;
    path_copy.replace('/', '_');
    String filename = String("/config/") + path_copy;

    if (!LittleFS.exists(filename)) {
        return false;
    }

    File file = LittleFS.open(filename);
    String error = config->update_from_file(file);

    file.close();

    if (error != "") {
        logger.printfln("Failed to restore persistent config %s: %s", path_copy.c_str(), error.c_str());
    }

    return error == "";
}

void API::registerDebugUrl(WebServer *server)
{
    server->on("/debug_report", HTTP_GET, [this](WebServerRequest request) {
        String result = "{\"uptime\": ";
        result += String(millis());
        result += ",\n \"free_heap_bytes\":";
        result += heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        result += ",\n \"largest_free_heap_block\":";
        result += heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        result += ",\n \"devices\": [";

        uint16_t i = 0;
        char uid_str[7] = {0};
        char port_name;
        uint16_t device_id;

        while (tf_hal_get_device_info(&hal, i, uid_str, &port_name, &device_id) == TF_E_OK) {
            char buf[100] = {0};

            snprintf(buf, sizeof(buf), "%c{\"UID\":\"%s\", \"DID\":%u, \"port\":\"%c\"}", i == 0 ? ' ' : ',', uid_str, device_id, port_name);
            result += buf;
            ++i;
        }

        result += "]";
        result += ",\n \"error_counters\": [";

        for (char c = 'A'; c <= 'F'; ++c) {
            uint32_t spitfp_checksum, spitfp_frame, tfp_frame, tfp_unexpected;
            char buf[100] = {0};

            tf_hal_get_error_counters(&hal, c, &spitfp_checksum, &spitfp_frame, &tfp_frame, &tfp_unexpected);
            snprintf(buf, sizeof(buf), "%c{\"port\": \"%c\", \"SpiTfpChecksum\": %u, \"SpiTfpFrame\": %u, \"TfpFrame\": %u, \"TfpUnexpected\": %u}", c == 'A' ? ' ': ',', c,
                     spitfp_checksum,
                     spitfp_frame,
                     tfp_frame,
                     tfp_unexpected);

            result += buf;
        }

        result += "]";

        for (auto &reg : states) {
            result += ",\n \"";
            result += reg.path;
            result += "\": ";
            result += reg.config->to_string_except(reg.keys_to_censor);
        }

        for (auto &reg : commands) {
            result += ",\n \"";
            result += reg.path;
            result += "\": ";
            result += reg.config->to_string_except(reg.keys_to_censor_in_debug_report);
        }

        for (auto &reg : responses) {
            result += ",\n \"";
            result += reg.path;
            result += "\": ";
            result += reg.config->to_string_except(reg.keys_to_censor_in_debug_report);
        }

        result += "}";

        return request.send(200, "application/json; charset=utf-8", result.c_str());
    });

    this->addState("info/features", &features);
    this->addState("info/version", &version);
}

size_t API::registerBackend(IAPIBackend *backend)
{
    size_t backendIdx = backends.size();

    backends.push_back(backend);

    return backendIdx;
}

String API::callCommand(CommandRegistration &reg, char *payload, size_t len) {
    if (this->mainTaskHandle == xTaskGetCurrentTaskHandle()) {
        return "Use ConfUpdate overload of callCommand in main thread!";
    }

    String result = "";

    auto task_id = task_scheduler.scheduleOnce(
        [&result, reg, payload, len]() mutable {
            if (payload == nullptr && !reg.config->is_null()) {
                result = "empty payload only allowed for null configs";
                return;
            }

            if (payload != nullptr) {
                result = reg.config->update_from_cstr(payload, len);
                if (result != "")
                    return;
            }

            reg.callback(result);
        }, 0);

    if (task_scheduler.await(task_id, 10000) == TaskScheduler::AwaitResult::Timeout) {
        if (task_scheduler.cancel(task_id) == TaskScheduler::CancelResult::WillBeCancelled)
            esp_system_abort("callCommand task timed out and can't be cancelled. Giving up.");
        return "Failed to execute command: Timeout reached.";
    }

    return result;
}

void API::callCommandNonBlocking(CommandRegistration &reg, char *payload, size_t len, std::function<void(String)> done_cb) {
    if (this->mainTaskHandle == xTaskGetCurrentTaskHandle()) {
        done_cb("callCommandNonBlocking: Use ConfUpdate overload of callCommand in main thread!");
        return;
    }

    char *cpy = (char *) malloc(len);
    memcpy(cpy, payload, len);

    task_scheduler.scheduleOnce(
        [reg, cpy, len, done_cb]() mutable {
            String result;

            defer {
                done_cb(result);
                free(cpy);
            };

            if (cpy == nullptr && !reg.config->is_null()) {
                result = "empty payload only allowed for null configs";
                return;
            }

            if (cpy != nullptr) {
                result = reg.config->update_from_cstr(cpy, len);
                if (result != "") {
                    return;
                }
            }

            reg.callback(result);
        }, 0);
}

String API::callCommand(const char *path, Config::ConfUpdate payload)
{
    if (this->mainTaskHandle != xTaskGetCurrentTaskHandle()) {
        return "Use char *, size_t overload of callCommand in non-main thread!";
    }

    for (CommandRegistration &reg : commands) {
        if (reg.path != path) {
            continue;
        }

        String error = reg.config->update(&payload);

        if (error != "") {
            return error;
        }
        reg.callback(error);
        return error;
    }

    return String("Unknown command ") + path;
}

Config *API::getState(const String &path, bool log_if_not_found)
{
    for (auto &reg : states) {
        if (reg.path != path) {
            continue;
        }

        return reg.config;
    }

    if (log_if_not_found) {
        logger.printfln("Key %s not found. Contents are:", path.c_str());

        for (auto &reg : states) {
            logger.printfln("%s,", reg.path.c_str());
        }
    }

    return nullptr;
}

void API::addFeature(const char *name)
{
    for (int i = 0; i < features.count(); ++i)
        if (features.get(i)->asString() == name)
            return;

    features.add();
    features.get(features.count() - 1)->updateString(name);
}

bool API::already_registered(const String &path, const char *api_type)
{
    for (auto &reg : this->states) {
        if (reg.path != path)
            continue;
        logger.printfln("Can't register %s %s. Already registered as state!", api_type, path.c_str());
        return true;
    }
    for (auto &reg : this->commands) {
        if (reg.path != path)
            continue;
        logger.printfln("Can't register %s %s. Already registered as command!", api_type, path.c_str());
        return true;
    }
    for (auto &reg : this->raw_commands) {
        if (reg.path != path)
            continue;
        logger.printfln("Can't register %s %s. Already registered as raw command!", api_type, path.c_str());
        return true;
    }
    for (auto &reg : this->responses) {
        if (reg.path != path)
            continue;
        logger.printfln("Can't register %s %s. Already registered as response!", api_type, path.c_str());
        return true;
    }

    return false;
}
