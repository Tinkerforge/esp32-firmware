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

#include <esp_task.h>
#include <LittleFS.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "options.h"
#include "bindings/hal_common.h"
#include "bindings/errors.h"
#include "config_migrations.h"
#include "tools.h"
#include "tools/fs.h"
#include "tools/malloc.h"
#include "tools/memory.h"

#include "gcc_warnings.h"

extern TF_HAL hal;

void API::pre_setup()
{
    features_prototype = Config::Str("", 0, 32);

    features = Config::Array(
        {},
        &features_prototype,
        0, 20, Config::type_id<Config::ConfString>()
    );

    version = Config::Object({
        {"firmware", Config::Str(build_version_full_str(), 0, static_cast<uint16_t>(build_version_full_str_len))},
        {"config", Config::Str("", 0, 12)},
        {"config_type", Config::Str("", 0, 32)},
    });

    modified_prototype = Config::Object({
        // 0 - Config not modified since boot, config has default values (i.e. does not exist in flash)
        // 1 - Config modified since boot,     config has default values (i.e. does not exist in flash)
        // 2 - Config not modified since boot, config is changed from defaults (i.e. exists in flash)
        // 3 - Config modified since boot,     config is changed from defaults (i.e. exists in flash)
        {"modified", Config::Uint8(0)}
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
        config_type    = OPTIONS_CONFIG_TYPE();
    }
    logger.printfln("%s config version: %s (%s)", OPTIONS_PRODUCT_NAME(), config_version.c_str(), config_type.c_str());
    version.get("config")->updateString(config_version);
    version.get("config_type")->updateString(config_type);

    task_scheduler.scheduleUncancelable([this]() {
        bool skip_high_latency_states = state_update_counter % 4 != 0;
        ++state_update_counter;

        const size_t states_count = states.size();

        for (size_t state_idx = 0; state_idx < states_count; ++state_idx) {
            const auto &reg = states[state_idx];

            if (skip_high_latency_states && !reg.get_low_latency())
                continue;

            const size_t backend_count = this->backends.size();

            uint8_t to_send = reg.config->was_updated(static_cast<uint8_t>((1 << backend_count) - 1));
            // If the config was not updated for any API, we don't have to serialize the payload.
            if (to_send == 0) {
                continue;
            }

            uint8_t sent = 0;
            String payload;

            for (size_t backend_idx = 0; backend_idx < backend_count; ++backend_idx) {
                if ((to_send & (1 << backend_idx)) == 0)
                    continue;

                switch (this->backends[backend_idx]->wantsStateUpdate(state_idx)) {
                    case IAPIBackend::WantsStateUpdate::Later:
                        break;

                    case IAPIBackend::WantsStateUpdate::No:
                        sent |= 1 << backend_idx;
                        break;


                    case IAPIBackend::WantsStateUpdate::AsString:
                        if (payload.length() == 0) {
                            // This assumes that no config can be serialized to an empty string.
                            // This will probably hold because an empty string is not valid JSON.
                            payload = reg.config->to_string_except(reg.keys_to_censor, reg.get_keys_to_censor_len());
                        }
                        [[fallthrough]];

                    case IAPIBackend::WantsStateUpdate::AsConfig:
                        if (this->backends[backend_idx]->pushStateUpdate(state_idx, payload, reg.path))
                            sent |= 1 << backend_idx;
                        break;

                    default: break;
                }
            }

            reg.config->clear_updated(sent);
        }
    }, 250_ms, 250_ms);

    reg_collector = new RegistrationCollector;
    reg_collector->states.reserve(300);
    reg_collector->commands.reserve(200);
    reg_collector->responses.reserve(4);
    this->states = std::span{reg_collector->states.data(), reg_collector->states.size()};
    this->commands = std::span{reg_collector->commands.data(), reg_collector->commands.size()};
    this->responses = std::span{reg_collector->responses.data(), reg_collector->responses.size()};

    initialized = true;
}

String API::getLittleFSConfigPath(const String &path, bool tmp) {
    String path_copy = path;
    path_copy.replace('/', '_');
    return (tmp ? String("/config/.") : String("/config/")) + path_copy;
}

void API::addCommand(const char * const path, ConfigRoot *config, const std::vector<const char *> &keys_to_censor_in_debug_report, std::function<void(String &)> &&callback, bool is_action)
{
    if (boot_stage != BootStage::REGISTER_URLS)
        esp_system_abort("Registering APIs is only allowed in register_urls!");

    size_t path_len = strlen(path);

    if (path_len > std::numeric_limits<std::result_of<decltype(&CommandRegistration::get_path_len)(CommandRegistration)>::type>::max()) {
        logger.printfln("Command %s: path too long!", path);
        return;
    }

    size_t ktc_size = keys_to_censor_in_debug_report.size();

    if (ktc_size > std::numeric_limits<std::result_of<decltype(&CommandRegistration::get_keys_to_censor_in_debug_report_len)(CommandRegistration)>::type>::max()) {
        logger.printfln("Command %s: keys_to_censor_in_debug_report too long!", path);
        return;
    }

    if (already_registered(path, path_len, "command"))
        return;

    auto ktc = ktc_size == 0 ? nullptr : perm_new_array_prefer<const char *>(ktc_size, IRAM, DRAM, _NONE);
    {
        int i = 0;
        for(const char *k : keys_to_censor_in_debug_report){
            if (!address_is_in_rodata(k))
                esp_system_abort("Key to censor not in flash! Please pass a string literal!");

            ktc[i] = k;
            ++i;
        }
    }

    uint32_t command_data = (path_len  & 0xFF) << 24
                          | (ktc_size  & 0xFF) << 16
                          | (is_action & 0xFF) <<  8;


    reg_collector->commands.push_back({
        path,
        ktc,
        config,
        std::move(callback),
        command_data
    });
    this->commands = std::span{reg_collector->commands.data(), reg_collector->commands.size()};

    const size_t commands_size = commands.size();
    const size_t commandIdx    = commands_size - 1;

    for (auto *backend : this->backends) {
        backend->addCommand(commandIdx, commands[commandIdx]);
    }

#if MODULE_DEBUG_AVAILABLE()
    debug.api_command_count(commands_size);
#endif
}

void API::addCommand(const String &path, ConfigRoot *config, const std::vector<const char *> &keys_to_censor_in_debug_report, std::function<void(String &)> &&callback, bool is_action)
{
    this->addCommand(perm_strdup(path.c_str()), config, keys_to_censor_in_debug_report, std::move(callback), is_action);
}

void API::addState(const char * const path, ConfigRoot *config, const std::vector<const char *> &keys_to_censor, const std::vector<const char *> &keys_to_censor_in_debug_report, bool low_latency)
{
    if (boot_stage != BootStage::REGISTER_URLS)
        esp_system_abort("Registering APIs is only allowed in register_urls!");

    size_t path_len = strlen(path);

    if (path_len > std::numeric_limits<std::result_of<decltype(&StateRegistration::get_path_len)(StateRegistration)>::type>::max()) {
        logger.printfln("State %s: path too long!", path);
        return;
    }

    size_t ktc_size = keys_to_censor.size();
    if (ktc_size > std::numeric_limits<std::result_of<decltype(&StateRegistration::get_keys_to_censor_len)(StateRegistration)>::type>::max()) {
        logger.printfln("State %s: keys_to_censor too long!", path);
        return;
    }

    size_t ktc_debug_size = keys_to_censor_in_debug_report.size() + ktc_size;
    if (ktc_debug_size > std::numeric_limits<std::result_of<decltype(&StateRegistration::get_keys_to_censor_in_debug_report_len)(StateRegistration)>::type>::max()) {
        logger.printfln("State %s: keys_to_censor_in_debug_report (includes keys_to_censor!) too long!", path);
        return;
    }

    if (already_registered(path, path_len, "state"))
        return;

    auto ktc = ktc_size == 0 ? nullptr : perm_new_array_prefer<const char *>(ktc_size, IRAM, DRAM, _NONE);
    {
        int i = 0;
        for(const char *k : keys_to_censor){
            if (!address_is_in_rodata(k))
                esp_system_abort("Key to censor not in flash! Please pass a string literal!");

            ktc[i] = k;
            ++i;
        }
    }

    auto ktc_debug = ktc_debug_size == 0 ? nullptr : perm_new_array_prefer<const char *>(ktc_debug_size, IRAM, DRAM, _NONE);
    {
        int i = 0;
        for(const char *k : keys_to_censor){
            if (!address_is_in_rodata(k))
                esp_system_abort("Key to censor not in flash! Please pass a string literal!");

            ktc_debug[i] = k;
            ++i;
        }
        for(const char *k : keys_to_censor_in_debug_report){
            if (!address_is_in_rodata(k))
                esp_system_abort("Key to censor not in flash! Please pass a string literal!");

            ktc_debug[i] = k;
            ++i;
        }
    }

    uint32_t state_data = (path_len       & 0xFF) << 24
                        | (ktc_size       & 0xFF) << 16
                        | (ktc_debug_size & 0xFF) <<  8
                        | (low_latency    & 0xFF) <<  0;

    reg_collector->states.push_back({
        path,
        ktc,
        ktc_debug,
        config,
        state_data
    });
    this->states = std::span{reg_collector->states.data(), reg_collector->states.size()};

    const size_t states_size = states.size();
    const size_t stateIdx    = states_size - 1;

    for (auto *backend : this->backends) {
        backend->addState(stateIdx, states[stateIdx]);
    }

#if MODULE_DEBUG_AVAILABLE()
    debug.api_state_count(states_size);
#endif
}

void API::addState(const String &path, ConfigRoot *config, const std::vector<const char *> &keys_to_censor, const std::vector<const char *> &keys_to_censor_in_debug_report, bool low_latency)
{
    this->addState(perm_strdup(path.c_str()), config, keys_to_censor, keys_to_censor_in_debug_report, low_latency);
}

bool API::addPersistentConfig(const String &path, ConfigRoot *config, const std::vector<const char *> &keys_to_censor, const std::vector<const char *> &keys_to_censor_in_debug_report)
{
    if (boot_stage != BootStage::REGISTER_URLS)
        esp_system_abort("Registering APIs is only allowed in register_urls!");

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
    ConfigRoot *conf_modified = perm_new<ConfigRoot>(RAM::DRAM, modified_prototype);

    {
        // If the config is written to flash, we assume that it is not the default configuration.
        // This does not have to be the case, however then we allow resetting the config once
        // before reporting that it how has the default values. This is good enough (tm).
        String filename = API::getLittleFSConfigPath(path);

        if (LittleFS.exists(filename)) {
            conf_modified->get("modified")->updateUint(2);
        }

        String conf_modified_path = path + "_modified";
        addState(conf_modified_path, conf_modified);
    }

    addState(path, config, keys_to_censor, keys_to_censor_in_debug_report);

    std::vector<const char *> ktc;
    ktc.reserve(keys_to_censor.size() + keys_to_censor_in_debug_report.size());
    for (const char *k : keys_to_censor) {
        ktc.push_back(k);
    }
    for (const char *k : keys_to_censor_in_debug_report) {
        ktc.push_back(k);
    }

    addCommand(path + "_update", config, ktc, [path, config, conf_modified](String &/*errmsg*/) {
        API::writeConfig(path, config);
        conf_modified->get("modified")->updateUint(3);
    }, false);

    addCommand(path + "_reset", Config::Null(), {}, [path, conf_modified](String &/*errmsg*/) {
        API::removeConfig(path);
        conf_modified->get("modified")->updateUint(1);
    }, true);

    return true;
}

void API::callResponse(ResponseRegistration &reg, char *payload, size_t len, IChunkedResponse *response, Ownership *response_ownership, uint32_t response_owner_id) {
    if (!running_in_main_task()) {
        logger.printfln("Don't use API::callResponse in non-main thread!");
        return;
    }

    if (len != 0 || !reg.config->is_null()) {
        String message = reg.config->update_from_cstr(payload, len);

        if (!message.isEmpty()) {
            OwnershipGuard ownership_guard(response_ownership, response_owner_id);

            if (!ownership_guard.have_ownership()) {
                return;
            }

            response->begin(false);
            response->write(message.c_str(), message.length());
            response->flush();
            response->end("");

            return;
        }
    }

    reg.callback(response, response_ownership, response_owner_id);
}

void API::addResponse(const char * const path, ConfigRoot *config, const std::vector<const char *> &keys_to_censor_in_debug_report, std::function<void(IChunkedResponse *, Ownership *, uint32_t)> &&callback)
{
    if (boot_stage != BootStage::REGISTER_URLS)
        esp_system_abort("Registering APIs is only allowed in register_urls!");

    size_t path_len = strlen(path);

    if (path_len > std::numeric_limits<decltype(ResponseRegistration::path_len)>::max()) {
        logger.printfln("Response %s: path too long!", path);
        return;
    }

    size_t ktc_size = keys_to_censor_in_debug_report.size();
    if (ktc_size > std::numeric_limits<decltype(ResponseRegistration::keys_to_censor_in_debug_report_len)>::max()) {
        logger.printfln("State %s: keys_to_censor_in_debug_report too long!", path);
        return;
    }

    if (already_registered(path, path_len, "response"))
        return;

    auto ktc = ktc_size == 0 ? nullptr : perm_new_array_prefer<const char *>(ktc_size, IRAM, DRAM, _NONE);
    {
        int i = 0;
        for(const char *k : keys_to_censor_in_debug_report){
            if (!address_is_in_rodata(k))
                esp_system_abort("Key to censor not in flash! Please pass a string literal!");

            ktc[i] = k;
            ++i;
        }
    }

    reg_collector->responses.push_back({
        path,
        ktc,
        config,
        std::move(callback),
        static_cast<uint8_t>(path_len),
        static_cast<uint8_t>(ktc_size)
    });
    this->responses = std::span{reg_collector->responses.data(), reg_collector->responses.size()};

    const size_t responses_size = responses.size();
    const size_t responseIdx    = responses_size - 1;

    for (auto *backend : this->backends) {
        backend->addResponse(responseIdx, responses[responseIdx]);
    }

#if MODULE_DEBUG_AVAILABLE()
    debug.api_response_count(responses_size);
#endif
}

bool API::hasFeature(const char *name)
{
    for (size_t i = 0; i < features.count(); ++i)
        if (features.get(i)->asString() == name)
            return true;
    return false;
}

void API::writeConfig(const String &path, Config *config)
{
    String cfg_path = API::getLittleFSConfigPath(path);
    String tmp_path = API::getLittleFSConfigPath(path, true);

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

void API::removeConfig(const String &path)
{
    String cfg_path = API::getLittleFSConfigPath(path);
    String tmp_path = API::getLittleFSConfigPath(path, true);

    if (LittleFS.exists(tmp_path)) {
        LittleFS.remove(tmp_path);
    }

    if (LittleFS.exists(cfg_path)) {
        LittleFS.remove(cfg_path);
    }
}

void API::removeAllConfig()
{
    remove_directory("/config");
}

/*
void API::addTemporaryConfig(String path, Config *config, const std::vector<const char *> &keys_to_censor, std::function<void(String &)> &&callback)
{
    addState(path, config, keys_to_censor);
    addCommand(path + "_update", config, std::move(callback));
}
*/

bool API::restorePersistentConfig(const String &path, ConfigRoot *config, SavedDefaultConfig remove_saved_default)
{
    String filename = API::getLittleFSConfigPath(path);

    if (!LittleFS.exists(filename)) {
        return false;
    }

    // Save previous updated state
    const auto updated = config->value.updated;
    config->value.updated = 0;

    const String error = config->update_from_file(LittleFS.open(filename));
    const bool restore_ok = error.isEmpty();

    // If the file load didn't update anything, the file's content matches the default config.
    // Remove the saved file in that case.
    // Don't remove configs that could not be validated to not remove a user's data.
    if (restore_ok && config->value.updated == 0 && remove_saved_default == SavedDefaultConfig::Remove) {
        removeConfig(path);
    }

    // Include previous updated state
    config->value.updated |= updated;

    if (!restore_ok) {
        logger.printfln("Failed to restore persistent config %s: %s", path.c_str(), error.c_str());
    }

    return restore_ok;
}

#ifdef DEBUG_FS_ENABLE
bool API::dump_all_registrations(WebServerRequest &request, StringWriter &sw, const std::function<bool(size_t i)> &dump_registration)
{
    for (size_t i = 0;; i++) {
        bool produced_output;

        auto result = task_scheduler.await([&dump_registration, i, &produced_output]() {
            produced_output = dump_registration(i);
        });

        if (result != TaskScheduler::AwaitResult::Done) {
            return false;
        }

        if (!produced_output) {
            return true;
        }

        if (sw.getRemainingLength() == 0) {
            esp_system_abort("API info buffer was too small!");
        }

        request.sendChunk(sw);

        sw.clear();
        sw.puts(",\n");
    }
}
#endif

// TODO: Merge this function and the very similar implementation of the first websocket frame in ws.cpp
template<typename T>
static bool print_regs_to_debug_report(std::span<T> regs, StringWriter &sw, WebServerRequest &request, const char *terminator) { // Dump states
    constexpr const char *prefix = ",\n \"";
    constexpr const char *infix = "\": ";
    constexpr const char *suffix = "";
    constexpr size_t prefix_len = strlen(prefix);
    constexpr size_t infix_len = strlen(infix);
    constexpr size_t suffix_len = strlen(suffix);
    size_t terminator_len = terminator == nullptr ? 0 : strlen(terminator);

    size_t i = 0;
    bool done = false;

    while (!done) {
        auto result = task_scheduler.await([regs, &i, &done, &sw, terminator_len]() {
            for (; i < regs.size(); ++i) {
                auto &reg = regs[i];
                auto path = reg.path;
                auto path_len = reg.get_path_len();
                auto config_len = reg.config->string_length();
                size_t req = prefix_len + path_len + infix_len + config_len + suffix_len + terminator_len;

                if (sw.getRemainingLength() < req) {
                    done = false;

                    if (req > sw.getCapacity()) {
                        logger.printfln("API %s exceeds max debug report buffer capacity! Required %u, buffer capacity %u", path, req, sw.getCapacity());
                    }

                    return;
                }

                sw.puts(prefix, prefix_len);
                sw.puts(path, path_len);
                sw.puts(infix, infix_len);

                reg.config->to_string_except(reg.keys_to_censor_in_debug_report, reg.get_keys_to_censor_in_debug_report_len(), &sw);

                sw.puts(suffix, suffix_len);
            }

            done = true;
        });

        if (result != TaskScheduler::AwaitResult::Done) {
            request.sendChunk("Failed to generate debug report: task timed out");
            request.endChunkedResponse();
            return false;
        }

        if (!done && sw.getLength() == 0) {
            request.sendChunk("Failed to generate debug report: truncated");
            request.endChunkedResponse();
            return false;
        }

        if (terminator != nullptr)
            sw.puts(terminator, terminator_len);

        if (request.sendChunk(sw.getPtr(), sw.getLength()) != ESP_OK) {
            request.endChunkedResponse();
            return false;
        }

        sw.clear();
    }
    return true;
}


void API::register_urls()
{
#ifdef DEBUG_FS_ENABLE
    server.on_HTTPThread("/api_info", HTTP_GET, [this](WebServerRequest request) {

        request.beginChunkedResponse_json(200);

        // meter/X/config can be > 16k because of the Modbus register table unions.
        constexpr size_t BUF_SIZE = 32768;

        auto buf = heap_alloc_array<char>(BUF_SIZE);
        StringWriter sw{buf.get(), BUF_SIZE};

        sw.putc('[');
        request.sendChunk(sw);
        sw.clear();

        bool success = dump_all_registrations(request, sw, [this, &sw](size_t i) -> bool {
            // There could be 0 states registered.
            if (i >= states.size()) {
                return false;
            }

            const auto &reg = states[i];

            sw.printf("{\"path\":\"%.*s\",\"type\":\"state\",\"low_latency\":%s,\"keys_to_censor\":[",
                                reg.get_path_len(), reg.path,
                                reg.get_low_latency() ? "true" : "false");

            for (int key = 0; key < reg.get_keys_to_censor_len(); ++key)
                if (key != reg.get_keys_to_censor_len() - 1)
                    sw.printf("\"%s\",", reg.keys_to_censor[key]);
                else
                    sw.printf("\"%s\"", reg.keys_to_censor[key]);

            sw.puts("],\"content\":");
            reg.config->print_api_info(sw);
            sw.putc('}');

            return true;
        });

        if (!success) {
            return request.endChunkedResponse();
        }

        success = dump_all_registrations(request, sw, [this, &sw](size_t i) -> bool {
            // There could be 0 commands registered.
            if (i >= commands.size()) {
                return false;
            }

            const auto &reg = commands[i];

            sw.printf("{\"path\":\"%.*s\",\"type\":\"command\",\"is_action\":%s,\"keys_to_censor\":[",
                            reg.get_path_len(), reg.path,
                            reg.get_is_action() ? "true" : "false");

            for (int key = 0; key < reg.get_keys_to_censor_in_debug_report_len(); ++key)
                if (key != reg.get_keys_to_censor_in_debug_report_len() - 1)
                    sw.printf("\"%s\",", reg.keys_to_censor_in_debug_report[key]);
                else
                    sw.printf("\"%s\"", reg.keys_to_censor_in_debug_report[key]);

            sw.puts("],\"content\":");
            reg.config->print_api_info(sw);
            sw.putc('}');

            return true;
        });

        if (!success) {
            return request.endChunkedResponse();
        }

        success = dump_all_registrations(request, sw, [this, &sw](size_t i) -> bool {
            // There could be 0 responses registered.
            if (i >= responses.size()) {
                return false;
            }

            const auto &reg = responses[i];

            sw.printf("{\"path\":\"%.*s\",\"type\":\"response\",\"keys_to_censor\":[",
                            reg.path_len, reg.path);

            for (int key = 0; key < reg.keys_to_censor_in_debug_report_len; ++key)
                if (key != reg.keys_to_censor_in_debug_report_len - 1)
                    sw.printf("\"%s\",", reg.keys_to_censor_in_debug_report[key]);
                else
                    sw.printf("\"%s\"", reg.keys_to_censor_in_debug_report[key]);

            sw.puts("],\"content\":");
            reg.config->print_api_info(sw);
            sw.putc('}');

            return true;
        });

        if (!success) {
            return request.endChunkedResponse();
        }

        WebServerHandler *handlers;
        WebServerHandler *wildcard_handlers;
        server.get_handlers(&handlers, &wildcard_handlers);
        bool non_wildcards_done = false;

        // The HTTP handlers belong to the HTTP task and can be accessed without going through the main task.
        while (true) {
            if (handlers == nullptr) {
                if (!non_wildcards_done) {
                    non_wildcards_done = true;
                    handlers = wildcard_handlers;
                    continue; // Check again, in case wildcard_handlers is a nullptr.
                } else {
                    // Non-wildcards and wildcards done.
                    break;
                }
            }

            sw.printf("{\"path\":\"%s\",\"type\":\"http_only\",\"method\":%d,\"accepts_upload\":%s}",
                handlers->uri + 1, handlers->method, handlers->accepts_upload ? "true" : "false"); // +1 to skip first /

            if (sw.getRemainingLength() == 0) {
                esp_system_abort("API info buffer was too small!");
            }

            request.sendChunk(sw);

            sw.clear();
            sw.puts(",\n");

            handlers = handlers->next;
        }

        sw.clear();     // Discard final comma and newline.
        sw.puts("]\n"); // Be nice and end the file with a newline.
        request.sendChunk(sw);

        return request.endChunkedResponse();
    });
#endif

    server.on_HTTPThread("/debug_report", HTTP_GET, [this](WebServerRequest request) {
        constexpr size_t BUF_SIZE = OPTIONS_API_JSON_MAX_LENGTH() + 1024;
        auto buf = heap_alloc_array<char>(BUF_SIZE);

        if (buf == nullptr)
            return request.send_plain(500, "Failed to allocate buffer");

        {
            TFJsonSerializer json{buf.get(), BUF_SIZE};
            json.addObject();
            json.addMemberNumber("uptime", now_us().to<millis_t>().as<int64_t>());

            constexpr struct {
                const char *name;
                uint32_t caps;
            } ram_info[3] = {
                {"dram", MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT},
                {"iram", MALLOC_CAP_IRAM},
                {"psram", MALLOC_CAP_SPIRAM},
            };

            for (size_t i = 0; i < std::size(ram_info); ++i) {
                multi_heap_info_t info;
                heap_caps_get_info(&info, ram_info[i].caps);

                json.addMemberObject(ram_info[i].name);
                json.addMemberNumber("free", static_cast<uint32_t>(info.total_free_bytes));
                json.addMemberNumber("largest_free_block", static_cast<uint32_t>(info.largest_free_block));
                json.addMemberNumber("min_free", static_cast<uint32_t>(info.minimum_free_bytes));
                json.endObject();
            }

            task_scheduler.await([&json](){
                uint16_t i = 0;
                char uid_str[7] = {0};
                // We need a string below. tf_hal_get_device_info will only write the first char.
                char port_name[2] = {0, 0};
                uint16_t device_id;

                json.addMemberArray("devices");

                while (tf_hal_get_device_info(&hal, i, uid_str, port_name, &device_id) == TF_E_OK) {
                    json.addObject();
                    json.addMemberString("UID", uid_str);
                    json.addMemberNumber("DID", device_id);
                    json.addMemberString("port", port_name);
                    json.endObject();

                    ++i;
                }

                json.endArray();
            });

            task_scheduler.await([&json](){
                json.addMemberArray("error_counters");
                for (char c = 'A'; c <= 'F'; ++c) {
                    uint32_t spitfp_checksum, spitfp_frame, tfp_frame, tfp_unexpected;

                    tf_hal_get_error_counters(&hal, c, &spitfp_checksum, &spitfp_frame, &tfp_frame, &tfp_unexpected);

                    // We need a string below.
                    char port_string[2] = {c, 0};

                    json.addObject();
                    json.addMemberString("port", port_string);
                    json.addMemberNumber("SpiTfpChecksum", spitfp_checksum);
                    json.addMemberNumber("SpiTfpFrame", spitfp_frame);
                    json.addMemberNumber("TfpFrame", tfp_frame);
                    json.addMemberNumber("TfpUnexpected", tfp_unexpected);
                    json.endObject();
                }

                json.endArray();
            });

            // Don't end the root object: APIs will be added below
            // json.endObject();
            size_t to_send = json.end();
            request.beginChunkedResponse_json(200);
            request.sendChunk(buf.get(), to_send);
        } // Drop JsonSerializer to prevent accidentially using it below.

        StringWriter sw{buf.get(), BUF_SIZE};

        print_regs_to_debug_report(this->states, sw, request, nullptr);
        print_regs_to_debug_report(this->commands, sw, request, nullptr);
        print_regs_to_debug_report(this->responses, sw, request, "}");

        return request.endChunkedResponse();
    });

    this->addState("info/features", &features);
    this->addState("info/version", &version);
}

void API::register_events() {
    this->states = make_permanent(std::move(reg_collector->states), IRAM);
    this->commands = make_permanent(std::move(reg_collector->commands), IRAM);
    this->responses = make_permanent(std::move(reg_collector->responses), DRAM);

    delete reg_collector;
    reg_collector = nullptr;
}

size_t API::registerBackend(IAPIBackend *backend)
{
    size_t backendIdx = backends.size();

    backends.push_back(backend);

    return backendIdx;
}

String API::callCommand(CommandRegistration &reg, char *payload, size_t len, const Config::Key *config_path, size_t config_path_len)
{
    if (running_in_main_task()) {
        return "Use ConfUpdate overload of callCommand in main thread!";
    }

    String result;

    auto await_result = task_scheduler.await(
        [&result, reg, payload, len, config_path, config_path_len]() mutable {
            if (payload == nullptr && !reg.config->is_null()) {
                result = "empty payload only allowed for null configs";
                return;
            }

            if (payload != nullptr) {
                result = reg.config->update_from_cstr(payload, len, config_path, config_path_len);
                if (!result.isEmpty())
                    return;
            } else if (config_path_len > 0) {
                result = "suffix not allowed for null configs";
            }

            reg.callback(result);
        });

    if (await_result == TaskScheduler::AwaitResult::Timeout) {
        const char *task_name = pcTaskGetName(xTaskGetCurrentTaskHandle());
        logger.printfln("callCommand timed out. This may affect the stack of task '%s'.", task_name);

        return "Failed to execute command: Timeout reached.";
    }

    return result;
}

void API::callCommandNonBlocking(CommandRegistration &reg, const char *payload, size_t len, const std::function<void(const String &errmsg)> &done_cb, API::SuffixPath &&suffix_path)
{
    if (running_in_main_task()) {
        String err = "callCommandNonBlocking: Use ConfUpdate overload of callCommand in main thread!";
        done_cb(err);
        return;
    }

    char *cpy = static_cast<char *>(malloc(len));
    if (cpy == nullptr) {
        String err = "callCommandNonBlocking: Failed to allocate payload copy!";
        done_cb(err);
        return;
    }
    memcpy(cpy, payload, len);

    // We use C++ >= 14, so suffix_path could be moved into the lambda capture list.
    // However std::function requires that all captured variables can be copy-constructed,
    // so we *still* have to do this by hand.
    char *suffix_ptr = suffix_path.suffix.release();

    task_scheduler.scheduleOnce(
        [reg, cpy, len, done_cb, suffix_ptr, path = suffix_path.path]() mutable {
            String result;

            defer {
                done_cb(result);
                free(cpy);
                delete[] suffix_ptr;
            };

            if ((cpy == nullptr || len == 0) && !reg.config->is_null()) {
                result = "empty payload only allowed for null configs";
                return;
            }

            if (cpy != nullptr) {
                result = reg.config->update_from_cstr(cpy, len, path.data(), path.size());
                if (!result.isEmpty()) {
                    return;
                }
            }

            reg.callback(result);
        });
}

String API::callCommand(const char *path, const Config::ConfUpdate &payload)
{
    if (!running_in_main_task()) {
        return "Use char *, size_t overload of callCommand in non-main thread!";
    }

    CommandRegistration *reg = nullptr;

    // If the called path is in rodata, try a quick address check first.
    if (address_is_in_rodata(path)) {
        for (CommandRegistration &chk_reg : commands) {
            if (chk_reg.path == path) { // Address comparison
                reg = &chk_reg;
                break;
            }
        }
    }

    // If the address check didn't find a match, try string comparison instead.
    if (!reg) {
        for (CommandRegistration &chk_reg : commands) {
            if (strcmp(chk_reg.path, path) == 0) {
                reg = &chk_reg;
                break;
            }
        }

        if (!reg) {
            return StringSumHelper("Unknown command: ") + path;
        }
    }

    String error = reg->config->update(&payload);

    if (!error.isEmpty()) {
        return error;
    }

    reg->callback(error);

    return error;
}

const Config *API::getState(const char *path, bool log_if_not_found, size_t path_len)
{
    if (!path) {
        return nullptr;
    }

    // If the requested path is in rodata, try a quick address check first.
    if (address_is_in_rodata(path)) {
        for (const auto &reg : states) {
            if (path == reg.path) { // Address check
                return reg.config;
            }
        }
    }

    if (!path_len) {
        path_len = strlen(path);
    }

    for (const auto &reg : states) {
        if (path_len != reg.get_path_len() || strcmp(path, reg.path) != 0) {
            continue;
        }

        return reg.config;
    }

    if (log_if_not_found) {
        logger.printfln("State %s not found. Known states are:", path);

        for (const auto &reg : states) {
            logger.printfln_continue("%s,", reg.path);
        }
    }

    return nullptr;
}

const Config *API::getState(const String &path, bool log_if_not_found)
{
    return getState(path.c_str(), log_if_not_found, path.length());
}

void API::addFeature(const char *name)
{
    size_t feature_count = features.count();
    for (size_t i = 0; i < feature_count; ++i)
        if (features.get(i)->asString() == name)
            return;

    auto new_feature = features.add();
    new_feature->updateString(name);
}

bool API::already_registered(const char *path, size_t path_len, const char *api_type)
{
    for (auto &reg : this->states) {
        if (path_len != reg.get_path_len() || memcmp(path, reg.path, path_len) != 0)
            continue;
        logger.printfln("Can't register %s %s. Already registered as state!", api_type, path);
        return true;
    }
    for (auto &reg : this->commands) {
        if (path_len != reg.get_path_len() || memcmp(path, reg.path, path_len) != 0)
            continue;
        logger.printfln("Can't register %s %s. Already registered as command!", api_type, path);
        return true;
    }
    for (auto &reg : this->responses) {
        if (path_len != reg.path_len || memcmp(path, reg.path, path_len) != 0)
            continue;
        logger.printfln("Can't register %s %s. Already registered as response!", api_type, path);
        return true;
    }

    return false;
}

const char *API::build_suffix_path(SuffixPath &suffix_path, const char *suffix, size_t suffix_len) {
    // If len == 1 the suffix must be / or else it would not have been matched against this API. / points to the complete API -> no need to walk
    if (suffix_len <= 1) {
        suffix_path.suffix = nullptr;
        return nullptr;
    }

    // Copy suffix into suffix_path, has to have the same lifetime.
    suffix_path.suffix = heap_alloc_array<char>(suffix_len + 1);
    memcpy(suffix_path.suffix.get(), suffix, suffix_len);
    suffix_path.suffix[suffix_len] = '\0';

    // Skip first /
    char *ptr = suffix_path.suffix.get();
    ++ptr;

    while (true) {
        size_t next_len = 0;
        bool is_number = true;
        while(ptr[next_len] != '\0' && ptr[next_len] != '/') {
            is_number &= ptr[next_len] >= '0' && ptr[next_len] <= '9';
            ++next_len;
        }

        if (next_len == 0) {
            return "path may not contain // or end on a /";
        }

        if (is_number) {
            if (!suffix_path.path.add_checked(static_cast<size_t>(strtoul(ptr, nullptr, 10)))) {
                return "path too long";
            }
        } else {
            if (!suffix_path.path.add_checked(ptr)) {
                return "path too long";
            }
        }

        if (ptr[next_len] == '\0')
            break;

        ptr[next_len] = '\0';
        ptr += next_len + 1;
    }
    return nullptr;
}
