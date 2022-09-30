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

extern TaskScheduler task_scheduler;
extern API api;
extern uint32_t local_uid_num;

#if MODULE_EVSE_V2_AVAILABLE()
extern EVSEV2 evse_v2;
#endif

#if MODULE_EVSE_AVAILABLE()
extern EVSE evse;
#endif


static portMUX_TYPE mtx;

void ModbusTCP::pre_setup()
{
    config = Config::Object({
        {"enabled", Config::Bool(false)},
    });
}

void ModbusTCP::setup()
{
    api.restorePersistentConfig("modbus_tcp/config", &config);

    if (config.get("enabled")->asBool() == true)
    {
        holding_base = (holding_s *)calloc(1, sizeof(holding_s));
        evse_holding_base = (evse_holding_s *)calloc(1, sizeof(evse_holding_s));
        meter_holding_base = (meter_holding_s *)calloc(1, sizeof(meter_holding_s));

        input_base = (input_s *)calloc(1, sizeof(input_s));
        evse_base = (evse_s *)calloc(1, sizeof(evse_s));
        meter_base = (meter_s *)calloc(1, sizeof(meter_s));
        meter_all_values_base = (meter_all_values_s *)calloc(1, sizeof(meter_all_values_s));

        features = (feature_bitmap_s *)calloc(1, sizeof(feature_bitmap_s));
        meter_phases = (meter_bitmap_s *)calloc(1, sizeof(meter_bitmap_s));


        void *modbus_handle = NULL;
        esp_err_t err = mbc_slave_init_tcp(&modbus_handle);
        if (err != ESP_OK || modbus_handle == NULL)
            printf("Modbus init failed with code %i", err);

        mb_communication_info_t comm_info;
        comm_info.ip_addr = NULL;
        comm_info.mode = MB_MODE_TCP;
        comm_info.ip_addr_type = MB_IPV4;
        comm_info.ip_port = 502;
        ESP_ERROR_CHECK(mbc_slave_setup((void *)&comm_info));

        mb_register_area_descriptor_t reg_area;

        reg_area.type = MB_PARAM_HOLDING;
        reg_area.start_offset = 0;
        reg_area.address = (void *)holding_base;
        reg_area.size = sizeof(holding_s);
        ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

        reg_area.type = MB_PARAM_HOLDING;
        reg_area.start_offset = 1000;
        reg_area.address = (void *)evse_holding_base;
        reg_area.size = sizeof(evse_holding_s);
        ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

        reg_area.type = MB_PARAM_HOLDING;
        reg_area.start_offset = 2000;
        reg_area.address = (void *)meter_holding_base;
        reg_area.size = sizeof(meter_holding_s);
        ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));


        reg_area.type = MB_PARAM_INPUT;
        reg_area.start_offset = 0;
        reg_area.address = (void *)input_base;
        reg_area.size = sizeof(input_s);
        ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

        reg_area.type = MB_PARAM_INPUT;
        reg_area.start_offset = 1000;
        reg_area.address = (void *)evse_base;
        reg_area.size = sizeof(evse_s);
        ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

        reg_area.type = MB_PARAM_INPUT;
        reg_area.start_offset = 2000;
        reg_area.address = (void *)meter_base;
        reg_area.size = sizeof(meter_s);
        ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

        reg_area.type = MB_PARAM_INPUT;
        reg_area.start_offset = 2100;
        reg_area.address = (void *)meter_all_values_base;
        reg_area.size = sizeof(meter_all_values_s);
        ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));


        reg_area.type = MB_PARAM_DISCRETE;
        reg_area.start_offset = 0;
        reg_area.address = (void *)features;
        reg_area.size = sizeof(feature_bitmap_s);
        printf("%u\n", sizeof(feature_bitmap_s));
        ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

        reg_area.type = MB_PARAM_DISCRETE;
        reg_area.start_offset = 2000;
        reg_area.address = (void *)meter_phases;
        reg_area.size = sizeof(meter_bitmap_s);
        ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

        ESP_ERROR_CHECK(mbc_slave_start());

    }

    initialized = true;
}

void ModbusTCP::loop() {}

void ModbusTCP::register_urls()
{
    api.addPersistentConfig("modbus_tcp/config", &config, {}, 1000);

    if (config.get("enabled")->asBool() == true)
    {
        spinlock_initialize(&mtx);

        portENTER_CRITICAL(&mtx);
        input_base->table_version = MODBUS_TABLE_VERSION;
        input_base->box_id = local_uid_num;
        input_base->firmware_major = BUILD_VERSION_MAJOR;
        input_base->firmware_minor = BUILD_VERSION_MINOR;
        input_base->firmware_patch = BUILD_VERSION_PATCH;
        input_base->firmware_build_ts = BUILD_TIMESTAMP;

        evse_holding_base->allowed_current = 32000;
        evse_holding_base->enable_charging = 1;
        portEXIT_CRITICAL(&mtx);

        task_scheduler.scheduleWithFixedDelay([this]() {
            bool reboot_flag = false;

            portENTER_CRITICAL(&mtx);

            if (this->holding_base->reboot == 0x012EB007 && api.getState("evse/slots")->get(9)->get("active")->asBool())
                reboot_flag = true;

#if MODULE_METER_AVAILABLE()
            if (api.hasFeature("meter"))
            {
                features->meter = 1;

                meter_base->meter_type = api.getState("meter/state")->get("type")->asUint();

                auto meter_values = api.getState("meter/values");
                meter_base->power = meter_values->get("power")->asFloat();
                meter_base->energy_raltive = meter_values->get("energy_rel")->asFloat();
                meter_base->energy_absolute = meter_values->get("energy_abs")->asFloat();

                if (meter_holding_base->trigger_reset == 1)
                    api.callCommand("meter/reset", {});
            }

            if (api.hasFeature("meter_phases"))
            {
                features->meter_phases = 1;

                auto meter_phase_values = api.getState("meter/phases");
                meter_phases->phase_one_active = meter_phase_values->get("phases_active")->get(0)->asBool() ? 1 : 0;
                meter_phases->phase_two_active = meter_phase_values->get("phases_active")->get(1)->asBool() ? 1 : 0;
                meter_phases->phase_three_active = meter_phase_values->get("phases_active")->get(2)->asBool() ? 1 : 0;
                meter_phases->phase_one_connected = meter_phase_values->get("phases_connected")->get(0)->asBool() ? 1 : 0;
                meter_phases->phase_two_connected = meter_phase_values->get("phases_connected")->get(1)->asBool() ? 1 : 0;
                meter_phases->phase_three_connected = meter_phase_values->get("phases_connected")->get(2)->asBool() ? 1 : 0;
            }

            if (api.hasFeature("meter_all_values"))
            {
                features->meter_all_values = 1;

                auto meter_all_values = api.getState("meter/all_values");
                for (int i = 0; i < 85; i++)
                    meter_all_values_base->meter_values[i] = meter_all_values->get(i)->asFloat();
            }
#endif

#if MODULE_EVSE_V2_AVAILABLE() || MODULE_EVSE_AVAILABLE()
            if (api.hasFeature("evse"))
            {
                features->evse = 1;

                evse_base->iec_state = api.getState("evse/state")->get("iec61851_state")->asUint();
                evse_base->charger_state = api.getState("evse/state")->get("charger_state")->asUint();

                auto slots = api.getState("evse/slots");

#if MODULE_EVSE_V2_AVAILABLE()
                evse_v2.set_modbus_current(evse_holding_base->allowed_current);
                evse_v2.set_modbus_enabled(evse_holding_base->enable_charging);
#else
                evse.set_modbus_current(evse_holding_base->allowed_current);
                evse.set_modbus_enabled(evse_holding_base->enable_charging);
#endif

                uint32_t min = 32000;
                for (int i = 0; i < slots->count(); i++)
                {
                    uint32_t slot = slots->get(i)->get("max_current")->asUint();
                    uint32_t val = 0xFFFFFFFF;

                    if (slots->get(i)->get("active")->asBool() == true)
                    {
                        val = slot;
                        if (val < min)
                            min = val;
                    }
                    evse_base->slots[i] = val;
                }
                evse_base->max_current = min;
            }
#endif

#if MODULE_CHARGE_TRACKER_AVAILABLE()
            timeval time;
            evse_base->start_time_min = api.getState("charge_tracker/current_charge")->get("timestamp_minutes")->asUint();
            if (evse_base->start_time_min != 0 && clock_synced(&time))
                evse_base->charging_time_sec = time.tv_sec - evse_base->start_time_min * 60;
            evse_base->current_user = api.getState("charge_tracker/current_charge")->get("user_id")->asInt();
#endif

            portEXIT_CRITICAL(&mtx);
            if (reboot_flag)
                esp_restart();
        }, 0, 500);
    }
}
