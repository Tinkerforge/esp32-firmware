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
#include <cstring>
#include <TFJson.h>

#include "mdns.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools/dns.h"
#include "tools/net.h"
#include "cool_string.h"


[[gnu::noinline]] [[gnu::unused]]
static void add_mdns_service_base()
{
    mdns_txt_item_t txt_items[] = {
        {"version", MACRO_VALUE_TO_STRING(CM_PACKET_MAGIC) "." MACRO_VALUE_TO_STRING(CM_STATE_VERSION)},
    };

    mdns_service_add(NULL, "_tf-warp-cm", "_udp", 34127, txt_items, ARRAY_SIZE(txt_items));
}

[[gnu::noinline]] [[gnu::unused]]
static void add_mdns_service_enabled(bool enabled)
{
    mdns_service_txt_item_set("_tf-warp-cm", "_udp", "enabled", enabled ? "true" : "false");
}

#if MODULE_EVENT_AVAILABLE() && MODULE_EVSE_COMMON_AVAILABLE()
static void add_mdns_service_enabled_by_evse_event()
{
    event.registerEvent("evse/management_enabled", {"enabled"}, [](const Config *cfg) {
        add_mdns_service_enabled(cfg->asBool());

        return EventResult::OK;
    });
}
#endif

[[gnu::noinline]] [[gnu::unused]]
static void add_mdns_service_display_name(const char *display_name)
{
    mdns_service_txt_item_set("_tf-warp-cm", "_udp", "display_name", display_name);
}

#if MODULE_EVENT_AVAILABLE()
[[gnu::unused]]
static void add_mdns_service_display_name_by_event()
{
#if MODULE_DEVICE_NAME_AVAILABLE()
    event.registerEvent("info/display_name", {"display_name"}, [](const Config *cfg) {
        add_mdns_service_display_name(cfg->asUnsafeCStr());

        return EventResult::OK;
    });
#endif
}
#endif

[[gnu::noinline]] [[gnu::unused]]
static void add_mdns_service_proxy(const char *proxy_of)
{
    mdns_service_txt_item_set("_tf-warp-cm", "_udp", "proxy_of", proxy_of);
}

#if MODULE_EVENT_AVAILABLE() && MODULE_EM_PHASE_SWITCHER_AVAILABLE() && MODULE_DEVICE_NAME_AVAILABLE()
static void add_mdns_service_proxy_by_event()
{
    event.registerEvent("em_phase_switcher/charger_config", {"host"}, [](const Config *cfg) {
        add_mdns_service_proxy(cfg->asUnsafeCStr());

        return EventResult::OK;
    });
}
#endif

#if MODULE_EVSE_COMMON_AVAILABLE()
static void add_charger_services() {
    add_mdns_service_base();

#if MODULE_EVENT_AVAILABLE()
    add_mdns_service_display_name_by_event();
    add_mdns_service_enabled_by_evse_event();
#else
    task_scheduler.scheduleWithFixedDelay([]() {
#if MODULE_DEVICE_NAME_AVAILABLE()
        // Keep "display_name" updated because it can be changed at runtime without reboot.
        add_mdns_service_display_name(device_name.display_name.get("display_name")->asUnsafeCStr());
#endif
        // Keep "enabled" updated because it is retrieved from the EVSE.
        add_mdns_service_enabled(evse_common.get_management_enabled());
    }, 10_s);
#endif
}
#endif

#if MODULE_EM_PHASE_SWITCHER_AVAILABLE()
static void add_wem_services() {

    static auto *cfg = api.getState("em_phase_switcher/charger_config");
    if (!cfg->get("proxy_mode")->asBool())
        return;

    add_mdns_service_base();
    add_mdns_service_enabled(true);

#if MODULE_DEVICE_NAME_AVAILABLE()
#if MODULE_EVENT_AVAILABLE()
    add_mdns_service_display_name_by_event();
    add_mdns_service_proxy_by_event();
#else
    task_scheduler.scheduleWithFixedDelay([]() {
        // Keep "display_name" updated because it can be changed at runtime without reboot.
        add_mdns_service_display_name(device_name.display_name.get("display_name")->asUnsafeCStr());
        add_mdns_service_proxy(cfg->get("host")->asEphemeralCStr());
    }, 10_s);
#endif
#endif
}
#endif

void CMNetworking::register_urls()
{
    api.addCommand("charge_manager/scan", Config::Null(), {}, [this](String &errmsg) {
#if MODULE_NETWORK_AVAILABLE()
        if (!network.get_enable_mdns()) {
            errmsg = "Cannot scan for chargers: mDNS is disabled.";
            return;
        }
#endif
        start_mdns_scan();
    }, true);

    server.on_HTTPThread("/charge_manager/scan_result", HTTP_GET, [](WebServerRequest request) {
        CoolString result;

        if (!cm_networking.get_scan_results(result))
            return request.send_plain(200, "In progress or not started");

        return request.send_json(200, result);
    });
}

void CMNetworking::register_events() {
#if MODULE_NETWORK_AVAILABLE()
    if (!network.get_enable_mdns())
        return;

#if MODULE_EVSE_COMMON_AVAILABLE()
    add_charger_services();
#elif MODULE_EM_PHASE_SWITCHER_AVAILABLE()
    add_wem_services();
#endif

#endif
}

// Sometimes executed by lwIP
void CMNetworking::dns_resolved(managed_device_data *device, const ip_addr_t *ip)
{
    if (ip->type != IPADDR_TYPE_V4) {
        return;
    }

    // Always mark as resolved even if the address didn't change, in case an unresponsive charger was resolved to its previous address.
    device->resolve_state = ResolveState::Resolved;

    in_addr_t in;

    // using memcpy to guarantee alignment https://mail.gnu.org/archive/html/lwip-users/2008-08/msg00166.html
    std::memcpy(&in, &ip->u_addr.ip4, sizeof(ip->u_addr.ip4));

    if (device->addr.sin_addr.s_addr != in) {
        device->addr.sin_addr.s_addr  = in;

        const char *hname = device->hostname;
        const ip4_addr_t ip4 = ip->u_addr.ip4; // Must copy the address because *ip is temporary.

        task_scheduler.scheduleOnce([hname, ip4]() { // Can't access the logger from lwIP context.
            char ip_str[16];
            tf_ip4addr_ntoa(&ip4, ip_str, sizeof(ip_str));
            logger.printfln("Resolved %s to %s", hname, ip_str);
        });
    }

    // Evict the resolved charger from the DNS cache.
    // It's unlikely that it will be resolved again anytime soon
    // and this frees up space in the cache for things that perform
    // duplicate lookups in sequence, such as the remote access.
    dns_removehostbyname_safe(device->hostname);
}

// Executed by lwIP
static void dns_cb(const char * /*host*/, const ip_addr_t *ip, void *callback_arg)
{
    cm_networking.dns_callback(ip, callback_arg);
}

// Executed by lwIP
void CMNetworking::dns_callback(const ip_addr_t *ip, void *callback_arg)
{
    managed_device_data *device = static_cast<decltype(device)>(callback_arg);

    if (ip) {
        dns_resolved(device, ip);
    } else {
        if (device->resolve_state == ResolveState::Unknown) {
            device->resolve_state = ResolveState::NotResolved;

            const char *hname = device->hostname;
            task_scheduler.scheduleOnce([hname]() { // Can't access the logger from lwIP context.
                logger.printfln("Failed to resolve %s", hname);
            });
        }
    }

    resolve_next_dns(device->device_index);
}

void CMNetworking::resolve_hostname(uint8_t charger_idx)
{
    if (manager_data == nullptr) {
        esp_system_abort("Call register_manager before resolving hostnames!");
    }

    managed_device_data *device = manager_data->managed_devices + charger_idx;

    if (device->host_address_type == HostAddressType::IP) {
        // Nothing to resolve for raw IP addresses.
        return;
    }

    if (device->host_address_type == HostAddressType::mDNS) {
#if MODULE_NETWORK_AVAILABLE()
        if (!network.get_enable_mdns()) {
            logger.printfln("mDNS required to resolve %s but it is disabled", device->hostname);
        } else
#endif
            start_mdns_scan();
        // No braces for the else branch because of the Network module check.

        return;
    }

    resolve_hostname_dns(charger_idx, true);
}

// Sometimes executed by lwIP
void CMNetworking::resolve_hostname_dns(uint8_t charger_idx, bool initial_request) {
    if (manager_data->dns_resolver_active) {
        if (initial_request) {
            return;
        }
    } else {
        manager_data->dns_resolver_active = true;
    }

    managed_device_data *device = manager_data->managed_devices + charger_idx;

    ip_addr_t ip;
    err_t err = dns_gethostbyname_addrtype_lwip_ctx(device->hostname, &ip, dns_cb, device, LWIP_DNS_ADDRTYPE_IPV4);

    if (err == ERR_INPROGRESS) {
        return;
    } else if (err == ERR_OK) {
        dns_resolved(device, &ip);
    } else {
        const char *hname = device->hostname;
        task_scheduler.scheduleOnce([hname, err]() { // Can't access the logger from lwIP context.
            if (err == ERR_VAL) {
                logger.printfln("Charger configured with hostname %s, but no DNS server is configured!", hname);
            } else {
                const int eno = err_to_errno(err);
                logger.printfln("Cannot resolve '%s': %s (%i|%hhi)", hname, strerror(eno), eno, err);
            }
        });
    }

    resolve_next_dns(device->device_index);
}

// Sometimes executed by lwIP
void CMNetworking::resolve_next_dns(uint8_t current_idx) {
    const size_t current_index = current_idx;
    size_t next_index = current_index;
    bool wrapped = false;

    do {
        next_index = next_index + 1;
        if (next_index >= manager_data->managed_device_count) {
            next_index = 0;
            wrapped = true;
        }

        managed_device_data *device = manager_data->managed_devices + next_index;

        if (device->host_address_type == HostAddressType::DNS && device->resolve_state != ResolveState::Resolved) {
            if (wrapped) {
                task_scheduler.scheduleOnce([this, next_index]() {
                    resolve_hostname_dns(static_cast<uint8_t>(next_index), false);
                }, 15_s);
            } else {
                resolve_hostname_dns(static_cast<uint8_t>(next_index), false);
            }
            return;
        }
    } while (next_index != current_index);

    manager_data->dns_resolver_active = false;
}

bool CMNetworking::is_resolved(uint8_t charger_idx)
{
    if (!manager_data) {
        logger.printfln("is_resolved called before register_manager");
        return false;
    }
    if (charger_idx >= manager_data->managed_device_count) {
        logger.printfln("Charger index %hhu is out of range for is_resolved", charger_idx);
        return false;
    }

    const ResolveState resolve_state = manager_data->managed_devices[charger_idx].resolve_state;
    return resolve_state == ResolveState::Resolved || resolve_state == ResolveState::Stale;
}

void CMNetworking::resolve_all()
{
    if (manager_data == nullptr) {
        esp_system_abort("Call register_manager before resolving hostnames!");
    }

    for (uint8_t i = 0; i < manager_data->managed_device_count; i++) {
        resolve_hostname(i);
    }
}

void CMNetworking::notify_charger_unresponsive(uint8_t charger_idx)
{
    // If the network is not connected, there's nothing that can be done.
    // Wait for another unresponsive notification once the network is connected.
    if (!manager_data->connected) {
        return;
    }

    managed_device_data *device = manager_data->managed_devices + charger_idx;

    if (device->resolve_state == ResolveState::Resolved && device->host_address_type != HostAddressType::IP) {
        device->resolve_state = ResolveState::Stale;
    }

    resolve_hostname(charger_idx);
}

// Executed by mDNS task
void CMNetworking::check_mdns_results_cb(mdns_search_once_t *)
{
    task_scheduler.scheduleOnce([]() {
        cm_networking.check_mdns_results();
    });
}

void CMNetworking::check_mdns_results()
{
    bool search_finished;
    {
        std::lock_guard<std::mutex> lock{scan_results_mutex};
        search_finished = mdns_query_async_get_results(mdns_scan, 0, &scan_results, nullptr);
    }

    mdns_query_async_delete(mdns_scan);
    mdns_scan = nullptr;
    mdns_scan_active = false;

    if (!search_finished) {
        // This should never happen as check_results is only called if we are notified the search has finished.
        logger.printfln("mDNS query failed");
        return;
    }

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

void CMNetworking::start_mdns_scan()
{
    if (mdns_scan_active) {
        return;
    }
    mdns_scan_active = true;

    {
        std::lock_guard<std::mutex> lock{scan_results_mutex};
        if (scan_results != nullptr) {
            mdns_query_results_free(scan_results);
            scan_results = nullptr;
        }
    }

    mdns_scan = mdns_query_async_new(NULL, "_tf-warp-cm", "_udp", MDNS_TYPE_PTR, 1000, INT8_MAX, &check_mdns_results_cb);

    if (!mdns_scan) {
        logger.printfln("mDNS scan could not be started");
    }
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

void CMNetworking::resolve_via_mdns(mdns_result_t *result_entry)
{
    if (!manager_data || !result_entry->addr) {
        return;
    }

    for (size_t i = 0; i < manager_data->managed_device_count; ++i) {
        managed_device_data *device = manager_data->managed_devices + i;

        if (device->host_address_type != HostAddressType::mDNS) {
            continue;
        }

        const char  *hostname            = device->hostname;
        const size_t hostname_substr_len = device->mdns_hostname_len;

        if (strncmp(hostname, result_entry->hostname, hostname_substr_len) != 0 || result_entry->hostname[hostname_substr_len] != 0) {
            continue;
        }

        mdns_ip_addr_t *addr_entry = result_entry->addr;
        do {
            if (addr_entry->addr.type == IPADDR_TYPE_V4) {
                if (device->addr.sin_addr.s_addr != addr_entry->addr.u_addr.ip4.addr) {
                    device->addr.sin_addr.s_addr  = addr_entry->addr.u_addr.ip4.addr;
                    device->resolve_state = ResolveState::Resolved;

                    char addr_str[16];
                    tf_ip4addr_ntoa(&addr_entry->addr, addr_str, sizeof(addr_str));
                    logger.printfln("Resolved %s to %s (via mDNS scan)", device->hostname, addr_str);
                }

                return;
            }

            addr_entry = addr_entry->next;
        } while (addr_entry);
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
