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

#pragma once

#include "module.h"
#include "config.h"
#include "modules/meters/meter_value_availability.h"
#include "modules/power_manager/phase_switcher_back-end.h"
#include "modules/debug_protocol/debug_protocol_backend.h"
#include "tools.h"
#include "module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

#define CHARGING_SLOT_COUNT 15
#define CHARGING_SLOT_COUNT_SUPPORTED_BY_EVSE 20

#define CHARGING_SLOT_INCOMING_CABLE 0
#define CHARGING_SLOT_OUTGOING_CABLE 1
#define CHARGING_SLOT_SHUTDOWN_INPUT 2
#define CHARGING_SLOT_GP_INPUT 3
#define CHARGING_SLOT_AUTOSTART_BUTTON 4
#define CHARGING_SLOT_GLOBAL 5
#define CHARGING_SLOT_USER 6
#define CHARGING_SLOT_CHARGE_MANAGER 7
#define CHARGING_SLOT_EXTERNAL 8
#define CHARGING_SLOT_MODBUS_TCP 9
#define CHARGING_SLOT_MODBUS_TCP_ENABLE 10
#define CHARGING_SLOT_OCPP 11
#define CHARGING_SLOT_CHARGE_LIMITS 12
#define CHARGING_SLOT_REQUIRE_METER 13
#define CHARGING_SLOT_AUTOMATION 14

#define IEC_STATE_A 0
#define IEC_STATE_B 1
#define IEC_STATE_C 2
#define IEC_STATE_D 3
#define IEC_STATE_EF 4

#define CHARGER_STATE_NOT_PLUGGED_IN 0
#define CHARGER_STATE_WAITING_FOR_RELEASE 1
#define CHARGER_STATE_READY_TO_CHARGE 2
#define CHARGER_STATE_CHARGING 3
#define CHARGER_STATE_ERROR 4

#define DATA_STORE_PAGE_CHARGE_TRACKER 0
#define DATA_STORE_PAGE_RECOVERY 15

#define SLOT_ACTIVE(x) ((bool)(x & 0x01))
#define SLOT_CLEAR_ON_DISCONNECT(x) ((bool)(x & 0x02))

#define EXTERNAL_TIMEOUT 30

class IEvseBackend : public PhaseSwitcherBackend, public IDebugProtocolBackend
{
    friend class EvseCommon;

protected:
    IEvseBackend() {}
    virtual ~IEvseBackend() {}

    virtual bool is_initialized() = 0;
    virtual void set_initialized(bool initialized) = 0;

    virtual void post_setup() = 0;
    virtual void post_register_urls() = 0;

    // Pass through to DeviceModule if used
    virtual bool setup_device() = 0;
    virtual bool is_in_bootloader(int rc) = 0;

    // Pass through to bindings functions
    virtual void factory_reset() = 0;
    virtual void reset() = 0;

    virtual void set_data_storage(uint8_t, const uint8_t *) = 0;
    virtual void get_data_storage(uint8_t, uint8_t *) = 0;

    virtual void set_indicator_led(int16_t, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t *) = 0;

    virtual void set_control_pilot_disconnect(bool, bool *) = 0;
    virtual bool get_control_pilot_disconnect() = 0;

    virtual void set_boost_mode(bool enabled) = 0;

    virtual int get_charging_slot(uint8_t, uint16_t *, bool *, bool *) = 0;
    virtual int set_charging_slot(uint8_t, uint16_t, bool, bool) = 0;
    virtual void set_charging_slot_max_current(uint8_t slot, uint16_t current) = 0;
    virtual void set_charging_slot_clear_on_disconnect(uint8_t slot, bool clear_on_disconnect) = 0;
    virtual void set_charging_slot_active(uint8_t slot, bool enabled) = 0;
    virtual int get_charging_slot_default(uint8_t slot, uint16_t *ret_max_current, bool *ret_enabled, bool *ret_clear_on_disconnect) = 0;
    virtual int set_charging_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear_on_disconnect) = 0;
    // End: Pass through to bindings functions

    virtual void update_all_data() = 0;
};

class EvseCommon final : public IModule
#if MODULE_AUTOMATION_AVAILABLE()
                       , public IAutomationBackend
#endif
{
    // TODO: It's a bit ugly that we have to declare all specific EVSE modules as friends here.
    // But this allows us to make the configs private, to enforce all access happens via the public methods below.
    friend class EVSE;
    friend class EVSEV2;

public: // We need to access the backend in cm_networking for the phase switch. FIXME: Let EvseCommon implement PhaseSwitcherBackend instead.
    IEvseBackend *backend = nullptr;
private:
    unsigned long last_external_update = 0;

public:
    EvseCommon();

    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void setup_evse();

    void set_managed_current(uint16_t);

    void set_user_current(uint16_t);

    void set_modbus_current(uint16_t);
    void set_modbus_enabled(bool);

    uint32_t get_charger_meter();
    MeterValueAvailability get_charger_meter_power(float *power, micros_t max_age = 0_us);
    MeterValueAvailability get_charger_meter_energy(float *energy, micros_t max_age = 0_us);
    bool get_use_imexsum();

    void set_require_meter_blocking(bool);
    void set_require_meter_enabled(bool);
    bool get_require_meter_blocking();
    bool get_require_meter_enabled();

    void set_charge_limits_slot(uint16_t, bool);

    void set_ocpp_current(uint16_t);
    uint16_t get_ocpp_current();

    void factory_reset();
    void reset();

    void set_data_storage(uint8_t, const uint8_t *);
    void get_data_storage(uint8_t, uint8_t *);
    void set_indicator_led(int16_t, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t *);

    bool apply_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear);
    void apply_defaults();

    ConfigRoot &get_state();
    ConfigRoot &get_slots();
    ConfigRoot &get_low_level_state();
    bool get_management_enabled();
    uint32_t get_evse_version();

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

    uint32_t last_current_update = 0;
    bool shutdown_logged = false;

private:
    ConfigRoot low_level_state;
    ConfigRoot management_enabled;
    ConfigRoot management_enabled_update;
    ConfigRoot state;
    Config     slots_prototype;
    ConfigRoot slots;
    ConfigRoot hardware_configuration;
    ConfigRoot indicator_led;
    ConfigRoot button_state;
    ConfigRoot external_current;
    ConfigRoot external_current_update;
    ConfigRoot external_clear_on_disconnect;
    ConfigRoot external_clear_on_disconnect_update;
    ConfigRoot management_current;
    ConfigRoot management_current_update;
    ConfigRoot boost_mode;
    ConfigRoot boost_mode_update;
    ConfigRoot auto_start_charging;
    ConfigRoot auto_start_charging_update;
    ConfigRoot global_current;
    ConfigRoot global_current_update;
    ConfigRoot user_current;
    ConfigRoot user_enabled;
    ConfigRoot user_enabled_update;
    ConfigRoot external_enabled;
    ConfigRoot external_enabled_update;
    ConfigRoot external_defaults;
    ConfigRoot external_defaults_update;
    ConfigRoot modbus_enabled;
    ConfigRoot modbus_enabled_update;
    ConfigRoot ocpp_enabled;
    ConfigRoot ocpp_enabled_update;
    ConfigRoot require_meter_enabled;
    ConfigRoot require_meter_enabled_update;
    ConfigRoot automation_current;
    ConfigRoot automation_current_update;

    // Stored on ESP
    ConfigRoot meter_config;
    uint32_t charger_meter_slot = 0;

    micros_t next_cm_send_deadline = 0_us;
    void send_cm_client_update();

    bool use_imexsum = false;
};

#include "module_available_end.h"
