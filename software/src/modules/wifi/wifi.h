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

#include "module.h"
#include "config.h"
#include "wifi_state.enum.h"

enum class EapConfigID: uint8_t {
    None,
    TLS,
    PEAP_TTLS
};

class StringBuilder;

class Wifi final : public IModule
{
public:
    Wifi(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    WifiState get_connection_state() const;
    bool is_sta_enabled() const;
    int get_sta_rssi() const;

    const char* get_ap_ssid() const;
    const char* get_ap_ip() const;
    const char* get_ap_passphrase() const;

private:
    void apply_soft_ap_config_and_start();
    bool apply_sta_config_and_connect();

    uint8_t get_ap_state();

    void start_scan();
    void get_scan_results(StringBuilder *sb, size_t network_count);

    struct ap_runtime {
        uint32_t scan_start_time_s; // This overflows after an uptime of 68 years, which seems unlikely enough.

        ip4_addr_t gateway;
        uint8_t subnet_cidr;

        uint8_t ssid_offset;
        uint8_t passphrase_offset;
        uint8_t stop_soft_ap_runs;

        uint8_t channel       : 4;
        bool ap_fallback_only : 1;
        bool hide_ssid        : 1;
        bool soft_ap_running  : 1;

        char ip_ssid_passphrase[];
    };

    struct eap_runtime {
        std::unique_ptr<unsigned char[]> ca_cert;
        std::unique_ptr<unsigned char[]> client_cert;
        std::unique_ptr<unsigned char[]> client_key;

        uint8_t eap_config_id;

        uint8_t username_offset;
        uint8_t password_offset;

        char identity_credentials[];
    };

    struct sta_runtime {
        micros_t last_connected;
        eap_runtime *runtime_eap;

        ip4_addr_t ip;
        ip4_addr_t gateway;
        ip4_addr_t dns;
        ip4_addr_t dns2;

        uint8_t bssid[6];

        uint8_t passphrase_offset;

        uint8_t subnet_cidr : 5;
        bool bssid_lock     : 1;
        bool enable_11b     : 1;
        bool was_connected  : 1;

        uint8_t connect_tries;

        char ssid_passphrase[];
    };

    ConfigRoot ap_config;
    ConfigRoot sta_config;
    ConfigRoot state;

    ConfUnionPrototype<EapConfigID> eap_config_prototypes[3];

    ap_runtime  *runtime_ap  = nullptr;
    sta_runtime *runtime_sta = nullptr;
};
