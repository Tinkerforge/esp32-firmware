/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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
#include "heating.h"

#include <time.h>
#include <type_traits>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "controlperiod.enum.h"


static constexpr auto HEATING_UPDATE_INTERVAL = 1_m;

#define HEATING_SG_READY_ACTIVE_CLOSED 0
#define HEATING_SG_READY_ACTIVE_OPEN   1

#define extended_logging(fmt, ...) \
    do { \
        if (extended_logging_active) { \
            logger.tracefln(this->trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__); \
        } \
    } while (0)

void Heating::pre_setup()
{
    this->trace_buffer_index = logger.alloc_trace_buffer("heating", 1 << 20);

    config = ConfigRoot{Config::Object({
        {"sg_ready_blocking_active_type", Config::Uint(0, 0, 1)},
        {"sg_ready_extended_active_type", Config::Uint(0, 0, 1)},
        {"minimum_holding_time", Config::Uint(15, 10, 60)},
        {"meter_slot_grid_power", Config::Uint(POWER_MANAGER_DEFAULT_METER_SLOT, 0, METERS_SLOTS - 1)},
        {"control_period", Config::Enum(ControlPeriod::Hours24, ControlPeriod::Hours24, ControlPeriod::Hours4)},
        {"extended_logging_active", Config::Bool(false)},
        {"yield_forecast_active", Config::Bool(false)},
        {"yield_forecast_threshold", Config::Uint(0)},
        {"extended_active", Config::Bool(false)},
        {"extended_hours", Config::Uint(4, 0, 24)},
        {"blocking_active", Config::Bool(false)},
        {"blocking_hours", Config::Uint(0, 0, 24)},
        {"pv_excess_control_active", Config::Bool(false)},
        {"pv_excess_control_threshold", Config::Uint(0)},
        {"p14enwg_active", Config::Bool(false)},
        {"p14enwg_input", Config::Uint(0, 0, 3)},
        {"p14enwg_active_type", Config::Uint(0, 0, 1)}
    }), [this](Config &update, ConfigSource source) -> String {
        task_scheduler.scheduleOnce([this]() {
            this->update();
        });
        return "";
    }};

    state = Config::Object({
        {"sg_ready_blocking_active", Config::Bool(false)},
        {"sg_ready_extended_active", Config::Bool(false)},
        {"p14enwg_active", Config::Bool(false)},
        {"remaining_holding_time", Config::Int(-1, -1, 60)}
    });
}

void Heating::setup()
{
    api.restorePersistentConfig("heating/config", &config);

    initialized = true;
}

void Heating::register_urls()
{
    api.addPersistentConfig("heating/config", &config);
    api.addState("heating/state",             &state);
    api.addCommand("heating/reset_holding_time", Config::Null(), {}, [this](String &/*errmsg*/) {
        this->last_sg_ready_change = 0;
        this->update();
    }, true);

    task_scheduler.scheduleWallClock([this]() {
        this->update();
    }, HEATING_UPDATE_INTERVAL, 0_ms, true);
}

bool Heating::is_active()
{
    const bool extended_active          = config.get("extended_active")->asBool();
    const bool blocking_active          = config.get("blocking_active")->asBool();
    const bool pv_excess_control_active = config.get("pv_excess_control_active")->asBool();
    const bool yield_forecast_active    = config.get("yield_forecast_active")->asBool();

    if(!yield_forecast_active && !extended_active && !blocking_active && !pv_excess_control_active) {
        return false;
    }

    return true;
}

bool Heating::is_p14enwg_active()
{
    return config.get("p14enwg_active")->asBool();
}

Heating::Status Heating::get_status()
{
    const bool p14enwg_active          = config.get("p14enwg_active")->asBool();
    const uint32_t p14enwg_active_type = config.get("p14enwg_active_type")->asUint();
    const uint32_t sg_ready0_type      = config.get("sg_ready_blocking_active_type")->asUint();
    const uint32_t sg_ready1_type      = config.get("sg_ready_extended_active_type")->asUint();
    const bool sg_ready_output_0       = em_v2.get_sg_ready_output(0);
    const bool sg_ready_output_1       = em_v2.get_sg_ready_output(1);
    const bool sg_ready0_on            = sg_ready_output_0 == (sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED);
    const bool sg_ready1_on            = sg_ready_output_1 == (sg_ready1_type == HEATING_SG_READY_ACTIVE_CLOSED);
    const bool p14_enwg_on             = p14enwg_active && (sg_ready_output_0 == (p14enwg_active_type == HEATING_SG_READY_ACTIVE_CLOSED));

    if(p14_enwg_on) {
        return Status::BlockingP14;
    } else if(sg_ready0_on) {
        return Status::Blocking;
    } else if(sg_ready1_on) {
        return Status::Extended;
    }

    return Status::Idle;
}

void Heating::update()
{
    const bool extended_logging_active = config.get("extended_logging_active")->asBool();
    // Only update if clock is synced. Heating control depends on time of day.
    struct timeval tv_now;
    if (!rtc.clock_synced(&tv_now)) {
        state.get("remaining_holding_time")->updateInt(-1);
        extended_logging("Clock not synced. Skipping update.");
        return;
    }

    // TODO: Does §14 EnWG need a smaller update interval or is it enough to check it every minute?
    const bool     p14enwg_active      = config.get("p14enwg_active")->asBool();
    const uint32_t p14enwg_input       = config.get("p14enwg_input")->asUint();
    const uint32_t p14enwg_active_type = config.get("p14enwg_active_type")->asUint();
    const uint32_t sg_ready0_type      = config.get("sg_ready_blocking_active_type")->asUint();
    const uint32_t sg_ready1_type      = config.get("sg_ready_extended_active_type")->asUint();

    // Check if §14 EnWG should be turned on
    bool p14enwg_on = false;
    if(p14enwg_active) {
        bool input_value = em_v2.get_input(p14enwg_input);

        if (p14enwg_active_type == 0) {
            p14enwg_on = input_value;
        } else {
            p14enwg_on = !input_value;
        }
    }

    // If p14enwg is triggered, we immediately set output 0 accordingly.
    // If it is not triggered it depends on the heating controller if it should be on or off.
    if (p14enwg_on) {
        state.get("p14enwg_active")->updateBool(true);
        extended_logging("§14 EnWG blocks heating. Turning on SG ready output 0 (%s).", sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
        em_v2.set_sg_ready_output(0, sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED);
    } else {
        state.get("p14enwg_active")->updateBool(false);
    }

    // Get values from config
    const uint8_t minimum_holding_time = config.get("minimum_holding_time")->asUint();
    const uint32_t minutes = rtc.timestamp_minutes();
    uint32_t remaining_holding_time = 0;
    if (minutes >= last_sg_ready_change) {
        uint32_t time_since_last_change = minutes - last_sg_ready_change;
        if(time_since_last_change < minimum_holding_time) {
            remaining_holding_time = minimum_holding_time - time_since_last_change;
        }
    }
    state.get("remaining_holding_time")->updateInt(remaining_holding_time);

    if (remaining_holding_time > 0) {
        extended_logging("Minimum control holding time not reached. Current time: %dmin, last change: %dmin, minimum holding time: %dmin.", minutes, last_sg_ready_change, minimum_holding_time);
        return;
    }

    const uint32_t meter_slot_grid_power       = config.get("meter_slot_grid_power")->asUint();
    const bool     yield_forecast_active       = config.get("yield_forecast_active")->asBool();
    const uint32_t yield_forecast_threshold    = config.get("yield_forecast_threshold")->asUint();
    const bool     extended_active             = config.get("extended_active")->asBool();
    const uint32_t extended_hours              = config.get("extended_hours")->asUint();
    const bool     blocking_active             = config.get("blocking_active")->asBool();
    const uint32_t blocking_hours              = config.get("blocking_hours")->asUint();
    const bool     pv_excess_control_active    = config.get("pv_excess_control_active")->asBool();
    const uint32_t pv_excess_control_threshold = config.get("pv_excess_control_threshold")->asUint();
    const ControlPeriod control_period         = config.get("control_period")->asEnum<ControlPeriod>();

    bool sg_ready0_on = false;
    bool sg_ready1_on = false;

    if(!yield_forecast_active && !extended_active && !blocking_active && !pv_excess_control_active) {
        extended_logging("No control active.");
    } else {
        const time_t now                      = time(NULL);
        const struct tm *current_time         = localtime(&now);
        const uint16_t minutes_since_midnight = current_time->tm_hour * 60 + current_time->tm_min;
        if (minutes_since_midnight >= 24*60) {
            extended_logging("Too many minutes since midnight: %d.", minutes_since_midnight);
            return;
        }

        // PV excess handling for winter and summer
        auto handle_pv_excess = [&] () {
            if (!pv_excess_control_active) {
                return;
            }
            float watt_current = 0;
            MeterValueAvailability meter_availability = meters.get_power(meter_slot_grid_power, &watt_current);
            if (meter_availability != MeterValueAvailability::Fresh) {
                extended_logging("Meter value not available (meter %d has availability %d). Ignoring PV excess control.", meter_slot_grid_power, static_cast<std::underlying_type<MeterValueAvailability>::type>(meter_availability));
            } else if ((-watt_current) > pv_excess_control_threshold) {
                extended_logging("Current PV excess is above threshold. Current PV excess: %dW, threshold: %dW.", (int)watt_current, pv_excess_control_threshold);
                sg_ready1_on |= true;
            }
        };

        // Dynamic price handling for winter and summer
        auto handle_dynamic_price = [&] (const bool handle_extended, const bool handle_blocking) {
            if (!extended_active && !blocking_active) {
                return;
            }

            auto print_hours_today = [&] (const char *name, const bool *hours, const uint8_t duration) {
                char buffer[duration*4 + 1] = {'\0'};
                for (int i = 0; i < duration*4; i++) {
                    buffer[i] = hours[i] ? '1' : '0';
                }
                extended_logging("%s: %s", name, buffer);
            };

            uint8_t duration = 0;
            switch(control_period) {
                case ControlPeriod::Hours24:
                    duration = 24;
                    break;
                case ControlPeriod::Hours12:
                    duration = 12;
                    break;
                case ControlPeriod::Hours8:
                    duration = 8;
                    break;
                case ControlPeriod::Hours6:
                    duration = 6;
                    break;
                case ControlPeriod::Hours4:
                    duration = 4;
                    break;
                default:
                    logger.printfln("Unknown control period %d", static_cast<std::underlying_type<ControlPeriod>::type>(control_period));
                    return;
            }
            bool data[duration*4] = {false};

            // start_time is the nearest time block with length duration and index for current time within the block
            const uint32_t duration_block = (minutes_since_midnight / (duration*60)) * (duration*60);
            time_t midnight;
            if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight))
                return;

            const uint32_t start_time     = midnight/60 + duration_block;
            const uint8_t current_index   = (minutes_since_midnight - duration_block)/15;

            if (handle_extended) {
                const bool data_available = day_ahead_prices.get_cheap_hours(start_time, duration, extended_hours, data);
                if (!data_available) {
                    extended_logging("Cheap hours not available. Ignoring extended control.");
                } else {
                    print_hours_today("Cheap hours", data, duration);
                    if (data[current_index]) {
                        extended_logging("Current time is in cheap hours.");
                        sg_ready1_on |= true;
                    } else {
                        extended_logging("Current time is not in cheap hours.");
                        sg_ready1_on |= false;
                    }
                }
            }
            if (handle_blocking) {
                const bool data_available = day_ahead_prices.get_expensive_hours(start_time, duration, blocking_hours, data);
                if (!data_available) {
                    extended_logging("Expensive hours not available. Ignoring blocking control.");
                } else {
                    print_hours_today("Expensive hours", data, duration);
                    if (data[current_index]) {
                        extended_logging("Current time is in expensive hours.");
                        sg_ready0_on |= true;
                    } else {
                        extended_logging("Current time is not in expensive hours.");
                        sg_ready0_on |= false;
                    }
                }
            }
        };

        auto is_expected_yield_high = [&] () {
            // we check the expected pv excess and unblock if it is below the threshold.
            if (!yield_forecast_active) {
                extended_logging("Yield forecast is inactive.");
                return false;
            }

            extended_logging("Yield forecast is active.");
            uint32_t wh_expected;

            if(!solar_forecast.get_wh_today().try_unwrap(&wh_expected)) {
                extended_logging("Expected PV yield not available. Ignoring yield forecast.");
            } else {
                if (wh_expected/1000 < yield_forecast_threshold) {
                    extended_logging("Expected PV yield %dkWh is below threshold of %dkWh.", wh_expected/1000, yield_forecast_threshold);
                } else {
                    extended_logging("Expected PV yield %dkWh is above or equal to threshold of %dkWh.", wh_expected/1000, yield_forecast_threshold);
                    return true;
                }
            }

            return false;
        };

        const bool pv_yield_high = is_expected_yield_high();
        if (pv_yield_high) {
            extended_logging("Day ahead prices are ignored for cheap hours because of expected PV yield.");
            handle_dynamic_price(false, blocking_active);
        } else {
            handle_dynamic_price(extended_active, blocking_active);
        }
        handle_pv_excess();
    }

    // If p14enwg is triggered, we never turn output 1 (extended operation) on.
    if (p14enwg_on) {
        sg_ready1_on = false;
    // If sg_ready0 and sg_ready1 are both to be turned on (e.g. high price but enough sun for heating with pv excess),
    // we turn off sg_ready0. E.g. we prefer extended operation instead of blocking operation in this case.
    } else if (sg_ready0_on && sg_ready1_on) {
        sg_ready0_on = false;
    }

    const bool sg_ready_output_1 = em_v2.get_sg_ready_output(1);
    if (sg_ready1_on) {
        state.get("sg_ready_extended_active")->updateBool(true);
        extended_logging("Heating decision: Turning on SG Ready output 1 (%s).", sg_ready1_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
        const bool new_value = sg_ready1_type == HEATING_SG_READY_ACTIVE_CLOSED;
        if (sg_ready_output_1 != new_value) {
            em_v2.set_sg_ready_output(1, new_value);
            last_sg_ready_change = rtc.timestamp_minutes();
            state.get("remaining_holding_time")->updateInt(config.get("minimum_holding_time")->asUint());
        }
    } else {
        state.get("sg_ready_extended_active")->updateBool(false);
        extended_logging("Heating decision: Turning off SG Ready output 1 (%s).", sg_ready1_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
        const bool new_value = sg_ready1_type != HEATING_SG_READY_ACTIVE_CLOSED;
        if (sg_ready_output_1 != new_value) {
            em_v2.set_sg_ready_output(1, new_value);
            last_sg_ready_change = rtc.timestamp_minutes();
            state.get("remaining_holding_time")->updateInt(config.get("minimum_holding_time")->asUint());
        }
    }

    // If §14 EnWG is triggerend, we don't override it.
    if (!p14enwg_on) {
        const bool sg_ready_output_0 = em_v2.get_sg_ready_output(0);
        if (sg_ready0_on) {
            state.get("sg_ready_blocking_active")->updateBool(true);
            extended_logging("Heating decision: Turning on SG Ready output 0 (%s).", sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
            const bool new_value = sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED;
            if (sg_ready_output_0 != new_value) {
                em_v2.set_sg_ready_output(0, new_value);
                last_sg_ready_change = rtc.timestamp_minutes();
                state.get("remaining_holding_time")->updateInt(config.get("minimum_holding_time")->asUint());
            }
        } else {
            state.get("sg_ready_blocking_active")->updateBool(false);
            extended_logging("Heating decision: Turning off SG Ready output 0 (%s).", sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
            const bool new_value = sg_ready0_type != HEATING_SG_READY_ACTIVE_CLOSED;
            if (sg_ready_output_0 != new_value) {
                em_v2.set_sg_ready_output(0, new_value);
                last_sg_ready_change = rtc.timestamp_minutes();
                state.get("remaining_holding_time")->updateInt(config.get("minimum_holding_time")->asUint());
            }
        }
    }
}
