#include "ocpp/Platform.h"

#include "ocpp/ChargePoint.h"
#include "ocpp/Configuration.h"
#include "time.h"

#define URL_PARSER_IMPLEMENTATION_STATIC
#include "lib/url.h"

#include "esp_websocket_client.h"
#include "esp_crt_bundle.h"
#include "mbedtls/base64.h"
#include "esp_transport_ws.h"

#include "modules.h"
#include "api.h"
#include "build.h"

static bool feature_evse = false;
static bool feature_meter = false;
static bool feature_meter_all_values = false;
static bool feature_meter_phases = false;
#define REQUIRE_FEATURE(x, default_val) do { if (!feature_##x && !api.hasFeature(#x)) { return default_val; } feature_##x = true;} while(0)

void(*recv_cb)(char *, size_t, void *) = nullptr;
void *recv_cb_userdata = nullptr;

static bool connected_by_event = false;
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        logger.printfln("OCPP WEBSOCKET CONNECTED");
        connected_by_event = true;
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        logger.printfln("OCPP WEBSOCKET DISCONNECTED");
        connected_by_event = false;
        break;
    case WEBSOCKET_EVENT_DATA:
        if (data->payload_len == 0)
            return;
        if (data->op_code != WS_TRANSPORT_OPCODES_TEXT)
            return;

        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        // ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        // ESP_LOGW(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
        ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);
        // const cast is safe here:
        // - data->data_ptr is only set in esp_websocket_client_dispatch_event to the const char *data param
        // - const char *data is either null or (in esp_websocket_client_recv) set to client->rx_buffer
        // - client->rx_buffer is char * (so not const)
        recv_cb(const_cast<char *>(data->data_ptr), data->data_len, recv_cb_userdata);
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}

esp_websocket_client_handle_t client;
void* platform_init(const char *websocket_url, const char *basic_auth_user, const uint8_t *basic_auth_pass, size_t basic_auth_pass_length)
{
    esp_websocket_client_config_t websocket_cfg = {};
    websocket_cfg.uri = websocket_url;
    websocket_cfg.subprotocol = "ocpp1.6";
    websocket_cfg.crt_bundle_attach = esp_crt_bundle_attach;
    websocket_cfg.disable_auto_reconnect = false;

    uint32_t ping_interval = getIntConfigUnsigned(ConfigKey::WebSocketPingInterval);
    if (ping_interval != 0) {
        websocket_cfg.ping_interval_sec = ping_interval;
        websocket_cfg.pingpong_timeout_sec = ping_interval * 3 + (ping_interval / 2);
        websocket_cfg.disable_pingpong_discon = false;
    } else {
        // We can't completely disable sending pings.
        websocket_cfg.ping_interval_sec = 0xFFFFFFFF;
        websocket_cfg.pingpong_timeout_sec = 0;
        websocket_cfg.disable_pingpong_discon = true;
    }

    // Username and password are "Not supported for now".
    //websocket_cfg.username = basic_auth_user;
    //websocket_cfg.password = basic_auth_pass;
    // Instead create and pass the authorization header directly.

    // We have to hold header outside of the if. Otherwise header.c_str() is a dangling pointer if we
    // leave the inner scope.
    String header = "Authorization: Basic ";

    if (basic_auth_user != nullptr && basic_auth_pass != nullptr) {
        size_t user_len = strlen(basic_auth_user);
        size_t buf_len = user_len + basic_auth_pass_length + 1; // +1 for ':'
        std::unique_ptr<char[]> buf = heap_alloc_array<char>(buf_len);
        memcpy(buf.get(), basic_auth_user, user_len);
        buf[user_len] = ':';
        memcpy(buf.get() + user_len + 1, basic_auth_pass, basic_auth_pass_length);

        size_t written = 0;
        mbedtls_base64_encode(nullptr, 0, &written, (const unsigned char *)buf.get(), buf_len);

        std::unique_ptr<char[]> base64_buf{new char[written + 1]()}; // +1 for '\0'
        mbedtls_base64_encode((unsigned char *) base64_buf.get(), written + 1, &written, (const unsigned char *)buf.get(), buf_len);
        base64_buf[written] = '\0';

        header += base64_buf.get();
        header += "\r\n";

        websocket_cfg.headers = header.c_str();
    }

    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    esp_websocket_client_start(client);

    return client;
}

bool platform_has_fixed_cable(int connectorId) {
    return true;
}

void platform_disconnect(void *ctx) {
    esp_websocket_client_close(client, pdMS_TO_TICKS(1000));
}

void platform_destroy(void *ctx) {
    esp_websocket_client_destroy(client);
}

bool platform_ws_connected(void *ctx)
{
    bool is_connected = esp_websocket_client_is_connected(client);

    if (connected_by_event && !is_connected) {
        logger.printfln("OCPP was disconnected immediately after connection was established! Reconnecting in 10 seconds.");

        connected_by_event = false;

        platform_disconnect(ctx);
        task_scheduler.scheduleOnce([ctx](){esp_websocket_client_start(client);}, 10000);
        return false;
    }

    return is_connected;
}

void platform_ws_send(void *ctx, const char *buf, size_t buf_len)
{
    esp_websocket_client_send_text(client, buf, buf_len, pdMS_TO_TICKS(1000));
}

void platform_ws_send_ping(void *ctx) {
    // NOP. esp_websocket automatically sends pings, pongs and checks for timeouts.
    // We can't send pings manually until we switch to ESP-IDF 5.0:
    // https://github.com/espressif/esp-protocols/commit/3330b96b10fc05287c2d3f52057e4ba453576b9a
}

void platform_ws_register_receive_callback(void *ctx, void(*cb)(char *, size_t, void *), void *user_data)
{
    recv_cb = cb;
    recv_cb_userdata = user_data;
}

uint32_t platform_now_ms() {
    return millis();
}

time_t last_system_time = 0;
uint32_t last_system_time_set_at = 0;

void platform_set_system_time(void *ctx, time_t t)
{
   struct timeval tv{t, 0};
   settimeofday(&tv, nullptr);
}

time_t platform_get_system_time(void *ctx) {
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    return tv_now.tv_sec;
}

void platform_printfln(int level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logger.printfln(fmt, args);
    va_end(args);
}



void platform_register_tag_seen_callback(void *ctx, void(*cb)(int32_t, const char *, void *), void *user_data) {
    ocpp.tag_seen_cb = cb;
    ocpp.tag_seen_cb_user_data = user_data;
}

const char *trt_string[] = {
"Blocked",
"Expired",
"Invalid",
"ConcurrentTx",
};

void platform_tag_rejected(const char *tag, TagRejectionType trt) {
    logger.printfln("Tag %s rejected: %s", tag, trt_string[(size_t)trt]);
}

void platform_tag_timed_out(int32_t connectorId)
{
    logger.printfln("Tag timeout!");
}

void platform_cable_timed_out(int32_t connectorId)
{
    logger.printfln("Cable timeout!");
}

EVSEState platform_get_evse_state(int32_t connectorId) {
    REQUIRE_FEATURE(evse, EVSEState::Faulted);

    auto state = api.getState("evse/state")->get("charger_state")->asUint();
    switch(state) {
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
int32_t platform_get_energy(int32_t connectorId) {
    REQUIRE_FEATURE(meter, 0);

    Config *meter_values = api.getState("meter/values", false);
    if (meter_values == nullptr)
        return 0;

    return (int32_t)(meter_values->get("energy_abs")->asFloat() * 1000);
}

bool platform_get_signed_meter_value(int32_t connectorId, SampledValueMeasurand measurand, SampledValuePhase phase, SampledValueLocation location, char buf[OCPP_PLATFORM_MEASURAND_MAX_DATA_LEN]){
    logger.printfln("platform_get_signed_meter_value not implemented yet!");
    return false;
}




const static SupportedMeasurand supported_measurands_sdm630[] = {
    //ENERGY_ACTIVE_EXPORT_REGISTER
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::K_WH, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::K_WH, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::K_WH, false},

    //ENERGY_ACTIVE_IMPORT_REGISTER
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::K_WH, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::K_WH, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::K_WH, false},

    //ENERGY_REACTIVE_EXPORT_REGISTER
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::KVARH, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::KVARH, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::KVARH, false},

    //ENERGY_REACTIVE_IMPORT_REGISTER
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::KVARH, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::KVARH, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::KVARH, false},

    //POWER_ACTIVE_EXPORT
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::W, false},

    //POWER_ACTIVE_IMPORT
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::W, false},

    //POWER_REACTIVE_EXPORT
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::W, false},

    //POWER_REACTIVE_IMPORT
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::W, false},

    //POWER_FACTOR
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::NONE, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::NONE, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::NONE, false},

    // We can measure this with 1. the offered current and 2. the connected phases

    //CURRENT_OFFERED
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::A, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::A, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::A, false},

    //VOLTAGE
    {SampledValuePhase::L1_N, SampledValueLocation::OUTLET, SampledValueUnit::V, false},
    {SampledValuePhase::L2_N, SampledValueLocation::OUTLET, SampledValueUnit::V, false},
    {SampledValuePhase::L3_N, SampledValueLocation::OUTLET, SampledValueUnit::V, false},
    {SampledValuePhase::L1_L2, SampledValueLocation::OUTLET, SampledValueUnit::V, false},
    {SampledValuePhase::L2_L3, SampledValueLocation::OUTLET, SampledValueUnit::V, false},
    {SampledValuePhase::L3_L1, SampledValueLocation::OUTLET, SampledValueUnit::V, false},

    //FREQUENCY
    /*
    NOTE: OCPP 1.6 does not have a UnitOfMeasure for
    frequency, the UnitOfMeasure for any SampledValue with measurand: Frequency is Hertz.
    */
    {SampledValuePhase::NONE, SampledValueLocation::OUTLET, SampledValueUnit::NONE, false},
};

const size_t supported_measurand_offsets_sdm630[] = {
    0,  /*ENERGY_ACTIVE_EXPORT_REGISTER*/
    3,  /*ENERGY_ACTIVE_IMPORT_REGISTER*/
    6,  /*ENERGY_REACTIVE_EXPORT_REGISTER*/
    9,  /*ENERGY_REACTIVE_IMPORT_REGISTER*/
    12, /*ENERGY_ACTIVE_EXPORT_INTERVAL*/
    12, /*ENERGY_ACTIVE_IMPORT_INTERVAL*/
    12, /*ENERGY_REACTIVE_EXPORT_INTERVAL*/
    12, /*ENERGY_REACTIVE_IMPORT_INTERVAL*/
    12, /*POWER_ACTIVE_EXPORT*/
    15, /*POWER_ACTIVE_IMPORT*/
    18, /*POWER_OFFERED*/
    18, /*POWER_REACTIVE_EXPORT*/
    21, /*POWER_REACTIVE_IMPORT*/
    24, /*POWER_FACTOR*/
    27, /*CURRENT_IMPORT*/
    27, /*CURRENT_EXPORT*/
    27, /*CURRENT_OFFERED*/
    30, /*VOLTAGE*/
    36, /*FREQUENCY*/
    37, /*TEMPERATURE*/
    37, /*SO_C*/
    37, /*RPM*/
    37  /*NONE*/
};

const static SupportedMeasurand supported_measurands_sdm72v2[] = {
    //ENERGY_ACTIVE_EXPORT_REGISTER
    {SampledValuePhase::NONE, SampledValueLocation::OUTLET, SampledValueUnit::K_WH, false},

    //ENERGY_ACTIVE_IMPORT_REGISTER
    {SampledValuePhase::NONE, SampledValueLocation::OUTLET, SampledValueUnit::K_WH, false},

    //ENERGY_REACTIVE_EXPORT_REGISTER
    //ENERGY_REACTIVE_IMPORT_REGISTER

    //POWER_ACTIVE_EXPORT
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::W, false},

    //POWER_ACTIVE_IMPORT
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::W, false},

    //POWER_OFFERED
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::W, false},

    //POWER_REACTIVE_EXPORT
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::W, false},

    //POWER_REACTIVE_IMPORT
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::W, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::W, false},

    //POWER_FACTOR
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::NONE, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::NONE, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::NONE, false},

    //CURRENT_IMPORT
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::A, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::A, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::A, false},
    {SampledValuePhase::N,  SampledValueLocation::OUTLET, SampledValueUnit::A, false},

    //CURRENT_EXPORT
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::A, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::A, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::A, false},
    {SampledValuePhase::N,  SampledValueLocation::OUTLET, SampledValueUnit::A, false},

    //CURRENT_OFFERED
    {SampledValuePhase::L1, SampledValueLocation::OUTLET, SampledValueUnit::A, false},
    {SampledValuePhase::L2, SampledValueLocation::OUTLET, SampledValueUnit::A, false},
    {SampledValuePhase::L3, SampledValueLocation::OUTLET, SampledValueUnit::A, false},

    //VOLTAGE
    {SampledValuePhase::L1_N, SampledValueLocation::OUTLET, SampledValueUnit::V, false},
    {SampledValuePhase::L2_N, SampledValueLocation::OUTLET, SampledValueUnit::V, false},
    {SampledValuePhase::L3_N, SampledValueLocation::OUTLET, SampledValueUnit::V, false},
    {SampledValuePhase::L1_L2, SampledValueLocation::OUTLET, SampledValueUnit::V, false},
    {SampledValuePhase::L2_L3, SampledValueLocation::OUTLET, SampledValueUnit::V, false},
    {SampledValuePhase::L3_L1, SampledValueLocation::OUTLET, SampledValueUnit::V, false},

    //FREQUENCY
    /*
    NOTE: OCPP 1.6 does not have a UnitOfMeasure for
    frequency, the UnitOfMeasure for any SampledValue with measurand: Frequency is Hertz.
    */
    {SampledValuePhase::NONE, SampledValueLocation::OUTLET, SampledValueUnit::NONE, false},
};

const size_t supported_measurand_offsets_sdm72v2[] = {
    0,  /*ENERGY_ACTIVE_EXPORT_REGISTER*/
    1,  /*ENERGY_ACTIVE_IMPORT_REGISTER*/
    2,  /*ENERGY_REACTIVE_EXPORT_REGISTER*/
    2,  /*ENERGY_REACTIVE_IMPORT_REGISTER*/
    2, /*ENERGY_ACTIVE_EXPORT_INTERVAL*/
    2, /*ENERGY_ACTIVE_IMPORT_INTERVAL*/
    2, /*ENERGY_REACTIVE_EXPORT_INTERVAL*/
    2, /*ENERGY_REACTIVE_IMPORT_INTERVAL*/
    2, /*POWER_ACTIVE_EXPORT*/
    5, /*POWER_ACTIVE_IMPORT*/
    8, /*POWER_OFFERED*/
    11, /*POWER_REACTIVE_EXPORT*/
    14, /*POWER_REACTIVE_IMPORT*/
    17, /*POWER_FACTOR*/
    20, /*CURRENT_IMPORT*/
    24, /*CURRENT_EXPORT*/
    28, /*CURRENT_OFFERED*/
    31, /*VOLTAGE*/
    37, /*FREQUENCY*/
    38, /*TEMPERATURE*/
    38, /*SO_C*/
    38, /*RPM*/
    38  /*NONE*/
};

static uint8_t meter_type = 0;
static const SupportedMeasurand *supported_measurands = nullptr;
static size_t supported_measurands_len = 0;
static const size_t *supported_measurand_offsets = nullptr;

void update_meter_type() {
    if (meter_type != 0)
        return;

    REQUIRE_FEATURE(meter, );

    meter_type = api.getState("meter/state")->get("type")->asUint();
    if (meter_type == METER_TYPE_SDM72DMV2) {
        supported_measurands = supported_measurands_sdm72v2;
        supported_measurands_len = ARRAY_SIZE(supported_measurands_sdm72v2);
        supported_measurand_offsets = supported_measurand_offsets_sdm72v2;
    } else if (meter_type == METER_TYPE_SDM630) {
        supported_measurands = supported_measurands_sdm630;
        supported_measurands_len = ARRAY_SIZE(supported_measurands_sdm630);
        supported_measurand_offsets = supported_measurand_offsets_sdm630;
    }
}

size_t platform_get_supported_measurand_count(int32_t connector_id, SampledValueMeasurand measurand) {
    if (connector_id == 0)
        return 0;

    update_meter_type();

    if (measurand == SampledValueMeasurand::NONE) {
        return supported_measurands_len;
    }

    if (supported_measurand_offsets == nullptr)
        return 0;

    return supported_measurand_offsets[(size_t)measurand + 1] - supported_measurand_offsets[(size_t)measurand];
}

const SupportedMeasurand *platform_get_supported_measurands(int32_t connector_id, SampledValueMeasurand measurand) {
    if (connector_id == 0)
        return nullptr;

    update_meter_type();

     if (supported_measurands == nullptr)
        return nullptr;

    if (measurand == SampledValueMeasurand::NONE)
        return supported_measurands;

    return supported_measurands + supported_measurand_offsets[(size_t)measurand];
}


float platform_get_raw_meter_value_sdm630(int32_t connectorId, SampledValueMeasurand measurand, SampledValuePhase phase, SampledValueLocation location) {
    if (connectorId != 1)
        return 0.0f;

    REQUIRE_FEATURE(meter_all_values, 0);

    Config *meter_all_values = api.getState("meter/all_values");

    switch(measurand) {
        case SampledValueMeasurand::ENERGY_ACTIVE_EXPORT_REGISTER:
            return meter_all_values->get(METER_ALL_VALUES_EXPORT_KWH_L1 + (size_t) phase)->asFloat();
        case SampledValueMeasurand::ENERGY_ACTIVE_IMPORT_REGISTER:
            return meter_all_values->get(METER_ALL_VALUES_IMPORT_KWH_L1 + (size_t) phase)->asFloat();
        case SampledValueMeasurand::ENERGY_REACTIVE_EXPORT_REGISTER:
            return meter_all_values->get(METER_ALL_VALUES_EXPORT_KVARH_L1 + (size_t) phase)->asFloat();
        case SampledValueMeasurand::ENERGY_REACTIVE_IMPORT_REGISTER:
            return meter_all_values->get(METER_ALL_VALUES_IMPORT_KVARH_L1 + (size_t) phase)->asFloat();

        case SampledValueMeasurand::POWER_ACTIVE_EXPORT:
            // The power factor's sign indicates the direction of the current flow.
            return meter_all_values->get(METER_ALL_VALUES_POWER_FACTOR_L1 + (size_t) phase)->asFloat() < 0 ?
                   meter_all_values->get(METER_ALL_VALUES_POWER_L1_W + (size_t) phase)->asFloat() :
                   0.0f;
        case SampledValueMeasurand::POWER_ACTIVE_IMPORT:
            return meter_all_values->get(METER_ALL_VALUES_POWER_FACTOR_L1 + (size_t) phase)->asFloat() >= 0 ?
                   meter_all_values->get(METER_ALL_VALUES_POWER_L1_W + (size_t) phase)->asFloat() :
                   0.0f;
        case SampledValueMeasurand::POWER_REACTIVE_EXPORT:
            return meter_all_values->get(METER_ALL_VALUES_POWER_FACTOR_L1 + (size_t) phase)->asFloat() >= 0 ?
                   meter_all_values->get(METER_ALL_VALUES_VOLT_AMPS_REACTIVE_L1 + (size_t) phase)->asFloat() :
                   0.0f;
        case SampledValueMeasurand::POWER_REACTIVE_IMPORT:
            return meter_all_values->get(METER_ALL_VALUES_POWER_FACTOR_L1 + (size_t) phase)->asFloat() >= 0 ?
                   meter_all_values->get(METER_ALL_VALUES_VOLT_AMPS_REACTIVE_L1 + (size_t) phase)->asFloat() :
                   0.0f;

        case SampledValueMeasurand::POWER_FACTOR:
            return fabs(meter_all_values->get(METER_ALL_VALUES_POWER_FACTOR_L1 + (size_t) phase)->asFloat());

        case SampledValueMeasurand::CURRENT_OFFERED:
            REQUIRE_FEATURE(meter_phases, 0);
            REQUIRE_FEATURE(evse, 0);
            return api.getState("meter/phases")->get("phases_connected")->get((size_t) phase)->asBool() ?
                   ((float)api.getState("evse/state")->get("allowed_charging_current")->asUint()) / 1000.0f :
                   0.0f;
        case SampledValueMeasurand::VOLTAGE:
            switch (phase) {
                case SampledValuePhase::L1_N:
                case SampledValuePhase::L2_N:
                case SampledValuePhase::L3_N:
                    return meter_all_values->get(METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L1 + ((size_t) phase - (size_t) SampledValuePhase::L1_N))->asFloat();

                case SampledValuePhase::L1_L2:
                case SampledValuePhase::L2_L3:
                case SampledValuePhase::L3_L1:
                    return meter_all_values->get(METER_ALL_VALUES_LINE1_TO_LINE2_VOLTS + ((size_t) phase - (size_t) SampledValuePhase::L1_L2))->asFloat();

                case SampledValuePhase::L1:
                case SampledValuePhase::L2:
                case SampledValuePhase::L3:
                case SampledValuePhase::N:
                case SampledValuePhase::NONE:
                    return 0.0f;
            }
            return 0.0f;

        case SampledValueMeasurand::FREQUENCY:
            return meter_all_values->get(METER_ALL_VALUES_FREQUENCY_OF_SUPPLY_VOLTAGES_HERTZ)->asFloat();

        case SampledValueMeasurand::ENERGY_ACTIVE_EXPORT_INTERVAL:
        case SampledValueMeasurand::ENERGY_ACTIVE_IMPORT_INTERVAL:
        case SampledValueMeasurand::ENERGY_REACTIVE_EXPORT_INTERVAL:
        case SampledValueMeasurand::ENERGY_REACTIVE_IMPORT_INTERVAL:
        case SampledValueMeasurand::POWER_OFFERED:
        case SampledValueMeasurand::CURRENT_IMPORT:
        case SampledValueMeasurand::CURRENT_EXPORT:
        case SampledValueMeasurand::TEMPERATURE:
        case SampledValueMeasurand::SO_C:
        case SampledValueMeasurand::RPM:
        case SampledValueMeasurand::NONE:
            return 0.0f;
    }
    return 0.0f;
}

float platform_get_raw_meter_value_sdm72v2(int32_t connectorId, SampledValueMeasurand measurand, SampledValuePhase phase, SampledValueLocation location) {
    if (connectorId != 1)
        return 0.0f;

    REQUIRE_FEATURE(meter_all_values, 0);

    Config *meter_all_values = api.getState("meter/all_values");

    switch(measurand) {
        case SampledValueMeasurand::ENERGY_ACTIVE_EXPORT_REGISTER:
            return meter_all_values->get(METER_ALL_VALUES_TOTAL_EXPORT_KWH)->asFloat();
        case SampledValueMeasurand::ENERGY_ACTIVE_IMPORT_REGISTER:
            return meter_all_values->get(METER_ALL_VALUES_TOTAL_IMPORT_KWH)->asFloat();

        case SampledValueMeasurand::POWER_ACTIVE_EXPORT:
            // The power's sign indicates the direction of the current flow. Positive = energy flow from grid to vehicle = import
            return meter_all_values->get(METER_ALL_VALUES_POWER_L1_W + (size_t) phase)->asFloat() < 0 ?
                   -meter_all_values->get(METER_ALL_VALUES_POWER_L1_W + (size_t) phase)->asFloat() :
                   0.0f;
        case SampledValueMeasurand::POWER_ACTIVE_IMPORT:
            return meter_all_values->get(METER_ALL_VALUES_POWER_L1_W + (size_t) phase)->asFloat() >= 0 ?
                   meter_all_values->get(METER_ALL_VALUES_POWER_L1_W + (size_t) phase)->asFloat() :
                   0.0f;

        case SampledValueMeasurand::POWER_OFFERED:
            /*
            Two measurands (Current.Offered and Power.Offered) are available that are strictly speaking no
            measured values. They indicate the maximum amount of current/power that is being offered
            to the EV and are intended for use in smart charging applications.
            */
            // (ChargingRateUnitType)
            /*
            If used for AC Charging, the phase current should be calculated via: Current per phase = Power / (Line Voltage * Number of
            Phases). The "Line Voltage" used in the calculation is not the measured voltage, but the set voltage for the area (hence, 230 of
            110 volt).
            */
            // Thus we use 230 to calculate the offered power. This ideally matches the power of the active ChargingSchedulePeriod.
            REQUIRE_FEATURE(meter_phases, 0);
            REQUIRE_FEATURE(evse, 0);
            return api.getState("meter/phases")->get("phases_connected")->get((size_t) phase)->asBool() ?
                   (((float)api.getState("evse/state")->get("allowed_charging_current")->asUint()) / 1000.0f) * 230.0f :
                   0.0f;

        case SampledValueMeasurand::POWER_REACTIVE_EXPORT:
            // Reactive is always positive. Use power factor sign to determine current flow direction.
            return meter_all_values->get(METER_ALL_VALUES_POWER_FACTOR_L1 + (size_t) phase)->asFloat() < 0 ?
                   meter_all_values->get(METER_ALL_VALUES_VOLT_AMPS_REACTIVE_L1 + (size_t) phase)->asFloat() :
                   0.0f;
        case SampledValueMeasurand::POWER_REACTIVE_IMPORT:
            return meter_all_values->get(METER_ALL_VALUES_POWER_FACTOR_L1 + (size_t) phase)->asFloat() >= 0 ?
                   meter_all_values->get(METER_ALL_VALUES_VOLT_AMPS_REACTIVE_L1 + (size_t) phase)->asFloat() :
                   0.0f;

        case SampledValueMeasurand::POWER_FACTOR:
            return fabs(meter_all_values->get(METER_ALL_VALUES_POWER_FACTOR_L1 + (size_t) phase)->asFloat());

        case SampledValueMeasurand::CURRENT_EXPORT:
            // Current is always positive. Use power factor sign to determine current flow direction.
            // Note that the neutral current is inverted in the import/export logic.
            if (phase == SampledValuePhase::N)
                return meter_all_values->get(METER_ALL_VALUES_POWER_FACTOR_L1 + (size_t) phase)->asFloat() >= 0 ?
                       meter_all_values->get(METER_ALL_VALUES_NEUTRAL_CURRENT_A + (size_t) phase)->asFloat() :
                       0.0f;

            return meter_all_values->get(METER_ALL_VALUES_POWER_FACTOR_L1 + (size_t) phase)->asFloat() < 0 ?
                   meter_all_values->get(METER_ALL_VALUES_CURRENT_L1_A + (size_t) phase)->asFloat() :
                   0.0f;

        case SampledValueMeasurand::CURRENT_IMPORT:
            if (phase == SampledValuePhase::N)
                return meter_all_values->get(METER_ALL_VALUES_POWER_FACTOR_L1 + (size_t) phase)->asFloat() < 0 ?
                       meter_all_values->get(METER_ALL_VALUES_NEUTRAL_CURRENT_A + (size_t) phase)->asFloat() :
                       0.0f;

            // Current is always positive. Use power factor sign to determine current flow direction.
            return meter_all_values->get(METER_ALL_VALUES_POWER_FACTOR_L1 + (size_t) phase)->asFloat() >= 0 ?
                   meter_all_values->get(METER_ALL_VALUES_CURRENT_L1_A + (size_t) phase)->asFloat() :
                   0.0f;

        case SampledValueMeasurand::CURRENT_OFFERED:
            REQUIRE_FEATURE(meter_phases, 0);
            REQUIRE_FEATURE(evse, 0);
            return api.getState("meter/phases")->get("phases_connected")->get((size_t) phase)->asBool() ?
                   ((float)api.getState("evse/state")->get("allowed_charging_current")->asUint()) / 1000.0f :
                   0.0f;

        case SampledValueMeasurand::VOLTAGE:
            switch (phase) {
                case SampledValuePhase::L1_N:
                case SampledValuePhase::L2_N:
                case SampledValuePhase::L3_N:
                    return meter_all_values->get(METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L1 + ((size_t) phase - (size_t) SampledValuePhase::L1_N))->asFloat();

                case SampledValuePhase::L1_L2:
                case SampledValuePhase::L2_L3:
                case SampledValuePhase::L3_L1:
                    return meter_all_values->get(METER_ALL_VALUES_LINE1_TO_LINE2_VOLTS + ((size_t) phase - (size_t) SampledValuePhase::L1_L2))->asFloat();

                case SampledValuePhase::L1:
                case SampledValuePhase::L2:
                case SampledValuePhase::L3:
                case SampledValuePhase::N:
                case SampledValuePhase::NONE:
                    return 0.0f;
            }
            return 0.0f;

        case SampledValueMeasurand::FREQUENCY:
            return meter_all_values->get(METER_ALL_VALUES_FREQUENCY_OF_SUPPLY_VOLTAGES_HERTZ)->asFloat();

        case SampledValueMeasurand::ENERGY_REACTIVE_EXPORT_REGISTER:
        case SampledValueMeasurand::ENERGY_REACTIVE_IMPORT_REGISTER:

        case SampledValueMeasurand::ENERGY_ACTIVE_EXPORT_INTERVAL:
        case SampledValueMeasurand::ENERGY_ACTIVE_IMPORT_INTERVAL:
        case SampledValueMeasurand::ENERGY_REACTIVE_EXPORT_INTERVAL:
        case SampledValueMeasurand::ENERGY_REACTIVE_IMPORT_INTERVAL:

        case SampledValueMeasurand::TEMPERATURE:
        case SampledValueMeasurand::SO_C:
        case SampledValueMeasurand::RPM:
        case SampledValueMeasurand::NONE:
            return 0.0f;
    }
    return 0.0f;
}


float platform_get_raw_meter_value(int32_t connectorId, SampledValueMeasurand measurand, SampledValuePhase phase, SampledValueLocation location) {
    update_meter_type();

    if (meter_type == METER_TYPE_SDM72DMV2)
        return platform_get_raw_meter_value_sdm72v2(connectorId, measurand, phase, location);
    else if (meter_type == METER_TYPE_SDM630)
        return platform_get_raw_meter_value_sdm630(connectorId, measurand, phase, location);

    return 0.0f;
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
    uint16_t current = (uint16_t)std::min(32000u, (uint32_t)milliAmps);
#if MODULE_EVSE_AVAILABLE()
    if (evse.get_ocpp_current() != current)
        evse.set_ocpp_current(current);
#elif MODULE_EVSE_V2_AVAILABLE()
    if (evse_v2.get_ocpp_current() != current)
        evse_v2.set_ocpp_current(current);
#endif
}

#define PATH_PREFIX String("/ocpp/")

size_t platform_read_file(const char *name, char *buf, size_t len) {
    File f = LittleFS.open(PATH_PREFIX + name);
    // File::read can return 2^32-1 because it returns -1 if the file is not open but the return type is size_t.
    if (f.read((uint8_t *)buf, len) > len)
        return 0;
    return len;
}
bool platform_write_file(const char *name, char *buf, size_t len) {
    File f = LittleFS.open(PATH_PREFIX + name, "w", true);
    return f.write((const uint8_t *)buf, len) == len;
}


// return nullptr if name does not exist or is not a directory
void *platform_open_dir(const char *name) {
    File *f = new File(LittleFS.open(PATH_PREFIX + name));
    if (!f->isDirectory()) {
        delete f;
        return nullptr;
    }
    return f;
}

OcppDirEnt dir_ent;

// return nullptr if no more files
OcppDirEnt *platform_read_dir(void *dir_fd) {
    File *dir = (File *)dir_fd;
    File f;
    while (f = dir->openNextFile()) {
        dir_ent.is_dir = f.isDirectory();
        strncpy(dir_ent.name, f.name(), ARRAY_SIZE(dir_ent.name) - 1);
        return &dir_ent;
    }
    return nullptr;
}
void platform_close_dir(void *dir_fd) {
    File *f = (File *)dir_fd;
    delete f;
}

void platform_remove_file(const char *name) {
    LittleFS.remove(PATH_PREFIX + name);
}

void platform_reset(bool hard) {
    if (hard) {
        /*
        At receipt of a hard reset the Charge Point SHALL restart (all) the hardware, it is not required to gracefully stop
        ongoing transaction.
        */
#if MODULE_EVSE_AVAILABLE()
        evse.reset();
#endif
#if MODULE_EVSE_V2_AVAILABLE()
        evse_v2.reset();
#endif
#if MODULE_MODBUS_METER_AVAILABLE()
        modbus_meter.reset();
#endif
#if MODULE_NFC_AVAILABLE()
        nfc.reset();
#endif
#if MODULE_RTC_AVAILABLE()
        rtc.reset();
#endif
    }

    /*
        At receipt of a soft reset, the Charge Point SHALL [...]
        It should then restart the application software (if possible,
        otherwise restart the processor/controller).
    */
    ESP.restart();
}

void platform_register_stop_callback(void *ctx, void (*cb)(int32_t, StopReason, void *), void *user_data) {

}

const char *platform_get_charge_point_vendor() {
    return "Tinkerforge GmbH";
}

char model[20] = {0}; // FIXME: Check if this needs to be one byte longer. See https://github.com/Tinkerforge/tfocpp/blob/5f07d2a7821167bf09eb4422d80657bb77ef886e/src/ocpp/Messages.cpp#L377
const char *platform_get_charge_point_model() {
    device_name.name.get("display_type")->asString().toCharArray(model, ARRAY_SIZE(model));
    return model;
}

const char *platform_get_charge_point_serial_number() {
    return device_name.name.get("name")->asUnsafeCStr(); // FIXME: Check if this use of the returned C string is safe or if a local copy like in platform_get_charge_point_model() is required. Might need to be truncated to a length of 25+\0.
}
const char *platform_get_firmware_version() {
    return build_version_full_str();
}
const char *platform_get_iccid() {
    return nullptr;
}
const char *platform_get_imsi() {
    return nullptr;
}
const char *platform_get_meter_type() {
    return nullptr;
}
const char *platform_get_meter_serial_number() {
    return nullptr;
}

uint32_t platform_get_maximum_charging_current(int32_t connectorId) {
    return 32000;
}

#ifdef OCPP_STATE_CALLBACKS
void platform_update_chargepoint_state(OcppState state,
                                       StatusNotificationStatus last_sent_status,
                                       time_t next_profile_eval) {

   ocpp.state.get("charge_point_state")->updateUint((uint8_t)state);
   ocpp.state.get("charge_point_status")->updateUint((uint8_t)last_sent_status);
   ocpp.state.get("next_profile_eval")->updateInt((int32_t)next_profile_eval);
}

void platform_update_connector_state(int32_t connector_id,
                                     ConnectorState state,
                                     StatusNotificationStatus last_sent_status,
                                     IdTagInfo auth_for,
                                     uint32_t tag_deadline,
                                     uint32_t cable_deadline,
                                     int32_t txn_id,
                                     time_t transaction_confirmed_timestamp,
                                     time_t transaction_start_time,
                                     uint32_t current_allowed,
                                     bool txn_with_invalid_id,
                                     bool unavailable_requested) {
    if (connector_id != 1)
        return;

    ocpp.state.get("connector_state")->updateUint((uint8_t)state);
    ocpp.state.get("connector_status")->updateUint((uint8_t)last_sent_status);
    ocpp.state.get("tag_id")->updateString(auth_for.tagId);
    ocpp.state.get("parent_tag_id")->updateString(auth_for.parentTagId);
    ocpp.state.get("tag_expiry_date")->updateInt((int32_t)auth_for.expiryDate);
    ocpp.state.get("tag_timeout")->updateUint(tag_deadline - millis());
    ocpp.state.get("cable_timeout")->updateUint(cable_deadline - millis());
    ocpp.state.get("txn_id")->updateInt(txn_id);
    ocpp.state.get("txn_confirmed_time")->updateInt((int32_t)transaction_confirmed_timestamp);
    ocpp.state.get("txn_start_time")->updateInt((int32_t)transaction_start_time);
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
                                      uint8_t transaction_message_queue_depth) {
    ocpp.state.get("message_in_flight_type")->updateUint((uint8_t)message_in_flight_type);
    ocpp.state.get("message_in_flight_id_high")->updateUint(message_in_flight_id >> 32);
    ocpp.state.get("message_in_flight_id_low")->updateUint(message_in_flight_id & (0xFFFFFFFF));
    ocpp.state.get("message_in_flight_len")->updateUint(message_in_flight_len);
    ocpp.state.get("message_timeout")->updateUint(message_timeout_deadline - millis());
    ocpp.state.get("txn_msg_retry_timeout")->updateUint(txn_msg_retry_deadline - millis());
    ocpp.state.get("message_queue_depth")->updateUint(message_queue_depth);
    ocpp.state.get("status_queue_depth")->updateUint(status_notification_queue_depth);
    ocpp.state.get("txn_msg_queue_depth")->updateUint(transaction_message_queue_depth);
}

void platform_update_config_state(ConfigKey key,
                                  const char *value) {
    ocpp.configuration.get(config_keys[(size_t) key])->updateString(value);
}
#endif
