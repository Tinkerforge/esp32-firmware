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

#include <FS.h> // FIXME: without this include here there is a problem with the IPADDR_NONE define in <lwip/ip4_addr.h>
#include <array>
#include <functional>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <mdns.h>
#include <TFJson.h>

#include "module.h"
#include "config.h"
#include "cm_networking_defs.h"
#include "TFTools/Micros.h"

struct cm_state_v1;
struct cm_state_v2;
struct cm_state_v3;

class CMNetworking final : public IModule
{
public:
    CMNetworking(){}
    void register_urls() override;
    void register_events() override;

    void register_manager(const char *const *const hosts,
                          size_t device_count,
                          const std::function<void(uint8_t /* client_id */, cm_state_v1 *, cm_state_v2 *, cm_state_v3 *, cm_state_v4 *)> &manager_callback,
                          const std::function<void(uint8_t, CASError)> &manager_error_callback);

    bool send_manager_update(uint8_t client_id, uint16_t allocated_current, bool cp_disconnect_requested, int8_t allocated_phases, uint8_t charge_mode, std::array<uint8_t, 2> supported_charge_mode_bitmask);

    void register_client(const std::function<void(uint16_t, bool, int8_t, uint8_t, uint8_t *, size_t)> &client_callback);
    bool send_client_update(uint32_t esp32_uid,
                            uint8_t iec61851_state,
                            uint8_t charger_state,
                            uint32_t time_since_state_change,
                            uint8_t error_state,
                            uint32_t uptime,
                            uint32_t charging_time,
                            uint16_t allowed_charging_current,
                            uint16_t supported_current,
                            bool managed,
                            bool cp_disconnected_state,
                            int8_t phases,
                            bool can_switch_phases_now,
                            uint8_t requested_charge_mode);

    void notify_charger_unresponsive(uint8_t charger_idx);

    void dns_callback(const ip_addr_t *ip, void *callback_arg);

private:
    enum class HostAddressType : uint8_t {
        IP,
        mDNS,
        DNS,
    };

    enum class ResolveState : uint8_t {
        Unknown,
        NotResolved,
        Resolved,
        Stale,
    };

    struct managed_device_data {
        micros_t last_resolve_attempt;
        sockaddr_in addr;
        const char *hostname;
        uint8_t mdns_hostname_len; // Length of the hostname without the .local TLD.
        HostAddressType host_address_type;
        ResolveState resolve_state;
        uint8_t device_index;
    };

    struct manager_data_t {
        int manager_sock;

        bool connected;
        bool dns_resolver_active;

        uint8_t managed_device_count;
        managed_device_data managed_devices[];
    };
    static_assert(MAX_CONTROLLED_CHARGERS < std::numeric_limits<decltype(manager_data_t::managed_device_count)>::max());

    bool send_command_packet(uint8_t charger_idx, cm_command_packet *command_pkt);
    bool send_state_packet(const cm_state_packet *state_pkt);

    // Always allow mDNS scans
    std::mutex scan_results_mutex;
    mdns_result_t *scan_results = nullptr;
    mdns_search_once_t *mdns_scan = nullptr;
    bool mdns_scan_active = false;

    // For managers
    manager_data_t *manager_data = nullptr;

    // For clients
    micros_t last_manager_addr_change = -1_min;
    int client_sock;
    bool manager_addr_valid = false;
    struct sockaddr_storage manager_addr;

    int create_socket(uint16_t port, bool blocking);

    void dns_resolved(managed_device_data *device, const ip_addr_t *ip);
    void resolve_hostname(uint8_t charger_idx);
    void resolve_hostname_dns(uint8_t charger_idx, bool initial_request);
    void resolve_next_dns(uint8_t current_idx);
    bool is_resolved(uint8_t charger_idx);
    void resolve_all();

    static void check_mdns_results_cb(mdns_search_once_t *search);
    void check_mdns_results();

    void start_mdns_scan();
    bool mdns_result_is_charger(mdns_result_t *entry, const char **ret_version, const char **ret_enabled, const char **ret_display_name, const char **ret_proxy_of);
    void resolve_via_mdns(mdns_result_t *entry);
    bool get_scan_results(CoolString &result);

    #define SCAN_RESULT_ERROR_OK 0
    #define SCAN_RESULT_ERROR_FIRMWARE_MISMATCH 1
    #define SCAN_RESULT_ERROR_MANAGEMENT_DISABLED 2
    void add_scan_result_entry(mdns_result_t *entry, TFJsonSerializer &json);
    size_t build_scan_result_json(mdns_result_t *list, char *buf, size_t len);
};
