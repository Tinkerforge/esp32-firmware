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

struct cm_state_v1;
struct cm_state_v2;
struct cm_state_v3;

class CMNetworking final : public IModule
{
public:
    CMNetworking(){}
    void setup() override;
    void register_urls() override;

    int create_socket(uint16_t port, bool blocking);

    void register_manager(const char *const *const hosts,
                          int charger_count,
                          std::function<void(uint8_t /* client_id */, cm_state_v1 *, cm_state_v2 *, cm_state_v3 *)> manager_callback,
                          std::function<void(uint8_t, uint8_t)> manager_error_callback);

    bool send_manager_update(uint8_t client_id, uint16_t allocated_current, bool cp_disconnect_requested, int8_t allocated_phases);

    void register_client(std::function<void(uint16_t, bool, int8_t)> client_callback);
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
                            bool can_switch_phases_now);

    bool get_scan_results(CoolString &result);

    void resolve_hostname(uint8_t charger_idx);
    bool is_resolved(uint8_t charger_idx);
    void clear_dns_cache_entry(uint8_t charger_idx);

    void check_results();

    bool scanning = false;
    bool periodic_scan_task_started = false;

    mdns_search_once_t *scan;

    std::mutex scan_results_mutex;
    std::mutex dns_resolve_mutex;
    mdns_result_t *scan_results = nullptr;

private:
    int manager_sock;

    #define RESOLVE_STATE_UNKNOWN 0
    #define RESOLVE_STATE_NOT_RESOLVED 1
    #define RESOLVE_STATE_RESOLVED 2

    uint8_t resolve_state[MAX_CONTROLLED_CHARGERS] = {};
    struct sockaddr_in dest_addrs[MAX_CONTROLLED_CHARGERS] = {};
    const char *const *hosts = nullptr;
    int charger_count = 0;
    // one bit per charger
    uint32_t needs_mdns = 0;
    static_assert(MAX_CONTROLLED_CHARGERS <= 32);

    int client_sock;
    bool manager_addr_valid = false;
    struct sockaddr_storage manager_addr;

    void start_scan();
    bool mdns_result_is_charger(mdns_result_t *entry, const char **ret_version, const char **ret_enabled, const char **ret_display_name);
    void resolve_via_mdns(mdns_result_t *entry);

    #define SCAN_RESULT_ERROR_OK 0
    #define SCAN_RESULT_ERROR_FIRMWARE_MISMATCH 1
    #define SCAN_RESULT_ERROR_MANAGEMENT_DISABLED 2
    void add_scan_result_entry(mdns_result_t *entry, TFJsonSerializer &json);
    size_t build_scan_result_json(mdns_result_t *list, char *buf, size_t len);
};
