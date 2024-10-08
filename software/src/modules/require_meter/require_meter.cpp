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

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "modules/meters/meter_value_availability.h"

#define METER_TIMEOUT micros_t{24ll * 60 * 60 * 1000 * 1000}
// #define METER_TIMEOUT micros_t{10 * 1000 * 1000}

#define METER_BOOTUP_GRACE_PERIOD micros_t{90ll * 1000 * 1000}

#define WARP_SMART 0
#define WARP_PRO_DISABLED 1
#define WARP_PRO_ENABLED 2

void RequireMeter::pre_setup()
{
    config = Config::Object({
        {"config", Config::Uint8(WARP_SMART)}
    });

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_trigger(
        AutomationTriggerID::RequireMeter,
        *Config::Null(),
        nullptr,
        false
    );
#endif
}

void RequireMeter::setup()
{
    api.restorePersistentConfig("require_meter/config", &config);

    if (config.get("config")->asUint() == WARP_PRO_ENABLED) {
        evse_common.set_require_meter_enabled(true);
        start_task();
    } else
        evse_common.set_require_meter_enabled(false);
    initialized = true;
}

void RequireMeter::register_urls()
{
    api.addPersistentConfig("require_meter/config", &config);
}

void RequireMeter::register_events()
{
    if (config.get("config")->asUint() == WARP_SMART) {
        // We've never seen an energy meter.
        // Listen to info/features in case a meter shows up.
        event.registerEvent("info/features", {}, [this](const Config * /*config*/) {
            if (!api.hasFeature("meter"))
                return EventResult::OK;

            logger.printfln("Seen energy meter for the first time (since factory reset). Will require up-to-date meter readings to start charging from now on.");
            config.get("config")->updateUint(WARP_PRO_ENABLED);
            api.writeConfig("require_meter/config", &config);
            evse_common.set_require_meter_enabled(true);
            start_task();
            return EventResult::Deregister;
        });
    }
}

#if MODULE_AUTOMATION_AVAILABLE()
bool RequireMeter::has_triggered(const Config *conf, void *data)
{
    switch (conf->getTag<AutomationTriggerID>()) {
        case AutomationTriggerID::RequireMeter:
            return true;

        default:
            return false;
    }
}
#endif

void RequireMeter::start_task()
{
    static bool is_running = false;
    if (is_running)
        return;

#if MODULE_AUTOMATION_AVAILABLE()
    automation.set_enabled(AutomationTriggerID::RequireMeter, true);
#endif

    task_scheduler.scheduleWithFixedDelay([this]() {
        bool meter_timeout = false;

        if (!meters.meter_has_value_changed(evse_common.get_charger_meter(), METER_TIMEOUT)) {
            // No value was _changed_ (i.e. was written to a _different_ value than it had before) for METER_TIMEOUT.
            // This can for example happen on a WARP2 when unplugging the meter from the EVSE.
            // The EVSE will then continue to report the last seen values until a reboot.
            meter_timeout = true;
        }

        float energy;
        MeterValueAvailability value_availability = evse_common.get_charger_meter_energy(&energy, METER_TIMEOUT);

        switch (value_availability) {
            case MeterValueAvailability::Fresh:
                // Value is considered fresh. Nothing to do here.
                // If the value is written successfully,
                // but did not change for METER_TIMEOUT
                // meter_timeout was already set above.
                // Note that this assumes that either all measurands
                // hang on their last value, or none do.
                // This is fine, because we want to detect a broken
                // communication link between the ESP and the meter.
                // Detecting a bugged meter firmware (i.e. one that
                // still reports some values but not the energy)
                // is out of scope.
                break;
            case MeterValueAvailability::Stale:
                // meter_timeout was probably already set to true above.
                // (Because "This value was changed" is stronger than "This value was written")
                // But set it again for good measure.
                meter_timeout = true;
                break;
            case MeterValueAvailability::CurrentlyUnknown:
                // Meter has never set the value and it is yet unknown whether
                // the meter supports the energy measurand.
                meter_timeout = true;
                break;
            case MeterValueAvailability::Unavailable:
                // Meter does not support the energy measurand.
                meter_timeout = true;
                break;
        }

        // We want to give all hardware some time to boot up,
        // but don't want to unblock if we were already blocked before the reboot
        // so only consider the meter timed out if
        // - the boot grace period is elapsed
        // - or the EVSE is already blocking (because we blocked it before a reboot)
        // - or the energy value is unavailable (because the used meter will never be able to report it).
        meter_timeout = meter_timeout && (
                            deadline_elapsed(METER_BOOTUP_GRACE_PERIOD)
                            || evse_common.get_require_meter_blocking()
                            || value_availability == MeterValueAvailability::Unavailable);

        evse_common.set_require_meter_blocking(meter_timeout);

#if MODULE_AUTOMATION_AVAILABLE()
        static bool was_triggered = false;
        if (meter_timeout) {
            if (!was_triggered) {
                automation.trigger(AutomationTriggerID::RequireMeter, nullptr, this);
                was_triggered = true;
            }
        } else {
            was_triggered = false;
        }
#endif

#if MODULE_USERS_AVAILABLE()
        if (meter_timeout)
            users.stop_charging(0, true, 0);
#endif

        static bool last_meter_timeout = false;
        if (meter_timeout != last_meter_timeout) {
            if (meter_timeout) {
                logger.printfln("Energy meter stuck or unreachable! Blocking charging.");
            } else {
                logger.printfln("Energy meter working again. Allowing charging.");
            }
        }
        last_meter_timeout = meter_timeout;

        // Delay first check to give the meter time to send initial values.
        // This might allow an immediate charge, just to abort it right away, but the EVSE
        // might start a charge by itself anyway before the ESP finished starting,
        // unless NFC or OCPP are enabled.
    }, 3_s, 1_s);
    is_running = true;
}

bool RequireMeter::allow_charging(float meter_value)
{
    if (evse_common.get_require_meter_enabled() && (isnan(meter_value) || evse_common.get_require_meter_blocking())) {
        evse_common.set_require_meter_blocking(true);
        return false;
    }
    return true;
}
