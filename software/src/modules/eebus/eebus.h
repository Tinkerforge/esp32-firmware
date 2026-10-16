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

#include <TFJson.h>

#include "config.h"
#include "module.h"
#include "ship.h"
#include "tools/malloc.h"

#include "generated/module_available.h"
#include "eebus_config.h"
#include "generated/module_available_end.h"

// Forward declarations (so we don't have to include spine_types.h here)
class EEBusUseCases;
class SpineDataTypeHandler;

class EEBus final : public IModule
{
public:
    EEBus();
    ~EEBus();

    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

    /**
     * Toggle the eebus module according to config/enable
     */
    void toggle_module();

    /**
     * Returns the Unique Name of the device. Used for mDns, SPINE and the usecases.
     * @return The unique name of the device.
     */
    String get_eebus_name();

    Config config_peers_prototype{};
    Config state_peers_prototype{};

    ConfigRoot add_peer{};
    ConfigRoot remove_peer{};
    ConfigRoot config{};
    ConfigRoot state{};

    ConfigRoot eebus_usecase_state{};

    /**
     * Failsafe values of LPC and LPP written by the Energy Guard (LPC 2.6.2.1: SHOULD be stored persistently).
     * Not part of the API, as they should not be changeable by the user once written by the Energy Guard (LPC 2.6.2.1).
     */
    ConfigRoot failsafe_config{};
    void persist_failsafe_config();

    Config charges_prototype{};
    Config usecase_list{};

    Ship ship{};
    unique_ptr_any<EEBusUseCases> usecases{};
    //EEBusUseCases usecases{};

    // To save memory the SpineDataTypeHandler is allocated to PSRAM if its available
    unique_ptr_any<SpineDataTypeHandler> data_handler{};

    /**
     * Get the index of the connection in state.get("connections") with the given ski.
     * @param ski The ski of the connection to find.
     * @return The index of the connection in state.get("connections") or -1 if not found.
     */
    int get_state_connection_id_by_ski(const String &ski);

    size_t trace_buffer_index{};

    void trace_strln(const char *str, const size_t length);
    void trace_jsonln(JsonVariantConst data);
    [[gnu::format(__printf__, 2, 3)]] void trace_fmtln(const char *fmt, ...);

    /**
     * Update the peers configuration based on the current mDNS results, clean up invalid peers and add trusted peers to the persistent configuration.
     * Only persistent peers are saved to config to reduce EEPROM wear.
     */
    void update_peers_config();

    /**
     * Update the peers state API with all peers (both persistent and discovered).
     * This updates the runtime state, not the persistent config.
     */
    void update_peers_state();

    /**
     * Sync a single persistent peer to config (incremental update).
     * Only updates config if the peer data has actually changed.
     * @param node The peer node to sync
     */
    void sync_persistent_peer_to_config(const ShipNode *node);

    void set_own_ski(const String &ski);

    bool is_enabled() const
    {
        return module_enabled;
    }

private:
    bool module_enabled = false;
    String eebus_name{};

#if EEBUS_MODE_EVSE()
    /**
     * Register EVSE-specific event handlers.
     */
    void register_evse_events();
#endif

#if EEBUS_MODE_EM()
    /**
     * Register Energy Manager specific event handlers.
     */
    void register_em_events();
#endif

    /**
     * Register meter value events for continuous measurement updates.
     * Works in both EVSE mode (charger meter -> EVCEM/MPC) and EM mode (grid meter -> MGCP).
     */
    void register_meter_events();
};

#include "eebus_config_end.h"
