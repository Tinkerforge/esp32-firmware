/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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

// The ISO15118 class is the coordinator for all of the different versions and layers of the ISO 15118 standard.
// It coordinates between the following protocols:
// * SPI/QCA700x (Low level SPI communication with the QCA700x modem),
// * SLAC (Signal-Level-Attenuation-Characterization) as described in ISO 15118-3,
// * IPv6/ICMPv6/NDP (Neighbor Discorver Protoocol with Neighbor Solicitation/Advertisement),
// * IPv6/UDP/SDP (SECC Discovery Protocol),
// * IPv6/TCP/DIN-SPEC-70121,
// * IPv6/TCP/ISO-15118-2,
// * IPv6/TCP/ISO-15118-20
// and the EVSE that runs on a different board (the EVSE Bricklet).
// The EXI message encoding/decoding is based on libcbv2g.
// The objective is to *robustly* be able to
// * fingerprint the EV for authentication,
// * read SOC from the EV and
// * if it will be implemented by EVs, allow for bi-directional charging.
// To read the necessary data we may start a DC charge and end it again immediatily or similar, depending on the standards that the EV supports.
// The actual charging will mostly still be done through IEC 61851-1 (PWM on CP).

#include "iso15118.h"

#include <time.h>
#include <type_traits>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"

#include "tools/hexdump.h"
#include "tools/printf.h"

#include "qca700x.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "bindings/bricklet_evse_v2.h"


void ISO15118::trace_packet(const uint8_t *packet, const size_t packet_size)
{
#if defined(BOARD_HAS_PSRAM)
    uint32_t secs = now_us().to<millis_t>().as<uint32_t>();
    char timestamp[16];
    size_t len = snprintf_u(timestamp, sizeof(timestamp), "%lu ", secs);
    logger.trace_plain(trace_buffer_index_ll, timestamp, len);

    // Hexdump the packet in one go (2 hex chars per byte + null terminator)
    size_t hex_buf_size = packet_size * 2 + 1;
    char hex_buf[hex_buf_size];
    len = hexdump(packet, packet_size, hex_buf, hex_buf_size, HexdumpCase::Lower);
    logger.trace_plain(trace_buffer_index_ll, hex_buf, len);
    logger.trace_plain(trace_buffer_index_ll, "\n", 1);
#endif
}

void ISO15118::trace(const char *fmt, ...)
{
#if defined(BOARD_HAS_PSRAM)
    uint32_t secs = now_us().to<millis_t>().as<uint32_t>();
    char buffer[32];
    size_t len = snprintf_u(buffer, sizeof(buffer), "%04lu ", secs % 10000);
    logger.trace_plain(trace_buffer_index, buffer, len);

    va_list args;
    va_start(args, fmt);
    logger.vtracefln_plain(trace_buffer_index, fmt, args);
    va_end(args);
#endif
}

void ISO15118::pre_setup()
{
    this->trace_buffer_index    = logger.alloc_trace_buffer("iso15118", 1 << 18);
    this->trace_buffer_index_ll = logger.alloc_trace_buffer("iso15118_ll", 1 << 18);

    config = ConfigRoot{Config::Object({
        {"autocharge", Config::Bool(false)},
        {"read_soc", Config::Bool(true)},
        {"charge_via_iso15118", Config::Bool(false)},
        {"min_charge_current", Config::Uint16(1000)},
    }), [this](Config &update, ConfigSource source) -> String {
        const bool was_enabled = is_enabled();
        const bool will_be_enabled = update.get("autocharge")->asBool() ||
                                     update.get("read_soc")->asBool() ||
                                     update.get("charge_via_iso15118")->asBool();

        if (will_be_enabled != was_enabled) {
            if (will_be_enabled) {
                if (!is_setup) {
                    task_scheduler.scheduleOnce([this]() {
                        sdp.setup_socket();
                        common.setup_socket();
                        is_setup = true;
                    });
                }

                if (state_machine_task == 0) {
                    state_machine_task = task_scheduler.scheduleWithFixedDelay([this]() {
                        this->state_machines_loop();
                    }, 20_ms, 20_ms);
                }

                // TODO: Check if charge is currently ongoing:
                //       If IEC 61851 charge is ongoing, we should only change the protocol after the charge is done.
                //       If no charge is ongoing, we can change the protocol immediately.
                //       If the EVSE Bricklet is already in ISO 15118 mode, we can continue with the state it is already in.
                evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_ISO15118, 50);
            } else {
                // TODO: Close sockets and set is_setup = false
                task_scheduler.cancel(state_machine_task);
                state_machine_task = 0;

                // TODO: Check if charge is currently ongoing:
                //       If IEC 61851 charge is ongoing, we should only change the protocol after the charge is done.
                //       If no charge is ongoing, we can change the protocol immediately.
                //       If the EVSE Bricklet is already in IEC 61851 mode, we can continue with the state it is already in.
                evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_IEC61851_PERMANENT, 1000);
            }
        }
        return "";
    }};

    slac.pre_setup();
    sdp.pre_setup();
    common.pre_setup();
    din70121.pre_setup();
    iso2.pre_setup();
    iso20.pre_setup();
}

void ISO15118::setup()
{
    // Initialize poll file descriptors array
    for (int i = 0; i < FDS_COUNT; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }

    api.restorePersistentConfig("iso15118/config", &config);

    initialized = true;

    if (is_enabled()) {
        sdp.setup_socket();
        common.setup_socket();
        is_setup = true;
    } else {
        is_setup = false;
    }
}

void ISO15118::register_urls()
{
    api.addPersistentConfig("iso15118/config", &config);
    api.addState("iso15118/state_slac",     &slac.api_state);
    api.addState("iso15118/state_sdp",      &sdp.api_state);
    api.addState("iso15118/state_common",   &common.api_state);
    api.addState("iso15118/state_din70121", &din70121.api_state);
    api.addState("iso15118/state_iso2",     &iso2.api_state);
    api.addState("iso15118/state_iso20",    &iso20.api_state);

    // Command to enable debug mode (use default Ethernet interface instead of QCA/PLC)
    api.addCommand("iso15118/debug_start", Config::Null(), {}, [this](Language /*language*/, String &error) {
        logger.printfln("Debug: Enabling debug mode");

        debug_mode = true;

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
            error = "Ethernet interface not found";
            debug_mode = false;
            return;
        }

        // Setup sockets for SDP and V2G
        sdp.setup_socket();
        common.setup_socket();

        // Initialize SLAC state minimally (use Ethernet MAC for EVSE)
        uint8_t eth_mac[6];
        esp_read_mac(eth_mac, ESP_MAC_ETH);
        memcpy(slac.evse_mac, eth_mac, SLAC_MAC_ADDRESS_LENGTH);
        for (size_t i = 0; i < SLAC_MAC_ADDRESS_LENGTH; i++) {
            slac.api_state.get("evse_mac")->get(i)->updateUint(slac.evse_mac[i]);
        }
        slac.api_state.get("modem_found")->updateBool(true);

        // Set SLAC state to wait for SDP (skip actual SLAC handshake)
        slac.state = SLAC::State::WaitForSDP;
        slac.api_state.get("state")->updateUint(static_cast<uint8_t>(SLAC::State::WaitForSDP));

        // Make sure the state machine is running
        if (state_machine_task == 0) {
            state_machine_task = task_scheduler.scheduleWithFixedDelay([this]() {
                this->state_machines_loop();
            }, 20_ms, 20_ms);
        }

        is_setup = true;
        logger.printfln("Debug: Debug mode enabled, waiting for SDP on Ethernet interface");
    }, true);

    // Command to disable debug mode
    // Close sockets and reset SLAC state, so we can start from scratch with a real PLC connection
    api.addCommand("iso15118/debug_stop", Config::Null(), {}, [this](Language /*language*/, String &error) {
        logger.printfln("Debug: Disabling debug mode");
        debug_mode = false;

        // Close sockets
        sdp.close_socket();
        common.close_socket();

        // Reset SLAC state
        slac.state = SLAC::State::ModemInitialization;
        slac.api_state.get("state")->updateUint(static_cast<uint8_t>(SLAC::State::ModemInitialization));
        slac.api_state.get("modem_found")->updateBool(false);

        logger.printfln("Debug: Debug mode disabled");
    }, true);

    // Enable ISO15118 on the EVSE Bricklet
    if (is_enabled()) {
        evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_ISO15118, 50);
        state_machine_task = task_scheduler.scheduleWithFixedDelay([this]() {
            this->state_machines_loop();
        }, 1000_ms, 20_ms);
    }
}

void ISO15118::state_machines_loop()
{
    // QCA700x SPI polling must always run (hardware communication)
    qca700x.state_machine_loop();

    // SLAC state machine for timeouts and protocol handling
    slac.state_machine_loop();

    // Clear revents before polling
    for (int i = 0; i < FDS_COUNT; i++) {
        fds[i].revents = 0;
    }

    int ret = poll(fds, FDS_COUNT, 0);

    if (ret < 0) {
        if (errno != EINTR) {
            logger.printfln("ISO15118: poll() failed: errno %d [%s]", errno, strerror(errno));
        }
        return;
    }

    if (ret == 0) {
        // No events, nothing to do
        return;
    }

    // Handle L2TAP (HomePlug/SLAC)
    if (fds[FDS_TAP_INDEX].fd >= 0) {
        if (fds[FDS_TAP_INDEX].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            logger.printfln("ISO15118: L2TAP error (revents=0x%x)", fds[FDS_TAP_INDEX].revents);
            // L2TAP error likely means modem issue, trigger SLAC reset
            slac.state = SLAC::State::ModemReset;
        } else if (fds[FDS_TAP_INDEX].revents & POLLIN) {
            slac.handle_tap();
        }
    }

    // Handle SDP UDP socket
    if (fds[FDS_SDP_INDEX].fd >= 0) {
        if (fds[FDS_SDP_INDEX].revents & (POLLERR | POLLNVAL)) {
            logger.printfln("ISO15118: SDP socket error (revents=0x%x), reopening", fds[FDS_SDP_INDEX].revents);
            sdp.close_socket();
            sdp.setup_socket();
        } else if (fds[FDS_SDP_INDEX].revents & POLLIN) {
            sdp.handle_socket();
        }
    }

    // Handle DIN/ISO2/ISO20 TCP listen socket
    if (fds[FDS_LISTEN_INDEX].fd >= 0) {
        if (fds[FDS_LISTEN_INDEX].revents & (POLLERR | POLLNVAL)) {
            logger.printfln("ISO15118: TCP listen socket error (revents=0x%x), reopening", fds[FDS_LISTEN_INDEX].revents);
            common.close_socket();
            common.setup_socket();
        } else if (fds[FDS_LISTEN_INDEX].revents & POLLIN) {
            common.handle_socket();
        }
    }

    // Handle active DIN/ISO2/ISO20 TCP connection
    if (fds[FDS_ACTIVE_INDEX].fd >= 0) {
        if (fds[FDS_ACTIVE_INDEX].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            logger.printfln("ISO15118: TCP active socket error (revents=0x%x), closing", fds[FDS_ACTIVE_INDEX].revents);
            common.reset_active_socket();
        } else if (fds[FDS_ACTIVE_INDEX].revents & POLLIN) {
            common.handle_socket();
        }
    }
}

void ISO15118::switch_to_iec_temporary()
{
    logger.printfln("Switching to IEC 61851 temporary mode");

    // Switch EVSE to IEC 61851 temporary mode
    // The EVSE will handle charging via PWM and automatically revert to ISO 15118 on EV disconnect
    evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_IEC61851_TEMPORARY, 1000);

    // Bring down PLC link since we no longer need it
    qca700x.link_down();

    // Reset SLAC state to be ready for next connection
    slac.state = SLAC::State::ModemReset;

    // Reset DC SoC session tracking for next EV connection
    iso2.reset_dc_soc_done();
}

ChargingInformation ISO15118::get_charging_information() const
{
    // TODO
    return {6000, true}; // 6A, 3-phase
}
