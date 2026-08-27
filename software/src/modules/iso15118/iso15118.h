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

#pragma once

#include "module.h"
#include "config.h"
#include "options.h"
#include "lwip/sockets.h"  // For struct pollfd
#include "qca700x.h"
#include "slac.h"
#include "sdp.h"
#include "common.h"
#include "din70121.h"
#include "iso2.h"
#include "iso20.h"
#include "debug_mode.h"
#include "pib_manager.h"
#include "bindings/bricklet_evse_v2.h"

// If defined, the iso15118 module logs verbose, per-message protocol decode dumps
// (DIN70121 / ISO15118-2 / ISO15118-20 / SLAC / SDP) to the "iso15118" tracelog via trace_iso().
//#define ISO15118_TRACE_MESSAGES

#ifdef ISO15118_TRACE_MESSAGES
#define trace_iso(...) iso15118.trace(__VA_ARGS__)
#else
// Disabled: Compiles to a no-op while keeping printf-style format checking
#define trace_iso(...) do { if (false) iso15118.trace(__VA_ARGS__); } while (0)
#endif

// Poll file descriptor indices (fixed positions in fds array)
static constexpr int FDS_TAP_INDEX    = 0;  // L2TAP for HomePlug/SLAC
static constexpr int FDS_SDP_INDEX    = 1;  // SDP UDP socket
static constexpr int FDS_LISTEN_INDEX = 2;  // DIN/ISO2/ISO20 TCP listen socket
static constexpr int FDS_ACTIVE_INDEX = 3;  // DIN/ISO2/ISO20 TCP active socket
static constexpr int FDS_COUNT        = 4;

static constexpr millis_t ISO15118_STATE_MACHINES_INTERVAL = 10_ms;

// SAP protocols served by this stack as "uri,major,minor". Matches
// Common::handle_supported_app_protocol_req, reported via
// ISO15118Ctrlr.ProtocolSupported when the OCPP 2.1 client runs.
static constexpr const char *iso15118_supported_protocols[] = {
    "urn:din:70121:2012:MsgDef,2,0",
    "urn:iso:15118:2:2013:MsgDef,2,0",
    "urn:iso:std:iso:15118:-20:AC,1,0",
};

struct ChargingInformation {
    uint16_t current_ma;
    bool three_phase;
};

class ISO15118 final : public IModule
{
private:
    ConfigRoot state_common;
    ConfigRoot state_din70121;
    ConfigRoot state_iso2;
    ConfigRoot state_iso20;

    size_t trace_buffer_index;
    size_t trace_buffer_index_ll;

    bool is_setup = false;
    uint64_t state_machine_task = 0;

public:
    ConfigRoot config;

    // Poll file descriptors for central I/O polling
    struct pollfd fds[FDS_COUNT];
    void set_poll_fd(int index, int fd) { fds[index].fd = fd; }

    ISO15118(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;
    void state_machines_loop();
    [[gnu::format(__printf__, 2, 3)]] void trace(const char *fmt, ...);
    void trace_packet(const uint8_t *packet, const size_t packet_size);

    // Returns true if ISO 15118 is enabled (any of autocharge, read_soc, or charge_via_iso15118 is set)
    bool is_enabled() const {
        return config.get("autocharge")->asBool() ||
               config.get("read_soc")->asBool() ||
               config.get("charge_via_iso15118")->asBool();
    }

    // Returns true if only autocharge is enabled, or if a Tesla is connected with
    // autocharge enabled. Teslas cannot start AC charging after the SoC was read.
    // In this mode, we switch to IEC 61851 temporary mode after SLAC completes.
    bool is_autocharge_only() const {
        return config.get("autocharge")->asBool() &&
               (!config.get("read_soc")->asBool() || (common.get_evcc_vendor() == EVCCVendor::Tesla)) &&
               !config.get("charge_via_iso15118")->asBool();
    }

    // Returns true if read_soc is enabled but charge_via_iso15118 is not.
    // In this mode, we read the SoC once via V2G and then switch to IEC 61851 temporary mode.
    bool is_read_soc_only() const {
        return config.get("read_soc")->asBool() &&
               !config.get("charge_via_iso15118")->asBool();
    }

    bool is_autocharge() const { return config.get("autocharge")->asBool(); }

    // Returns true if SLAC init should fall back after one attempt instead of
    // retrying C_SEQU_RETRY times with State E/F cycling.
    bool is_fast_timeout() const { return config.get("fast_timeout")->asBool(); }

    // TT_EVSE_SLAC_init value: ISO15118Ctrlr.PWMChargingFallbackTimeout while
    // the OCPP 2.1 client runs (HUB20-51-002)
    seconds_t get_slac_init_timeout() const;

    // Returns true if TLS may be offered in the SDP response. TLS is tied to
    // charging via ISO 15118-20.
    bool is_tls_enabled() const { return config.get("charge_via_iso15118")->asBool(); }

    static constexpr bool supports_pnc() { return OPTIONS_ISO15118_PNC_ENABLED() != 0; }

    // Experimental DC->AC fallback methods for EVs that refuse to
    // start basic AC charging after a graceful ISO 15118 session stop.
    // When true, begin_iec_transition() uses the ISO 15118-3 error teardown
    // [V2G3-M07-05..09]: X1 -> leave logical network -> E/F >= T_step_EF -> nominal PWM.
    bool opt_ef_teardown = false;
    // When true and in autocharge-only mode, complete SLAC/SDP/TCP and answer
    // supportedAppProtocolReq with Failed_NoNegotiation instead of skipping SDP/V2G.
    bool opt_nonegotiation_autocharge = false;
    // When true and in read_soc-only mode, keep the PLC modem enabled after the
    // DC SoC session, force a second SLAC round and answer the second
    // supportedAppProtocolReq with Failed_NoNegotiation.
    bool opt_nonegotiation_after_soc = false;

    bool nonegotiation_pending = false;
    // Deadline for the NoNegotiation wait, polled in state_machines_loop().
    micros_t reslac_guard_deadline = 0_us;
    // V2G_SECC_CommunicationSetup_Timer [V2G2-716]
    // Armed when CM_SLAC_MATCH.CNF is sent, cleared when supportedAppProtocolReq arrives.
    micros_t communication_setup_deadline = 0_us;
    void begin_reslac_for_nonegotiation();
    bool end_hlc_after_session_stop(uint64_t &next_timeout);

    // Switch to IEC 61851 temporary mode (EVSE controls charging, reverts to ISO15118 on disconnect)
    void switch_to_iec_temporary();
    void set_charging_protocol(const uint8_t charging_protocol, const uint16_t cp_duty_cycle);

    bool iec_temporary_active = false;

    uint64_t plc_modem_off_task = 0;
    void disable_plc_modem();
    void schedule_delayed_modem_off();

    uint64_t iec_switch_task = 0;

    uint64_t cp_resume_task = 0;
    void cancel_cp_resume_task();

    void cancel_pending_tasks();

    // Modem shutdown policy for begin_iec_transition():
    // None:      Caller handles the modem (already off, off pending, or keep on).
    // Immediate: No open socket, disable right away.
    // Delayed:   Socket still open; 5s timer, killed early when the EV closes TCP.
    enum class ModemOff : uint8_t { None, Immediate, Delayed };
    void begin_iec_transition(ModemOff modem_off = ModemOff::None);

    // Ensure the state machine task is running (schedules it if not already running)
    void ensure_state_machine_running();
    bool prepare_for_pib();
    bool prepare_for_pib_blocking(uint32_t timeout_ms);
    bool is_pib_modem_ready() const { return qca700x.is_modem_detected() && qca700x.is_l2tap_ready() && slac.state == SLACState::WaitForSlacParamRequest; }

    // Returns current limit and phase info
    ChargingInformation get_charging_information() const;

    QCA700x qca700x;
    SLAC slac;
    SDP sdp;
    Common common;
    DIN70121 din70121;
    ISO2 iso2;
    ISO20 iso20;
    DebugMode debug_mode;
    PibManager pib_manager;

    char evseid_iso[38];
    uint16_t evseid_iso_len;

    char evseid_iso_default[14];
    uint16_t evseid_iso_default_len;

    void refresh_evseid();

    uint8_t evseid_din[16];
    uint16_t evseid_din_len;
};
