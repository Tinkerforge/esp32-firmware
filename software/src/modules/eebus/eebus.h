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
#include "spine_connection.h"
#include "eebus_usecases.h"

#define DNS_SD_UUID "Tinkerforge-WARP3-12345"
#define EEBUS_PEER_FILE "/eebus/peers"
#define MAX_PEER_REMEMBERED 64 // How man ship peers configured to be remembered
#define SHIP_AUTODISCOVER_INTERVAL 30_s // How often to autodiscover ship peers

class EEBus final : public IModule
{
public:
    EEBus(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;


    Config config_peers_prototype;
    Config state_connections_prototype;
    ConfigRoot add_peer;
    ConfigRoot remove_peer;
    ConfigRoot config;
    ConfigRoot state;    
    Ship ship;
    EEBusUseCases usecases{};    

    SpineDataTypeHandler data_handler{};

    int get_state_connection_id_by_ski(const String &ski);
    
    bool usecase_function_call(SpineHeader &header, SpineDataTypeHandler &data);

private:
    void update_peers_config();

};
