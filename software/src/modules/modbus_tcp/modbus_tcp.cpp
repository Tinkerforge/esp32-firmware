/* esp32-firmware
 * Copyright (C) 2022 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "modbus_tcp.h"

#include <Arduino.h>
#include <esp_modbus_common.h>
#include <esp_modbus_slave.h>
#include <esp_netif.h>
#include <math.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"

extern uint32_t local_uid_num;

// MODBUS TABLE CHANGELOG
// 1 - Initial release
// 2 - Add coils 1000, 1001
#define MODBUS_TABLE_VERSION 2

template<typename T>
static void calloc_struct(T **out)
{
    *out = (T *)heap_caps_calloc_prefer(1, sizeof(T), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
}

// We can't implicit convert from an uint32_t to this, as the conversion is done by a constructor.
// If we add a constructor, this is not a POD type anymore so we can't use it in packed structs.
struct uint32swapped_t {
    //uint32swapped_t(uint32_t x): val(swapReg(x)) {}
    operator uint32_t() {return swapReg(val);}

    static inline uint32_t swapReg(uint32_t x) { return (x << 16) | (x >> 16);}
    uint32_t val;
};
static uint32swapped_t fromUint(uint32_t x) {return {uint32swapped_t::swapReg(x)};}

struct floatswapped_t {
    //floatswapped_t(float x): val(swapReg(x)) {}
    operator float() {return swapReg(val);}

    static inline float swapReg(float x) {
        union {
            float f;
            uint32_t u;
        } u;

        u.f = x;
        u.u = (u.u << 16) | (u.u >> 16);

        return u.f;
    }
    float val;
};
static floatswapped_t fromFloat(float x) {return {floatswapped_t::swapReg(x)};}

//-------------------
// Input Registers
//-------------------
struct input_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_INPUT;
    static const uint16_t OFFSET = 0;
    uint32swapped_t table_version;
    uint32swapped_t firmware_major;
    uint32swapped_t firmware_minor;
    uint32swapped_t firmware_patch;
    uint32swapped_t firmware_build_ts;
    uint32swapped_t box_id;
    uint32swapped_t uptime;
};

struct evse_input_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_INPUT;
    static const uint16_t OFFSET = 1000;
    uint32swapped_t iec_state;
    uint32swapped_t charger_state;
    uint32swapped_t current_user;
    uint32swapped_t start_time_min;
    uint32swapped_t charging_time_sec;
    uint32swapped_t max_current;
    uint32swapped_t slots[CHARGING_SLOT_COUNT_SUPPORTED_BY_EVSE];
};

struct meter_input_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_INPUT;
    static const uint16_t OFFSET = 2000;
    uint32swapped_t meter_type;
    floatswapped_t power;
    floatswapped_t energy_absolute;
    floatswapped_t energy_relative;
    floatswapped_t energy_this_charge;
};

struct meter_all_values_input_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_INPUT;
    static const uint16_t OFFSET = 2100;
    floatswapped_t meter_values[85];
};

struct nfc_input_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_INPUT;
    static const uint16_t OFFSET = 4000;
    uint32swapped_t tag_id[5];
    uint32swapped_t tag_id_age;
};

//-------------------
// Holding Registers
//-------------------
struct holding_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint16_t OFFSET = 0;
    static const uint32_t REBOOT_PASSWORD = 0x012EB007;
    uint32swapped_t reboot;
};

struct evse_holding_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint16_t OFFSET = 1000;
    uint32swapped_t enable_charging;
    uint32swapped_t allowed_current;
    uint32swapped_t led_blink_state;
    uint32swapped_t led_blink_duration;
};

struct meter_holding_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint16_t OFFSET = 2000;
    static const uint32_t TRIGGER_RESET_PASSWORD = 0x3E12E5E7;
    uint32swapped_t trigger_reset;
};

struct [[gnu::packed]] bender_general_s {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint16_t OFFSET = 100;
    char firmware_version[4];
    uint16_t padding[2];
    uint16_t ocpp_cp_state;
    uint32swapped_t errorcodes[4];
    uint16_t padding2[7];
    char protocol_version[4];
    uint16_t vehicle_state;
    uint16_t vehicle_state_hex;
    uint16_t chargepoint_available;
    uint16_t padding3[6];
    uint16_t safe_current;
    uint16_t comm_timeout;
    uint16_t hardware_curr_limit;
    uint16_t operator_curr_limit;
    uint16_t rcmb_mode;
    uint16_t rcmb_rms_int;
    uint16_t rcmb_rms_frac;
    uint16_t rcmb_dc_int;
    uint16_t rcmb_dc_frac;
    uint16_t relays_state;
    uint16_t device_id;
    uint32swapped_t charger_model[5];
    uint16_t pluglock_detected;
};

struct [[gnu::packed]] bender_phases_s {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint16_t OFFSET = 200;
    uint32swapped_t energy[3];
    uint32swapped_t power[3];
    uint32swapped_t current[3];
    uint32swapped_t total_energy;
    uint32swapped_t total_power;
    uint32swapped_t voltage[3];
};

struct [[gnu::packed]] bender_dlm_s {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint32_t OFFSET = 600;
    uint16_t dlm_mode;
    uint16_t padding[9];
    uint16_t evse_limit[3];
    uint16_t operator_evse_limit[3];
    uint16_t padding2[4];
    uint16_t external_meter_support;
    uint16_t number_of_slaves;
    uint16_t padding3[8];
    uint16_t overall_current_applied[3];
    uint16_t overall_current_available[3];
};

struct [[gnu::packed]] bender_charge_s {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint32_t OFFSET = 700;
    uint16_t padding[5];
    uint16_t wh_charged;
    uint16_t current_signaled;
    uint32swapped_t start_time;
    uint16_t charge_duration;
    uint32swapped_t end_time;
    uint16_t minimum_current_limit;
    uint32swapped_t ev_required_energy;
    uint16_t ev_max_curr;
    uint32swapped_t charged_energy;
    uint32swapped_t charge_duration_new;
    uint32swapped_t user_id[5];
    uint16_t padding3[10];
    uint32swapped_t evccid[3];
};

struct [[gnu::packed]] bender_hems_s {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint32_t OFFSET = 1000;
    uint16_t hems_limit;
};

struct [[gnu::packed]] bender_write_uid_s {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint32_t OFFSET = 1110;
    uint32swapped_t user_id[5];
};

struct [[gnu::packed]] keba_read_general_s {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint32_t OFFSET = 1000;
    uint32swapped_t charging_state;
    uint32swapped_t padding;
    uint32swapped_t cable_state;
    uint32swapped_t error_code;
    uint32swapped_t currents[3];
    uint32swapped_t serial_number;
    uint32swapped_t features;
    uint32swapped_t firmware_version;
    uint32swapped_t power;
    uint16_t padding2[14];
    uint32swapped_t total_energy;
    uint32swapped_t padding3;
    uint32swapped_t voltages[3];
    uint32swapped_t power_factor;
};

struct [[gnu::packed]] keba_read_max_s {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint32_t OFFSET = 1100;
    uint32swapped_t max_current;
    uint16_t padding[8];
    uint32swapped_t max_hardware_current;
};

struct [[gnu::packed]] keba_read_charge_s {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint32_t OFFSET = 1500;
    uint32swapped_t rfid_tag;
    uint32swapped_t charged_energy;
};

struct [[gnu::packed]] keba_write_s {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint32_t OFFSET = 5004;
    uint16_t set_charging_current;
    uint16_t padding[5];
    uint16_t set_energy;
    uint16_t padding2;
    uint16_t unlock_plug;
    uint16_t padding3;
    uint16_t enable_station;
    uint16_t padding4;
    uint16_t failsafe_current;
    uint16_t padding5;
    uint16_t failsafe_timeout;
    uint16_t padding6;
    uint16_t failsafe_persist;
};

//-------------------
// Discrete Inputs
//-------------------
struct discrete_inputs_t {
    static const mb_param_type_t TYPE = MB_PARAM_DISCRETE;
    static const uint16_t OFFSET = 0;
    bool evse : 1;
    bool meter : 1;
    bool meter_phases : 1;
    bool meter_all_values : 1;
    bool cp_disconnect : 1;
    bool nfc : 1;
};

struct meter_discrete_inputs_t {
    static const mb_param_type_t TYPE = MB_PARAM_DISCRETE;
    static const uint16_t OFFSET = 2100;
    bool phase_one_connected : 1;
    bool phase_two_connected : 1;
    bool phase_three_connected : 1;
    bool phase_one_active : 1;
    bool phase_two_active : 1;
    bool phase_three_active : 1;
};

//-------------------
// Coils
//-------------------
struct evse_coils_t {
    static const mb_param_type_t TYPE = MB_PARAM_COIL;
    static const uint16_t OFFSET = 1000;
    bool enable_charging : 1;
    bool autostart_button : 1;
};

static input_regs_t *input_regs, *input_regs_copy;
static evse_input_regs_t *evse_input_regs, *evse_input_regs_copy;
static meter_input_regs_t *meter_input_regs, *meter_input_regs_copy;
static meter_all_values_input_regs_t *meter_all_values_input_regs, *meter_all_values_input_regs_copy;
static nfc_input_regs_t *nfc_input_regs, *nfc_input_regs_copy;

static holding_regs_t *holding_regs, *holding_regs_copy;
static evse_holding_regs_t *evse_holding_regs, *evse_holding_regs_copy;
static meter_holding_regs_t *meter_holding_regs, *meter_holding_regs_copy;

static discrete_inputs_t *discrete_inputs, *discrete_inputs_copy;
static meter_discrete_inputs_t *meter_discrete_inputs, *meter_discrete_inputs_copy;

static evse_coils_t *evse_coils, *evse_coils_copy;

static bender_general_s *bender_general, *bender_general_cpy;
static bender_phases_s *bender_phases, *bender_phases_cpy;
static bender_dlm_s *bender_dlm, *bender_dlm_cpy;
static bender_charge_s *bender_charge, *bender_charge_cpy;
static bender_hems_s *bender_hems, *bender_hems_cpy;
static bender_write_uid_s *bender_write_uid, *bender_write_uid_cpy;

static keba_read_general_s *keba_read_general, *keba_read_general_cpy;
static keba_read_charge_s *keba_read_charge, *keba_read_charge_cpy;
static keba_read_max_s *keba_read_max, *keba_read_max_cpy;
static keba_write_s *keba_write, *keba_write_cpy;

static portMUX_TYPE mtx;

ModbusTcp::ModbusTcp()
{
}

void ModbusTcp::pre_setup()
{
    config = Config::Object({
        {"enable", Config::Bool(false)},
        {"port", Config::Uint16(502)},
        {"table", Config::Uint16(0)},
    });
}

static void allocate_table()
{
    calloc_struct(&input_regs);
    calloc_struct(&input_regs_copy);
    calloc_struct(&evse_input_regs);
    calloc_struct(&evse_input_regs_copy);
    calloc_struct(&meter_input_regs);
    calloc_struct(&meter_input_regs_copy);
    calloc_struct(&meter_all_values_input_regs);
    calloc_struct(&meter_all_values_input_regs_copy);
    calloc_struct(&nfc_input_regs);
    calloc_struct(&nfc_input_regs_copy);
    calloc_struct(&holding_regs);
    calloc_struct(&holding_regs_copy);
    calloc_struct(&evse_holding_regs);
    calloc_struct(&evse_holding_regs_copy);
    calloc_struct(&meter_holding_regs);
    calloc_struct(&meter_holding_regs_copy);
    calloc_struct(&discrete_inputs);
    calloc_struct(&discrete_inputs_copy);
    calloc_struct(&meter_discrete_inputs);
    calloc_struct(&meter_discrete_inputs_copy);
    calloc_struct(&evse_coils);
    calloc_struct(&evse_coils_copy);
}

static void allocate_bender_table()
{
    calloc_struct(&bender_general);
    calloc_struct(&bender_general_cpy);
    calloc_struct(&bender_phases);
    calloc_struct(&bender_phases_cpy);
    calloc_struct(&bender_dlm);
    calloc_struct(&bender_dlm_cpy);
    calloc_struct(&bender_charge);
    calloc_struct(&bender_charge_cpy);
    calloc_struct(&bender_hems);
    calloc_struct(&bender_hems_cpy);
    calloc_struct(&bender_write_uid);
    calloc_struct(&bender_write_uid_cpy);
}

static void allocate_keba_table()
{
    calloc_struct(&keba_read_charge);
    calloc_struct(&keba_read_charge_cpy);
    calloc_struct(&keba_read_general);
    calloc_struct(&keba_read_general_cpy);
    calloc_struct(&keba_read_max);
    calloc_struct(&keba_read_max_cpy);
    calloc_struct(&keba_write);
    calloc_struct(&keba_write_cpy);
}

void ModbusTcp::setup()
{
    api.restorePersistentConfig("modbus_tcp/config", &config);

    if (!config.get("enable")->asBool()) {
        initialized = true;
        return;
    }

    void *modbus_handle = NULL;
    esp_err_t err = mbc_slave_init_tcp(&modbus_handle);
    if (err != ESP_OK || modbus_handle == NULL)
        printf("Modbus init failed with code %i", err);

    mb_communication_info_t comm_info;
    comm_info.ip_addr = NULL;
    comm_info.mode = MB_MODE_TCP;
    comm_info.ip_addr_type = MB_IPV4;
    comm_info.ip_port = config.get("port")->asUint();
    // For some reason, mbc_slave_setup asserts that comm_info.ip_netif_ptr is not null,
    // but the ip_netif_ptr is never used.
    // Fortunately this means that we can just pass anything to circumvent the assertion
    // and the modbus_tcp server will listen on any network interface.
    comm_info.ip_netif_ptr = (void *)0x00000004;
    ESP_ERROR_CHECK(mbc_slave_setup((void *)&comm_info));

    mb_register_area_descriptor_t reg_area;

#define REGISTER_DESCRIPTOR(x) do { \
    reg_area.type = x->TYPE; \
    reg_area.start_offset = x->OFFSET; \
    reg_area.address = x; \
    reg_area.size = sizeof(*x); \
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area)); \
} while (0)

    uint32_t config_table = config.get("table")->asUint();

    if (config_table == 0)
    {
        allocate_table();

        REGISTER_DESCRIPTOR(evse_input_regs);
        REGISTER_DESCRIPTOR(meter_input_regs);
        REGISTER_DESCRIPTOR(meter_all_values_input_regs);
        REGISTER_DESCRIPTOR(input_regs);
        REGISTER_DESCRIPTOR(holding_regs);
        REGISTER_DESCRIPTOR(evse_holding_regs);
        REGISTER_DESCRIPTOR(meter_holding_regs);
        REGISTER_DESCRIPTOR(discrete_inputs);
        REGISTER_DESCRIPTOR(meter_discrete_inputs);
        REGISTER_DESCRIPTOR(evse_coils);
        REGISTER_DESCRIPTOR(nfc_input_regs);

        evse_holding_regs->led_blink_state = fromUint(-2);
    }
    else if (config_table == 1)
    {
        allocate_bender_table();

        REGISTER_DESCRIPTOR(bender_general);
        REGISTER_DESCRIPTOR(bender_phases);
        REGISTER_DESCRIPTOR(bender_dlm);
        REGISTER_DESCRIPTOR(bender_charge);
        REGISTER_DESCRIPTOR(bender_hems);
        REGISTER_DESCRIPTOR(bender_write_uid);
    }
    else if (config_table == 2)
    {
        allocate_keba_table();

        REGISTER_DESCRIPTOR(keba_read_charge);
        REGISTER_DESCRIPTOR(keba_read_general);
        REGISTER_DESCRIPTOR(keba_read_max);
        REGISTER_DESCRIPTOR(keba_write);
    }

    ESP_ERROR_CHECK(mbc_slave_start());

    started = true;
    initialized = true;
}

void ModbusTcp::update_bender_regs()
{
    taskENTER_CRITICAL(&mtx);
        *bender_general_cpy = *bender_general;
        *bender_dlm_cpy = *bender_dlm;
        *bender_hems_cpy = *bender_hems;
        *bender_write_uid_cpy = *bender_write_uid;
    taskEXIT_CRITICAL(&mtx);

    bool charging = false;

    bender_general_cpy->device_id = 0xEBEE;

#define NOT_SUPPORTED 0

    for (int i = 0; i < 4; i++)
        bender_general_cpy->errorcodes[i] = fromUint(NOT_SUPPORTED);
    for (int i = 0; i < 5; i++) {
        if (bender_write_uid_cpy->user_id[i]) {
            logger.printfln("Writing userid is not supported");
            bender_write_uid_cpy->user_id[i] = fromUint(NOT_SUPPORTED);
        }
    }

    if (bender_general_cpy->comm_timeout) {
        logger.printfln("Writing communication timeout is not supported");
        bender_general_cpy->comm_timeout = fromUint(NOT_SUPPORTED);
    }

    if (bender_general_cpy->safe_current) {
        logger.printfln("Writing safe current is not supported");
        bender_general_cpy->safe_current = fromUint(NOT_SUPPORTED);
    }

    for (int i = 0; i < 3; i++) {
        if (bender_dlm_cpy->operator_evse_limit[i]) {
            logger.printfln("Writing dlm operator current l%i is not supported", i);
            bender_dlm_cpy->operator_evse_limit[i] = fromUint(NOT_SUPPORTED);
        }
    }

    memcpy(bender_general_cpy->firmware_version, ".404", 4);
    memcpy(bender_general_cpy->protocol_version, "0\0006.", 4);

    if (api.hasFeature("evse")) {
        switch (api.getState("evse/state")->get("charger_state")->asUint()) {
        case 0:
            bender_general_cpy->ocpp_cp_state = 0;
            break;

        case 4:
            bender_general_cpy->ocpp_cp_state = 4;
            break;

        default:
            bender_general_cpy->ocpp_cp_state = 1;
            break;
        }
        bender_general_cpy->vehicle_state = api.getState("evse/state")->get("iec61851_state")->asUint() + 1;
        bender_general_cpy->vehicle_state_hex = api.getState("evse/state")->get("iec61851_state")->asUint() + 10;

        bender_general_cpy->hardware_curr_limit = api.getState("evse/slots")->get(1)->get("max_current")->asUint() / 1000;
        bender_charge_cpy->current_signaled = api.getState("evse/state")->get("allowed_charging_current")->asUint() / 1000;

        evse_common.set_modbus_current(bender_hems->hems_limit * 1000);
        evse_common.set_modbus_enabled(true);

#if MODULE_CHARGE_TRACKER_AVAILABLE()
        int32_t user_id = api.getState("charge_tracker/current_charge")->get("user_id")->asInt();
        charging = user_id != -1;
        if (charging) {
            bender_charge_cpy->charge_duration = fromUint((api.getState("evse/low_level_state")->get("uptime")->asUint() - api.getState("charge_tracker/current_charge")->get("evse_uptime_start")->asUint()) / 1000);
            bender_charge_cpy->charge_duration_new = fromUint((api.getState("evse/low_level_state")->get("uptime")->asUint() - api.getState("charge_tracker/current_charge")->get("evse_uptime_start")->asUint()) / 1000);
        } else {
            bender_charge_cpy->charge_duration = fromUint(0);
            bender_charge_cpy->charge_duration_new = fromUint(0);
        }
#endif
    }

#if MODULE_METERS_LEGACY_API_AVAILABLE()
    if (api.hasFeature("meter")) {
        if (api.hasFeature("meter_all_values")) {
            auto meter_values = api.getState("meter/all_values");

            for (int i = 0; i < 3; i++) {
                bender_phases_cpy->current[i] = fromUint(meter_values->get(i + METER_ALL_VALUES_CURRENT_L1_A)->asFloat() * 1000);
                bender_phases_cpy->energy[i] =  fromUint(meter_values->get(i + METER_ALL_VALUES_IMPORT_KWH_L1)->asFloat() * 1000);
                bender_phases_cpy->power[i] =  fromUint(meter_values->get(i + METER_ALL_VALUES_POWER_L1_W)->asFloat() * 1000);
                bender_phases_cpy->voltage[i] = fromUint(meter_values->get(i)->asFloat() * 1000);
            }
            bender_phases_cpy->total_energy = fromUint(meter_values->get(METER_ALL_VALUES_TOTAL_IMPORT_KWH)->asFloat() * 1000);
            bender_phases_cpy->total_power = fromUint(meter_values->get(METER_ALL_VALUES_TOTAL_SYSTEM_POWER_W)->asFloat());
        }

#if MODULE_CHARGE_TRACKER_AVAILABLE()
        auto meter_start = api.getState("charge_tracker/current_charge")->get("meter_start")->asFloat();
        auto meter_absolute = api.getState("meter/values")->get("energy_abs")->asFloat();
        if (!charging) {
            bender_charge_cpy->wh_charged = fromUint(0);
            bender_charge_cpy->charged_energy = fromUint(0);
        } else if (isnan(meter_start)) {
            bender_charge_cpy->wh_charged = 0;
            bender_charge_cpy->charged_energy = fromUint(0);
        } else {
            bender_charge_cpy->wh_charged = uint16_t((meter_absolute - meter_start) * 1000);
            bender_charge_cpy->charged_energy = fromUint((meter_absolute - meter_start) * 1000);
        }
#endif
    }
#endif

    taskENTER_CRITICAL(&mtx);
        *bender_charge = *bender_charge_cpy;
        *bender_dlm = *bender_dlm_cpy;
        *bender_general = *bender_general_cpy;
        *bender_hems = *bender_hems_cpy;
        *bender_phases = *bender_phases_cpy;
        *bender_write_uid = *bender_write_uid_cpy;
    taskEXIT_CRITICAL(&mtx);
}

static inline void swap_bytes(char *str, size_t len)
{
    char tmp;
    for (int i = 0; i < len; i += 2) {
        tmp = str[i];
        str[i] = str[i + 1];
        str[i + 1] = tmp;
    }
}

void ModbusTcp::update_regs()
{
    bool has_feature_evse = api.hasFeature("evse");

    bool call_start_charging = false;
    bool call_stop_charging = false;
    bool enable_charging = has_feature_evse && api.getState("evse/slots")->get(CHARGING_SLOT_MODBUS_TCP_ENABLE)->get("max_current")->asUint() == 32000;
    bool reset_meter = false;
    bool set_evse_led = false;

    bool autostart_slot = has_feature_evse && api.getState("evse/slots")->get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("max_current")->asUint() == 32000;

    // We want to keep the critical sections as small as possible
    // -> Do all work in a copy of the registers.
    // However if we read _and_ write a register, we have to write back the new value immediately.
    // Otherwise we would overwrite and ignore a value that was written between the critical sections.
    taskENTER_CRITICAL(&mtx);
        reset_meter = meter_holding_regs->trigger_reset == meter_holding_regs->TRIGGER_RESET_PASSWORD;
        // Clear the trigger_reset register to make sure the meter is reset only once.
        meter_holding_regs->trigger_reset = fromUint(0);

        if (evse_holding_regs->enable_charging != evse_holding_regs_copy->enable_charging)
            enable_charging = evse_holding_regs->enable_charging != 0;

        // Prefer (new!) enable charging coil over (deprecated!) holding register.
        if (evse_coils->enable_charging != evse_coils_copy->enable_charging)
            enable_charging = evse_coils->enable_charging;

        /*
        autostart_button logic
        ----------------------
        slot  reg reg_copy new_reg call     comment
        0     0   0        0       ---      nothing changed
        0     0   1        0       ---      master wrote a 0. slot changed to 0 (i.e. button was pressed/stop_charging was called)
        0     1   0        1       start    master wrote a 1. slot unchanged.
        0     1   1        0       ---      slot changed to 0. (i.e. button was pressed/stop_charging was called)
        1     0   0        1       ---      slot changed to 1. (i.e. button was pressed/start_charging was called)
        1     0   1        0       stop     master wrote a 0. slot unchanged.
        1     1   0        1       ---      master wrote a 1. slot changed to 1 (i.e. button was pressed/start_charging was called)
        1     1   1        1       ---      nothing changed

        new_reg = slot ? (reg || !copy) : (reg && !copy);
        */

        call_start_charging = !autostart_slot && evse_coils->autostart_button && !evse_coils_copy->autostart_button;
        call_stop_charging = autostart_slot && !evse_coils->autostart_button && evse_coils_copy->autostart_button;
        evse_coils->autostart_button = autostart_slot ? (evse_coils->autostart_button || !evse_coils_copy->autostart_button)
                                                      : (evse_coils->autostart_button && !evse_coils_copy->autostart_button);


        *holding_regs_copy = *holding_regs;
        *evse_holding_regs_copy = *evse_holding_regs;
        *meter_holding_regs_copy = *meter_holding_regs;
        *evse_coils_copy = *evse_coils;

        if (evse_holding_regs->led_blink_duration != 0 && evse_holding_regs->led_blink_state != -2) {
            set_evse_led = true;
            evse_holding_regs->led_blink_duration = fromUint(0);
            evse_holding_regs->led_blink_state = fromUint(-2);
        }
    taskEXIT_CRITICAL(&mtx);

    bool write_allowed = has_feature_evse && api.getState("evse/slots")->get(CHARGING_SLOT_MODBUS_TCP)->get("active")->asBool();
    bool charging = false;

    if (holding_regs_copy->reboot == holding_regs_copy->REBOOT_PASSWORD && write_allowed)
        trigger_reboot("Modbus TCP");

    input_regs_copy->table_version = fromUint(MODBUS_TABLE_VERSION);
    input_regs_copy->box_id = fromUint(local_uid_num);
    input_regs_copy->firmware_major = fromUint(BUILD_VERSION_MAJOR);
    input_regs_copy->firmware_minor = fromUint(BUILD_VERSION_MINOR);
    input_regs_copy->firmware_patch = fromUint(BUILD_VERSION_PATCH);
    input_regs_copy->firmware_build_ts = fromUint(build_timestamp());
    input_regs_copy->uptime = fromUint((uint32_t)(esp_timer_get_time() / 1000000));

#if MODULE_NFC_AVAILABLE()
    if (api.hasFeature("nfc")) {
        discrete_inputs_copy->nfc = true;
        auto tag = nfc.old_tags[0];
        auto injected_tag = nfc.old_tags[TAG_LIST_LENGTH - 1];
        char buf[NFC_TAG_ID_STRING_LENGTH + 1] = {0};
        if (tag.last_seen > injected_tag.last_seen && injected_tag.last_seen > 0)
            tag = injected_tag;
        remove_separator(tag.tag_id, buf);
        swap_bytes(buf, 20);
        memcpy(&nfc_input_regs_copy->tag_id, buf, 20);
        nfc_input_regs_copy->tag_id_age = fromUint(tag.last_seen);
    }
#endif

    if (has_feature_evse) {
        discrete_inputs_copy->evse = true;
        evse_input_regs_copy->iec_state = fromUint(api.getState("evse/state")->get("iec61851_state")->asUint());
        evse_input_regs_copy->charger_state = fromUint(api.getState("evse/state")->get("charger_state")->asUint());

        if (set_evse_led)
            evse_led.set_api(EvseLed::Blink((uint32_t)evse_holding_regs_copy->led_blink_state), evse_holding_regs_copy->led_blink_duration);

        evse_common.set_modbus_current(evse_holding_regs_copy->allowed_current);
        evse_common.set_modbus_enabled(enable_charging);

        auto slots = api.getState("evse/slots");

        for (int i = 0; i < slots->count(); i++) {
            uint32_t current = slots->get(i)->get("max_current")->asUint();
            uint32_t val = 0xFFFFFFFF;

            if (slots->get(i)->get("active")->asBool()) {
                val = current;
            }
            evse_input_regs_copy->slots[i] = fromUint(val);
        }

        evse_input_regs_copy->max_current = fromUint(api.getState("evse/state")->get("allowed_charging_current")->asUint());
        evse_input_regs_copy->start_time_min = fromUint(0);
        evse_input_regs_copy->charging_time_sec = fromUint(0);

        if (write_allowed && call_start_charging)
            api.callCommand("evse/start_charging", nullptr);
        if (write_allowed && call_stop_charging)
            api.callCommand("evse/stop_charging", nullptr);

#if MODULE_CHARGE_TRACKER_AVAILABLE()
        int32_t user_id = api.getState("charge_tracker/current_charge")->get("user_id")->asInt();
        charging = user_id != -1;
        evse_input_regs_copy->current_user = fromUint(charging ? UINT32_MAX : (uint32_t)user_id);
        if (charging) {
            evse_input_regs_copy->start_time_min = fromUint(api.getState("charge_tracker/current_charge")->get("timestamp_minutes")->asUint());
            evse_input_regs_copy->charging_time_sec = fromUint((api.getState("evse/low_level_state")->get("uptime")->asUint() - api.getState("charge_tracker/current_charge")->get("evse_uptime_start")->asUint()) / 1000);
        } else {
            evse_input_regs_copy->start_time_min = fromUint(0);
            evse_input_regs_copy->charging_time_sec = fromUint(0);
        }
#endif
    }

#if MODULE_METERS_LEGACY_API_AVAILABLE()
    if (api.hasFeature("meter")) {
        discrete_inputs_copy->meter = true;

        meter_input_regs_copy->meter_type = fromUint(api.getState("meter/state")->get("type")->asUint());

        auto meter_values = api.getState("meter/values");
        meter_input_regs_copy->power = fromFloat(meter_values->get("power")->asFloat());
        meter_input_regs_copy->energy_relative = fromFloat(meter_values->get("energy_rel")->asFloat());
        meter_input_regs_copy->energy_absolute = fromFloat(meter_values->get("energy_abs")->asFloat());
#if MODULE_CHARGE_TRACKER_AVAILABLE()
        auto meter_start = api.getState("charge_tracker/current_charge")->get("meter_start")->asFloat();
        if (!charging)
            meter_input_regs_copy->energy_this_charge = fromFloat(0);
        else if (isnan(meter_start))
            meter_input_regs_copy->energy_this_charge = fromFloat(NAN);
        else
            meter_input_regs_copy->energy_this_charge = fromFloat(meter_input_regs_copy->energy_absolute - meter_start);
#endif

        if (reset_meter && write_allowed)
            api.callCommand("meter/reset", {});
    }

    if (api.hasFeature("meter_phases")) {
        discrete_inputs_copy->meter_phases = true;

        auto meter_phase_values = api.getState("meter/phases");
        meter_discrete_inputs_copy->phase_one_active = meter_phase_values->get("phases_active")->get(0)->asBool();
        meter_discrete_inputs_copy->phase_two_active = meter_phase_values->get("phases_active")->get(1)->asBool();
        meter_discrete_inputs_copy->phase_three_active = meter_phase_values->get("phases_active")->get(2)->asBool();
        meter_discrete_inputs_copy->phase_one_connected = meter_phase_values->get("phases_connected")->get(0)->asBool();
        meter_discrete_inputs_copy->phase_two_connected = meter_phase_values->get("phases_connected")->get(1)->asBool();
        meter_discrete_inputs_copy->phase_three_connected = meter_phase_values->get("phases_connected")->get(2)->asBool();
    }

    if (api.hasFeature("meter_all_values")) {
        discrete_inputs_copy->meter_all_values = true;

        auto meter_all_values = api.getState("meter/all_values");

        for (int i = 0; i < 85; i++)
            meter_all_values_input_regs_copy->meter_values[i] = fromFloat(meter_all_values->get(i)->asFloat());
    }
#endif

    // DO NOT write back into holding registers and coils here.
    // Write ONLY input registers and discrete inputs.
    // If we want to read and write the same register, writing into it
    // has to be done in the first critical section, not this one.
    // Otherwise we would overwrite and ignore a value that was written between the critical sections.
    taskENTER_CRITICAL(&mtx);
        *input_regs = *input_regs_copy;
        *evse_input_regs = *evse_input_regs_copy;
        *meter_input_regs = *meter_input_regs_copy;
        *meter_all_values_input_regs = *meter_all_values_input_regs_copy;
        *discrete_inputs = *discrete_inputs_copy;
        *meter_discrete_inputs = *meter_discrete_inputs_copy;
        *nfc_input_regs = *nfc_input_regs_copy;
    taskEXIT_CRITICAL(&mtx);
}

uint32_t keba_get_features()
{
    static bool warned;
    uint32_t features = 31;
    features *= 10;
    if (api.hasFeature("evse"))
    {
        switch (api.getState("evse/hardware_configuration")->get("jumper_configuration")->asUint())
        {
        case 2:
            features += 1;
            break;

        case 3:
            features += 2;
            break;

        case 4:
            features += 3;
            break;

        case 6:
            features += 4;
            break;

        default:
            logger.printfln("No matching keba cable configuration! It will be set to 0!");
            break;
        }
    }
    features *= 10;
    features += 1;
    features *= 10;
    if (api.hasFeature("meter")) {
        features += 2;
    } else {
        if (!warned) {
            logger.printfln("Wallbox has no meter. Erros are expected!");
            warned = true;
        }
    }
    features *= 10;
    if (api.hasFeature("nfc"))
        features += 1;
    return features;
}

static uint8_t hextouint(const char c)
{
    uint8_t i = 0;
    if (c > 96)
        i = c - 'a' + 10;
    else if (c > 64)
        i = c - 'A' + 10;
    else
        i = c - '0';
    return i;
}

static uint32_t export_tag_id_as_uint32(const String &str)
{
    int str_idx = 0;
    char c[4];

    for (int i = 3; i >= 0; i--) {
        c[i] = hextouint(str[str_idx++]) << 4;
        c[i] |= hextouint(str[str_idx++]);
        str_idx++;
    }
    uint32_t *ret = reinterpret_cast<uint32_t *>(c);
    return *ret;
}

void ModbusTcp::update_keba_regs()
{
    taskENTER_CRITICAL(&mtx);
        *keba_write_cpy = *keba_write;
    taskEXIT_CRITICAL(&mtx);

    keba_read_general_cpy->features = fromUint(keba_get_features());
    keba_read_general_cpy->firmware_version = fromUint(0x30A1B00);

    if (api.hasFeature("evse")) {

        evse_common.set_modbus_current(keba_write_cpy->set_charging_current);
        evse_common.set_modbus_enabled(keba_write_cpy->enable_station == 1 ? true : false);

        if (api.getState("evse/state")->get("iec61851_state")->asUint() == 4)
            keba_read_general_cpy->charging_state = fromUint(4);
        else
            keba_read_general_cpy->charging_state = fromUint(api.getState("evse/state")->get("iec61851_state")->asUint() + 1);
        if (api.getState("evse/state")->get("charger_state")->asUint() == 0)
            keba_read_general_cpy->cable_state = fromUint(0);
        else if (api.getState("evse/state")->get("charger_state")->asUint() == 1 || api.getState("evse/state")->get("charger_state")->asUint() == 2)
            keba_read_general_cpy->cable_state = fromUint(3);
        else
            keba_read_general_cpy->cable_state = fromUint(7);

        keba_read_max_cpy->max_current = fromUint(api.getState("evse/state")->get("allowed_charging_current")->asUint());

        keba_read_max_cpy->max_hardware_current = fromUint(api.getState("evse/slots")->get(CHARGING_SLOT_INCOMING_CABLE)->get("max_current")->asUint());
    }

#if MODULE_METERS_LEGACY_API_AVAILABLE()
    if (api.hasFeature("meter")) {
#if MODULE_CHARGE_TRACKER_AVAILABLE()
        int32_t user_id = api.getState("charge_tracker/current_charge")->get("user_id")->asInt();
        bool charging = user_id != -1;
        auto meter_absolute = api.getState("meter/values")->get("energy_abs")->asFloat();
        auto meter_start = api.getState("charge_tracker/current_charge")->get("meter_start")->asFloat();

        if (!charging)
            keba_read_charge->charged_energy = fromUint(0);
        else if (isnan(meter_start))
            keba_read_charge->charged_energy = fromUint(0);
        else
            keba_read_charge->charged_energy = fromUint((uint32_t)((meter_absolute - meter_start) * 1000));
    }
#endif
#endif

#if MODULE_CHARGE_TRACKER_AVAILABLE()
    if (api.getState("charge_tracker/current_charge")->get("authorization_type")->asUint() == 2) {
        const auto &tag_id = api.getState("charge_tracker/current_charge")->get("authorization_info")->get("tag_id")->asString();
        keba_read_charge_cpy->rfid_tag = fromUint(export_tag_id_as_uint32(tag_id));
    }
#endif

#if MODULE_METERS_LEGACY_API_AVAILABLE()
    if (api.hasFeature("meter")) {
        if (api.hasFeature("meter_all_values")) {
            auto meter_all_values = api.getState("meter/all_values");
            for (int i = 0; i < 3; i++) {
                keba_read_general_cpy->currents[i] = fromUint(meter_all_values->get(i + METER_ALL_VALUES_CURRENT_L1_A)->asFloat() * 1000);
                keba_read_general_cpy->voltages[i] = fromUint(meter_all_values->get(i)->asFloat());
            }
            keba_read_general_cpy->power_factor = fromUint(meter_all_values->get(METER_ALL_VALUES_TOTAL_SYSTEM_POWER_FACTOR)->asFloat() * 1000);
        }
        keba_read_general_cpy->power = fromUint(api.getState("meter/values")->get("power")->asFloat() * 1000);
        keba_read_general_cpy->total_energy = fromUint(api.getState("meter/values")->get("energy_abs")->asFloat() * 1000);
    }
#endif

    taskENTER_CRITICAL(&mtx);
        *keba_read_charge = *keba_read_charge_cpy;
        *keba_read_general = *keba_read_general_cpy;
        *keba_read_max = *keba_read_max_cpy;
    taskEXIT_CRITICAL(&mtx);
}

void ModbusTcp::register_urls()
{
    api.addPersistentConfig("modbus_tcp/config", &config);

    if (!config.get("enable")->asBool()) {
        return;
    }

    spinlock_initialize(&mtx);

    uint32_t config_table = config.get("table")->asUint();

    if (config_table == 0) {
        uint16_t allowed_current = 32000;
        bool enable_charging = false;
        bool autostart_button = false;

        if (api.hasFeature("evse")) {
            auto slots = api.getState("evse/slots");
            allowed_current = slots->get(CHARGING_SLOT_MODBUS_TCP)->get("max_current")->asUint();
            enable_charging = slots->get(CHARGING_SLOT_MODBUS_TCP_ENABLE)->get("max_current")->asUint() == 32000;
            autostart_button = slots->get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("max_current")->asUint() == 32000;
        }

        taskENTER_CRITICAL(&mtx);
            input_regs->table_version = fromUint(MODBUS_TABLE_VERSION);
            input_regs->box_id = fromUint(local_uid_num);
            input_regs->firmware_major = fromUint(BUILD_VERSION_MAJOR);
            input_regs->firmware_minor = fromUint(BUILD_VERSION_MINOR);
            input_regs->firmware_patch = fromUint(BUILD_VERSION_PATCH);
            input_regs->firmware_build_ts = fromUint(build_timestamp());

            evse_holding_regs->allowed_current = fromUint(allowed_current);
            evse_holding_regs->enable_charging = fromUint(enable_charging);

            evse_coils->autostart_button = autostart_button;
            evse_coils_copy->autostart_button = autostart_button;

            evse_coils->enable_charging = enable_charging;
            evse_coils_copy->enable_charging = enable_charging;
        taskEXIT_CRITICAL(&mtx);

        task_scheduler.scheduleWithFixedDelay([this]() {
            this->update_regs();
        }, 0, 500);
    }
    else if (config_table == 1)
    {
        if (api.hasFeature("evse"))
        {
            auto slots = api.getState("evse/slots");
            uint16_t current = slots->get(CHARGING_SLOT_MODBUS_TCP)->get("max_current")->asUint() / 1000;
            uint16_t enable = slots->get(CHARGING_SLOT_MODBUS_TCP_ENABLE)->get("max_current")->asUint() == 32000 ? 1 : 0;
            taskENTER_CRITICAL(&mtx);
                bender_hems->hems_limit = current;
                bender_general->chargepoint_available = enable;
            taskEXIT_CRITICAL(&mtx);
        }

        task_scheduler.scheduleWithFixedDelay([this]() {
            this->update_bender_regs();
        }, 0, 500);
    }
    else if (config_table == 2)
    {
        if (api.hasFeature("evse"))
        {
            auto slots = api.getState("evse/slots");
            uint16_t current = slots->get(CHARGING_SLOT_MODBUS_TCP)->get("max_current")->asUint() / 1000;
            uint16_t enable = slots->get(CHARGING_SLOT_MODBUS_TCP_ENABLE)->get("max_current")->asUint() == 32000 ? 1 : 0;
            taskENTER_CRITICAL(&mtx);
                keba_write->set_charging_current = current;
                keba_write->enable_station = enable;
            taskEXIT_CRITICAL(&mtx);
        }

        task_scheduler.scheduleWithFixedDelay([this]() {
            this->update_keba_regs();
        }, 0, 500);
    }
}

void ModbusTcp::pre_reboot()
{
    if (started) {
        ESP_ERROR_CHECK(mbc_slave_destroy());
    }
}
