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

extern TaskScheduler task_scheduler;
extern API api;
extern EventLog logger;
extern WebServer server;

CMNetworking::CMNetworking()
{
    scan_cfg = Config::Null();
}

void CMNetworking::setup()
{
    mdns_init();
    initialized = true;
}

void CMNetworking::register_urls()
{
    api.addCommand("charge_manager/scan", &scan_cfg, {}, [this]() {
        start_scan();
    }, true);

    server.on("/charge_manager/scan_result", HTTP_GET, [this](WebServerRequest request) {
        String result = cm_networking.get_scan_results();

        if (result == "In progress or not started")
            request.send(200, "text/plain; charset=utf-8", result.c_str());

        request.send(200, "application/json; charset=utf-8", result.c_str());
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

void CMNetworking::register_manager(const std::vector<String> &hosts,
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

    for (int i = 0; i < names.size(); ++i) {
        dest_addrs[i].sin_addr.s_addr = inet_addr(hosts[i].c_str());
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

        if (charger_idx == -1) {
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

    int err = sendto(manager_sock, &request, sizeof(request), 0, (sockaddr *)&dest_addrs[client_id], sizeof(dest_addrs[client_id]));

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
        request_packet recv_buf[2] = {};

        struct sockaddr_storage temp_addr;
        socklen_t socklen = sizeof(temp_addr);
        int len = recvfrom(client_sock, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&temp_addr, &socklen);

        if (len < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                logger.printfln("recvfrom failed: errno %d", errno);
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
        source_addr_valid = false;

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

bool CMNetworking::check_txt_entries(mdns_result_t *entry)
{
    String version = "0";
    String enabled = "false";
    display_name = "[no_display_name]";
    error = "3";

    if (entry->txt_count < 3)
        return false;

    if (String(entry->txt[0].key) == "enabled" && entry->txt_value_len[0] > 0)
        enabled = entry->txt[0].value;
    else
        return false;

    if (String(entry->txt[1].key) == "display_name" && entry->txt_value_len[1] > 0)
        display_name = entry->txt[1].value;
    else
        return false;

    if (String(entry->txt[2].key) == "version" && entry->txt_value_len[2] > 0)
        version = entry->txt[2].value;
    else
        return false;

    if ((version == String(PROTOCOL_VERSION)) == 0)
        error = "1";
    else if (enabled == "false")
        error = "2";
    else
        error = "0";
    return true;
}

String CMNetworking::get_scan_results()
{
    std::lock_guard<std::mutex> lock{scan_results_mutex};
    if (scan_results == nullptr)
        return "In progress or not started";

    String result = "No services found.";

    mdns_result_t *list = scan_results;

    result = "";

    result += "[";

    for (int i = 0; list; i++)
    {
        if (!check_txt_entries(list))
        {
            list = list->next;
            continue;
        }

        if (i != 0)
            result += ", ";

        char buff[32] = "[no address]";
        if (list->addr && list->addr->addr.type == IPADDR_TYPE_V4)
           esp_ip4addr_ntoa(&list->addr->addr.u_addr.ip4, buff, 32);

        result += "{\"hostname\": \"";
        result += list->hostname;
        result += "\", \"ip\": \"";
        result += buff;
        result += "\", \"display_name\": \"";
        result += display_name;
        result += "\", \"error\": \"";
        result += error;
        result += "\"}";

        list = list->next;
    }
    result += "]";

    return result;
}
