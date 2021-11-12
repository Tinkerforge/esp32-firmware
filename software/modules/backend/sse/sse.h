/* esp32-brick
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

#include "ESPAsyncWebServer.h"

#include "api.h"
#include "config.h"

class Sse : public IAPIBackend {
public:
    Sse();
    void setup();
    void register_urls();
    void loop();

    //IAPIBackend implementation
    void addCommand(CommandRegistration reg);
    void addState(StateRegistration reg);
    void pushStateUpdate(String payload, String path);
    void wifiAvailable();

    bool initialized = false;

    AsyncEventSource events;
};
