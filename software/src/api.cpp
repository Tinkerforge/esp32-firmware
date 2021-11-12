/* esp32-lib
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

#include "SPIFFS.h"
#include "bindings/hal_common.h"
#include "bindings/errors.h"

#include "event_log.h"
#include "task_scheduler.h"

extern TF_HalContext hal;

extern TaskScheduler task_scheduler;
extern EventLog logger;

void API::setup()
{
    task_scheduler.scheduleWithFixedDelay("API state update", [this](){
        for (auto &reg: states) {
            if (!deadline_elapsed(reg.last_update + reg.interval))
                continue;

            reg.last_update = millis();

            if(!reg.config->was_updated())
                continue;

            reg.config->set_update_handled();

            String payload = reg.config->to_string_except(reg.keys_to_censor);

            for (auto* backend: this->backends) {
                backend->pushStateUpdate(payload, reg.path);
            }
        }
    }, 250, 250);
}

void API::addCommand(String path, Config *config, std::initializer_list<String> keys_to_censor_in_debug_report, std::function<void(void)> callback, bool is_action)
{
    commands.push_back({path, config, callback, keys_to_censor_in_debug_report, is_action});

    for (auto* backend: this->backends) {
        backend->addCommand(commands[commands.size() - 1]);
    }
}

void API::addState(String path, Config *config, std::initializer_list<String> keys_to_censor, uint32_t interval_ms)
{
    states.push_back({path, config, keys_to_censor, interval_ms, millis()});
    for (auto* backend: this->backends) {
        backend->addState(states[states.size() - 1]);
    }
}

bool API::addPersistentConfig(String path, Config *config, std::initializer_list<String> keys_to_censor, uint32_t interval_ms)
{
    if (path.length() > 29) {
        logger.printfln("The maximum allowed config path length is 29 bytes. Got %u bytes instead.", path.length());
        return false;
    }

    if (path[0] == '.') {
        logger.printfln("A config path may not start with a '.' as it is used to mark temporary files.");
        return false;
    }

    addState(path, config, keys_to_censor, interval_ms);
    addCommand(path + String("_update"), config, keys_to_censor, [path, config](){
        String path_copy = path;
        path_copy.replace('/', '_');
        String cfg_path = String("/") + path_copy;
        String tmp_path = String("/.") + path_copy; //max len is 31 - len("/.") = 29

        if (SPIFFS.exists(tmp_path))
            SPIFFS.remove(tmp_path);

        File file = SPIFFS.open(tmp_path, "w");
        config->save_to_file(file);
        file.close();

        if (SPIFFS.exists(cfg_path))
            SPIFFS.remove(cfg_path);

        SPIFFS.rename(tmp_path, cfg_path);
    }, false);

    return true;
}

void API::blockCommand(String path, String reason)
{
    for(auto &reg: commands) {
        if (reg.path != path)
            continue;
        reg.blockedReason = reason;
    }
}

void API::unblockCommand(String path)
{
    blockCommand(path, "");
}

String API::getCommandBlockedReason(String path)
{
    for(auto &reg: commands) {
        if (reg.path != path)
            continue;
        return reg.blockedReason;
    }
    return "";
}

/*
void API::addTemporaryConfig(String path, Config *config, std::initializer_list<String> keys_to_censor, uint32_t interval_ms, std::function<void(void)> callback)
{
    addState(path, config, keys_to_censor, interval_ms);
    addCommand(path + String("_update"), config, callback);
}
*/

bool API::restorePersistentConfig(String path, Config *config)
{
    path.replace('/', '_');
    String filename = String("/") + path;

    if(!SPIFFS.exists(filename)) {
        // We have to migrate from the old naming schema here
        // /xyz.json.tmp is now /.xyz
        // /xyz.json is now /xyz

        if(SPIFFS.exists(filename + ".json.tmp"))
            SPIFFS.remove(filename + ".json.tmp");

        if(!SPIFFS.exists(filename + ".json"))
            return false;

        logger.printfln("Migrating config file %s to %s.", (filename + ".json").c_str(), filename.c_str());
        SPIFFS.rename(filename + ".json", filename);
    }

    File file = SPIFFS.open(filename);
    String error = config->update_from_file(file);
    file.close();
    if(error != "")
        logger.printfln("Failed to restore persistent config %s: %s", path.c_str(), error.c_str());
    return error == "";
}

void API::registerDebugUrl(WebServer *server) {
    server->on("/debug_report", HTTP_GET, [this](WebServerRequest request) {

        String result = "{\"uptime\": ";
        result += String(millis());
        result += ",\n \"free_heap_bytes\":" ;
        result += ESP.getFreeHeap();
        result += ",\n \"largest_free_heap_block\":" ;
        result += ESP.getMaxAllocHeap();

        result += ",\n \"devices\": [";
        size_t i = 0;
        char uid[7] = {0};
        char pos = 0;
        uint16_t did = 0;
        while(tf_hal_get_device_info(&hal, i, uid, &pos, &did) == TF_E_OK) {
            char buf[100] = {0};
            snprintf(buf, sizeof(buf), "%c{\"UID\":\"%s\", \"DID\":%u, \"port\":\"%c\"}", i == 0 ? ' ': ',', uid, did, pos);
            result += buf;
            ++i;
        }
        result += "]";

        result += ",\n \"error_counters\": [";
        for(char c = 'A'; c <= 'F'; ++c) {
            uint32_t spitfp_checksum, spitfp_frame, tfp_frame, tfp_unexpected;

            tf_hal_get_error_counters(&hal, c, &spitfp_checksum, &spitfp_frame, &tfp_frame, &tfp_unexpected);
            char buf[100] = {0};
            snprintf(buf, sizeof(buf), "%c{\"port\": \"%c\", \"SpiTfpChecksum\": %u, \"SpiTfpFrame\": %u, \"TfpFrame\": %u, \"TfpUnexpected\": %u}", c == 'A' ? ' ': ',', c,
                spitfp_checksum,
                spitfp_frame,
                tfp_frame,
                tfp_unexpected);
            result += buf;
        }
        result += "]";

        for(auto &reg : states) {
            result += ",\n \"";
            result += reg.path;
            result += "\": ";
            result += reg.config->to_string_except(reg.keys_to_censor);
        }

        for(auto &reg: commands) {
            result += ",\n \"";
            result += reg.path;
            result += "\": ";
            result += reg.config->to_string_except(reg.keys_to_censor_in_debug_report);
        }
        result += "}";
        request.send(200, "application/json; charset=utf-8", result.c_str());
    });
}

void API::registerBackend(IAPIBackend *backend)
{
    backends.push_back(backend);
}

void API::loop()
{

}

String API::callCommand(String path, Config::ConfUpdate payload)
{
    for (CommandRegistration &reg : commands) {
        if (reg.path != path)
            continue;

        String error = reg.config->update(&payload);
        if (error == "")
            task_scheduler.scheduleOnce((String("notify command update for ") + reg.path).c_str(), [reg](){reg.callback();}, 0);
        return error;
    }
    return String("Unknown command ") + path;
}

Config* API::getState(String path, bool log_if_not_found)
{
    for (auto &reg: states) {
        if(reg.path != path)
            continue;

        return reg.config;
    }

    if(log_if_not_found) {
        logger.printfln("Key %s not found. Contents are:", path.c_str());
        for (auto &reg: states) {
            logger.printfln("%s,",reg.path.c_str());
        }
    }
    return nullptr;
}

void API::wifiAvailable()
{
    task_scheduler.scheduleOnce("wifi_available", [this](){
        for (auto* backend: this->backends) {
            backend->wifiAvailable();
        }
    }, 0);
}
