/* esp32-firmware
 * Copyright (C) 2022 Olaf Lüke <olaf@tinkerforge.com>
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

#include "em_meter.h"

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "tools.h"
#include "task_scheduler.h"
#include "web_server.h"
#include "modules.h"

extern EventLog logger;

extern TF_HAL hal;
extern WebServer server;
extern TaskScheduler task_scheduler;
extern Config modules;

extern API api;

#define ALL_VALUES_COUNT 85

EMMeter::EMMeter()
{
    state = Config::Object({
        {"state", Config::Uint8(0)}, // 0 - no energy meter, 1 - initialization error, 2 - meter available
        {"type", Config::Uint8(0)} // 0 - not available, 1 - sdm72, 2 - sdm630, 3 - sdm72v2
    });

    values = Config::Object({
        {"power", Config::Float(0.0)},
        {"energy_rel", Config::Float(0.0)},
        {"energy_abs", Config::Float(0.0)},
    });

    phases = Config::Object({
        {"phases_connected", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false)},
            new Config{Config::Bool(false)},
            3, 3, Config::type_id<Config::ConfBool>())},
        {"phases_active", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false)},
            new Config{Config::Bool(false)},
            3, 3, Config::type_id<Config::ConfBool>())}
    });

    all_values = Config::Array({},
        new Config{Config::Float(0)},
        0, ALL_VALUES_COUNT, Config::type_id<Config::ConfFloat>());

    errors = Config::Object({
        {"local_timeout", Config::Uint32(0)},
        {"global_timeout", Config::Uint32(0)},
        {"illegal_function", Config::Uint32(0)},
        {"illegal_data_access", Config::Uint32(0)},
        {"illegal_data_value", Config::Uint32(0)},
        {"slave_device_failure", Config::Uint32(0)},
    });

    reset = Config::Null();
}

void EMMeter::setupEM(bool update_module_initialized)
{
    energy_manager.update_all_data();

    uint8_t meter_type = energy_manager.all_data.energy_meter_type;

    if (meter_type == 0) {
        task_scheduler.scheduleOnce([this](){
            this->setupEM(true);
        }, 3000);
        return;
    }

    state.get("state")->updateUint(2);
    state.get("type")->updateUint(meter_type);
    hardware_available = true;

    for (int i = 0; i < power_history.size(); ++i) {
        //float f = 5000.0 * sin(PI/120.0 * i) + 5000.0;
        // Use negative state to mark that these are pre-filled.
        power_history.push(-1);
    }

    for (int i = 0; i < ALL_VALUES_COUNT; ++i) {
        all_values.add();
    }

    task_scheduler.scheduleWithFixedDelay([this](){
        float power = energy_manager.all_data.power;
        values.get("power")->updateFloat(power);
        values.get("energy_rel")->updateFloat(energy_manager.all_data.energy_relative);
        values.get("energy_abs")->updateFloat(energy_manager.all_data.energy_absolute);

        for(int i = 0; i < 3; ++i)
            phases.get("phases_active")->get(i)->updateBool(energy_manager.all_data.phases_active[i]);

        for(int i = 0; i < 3; ++i)
            phases.get("phases_connected")->get(i)->updateBool(energy_manager.all_data.phases_connected[i]);

        errors.get("local_timeout")->updateUint(energy_manager.all_data.error_count[0]);
        errors.get("global_timeout")->updateUint(energy_manager.all_data.error_count[1]);
        errors.get("illegal_function")->updateUint(energy_manager.all_data.error_count[2]);
        errors.get("illegal_data_access")->updateUint(energy_manager.all_data.error_count[3]);
        errors.get("illegal_data_value")->updateUint(energy_manager.all_data.error_count[4]);
        errors.get("slave_device_failure")->updateUint(energy_manager.all_data.error_count[5]);

        int16_t val = (int16_t)min((float)INT16_MAX, power);
        interval_samples.push(val);
        ++samples_last_interval;
    }, 500, 500);

    task_scheduler.scheduleWithFixedDelay([this](){
        float interval_sum = 0;
        int16_t val;
        for(int i = 0; i < samples_last_interval; ++i) {
            interval_samples.peek_offset(&val, interval_samples.used() - 1 - i);
            interval_sum += val;
        }

        power_history.push((int16_t)(interval_sum / samples_last_interval));
        samples_per_interval = samples_last_interval;
        samples_last_interval = 0;
    }, 1000 * 60 * HISTORY_MINUTE_INTERVAL, 1000 * 60 * HISTORY_MINUTE_INTERVAL);

    task_scheduler.scheduleWithFixedDelay([this](){
        uint16_t len;
        float result[ALL_VALUES_COUNT] = {0};
        if (tf_warp_energy_manager_get_energy_meter_detailed_values(&energy_manager.device, result, &len) != TF_E_OK)
            return;

        for(int i = 0; i < ALL_VALUES_COUNT; ++i) {
            all_values.get(i)->updateFloat(result[i]);
        }
    }, 1000, 1000);

    initialized = true;

    if (update_module_initialized)
        modules.get("em_meter")->updateBool(true);
}

void EMMeter::setup()
{
    initialized = false;
    hardware_available = false;

    if (!energy_manager.initialized) {
        // If the Energy Manager is not initialized, we will never be able to reach the energy meter.
        return;
    }

    setupEM(false);
}

void EMMeter::register_urls()
{
    api.addState("meter/state", &state, {}, 1000);
    api.addState("meter/values", &values, {}, 1000);
    api.addState("meter/phases", &phases, {}, 1000);
    api.addState("meter/all_values", &all_values, {}, 1000);
    api.addState("meter/error_counters", &errors, {}, 1000);

    api.addCommand("meter/reset", &reset, {}, [this](){
        if(!initialized) {
            return;
        }

        tf_warp_energy_manager_reset_energy_meter_relative_energy(&energy_manager.device);
    }, true);

    server.on("/meter/history", HTTP_GET, [this](WebServerRequest request) {
        if (!initialized) {
            request.send(400, "text/html", "not initialized");
            return;
        }

        const size_t buf_size = RING_BUF_SIZE * 6 + 100;
        char buf[buf_size] = {0};
        size_t buf_written = 0;

        int16_t val;
        power_history.peek(&val);
        // Negative values are prefilled, because the ESP was booted less than 48 hours ago.
        if (val < 0)
            buf_written += snprintf(buf + buf_written, buf_size - buf_written, "%s", "[null");
        else
            buf_written += snprintf(buf + buf_written, buf_size - buf_written, "[%d", (int)val);

        for (int i = 1; i < power_history.used() && power_history.peek_offset(&val, i) && buf_written < buf_size; ++i) {
            // Negative values are prefilled, because the ESP was booted less than 48 hours ago.
            if (val < 0)
                buf_written += snprintf(buf + buf_written, buf_size - buf_written, "%s", ",null");
            else
                buf_written += snprintf(buf + buf_written, buf_size - buf_written, ",%d", (int)val);
        }

        if (buf_written < buf_size)
            buf_written += snprintf(buf + buf_written, buf_size - buf_written, "%c", ']');

        request.send(200, "application/json; charset=utf-8", buf, buf_written);
    });

    server.on("/meter/live", HTTP_GET, [this](WebServerRequest request) {
        if (!initialized) {
            request.send(400, "text/html", "not initialized");
            return;
        }

        const size_t buf_size = RING_BUF_SIZE * 6 + 100;
        char buf[buf_size] = {0};
        size_t buf_written = 0;

        int16_t val;
        interval_samples.peek(&val);
        float samples_per_second = 0;
        if (this->samples_per_interval > 0) {
            samples_per_second = ((float)this->samples_per_interval) / (60 * HISTORY_MINUTE_INTERVAL);
        } else {
            samples_per_second = (float)this->samples_last_interval / millis() * 1000;
        }
        buf_written += snprintf(buf + buf_written, buf_size - buf_written, "{\"samples_per_second\":%f,\"samples\":[%d", samples_per_second, val);

        for (int i = 1; (i < interval_samples.used() - 1) && interval_samples.peek_offset(&val, i) && buf_written < buf_size; ++i) {
            buf_written += snprintf(buf + buf_written, buf_size - buf_written, ",%d", val);
        }
        if (buf_written < buf_size)
            buf_written += snprintf(buf + buf_written, buf_size - buf_written, "%s", "]}");
        request.send(200, "application/json; charset=utf-8", buf, buf_written);
    });
}

void EMMeter::loop()
{
}
