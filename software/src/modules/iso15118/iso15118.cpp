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

// ============================================================================
// Overview
// ============================================================================
//
// The ISO15118 class is the coordinator for all of the different versions and
// layers of the ISO 15118 standard.
// It coordinates between the following protocols:
// * SPI/QCA700x (Low level SPI communication with the QCA700x modem),
// * SLAC (Signal-Level-Attenuation-Characterization) as described in
//   ISO 15118-3,
// * IPv6/ICMPv6/NDP (Neighbor Discovery Protocol with Neighbor
//   Solicitation/Advertisement),
// * IPv6/UDP/SDP (SECC Discovery Protocol),
// * IPv6/TCP/DIN-SPEC-70121,
// * IPv6/TCP/ISO-15118-2,
// * IPv6/TCP/ISO-15118-20
// and the EVSE that runs on a different board (the EVSE Bricklet).
// The EXI message encoding/decoding is based on libcbv2g.
// The objective is to *robustly* be able to
// * fingerprint the EV for authentication,
// * read SoC from the EV and
// * if it will be implemented by EVs, allow for bi-directional charging.
//
// Details for the three supported modes below.
// ============================================================================


// ============================================================================
// Autocharge only (autocharge=true, read_soc/charge_via_iso15118=false)
// ============================================================================
//
// In autocharge mode, the EVSE performs SLAC to identify the EV by its PLC
// MAC address, then immediately switches to IEC 61851 PWM-based charging
// without doing SDP or any V2G communication.
//
// SLAC Flow
// ---------
//   1. EV plugs in, sees 5% CP duty (digital communication mode)
//   2. Full SLAC process runs (CM_SLAC_PARM, attenuation characterization,
//      CM_SLAC_MATCH.REQ) to verify the EV is physically connected (not
//      crosstalk from a neighboring charger)
//   3. PEV MAC is captured from the validated SLAC exchange
//   4. CM_SLAC_MATCH.CNF is NOT sent: confirming the match would cause the
//      EV to join the PLC network and attempt SDP/V2G, which times out
//      (~90-100s) before falling back to IEC. By not confirming, the EV's
//      TT_MATCH_RESPONSE (200ms) expires.
//
// Transition to IEC 61851 (after SLAC)
// -------------------------------------
//   Phase 1 (0-2s):  CP is set to 100% duty cycle (begin_iec_transition).
//   Phase 2 (>2s):   CP switches to IEC 61851 temporary mode with PWM.
//
// PLC Modem Shutdown (immediate)
// ------------------------------
// The PLC modem is disabled immediately after SLAC completes. Unlike the
// SoC reading mode, there is no TCP socket or pending V2G response that the
// EV needs to receive.
// ============================================================================


// ============================================================================
// SoC Reading Mode (read_soc=true, charge_via_iso15118=false)
// ============================================================================
//
// The EV's State of Charge (SoC) is only transmitted during DC charging
// sessions in ISO 15118-2 / DIN 70121.
// To read the SoC anyway, we initiate a "fake" DC session: we negotiate DC
// parameters long enough for the EV to report its SoC, then shut the session
// down before any actual DC power transfer begins. Afterwards, charging
// continues via IEC 61851 PWM.
//
// V2G Message Flow
// ----------------
//   1. SLAC, SDP, TCP connection  -> normal ISO 15118 link setup
//   2. SessionSetupReq/Res        -> new session established
//   3. ServiceDiscoveryReq/Res    -> we offer DC_extended as the energy
//                                    transfer mode
//   4. PaymentServiceSelectionReq/Res
//   5. AuthorizationReq/Res       -> EVSEProcessing=Finished
//   6. ChargeParameterDiscoveryReq (1st)
//      - The EV includes DC_EVChargeParameter with its current EVRESSSOC.
//      - We extract and store the SoC (+ energy capacity if available).
//      - We respond with EVSEProcessing=Ongoing and EVSE_Ready.
//        This keeps the session alive without advancing to CableCheck.
//   7. ChargeParameterDiscoveryReq (2nd)
//      - soc_read is now true. We respond with EVSE_Shutdown and
//        EVSEProcessing=Ongoing to signal the EV to end the session.
//   8. SessionStopReq/Res         -> The EV terminates itself.
//
//   If the EV ignores the shutdown signal and proceeds to CableCheck,
//   PreCharge, or CurrentDemand, each of those handlers responds with
//   ResponseCode=FAILED per [V2G2-539] to force session termination.
//
// Transition to IEC 61851 (after SessionStop or EVSE_Shutdown)
// ------------------------------------------------------------
// After SessionStopRes is sent, the charger transitions from ISO 15118
// signaling to IEC 61851 PWM-based charging. This is a two-phase process:
//
//   Phase 1 (0-2s):  CP is set to 100% duty cycle. This gives the EV a
//                    clean break from the 5% ISO 15118 signal. Some EVs
//                    (e.g. Cupra Born, ID.Buzz and probably all other MEB
//                    platform EVs) require seeing a steady-state CP before
//                    they accept a new PWM signal.
//   Phase 2 (>2s):   CP switches to IEC 61851 temporary mode with actual
//                    PWM. The EVSE Bricklet controls charging from here and
//                    automatically reverts to ISO 15118 mode when the EV
//                    disconnects.
//
// PLC Modem Shutdown (after socket close)
// ---------------------------------------
// The PLC modem is not killed immediately. Some EVs will refuse to react to
// PWM changes or reconnect via ISO 15118 if the TCP socket or PLC link
// disappears too early.
//
// The approach:
//   - The TCP socket is intentionally left open after SessionStopRes.
//   - A timer is scheduled to disable the PLC modem.
//   - If the EV closes TCP early (this is the expected path),
//     the timer is cancelled and the modem is disabled immediately.
//   - On EV physical disconnect (State A), all pending timers are cancelled,
//     the modem is re-enabled, and CP is reset to 5% for the next EV.
//
// Real-world EV behaviors
// -----------------------
// For now we were able to observe two distinct behaviors:
//    1. EV closes TCP connection immediately after EVSE_Shutdown response
//    2. EV does not react on *any* EVSE failure responses and keeps the
//       session alive.
//
// The former will close the socket on the second ChargeParameterDiscoveryReq
// and cleanly transition to IEC 61851 mode. For the latter we rely on the
// internal timeout that is triggered after ~10s of receiving the "ongoing"
// ChargeParameterDiscoveryRes in a loop. After that the EV will give up
// and send a SessionStopReq, after which we can transition to IEC 61851 mode.
// ============================================================================


// ============================================================================
// Charge via ISO 15118-20 (charge_via_iso15118=true)
// ============================================================================
// TBD: Will only be available for EVs that support ISO 15118-20,
//      In this mode it will be possible to read SoC and charge directly
//      through ISO 15118 without switching to IEC 61851 at all.
//      This will (if the EV supports it) also allow bidirectional charging.
// ============================================================================


// ============================================================================
// Fallback for EVs without ISO 15118 support
// ============================================================================
//
// When the EVSE is in any ISO 15118 mode, CP is set to 5% duty cycle to
// signal digital communication capability. If the EV does not support
// ISO 15118 (no PLC modem), it will never send CM_SLAC_PARM.REQ and
// would be stuck on 5% indefinitely.
//
// Per ISO 15118-3 [V2G3-M06-07], we implement the following fallback:
//
//   1. After EV connects (IEC State A -> B), start TT_EVSE_SLAC_init
//      timer (50s, SLAC_TT_EVSE_SLAC_INIT_MAX).
//   2. If no CM_SLAC_PARM.REQ is received within TT_EVSE_SLAC_init:
//      a. Set CP to State E/F (0% duty, -12V) for T_step_EF (4s).
//      b. Return to 5% duty cycle and restart TT_EVSE_SLAC_init.
//      c. Repeat up to C_SEQU_RETRY (2) times.
//   3. After all retries exhausted: switch to IEC 61851 temporary mode
//      via begin_iec_transition() and disable PLC modem.
//
// Timeline for non-ISO EV:
//   t=0s    EV plugs in, timeout starts (50s)
//   t=50s   Timeout #1: CP -> E/F for 4s, then back to 5%
//   t=104s  Timeout #2: CP -> E/F for 4s, then back to 5%
//   t=158s  Timeout #3: retries exhausted -> IEC fallback
//   t=160s  CP -> IEC 61851 PWM, non-ISO EV can charge
//
// This is crazy long... but not much we can do about it.
// ============================================================================

#include "iso15118.h"

#include <time.h>
#include <type_traits>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "build.h"

#include "tools/hexdump.h"
#include "tools/printf.h"

#include "qca700x.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "bindings/bricklet_evse_v2.h"

#include "gcc_warnings.h"

extern uint32_t local_uid_num;

// RFC 4648 Base32 alphabet: A-Z (0-25), 2-7 (26-31)
static const char base32_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

// Encode a uint32_t into 7 base32 characters (ceil(32/5) = 7).
// The top 3 bits of the first character are always 0.
static void encode_iso_evseid(uint32_t value, char *out)
{
    for (int i = 6; i >= 0; i--) {
        out[i] = base32_alphabet[value & 0x1F];
        value >>= 5;
    }
    out[7] = '\0';
}

// Convert a DIN SPEC 91286 character to its nibble representation.
// '0'-'9' -> 0x0-0x9, '*' -> 0xA, invalid -> 0xF
static uint8_t din_char_to_nibble(char c)
{
    if (c >= '0' && c <= '9') {
        return static_cast<uint8_t>(c - '0');
    }
    if (c == '*') {
        return 0xA;
    }
    return 0xF;
}

// Encode a DIN SPEC 91286 EVSEID string into nibble-packed bytes.
// Each character maps to one nibble, two nibbles per byte (high nibble first).
// If the string has odd length, the last byte's low nibble is set to 0xF.
static uint16_t encode_din_evseid(const char *str, uint8_t *out, size_t out_size)
{
    size_t len = strlen(str);
    uint16_t byte_count = 0;

    for (size_t i = 0; i < len && byte_count < out_size; i += 2) {
        uint8_t high = din_char_to_nibble(str[i]);
        uint8_t low = (i + 1 < len) ? din_char_to_nibble(str[i + 1]) : 0xF;
        out[byte_count++] = static_cast<uint8_t>((high << 4) | low);
    }

    return byte_count;
}

void ISO15118::trace_packet(const uint8_t *packet, const size_t packet_size)
{
#if defined(BOARD_HAS_PSRAM)
    uint32_t secs = now_us().to<millis_t>().as<uint32_t>();
    char timestamp[16];
    size_t len = snprintf_u(timestamp, sizeof(timestamp), "%lu ", secs);
    logger.trace_plain(trace_buffer_index_ll, timestamp, len);

    // Hexdump the packet in one go (2 hex chars per byte + null terminator)
    size_t hex_buf_size = packet_size * 2 + 1;
    if (hex_buf_size <= 2048) {
        char hex_buf[2048];
        len = hexdump(packet, packet_size, hex_buf, hex_buf_size, HexdumpCase::Lower);
        logger.trace_plain(trace_buffer_index_ll, hex_buf, len);
    } else {
        logger.printfln("trace_packet: packet too large for hexdump (%zu bytes)", packet_size);
    }
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
                evse_v2.set_plc_modem(true);
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
                evse_v2.set_plc_modem(false);
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
    debug_mode.pre_setup();
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

    // Generate EVSEID for ISO 15118-2 / ISO 15118-20
    // Format: "DEWRPE" + base32(local_uid_num)
    // "DE" = country code (Germany), "WRP" = EVSE operator ID, "E" = ID type (EVSE)
    // base32 encoding uses RFC 4648 alphabet (A-Z, 2-7) for an unambiguous 7-char outlet ID.
    strcpy(evseid_iso, "DEWRPE");
    encode_iso_evseid(local_uid_num, evseid_iso + 6);
    evseid_iso_len = 13;

    // Generate EVSEID for DIN 70121
    // Format: "49*000*" + decimal(local_uid_num), nibble-encoded per DIN SPEC 91286
    // "49" = numeric country code (Germany), "000" = operator ID
    // Each character maps to one nibble: '0'-'9' -> 0x0-0x9, '*' -> 0xA
    char din_str[32];
    int din_str_len = snprintf(din_str, sizeof(din_str), "49*000*%lu", local_uid_num);
    // Ensure even number of nibbles for clean byte packing
    if (din_str_len % 2 != 0) {
        snprintf(din_str, sizeof(din_str), "49*000*0%lu", local_uid_num);
    }
    evseid_din_len = encode_din_evseid(din_str, evseid_din, sizeof(evseid_din));

    logger.printfln("ISO15118: EVSEID ISO: %s, DIN: %s", evseid_iso, din_str);

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
    api.addState("iso15118/debug",          &debug_mode.api_state);
    api.addCommand("iso15118/debug_update", &debug_mode.api_state, {}, [this](Language language, String &errmsg) {
        debug_mode.handle_update(language, errmsg);
    }, true);

    // Enable ISO15118 on the EVSE Bricklet
    if (is_enabled()) {
        // Ensure PLC modem is enabled. If only the ESP32 restarted while the
        // EVSE Bricklet kept running with the modem in hardware reset (from a
        // previous set_plc_modem(false) after SessionStop), the modem would
        // stay disabled forever.
        evse_v2.set_plc_modem(true);
        evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_ISO15118, 50);
        state_machine_task = task_scheduler.scheduleWithFixedDelay([this]() {
            this->state_machines_loop();
        }, 1000_ms, 20_ms);
    }
}

void ISO15118::register_events()
{
    // Monitor charger state for EV connect/disconnect
    event.registerEvent("evse/state", {"charger_state"}, [this](const Config *charger_state) {
        if (charger_state->asUint() == 0) {
            // EV disconnected (State A)
            if (iec_temporary_active) {
                logger.printfln("ISO15118: EV disconnected (State A), cleaning up");
                common.reset_active_socket();
                qca700x.link_down();
                slac.state = SLACState::ModemReset;
                slac.api_state.get("modem_initialization_tries")->updateUint(0);
                iso2.reset_dc_soc_done();
                iec_temporary_active = false;

                // Cancel any pending delayed modem-off task.
                if (plc_modem_off_task != 0) {
                    task_scheduler.cancel(plc_modem_off_task);
                    plc_modem_off_task = 0;
                }

                // Cancel any pending IEC switch task.
                if (iec_switch_task != 0) {
                    task_scheduler.cancel(iec_switch_task);
                    iec_switch_task = 0;
                }

                // Re-enable PLC modem for the next EV.
                evse_v2.set_plc_modem(true);

                // Reset CP back to 5% duty. This assumes that we are in some
                // ISO15118 mode, since iec_temporary_active was set.
                evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_ISO15118, 50);

                // Cancel any pending E/F reset task.
                if (ef_reset_task != 0) {
                    task_scheduler.cancel(ef_reset_task);
                    ef_reset_task = 0;
                }
                ef_retry_count = 0;
                slac.reset_slac_init_retry_count();
            }
        }
        return EventResult::OK;
    });
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
        if (static_cast<unsigned short>(fds[FDS_TAP_INDEX].revents) & (POLLERR | POLLHUP | POLLNVAL)) {
            logger.printfln("ISO15118: L2TAP error (revents=0x%x)", static_cast<unsigned>(fds[FDS_TAP_INDEX].revents));
            // L2TAP error likely means modem issue, trigger SLAC reset
            slac.state = SLACState::ModemReset;
        } else if (static_cast<unsigned short>(fds[FDS_TAP_INDEX].revents) & POLLIN) {
            slac.handle_tap();
        }
    }

    // Handle SDP UDP socket
    if (fds[FDS_SDP_INDEX].fd >= 0) {
        if (static_cast<unsigned short>(fds[FDS_SDP_INDEX].revents) & (POLLERR | POLLNVAL)) {
            logger.printfln("ISO15118: SDP socket error (revents=0x%x), reopening", static_cast<unsigned>(fds[FDS_SDP_INDEX].revents));
            sdp.close_socket();
            sdp.setup_socket();
        } else if (static_cast<unsigned short>(fds[FDS_SDP_INDEX].revents) & POLLIN) {
            sdp.handle_socket();
        }
    }

    // Handle DIN/ISO2/ISO20 TCP listen socket
    if (fds[FDS_LISTEN_INDEX].fd >= 0) {
        if (static_cast<unsigned short>(fds[FDS_LISTEN_INDEX].revents) & (POLLERR | POLLNVAL)) {
            logger.printfln("ISO15118: TCP listen socket error (revents=0x%x), reopening", static_cast<unsigned>(fds[FDS_LISTEN_INDEX].revents));
            common.close_socket();
            common.setup_socket();
        } else if (static_cast<unsigned short>(fds[FDS_LISTEN_INDEX].revents) & POLLIN) {
            common.handle_socket();
        }
    }

    // Handle active DIN/ISO2/ISO20 TCP connection
    if (fds[FDS_ACTIVE_INDEX].fd >= 0) {
        bool connection_closed = false;

        if (static_cast<unsigned short>(fds[FDS_ACTIVE_INDEX].revents) & (POLLERR | POLLHUP | POLLNVAL)) {
            logger.printfln("ISO15118: TCP active socket error (revents=0x%x), closing", static_cast<unsigned>(fds[FDS_ACTIVE_INDEX].revents));
            common.reset_active_socket();
            connection_closed = true;
        } else if (static_cast<unsigned short>(fds[FDS_ACTIVE_INDEX].revents) & POLLIN) {
            common.handle_socket();

            // A clean TCP close (FIN) triggers POLLIN with recv()=0, not POLLHUP.
            // If handle_socket() detected this and closed the fd, handle it the same
            // as the POLLHUP/POLLERR path.
            if (fds[FDS_ACTIVE_INDEX].fd < 0) {
                connection_closed = true;
            }
        }

        if (connection_closed) {
            // If we're waiting for the EV to close TCP after SessionStop,
            // cancel the 5s safety timer and schedule modem shutdown after a short
            // delay. The 200ms gives the TCP stack time to complete the FIN/ACK
            // exchange before we kill the PLC link.
            if (plc_modem_off_task != 0) {
                logger.printfln("ISO15118: EV closed TCP, cancelling 5s timer, disabling modem in 200ms");
                task_scheduler.cancel(plc_modem_off_task);
                plc_modem_off_task = task_scheduler.scheduleOnce([this]() {
                    disable_plc_modem();
                }, 200_ms);
            }

            // If the EV closes the socket unexpectedly (e.g. after FAILED response),
            // begin IEC transition.
            if (!iec_temporary_active && is_read_soc_only()) {
                logger.printfln("ISO15118: EV closed TCP after shutdown/FAILED, beginning IEC transition");
                begin_iec_transition();
            }
        }
    }
}

void ISO15118::switch_to_iec_temporary()
{
    logger.printfln("Switching to IEC 61851 temporary mode");

    // Switch EVSE to IEC 61851 temporary mode.
    // The EVSE will handle charging via PWM and automatically revert to ISO 15118 on EV disconnect.
    evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_IEC61851_TEMPORARY, 1000);

    // Mark that we're in IEC temporary mode waiting for EV disconnect.
    iec_temporary_active = true;

    // Schedule E/F reset
    ef_retry_count = 0;
    if (ef_reset_task != 0) {
        task_scheduler.cancel(ef_reset_task);
    }
    // TODO: ef_reset is currently turned off.
    //       The EVSE Bricklet needs additional support for this. Currently this can
    //       collide with the CP disconnect that the Bricklet uses to try to wake up the EV.
    // ef_reset_task = task_scheduler.scheduleOnce([this]() { check_ef_reset(); }, 15000_ms);
}

void ISO15118::check_ef_reset()
{
    ef_reset_task = 0;

    if (!iec_temporary_active) {
        return; // EV disconnected or mode changed, nothing to do
    }

    uint32_t charger_state = evse_common.get_state().get("charger_state")->asUint();
    if (charger_state >= 3) {
        logger.printfln("ISO15118: EV charging (charger_state %lu), E/F reset not needed", charger_state);
        return; // EV already charging
    }
    if (charger_state == 0) {
        return; // EV disconnected (race with event handler)
    }

    if (ef_retry_count >= 3) { // C_sequ_retry = 3 per ISO 15118-3 Table 3
        logger.printfln("ISO15118: E/F reset retries exhausted (%u), giving up", ef_retry_count);
        return;
    }

    ef_retry_count++;
    logger.printfln("ISO15118: EV still in State B after IEC temporary switch, triggering E/F reset (attempt %u/3)", ef_retry_count);

    // Set 0% duty cycle on CP (-12V), which signals State E/F to the EV.
    // Per ISO 15118-3 Table 3: T_step_EF >= 4 seconds, C_sequ_retry = 3.
    evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_ISO15118, 0);

    // After 4 seconds, switch back to IEC temporary mode and schedule another check
    ef_reset_task = task_scheduler.scheduleOnce([this]() {
        evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_IEC61851_TEMPORARY, 1000);

        // Give the EV 15 seconds to start charging before retrying
        ef_reset_task = task_scheduler.scheduleOnce([this]() { check_ef_reset(); }, 15000_ms);
    }, 4000_ms);
}

void ISO15118::ensure_state_machine_running()
{
    if (state_machine_task == 0) {
        state_machine_task = task_scheduler.scheduleWithFixedDelay([this]() {
            this->state_machines_loop();
        }, 20_ms, 20_ms);
    }
    is_setup = true;
}

void ISO15118::disable_plc_modem()
{
    plc_modem_off_task = 0;

    logger.printfln("ISO15118: Disabling PLC modem");
    evse_v2.set_plc_modem(false);
    slac.state = SLACState::ModemDisabled;

    // Close the active socket if it's still open.
    if (fds[FDS_ACTIVE_INDEX].fd >= 0) {
        common.reset_active_socket();
    }
}

void ISO15118::begin_iec_transition()
{
    logger.printfln("ISO15118: Stopping PWM (100%% duty) before IEC transition");

    // Set CP to 100% duty. This gives the EV a clean break from the ISO 15118 signal
    // before we offer IEC 61851 PWM charging. Some EVs (e.g. Cupra Born and ID.Buzz)
    // won't accept a PWM unless they see 100% duty cycle first.
    evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_ISO15118, 1000);

    // Mark that the IEC transition is in progress so the State A handler
    // knows to clean up if the EV disconnects during the delay.
    iec_temporary_active = true;

    // After 2 seconds, switch to IEC temporary mode.
    if (iec_switch_task != 0) {
        task_scheduler.cancel(iec_switch_task);
    }
    iec_switch_task = task_scheduler.scheduleOnce([this]() {
        iec_switch_task = 0;
        switch_to_iec_temporary();
    }, 2000_ms);
}

// TODO: Upgrade to per-phase power control based on protocol version and EV capabilities.
ChargingInformation ISO15118::get_charging_information() const
{
    // In debug mode there is no EVSE connected, so we return the values
    // set by the user via the iso15118/debug API.
    if (debug_mode.is_enabled()) {
        return debug_mode.get_charging_information();
    }

    uint16_t current_ma = evse_common.get_state().get("allowed_charging_current")->asUint16();
    bool three_phase = evse_common.get_low_level_state().get("phases_current")->asUint() == 3;
    return {current_ma, three_phase};
}
