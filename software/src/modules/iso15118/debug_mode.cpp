/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include "debug_mode.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"

#include "iso15118.h"
#include "slac.h"

#include "esp_mac.h"
#include "esp_netif.h"

void DebugMode::pre_setup()
{
    api_state = Config::Object({
        {"enable",  Config::Bool(false)},
        {"current", Config::Uint16(6000)},  // mA
        {"phases",  Config::Uint8(3)},      // 1 or 3
    });
}

void DebugMode::handle_update(Language /*language*/, String &/*error*/)
{
    const bool will_be_enabled = api_state.get("enable")->asBool();

    logger.printfln("Debug: handle_update called, will_be_enabled=%d, enabled=%d", will_be_enabled, enabled);

    if (will_be_enabled && !enabled) {
        enabled = true;
        start();
        iso15118.ensure_state_machine_running();
    } else if (!will_be_enabled && enabled) {
        enabled = false;
        stop();
    }
}

bool DebugMode::is_enabled() const
{
    return enabled;
}

ChargingInformation DebugMode::get_charging_information() const
{
    return {
        api_state.get("current")->asUint16(),
        api_state.get("phases")->asUint() == 3
    };
}

void DebugMode::start()
{
    logger.printfln("Debug: Enabling debug mode");

    // Create IPv6 link-local address on the Ethernet interface
    esp_netif_t *eth_netif = esp_netif_get_handle_from_ifkey("ETH_DEF");
    if (eth_netif != NULL) {
        esp_err_t err = esp_netif_create_ip6_linklocal(eth_netif);
        if (err == ESP_OK) {
            logger.printfln("Debug: Created IPv6 link-local address on Ethernet");
        } else if (err == ESP_ERR_INVALID_STATE) {
            logger.printfln("Debug: IPv6 link-local address already exists on Ethernet");
        } else {
            logger.printfln("Debug: Failed to create IPv6 link-local address: %d", err);
        }
    } else {
        logger.printfln("Debug: Ethernet interface not found, disabling debug mode");
        api_state.get("enable")->updateBool(false);
        enabled = false;
        return;
    }

    // Setup sockets for SDP and V2G
    iso15118.sdp.setup_socket();
    iso15118.common.setup_socket();

    // Initialize SLAC state minimally (use Ethernet MAC for EVSE)
    uint8_t eth_mac[6];
    esp_read_mac(eth_mac, ESP_MAC_ETH);
    memcpy(iso15118.slac.evse_mac, eth_mac, SLAC_MAC_ADDRESS_LENGTH);
    for (size_t i = 0; i < SLAC_MAC_ADDRESS_LENGTH; i++) {
        iso15118.slac.api_state.get("evse_mac")->get(i)->updateUint(iso15118.slac.evse_mac[i]);
    }
    iso15118.slac.api_state.get("modem_found")->updateBool(true);

    // Set SLAC state to wait for SDP (skip actual SLAC handshake)
    iso15118.slac.state = SLACState::WaitForSDP;
    iso15118.slac.api_state.get("state")->updateEnum(SLACState::WaitForSDP);

    logger.printfln("Debug: Debug mode enabled, waiting for SDP on Ethernet interface");
}

void DebugMode::stop()
{
    logger.printfln("Debug: Disabling debug mode");

    // Close sockets
    iso15118.sdp.close_socket();
    iso15118.common.close_socket();

    // Reset SLAC state
    iso15118.slac.state = SLACState::ModemInitialization;
    iso15118.slac.api_state.get("state")->updateEnum(SLACState::ModemInitialization);
    iso15118.slac.api_state.get("modem_found")->updateBool(false);

    logger.printfln("Debug: Debug mode disabled");
}
