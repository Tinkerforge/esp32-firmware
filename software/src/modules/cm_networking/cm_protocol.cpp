/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "cm_networking.h"
#include "module_dependencies.h"

#include <Arduino.h>

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"

#include <ESPmDNS.h>
#include "lwip/ip_addr.h"
#include "lwip/opt.h"
#include "lwip/dns.h"
#include <cstring>

int CMNetworking::create_socket(uint16_t port)
{
    int sock;
    struct sockaddr_in dest_addr;

    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        logger.printfln("Unable to create socket: errno %d", errno);
        return -1;
    }

    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        logger.printfln("Socket unable to bind: errno %d", errno);
        return -1;
    }

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        logger.printfln("Failed to get flags from socket: errno %d", errno);
        return -1;
    }

    err = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    if (err < 0) {
        logger.printfln("Failed to set O_NONBLOCK flag: errno %d", errno);
        return -1;
    }

    return sock;
}

static const uint8_t cm_command_packet_length_versions[] = {
    sizeof(struct cm_packet_header),
    sizeof(struct cm_packet_header) + sizeof(struct cm_command_v1),
};
static_assert(ARRAY_SIZE(cm_command_packet_length_versions) == (CM_COMMAND_VERSION + 1), "Unexpected amount of command packet length versions.");

static const uint8_t cm_state_packet_length_versions[] = {
    sizeof(struct cm_packet_header),
    sizeof(struct cm_packet_header) + sizeof(struct cm_state_v1),
    sizeof(struct cm_packet_header) + sizeof(struct cm_state_v1) + sizeof(struct cm_state_v2),
};
static_assert(ARRAY_SIZE(cm_state_packet_length_versions) == (CM_STATE_VERSION + 1), "Unexpected amount of state packet length versions.");

static String validate_packet_header_common(const struct cm_packet_header *header, ssize_t recv_length, const uint8_t packet_length_versions[], const char *packet_type_name)
{
    if (recv_length < sizeof(struct cm_packet_header)) {
        return String("Truncated header with ") + recv_length + " bytes.";
    }

    if (header->magic != CM_PACKET_MAGIC) {
        return String("Invalid magic. Got ") + header->magic + '.';
    }

    return String();
}

static String validate_protocol_version(const struct cm_packet_header *header, uint8_t min_version, uint8_t max_known_version, const uint8_t packet_length_versions[], const char *packet_type_name, ssize_t recv_length) {
    if (header->version < min_version) {
        return String("Protocol version ") + header->version + " too old. Need at least version " + min_version + ".";
    }

    if (header->version <= max_known_version) { // Known protocol version; match against known packet length.
        if (header->length != packet_length_versions[header->version])
            return String("Invalid ") + packet_type_name + " packet length for known protocol version " + header->version + ": " + header->length + " bytes.";

        // This is a known version. The recv_buf was large enough to receive the complete packet. Enforce length correctness
        if (recv_length != header->length)
            return String("Received truncated ") + packet_type_name + " packet for known protocol version " + header->version + ": " + recv_length + '/' + header->length + " bytes.";

    } else { // Newer protocol than known; packet must be at least as long as our newest known version.
        if (header->length < packet_length_versions[max_known_version])
            return String("Invalid ") + packet_type_name + " packet length for protocol version " + header->version + " from the future: " + header->length + " bytes.";

        // Received packet must be truncated because of the buffer size, other truncations are errors.
        if (recv_length != packet_length_versions[max_known_version])
            return String("Received truncated ") + packet_type_name + " packet for protocol version " + header->version + ": " + recv_length + '/' + header->length + " bytes.";
    }
    return "";
}

static String validate_command_packet_header(const struct cm_command_packet *pkt, ssize_t recv_length)
{
    String result = validate_packet_header_common(&(pkt->header), recv_length, cm_command_packet_length_versions, "command");
    if (result != "")
        return result;

    return validate_protocol_version(&(pkt->header), CM_COMMAND_VERSION_MIN, CM_COMMAND_VERSION, cm_command_packet_length_versions, "command", recv_length);
}

static String validate_state_packet_header(const struct cm_state_packet *pkt, ssize_t recv_length)
{
    String result = validate_packet_header_common(&(pkt->header), recv_length, cm_state_packet_length_versions, "state");
    if (result != "")
        return result;

    return validate_protocol_version(&(pkt->header), CM_STATE_VERSION_MIN, CM_STATE_VERSION, cm_state_packet_length_versions, "state", recv_length);
}

static bool seq_num_invalid(uint16_t received_sn, uint16_t last_seen_sn)
{
    return received_sn <= last_seen_sn && last_seen_sn - received_sn < 5;
}

void CMNetworking::register_manager(std::vector<String> &&hosts,
                                    const std::vector<String> &names,
                                    std::function<void(uint8_t /* client_id */, cm_state_v1 *, cm_state_v2 *)> manager_callback,
                                    std::function<void(uint8_t, uint8_t)> manager_error_callback)
{
    hostnames = hosts;

    for (int i = 0; i < names.size(); ++i) {
        dest_addrs[i].sin_addr.s_addr = 0;
        resolve_state[i] = RESOLVE_STATE_UNKNOWN;
        resolve_hostname(i);
        dest_addrs[i].sin_family = AF_INET;
        dest_addrs[i].sin_port = htons(CHARGE_MANAGEMENT_PORT);
    }

    manager_sock = create_socket(CHARGE_MANAGER_PORT);
    if (manager_sock < 0)
        return;

    task_scheduler.scheduleWithFixedDelay([this, names, manager_callback, manager_error_callback](){
        static uint16_t last_seen_seq_num[MAX_CLIENTS];
        static bool initialized = false;
        if (!initialized) {
            memset(last_seen_seq_num, 255, sizeof(last_seen_seq_num));
            initialized = true;
        }

        struct cm_state_packet state_pkt;
        struct sockaddr_in source_addr;
        socklen_t socklen = sizeof(source_addr);

        // Try to receive up to four packets in one go to catch up on any backlog.
        // Retrieving one packet every 100+ms is not enough with 10 chargers configured.
        // Also, chargers might sync up and send their status packets bunched up.
        // Retrieve them quickly to free up the RX buffer.
        // Don't process more than four packets in one go, to avoid stalling other tasks for too long.
        for (int poll_ctr = 0; poll_ctr < 4; ++poll_ctr) {
            int len = recvfrom(manager_sock, &state_pkt, sizeof(state_pkt), 0, (sockaddr *)&source_addr, &socklen);

            if (len < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                    logger.printfln("recvfrom failed: errno %d", errno);
                return;
            }

            int charger_idx = -1;
            for(int idx = 0; idx < names.size(); ++idx)
                if (source_addr.sin_family == dest_addrs[idx].sin_family &&
                    source_addr.sin_port == dest_addrs[idx].sin_port &&
                    source_addr.sin_addr.s_addr == dest_addrs[idx].sin_addr.s_addr) {
                        charger_idx = idx;
                        break;
                }

            // Don't log in the first 20 seconds after startup: We are probably still resolving hostnames.
            if (charger_idx == -1) {
                if (deadline_elapsed(20000))
                    logger.printfln("Received packet from unknown %s. Is the config complete?", inet_ntoa(source_addr.sin_addr));
                return;
            }

            String validation_error = validate_state_packet_header(&state_pkt, len);
            if (validation_error != "") {
                logger.printfln("Received state packet from %s (%s) (%i bytes) failed validation: %s",
                                names[charger_idx].c_str(),
                                inet_ntoa(source_addr.sin_addr),
                                len,
                                validation_error.c_str());
                manager_error_callback(charger_idx, CM_NETWORKING_ERROR_INVALID_HEADER);
                return;
            }

            if (seq_num_invalid(state_pkt.header.seq_num, last_seen_seq_num[charger_idx])) {
                logger.printfln("Received stale (out of order?) state packet from %s (%s). Last seen seq_num is %u, Received seq_num is %u",
                                names[charger_idx].c_str(),
                                inet_ntoa(source_addr.sin_addr),
                                last_seen_seq_num[charger_idx],
                                state_pkt.header.seq_num);
                return;
            }

            last_seen_seq_num[charger_idx] = state_pkt.header.seq_num;

            if (!CM_STATE_FLAGS_MANAGED_IS_SET(state_pkt.v1.state_flags)) {
                manager_error_callback(charger_idx, CM_NETWORKING_ERROR_NOT_MANAGED);
                logger.printfln("%s (%s) reports managed is not activated!",
                    names[charger_idx].c_str(),
                    inet_ntoa(source_addr.sin_addr));
                return;
            }

            manager_callback(charger_idx, &state_pkt.v1, state_pkt.header.version >= 2 ? &state_pkt.v2 : nullptr);
        }
    }, 100, 100);
}

bool CMNetworking::send_manager_update(uint8_t client_id, uint16_t allocated_current, bool cp_disconnect_requested)
{
    static uint16_t next_seq_num = 1;

    if (manager_sock < 0)
        return true;

    resolve_hostname(client_id);
    if (!is_resolved(client_id))
        return true;

    struct cm_command_packet command_pkt;
    command_pkt.header.magic = CM_PACKET_MAGIC;
    command_pkt.header.length = CM_COMMAND_PACKET_LENGTH;
    command_pkt.header.seq_num = next_seq_num;
    ++next_seq_num;
    command_pkt.header.version = CM_COMMAND_VERSION;

    command_pkt.v1.allocated_current = allocated_current;
    command_pkt.v1.command_flags = cp_disconnect_requested << CM_COMMAND_FLAGS_CPPDISC_BIT_POS;

    int err = sendto(manager_sock, &command_pkt, sizeof(command_pkt), 0, (sockaddr *)&dest_addrs[client_id], sizeof(dest_addrs[client_id]));

    if (err < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            // Intentionally don't increment here, we want to resend to this charger next.
            return false;
        if (errno == ENOMEM) {
            // Ignore ENOMEM for now. Usually indicates that we don't have a network connection yet.
            return true;
        }

        logger.printfln("CM failed to send command: %s (%d)", strerror(errno), errno);
        return true;
    }
    if (err != CM_COMMAND_PACKET_LENGTH) {
        logger.printfln("CM failed to send command: sendto truncated packet (of %u bytes) to %d bytes.", CM_COMMAND_PACKET_LENGTH, err);
        return true;
    }
    return true;
}

void CMNetworking::register_client(std::function<void(uint16_t, bool)> client_callback)
{
    client_sock = create_socket(CHARGE_MANAGEMENT_PORT);

    if (client_sock < 0)
        return;

    memset(&manager_addr, 0, sizeof(manager_addr));

    task_scheduler.scheduleWithFixedDelay([this, client_callback](){
        static uint16_t last_seen_seq_num = 255;
        static uint32_t last_successful_recv = millis();

        struct cm_command_packet command_pkt;

        struct sockaddr_storage temp_addr;
        socklen_t socklen = sizeof(temp_addr);
        int len = recvfrom(client_sock, &command_pkt, sizeof(command_pkt), 0, (struct sockaddr *)&temp_addr, &socklen);

        if (len < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                logger.printfln("recvfrom failed: errno %d", errno);

            // If we have not received a valid packet for one minute, invalidate manager_addr.
            // Otherwise we would send state packets to this address forever.
            if (deadline_elapsed(last_successful_recv + 60 * 1000))
                manager_addr_valid = false;

            return;
        }

        String validation_error = validate_command_packet_header(&command_pkt, len);
        if (validation_error != "") {
            logger.printfln("Received command packet from %s (%i bytes) failed validation: %s",
                inet_ntoa(((struct sockaddr_in*)&temp_addr)->sin_addr),
                len,
                validation_error.c_str());
            return;
        }

        if (seq_num_invalid(command_pkt.header.seq_num, last_seen_seq_num)) {
            logger.printfln("received stale (out of order?) command packet. last seen seq_num is %u, received seq_num is %u", last_seen_seq_num, command_pkt.header.seq_num);
            return;
        }

        last_seen_seq_num = command_pkt.header.seq_num;

        if (manager_addr_valid && memcmp(&manager_addr, &temp_addr, manager_addr.s2_len) != 0) {
            char manager_str[16];
            char temp_str[16];
            inet_ntoa_r(((struct sockaddr_in*)&manager_addr)->sin_addr, manager_str, sizeof(manager_str));
            inet_ntoa_r(((struct sockaddr_in*)&temp_addr   )->sin_addr, temp_str,    sizeof(temp_str   ));
            logger.printfln("cm_networking: Warning: Manager address changed from %s to %s.", manager_str, temp_str);
        }

        last_successful_recv = millis();
        manager_addr = temp_addr;
        manager_addr_valid = true;

        client_callback(command_pkt.v1.allocated_current, CM_COMMAND_FLAGS_CPPDISC_IS_SET(command_pkt.v1.command_flags));
        //logger.printfln("Received command packet. Allocated current is %u", command_pkt.v1.allocated_current);
    }, 100, 100);
}

bool CMNetworking::send_client_update(uint32_t esp32_uid,
                                      uint8_t iec61851_state,
                                      uint8_t charger_state,
                                      uint32_t time_since_state_change,
                                      uint8_t error_state,
                                      uint32_t uptime,
                                      uint32_t charging_time,
                                      uint16_t allowed_charging_current,
                                      uint16_t supported_current,
                                      bool managed,
                                      bool cp_disconnected_state)
{
    static uint16_t next_seq_num = 0;

    if (!manager_addr_valid) {
        //logger.printfln("manager addr not valid.");
        return false;
    }
    //logger.printfln("Sending state packet.");

    struct cm_state_packet state_pkt;
    state_pkt.header.magic = CM_PACKET_MAGIC;
    state_pkt.header.length = CM_STATE_PACKET_LENGTH;
    state_pkt.header.seq_num = next_seq_num;
    ++next_seq_num;
    state_pkt.header.version = CM_STATE_VERSION;

    bool has_meter_values = api.hasFeature("meter_all_values");
    bool has_meter_phases = api.hasFeature("meter_phases");
    bool has_meter        = api.hasFeature("meter");

    state_pkt.v1.feature_flags = 0
        | api.hasFeature("cp_disconnect")           << CM_FEATURE_FLAGS_CP_DISCONNECT_BIT_POS
        | api.hasFeature("evse")                    << CM_FEATURE_FLAGS_EVSE_BIT_POS
        | api.hasFeature("nfc")                     << CM_FEATURE_FLAGS_NFC_BIT_POS
        | has_meter_values                          << CM_FEATURE_FLAGS_METER_ALL_VALUES_BIT_POS
        | has_meter_phases                          << CM_FEATURE_FLAGS_METER_PHASES_BIT_POS
        | has_meter                                 << CM_FEATURE_FLAGS_METER_BIT_POS
        | api.hasFeature("button_configuration")    << CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_BIT_POS;

    state_pkt.v1.esp32_uid = esp32_uid;
    state_pkt.v1.evse_uptime = uptime;
    state_pkt.v1.charging_time = charging_time;
    state_pkt.v1.allowed_charging_current = allowed_charging_current;
    state_pkt.v1.supported_current = supported_current;
    state_pkt.v1.iec61851_state = iec61851_state;
    state_pkt.v1.charger_state = charger_state;
    state_pkt.v1.error_state = error_state;

    long flags = 0
        | managed               << CM_STATE_FLAGS_MANAGED_BIT_POS
        | cp_disconnected_state << CM_STATE_FLAGS_CP_DISCONNECTED_BIT_POS;
    if (has_meter_phases) {
        auto meter_phase_values = api.getState("meter/phases");
        flags |= meter_phase_values->get("phases_connected")->get(0)->asBool() << CM_STATE_FLAGS_L1_CONNECTED_BIT_POS;
        flags |= meter_phase_values->get("phases_connected")->get(1)->asBool() << CM_STATE_FLAGS_L2_CONNECTED_BIT_POS;
        flags |= meter_phase_values->get("phases_connected")->get(2)->asBool() << CM_STATE_FLAGS_L3_CONNECTED_BIT_POS;
        flags |= meter_phase_values->get("phases_active")->get(0)->asBool() << CM_STATE_FLAGS_L1_ACTIVE_BIT_POS;
        flags |= meter_phase_values->get("phases_active")->get(1)->asBool() << CM_STATE_FLAGS_L2_ACTIVE_BIT_POS;
        flags |= meter_phase_values->get("phases_active")->get(2)->asBool() << CM_STATE_FLAGS_L3_ACTIVE_BIT_POS;
    }
    state_pkt.v1.state_flags = static_cast<uint8_t>(flags);

    if (has_meter_values) {
        auto meter_all_values = api.getState("meter/all_values");
        for (int i = 0; i < 3; i++) {
            state_pkt.v1.line_voltages[i]      = meter_all_values->get(i + METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L1)->asFloat();
            state_pkt.v1.line_currents[i]      = meter_all_values->get(i + METER_ALL_VALUES_CURRENT_L1_A            )->asFloat();
            state_pkt.v1.line_power_factors[i] = meter_all_values->get(i + METER_ALL_VALUES_POWER_FACTOR_L1         )->asFloat();
        }
    } else {
        for (int i = 0; i < 3; i++) {
            state_pkt.v1.line_voltages[i]      = 0;
            state_pkt.v1.line_currents[i]      = 0;
            state_pkt.v1.line_power_factors[i] = 0;
        }
    }

    if (has_meter) {
        auto meter_values = api.getState("meter/values");
        state_pkt.v1.power_total = meter_values->get("power")->asFloat();
        state_pkt.v1.energy_rel = meter_values->get("energy_rel")->asFloat();
        state_pkt.v1.energy_abs = meter_values->get("energy_abs")->asFloat();
    } else {
        state_pkt.v1.power_total = 0;
        state_pkt.v1.energy_rel = 0;
        state_pkt.v1.energy_abs = 0;
    }

    state_pkt.v2.time_since_state_change = time_since_state_change;

    int err = sendto(client_sock, &state_pkt, sizeof(state_pkt), 0, (sockaddr *)&manager_addr, sizeof(manager_addr));
    if (err < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            logger.printfln("CM failed to send state: %s (%d)", strerror(errno), errno);
        return false;
    }
    if (err != CM_STATE_PACKET_LENGTH) {
        logger.printfln("CM failed to send state: sendto truncated packet (of %u bytes) to %d bytes.", CM_STATE_PACKET_LENGTH, err);
        return false;
    }

    return true;
}
