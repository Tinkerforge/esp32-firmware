/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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

#include "module.h"
#include "config.h"
#include "ship.h"
#include <TFJson.h>

#define DNS_SD_UUID "Tinkerforge-WARP3-12345"
#define EEBUS_PEER_FILE "/eebus/peers"
#define MAX_PEER_REMEMBERED 64 // How man ship peers configured to be remembered


class EEBus final : public IModule
{
public:
    EEBus(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;


    Config config_peers_prototype;
    ConfigRoot config;
    ConfigRoot state;
    
    Ship ship;

private:
    void cleanup_peers();
    void update_peers_config();

};
