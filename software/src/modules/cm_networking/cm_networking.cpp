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
    MDNS.addService("tf-warp-cm", "udp", 34127);
    MDNS.addServiceTxt("tf-warp-cm", "udp", "version", String(PROTOCOL_VERSION));
    task_scheduler.scheduleWithFixedDelay([](){
        #if MODULE_DEVICE_NAME_AVAILABLE()
            MDNS.addServiceTxt("tf-warp-cm", "udp", "display_name", device_name.display_name.get("display_name")->asString());
        #endif

            bool management_enabled = false;
        #if MODULE_EVSE_AVAILABLE()
            management_enabled = evse.evse_management_enabled.get("enabled")->asBool();
        #elif MODULE_EVSE_V2_AVAILABLE()
            management_enabled = evse_v2.evse_management_enabled.get("enabled")->asBool();
        #endif

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

    // using memcpy to guaranty alignment https://mail.gnu.org/archive/html/lwip-users/2008-08/msg00166.html
    std::memcpy(&in, &ip.u_addr, sizeof(ip4_addr_t));

    std::lock_guard<std::mutex> lock{dns_resolve_mutex};
    if (resolve_state[charger_idx] != RESOLVE_STATE_RESOLVED || dest_addrs[charger_idx].sin_addr.s_addr != in) {
        logger.printfln("Resolved %s to %s", hostnames[charger_idx].c_str(), ipaddr_ntoa(&ip));
    }

    dest_addrs[charger_idx].sin_addr.s_addr = in;
    resolve_state[charger_idx] = RESOLVE_STATE_RESOLVED;
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
                                                       uint16_t  // supported_current
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
        static uint8_t last_seen_seq_num[MAX_CLIENTS];
        static bool initialized = false;
        if (!initialized) {
            memset(last_seen_seq_num, 255, MAX_CLIENTS);
            initialized = true;
        }

        response_packet recv_buf[2] = {};
        struct sockaddr_in source_addr;
        socklen_t socklen = sizeof(source_addr);

        int len = recvfrom(manager_sock, recv_buf, sizeof(recv_buf), 0, (sockaddr *)&source_addr, &socklen);

        if (len < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                logger.printfln("recvfrom failed: errno %d", errno);
            return;
        }

        if (len != sizeof(response_packet)) {
            logger.printfln("Received datagram of wrong size %d from %s", len, inet_ntoa(source_addr.sin_addr));
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

        response_packet response;
        memcpy(&response, recv_buf, sizeof(response));

        if (response.header.seq_num <= last_seen_seq_num[charger_idx] && last_seen_seq_num[charger_idx] - response.header.seq_num < 5) {
            logger.printfln("Received stale (out of order?) packet from %s (%s). Last seen seq_num is %u, Received seq_num is %u",
                names[charger_idx].c_str(),
                inet_ntoa(source_addr.sin_addr),
                last_seen_seq_num[charger_idx],
                response.header.seq_num);
            return;
        }

        if (response.header.version != PROTOCOL_VERSION) {
            manager_error_callback(charger_idx, CM_NETWORKING_ERROR_FW_MISMATCH);
            logger.printfln("Received packet from %s (%s) with incompatible firmware. Our protocol version is %u, received packet had %u",
                names[charger_idx].c_str(),
                inet_ntoa(source_addr.sin_addr),
                PROTOCOL_VERSION,
                response.header.version);
            return;
        }

        last_seen_seq_num[charger_idx] = response.header.seq_num;

        if (!response.managed) {
            manager_error_callback(charger_idx, CM_NETWORKING_ERROR_NOT_MANAGED);
            logger.printfln("%s (%s) reports managed is not activated!",
                names[charger_idx].c_str(),
                inet_ntoa(source_addr.sin_addr));
            return;
        }

        manager_callback(charger_idx,
                         response.iec61851_state,
                         response.charger_state,
                         response.error_state,
                         response.uptime,
                         response.charging_time,
                         response.allowed_charging_current,
                         response.supported_current);
        }, 100, 100);
}

bool CMNetworking::send_manager_update(uint8_t client_id, uint16_t allocated_current)
{
    static uint8_t next_seq_num = 1;

    if (manager_sock < 0)
        return true;

    request_packet request;
    request.header.version = PROTOCOL_VERSION;
    request.header.seq_num = next_seq_num;
    ++next_seq_num;

    request.allocated_current = allocated_current;

    int err = -1;


    resolve_hostname(client_id);
    err = sendto(manager_sock, &request, sizeof(request), 0, (sockaddr *)&dest_addrs[client_id], sizeof(dest_addrs[client_id]));

    if (err < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            // Intentionally don't increment here, we want to resend to this charger next.
            return false;
        if (errno == ENOMEM) {
            // Ignore ENOMEM for now. Usually indicates that we don't have a network connection yet.
            return true;
        }

        logger.printfln("Failed to send: %s %d", strerror(errno), errno);
        return true;
    }
    if (err != sizeof(request)) {
        logger.printfln("Failed to send. sendto truncated request (of %u bytes) to %d bytes.", sizeof(request), err);
        return true;
    }
    return true;
}

void CMNetworking::register_client(std::function<void(uint16_t)> client_callback)
{
    client_sock = create_socket(CHARGE_MANAGEMENT_PORT);

    if (client_sock < 0)
        return;

    memset(&source_addr, 0, sizeof(source_addr));

    task_scheduler.scheduleWithFixedDelay([this, client_callback](){
        static uint8_t last_seen_seq_num = 255;
        static uint32_t last_successful_recv = millis();

        request_packet recv_buf[2] = {};

        struct sockaddr_storage temp_addr;
        socklen_t socklen = sizeof(temp_addr);
        int len = recvfrom(client_sock, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&temp_addr, &socklen);

        if (len < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                logger.printfln("recvfrom failed: errno %d", errno);

            // If we have not received a valid packet for one minute, devalidate source_addr.
            // Otherwise we would send response packets to this address forever.
            if (deadline_elapsed(last_successful_recv + 60 * 1000))
                source_addr_valid = false;

            return;
        }

        if (len != sizeof(request_packet)) {
            logger.printfln("received datagram of wrong size %d", len);
            return;
        }

        request_packet request;
        memcpy(&request, recv_buf, sizeof(request));

        if (request.header.seq_num <= last_seen_seq_num && last_seen_seq_num - request.header.seq_num < 5) {
            logger.printfln("received stale (out of order?) packet. last seen seq_num is %u, received seq_num is %u", last_seen_seq_num, request.header.seq_num);
            return;
        }

        if (request.header.version != PROTOCOL_VERSION) {
            logger.printfln("received packet from box with incompatible firmware. Our protocol version is %u, received packet had %u",
                PROTOCOL_VERSION,
                request.header.version);
            return;
        }

        last_seen_seq_num = request.header.seq_num;

        last_successful_recv = millis();
        source_addr = temp_addr;

        source_addr_valid = true;
        client_callback(request.allocated_current);
        //logger.printfln("Received request. Allocated current is %u", request.allocated_current);
    }, 100, 100);
}

bool CMNetworking::send_client_update(uint8_t iec61851_state,
                                      uint8_t charger_state,
                                      uint8_t error_state,
                                      uint32_t uptime,
                                      uint32_t charging_time,
                                      uint16_t allowed_charging_current,
                                      uint16_t supported_current,
                                      bool managed)
{
    static uint8_t next_seq_num = 0;

    if (!source_addr_valid) {
        //logger.printfln("source addr not valid.");
        return false;
    }
    //logger.printfln("Sending response.");

    response_packet response;
    response.header.seq_num = next_seq_num;
    ++next_seq_num;
    response.header.version = PROTOCOL_VERSION;

    response.iec61851_state = iec61851_state;
    response.charger_state = charger_state;
    response.error_state = error_state;
    response.uptime = uptime;
    response.charging_time = charging_time;
    response.allowed_charging_current = allowed_charging_current;
    response.supported_current = supported_current;
    response.managed = managed;

    int err = sendto(client_sock, &response, sizeof(response), 0, (sockaddr *)&source_addr, sizeof(source_addr));
    if (err < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            logger.printfln("sendto failed: errno %d", errno);
        return false;
    }
    if (err != sizeof(response)) {
        logger.printfln("sendto truncated the response (of size %u bytes) to %d bytes.", sizeof(response), err);
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
        if (String(entry->txt[i].key) == "enabled" && entry->txt_value_len[i] > 0) {
            enabled = entry->txt[i].value;
            ++found;
        }
        else if (String(entry->txt[i].key) == "display_name" && entry->txt_value_len[i] > 0) {
            display_name = entry->txt[i].value;
            ++found;
        }
        else if (String(entry->txt[i].key) == "version" && entry->txt_value_len[i] > 0) {
            version = entry->txt[i].value;
            ++found;
        }
    }

    if (found < 3)
        return;

    uint8_t error = SCAN_RESULT_ERROR_OK;

    if (String(version) != String(PROTOCOL_VERSION))
        error = SCAN_RESULT_ERROR_FIRMWARE_MISMATCH;
    else if (String(enabled) != "true")
        error = SCAN_RESULT_ERROR_MANAGEMENT_DISABLED;

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
