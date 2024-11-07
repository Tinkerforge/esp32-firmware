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


static constexpr auto HEATING_UPDATE_INTERVAL = 1_m;

#define HEATING_SG_READY_ACTIVE_CLOSED 0
#define HEATING_SG_READY_ACTIVE_OPEN   1

#define extended_logging(fmt, ...) \
    do { \
        if (extended_logging_active) { \
            logger.tracefln(fmt __VA_OPT__(,) __VA_ARGS__); \
        } \
    } while (0)

void Heating::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"sg_ready_blocking_active_type", Config::Uint(0, 0, 1)},
        {"sg_ready_extended_active_type", Config::Uint(0, 0, 1)},
        {"minimum_control_holding_time", Config::Uint(15, 0, 60)},
        {"meter_slot_grid_power", Config::Uint(POWER_MANAGER_DEFAULT_METER_SLOT, 0, METERS_SLOTS - 1)},
        {"extended_logging_active", Config::Bool(false)},
        {"summer_start_day", Config::Uint(15, 1, 31)},
        {"summer_start_month", Config::Uint(3, 1, 12)},
        {"summer_end_day", Config::Uint(1, 1, 31)},
        {"summer_end_month", Config::Uint(11, 1, 12)},
        {"summer_active_time_active", Config::Bool(false)},
        {"summer_active_time_start", Config::Int(8*60)}, // localtime in minutes since 00:00
        {"summer_active_time_end", Config::Int(20*60)}, // localtime in minutes since 00:00
        {"summer_yield_forecast_active", Config::Bool(false)},
        {"summer_yield_forecast_threshold", Config::Uint(0)},
        {"dpc_extended_active", Config::Bool(false)},
        {"dpc_extended_threshold", Config::Uint(80, 0, 100)},
        {"dpc_blocking_active", Config::Bool(false)},
        {"dpc_blocking_threshold", Config::Uint(120, 100, 1000)},
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
        {"p14enwg_active", Config::Bool(false)}
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

    task_scheduler.scheduleWallClock([this]() {
        this->update();
    }, HEATING_UPDATE_INTERVAL, 0_ms, true);
}

bool Heating::is_active()
{
    const bool dpc_extended_active          = config.get("dpc_extended_active")->asBool();
    const bool pv_excess_control_active     = config.get("pv_excess_control_active")->asBool();
    const bool summer_active_time_active    = config.get("summer_active_time_active")->asBool();
    const bool summer_yield_forecast_active = config.get("summer_yield_forecast_active")->asBool();

    if(!summer_active_time_active && !summer_yield_forecast_active && !dpc_extended_active && !pv_excess_control_active) {
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
    const uint8_t minimum_control_holding_time = config.get("minimum_control_holding_time")->asUint();
    const uint32_t minutes = rtc.timestamp_minutes();
    if(minutes < (last_sg_ready_change + minimum_control_holding_time)) {
        extended_logging("Minimum control holding time not reached. Current time: %dmin, last change: %dmin, minimum holding time: %dmin.", minutes, last_sg_ready_change, minimum_control_holding_time);
        return;
    }

    const uint32_t meter_slot_grid_power           = config.get("meter_slot_grid_power")->asUint();
    const uint32_t summer_start_day                = config.get("summer_start_day")->asUint();
    const uint32_t summer_start_month              = config.get("summer_start_month")->asUint();
    const uint32_t summer_end_day                  = config.get("summer_end_day")->asUint();
    const uint32_t summer_end_month                = config.get("summer_end_month")->asUint();
    const bool     summer_active_time_active       = config.get("summer_active_time_active")->asBool();
    const int32_t  summer_active_time_start        = config.get("summer_active_time_start")->asInt();
    const int32_t  summer_active_time_end          = config.get("summer_active_time_end")->asInt();
    const bool     summer_yield_forecast_active    = config.get("summer_yield_forecast_active")->asBool();
    const uint32_t summer_yield_forecast_threshold = config.get("summer_yield_forecast_threshold")->asUint();
    const bool     dpc_extended_active             = config.get("dpc_extended_active")->asBool();
    const uint32_t dpc_extended_threshold          = config.get("dpc_extended_threshold")->asUint();
    const bool     dpc_blocking_active             = config.get("dpc_blocking_active")->asBool();
    const uint32_t dpc_blocking_threshold          = config.get("dpc_blocking_threshold")->asUint();
    const bool     pv_excess_control_active        = config.get("pv_excess_control_active")->asBool();
    const uint32_t pv_excess_control_threshold     = config.get("pv_excess_control_threshold")->asUint();

    if(!summer_active_time_active && !summer_yield_forecast_active && !dpc_extended_active && !pv_excess_control_active) {
        extended_logging("No control active.");
        return;
    }

    const time_t now              = time(NULL);
    const struct tm *current_time = localtime(&now);
    const int current_month       = current_time->tm_mon + 1;
    const int current_day         = current_time->tm_mday;
    const int current_minutes     = current_time->tm_hour * 60 + current_time->tm_min;

    const bool is_summer = ((current_month == summer_start_month) && (current_day >= summer_start_day )) ||
                           ((current_month == summer_end_month  ) && (current_day <= summer_end_day   )) ||
                           ((current_month > summer_start_month ) && (current_month < summer_end_month));

    bool sg_ready0_on = false;
    bool sg_ready1_on = false;

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
    auto handle_dynamic_price = [&] () {
        if (!dpc_extended_active && !dpc_blocking_active) {
            return;
        }

        const auto price_average = day_ahead_prices.get_average_price_today();
        const auto price_current = day_ahead_prices.get_current_price();

        if (!price_average.data_available) {
            extended_logging("Average price for today not available. Ignoring dynamic price control.");
        } else if (!price_current.data_available) {
            extended_logging("Current price not available. Ignoring dynamic price control.");
        } else {
            if (dpc_extended_active) {
                if (price_current.data < price_average.data * dpc_extended_threshold / 100.0) {
                    extended_logging("Price is below extended threshold. Average price: %dmct, current price: %dmct, threshold: %d%%.", price_average.data, price_current.data, dpc_extended_threshold);
                    sg_ready1_on |= true;
                } else {
                    extended_logging("Price is above extended threshold. Average price: %dmct, current price: %dmct, threshold: %d%%.", price_average.data, price_current.data, dpc_extended_threshold);
                    sg_ready1_on |= false;
                }
            }
            if (dpc_blocking_active) {
                if (price_current.data > price_average.data * dpc_blocking_threshold / 100.0) {
                    extended_logging("Price is above blocking threshold. Average price: %dmct, current price: %dmct, threshold: %d%%.", price_average.data, price_current.data, dpc_blocking_threshold);
                    sg_ready0_on |= true;
                } else {
                    extended_logging("Price is below blocking threshold. Average price: %dmct, current price: %dmct, threshold: %d%%.", price_average.data, price_current.data, dpc_blocking_threshold);
                    sg_ready0_on |= false;
                }
            }
        }
    };

    if (!is_summer) { // Winter
        extended_logging("It is winter. Current month: %d, summer start month: %d, summer end month: %d, current day: %d, summer start day: %d, summer end day: %d.", current_month, summer_start_month, summer_end_month, current_day, summer_start_day, summer_end_day);
        if (!dpc_extended_active && !dpc_blocking_active && !pv_excess_control_active) {
            extended_logging("It is winter but no winter control active.");
        } else {
            handle_dynamic_price();
            handle_pv_excess();
        }
    } else { // Summer
        extended_logging("It is summer. Current month: %d, summer start month: %d, summer end month: %d, current day: %d, summer start day: %d, summer end day: %d.", current_month, summer_start_month, summer_end_month, current_day, summer_start_day, summer_end_day);
        bool blocked = false;
        bool is_morning = false;
        bool is_evening = false;
        if (summer_active_time_active) {
            if (current_minutes <= summer_active_time_start) { // if is between 00:00 and summer_active_time_start
                extended_logging("We are outside morning active time. Current time: %d, active time morning start: %d.", current_minutes, summer_active_time_start);
                blocked    = true;
                is_morning = true;
            } else if(summer_active_time_end <= current_minutes) { // if is between summer_active_time_end and 23:59
                extended_logging("We are outside evening active time. Current time: %d, active time evening end: %d.", current_minutes, summer_active_time_end);
                blocked    = true;
                is_evening = true;
            } else {
                extended_logging("We are in active time. Current time: %d, active time morning: %d, active time evening: %d.", current_minutes, summer_active_time_start, summer_active_time_end);
            }
        }

        // If we are outside active time and px excess control is active,
        // we check the expected px excess and unblock if it is below the threshold.
        if (blocked && summer_yield_forecast_active) {
            extended_logging("We are outside active time and yield forecast is active.");
            DataReturn<uint32_t> wh_expected = {false, 0};
            if (is_morning) {
                wh_expected = solar_forecast.get_wh_today();
            } else if (is_evening) {
                wh_expected = solar_forecast.get_wh_tomorrow();
            } else {
                extended_logging("We are outside active time but not in morning or evening. Ignoring yield forecast.");
            }

            if(!wh_expected.data_available) {
                extended_logging("Expected PV yield not available. Ignoring yield forecast.");
            } else {
                if (wh_expected.data/1000 < summer_yield_forecast_threshold) {
                    extended_logging("Expected PV yield %dkWh is below threshold of %dkWh.", wh_expected.data/1000, summer_yield_forecast_threshold);
                    blocked = false;
                } else {
                    extended_logging("Expected PV yield %dkWh is above or equal to threshold of %dkWh.", wh_expected.data/1000, summer_yield_forecast_threshold);
                }
            }
        }

        if (blocked) {
            extended_logging("We are outside of the active time.");
            sg_ready1_on = false;
        } else {
            if (!dpc_extended_active && !pv_excess_control_active) {
                extended_logging("It is summer but no summer control active.");
            } else {
                handle_dynamic_price();
                handle_pv_excess();
            }
        }
    }

    const bool sg_ready_output_1 = em_v2.get_sg_ready_output(1);
    if (sg_ready1_on) {
        state.get("sg_ready_extended_active")->updateBool(true);
        extended_logging("Heating decision: Turning on SG Ready output 1 (%s).", sg_ready1_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
        const bool new_value = sg_ready1_type == HEATING_SG_READY_ACTIVE_CLOSED;
        if (sg_ready_output_1 != new_value) {
            em_v2.set_sg_ready_output(1, new_value);
            last_sg_ready_change = rtc.timestamp_minutes();
        }
    } else {
        state.get("sg_ready_extended_active")->updateBool(false);
        extended_logging("Heating decision: Turning off SG Ready output 1 (%s).", sg_ready1_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
        const bool new_value = sg_ready1_type != HEATING_SG_READY_ACTIVE_CLOSED;
        if (sg_ready_output_1 != new_value) {
            em_v2.set_sg_ready_output(1, new_value);
            last_sg_ready_change = rtc.timestamp_minutes();
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
            }
        } else {
            state.get("sg_ready_blocking_active")->updateBool(false);
            extended_logging("Heating decision: Turning off SG Ready output 0 (%s).", sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
            const bool new_value = sg_ready0_type != HEATING_SG_READY_ACTIVE_CLOSED;
            if (sg_ready_output_0 != new_value) {
                em_v2.set_sg_ready_output(0, new_value);
                last_sg_ready_change = rtc.timestamp_minutes();
            }
        }
    }
}
