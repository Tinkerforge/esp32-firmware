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

#include "evse_common.h"
#include "module_dependencies.h"

extern uint32_t local_uid_num;

EvseCommon::EvseCommon() {
#if MODULE_EVSE_AVAILABLE()
    backend = &evse;
#elif MODULE_EVSE_V2_AVAILABLE()
    backend = &evse_v2;
#else
    #error Unknown or missing EVSE module!
#endif
}

void EvseCommon::pre_setup() {
    button_state = Config::Object({
        {"button_press_time", Config::Uint32(0)},
        {"button_release_time", Config::Uint32(0)},
        {"button_pressed", Config::Bool(false)},
    });

    Config *evse_charging_slot = new Config{Config::Object({
        {"max_current", Config::Uint16(0)},
        {"active", Config::Bool(false)},
        {"clear_on_disconnect", Config::Bool(false)}
    })};

    slots = Config::Array({},
        evse_charging_slot,
        CHARGING_SLOT_COUNT, CHARGING_SLOT_COUNT,
        Config::type_id<Config::ConfObject>());

    for (int i = 0; i < CHARGING_SLOT_COUNT; ++i)
        slots.add();

    indicator_led = Config::Object({
        {"indication", Config::Int16(0)},
        {"duration", Config::Uint16(0)},
    });

    auto_start_charging = Config::Object({
        {"auto_start_charging", Config::Bool(true)}
    });

    auto_start_charging_update = Config::Object({
        {"auto_start_charging", Config::Bool(true)}
    });

    global_current = Config::Object({
        {"current", Config::Uint16(32000)}
    });

    global_current_update = global_current;

    management_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    management_enabled_update = management_enabled;

    user_current = Config::Object({
        {"current", Config::Uint16(32000)}
    });

    user_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    user_enabled_update = user_enabled;

    external_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    external_enabled_update = external_enabled;

    external_defaults = Config::Object({
        {"current", Config::Uint16(0)},
        {"clear_on_disconnect", Config::Bool(false)},
    });
    external_defaults_update = external_defaults;

    management_current = Config::Object({
        {"current", Config::Uint16(32000)}
    });

    management_current_update = management_current;

    external_current = Config::Object({
        {"current", Config::Uint16(32000)}
    });

    external_current_update = external_current;

    external_clear_on_disconnect = Config::Object({
        {"clear_on_disconnect", Config::Bool(false)}
    });

    external_clear_on_disconnect_update = external_clear_on_disconnect;

    modbus_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    modbus_enabled_update = modbus_enabled;

    ocpp_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    ocpp_enabled_update = ocpp_enabled;

    boost_mode = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    boost_mode_update = boost_mode;

    require_meter_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });

    require_meter_enabled_update = require_meter_enabled;
}

bool EvseCommon::apply_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear)
{
    uint16_t old_current;
    bool old_enabled;
    bool old_clear;
    int rc = backend->get_charging_slot_default(slot, &old_current, &old_enabled, &old_clear);
    if (rc != TF_E_OK) {
        backend->is_in_bootloader(rc);
        logger.printfln("Failed to apply slot default (read failed). rc %d", rc);
        return false;
    }

    if ((old_current == current) && (old_enabled == enabled) && (old_clear == clear))
        return false;

    rc = backend->set_charging_slot_default(slot, current, enabled, clear);
    if (rc != TF_E_OK) {
        backend->is_in_bootloader(rc);
        logger.printfln("Failed to apply slot default (write failed). rc %d", rc);
        return false;
    }
    return true;
}

void EvseCommon::apply_defaults()
{
    // Maybe this is the first start-up after updating the EVSE to firmware 2.1.0 (or higher)
    // (Or the first start-up at all)
    // Make sure, that the charging slot defaults are the expected ones.

    // Slot 0 to 3 are readonly

    // Slot 4 (auto start): Enabled is read only, current and clear depend on the auto start setting
    // Doing anything with this slot should be unnecessary, as we would only change the current to 32000 or 0
    // depending on clear_on_disconnect. Any other value could be considered a bug in the EVSE firmware.

    // Slot 5 (global) has to be always enabled and never cleared. current is set per API
    // The charger API ensures that the slot's current and its default are always the same,
    // but if for any reason, they are not equal when booting up, just set the current value as
    // default. In the common case this has no effect. Also set enabled and clear in case this is
    // the first start-up.
    uint16_t global_current;
    bool global_active;
    int rc = backend->get_charging_slot(CHARGING_SLOT_GLOBAL, &global_current, &global_active, nullptr);
    if (rc != TF_E_OK) {
        backend->is_in_bootloader(rc);
        logger.printfln("Failed to apply defaults (global read failed). rc %d", rc);
        return;
    }
    // If this is the first start-up, this slot will not be active.
    // In the old firmwares, the global current was not persistant
    // so setting it to 32000 is expected after start-up.
    if (!global_active)
        global_current = 32000;

    if (this->apply_slot_default(CHARGING_SLOT_GLOBAL, global_current, true, false))
        backend->set_charging_slot(CHARGING_SLOT_GLOBAL, global_current, true, false);

    // Slot 6 (user) depends on user config.
    // It can be enabled per API (is stored in the EVSEs flash, not ours).
    // Set clear to true and current to 0 in any case: If disabled those are ignored anyway.
    bool user_enabled;
    rc = backend->get_charging_slot(CHARGING_SLOT_USER, nullptr, &user_enabled, nullptr);
    if (rc != TF_E_OK) {
        backend->is_in_bootloader(rc);
        logger.printfln("Failed to apply defaults (cm read failed). rc %d", rc);
        return;
    }
    if (this->apply_slot_default(CHARGING_SLOT_USER, 0, user_enabled, true))
        backend->set_charging_slot(CHARGING_SLOT_USER, 0, user_enabled, true);

    // Slot 7 (charge manager) can be enabled per API (is stored in the EVSEs flash, not ours).
    // Set clear to true and current to 0 in any case: If disabled those are ignored anyway.
    bool cm_enabled;
    rc = backend->get_charging_slot(CHARGING_SLOT_CHARGE_MANAGER, nullptr, &cm_enabled, nullptr);
    if (rc != TF_E_OK) {
        backend->is_in_bootloader(rc);
        logger.printfln("Failed to apply defaults (cm read failed). rc %d", rc);
        return;
    }
    if (this->apply_slot_default(CHARGING_SLOT_CHARGE_MANAGER, 0, cm_enabled, true))
        backend->set_charging_slot(CHARGING_SLOT_CHARGE_MANAGER, 0, cm_enabled, true);

    // Slot 8 (external) is controlled via API, no need to change anything here

    // Disabling all unused charging slots.
    for (int i = CHARGING_SLOT_COUNT; i < CHARGING_SLOT_COUNT_SUPPORTED_BY_EVSE; i++) {
        bool active;
        backend->is_in_bootloader(backend->get_charging_slot_default(i, NULL, &active, NULL));
        if (active)
            backend->is_in_bootloader(backend->set_charging_slot_default(i, 32000, false, false));
        backend->set_charging_slot_active(i, false);
    }
}


void EvseCommon::setup() {
    setup_evse();

    if (!backend->initialized)
        return;

    // Get all data once before announcing the EVSE feature.
    backend->update_all_data();
    api.addFeature("evse");
    task_scheduler.scheduleWithFixedDelay([this](){
        backend->update_all_data();
    }, 0, 250);

    backend->post_setup();
    initialized = true;
}

void EvseCommon::setup_evse()
{
    if (!backend->setup_device()) {
        return;
    }

    this->apply_defaults();
    backend->initialized = true;
}

void EvseCommon::register_urls() {
#if MODULE_CM_NETWORKING_AVAILABLE()
    cm_networking.register_client([this](uint16_t current, bool cp_disconnect_requested) {
        set_managed_current(current);

        backend->set_control_pilot_disconnect(cp_disconnect_requested, nullptr);
    });

    task_scheduler.scheduleWithFixedDelay([this](){
        uint16_t supported_current = 32000;
        for (int i = 0; i < CHARGING_SLOT_COUNT; ++i) {
            if (i == CHARGING_SLOT_CHARGE_MANAGER)
                continue;
            if (!slots.get(i)->get("active")->asBool())
                continue;
            supported_current = min(supported_current, (uint16_t)slots.get(i)->get("max_current")->asUint());
        }

        cm_networking.send_client_update(
            local_uid_num,
            state.get("iec61851_state")->asUint(),
            state.get("charger_state")->asUint(),
            low_level_state.get("time_since_state_change")->asUint(),
            state.get("error_state")->asUint(),
            low_level_state.get("uptime")->asUint(),
            low_level_state.get("charging_time")->asUint(),
            slots.get(CHARGING_SLOT_CHARGE_MANAGER)->get("max_current")->asUint(),
            supported_current,
            management_enabled.get("enabled")->asBool(),
            backend->get_control_pilot_disconnect()
        );
    }, 1000, 1000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (!deadline_elapsed(last_current_update + 30000))
            return;
        if (!management_enabled.get("enabled")->asBool()) {
            // Push back the next check for 30 seconds: If managed gets enabled,
            // we want to wait 30 seconds before setting the current for the first time.
            last_current_update = millis();
            return;
        }
        if(!shutdown_logged)
            logger.printfln("Got no managed current update for more than 30 seconds. Setting managed current to 0");
        shutdown_logged = true;
        backend->set_charging_slot_max_current(CHARGING_SLOT_CHARGE_MANAGER, 0);
    }, 1000, 1000);
#endif

    // States
    api.addState("evse/state", &state, {}, 1000);
    api.addState("evse/hardware_configuration", &hardware_configuration, {}, 1000);
    api.addState("evse/low_level_state", &low_level_state, {}, 1000);
    api.addState("evse/button_state", &button_state, {}, 250);
    api.addState("evse/slots", &slots, {}, 1000);
    api.addState("evse/indicator_led", &indicator_led, {}, 1000);

    //Actions
    api.addCommand("evse/stop_charging", Config::Null(), {}, [this](){
        if (state.get("iec61851_state")->asUint() != IEC_STATE_A)
            backend->set_charging_slot_max_current(CHARGING_SLOT_AUTOSTART_BUTTON, 0);
    }, true);

    api.addCommand("evse/start_charging", Config::Null(), {}, [this](){
        if (state.get("iec61851_state")->asUint() != IEC_STATE_A)
            backend->set_charging_slot_max_current(CHARGING_SLOT_AUTOSTART_BUTTON, 32000);
    }, true);

#if MODULE_WS_AVAILABLE()
    server.on("/evse/start_debug", HTTP_GET, [this](WebServerRequest request) {
        task_scheduler.scheduleOnce([this](){
            last_debug_keep_alive = millis();
            check_debug();
            ws.pushRawStateUpdate(backend->get_evse_debug_header(), "evse/debug_header");
            debug = true;
        }, 0);
        return request.send(200);
    });

    server.on("/evse/continue_debug", HTTP_GET, [this](WebServerRequest request) {
        last_debug_keep_alive = millis();
        return request.send(200);
    });

    server.on("/evse/stop_debug", HTTP_GET, [this](WebServerRequest request){
        task_scheduler.scheduleOnce([this](){
            debug = false;
        }, 0);
        return request.send(200);
    });
#endif

    api.addState("evse/external_current", &external_current, {}, 1000);
    api.addCommand("evse/external_current_update", &external_current_update, {}, [this](){
        backend->set_charging_slot_max_current(CHARGING_SLOT_EXTERNAL, external_current_update.get("current")->asUint());
    }, false);

    api.addState("evse/external_clear_on_disconnect", &external_clear_on_disconnect, {}, 1000);
    api.addCommand("evse/external_clear_on_disconnect_update", &external_clear_on_disconnect_update, {}, [this](){
        backend->set_charging_slot_clear_on_disconnect(CHARGING_SLOT_EXTERNAL, external_clear_on_disconnect_update.get("clear_on_disconnect")->asBool());
    }, false);

    api.addState("evse/management_current", &management_current, {}, 1000);
    api.addCommand("evse/management_current_update", &management_current_update, {}, [this](){
        set_managed_current(management_current_update.get("current")->asUint());
    }, false);

    api.addState("evse/boost_mode", &boost_mode, {}, 1000);
    api.addCommand("evse/boost_mode_update", &boost_mode_update, {}, [this](){
        backend->set_boost_mode(boost_mode_update.get("enabled")->asBool());
    }, true);

    api.addState("evse/auto_start_charging", &auto_start_charging, {}, 1000);
    api.addCommand("evse/auto_start_charging_update", &auto_start_charging_update, {}, [this](){
        // 1. set auto start
        // 2. make persistent
        // 3. fake a start/stop charging

        bool enable_auto_start = auto_start_charging_update.get("auto_start_charging")->asBool();

        backend->set_charging_slot_clear_on_disconnect(CHARGING_SLOT_AUTOSTART_BUTTON, !enable_auto_start);

        if (enable_auto_start) {
            apply_slot_default(CHARGING_SLOT_AUTOSTART_BUTTON, 32000, true, false);
        } else {
            apply_slot_default(CHARGING_SLOT_AUTOSTART_BUTTON, 0, true, true);
        }

        if (enable_auto_start) {
            backend->set_charging_slot_max_current(CHARGING_SLOT_AUTOSTART_BUTTON, 32000);
        } else {
            // Only "stop" charging if no car is currently plugged in.
            // Clear on disconnect only triggers once, so we have to zero the current manually here.
            uint8_t iec_state = state.get("iec61851_state")->asUint();
            if (iec_state != 2 && iec_state != 3)
                backend->set_charging_slot_max_current(CHARGING_SLOT_AUTOSTART_BUTTON, 0);
        }
    }, false);

    api.addState("evse/global_current", &global_current, {}, 1000);
    api.addCommand("evse/global_current_update", &global_current_update, {}, [this](){
        uint16_t current = global_current_update.get("current")->asUint();
        backend->set_charging_slot_max_current(CHARGING_SLOT_GLOBAL, current);
        apply_slot_default(CHARGING_SLOT_GLOBAL, current, true, false);
    }, false);

    api.addState("evse/management_enabled", &management_enabled, {}, 1000);
    api.addCommand("evse/management_enabled_update", &management_enabled_update, {}, [this](){
        bool enabled = management_enabled_update.get("enabled")->asBool();

        if (enabled == management_enabled.get("enabled")->asBool())
            return;

        if (enabled)
            backend->set_charging_slot(CHARGING_SLOT_CHARGE_MANAGER, 0, true, true);
        else
            backend->set_charging_slot(CHARGING_SLOT_CHARGE_MANAGER, 32000, false, false);

        if (enabled)
            apply_slot_default(CHARGING_SLOT_CHARGE_MANAGER, 0, true, true);
        else
            apply_slot_default(CHARGING_SLOT_CHARGE_MANAGER, 32000, false, false);
    }, false);

    api.addState("evse/user_current", &user_current, {}, 1000);
    api.addState("evse/user_enabled", &user_enabled, {}, 1000);
    api.addCommand("evse/user_enabled_update", &user_enabled_update, {}, [this](){
        bool enabled = user_enabled_update.get("enabled")->asBool();

        if (enabled == user_enabled.get("enabled")->asBool())
            return;

#if MODULE_USERS_AVAILABLE()
        if (enabled) {
            users.stop_charging(0, true);
        }
#endif

        if (enabled)
            backend->set_charging_slot(CHARGING_SLOT_USER, 0, true, true);
        else
            backend->set_charging_slot(CHARGING_SLOT_USER, 32000, false, false);

        if (enabled)
            apply_slot_default(CHARGING_SLOT_USER, 0, true, true);
        else
            apply_slot_default(CHARGING_SLOT_USER, 32000, false, false);
    }, false);

    api.addState("evse/external_enabled", &external_enabled, {}, 1000);
    api.addCommand("evse/external_enabled_update", &external_enabled_update, {}, [this](){
        bool enabled = external_enabled_update.get("enabled")->asBool();

        if (enabled == external_enabled.get("enabled")->asBool())
            return;

        backend->set_charging_slot_active(CHARGING_SLOT_EXTERNAL, enabled);
        apply_slot_default(CHARGING_SLOT_EXTERNAL, 32000, enabled, false);
    }, false);

    api.addState("evse/external_defaults", &external_defaults, {}, 1000);
    api.addCommand("evse/external_defaults_update", &external_defaults_update, {}, [this](){
        bool enabled;
        backend->get_charging_slot_default(CHARGING_SLOT_EXTERNAL, nullptr, &enabled, nullptr);
        apply_slot_default(CHARGING_SLOT_EXTERNAL, external_defaults_update.get("current")->asUint(), enabled, external_defaults_update.get("clear_on_disconnect")->asBool());
    }, false);

    api.addState("evse/modbus_tcp_enabled", &modbus_enabled, {}, 1000);
    api.addCommand("evse/modbus_tcp_enabled_update", &modbus_enabled_update, {}, [this](){
        bool enabled = modbus_enabled_update.get("enabled")->asBool();

        if (enabled == modbus_enabled.get("enabled")->asBool())
            return;

        if (enabled) {
            backend->set_charging_slot(CHARGING_SLOT_MODBUS_TCP, 32000, true, false);
            apply_slot_default(CHARGING_SLOT_MODBUS_TCP, 32000, true, false);


            backend->set_charging_slot(CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, true, false);
            apply_slot_default(CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, true, false);
        }
        else {
            backend->set_charging_slot(CHARGING_SLOT_MODBUS_TCP, 32000, false, false);
            apply_slot_default(CHARGING_SLOT_MODBUS_TCP, 32000, false, false);

            backend->set_charging_slot(CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, false, false);
            apply_slot_default(CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, false, false);
        }
    }, false);

    api.addState("evse/ocpp_enabled", &ocpp_enabled, {}, 1000);
    api.addCommand("evse/ocpp_enabled_update", &ocpp_enabled_update, {}, [this](){
        bool enabled = ocpp_enabled_update.get("enabled")->asBool();

        if (enabled == ocpp_enabled.get("enabled")->asBool())
            return;

        if (enabled) {
            backend->set_charging_slot(CHARGING_SLOT_OCPP, 32000, true, false);
            apply_slot_default(CHARGING_SLOT_OCPP, 32000, true, false);
        }
        else {
            backend->set_charging_slot(CHARGING_SLOT_OCPP, 32000, false, false);
            apply_slot_default(CHARGING_SLOT_OCPP, 32000, false, false);
        }
    }, false);

    backend->post_register_urls();
}

void EvseCommon::loop() {
#if MODULE_WS_AVAILABLE()
    static uint32_t last_debug = 0;
    if (debug && deadline_elapsed(last_debug + 50)) {
        last_debug = millis();
        ws.pushRawStateUpdate(backend->get_evse_debug_line(), "evse/debug");
    }
#endif
}

void EvseCommon::set_managed_current(uint16_t current) {
    backend->set_charging_slot_max_current(CHARGING_SLOT_CHARGE_MANAGER, current);
    last_current_update = millis();
    shutdown_logged = false;
}

void EvseCommon::set_user_current(uint16_t current) {
    backend->set_charging_slot_max_current(CHARGING_SLOT_USER, current);
}

void EvseCommon::set_modbus_current(uint16_t current) {
    backend->set_charging_slot_max_current(CHARGING_SLOT_MODBUS_TCP, current);
}

void EvseCommon::set_modbus_enabled(bool enabled) {
    backend->set_charging_slot_max_current(CHARGING_SLOT_MODBUS_TCP_ENABLE, enabled ? 32000 : 0);
}

void EvseCommon::set_require_meter_blocking(bool blocking) {
    backend->set_charging_slot_max_current(CHARGING_SLOT_REQUIRE_METER, blocking ? 0 : 32000);
}

void EvseCommon::set_require_meter_enabled(bool enabled) {
    if (!initialized)
        return;

    apply_slot_default(CHARGING_SLOT_REQUIRE_METER, 0, enabled, false);
    backend->set_charging_slot_active(CHARGING_SLOT_REQUIRE_METER, enabled);
}

bool EvseCommon::get_require_meter_blocking() {
    uint16_t current = 0;
    bool enabled = get_require_meter_enabled();
    if (!enabled)
        return false;

    backend->get_charging_slot(CHARGING_SLOT_REQUIRE_METER, &current, &enabled, nullptr);
    return enabled && current == 0;
}

bool EvseCommon::get_require_meter_enabled() {
    return require_meter_enabled.get("enabled")->asBool();
}

void EvseCommon::set_charge_limits_slot(uint16_t current, bool enabled) {
    backend->set_charging_slot(CHARGING_SLOT_CHARGE_LIMITS, current, enabled, false);
}

void EvseCommon::set_ocpp_current(uint16_t current) {
    backend->set_charging_slot_max_current(CHARGING_SLOT_OCPP, current);
}

uint16_t EvseCommon::get_ocpp_current() {
    return slots.get(CHARGING_SLOT_OCPP)->get("max_current")->asUint();
}

void EvseCommon::factory_reset() {
    backend->factory_reset();
}

void EvseCommon::reset() {
    backend->reset();
}

void EvseCommon::set_data_storage(uint8_t page, const uint8_t* data) {
    backend->set_data_storage(page, data);
}

void EvseCommon::get_data_storage(uint8_t page, uint8_t* data) {
    backend->get_data_storage(page, data);
}

void EvseCommon::set_indicator_led(int16_t indication, uint16_t duration, uint8_t *ret_status) {
    backend->set_indicator_led(indication, duration, ret_status);
}

ConfigRoot &EvseCommon::get_slots() {
    return slots;
}

ConfigRoot &EvseCommon::get_low_level_state() {
    return low_level_state;
}

ConfigRoot &EvseCommon::get_state() {
    return state;
}

bool EvseCommon::get_management_enabled() {
    return management_enabled.get("enabled")->asBool();
}

void EvseCommon::check_debug()
{
    task_scheduler.scheduleOnce([this](){
        if (deadline_elapsed(last_debug_keep_alive + 60000) && debug)
        {
            logger.printfln("Debug log creation canceled because no continue call was received for more than 60 seconds.");
            debug = false;
        }
        else if (debug)
            check_debug();
    }, 10000);
}
