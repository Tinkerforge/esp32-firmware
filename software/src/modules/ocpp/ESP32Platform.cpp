#include "ocpp/Platform.h"

#include "ocpp/ChargePoint.h"
#include "time.h"

#define URL_PARSER_IMPLEMENTATION_STATIC
#include "lib/url.h"

#include "esp_websocket_client.h"


#include "esp_crt_bundle.h"

#include "modules.h"
#include "api.h"
#include "build_timestamp.h"
extern API api;

void(*recv_cb)(char *, size_t, void *) = nullptr;
void *recv_cb_userdata = nullptr;
bool connected = false;

enum ws_transport_opcodes {
    WS_TRANSPORT_OPCODES_CONT =  0x00,
    WS_TRANSPORT_OPCODES_TEXT =  0x01,
    WS_TRANSPORT_OPCODES_BINARY = 0x02,
    WS_TRANSPORT_OPCODES_CLOSE = 0x08,
    WS_TRANSPORT_OPCODES_PING = 0x09,
    WS_TRANSPORT_OPCODES_PONG = 0x0a,
    WS_TRANSPORT_OPCODES_FIN = 0x80,
    WS_TRANSPORT_OPCODES_NONE = 0x100,   /*!< not a valid opcode to indicate no message previously received
                                          * from the API esp_transport_ws_get_read_opcode() */
} ws_transport_opcodes_t;

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        logger.printfln("OCPP WEBSOCKET CONNECTED");
        connected = true;
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        logger.printfln("OCPP WEBSOCKET DISCONNECTED");
        connected = false;
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
void* platform_init(const char *websocket_url)
{
    esp_websocket_client_config_t websocket_cfg = {};
    websocket_cfg.uri = websocket_url;
    websocket_cfg.subprotocol = "ocpp1.6";
    websocket_cfg.crt_bundle_attach = esp_crt_bundle_attach;
    websocket_cfg.disable_auto_reconnect = false;
    websocket_cfg.ping_interval_sec = 10;
    websocket_cfg.pingpong_timeout_sec = 25;
    websocket_cfg.disable_pingpong_discon = false;

    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    esp_websocket_client_start(client);

    return client;
}

void platform_disconnect(void *ctx) {
    esp_websocket_client_close(client, pdMS_TO_TICKS(1000));
}

void platform_destroy(void *ctx) {
    esp_websocket_client_destroy(client);
}

bool platform_ws_connected(void *ctx)
{
    return connected;
}

void platform_ws_send(void *ctx, const char *buf, size_t buf_len)
{
    if (esp_websocket_client_send_text(client, buf, buf_len, pdMS_TO_TICKS(1000)) != ESP_OK)
        if (!esp_websocket_client_is_connected(client))
            esp_websocket_client_start(client);
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
    Config *meter_values = api.getState("meter/values", false);
    if (meter_values == nullptr)
        return 0;

    return (int32_t)(meter_values->get("energy_abs")->asFloat() * 1000);
}

bool platform_get_signed_meter_value(int32_t connectorId, SampledValueMeasurand measurand, SampledValuePhase phase, SampledValueLocation location, char buf[OCPP_PLATFORM_MEASURAND_MAX_DATA_LEN]){
    logger.printfln("platform_get_signed_meter_value not implemented yet!");
    return false;
}





const SupportedMeasurand supported_measurands[] = {
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

const size_t supported_measurand_offsets[] = {
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

size_t platform_get_supported_measurand_count(int32_t connector_id, SampledValueMeasurand measurand) {
    if (connector_id == 0)
        return 0;
    if (measurand == SampledValueMeasurand::NONE)
        return ARRAY_SIZE(supported_measurands);
    return supported_measurand_offsets[(size_t)measurand + 1] - supported_measurand_offsets[(size_t)measurand];
}

const SupportedMeasurand *platform_get_supported_measurands(int32_t connector_id, SampledValueMeasurand measurand) {
    if (connector_id == 0)
        return nullptr;
    if (measurand == SampledValueMeasurand::NONE)
        return supported_measurands;
    return supported_measurands + supported_measurand_offsets[(size_t)measurand];
}

float platform_get_raw_meter_value(int32_t connectorId, SampledValueMeasurand measurand, SampledValuePhase phase, SampledValueLocation location) {
    if (connectorId != 1)
        return 0.0f;

    Config *meter_all_values = api.getState("meter/all_values");
    if (meter_all_values == nullptr)
        return 0.0f;

    switch(measurand) {
        case SampledValueMeasurand::ENERGY_ACTIVE_EXPORT_REGISTER:
            return meter_all_values->get(70 + (size_t) phase)->asFloat();
        case SampledValueMeasurand::ENERGY_ACTIVE_IMPORT_REGISTER:
            return meter_all_values->get(67 + (size_t) phase)->asFloat();
        case SampledValueMeasurand::ENERGY_REACTIVE_EXPORT_REGISTER:
            return meter_all_values->get(79 + (size_t) phase)->asFloat();
        case SampledValueMeasurand::ENERGY_REACTIVE_IMPORT_REGISTER:
            return meter_all_values->get(76 + (size_t) phase)->asFloat();

        case SampledValueMeasurand::POWER_ACTIVE_EXPORT:
            // The power factor's sign indicates the direction of the current flow.
            return meter_all_values->get(15 + (size_t) phase)->asFloat() < 0 ?
                   meter_all_values->get(6 + (size_t) phase)->asFloat() :
                   0.0f;
        case SampledValueMeasurand::POWER_ACTIVE_IMPORT:
            return meter_all_values->get(15 + (size_t) phase)->asFloat() >= 0 ?
                   meter_all_values->get(6 + (size_t) phase)->asFloat() :
                   0.0f;
        case SampledValueMeasurand::POWER_REACTIVE_EXPORT:
            return meter_all_values->get(15 + (size_t) phase)->asFloat() >= 0 ?
                   meter_all_values->get(12 + (size_t) phase)->asFloat() :
                   0.0f;
        case SampledValueMeasurand::POWER_REACTIVE_IMPORT:
            return meter_all_values->get(15 + (size_t) phase)->asFloat() >= 0 ?
                   meter_all_values->get(12 + (size_t) phase)->asFloat() :
                   0.0f;

        case SampledValueMeasurand::POWER_FACTOR:
            return fabs(meter_all_values->get(15 + (size_t) phase)->asFloat());

        case SampledValueMeasurand::CURRENT_OFFERED:
            return api.getState("meter/phases")->get("phases_connected")->get((size_t) phase)->asBool() ?
                   ((float)api.getState("evse/state")->get("allowed_charging_current")->asUint()) / 1000.0f :
                   0.0f;
        case SampledValueMeasurand::VOLTAGE:
            switch (phase) {
                case SampledValuePhase::L1_N:
                case SampledValuePhase::L2_N:
                case SampledValuePhase::L3_N:
                    return meter_all_values->get(0 + (size_t) phase)->asFloat();

                case SampledValuePhase::L1_L2:
                case SampledValuePhase::L2_L3:
                case SampledValuePhase::L3_L1:
                    return meter_all_values->get(42 + (size_t) phase)->asFloat();

                case SampledValuePhase::L1:
                case SampledValuePhase::L2:
                case SampledValuePhase::L3:
                case SampledValuePhase::N:
                case SampledValuePhase::NONE:
                    return 0.0f;
            }
        case SampledValueMeasurand::FREQUENCY:
            return meter_all_values->get(29 + (size_t) phase)->asFloat();

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
    return f.read((uint8_t *)buf, len);
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
        strncpy(dir_ent.name, f.name(), ARRAY_SIZE(dir_ent.name));
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

void platform_reset() {
    logger.printfln("Ignoring reset request for now.");
}

void platform_register_stop_callback(void *ctx, void (*cb)(int32_t, StopReason, void *), void *user_data) {

}

const char *platform_get_charge_point_vendor() {
    return "Tinkerforge GmbH";
}

char model[20] = {0};
const char *platform_get_charge_point_model() {
    strncpy(model, device_name.name.get("display_type")->asCStr(), ARRAY_SIZE(model));
    return model;
}

const char *platform_get_charge_point_serial_number() {
    return device_name.name.get("name")->asCStr();
}
const char *platform_get_firmware_version() {
    return BUILD_VERSION_FULL_STR;
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
                                      int32_t message_in_flight_id,
                                      size_t message_in_flight_len,
                                      uint32_t message_timeout_deadline,
                                      uint32_t txn_msg_retry_deadline,
                                      uint8_t message_queue_depth,
                                      uint8_t status_notification_queue_depth,
                                      uint8_t transaction_message_queue_depth) {
    ocpp.state.get("message_in_flight_type")->updateUint((uint8_t)message_in_flight_type);
    ocpp.state.get("message_in_flight_id")->updateInt(message_in_flight_id);
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
