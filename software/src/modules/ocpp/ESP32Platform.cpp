/* esp32-firmware
 * Copyright (C) 2022-2024 Erik Fleckstein <erik@tinkerforge.com>
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

#include <ocpp/Platform.h>
#include <ocpp/ChargePoint.h>
#include <ocpp/Configuration.h>
#include <ocpp/Types.h>
#include <time.h>
#define URL_PARSER_IMPLEMENTATION_STATIC
#include <lib/url.h>
#include <esp_crt_bundle.h>
#include <mbedtls/base64.h>
#include <esp_transport_ws.h>
#include <LittleFS.h>

#define TRACE_LOG_PREFIX nullptr

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tf_websocket_client.h"
#include "build.h"
#include "ocpp.h"
#include "modules/meters/meter_defs.h"

#include "mvid_to_measurand.h"

static bool feature_evse = false;
#define REQUIRE_FEATURE(x, default_val) do { if (!feature_##x && !api.hasFeature(#x)) { return default_val; } feature_##x = true;} while(0)

void(*recv_cb)(char *, size_t, void *) = nullptr;
void *recv_cb_userdata = nullptr;

void (*pong_cb)(void *) = nullptr;
void *pong_cb_userdata = nullptr;

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    tf_websocket_event_data_t *data = (tf_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_DATA:
        if (data->payload_len == 0)
            return;
        if (data->op_code != WS_TRANSPORT_OPCODES_TEXT)
            return;

        // const cast is safe here:
        // - data->data_ptr is only set in tf_websocket_client_dispatch_event to the const char *data param
        // - const char *data is either null or (in tf_websocket_client_recv) set to client->rx_buffer
        // - client->rx_buffer is char * (so not const)
        task_scheduler.await([data](){
            recv_cb(const_cast<char *>(data->data_ptr), data->data_len, recv_cb_userdata);
        });
        break;
    case WEBSOCKET_EVENT_PONG:
        task_scheduler.await([](){pong_cb(pong_cb_userdata);});
        break;
    }
}

extern "C" esp_err_t esp_crt_bundle_attach(void *conf);

static tf_websocket_client_handle_t client;
static bool client_running = false;
static std::unique_ptr<String[]> auth_headers;
static size_t auth_headers_count = 0;
static size_t next_auth_header = 0;
void *platform_init(const char *websocket_url, BasicAuthCredentials *credentials, size_t credentials_length)
{
    tf_websocket_client_config_t websocket_cfg = {};
    websocket_cfg.uri = websocket_url;
    websocket_cfg.subprotocol = "ocpp1.6";
    int8_t cert_id = ocpp.config_in_use.get("cert_id")->asInt();
    if (cert_id == -1)
        websocket_cfg.crt_bundle_attach = esp_crt_bundle_attach;
    else {
        size_t cert_len = 0;
        auto cert = certs.get_cert(cert_id, &cert_len);
        if (cert == nullptr) {
            logger.printfln("OCPP platform: Configured TLS certificate does not exist!");
            return nullptr;
        }

        // Release the cert buffer unique_ptr's ownership.
        // This effectively leaks the buffer, but as per the
        // esp_transport_ssl_set_cert_data documentation:
        // "Note that, this function stores the pointer to data, rather than making a copy.
        //  So this data must remain valid until after the connection is cleaned up"
        // esp_transport_ssl_set_cert_data is called by esp_websocket_client_start.
        websocket_cfg.cert_pem = (const char *)cert.release();
    }

    websocket_cfg.disable_auto_reconnect = true;

    // We can't completely disable sending pings.
    websocket_cfg.ping_interval_sec = 0xFFFFFFFF;
    websocket_cfg.pingpong_timeout_sec = 0;
    websocket_cfg.disable_pingpong_discon = true;
    websocket_cfg.task_stack = 8192;

    // Username and password are "Not supported for now".
    //websocket_cfg.username = basic_auth_user;
    //websocket_cfg.password = basic_auth_pass;
    // Instead create and pass the authorization header(s) directly.

    if (credentials_length > 0) {
        auth_headers = heap_alloc_array<String>(credentials_length);
        auth_headers_count = credentials_length;

        for(size_t i = 0; i < credentials_length; ++i) {
            String header = "Authorization: Basic ";
            auto &cred = credentials[i];

            size_t user_len = strlen(cred.user);
            size_t buf_len = user_len + cred.pass_length + 1; // +1 for ':'
            std::unique_ptr<char[]> buf = heap_alloc_array<char>(buf_len);
            memcpy(buf.get(), cred.user, user_len);
            buf[user_len] = ':';
            memcpy(buf.get() + user_len + 1, cred.pass, cred.pass_length);

            size_t written = 0;
            mbedtls_base64_encode(nullptr, 0, &written, (const unsigned char *)buf.get(), buf_len);

            std::unique_ptr<char[]> base64_buf{new char[written + 1]()}; // +1 for '\0'
            mbedtls_base64_encode((unsigned char *)base64_buf.get(), written + 1, &written, (const unsigned char *)buf.get(), buf_len);
            base64_buf[written] = '\0';

            header += base64_buf.get();
            header += "\r\n";

            auth_headers[i] = header;
        }

        websocket_cfg.headers = auth_headers[0].c_str();
        next_auth_header = (next_auth_header + 1) % auth_headers_count;
    }

    client = tf_websocket_client_init(&websocket_cfg);
    tf_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    if (network.is_connected()) {
        tf_websocket_client_start(client);
        client_running = true;
    }

    return client;
}

bool platform_has_fixed_cable(int32_t connectorId)
{
    return true;
}

void platform_disconnect(void *ctx)
{
    if (client_running) {
        tf_websocket_client_close(client, pdMS_TO_TICKS(1000));
        client_running = false;
    }
}

void platform_reconnect(void *ctx)
{
    if (client_running) {
        tf_websocket_client_stop(client);
        client_running = false;
    }

    // Try next set of credentials if available.
    if (auth_headers_count > 0) {
        tf_websocket_client_set_headers(client, auth_headers[next_auth_header].c_str());
        next_auth_header = (next_auth_header + 1) % auth_headers_count;
    }

    if (network.is_connected()) {
        tf_websocket_client_start(client);
        client_running = true;
    }
}

void platform_destroy(void *ctx)
{
    tf_websocket_client_destroy(client);
}

bool platform_ws_connected(void *ctx)
{
    return tf_websocket_client_is_connected(client);
}

bool platform_ws_send(void *ctx, const char *buf, size_t buf_len)
{
    return tf_websocket_client_send_text(client, buf, buf_len, pdMS_TO_TICKS(10), pdMS_TO_TICKS(1000)) == buf_len;
}

bool platform_ws_send_ping(void *ctx)
{
    return tf_websocket_client_send_ping(client, pdMS_TO_TICKS(1000), pdMS_TO_TICKS(1000)) >= 0;
}

void platform_ws_register_receive_callback(void *ctx, void (*cb)(char *, size_t, void *), void *user_data)
{
    recv_cb = cb;
    recv_cb_userdata = user_data;
}

void platform_ws_register_pong_callback(void *ctx, void (*cb)(void *), void *user_data)
{
    pong_cb = cb;
    pong_cb_userdata = user_data;
}

uint32_t platform_now_ms()
{
    return now_us().to<millis_t>().as<uint32_t>();
}

time_t last_system_time = 0;
uint32_t last_system_time_set_at = 0;

void platform_set_system_time(void *ctx, time_t t)
{
    struct timeval tv{t, 0};
    rtc.push_system_time(tv, Rtc::Quality::Force);
}

time_t platform_get_system_time(void *ctx)
{
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    return tv_now.tv_sec;
}

void platform_printfln(int level, const char *fmt, ...)
{
    va_list args;

    if (level <= OCPP_LOG_LEVEL_WARN) {
        va_start(args, fmt);
        logger.vprintfln(fmt, args);
        va_end(args);
    }

    va_start(args, fmt);
    logger.vtracefln(ocpp.trace_buf_idx, fmt, args);
    va_end(args);
}

void platform_register_tag_seen_callback(void *ctx, void (*cb)(int32_t, const char *, void *), void *user_data)
{
    ocpp.tag_seen_cb = cb;
    ocpp.tag_seen_cb_user_data = user_data;
}

const char *trt_string[] = {
    "Blocked",
    "Expired",
    "Invalid",
    "ConcurrentTx",
};

void platform_tag_expected(int32_t connectorId)
{
    //logger.printfln("Waiting for tag");
#if MODULE_EVSE_LED_AVAILABLE()
    evse_led.set_module(EvseLed::Blink::Nag, 60000); //TODO: Check configuration for correct interval here!
#endif
}

void platform_clear_tag_expected(int32_t connectorId)
{
    //logger.printfln("Not waiting for tag");
#if MODULE_EVSE_LED_AVAILABLE()
    evse_led.set_module(EvseLed::Blink::None, 1000); //Duration is not relevant here.
#endif
}

void platform_tag_rejected(int32_t connectorId, const char *tag, TagRejectionType trt)
{
#if MODULE_EVSE_LED_AVAILABLE()
    evse_led.set_module(EvseLed::Blink::Nack, 2000);
#endif
    ocpp.state.get("last_rejected_tag")->updateString(tag);
    ocpp.state.get("last_rejected_tag_reason")->updateUint((uint8_t)trt);
    logger.printfln("Tag %s rejected: %s", tag, trt_string[(size_t)trt]);
}

void platform_tag_accepted(int32_t connectorId, const char *tag)
{
#if MODULE_EVSE_LED_AVAILABLE()
    evse_led.set_module(EvseLed::Blink::Ack, 2000);
#endif
    ocpp.state.get("last_rejected_tag")->updateString("");
    ocpp.state.get("last_rejected_tag_reason")->updateUint(0);
    logger.printfln("Tag %s accepted", tag);
}

void platform_tag_timed_out(int32_t connectorId)
{
    logger.printfln("Tag timeout!");
}

void platform_cable_timed_out(int32_t connectorId)
{
    logger.printfln("Cable timeout!");
}

EVSEState platform_get_evse_state(int32_t connectorId)
{
    REQUIRE_FEATURE(evse, EVSEState::Faulted);

    auto state = api.getState("evse/state")->get("charger_state")->asUint();
    switch (state) {
        case CHARGER_STATE_NOT_PLUGGED_IN:
            return EVSEState::NotConnected;

        case CHARGER_STATE_WAITING_FOR_RELEASE:
            return EVSEState::Connected;

        case CHARGER_STATE_READY_TO_CHARGE:
            return EVSEState::ReadyToCharge;

        case CHARGER_STATE_CHARGING:
            return EVSEState::Charging;

        case CHARGER_STATE_ERROR:
        default:
            return EVSEState::Faulted;
    }
}

// This is the Energy.Active.Import.Register measurand in Wh
int32_t platform_get_energy(int32_t connectorId)
{
    REQUIRE_FEATURE(evse, 0);
    float result = 0;
    evse_common.get_charger_meter_energy(&result);
    return result * 1000;
}

bool platform_get_signed_meter_value(int32_t connectorId, SampledValueMeasurand measurand, SampledValuePhase phase, SampledValueLocation location, char buf[OCPP_PLATFORM_MEASURAND_MAX_DATA_LEN]){
    logger.printfln("platform_get_signed_meter_value not implemented yet!");
    return false;
}

struct PlatformMeterCache {
    std::unique_ptr<MeterValueID[]> value_ids = nullptr;
    size_t value_ids_length = 0;
    uint32_t charger_meter_slot = UINT32_MAX;
    std::unique_ptr<uint32_t[]> idx_cache = nullptr;
    uint32_t power_offered_l1_idx;
    uint32_t power_offered_l2_idx;
    uint32_t power_offered_l3_idx;
    uint32_t current_offered_l1_idx;
    uint32_t current_offered_l2_idx;
    uint32_t current_offered_l3_idx;
};

static bool platform_meter_available(int32_t connector_id, PlatformMeterCache *meter_cache) {
    if (connector_id != 1)
        return false;

    if (meter_cache->value_ids != nullptr)
        return true;

    if (meter_cache->charger_meter_slot == UINT32_MAX) {
        REQUIRE_FEATURE(evse, false);
        meter_cache->charger_meter_slot = evse_common.get_charger_meter();
    }

    size_t value_ids_length;
    auto result = meters.get_value_ids_extended(meter_cache->charger_meter_slot, nullptr, &value_ids_length);
    if (result != MeterValueAvailability::Fresh)
        return false;

    auto value_ids = heap_alloc_array<MeterValueID>(value_ids_length);
    result = meters.get_value_ids_extended(meter_cache->charger_meter_slot, value_ids.get(), &value_ids_length);

    if (result != MeterValueAvailability::Fresh)
        return false;

    meter_cache->value_ids = std::move(value_ids);
    meter_cache->value_ids_length = value_ids_length;
    return true;
}

static constexpr MeterValueID POWER_OFFERED_L1   = (MeterValueID)((size_t)(MeterValueID::_max) + 1);
static constexpr MeterValueID POWER_OFFERED_L2   = (MeterValueID)((size_t)(MeterValueID::_max) + 2);
static constexpr MeterValueID POWER_OFFERED_L3   = (MeterValueID)((size_t)(MeterValueID::_max) + 3);
static constexpr MeterValueID CURRENT_OFFERED_L1 = (MeterValueID)((size_t)(MeterValueID::_max) + 4);
static constexpr MeterValueID CURRENT_OFFERED_L2 = (MeterValueID)((size_t)(MeterValueID::_max) + 5);
static constexpr MeterValueID CURRENT_OFFERED_L3 = (MeterValueID)((size_t)(MeterValueID::_max) + 6);

static MeterValueID get_mvid_for_measurand(int32_t connector_id, SampledValueMeasurand m, SampledValuePhase p, PlatformMeterCache *meter_cache) {
    if (m == SampledValueMeasurand::POWER_OFFERED) {
        if (p == SampledValuePhase::L1)
            return POWER_OFFERED_L1;
        if (p == SampledValuePhase::L2)
            return POWER_OFFERED_L2;
        if (p == SampledValuePhase::L3)
            return POWER_OFFERED_L3;
    }
    if (m == SampledValueMeasurand::CURRENT_OFFERED) {
        if (p == SampledValuePhase::L1)
            return CURRENT_OFFERED_L1;
        if (p == SampledValuePhase::L2)
            return CURRENT_OFFERED_L2;
        if (p == SampledValuePhase::L3)
            return CURRENT_OFFERED_L3;
    }

    for (size_t mvidx = 0; mvidx < ARRAY_SIZE(mvid_to_measurand); ++mvidx) {
        const auto entry = mvid_to_measurand[mvidx];
        if (entry.measurand == SampledValueMeasurand::NONE)
            continue;

        if (entry.measurand != m || entry.phase != p)
            continue;

        for (size_t i = 0; i < meter_cache->value_ids_length; ++i)
            if (meter_cache->value_ids[i] == (MeterValueID)mvidx)
                return (MeterValueID) mvidx;
    }

    return MeterValueID::NotSupported;
}

bool platform_supports_measurand(int32_t connector_id, SampledValueMeasurand m, SampledValuePhase p, PlatformMeterCache *meter_cache) {
    return get_mvid_for_measurand(connector_id, m, p, meter_cache) != MeterValueID::NotSupported;
}

static size_t platform_get_supported_measurand_count(int32_t connector_id, SampledValueMeasurand *measurands, SampledValuePhase *phases, size_t len, PlatformMeterCache *meter_cache) {
    if (!platform_meter_available(connector_id, meter_cache))
        return 0;

    size_t result = 0;

    for (size_t i = 0; i < len; ++i) {
        auto measurand = measurands[i];
        auto phase = phases == nullptr ? SampledValuePhase::NONE : phases[i];

        if (phase != SampledValuePhase::NONE) {
            if (platform_supports_measurand(connector_id, measurand, phase, meter_cache)) {
                ++result;
                continue;
            }
        }

        // If no phase value is requested, check first whether phase values are available.
        // If this is the case, only use those.
        bool supports_phase_values = false;
        for (size_t p = (size_t)SampledValuePhase::L1; p < (size_t)SampledValuePhase::NONE; ++p) {
            if (platform_supports_measurand(connector_id, measurand, (SampledValuePhase)p, meter_cache)) {
                ++result;
                supports_phase_values = true;
            }
        }

        if (supports_phase_values)
            continue;

        // If no phase values are supported, use the NONE value as fallback
        if (platform_supports_measurand(connector_id, measurand, SampledValuePhase::NONE, meter_cache)) {
            ++result;
        }
    }

    return result;
}

const SampledValueUnit measurand_to_unit[(int)SampledValueMeasurand::NONE + 1] {
    SampledValueUnit::K_WH, /*ENERGY_ACTIVE_EXPORT_REGISTER*/
    SampledValueUnit::K_WH, /*ENERGY_ACTIVE_IMPORT_REGISTER*/
    SampledValueUnit::K_WH, /*ENERGY_REACTIVE_EXPORT_REGISTER*/
    SampledValueUnit::K_WH, /*ENERGY_REACTIVE_IMPORT_REGISTER*/
    SampledValueUnit::K_WH, /*ENERGY_ACTIVE_EXPORT_INTERVAL*/
    SampledValueUnit::K_WH, /*ENERGY_ACTIVE_IMPORT_INTERVAL*/
    SampledValueUnit::K_WH, /*ENERGY_REACTIVE_EXPORT_INTERVAL*/
    SampledValueUnit::K_WH, /*ENERGY_REACTIVE_IMPORT_INTERVAL*/
    SampledValueUnit::W, /*POWER_ACTIVE_EXPORT*/
    SampledValueUnit::W, /*POWER_ACTIVE_IMPORT*/
    SampledValueUnit::W, /*POWER_OFFERED*/
    SampledValueUnit::W, /*POWER_REACTIVE_EXPORT*/
    SampledValueUnit::W, /*POWER_REACTIVE_IMPORT*/
    SampledValueUnit::W, /*POWER_FACTOR*/
    SampledValueUnit::A, /*CURRENT_IMPORT*/
    SampledValueUnit::A, /*CURRENT_EXPORT*/
    SampledValueUnit::A, /*CURRENT_OFFERED*/
    SampledValueUnit::V, /*VOLTAGE*/
    SampledValueUnit::NONE, /*FREQUENCY*/
    SampledValueUnit::CELSIUS, /*TEMPERATURE*/
    SampledValueUnit::PERCENT, /*SO_C*/
    SampledValueUnit::NONE, /*RPM*/
    SampledValueUnit::NONE, /*NONE*/
};

void add_custom_value_to_cache(SampledValueMeasurand m, SampledValuePhase p, uint32_t index, PlatformMeterCache *meter_cache) {
    if (m == SampledValueMeasurand::POWER_OFFERED) {
        if (p == SampledValuePhase::L1)
            meter_cache->power_offered_l1_idx = index;
        if (p == SampledValuePhase::L2)
            meter_cache->power_offered_l2_idx = index;
        if (p == SampledValuePhase::L3)
            meter_cache->power_offered_l3_idx = index;
    }
    if (m == SampledValueMeasurand::CURRENT_OFFERED) {
        if (p == SampledValuePhase::L1)
            meter_cache->current_offered_l1_idx = index;
        if (p == SampledValuePhase::L2)
            meter_cache->current_offered_l2_idx = index;
        if (p == SampledValuePhase::L3)
            meter_cache->current_offered_l3_idx = index;
    }
}

float get_custom_value(uint32_t index, PlatformMeterCache *meter_cache) {
    // WARP1 is either
    //  - Smart -> we don't know the number of phases connected
    //  - or Pro -> we've sold those only with the SDM72 (V1) that can only be used with three phases connected.
    // So we assume that we always offer three-phase charging.
    auto phases = evse_common.get_evse_version() >= 20 ? evse_common.get_low_level_state().get("phases_current")->asUint() : 3;

    float current = evse_common.get_state().get("allowed_charging_current")->asUint() / 1000.0f;

    if (index == meter_cache->power_offered_l1_idx)
        return current * (phases >= 1 ? 1 : 0) * 230;
    if (index == meter_cache->power_offered_l2_idx)
        return current * (phases >= 2 ? 1 : 0) * 230;
    if (index == meter_cache->power_offered_l3_idx)
        return current * (phases >= 3 ? 1 : 0) * 230;

    if (index == meter_cache->current_offered_l1_idx)
        return current * (phases >= 1 ? 1 : 0);
    if (index == meter_cache->current_offered_l2_idx)
        return current * (phases >= 2 ? 1 : 0);
    if (index == meter_cache->current_offered_l3_idx)
        return current * (phases >= 3 ? 1 : 0);

    return 0.0f;
}

static size_t platform_prepare_meter_cache(int32_t connector_id, SampledValueMeasurand *measurands, SampledValuePhase *phases, size_t len, SupportedMeasurand *result, size_t result_len, PlatformMeterCache *meter_cache) {
    meter_cache->idx_cache = heap_alloc_array<uint32_t>(result_len);

    auto mvids = heap_alloc_array<MeterValueID>(result_len);

    size_t written = 0;

    for (size_t i = 0; i < len; ++i) {
        auto measurand = measurands[i];
        auto phase = phases == nullptr ? SampledValuePhase::NONE : phases[i];

        if (phase != SampledValuePhase::NONE) {
            MeterValueID mvid = get_mvid_for_measurand(connector_id, measurand, phase, meter_cache);
            if (mvid != MeterValueID::NotSupported) {
                if (written >= result_len)
                    goto done;

                mvids[written] = mvid;
                result[written] = SupportedMeasurand{measurand, phase, SampledValueLocation::OUTLET, measurand_to_unit[(size_t)measurand], false};
                if (mvid > MeterValueID::_max)
                    add_custom_value_to_cache(measurand, phase, written, meter_cache);

                ++written;
                continue;
            }
        }

        // If no phase value is requested, check first whether phase values are available.
        // If this is the case, only use those.
        bool supports_phase_values = false;
        for (size_t p = (size_t)SampledValuePhase::L1; p < (size_t)SampledValuePhase::NONE; ++p) {
            MeterValueID mvid = get_mvid_for_measurand(connector_id, measurand, (SampledValuePhase)p, meter_cache);
            if (mvid != MeterValueID::NotSupported) {
                if (written >= result_len)
                    goto done;

                mvids[written] = mvid;
                result[written] = SupportedMeasurand{measurand, (SampledValuePhase)p, SampledValueLocation::OUTLET, measurand_to_unit[(size_t)measurand], false};
                if (mvid > MeterValueID::_max)
                    add_custom_value_to_cache(measurand, (SampledValuePhase)p, written, meter_cache);

                ++written;

                supports_phase_values = true;
            }
        }

        if (supports_phase_values)
            continue;

        // If no phase values are supported, use the NONE value as fallback
        MeterValueID mvid = get_mvid_for_measurand(connector_id, measurand, SampledValuePhase::NONE, meter_cache);
        if (mvid != MeterValueID::NotSupported) {
            if (written >= result_len)
                goto done;

            mvids[written] = mvid;
            result[written] = SupportedMeasurand{measurand, SampledValuePhase::NONE, SampledValueLocation::OUTLET, measurand_to_unit[(size_t)measurand], false};
            if (mvid > MeterValueID::_max)
                add_custom_value_to_cache(measurand, phase, written, meter_cache);

            ++written;
        }
    }

done:
    meters.fill_index_cache(meter_cache->charger_meter_slot, written, mvids.get(), meter_cache->idx_cache.get());
    return written;
}

bool platform_prepare_meter(int32_t connector_id, SampledValueMeasurand *measurands, SampledValuePhase *phases, size_t measurand_count, SupportedMeasurand **out_supported_measurands, size_t *out_supported_measurand_count, void **out_platform_meter_cache) {
    if (*out_platform_meter_cache == nullptr)
        *out_platform_meter_cache = new PlatformMeterCache();

    if (!platform_meter_available(connector_id, (PlatformMeterCache *)*out_platform_meter_cache))
        return false;

    *out_supported_measurand_count = platform_get_supported_measurand_count(connector_id, measurands, phases, measurand_count, (PlatformMeterCache *)*out_platform_meter_cache);
    *out_supported_measurands = heap_alloc_array<SupportedMeasurand>(*out_supported_measurand_count).release();

    auto written = platform_prepare_meter_cache(connector_id, measurands, phases, measurand_count, *out_supported_measurands, *out_supported_measurand_count, (PlatformMeterCache *)*out_platform_meter_cache);

    if (*out_supported_measurand_count != written)
        printf("!!!! %zu != %zu und jetzt?", measurand_count, written);

    return true;
}


float platform_get_raw_meter_value(int32_t connectorId, size_t measurand_idx, void *platform_meter_cache) {
    PlatformMeterCache *meter_cache = (PlatformMeterCache *)platform_meter_cache;
    if (meter_cache->idx_cache == nullptr)
        return 0.0f;

    if (meter_cache->idx_cache[measurand_idx] == UINT32_MAX)
        return get_custom_value(measurand_idx, meter_cache);

    float result = 0.0f;
    meters.get_value_by_index(meter_cache->charger_meter_slot, meter_cache->idx_cache[measurand_idx], &result);
    return result;
}

void platform_lock_cable(int32_t connectorId)
{
    //pm.connector_locked |= (1 << (connectorId - 1));
    //logger.printfln("Locked connector");
}

void platform_unlock_cable(int32_t connectorId)
{
    //pm.connector_locked &= ~(1 << (connectorId - 1));
    //logger.printfln("Unlocked connector");
}

void platform_set_charging_current(int32_t connectorId, uint32_t milliAmps)
{
    uint16_t current = (uint16_t)std::min(32000ul, (uint32_t)milliAmps);
    if (evse_common.get_ocpp_current() != current)
        evse_common.set_ocpp_current(current);
}

#define PATH_PREFIX String("/ocpp/")

size_t platform_read_file(const char *name, char *buf, size_t len)
{
    auto path = PATH_PREFIX + name;
    if (!LittleFS.exists(path))
        return 0;

    File f = LittleFS.open(path);
    size_t read = f.read((uint8_t *)buf, len);
    // File::read can return 2^32-1 because it returns -1 if the file is not open but the return type is size_t.
    if (read > len)
        return 0;
    return read;
}
bool platform_write_file(const char *name, char *buf, size_t len)
{
    File f = LittleFS.open(PATH_PREFIX + name, "w", true);
    return f.write((const uint8_t *)buf, len) == len;
}

// return nullptr if name does not exist or is not a directory
void *platform_open_dir(const char *name)
{
    File *f = new File(LittleFS.open(PATH_PREFIX + name));
    if (!f->isDirectory()) {
        delete f;
        return nullptr;
    }
    return f;
}

OcppDirEnt dir_ent;

// return nullptr if no more files
OcppDirEnt *platform_read_dir(void *dir_fd)
{
    File *dir = (File *)dir_fd;
    File f;
    while ((f = dir->openNextFile())) {
        dir_ent.is_dir = f.isDirectory();
        strncpy(dir_ent.name, f.name(), ARRAY_SIZE(dir_ent.name) - 1);
        return &dir_ent;
    }
    return nullptr;
}
void platform_close_dir(void *dir_fd)
{
    File *f = (File *)dir_fd;
    delete f;
}

void platform_remove_file(const char *name)
{
    auto path = PATH_PREFIX + name;
    if (!LittleFS.exists(path))
        return;

    LittleFS.remove(path);
}

void platform_reset(bool hard)
{
    if (hard) {
        /*
        At receipt of a hard reset the Charge Point SHALL restart (all) the hardware, it is not required to gracefully stop
        ongoing transaction.
        */
        evse_common.reset();
#if MODULE_METERS_RS485_BRICKLET_AVAILABLE()
        meters_rs485_bricklet.reset();
#endif
#if MODULE_NFC_AVAILABLE()
        nfc.reset();
#endif
        rtc.reset();
    }

    /*
        At receipt of a soft reset, the Charge Point SHALL [...]
        It should then restart the application software (if possible,
        otherwise restart the processor/controller).
    */
    trigger_reboot("OCPP", 1_s);
}

void platform_register_stop_callback(void *ctx, void (*cb)(int32_t, StopReason, void *), void *user_data)
{
}

const char *platform_get_charge_point_vendor()
{
    static_assert(constexpr_strnlen(BUILD_MANUFACTURER_FULL, 21) <= 20, "OCPP: BUILD_MANUFACTURER_FULL \"" BUILD_MANUFACTURER_FULL "\" is too long for platform_get_charge_point_vendor!");
    return BUILD_MANUFACTURER_FULL;
}

char model[21] = {0};
const char *platform_get_charge_point_model()
{
#if BUILD_IS_WARP() || BUILD_IS_WARP2() || BUILD_IS_WARP3()
    device_name.get20CharDisplayType().toCharArray(model, ARRAY_SIZE(model));
    return model;
#else
    return "Unknown Device";
#endif

}

const char *platform_get_charge_point_serial_number()
{
    return device_name.name.get("name")->asUnsafeCStr(); // FIXME: Check if this use of the returned C string is safe or if a local copy like in platform_get_charge_point_model() is required. Might need to be truncated to a length of 25+\0.
}
const char *platform_get_firmware_version()
{
    return build_version_full_str();
}
const char *platform_get_iccid()
{
    return nullptr;
}
const char *platform_get_imsi()
{
    return nullptr;
}
const char *platform_get_meter_type()
{
    return nullptr;
}
const char *platform_get_meter_serial_number()
{
    return nullptr;
}

uint32_t platform_get_maximum_charging_current(int32_t connectorId)
{
    return 32000;
}

#ifdef OCPP_STATE_CALLBACKS
void platform_update_chargepoint_state(OcppState state,
                                       StatusNotificationStatus last_sent_status,
                                       time_t next_profile_eval) {

   ocpp.state.get("charge_point_state")->updateUint((uint8_t)state);
   ocpp.state.get("charge_point_status")->updateUint((uint8_t)last_sent_status);
   ocpp.state.get("next_profile_eval")->updateInt52(next_profile_eval);
}

void platform_update_connector_state(int32_t connector_id,
                                     ConnectorState state,
                                     StatusNotificationStatus last_sent_status,
                                     IdTagInfo auth_for,
                                     uint32_t tag_deadline,
                                     uint32_t cable_deadline,
                                     int32_t txn_id,
                                     uint64_t transaction_confirmed_id,
                                     time_t transaction_start_time,
                                     uint32_t current_allowed,
                                     bool txn_with_invalid_id,
                                     bool unavailable_requested)
{
    if (connector_id != 1)
        return;

    auto now = now_us().to<millis_t>().as<uint32_t>();

    ocpp.state.get("connector_state")->updateUint((uint8_t)state);
    ocpp.state.get("connector_status")->updateUint((uint8_t)last_sent_status);
    ocpp.state.get("tag_id")->updateString(auth_for.tagId);
    ocpp.state.get("parent_tag_id")->updateString(auth_for.parentTagId);
    ocpp.state.get("tag_expiry_date")->updateInt((int32_t)auth_for.expiryDate);
    ocpp.state.get("tag_timeout")->updateUint(tag_deadline == 0 ? 0xFFFFFFFF : (tag_deadline - now));
    ocpp.state.get("cable_timeout")->updateUint(cable_deadline == 0 ? 0xFFFFFFFF : (cable_deadline - now));
    ocpp.state.get("txn_id")->updateInt(txn_id);
    ocpp.state.get("txn_start_time")->updateInt52(transaction_start_time);
    ocpp.state.get("current")->updateUint(current_allowed);
    ocpp.state.get("txn_with_invalid_id")->updateBool(txn_with_invalid_id);
    ocpp.state.get("unavailable_requested")->updateBool(unavailable_requested);
}

void platform_update_connection_state(CallAction message_in_flight_type,
                                      uint64_t message_in_flight_id,
                                      size_t message_in_flight_len,
                                      uint32_t message_timeout_deadline,
                                      uint32_t txn_msg_retry_deadline,
                                      uint8_t message_queue_depth,
                                      uint8_t status_notification_queue_depth,
                                      uint8_t transaction_message_queue_depth,
                                      bool connected,
                                      time_t connected_change_time,
                                      uint32_t last_ping_sent,
                                      uint32_t pong_deadline)
{
    auto now = now_us().to<millis_t>().as<uint32_t>();

    ocpp.state.get("message_in_flight_type")->updateUint((uint8_t)message_in_flight_type);
    ocpp.state.get("message_in_flight_id_high")->updateUint(message_in_flight_id >> 32);
    ocpp.state.get("message_in_flight_id_low")->updateUint(message_in_flight_id & (0xFFFFFFFF));
    ocpp.state.get("message_in_flight_len")->updateUint(message_in_flight_len);
    ocpp.state.get("message_timeout")->updateUint(message_timeout_deadline == 0 ? 0xFFFFFFFF : (message_timeout_deadline - now));
    ocpp.state.get("txn_msg_retry_timeout")->updateUint(txn_msg_retry_deadline == 0 ? 0xFFFFFFFF : (txn_msg_retry_deadline - now));
    ocpp.state.get("message_queue_depth")->updateUint(message_queue_depth);
    ocpp.state.get("status_queue_depth")->updateUint(status_notification_queue_depth);
    ocpp.state.get("connected")->updateBool(connected);
    ocpp.state.get("connected_change_time")->updateInt52(connected_change_time);
    ocpp.state.get("last_ping_sent")->updateUint(now - last_ping_sent);
    ocpp.state.get("pong_timeout")->updateUint(pong_deadline == 0 ? 0xFFFFFFFF : (pong_deadline - now));
}

void platform_update_config_state(ConfigKey key,
                                  const char *value) {
    ocpp.configuration.get(config_keys[(size_t) key])->updateString(value);
}
#endif
