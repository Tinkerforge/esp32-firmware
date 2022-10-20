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

// #include "modules.h"

#include <Arduino.h>

#include "esp_modbus_common.h"
#include "esp_modbus_slave.h"
#include "esp_netif.h"

#include "modules.h"
#include "modbus_tcp.h"
#include "build.h"
#include "build_timestamp.h"
#include "math.h"

extern TaskScheduler task_scheduler;
extern API api;
extern uint32_t local_uid_num;

#if MODULE_EVSE_V2_AVAILABLE()
extern EVSEV2 evse_v2;
#endif

#if MODULE_EVSE_AVAILABLE()
extern EVSE evse;
#endif

#define MODBUS_TABLE_VERSION 1
//-------------------
// Input Registers
//-------------------
struct input_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_INPUT;
    static const uint16_t OFFSET = 0;
    uint32_t table_version;
    uint32_t firmware_major;
    uint32_t firmware_minor;
    uint32_t firmware_patch;
    uint32_t firmware_build_ts;
    uint32_t box_id;
    uint32_t uptime;
};

struct evse_input_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_INPUT;
    static const uint16_t OFFSET = 1000;
    uint32_t iec_state;
    uint32_t charger_state;
    uint32_t current_user;
    uint32_t start_time_min;
    uint32_t charging_time_sec;
    uint32_t max_current;
    uint32_t slots[CHARGING_SLOT_COUNT_SUPPORTED_BY_EVSE];
};

struct meter_input_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_INPUT;
    static const uint16_t OFFSET = 2000;
    uint32_t meter_type;
    float power;
    float energy_absolute;
    float energy_relative;
    float energy_this_charge;
};

struct meter_all_values_input_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_INPUT;
    static const uint16_t OFFSET = 2100;
    float meter_values[85];
};

//-------------------
// Holding Registers
//-------------------
struct holding_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint16_t OFFSET = 0;
    static const uint32_t REBOOT_PASSWORD = 0x012EB007;
    uint32_t reboot;
};

struct evse_holding_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint16_t OFFSET = 1000;
    uint32_t enable_charging;
    uint32_t allowed_current;
};

struct meter_holding_regs_t {
    static const mb_param_type_t TYPE = MB_PARAM_HOLDING;
    static const uint16_t OFFSET = 2000;
    static const uint32_t TRIGGER_RESET_PASSWORD = 0x3E12E5E7;
    uint32_t trigger_reset;
};

//-------------------
// Discrete Inputs
//-------------------
struct discrete_inputs_t {
    static const mb_param_type_t TYPE = MB_PARAM_DISCRETE;
    static const uint16_t OFFSET = 0;
    bool evse:1;
    bool meter:1;
    bool meter_phases:1;
    bool meter_all_values:1;
};

struct meter_discrete_inputs_t {
    static const mb_param_type_t TYPE = MB_PARAM_DISCRETE;
    static const uint16_t OFFSET = 2100;
    bool phase_one_connected:1;
    bool phase_two_connected:1;
    bool phase_three_connected:1;
    bool phase_one_active:1;
    bool phase_two_active:1;
    bool phase_three_active:1;
};

static input_regs_t input_regs, input_regs_copy;
static evse_input_regs_t evse_input_regs, evse_input_regs_copy;
static meter_input_regs_t meter_input_regs, meter_input_regs_copy;
static meter_all_values_input_regs_t meter_all_values_input_regs, meter_all_values_input_regs_copy;

static holding_regs_t holding_regs, holding_regs_copy;
static evse_holding_regs_t evse_holding_regs, evse_holding_regs_copy;
static meter_holding_regs_t meter_holding_regs, meter_holding_regs_copy;

static discrete_inputs_t discrete_inputs, discrete_inputs_copy;
static meter_discrete_inputs_t meter_discrete_inputs, meter_discrete_inputs_copy;

static portMUX_TYPE mtx;

ModbusTcp::ModbusTcp()
{
    memset(&input_regs, 0, sizeof(input_regs));
    memset(&evse_input_regs, 0, sizeof(evse_input_regs));
    memset(&meter_input_regs, 0, sizeof(meter_input_regs));
    memset(&meter_all_values_input_regs, 0, sizeof(meter_all_values_input_regs));
    memset(&holding_regs, 0, sizeof(holding_regs));
    memset(&evse_holding_regs, 0, sizeof(evse_holding_regs));
    memset(&meter_holding_regs, 0, sizeof(meter_holding_regs));
    memset(&discrete_inputs, 0, sizeof(discrete_inputs));
    memset(&meter_discrete_inputs, 0, sizeof(meter_discrete_inputs));

    memset(&input_regs_copy, 0, sizeof(input_regs));
    memset(&evse_input_regs_copy, 0, sizeof(evse_input_regs));
    memset(&meter_input_regs_copy, 0, sizeof(meter_input_regs));
    memset(&meter_all_values_input_regs_copy, 0, sizeof(meter_all_values_input_regs));
    memset(&holding_regs_copy, 0, sizeof(holding_regs));
    memset(&evse_holding_regs_copy, 0, sizeof(evse_holding_regs));
    memset(&meter_holding_regs_copy, 0, sizeof(meter_holding_regs));
    memset(&discrete_inputs_copy, 0, sizeof(discrete_inputs));
    memset(&meter_discrete_inputs_copy, 0, sizeof(meter_discrete_inputs));

    // Initialize all slots to 0xFFFFFFFF (i.e. "this slot is not active")
    // This means that we can add more slots (up to the currently supported maximum of 20)
    // and Modbus TCP users can already support those.
    memset(&evse_input_regs.slots, 0xFF, sizeof(evse_input_regs.slots));
    memset(&evse_input_regs_copy.slots, 0xFF, sizeof(evse_input_regs.slots));
}

void ModbusTcp::pre_setup()
{
    config = Config::Object({
        {"enable", Config::Bool(false)},
        {"port", Config::Uint16(502)},
    });
}

void ModbusTcp::setup()
{
    api.restorePersistentConfig("modbus_tcp/config", &config);

    if (config.get("enable")->asBool() == true)
    {
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
        comm_info.ip_netif_ptr = (void *) 0x12345678;
        ESP_ERROR_CHECK(mbc_slave_setup((void *)&comm_info));

        mb_register_area_descriptor_t reg_area;

#define REGISTER_DESCRIPTOR(x) do { \
    reg_area.type = x.TYPE; \
    reg_area.start_offset = x.OFFSET; \
    reg_area.address = &x; \
    reg_area.size = sizeof(x); \
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area)); \
} while (0)

        REGISTER_DESCRIPTOR(evse_input_regs);
        REGISTER_DESCRIPTOR(meter_input_regs);
        REGISTER_DESCRIPTOR(meter_all_values_input_regs);
        REGISTER_DESCRIPTOR(input_regs);
        REGISTER_DESCRIPTOR(holding_regs);
        REGISTER_DESCRIPTOR(evse_holding_regs);
        REGISTER_DESCRIPTOR(meter_holding_regs);
        REGISTER_DESCRIPTOR(discrete_inputs);
        REGISTER_DESCRIPTOR(meter_discrete_inputs);

        ESP_ERROR_CHECK(mbc_slave_start());
    }

    initialized = true;
}

void ModbusTcp::update_regs() {
    // We want to keep the critical sections as small as possible
    // -> Do all work in a copy of the registers.
    portENTER_CRITICAL(&mtx);
        holding_regs_copy = holding_regs;
        evse_holding_regs_copy = evse_holding_regs;
        meter_holding_regs_copy = meter_holding_regs;
    portEXIT_CRITICAL(&mtx);

    bool write_allowed = false;
    if (api.hasFeature("evse"))
        api.getState("evse/slots")->get(CHARGING_SLOT_MODBUS_TCP)->get("active")->asBool();
    bool charging = false;

    if (holding_regs_copy.reboot == holding_regs_copy.REBOOT_PASSWORD && write_allowed)
        trigger_reboot("Modbus TCP");

    input_regs_copy.table_version = MODBUS_TABLE_VERSION;
    input_regs_copy.box_id = local_uid_num;
    input_regs_copy.firmware_major = BUILD_VERSION_MAJOR;
    input_regs_copy.firmware_minor = BUILD_VERSION_MINOR;
    input_regs_copy.firmware_patch = BUILD_VERSION_PATCH;
    input_regs_copy.firmware_build_ts = BUILD_TIMESTAMP;
    input_regs_copy.uptime = (uint32_t)(esp_timer_get_time() / 1000000);

#if MODULE_EVSE_V2_AVAILABLE() || MODULE_EVSE_AVAILABLE()
    if (api.hasFeature("evse"))
    {
        discrete_inputs_copy.evse = true;

        evse_input_regs_copy.iec_state = api.getState("evse/state")->get("iec61851_state")->asUint();
        evse_input_regs_copy.charger_state = api.getState("evse/state")->get("charger_state")->asUint();

#if MODULE_EVSE_V2_AVAILABLE()
        evse_v2.set_modbus_current(evse_holding_regs_copy.allowed_current);
        evse_v2.set_modbus_enabled(evse_holding_regs_copy.enable_charging);
#elif
        evse.set_modbus_current(evse_holding_regs_copy.allowed_current);
        evse.set_modbus_enabled(evse_holding_regs_copy.enable_charging);
#endif

        auto slots = api.getState("evse/slots");

        for (int i = 0; i < slots->count(); i++)
        {
            uint32_t current = slots->get(i)->get("max_current")->asUint();
            uint32_t val = 0xFFFFFFFF;

            if (slots->get(i)->get("active")->asBool() == true)
            {
                val = current;
            }
            evse_input_regs_copy.slots[i] = val;
        }

        evse_input_regs_copy.max_current = api.getState("evse/state")->get("allowed_charging_current")->asUint();
        evse_input_regs_copy.start_time_min = 0;
        evse_input_regs_copy.charging_time_sec = 0;

#if MODULE_CHARGE_TRACKER_AVAILABLE()
        int32_t user_id = api.getState("charge_tracker/current_charge")->get("user_id")->asInt();
        charging = user_id != -1;
        evse_input_regs_copy.current_user = charging ? UINT32_MAX : (uint32_t)user_id;
        if (charging) {
            evse_input_regs_copy.start_time_min = api.getState("charge_tracker/current_charge")->get("timestamp_minutes")->asUint();
            evse_input_regs_copy.charging_time_sec = (api.getState("evse/low_level_state")->get("uptime")->asUint() - api.getState("charge_tracker/current_charge")->get("evse_uptime_start")->asUint()) / 1000;
        } else {
            evse_input_regs_copy.start_time_min = 0;
            evse_input_regs_copy.charging_time_sec = 0;
        }
#endif
    }
#endif

#if MODULE_METER_AVAILABLE()
    if (api.hasFeature("meter"))
    {
        discrete_inputs_copy.meter = true;

        meter_input_regs_copy.meter_type = api.getState("meter/state")->get("type")->asUint();

        auto meter_values = api.getState("meter/values");
        meter_input_regs_copy.power = meter_values->get("power")->asFloat();
        meter_input_regs_copy.energy_relative = meter_values->get("energy_rel")->asFloat();
        meter_input_regs_copy.energy_absolute = meter_values->get("energy_abs")->asFloat();
#if MODULE_CHARGE_TRACKER_AVAILABLE()
        auto meter_start = api.getState("charge_tracker/current_charge")->get("meter_start")->asFloat();
        if (!charging)
            meter_input_regs_copy.energy_this_charge = 0;
        else if (isnan(meter_start))
            meter_input_regs_copy.energy_this_charge = NAN;
        else
            meter_input_regs_copy.energy_this_charge = meter_input_regs_copy.energy_absolute - meter_start;
#endif

        if (meter_holding_regs_copy.trigger_reset == meter_holding_regs_copy.TRIGGER_RESET_PASSWORD && write_allowed)
            api.callCommand("meter/reset", {});
    }

    if (api.hasFeature("meter_phases"))
    {
        discrete_inputs_copy.meter_phases = true;

        auto meter_phase_values = api.getState("meter/phases");
        meter_discrete_inputs_copy.phase_one_active = meter_phase_values->get("phases_active")->get(0)->asBool();
        meter_discrete_inputs_copy.phase_two_active = meter_phase_values->get("phases_active")->get(1)->asBool();
        meter_discrete_inputs_copy.phase_three_active = meter_phase_values->get("phases_active")->get(2)->asBool();
        meter_discrete_inputs_copy.phase_one_connected = meter_phase_values->get("phases_connected")->get(0)->asBool();
        meter_discrete_inputs_copy.phase_two_connected = meter_phase_values->get("phases_connected")->get(1)->asBool();
        meter_discrete_inputs_copy.phase_three_connected = meter_phase_values->get("phases_connected")->get(2)->asBool();
    }

    if (api.hasFeature("meter_all_values"))
    {
        discrete_inputs_copy.meter_all_values = true;

        auto meter_all_values = api.getState("meter/all_values");
        meter_all_values->fillFloatArray(meter_all_values_input_regs_copy.meter_values, sizeof(meter_all_values_input_regs_copy.meter_values) / sizeof(meter_all_values_input_regs_copy.meter_values[0]));
    }
#endif

    portENTER_CRITICAL(&mtx);
        input_regs = input_regs_copy;
        evse_input_regs = evse_input_regs_copy;
        meter_input_regs = meter_input_regs_copy;
        meter_all_values_input_regs = meter_all_values_input_regs_copy;
        discrete_inputs = discrete_inputs_copy;
        meter_discrete_inputs = meter_discrete_inputs_copy;
    portEXIT_CRITICAL(&mtx);
}

void ModbusTcp::register_urls()
{
    api.addPersistentConfig("modbus_tcp/config", &config, {}, 1000);

    if (config.get("enable")->asBool() == true)
    {
        spinlock_initialize(&mtx);

        uint16_t allowed_current = 32000;
        uint8_t enable_charging = 1;

#if MODULE_EVSE_V2_AVAILABLE() || MODULE_EVSE_AVAILABLE()
        auto slots = api.getState("evse/slots");
        allowed_current = slots->get(CHARGING_SLOT_MODBUS_TCP)->get("max_current")->asUint();
        enable_charging = slots->get(CHARGING_SLOT_MODBUS_TCP_ENABLE)->get("max_current")->asUint() == 32000;
#endif

    #if MODULE_EVSE_V2_AVAILABLE() || MODULE_EVSE_AVAILABLE()
            if (api.hasFeature("evse"))
            {
                auto slots = api.getState("evse/slots");
                allowed_current = slots->get(CHARGING_SLOT_MODBUS_TCP)->get("max_current")->asUint();
                enable_charging = slots->get(CHARGING_SLOT_MODBUS_TCP_ENABLE)->get("max_current")->asUint() == 32000;
            }
    #endif

        portENTER_CRITICAL(&mtx);
            input_regs.table_version = MODBUS_TABLE_VERSION;
            input_regs.box_id = local_uid_num;
            input_regs.firmware_major = BUILD_VERSION_MAJOR;
            input_regs.firmware_minor = BUILD_VERSION_MINOR;
            input_regs.firmware_patch = BUILD_VERSION_PATCH;
            input_regs.firmware_build_ts = BUILD_TIMESTAMP;

            evse_holding_regs.allowed_current = allowed_current;
            evse_holding_regs.enable_charging = enable_charging;
        portEXIT_CRITICAL(&mtx);

        task_scheduler.scheduleWithFixedDelay([this]() {
            this->update_regs();
        }, 0, 500);
    }
}

void ModbusTcp::loop() {}
