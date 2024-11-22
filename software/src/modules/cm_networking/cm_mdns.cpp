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

#include <lwip/ip_addr.h>
#include <lwip/opt.h>
#include <lwip/dns.h>
#include <cstring>
#include <TFJson.h>

#include "mdns.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "tools/net.h"
#include "cool_string.h"

void CMNetworking::setup()
{
    initialized = true;
}

#if MODULE_EVSE_COMMON_AVAILABLE()
static void add_charger_services() {
    mdns_service_add(NULL, "_tf-warp-cm", "_udp", 34127, NULL, 0);
    mdns_service_txt_item_set("_tf-warp-cm", "_udp", "version", MACRO_VALUE_TO_STRING(CM_PACKET_MAGIC) "." MACRO_VALUE_TO_STRING(CM_STATE_VERSION));

    task_scheduler.scheduleWithFixedDelay([](){
#if MODULE_DEVICE_NAME_AVAILABLE()
        // Keep "display_name" updated because it can be changed at runtime without reboot.
        mdns_service_txt_item_set("_tf-warp-cm", "_udp", "display_name", device_name.display_name.get("display_name")->asEphemeralCStr());
#endif
        // Keep "enabled" updated because it is retrieved from the EVSE.
        mdns_service_txt_item_set("_tf-warp-cm", "_udp", "enabled", evse_common.get_management_enabled() ? "true" : "false");
    }, 10_s);
}
#endif

#if MODULE_EM_PHASE_SWITCHER_AVAILABLE()
static void add_wem_services() {

    static auto *cfg = api.getState("em_phase_switcher/charger_config");
    if (!cfg->get("proxy_mode")->asBool())
        return;

    mdns_service_add(NULL, "_tf-warp-cm", "_udp", 34127, NULL, 0);
    // TODO comment
    mdns_service_txt_item_set("_tf-warp-cm", "_udp", "version", MACRO_VALUE_TO_STRING(CM_PACKET_MAGIC) "." MACRO_VALUE_TO_STRING(CM_STATE_VERSION));
    mdns_service_txt_item_set("_tf-warp-cm", "_udp", "enabled", "true");

#if MODULE_DEVICE_NAME_AVAILABLE()
    task_scheduler.scheduleWithFixedDelay([](){
        // Keep "display_name" updated because it can be changed at runtime without reboot.
        mdns_service_txt_item_set("_tf-warp-cm", "_udp", "display_name", device_name.display_name.get("display_name")->asEphemeralCStr());
        mdns_service_txt_item_set("_tf-warp-cm", "_udp", "proxy_of", cfg->get("host")->asEphemeralCStr());
    }, 10_s);
#endif
}
#endif

void CMNetworking::register_urls()
{
    api.addCommand("charge_manager/scan", Config::Null(), {}, [this](String &/*errmsg*/) {
        start_scan();
    }, true);

    server.on_HTTPThread("/charge_manager/scan_result", HTTP_GET, [](WebServerRequest request) {
        CoolString result;

        if (!cm_networking.get_scan_results(result))
            return request.send(200, "text/plain; charset=utf-8", "In progress or not started");

        return request.send(200, "application/json; charset=utf-8", result.c_str());
    });
}

void CMNetworking::register_events() {
#if MODULE_NETWORK_AVAILABLE()
    if (!network.config.get("enable_mdns")->asBool())
        return;

#if MODULE_EVSE_COMMON_AVAILABLE()
    add_charger_services();
#elif MODULE_EM_PHASE_SWITCHER_AVAILABLE()
    add_wem_services();
#endif

#endif
}

static void dns_callback(const char *host, const ip_addr_t *ip, void *args)
{
    std::lock_guard<std::mutex> lock{cm_networking.dns_resolve_mutex};
    uint8_t *resolve_state = (uint8_t *)args;
    if (ip == nullptr && *resolve_state != RESOLVE_STATE_NOT_RESOLVED) {
        *resolve_state = RESOLVE_STATE_NOT_RESOLVED;
        logger.printfln("Failed to resolve %s", host);
    }
}

void CMNetworking::resolve_hostname(uint8_t charger_idx)
{
    if ((this->needs_mdns & (1ull << charger_idx)) != 0) {
        if (!periodic_scan_task_started)
            task_scheduler.scheduleWithFixedDelay([this](){this->start_scan();}, 1_m);
        periodic_scan_task_started = true;
        return;
    }

    ip_addr_t ip;
    int err = dns_gethostbyname_addrtype_lwip_ctx(this->hosts[charger_idx], &ip, dns_callback, &resolve_state[charger_idx], LWIP_DNS_ADDRTYPE_IPV4);

    if (err == ERR_VAL)
        logger.printfln("Charger configured with hostname %s, but no DNS server is configured!", this->hosts[charger_idx]);

    if (err != ERR_OK || ip.type != IPADDR_TYPE_V4)
        return;

    in_addr_t in;

    // using memcpy to guarantee alignment https://mail.gnu.org/archive/html/lwip-users/2008-08/msg00166.html
    std::memcpy(&in, &ip.u_addr, sizeof(ip4_addr_t));

    std::lock_guard<std::mutex> lock{dns_resolve_mutex};
    if (resolve_state[charger_idx] != RESOLVE_STATE_RESOLVED || dest_addrs[charger_idx].sin_addr.s_addr != in) {
        char ip_str[16];
        tf_ip4addr_ntoa(&ip, ip_str, sizeof(ip_str));
        // Show resolved hostname only if it wasn't already an IP
        if (strcmp(this->hosts[charger_idx], ip_str) != 0) {
            logger.printfln("Resolved %s to %s", this->hosts[charger_idx], ip_str);
        }
    }

    dest_addrs[charger_idx].sin_addr.s_addr = in;
    resolve_state[charger_idx] = RESOLVE_STATE_RESOLVED;
}

bool CMNetworking::is_resolved(uint8_t charger_idx)
{
    return resolve_state[charger_idx] == RESOLVE_STATE_RESOLVED;
}

void CMNetworking::clear_dns_cache_entry(uint8_t charger_idx)
{
    auto err = dns_removehost(this->hosts[charger_idx], nullptr);
    if (err != ESP_OK)
        logger.printfln("Couldn't remove hostname from cache: error %i", err);
}

void CMNetworking::check_results()
{
    {
        std::lock_guard<std::mutex> lock{scan_results_mutex};
        if (!mdns_query_async_get_results(scan, 0, &scan_results))
            return; // This should never happen as check_results is only called if we are notified the search has finished.
    }

    mdns_query_async_delete(scan);

    scanning = false;

#if MODULE_WS_AVAILABLE()
    CoolString result;
    if (get_scan_results(result))
        ws.pushRawStateUpdate(result, "charge_manager/scan_result");
#else
    {
        std::lock_guard<std::mutex> lock{scan_results_mutex};
        if (scan_results == nullptr)
            return;

        while (scan_results != nullptr) {
            if (this->mdns_result_is_charger(scan_results, nullptr, nullptr, nullptr))
                this->resolve_via_mdns(scan_results);
            scan_results = scan_results->next;
        }
    }
#endif
    return;
}

void CMNetworking::start_scan()
{
    if (scanning)
        return;
    scanning = true;

    {
        std::lock_guard<std::mutex> lock{scan_results_mutex};
        if (scan_results != nullptr) {
            mdns_query_results_free(scan_results);
            scan_results = nullptr;
        }
    }

    scan = mdns_query_async_new(NULL, "_tf-warp-cm", "_udp", MDNS_TYPE_PTR, 1000, INT8_MAX, [](mdns_search_once_t *search) {
        task_scheduler.scheduleOnce([](){ cm_networking.check_results(); });
    });
}

bool CMNetworking::mdns_result_is_charger(mdns_result_t *entry, const char ** ret_version, const char **ret_enabled, const char **ret_display_name, const char **ret_proxy_of) {
    if (ret_version != nullptr)
        *ret_version = "0";
    if (ret_enabled != nullptr)
        *ret_enabled = "false";
    if (ret_display_name != nullptr)
        *ret_display_name = "[no_display_name]";

    if (entry->txt_count < 3)
        return false;

    int found = 0;
    for (size_t i = 0; i < entry->txt_count; ++i) {
        // strcmp is safe here: Keys are always null terminated.
        // https://github.com/espressif/esp-idf/blob/7eba5f80027e1648775b46f889cb4d9519afc965/components/mdns/mdns.c#L3000-L3011
        if (strcmp(entry->txt[i].key, "enabled") == 0 && entry->txt_value_len[i] > 0) {
            if (ret_enabled != nullptr)
                *ret_enabled = entry->txt[i].value;
            ++found;
        }
        else if (strcmp(entry->txt[i].key, "display_name") == 0 && entry->txt_value_len[i] > 0) {
            if (ret_display_name != nullptr)
                *ret_display_name = entry->txt[i].value;
            ++found;
        }
        else if (strcmp(entry->txt[i].key, "version") == 0 && entry->txt_value_len[i] > 0) {
            if (ret_version != nullptr)
                *ret_version = entry->txt[i].value;
            ++found;
        }
        else if (strcmp(entry->txt[i].key, "proxy_of") == 0 && entry->txt_value_len[i] > 0) {
            if (ret_proxy_of != nullptr)
                *ret_proxy_of = entry->txt[i].value;
            // don't increase found: this is an optional entry.
        }
    }

    if (found < 3)
        return false;

    return true;
}

void CMNetworking::resolve_via_mdns(mdns_result_t *entry)
{
    if (entry->addr && entry->addr->addr.type == IPADDR_TYPE_V4) {
        for (size_t i = 0; i < charger_count; ++i) {
            if ((this->needs_mdns & (1ull << i)) == 0)
                continue;

            String host = String(this->hosts[i]);
            host = host.substring(0, host.length() - 6);

            if (host == entry->hostname) {
                this->dest_addrs[i].sin_addr.s_addr = entry->addr->addr.u_addr.ip4.addr;
                if (this->resolve_state[i] != RESOLVE_STATE_RESOLVED) {
                    char addr_str[16];
                    tf_ip4addr_ntoa(&entry->addr->addr, addr_str, sizeof(addr_str));
                    logger.printfln("Resolved %s to %s (via mDNS scan)", this->hosts[i], addr_str);
                }
                this->resolve_state[i] = RESOLVE_STATE_RESOLVED;
            }
        }
    }
}

void CMNetworking::add_scan_result_entry(mdns_result_t *entry, TFJsonSerializer &json)
{
    const char *version;
    const char *enabled;
    const char *display_name;
    const char *proxy_of = nullptr;
    if (!this->mdns_result_is_charger(entry, &version, &enabled, &display_name, &proxy_of))
        return;

    this->resolve_via_mdns(entry);

    uint8_t error = SCAN_RESULT_ERROR_OK;

    // strcmp is safe here: txt values such as enabled are null terminated:
    // https://github.com/espressif/esp-idf/blob/7eba5f80027e1648775b46f889cb4d9519afc965/components/mdns/mdns.c#L3017-L3023
    if (strcmp(enabled, "true") != 0) {
        error = SCAN_RESULT_ERROR_MANAGEMENT_DISABLED;
    } else {
        const char *protocol_version = strchr(version, '.');
        if (!protocol_version) {
            error = SCAN_RESULT_ERROR_FIRMWARE_MISMATCH;
        } else {
            if (strncmp_with_same_len(MACRO_VALUE_TO_STRING(CM_PACKET_MAGIC), version, protocol_version - version) != 0) {
                error = SCAN_RESULT_ERROR_FIRMWARE_MISMATCH;
            } else {
                long num_version = strtol(++protocol_version, nullptr, 10);
                if (num_version < CM_STATE_VERSION_MIN) {
                    error = SCAN_RESULT_ERROR_FIRMWARE_MISMATCH;
                }
            }
        }
    }

    json.addObject();
        json.addMemberString("hostname", entry->hostname);

        char addr_str[32] = "[no_address]";
        if (entry->addr && entry->addr->addr.type == IPADDR_TYPE_V4) {
            tf_ip4addr_ntoa(&entry->addr->addr, addr_str, sizeof(addr_str));
        }
        json.addMemberString("ip", addr_str);

        json.addMemberString("display_name", display_name);
        json.addMemberNumber("error", error);
        if (proxy_of != nullptr)
            json.addMemberString("proxy_of", proxy_of);
    json.endObject();
}

size_t CMNetworking::build_scan_result_json(mdns_result_t *list, char *buf, size_t len)
{
    TFJsonSerializer json{buf, len};
    json.addArray();

    while (list != nullptr) {
        add_scan_result_entry(list, json);
        list = list->next;
    }

    json.endArray();
    return json.end();
}

bool CMNetworking::get_scan_results(CoolString &result)
{
    std::lock_guard<std::mutex> lock{scan_results_mutex};
    if (scan_results == nullptr)
        return false;

    size_t payload_size = build_scan_result_json(scan_results, nullptr, 0) + 1; // null terminator

    if (!result.reserve(payload_size))
        return false;

    build_scan_result_json(scan_results, result.begin(), payload_size);
    result.setLength(payload_size - 1);

    return true;
}
