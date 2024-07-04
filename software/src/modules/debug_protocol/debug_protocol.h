/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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
#include <vector>

#include "module.h"
#include "config.h"

class IDebugProtocolBackend;

class DebugProtocol final : public IModule
{
public:
    DebugProtocol(){}
    void register_urls() override;
    void loop() override;

    void register_backend(IDebugProtocolBackend *backend);
    void check_debug();

    uint32_t last_debug_keep_alive = 0;
    bool debug = false;
    std::vector<IDebugProtocolBackend *> backends;
};
