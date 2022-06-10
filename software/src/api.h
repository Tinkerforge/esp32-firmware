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

#include "config.h"
#include "web_server.h"

struct StateRegistration {
    String path;
    ConfigRoot *config;
    std::vector<String> keys_to_censor;
    uint32_t interval;
    uint32_t last_update;
};

struct CommandRegistration {
    String path;
    ConfigRoot *config;
    std::function<void(void)> callback;
    std::vector<String> keys_to_censor_in_debug_report;
    bool is_action;
    String blockedReason;
};

struct RawCommandRegistration {
    String path;
    std::function<String(char *, size_t)> callback;
    bool is_action;
};

class IAPIBackend
{
public:
    virtual void addCommand(size_t commandIdx, const CommandRegistration &reg) = 0;
    virtual void addState(size_t stateIdx, const StateRegistration &reg) = 0;
    virtual void addRawCommand(size_t rawCommandIdx, const RawCommandRegistration &reg) = 0;
    virtual bool pushStateUpdate(size_t stateIdx, String payload, String path) = 0;
    virtual void pushRawStateUpdate(String payload, String path) = 0;
    virtual void wifiAvailable() = 0;
};

class API
{
public:
    API();

    void setup();
    void loop();

    String callCommand(String path, Config::ConfUpdate payload);

    Config *getState(String path, bool log_if_not_found = true);

    void addFeature(const char *name);
    void addCommand(String path, ConfigRoot *config, std::initializer_list<String> keys_to_censor_in_debug_report, std::function<void(void)> callback, bool is_action);
    void addState(String path, ConfigRoot *config, std::initializer_list<String> keys_to_censor, uint32_t interval_ms);
    bool addPersistentConfig(String path, ConfigRoot *config, std::initializer_list<String> keys_to_censor, uint32_t interval_ms);
    //void addTemporaryConfig(String path, Config *config, std::initializer_list<String> keys_to_censor, uint32_t interval_ms, std::function<void(void)> callback);

    void addRawCommand(String path, std::function<String(char *, size_t)> callback, bool is_action);

    bool hasFeature(const char *name);

    static void writeConfig(String path, ConfigRoot *config);

    void blockCommand(String path, String reason);
    void unblockCommand(String path);
    String getCommandBlockedReason(size_t commandIdx);

    bool restorePersistentConfig(String path, ConfigRoot *config);

    void registerDebugUrl(WebServer *server);

    void registerBackend(IAPIBackend *backend);

    void wifiAvailable();

    std::vector<StateRegistration> states;
    std::vector<CommandRegistration> commands;
    std::vector<RawCommandRegistration> raw_commands;

    std::vector<IAPIBackend *> backends;

    ConfigRoot features;
    ConfigRoot version;

private:
    bool already_registered(const String &path, const char *api_type);
};
