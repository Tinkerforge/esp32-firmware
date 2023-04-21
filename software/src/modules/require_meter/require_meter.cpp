/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "require_meter.h"
#include "modules.h"

void RequireMeter::start_task() {
    static bool is_running = false;
    if (is_running)
        return;

    meter.last_value_change = 0;
    task_scheduler.scheduleWithFixedDelay([]() {
        int64_t now = esp_timer_get_time();


        bool meter_timeout = (now > METER_BOOTUP_ENERGY_TIMEOUT_US) && isnan(meter.values.get("energy_abs")->asFloat());

        meter_timeout |= (now - meter.last_value_change > METER_TIMEOUT_US);

        #if MODULE_EVSE_V2_AVAILABLE()
            evse_v2.set_require_meter_blocking(meter_timeout);
        #elif MODULE_EVSE_AVAILABLE()
            evse.set_require_meter_blocking(meter_timeout);
        #endif

        if (meter_timeout)
            users.stop_charging(0, true, 0);

    }, 0, 1000);
    is_running = true;
}

void RequireMeter::pre_setup() {
    config = Config::Object({
        {"config", Config::Uint8(WARP_SMART)}
    });
}

void RequireMeter::set_require_meter_enabled(bool enabled) {
    #if MODULE_EVSE_V2_AVAILABLE()
        evse_v2.set_require_meter_enabled(enabled);
    #elif MODULE_EVSE_AVAILABLE()
        evse.set_require_meter_enabled(enabled);
    #endif
}

void RequireMeter::set_require_meter_blocking(bool blocking) {
    #if MODULE_EVSE_V2_AVAILABLE()
        evse_v2.set_require_meter_blocking(blocking);
    #elif MODULE_EVSE_AVAILABLE()
        evse.set_require_meter_blocking(blocking);
    #endif
}

bool RequireMeter::get_require_meter_blocking() {
    #if MODULE_EVSE_V2_AVAILABLE()
        return evse_v2.get_require_meter_blocking();
    #elif MODULE_EVSE_AVAILABLE()
        return evse.get_require_meter_blocking();
    #endif
}

bool RequireMeter::get_require_meter_enabled() {
    #if MODULE_EVSE_V2_AVAILABLE()
        return evse_v2.get_require_meter_enabled();
    #elif MODULE_EVSE_AVAILABLE()
        return evse.get_require_meter_enabled();
    #endif
}

bool RequireMeter::allow_charging(float meter_value) {
    if (get_require_meter_enabled() && (isnan(meter_value) || get_require_meter_blocking())) {
        set_require_meter_blocking(true);
        return false;
    }
    return true;
}

void RequireMeter::meter_found() {
    if (config.get("config")->asUint() == WARP_SMART) {
        config.get("config")->updateUint(WARP_PRO_ENABLED);
        api.writeConfig("require_meter/config", &config);
        set_require_meter_enabled(true);
        start_task();
    }
}

void RequireMeter::setup() {
    api.restorePersistentConfig("require_meter/config", &config);

    if (config.get("config")->asUint() == WARP_PRO_ENABLED) {
        set_require_meter_enabled(true);
        start_task();
    } else
        set_require_meter_enabled(false);
    initialized = true;
}

void RequireMeter::register_urls() {
    api.addPersistentConfig("require_meter/config", &config, {}, 1000);
}