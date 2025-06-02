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

    bool was_connected = false;

    WifiState get_connection_state() const;
    bool is_sta_enabled() const;
    int get_sta_rssi() const;

    const char* get_ap_ssid() const;
    const char* get_ap_ip() const;
    const char* get_ap_passphrase() const;

private:
    void apply_soft_ap_config_and_start();
    bool apply_sta_config_and_connect();

    int get_ap_state();

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

    ConfigRoot ap_config;
    ConfigRoot sta_config;
    ConfigRoot state;

    ap_runtime *runtime_ap = nullptr;

    OwnedConfig sta_config_in_use;

    std::unique_ptr<unsigned char[]> ca_cert = nullptr;
    size_t ca_cert_len = 0;
    std::unique_ptr<unsigned char[]> client_cert = nullptr;
    size_t client_cert_len = 0;
    std::unique_ptr<unsigned char[]> client_key = nullptr;
    size_t client_key_len = 0;

    std::vector<ConfUnionPrototype<EapConfigID>> eap_config_prototypes;

    String eap_username;
    String eap_password;
    String eap_identity;

    micros_t last_connected = 0_us;
};
