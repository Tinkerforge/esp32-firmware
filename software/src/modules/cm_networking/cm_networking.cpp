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

#include <Arduino.h>

#include "task_scheduler.h"

#include "modules.h"

#include <ESPmDNS.h>
#include "NetBIOS.h"
#include "lwip/ip_addr.h"
#include "lwip/opt.h"
#include "lwip/dns.h"
#include <cstring>

#include "TFJson.h"

extern TaskScheduler task_scheduler;
extern API api;
extern EventLog logger;
extern WebServer server;

void CMNetworking::pre_setup()
{
}

void CMNetworking::setup()
{
    mdns_init();
    initialized = true;
}

void CMNetworking::register_urls()
{
    api.addCommand("charge_manager/scan", Config::Null(), {}, [this]() {
        start_scan();
    }, true);

    server.on("/charge_manager/scan_result", HTTP_GET, [this](WebServerRequest request) {
        String result = cm_networking.get_scan_results();

        if (result == "In progress or not started")
            return request.send(200, "text/plain; charset=utf-8", result.c_str());

        return request.send(200, "application/json; charset=utf-8", result.c_str());
    });

// If we don't have the evse or evse_v2 module, but have cm_networking, this is probably an energy manager.
// We only want to announce manageable chargers, not managers.
#if MODULE_EVSE_AVAILABLE() || MODULE_EVSE_V2_AVAILABLE()
    if (!network.config.get("enable_mdns")->asBool())
        return;

    MDNS.addService("tf-warp-cm", "udp", 34127);
    MDNS.addServiceTxt("tf-warp-cm", "udp", "version", __XSTRING(CM_PACKET_MAGIC) "." __XSTRING(CM_PROTOCOL_VERSION));
    task_scheduler.scheduleWithFixedDelay([](){
        #if MODULE_DEVICE_NAME_AVAILABLE()
            // Keep "display_name" updated because it can be changed at runtime without clicking "Save".
            MDNS.addServiceTxt("tf-warp-cm", "udp", "display_name", device_name.display_name.get("display_name")->asString());
        #endif

            bool management_enabled = false;
        #if MODULE_EVSE_AVAILABLE()
            management_enabled = evse.evse_management_enabled.get("enabled")->asBool();
        #elif MODULE_EVSE_V2_AVAILABLE()
            management_enabled = evse_v2.evse_management_enabled.get("enabled")->asBool();
        #endif

        // Keep "enabled" updated because it is retrieved from the EVSE.
        MDNS.addServiceTxt("tf-warp-cm", "udp", "enabled", management_enabled ? "true" : "false");
    }, 0, 10000);
#endif
}

void CMNetworking::loop()
{
}

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

static void dns_callback(const char *host, const ip_addr_t *ip, void *args)
{
    std::lock_guard<std::mutex> lock{cm_networking.dns_resolve_mutex};
    uint8_t *resolve_state = (uint8_t*)args;
    if (ip == nullptr && *resolve_state != RESOLVE_STATE_NOT_RESOLVED) {
        *resolve_state = RESOLVE_STATE_NOT_RESOLVED;
        logger.printfln("Failed to resolve %s", host);
    }
}

void CMNetworking::resolve_hostname(uint8_t charger_idx)
{
    ip_addr_t ip;
    int err = dns_gethostbyname(hostnames[charger_idx].c_str(), &ip, dns_callback, &resolve_state[charger_idx]);

    if (err == ERR_VAL)
        logger.printfln("Charge manager has charger configured with hostname %s, but no DNS server is configured!", hostnames[charger_idx].c_str());

    if (err != ERR_OK || ip.type != IPADDR_TYPE_V4)
        return;

    in_addr_t in;

    // using memcpy to guarantee alignment https://mail.gnu.org/archive/html/lwip-users/2008-08/msg00166.html
    std::memcpy(&in, &ip.u_addr, sizeof(ip4_addr_t));

    std::lock_guard<std::mutex> lock{dns_resolve_mutex};
    if (resolve_state[charger_idx] != RESOLVE_STATE_RESOLVED || dest_addrs[charger_idx].sin_addr.s_addr != in) {
        logger.printfln("Resolved %s to %s", hostnames[charger_idx].c_str(), ipaddr_ntoa(&ip));
    }

    dest_addrs[charger_idx].sin_addr.s_addr = in;
    resolve_state[charger_idx] = RESOLVE_STATE_RESOLVED;
}

bool CMNetworking::is_resolved(uint8_t charger_idx) {
    return resolve_state[charger_idx] == RESOLVE_STATE_RESOLVED;
}

static const uint8_t cm_command_packet_length_versions[] = {
    sizeof(struct cm_packet_header),
    sizeof(struct cm_packet_header) + sizeof(struct cm_command_v1),
};
static_assert(ARRAY_SIZE(cm_command_packet_length_versions) == (CM_PROTOCOL_VERSION + 1));

static const uint8_t cm_state_packet_length_versions[] = {
    sizeof(struct cm_packet_header),
    sizeof(struct cm_packet_header) + sizeof(struct cm_state_v1),
};
static_assert(ARRAY_SIZE(cm_state_packet_length_versions) == (CM_PROTOCOL_VERSION + 1));

String CMNetworking::validate_packet_header(const struct cm_packet_header *header, ssize_t recv_length) const
{
    if (recv_length < sizeof(struct cm_packet_header)) {
        return String("Truncated header with ") + recv_length + " bytes.";
    }

    if (header->magic != CM_PACKET_MAGIC) {
        return String("Invalid magic. Got ") + header->magic + '.';
    }

    if (header->version < CM_PROTOCOL_VERSION_MIN) {
        return String("Protocol version ") + header->version + " too old. Need at least version " __XSTRING(CM_PROTOCOL_VERSION_MIN) ".";
    }

    return String();
}

String CMNetworking::validate_command_packet_header(const struct cm_command_packet *pkt, ssize_t recv_length) const
{
    String err = validate_packet_header(&(pkt->header), recv_length);
    if (err != "")
        return err;

    if (((pkt->header.version > CM_PROTOCOL_VERSION) && (pkt->header.length < cm_command_packet_length_versions[CM_PROTOCOL_VERSION]))  // Newer protocol than known. Packet must be at least as long as our newest known version.
        || (pkt->header.length != cm_command_packet_length_versions[pkt->header.version])) {                                            // Match length of known protocol version.
        return String("Invalid packet length for protocol version ") + pkt->header.version + ": " + pkt->header.length + " bytes.";
    }

    if (((pkt->header.version > CM_PROTOCOL_VERSION) && (recv_length < cm_command_packet_length_versions[CM_PROTOCOL_VERSION])) // Newer protocol than known. Packet must be at least as long as our newest known version.
        || (recv_length != cm_command_packet_length_versions[pkt->header.version])) {                                           // Match length of known protocol version.
        return String("Received truncated packet for protocol version ") + pkt->header.version + ": " + recv_length + " bytes.";
    }

    return String();
}

String CMNetworking::validate_state_packet_header(const struct cm_state_packet *pkt, ssize_t recv_length) const
{
    String err = validate_packet_header(&(pkt->header), recv_length);
    if (err != "")
        return err;

    if (((pkt->header.version > CM_PROTOCOL_VERSION) && (pkt->header.length < cm_state_packet_length_versions[CM_PROTOCOL_VERSION]))    // Newer protocol than known. Packet must be at least as long as our newest known version.
        || (pkt->header.length != cm_state_packet_length_versions[pkt->header.version])) {                                              // Match length of known protocol version.
        return String("Invalid packet length for protocol version ") + pkt->header.version + ": " + pkt->header.length + " bytes.";
    }

    if (((pkt->header.version > CM_PROTOCOL_VERSION) && (recv_length < cm_state_packet_length_versions[CM_PROTOCOL_VERSION]))    // Newer protocol than known. Packet must be at least as long as our newest known version.
        || (recv_length != cm_state_packet_length_versions[pkt->header.version])) {                                              // Match length of known protocol version.
        return String("Received truncated packet for protocol version ") + pkt->header.version + ": " + recv_length + " bytes.";
    }

    return String();
}

bool CMNetworking::seq_num_invalid(uint16_t received_sn, uint16_t last_seen_sn) const
{
    return received_sn <= last_seen_sn && last_seen_sn - received_sn < 5;
}

void CMNetworking::register_manager(std::vector<String> &&hosts,
                                    const std::vector<String> &names,
                                    std::function<void(uint8_t,  // client_id
                                                       uint8_t,  // iec61851_state
                                                       uint8_t,  // charger_state
                                                       uint8_t,  // error_state
                                                       uint32_t, // uptime
                                                       uint32_t, // charging_time
                                                       uint16_t, // allowed_charging_current
                                                       uint16_t, // supported_current
                                                       bool,     // cp_disconnect_supported
                                                       bool      // cp_disconnected_state
                                                       )> manager_callback,
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

        int len = recvfrom(manager_sock, &state_pkt, sizeof(state_pkt), 0, (sockaddr *)&source_addr, &socklen);

        if (len < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                logger.printfln("recvfrom failed: errno %d", errno);
            return;
        }

        int charger_idx = -1;
        for(int i = 0; i < names.size(); ++i)
            if (source_addr.sin_family == dest_addrs[i].sin_family &&
                source_addr.sin_port == dest_addrs[i].sin_port &&
                source_addr.sin_addr.s_addr == dest_addrs[i].sin_addr.s_addr) {
                charger_idx = i;
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

        manager_callback(charger_idx,
                         state_pkt.v1.iec61851_state,
                         state_pkt.v1.charger_state,
                         state_pkt.v1.error_state,
                         state_pkt.v1.evse_uptime,
                         state_pkt.v1.charging_time,
                         state_pkt.v1.allowed_charging_current,
                         state_pkt.v1.supported_current,
                         CM_FEATURE_FLAGS_CP_DISCONNECT_IS_SET(state_pkt.v1.feature_flags),
                         CM_STATE_FLAGS_CP_DISCONNECTED_IS_SET(state_pkt.v1.state_flags));
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
    command_pkt.header.version = CM_PROTOCOL_VERSION;

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
            logger.printfln("Received command packet from %s (%iB) failed validation: %s",
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

        last_successful_recv = millis();
        manager_addr = temp_addr;
        manager_addr_valid = true;

        client_callback(command_pkt.v1.allocated_current, CM_COMMAND_FLAGS_CPPDISC_IS_SET(command_pkt.v1.command_flags));
        //logger.printfln("Received command packet. Allocated current is %u", command_pkt.v1.allocated_current);
    }, 100, 100);
}

bool CMNetworking::send_client_update(uint8_t iec61851_state,
                                      uint8_t charger_state,
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
    state_pkt.header.version = CM_PROTOCOL_VERSION;

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
        state_pkt.v1.energy_rel = meter_values->get("energy_rel")->asFloat();
        state_pkt.v1.energy_abs = meter_values->get("energy_abs")->asFloat();
    } else {
        state_pkt.v1.energy_rel = 0;
        state_pkt.v1.energy_abs = 0;
    }

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

bool CMNetworking::check_results()
{
    {
        std::lock_guard<std::mutex> lock{scan_results_mutex};
        if (!mdns_query_async_get_results(scan, 0, &scan_results))
            return false; // This should never happen as check_results is only called if we are notified the search has finished.
    }

    mdns_query_async_delete(scan);

    scanning = false;

#if MODULE_WS_AVAILABLE()
    String s = get_scan_results();
    ws.pushRawStateUpdate(s, "charge_manager/scan_result");
#endif
    return true;
}

void CMNetworking::start_scan()
{
    if (scanning)
        return;
    scanning = true;

    {
        std::lock_guard<std::mutex> lock{scan_results_mutex};
        if (scan_results != nullptr)
        {
            mdns_query_results_free(scan_results);
            scan_results = nullptr;
        }
    }

    scan = mdns_query_async_new(NULL, "_tf-warp-cm", "_udp", MDNS_TYPE_PTR, 1000, INT8_MAX, [](mdns_search_once_t *search) {
        task_scheduler.scheduleOnce([](){ cm_networking.check_results(); }, 0);
    });
}

void CMNetworking::add_scan_result_entry(mdns_result_t *entry, TFJsonSerializer &json)
{
    const char *version = "0";
    const char *enabled = "false";
    const char *display_name = "[no_display_name]";

    if (entry->txt_count < 3)
        return;

    int found = 0;
    for(size_t i = 0; i < entry->txt_count; ++i) {
        if (strcmp(entry->txt[i].key, "enabled") == 0 && entry->txt_value_len[i] > 0) {
            enabled = entry->txt[i].value;
            ++found;
        }
        else if (strcmp(entry->txt[i].key, "display_name") == 0 && entry->txt_value_len[i] > 0) {
            display_name = entry->txt[i].value;
            ++found;
        }
        else if (strcmp(entry->txt[i].key, "version") == 0 && entry->txt_value_len[i] > 0) {
            version = entry->txt[i].value;
            ++found;
        }
    }

    if (found < 3)
        return;

    uint8_t error = SCAN_RESULT_ERROR_OK;

    if (strcmp(enabled, "true") != 0) {
        error = SCAN_RESULT_ERROR_MANAGEMENT_DISABLED;
    } else {
        const char *protocol_version = strchr(version, '.');
        if (!protocol_version) {
            error = SCAN_RESULT_ERROR_FIRMWARE_MISMATCH;
        } else {
            if (strncmp(version, __XSTRING(CM_PACKET_MAGIC), protocol_version - version) != 0) {
                error = SCAN_RESULT_ERROR_FIRMWARE_MISMATCH;
            } else {
                long num_version = strtol(++protocol_version, nullptr, 10);
                if (num_version < CM_PROTOCOL_VERSION_MIN) {
                    error = SCAN_RESULT_ERROR_FIRMWARE_MISMATCH;
                }
            }
        }
    }

    json.addObject();
        json.add("hostname", entry->hostname);

        char buf[32] = "[no_address]";
        if (entry->addr && entry->addr->addr.type == IPADDR_TYPE_V4)
            esp_ip4addr_ntoa(&entry->addr->addr.u_addr.ip4, buf, ARRAY_SIZE(buf));
        json.add("ip", buf);

        json.add("display_name", display_name);
        json.add("error", error);
    json.endObject();
}

size_t CMNetworking::build_scan_result_json(mdns_result_t *list, char *buf, size_t len) {
    TFJsonSerializer json{buf, len};
    json.addArray();

    while (list != nullptr) {
        add_scan_result_entry(list, json);
        list = list->next;
    }

    json.endArray();
    return json.end();
}

String CMNetworking::get_scan_results()
{
    std::lock_guard<std::mutex> lock{scan_results_mutex};
    if (scan_results == nullptr)
        return "In progress or not started";

    size_t payload_size = build_scan_result_json(scan_results, nullptr, 0);

    CoolString result;
    result.reserve(payload_size);

    build_scan_result_json(scan_results, result.begin(), payload_size);
    result.setLength(payload_size);

    return result;
}
