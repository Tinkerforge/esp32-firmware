/* esp32-firmware
 * Copyright (C) 2022 Erik Fleckstein <erik@tinkerforge.com>
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

#include <ocpp16/ChargePoint16.h>
#include <ocpp21/ChargePoint21.h>

#include "module.h"
#include "config.h"

enum class OcppProtocolVersion : uint8_t {
    V16 = 0,
    V21 = 1,
};

class Ocpp final : public IModule
{
public:
    Ocpp(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void pre_reboot() override;

    bool on_tag_seen(const char *tag_id);

    bool get_iso15118_secc_chain(bool iso20, std::unique_ptr<char[]> *chain_pem_out, std::unique_ptr<char[]> *key_pem_out);
    std::unique_ptr<char[]> get_iso15118_root_bundle(bool oem);

    bool is_iso15118_store_live();
    bool is_iso20_tls_ready();
    bool get_iso15118_ocsp_staple(uint8_t cert_idx, std::unique_ptr<uint8_t[]> *der_out, size_t *der_len_out);

    enum class VehicleChainCheck : uint8_t {
        NotRequired, // no live 2.1 client or private environment
        Pending,     // response not received yet
        Good,
        Revoked,
        Unknown,     // Unknown/missing/expired status or failed request
    };
    struct VehicleChainCertDer {
        const uint8_t *der;
        size_t len;
    };
    bool request_iso15118_vehicle_chain_status(const VehicleChainCertDer *chain, size_t chain_len, const uint8_t *root_der, size_t root_len);
    VehicleChainCheck get_iso15118_vehicle_chain_check();
    void on_vehicle_chain_status_result(bool response_received);

    struct Iso15118CtrlrValues {
        bool enforce_tls;
        int32_t pwm_charging_fallback_timeout_s;
        char evse_id[OCPP21_ISO15118_EVSE_ID_MAX_LEN + 1];
    };
    bool get_iso15118_ctrlr(Iso15118CtrlrValues *out);

    void (*tag_seen_cb)(int32_t, const char *, void *) = nullptr;
    void *tag_seen_cb_user_data = nullptr;

    ConfigRoot state;
    ConfigRoot config;
    ConfigRoot configuration;

    size_t trace_buf_idx;
private:
    void apply_config();

    bool start_client();
    bool start_client_21();

    const Ocpp21::CertEntry *best_iso15118_secc_chain(bool iso20, bool *valid_out);

    OcppCertHashData21 vehicle_chain_hashes[OCPP21_VEHICLE_OCSP_CACHE_SIZE];
    size_t vehicle_chain_count = 0;
    bool vehicle_chain_response_received = false;
    bool vehicle_chain_request_failed = false;

    ConfigRoot change_configuration;

    uint64_t task_id = 0;
    bool client_started = false;

    std::unique_ptr<OcppChargePoint> cp;
    std::unique_ptr<Ocpp21::ChargePoint> cp21;
};
