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

#include "mdns.h"
#include "TFJson.h"

#include <functional>

#define CHARGE_MANAGER_PORT 34127
#define CHARGE_MANAGEMENT_PORT (CHARGE_MANAGER_PORT + 1)

// Keep in sync with charge_manager.cpp
#define MAX_CLIENTS 10

// Increment when changing packet structs
#define CM_PROTOCOL_VERSION 1

// Minimum protocol version supported
#define CM_PROTOCOL_VERSION_MIN 1

#define CM_PACKET_MAGIC 34127

#define CM_NETWORKING_ERROR_NO_ERROR 0
#define CM_NETWORKING_ERROR_UNREACHABLE 1
#define CM_NETWORKING_ERROR_INVALID_HEADER 2
#define CM_NETWORKING_ERROR_NOT_MANAGED 3

struct cm_packet_header {
    uint16_t magic;
    uint16_t length;
    uint16_t seq_num;
    uint8_t version;
    uint8_t padding;
} __attribute__((packed));

#define CM_PACKET_HEADER_LENGTH (sizeof(cm_packet_header))
static_assert(CM_PACKET_HEADER_LENGTH == 8);

#define CM_COMMAND_FLAGS_CPPDISC_BIT_POS 6
#define CM_COMMAND_FLAGS_CPPDISC_MASK (1 << CM_COMMAND_FLAGS_CPPDISC_BIT_POS)
#define CM_COMMAND_FLAGS_CPPDISC_IS_SET(FLAGS) (((FLAGS) & CM_COMMAND_FLAGS_CPPDISC_MASK) != 0)

struct cm_command_v1 {
    uint16_t allocated_current;
    /* command_flags
    bit 6 - control pilot permanently disconnected
    Other bits must be sent unset and ignored on reception.
    */
    uint8_t command_flags;
    uint8_t padding;
} __attribute__((packed));

#define CM_COMMAND_V1_LENGTH (sizeof(cm_command_v1))
static_assert(CM_COMMAND_V1_LENGTH == 4);

struct cm_command_packet {
    cm_packet_header header;
    cm_command_v1 v1;
} __attribute__((packed));

#define CM_COMMAND_PACKET_LENGTH (sizeof(cm_command_packet))
static_assert(CM_COMMAND_PACKET_LENGTH == 12);

#define CM_FEATURE_FLAGS_CP_DISCONNECT_BIT_POS 6
#define CM_FEATURE_FLAGS_CP_DISCONNECT_MASK (1 << CM_FEATURE_FLAGS_CP_DISCONNECT_BIT_POS)
#define CM_FEATURE_FLAGS_CP_DISCONNECT_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_CP_DISCONNECT_MASK) != 0)
#define CM_FEATURE_FLAGS_EVSE_BIT_POS 5
#define CM_FEATURE_FLAGS_EVSE_MASK (1 << CM_FEATURE_FLAGS_EVSE_BIT_POS)
#define CM_FEATURE_FLAGS_EVSE_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_EVSE_MASK) != 0)
#define CM_FEATURE_FLAGS_NFC_BIT_POS 4
#define CM_FEATURE_FLAGS_NFC_MASK (1 << CM_FEATURE_FLAGS_NFC_BIT_POS)
#define CM_FEATURE_FLAGS_NFC_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_NFC_MASK) != 0)
#define CM_FEATURE_FLAGS_METER_ALL_VALUES_BIT_POS 3
#define CM_FEATURE_FLAGS_METER_ALL_VALUES_MASK (1 << CM_FEATURE_FLAGS_METER_ALL_VALUES_BIT_POS)
#define CM_FEATURE_FLAGS_METER_ALL_VALUES_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_METER_ALL_VALUES_MASK) != 0)
#define CM_FEATURE_FLAGS_METER_PHASES_BIT_POS 2
#define CM_FEATURE_FLAGS_METER_PHASES_MASK (1 << CM_FEATURE_FLAGS_METER_PHASES_BIT_POS)
#define CM_FEATURE_FLAGS_METER_PHASES_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_METER_PHASES_MASK) != 0)
#define CM_FEATURE_FLAGS_METER_BIT_POS 1
#define CM_FEATURE_FLAGS_METER_MASK (1 << CM_FEATURE_FLAGS_METER_BIT_POS)
#define CM_FEATURE_FLAGS_METER_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_METER_MASK) != 0)
#define CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_BIT_POS 0
#define CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_MASK (1 << CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_BIT_POS)
#define CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_MASK) != 0)

#define CM_STATE_FLAGS_MANAGED_BIT_POS 7
#define CM_STATE_FLAGS_MANAGED_MASK (1 << CM_STATE_FLAGS_MANAGED_BIT_POS)
#define CM_STATE_FLAGS_MANAGED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_MANAGED_MASK) != 0)
#define CM_STATE_FLAGS_CP_DISCONNECTED_BIT_POS 6
#define CM_STATE_FLAGS_CP_DISCONNECTED_MASK (1 << CM_STATE_FLAGS_CP_DISCONNECTED_BIT_POS)
#define CM_STATE_FLAGS_CP_DISCONNECTED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_CP_DISCONNECTED_MASK) != 0)
#define CM_STATE_FLAGS_L1_CONNECTED_BIT_POS 5
#define CM_STATE_FLAGS_L1_CONNECTED_MASK (1 << CM_STATE_FLAGS_L1_CONNECTED_BIT_POS)
#define CM_STATE_FLAGS_L1_CONNECTED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L1_CONNECTED_MASK) != 0)
#define CM_STATE_FLAGS_L2_CONNECTED_BIT_POS 4
#define CM_STATE_FLAGS_L2_CONNECTED_MASK (1 << CM_STATE_FLAGS_L2_CONNECTED_BIT_POS)
#define CM_STATE_FLAGS_L2_CONNECTED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L2_CONNECTED_MASK) != 0)
#define CM_STATE_FLAGS_L3_CONNECTED_BIT_POS 3
#define CM_STATE_FLAGS_L3_CONNECTED_MASK (1 << CM_STATE_FLAGS_L3_CONNECTED_BIT_POS)
#define CM_STATE_FLAGS_L3_CONNECTED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L3_CONNECTED_MASK) != 0)
#define CM_STATE_FLAGS_CONNECTED_BIT_POS 3
#define CM_STATE_FLAGS_CONNECTED_MASK (0x7 << CM_STATE_FLAGS_CONNECTED_BIT_POS)
#define CM_STATE_FLAGS_CONNECTED_GET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_CONNECTED_MASK) >> 3)
#define CM_STATE_FLAGS_L1_ACTIVE_BIT_POS 2
#define CM_STATE_FLAGS_L1_ACTIVE_MASK (1 << CM_STATE_FLAGS_L1_ACTIVE_BIT_POS)
#define CM_STATE_FLAGS_L1_ACTIVE_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L1_ACTIVE_MASK) != 0)
#define CM_STATE_FLAGS_L2_ACTIVE_BIT_POS 1
#define CM_STATE_FLAGS_L2_ACTIVE_MASK (1 << CM_STATE_FLAGS_L2_ACTIVE_BIT_POS)
#define CM_STATE_FLAGS_L2_ACTIVE_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L2_ACTIVE_MASK) != 0)
#define CM_STATE_FLAGS_L3_ACTIVE_BIT_POS 0
#define CM_STATE_FLAGS_L3_ACTIVE_MASK (1 << CM_STATE_FLAGS_L3_ACTIVE_BIT_POS)
#define CM_STATE_FLAGS_L3_ACTIVE_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L3_ACTIVE_MASK) != 0)
#define CM_STATE_FLAGS_ACTIVE_BIT_POS 0
#define CM_STATE_FLAGS_ACTIVE_MASK (0x7 << CM_STATE_FLAGS_ACTIVE_BIT_POS)
#define CM_STATE_FLAGS_ACTIVE_GET(FLAGS) ((FLAGS) & CM_STATE_FLAGS_ACTIVE_MASK)

struct cm_state_v1 {
    /* feature_flags
    bit 6 - has cp_disconnect
    bit 5 - has evse
    bit 4 - has nfc
    bit 3 - has meter_all_values
    bit 2 - has meter_phases
    bit 1 - has meter
    bit 0 - has button_configuration
    Other bits must be sent unset and ignored on reception.
    */
    uint32_t feature_flags;
    uint32_t evse_uptime;
    uint32_t charging_time;
    uint16_t allowed_charging_current;
    uint16_t supported_current;
    
    uint8_t iec61851_state;
    uint8_t charger_state;
    uint8_t error_state;    
    /* state_flags
    bit 7 - managed
    bit 6 - control_pilot_permanently_disconnected
    bit 5 - L1_connected
    bit 4 - L2_connected
    bit 3 - L3_connected
    bit 2 - L1_active
    bit 1 - L2_active
    bit 0 - L3_active
    */
    uint8_t state_flags;
    float line_voltages[3];
    float line_currents[3];
    float line_power_factors[3];
    float power_total;
    float energy_rel;
    float energy_abs;
} __attribute__((packed));

#define CM_STATE_V1_LENGTH (sizeof(cm_state_v1))
static_assert(CM_STATE_V1_LENGTH == 68);

struct cm_state_packet {
    cm_packet_header header;
    cm_state_v1 v1;
} __attribute__((packed));

#define CM_STATE_PACKET_LENGTH (sizeof(cm_state_packet))
static_assert(CM_STATE_PACKET_LENGTH == 76);

class CMNetworking
{
public:
    CMNetworking(){}
    void pre_setup();
    void setup();
    void register_urls();
    void loop();

    bool initialized = false;

    int create_socket(uint16_t port);

    void register_manager(std::vector<String> &&hosts,
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
                          std::function<void(uint8_t, uint8_t)> manager_error_callback);

    bool send_manager_update(uint8_t client_id, uint16_t allocated_current, bool cp_disconnect_requested);

    void register_client(std::function<void(uint16_t, bool)> client_callback);
    bool send_client_update(uint8_t iec61851_state,
                            uint8_t charger_state,
                            uint8_t error_state,
                            uint32_t uptime,
                            uint32_t charging_time,
                            uint16_t allowed_charging_current,
                            uint16_t supported_current,
                            bool managed,
                            bool cp_disconnected_state);

    String get_scan_results();

    void resolve_hostname(uint8_t charger_idx);
    bool is_resolved(uint8_t charger_idx);

    String validate_packet_header(const struct cm_packet_header *header, ssize_t recv_length) const;
    String validate_command_packet_header(const struct cm_command_packet *pkt, ssize_t recv_length) const;
    String validate_state_packet_header(const struct cm_state_packet *pkt, ssize_t recv_length) const;
    bool seq_num_invalid(uint16_t received_sn, uint16_t last_seen_sn) const;

    bool check_results();

    bool scanning = false;

    mdns_search_once_t *scan;

    std::mutex scan_results_mutex;
    std::mutex dns_resolve_mutex;
    mdns_result_t *scan_results = nullptr;

private:
    int manager_sock;

    #define RESOLVE_STATE_UNKNOWN 0
    #define RESOLVE_STATE_NOT_RESOLVED 1
    #define RESOLVE_STATE_RESOLVED 2

    uint8_t resolve_state[MAX_CLIENTS] = {};
    struct sockaddr_in dest_addrs[MAX_CLIENTS] = {};
    std::vector<String> hostnames;

    int client_sock;
    bool manager_addr_valid = false;
    struct sockaddr_storage manager_addr;

    void start_scan();

    #define SCAN_RESULT_ERROR_OK 0
    #define SCAN_RESULT_ERROR_FIRMWARE_MISMATCH 1
    #define SCAN_RESULT_ERROR_MANAGEMENT_DISABLED 2
    void add_scan_result_entry(mdns_result_t *entry, TFJsonSerializer &json);
    size_t build_scan_result_json(mdns_result_t *list, char *buf, size_t len);
};
