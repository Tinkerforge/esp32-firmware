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

#include "config.h"
#include "eebus_usecases.h"
#include "module.h"
#include "config.h"
#include "ship.h"
#include "spine_connection.h"
#include <TFJson.h>
#include "tools/malloc.h"

#define EEBUS_PEER_FILE "/eebus/peers"
#define MAX_PEER_REMEMBERED 4           // How man ship peers configured to be remembered
#define SHIP_AUTODISCOVER_INTERVAL 30_s // How often to autodiscover ship peers

// EEBUS Device definitions
// These can be freely defined and are not limited by the spec
#define EEBUS_DEVICE_TYPE  "ChargingStation" // The device type as defined in EEBUS SPINE TS ResourceSpecification. Can be freely defined i


#define SUPPORTED_SPINE_VERSION "1.3.0" // The supported SPINE version for EEBus

#define EEBUS_DEV_ENABLE_RESPONSE // If defined, the EEBus device will respond to SPINE requests. Currently this is used for testing purposes only.
//#define EEBUS_SHIP_AUTOCONNECT // If defined, the EEBus device will automatically connect to discovered and trusted SHIP peers. This is currently in testing
#define EEBUS_DEV_TEST_ENABLE // Enable to test certain features that would otherwise require external hardware or an EV

class EEBus final : public IModule
{
public:
    EEBus()
    {
    }

    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    /**
     * Toggle the eebus module according to config/enable
     */
    void toggle_module();

    /**
     * Returns the Unique Name of the device. Used for mDns, SPINE and the usecases.
     * @return The unique name of the device.
     */
    String get_eebus_name();


    Config config_peers_prototype;
    Config state_connections_prototype;

    ConfigRoot add_peer;
    ConfigRoot remove_peer;
    ConfigRoot config;
    ConfigRoot state;
    ConfigRoot scan_command;

    ConfigRoot eebus_usecase_state;
    Config charges_prototype;


    Ship ship;
    unique_ptr_any<EEBusUseCases> usecases;
    //EEBusUseCases usecases{};

    // To save memory the SpineDataTypeHandler is allocated to PSRAM if its available
    unique_ptr_any<SpineDataTypeHandler> data_handler;

    /**
     * Get the index of the connection in state.get("connections") with the given ski.
     * @param ski The ski of the connection to find.
     * @return The index of the connection in state.get("connections") or -1 if not found.
     */
    int get_state_connection_id_by_ski(const String &ski);

    size_t trace_buffer_index;

    void trace_strln(const char *str, const size_t length);
    void trace_jsonln(JsonVariantConst data);
    void trace_fmtln(const char *fmt, ...);


    /**
     * Update the peers configuration based on the current mDNS results, clean up invalid peers and add trusted peers to the persistent configuration.
     */
    void update_peers_config();
};