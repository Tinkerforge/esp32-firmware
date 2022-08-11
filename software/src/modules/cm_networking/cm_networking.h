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

#pragma once

#include "config.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <functional>

#define CHARGE_MANAGER_PORT 34127
#define CHARGE_MANAGEMENT_PORT (CHARGE_MANAGER_PORT + 1)

// Keep in sync with charge_manager.cpp
#define MAX_CLIENTS 10

// Increment when changing packet structs
#define PROTOCOL_VERSION 3

#define CM_NETWORKING_ERROR_NO_ERROR 0
#define CM_NETWORKING_ERROR_UNREACHABLE 1
#define CM_NETWORKING_ERROR_FW_MISMATCH 2
#define CM_NETWORKING_ERROR_NOT_MANAGED 3

struct packet_header {
    uint8_t seq_num;
    uint8_t version;
    uint16_t padding;
} __attribute__((packed));

struct request_packet {
    packet_header header;

    uint16_t allocated_current;
} __attribute__((packed));

struct response_packet {
    packet_header header;

    uint8_t iec61851_state;
    uint8_t charger_state;
    uint8_t error_state;
    uint32_t uptime;
    uint32_t charging_time;
    uint16_t allowed_charging_current;
    uint16_t supported_current;
    bool managed;
} __attribute__((packed));

class CMNetworking
{
public:
    CMNetworking();
    void setup();
    void register_urls();
    void loop();

    bool initialized = false;

    int create_socket(uint16_t port);

    void register_manager(const std::vector<String> &hosts,
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
                          std::function<void(uint8_t, uint8_t)> manager_error_callback);

    bool send_manager_update(uint8_t client_id, uint16_t allocated_current);

    void register_client(std::function<void(uint16_t)> client_callback);
    bool send_client_update(uint8_t iec61851_state,
                            uint8_t charger_state,
                            uint8_t error_state,
                            uint32_t uptime,
                            uint32_t charging_time,
                            uint16_t allowed_charging_current,
                            uint16_t supported_current,
                            bool managed);

private:
    int manager_sock;
    struct sockaddr_in dest_addrs[MAX_CLIENTS] = {};

    int client_sock;
    bool source_addr_valid = false;
    struct sockaddr_storage source_addr;
};
