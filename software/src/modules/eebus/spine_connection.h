
/* esp32-firmware
 * Copyright (C) 2025 Julius Dill <julius@tinkerforge.com>
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

#include "build.h"
#include <ArduinoJson.h>
#include <esp_http_client.h>

#include "config.h"
#include "module.h"

#define SPINE_CONNECTION_MAX_JSON_SIZE 8192 // TODO: What is a sane value here?
#define SPINE_CONNECTION_MAX_DEPTH 30 // Maximum depth of serialization of a json document

/*
struct SpineDatagramHeader
{
    
};

*/
class SpineConnection
{

public:
    SpineConnection();

    void process_datagram(String datagram);

    // SPINE TS 5.2.3.1
    // Specification recommends these be stored in non-volatile memory

    uint64_t msg_counter = 0;
    uint64_t msg_counter_received = 0;
};