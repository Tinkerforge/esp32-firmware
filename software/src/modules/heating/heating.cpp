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

#define TRACE_LOG_PREFIX nullptr

#include "heating.h"

#include <time.h>
#include <type_traits>
#include <climits>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "options.h"
#include "control_period.enum.h"

// Heating curve temperature endpoints in degrees Celsius * 100
static constexpr int16_t HEATING_CURVE_WARM_TEMP = 2000;  // 20.00°C
static constexpr int16_t HEATING_CURVE_COLD_TEMP = -1000; // -10.00°C

// Daytime extension: extended operation from 09:00 to 18:00 if daily average < 5°C
static constexpr int16_t DAYTIME_EXTENSION_TEMP_THRESHOLD = 500; // 5.00°C
static constexpr uint16_t DAYTIME_EXTENSION_START_MINUTES = 9 * 60;   // 09:00
static constexpr uint16_t DAYTIME_EXTENSION_END_MINUTES   = 18 * 60;  // 18:00

static constexpr auto HEATING_UPDATE_INTERVAL = 1_min;
static constexpr auto MAX_STARTUP_DELAY = 1_min;

#define HEATING_SG_READY_ACTIVE_CLOSED 0
#define HEATING_SG_READY_ACTIVE_OPEN   1

void Heating::pre_setup()
{
    this->trace_buffer_index = logger.alloc_trace_buffer("heating", 1 << 18);

    config = ConfigRoot{Config::Object({
        {"sgr_blocking_type", Config::Uint(0, 0, 1)},
        {"sgr_extended_type", Config::Uint(0, 0, 1)},
        {"min_hold_time", Config::Uint(15, 10, 60)},
        {"meter_slot_grid_power", Config::Uint(OPTIONS_POWER_MANAGER_DEFAULT_METER_SLOT(), 0, OPTIONS_METERS_MAX_SLOTS() - 1)},
        {"control_period", Config::Enum(ControlPeriod::Hours24)},
        {"enable_heating_curve", Config::Bool(false)},
        {"enable_daytime_extension", Config::Bool(false)},
        {"extended_logging", Config::Bool(false)}, // Obsolete. Logging to trace log is now always on.
        {"yield_forecast", Config::Bool(false)},
        {"yield_forecast_threshold", Config::Uint16(0)},
        {"extended", Config::Bool(false)},
        {"extended_hours", Config::Uint(4, 0, 24)},
        {"extended_hours_warm", Config::Uint(3, 0, 24)},
        {"extended_hours_cold", Config::Uint(8, 0, 24)},
        {"blocking", Config::Bool(false)},
        {"blocking_hours", Config::Uint(0, 0, 24)},
        {"blocking_hours_warm", Config::Uint(8, 0, 24)},
        {"blocking_hours_cold", Config::Uint(0, 0, 24)},
        {"pv_excess_control", Config::Bool(false)},
        {"pv_excess_control_threshold", Config::Uint32(0)},
        {"p14enwg", Config::Bool(false)},
        {"p14enwg_input", Config::Uint(0, 0, 3)},
        {"p14enwg_type", Config::Uint(0, 0, 1)}
    }), [this](Config &update, ConfigSource source) -> String {
        uint8_t control_period_hours = 24;
        switch (update.get("control_period")->asEnum<ControlPeriod>()) {
            case ControlPeriod::Hours24: control_period_hours = 24; break;
            case ControlPeriod::Hours12: control_period_hours = 12; break;
            case ControlPeriod::Hours8:  control_period_hours = 8;  break;
            case ControlPeriod::Hours6:  control_period_hours = 6;  break;
            case ControlPeriod::Hours4:  control_period_hours = 4;  break;
        }

        // Validate that hours don't exceed the control period
        if (update.get("extended_hours")->asUint() > control_period_hours) {
            return "Extended hours exceed control period";
        }
        if (update.get("blocking_hours")->asUint() > control_period_hours) {
            return "Blocking hours exceed control period";
        }
        if (update.get("extended_hours")->asUint() + update.get("blocking_hours")->asUint() > control_period_hours) {
            return "Sum of extended and blocking hours exceeds control period";
        }

        // Validate curve hours at both endpoints
        if (update.get("extended_hours_warm")->asUint() + update.get("blocking_hours_warm")->asUint() > control_period_hours) {
            return "Sum of extended and blocking hours at warm endpoint exceeds control period";
        }
        if (update.get("extended_hours_cold")->asUint() + update.get("blocking_hours_cold")->asUint() > control_period_hours) {
            return "Sum of extended and blocking hours at cold endpoint exceeds control period";
        }

        task_scheduler.scheduleOnce([this]() {
            this->update();
        });
        return "";
    }};

    sgr_blocking_override = ConfigRoot{Config::Object({
        // timestamp minutes.
        {"override_until", Config::Uint32(0)},
    })};

    plan = Config::Object({
        {"cheap",     Config::Array({}, Config::get_prototype_bool_false(), 0, DAY_AHEAD_PRICE_MAX_AMOUNT)},
        {"expensive", Config::Array({}, Config::get_prototype_bool_false(), 0, DAY_AHEAD_PRICE_MAX_AMOUNT)},
    });
    state = Config::Object({
        {"automation_override", Config::Bool(false)},
        {"sgr_blocking", Config::Bool(false)},
        {"sgr_extended", Config::Bool(false)},
        {"p14enwg", Config::Bool(false)},
        {"next_update", Config::Uint32(0)} // Unix timestamp in minutes, 0 = next update not yet known
    });
}

void Heating::setup()
{
    api.restorePersistentConfig("heating/config", &config);

    initialized = true;

    if (must_delay_startup()) {
        logger.tracefln(this->trace_buffer_index, "Startup delay started");

        startup_delay_task_id = task_scheduler.scheduleOnce([this]() {
            logger.tracefln(this->trace_buffer_index, "Startup delay expired");

            // Mark end of startup delay by resetting the stored task ID.
            this->startup_delay_task_id = 0;

            this->update();
        }, MAX_STARTUP_DELAY);
    }
}

void Heating::register_urls()
{
    api.addPersistentConfig("heating/config", &config);
    api.addState("heating/state",             &state);
    api.addState("heating/plan",              &plan);
    api.addCommand("heating/reset_holding_time", Config::Null(), {}, [this](Language /*language*/, String &/*errmsg*/) {
        this->last_sg_ready_change = 0;
        this->update();
    }, true);

    // We dont want a persistent config since we dont want to save this across reboots.
    // This is why the config is build manually here.
    api.addState("heating/sgr_blocking_override", &sgr_blocking_override);
    api.addCommand("heating/sgr_blocking_override_update", &sgr_blocking_override, {}, [this](Language /*language*/, String &/*errmsg*/) {
        task_scheduler.cancel(this->override_task_id);
        this->override_task_id = 0;

        uint32_t override_until = sgr_blocking_override.get("override_until")->asUint();
        uint32_t now = rtc.timestamp_minutes();

        if (override_until <= now) {
            return;
        }

        // Override the timeout to force sg_ready to switch instantly
        this->last_sg_ready_change = 0;
        this->update();

        const millis_t timeout = millis_t((uint64_t)(override_until - now) * 1000 * 60);
        this->override_task_id = task_scheduler.scheduleOnce([this]() {
            this->sgr_blocking_override.get("override_until")->updateUint(0);
            this->last_sg_ready_change = 0;
            this->update();
        }, timeout);
    }, true);

    api.addCommand("heating/toggle_sgr_blocking", Config::Null(), {}, [this](Language /*language*/, String &/*errmsg*/) {
        const bool sg_ready_output_0 = em_v2.get_sg_ready_output(0);
        em_v2.set_sg_ready_output(0, !sg_ready_output_0);
        last_sg_ready_change = rtc.timestamp_minutes();
        state.get("next_update")->updateUint(last_sg_ready_change + config.get("min_hold_time")->asUint());
        state.get("sgr_blocking")->updateBool(!state.get("sgr_blocking")->asBool());
    }, true);

    api.addCommand("heating/toggle_sgr_extended", Config::Null(), {}, [this](Language /*language*/, String &errmsg) {
        if (this->is_p14enwg_active()) {
            errmsg = "Cannot toggle SG Ready output 2 when §14 EnWG is active.";
            return;
        }

        const bool sg_ready_output_1 = em_v2.get_sg_ready_output(1);
        em_v2.set_sg_ready_output(1, !sg_ready_output_1);
        last_sg_ready_change = rtc.timestamp_minutes();
        state.get("next_update")->updateUint(last_sg_ready_change + config.get("min_hold_time")->asUint());
        state.get("sgr_extended")->updateBool(!state.get("sgr_extended")->asBool());
    }, true);

#if MODULE_AUTOMATION_AVAILABLE()
    if (automation.has_task_with_action(AutomationActionID::EMSGReadySwitch)) {
        logger.printfln("Module disabled because automation rules with SG Ready actions exist");
        logger.tracefln(this->trace_buffer_index, "Module disabled because automation rules with SG Ready actions exist");

        state.get("automation_override")->updateBool(true);

        return;
    }
#endif

    task_scheduler.scheduleWallClock([this]() {
        this->update();
    }, HEATING_UPDATE_INTERVAL, 0_ms, true);
}

void Heating::register_events()
{
    if (state.get("automation_override")->asBool()) {
        return;
    }

    if (startup_delay_task_id != 0) {
        if (config.get("extended")->asBool() || config.get("blocking")->asBool() || config.get("enable_heating_curve")->asBool()) {
            event.registerEvent("day_ahead_prices/state", {}, [this](const Config * /*cfg*/) {
                return this->check_startup_delay_event();
            });
        }

        if (config.get("yield_forecast")->asBool()) {
            event.registerEvent("solar_forecast/state", {}, [this](const Config * /*cfg*/) {
                return this->check_startup_delay_event();
            });
        }
    }
}

bool Heating::is_active()
{
    const bool extended          = config.get("extended")->asBool();
    const bool blocking          = config.get("blocking")->asBool();
    const bool pv_excess_control = config.get("pv_excess_control")->asBool();
    const bool yield_forecast    = config.get("yield_forecast")->asBool();
    const bool heating_curve     = config.get("enable_heating_curve")->asBool();

    if(!yield_forecast && !extended && !blocking && !pv_excess_control && !heating_curve) {
        return false;
    }

    return true;
}

bool Heating::is_p14enwg_active()
{
    return config.get("p14enwg")->asBool();
}

Heating::Status Heating::get_status()
{
    const bool p14enwg            = config.get("p14enwg")->asBool();
    const uint32_t p14enwg_input  = config.get("p14enwg_input")->asUint();
    const uint32_t p14enwg_type   = config.get("p14enwg_type")->asUint();
    const uint32_t sg_ready0_type = config.get("sgr_blocking_type")->asUint();
    const uint32_t sg_ready1_type = config.get("sgr_extended_type")->asUint();
    const bool sg_ready_output_0  = em_v2.get_sg_ready_output(0);
    const bool sg_ready_output_1  = em_v2.get_sg_ready_output(1);
    const bool sg_ready0_on       = sg_ready_output_0 == (sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED);
    const bool sg_ready1_on       = sg_ready_output_1 == (sg_ready1_type == HEATING_SG_READY_ACTIVE_CLOSED);
    const bool p14_enwg_on        = p14enwg && (em_v2.get_input(p14enwg_input) == (p14enwg_type == 0));

    if(p14_enwg_on) {
        return Status::BlockingP14;
    } else if(sg_ready0_on) {
        return Status::Blocking;
    } else if(sg_ready1_on) {
        return Status::Extended;
    }

    return Status::Idle;
}

bool Heating::get_heating_curve_hours(uint32_t *out_extended_hours, uint32_t *out_blocking_hours)
{
#if MODULE_TEMPERATURES_AVAILABLE()
    const int16_t today_avg = temperatures.get_today_avg();

    // Check for sentinel value (no data available)
    if (today_avg == INT16_MAX) {
        logger.tracefln(this->trace_buffer_index, "Heating curve: Temperature data not available (avg=%d). Falling back to flat config.", today_avg);
        return false;
    }

    // Daily average temperature (in units of °C * 100)
    const int32_t avg_temp = static_cast<int32_t>(today_avg);

    // Clamp to curve endpoints
    const int32_t clamped_temp = std::max(static_cast<int32_t>(HEATING_CURVE_COLD_TEMP), std::min(static_cast<int32_t>(HEATING_CURVE_WARM_TEMP), avg_temp));

    // Linear interpolation factor: 0.0 at warm (20°C), 1.0 at cold (-10°C)
    // t = (warm - clamped) / (warm - cold)
    const int32_t temp_range = HEATING_CURVE_WARM_TEMP - HEATING_CURVE_COLD_TEMP; // 3000
    const int32_t t_num = HEATING_CURVE_WARM_TEMP - clamped_temp;

    const uint32_t extended_hours_warm = config.get("extended_hours_warm")->asUint();
    const uint32_t extended_hours_cold = config.get("extended_hours_cold")->asUint();
    const uint32_t blocking_hours_warm = config.get("blocking_hours_warm")->asUint();
    const uint32_t blocking_hours_cold = config.get("blocking_hours_cold")->asUint();

    // Interpolate: hours = warm_hours + (cold_hours - warm_hours) * t
    // Using integer arithmetic with rounding: result = warm + (cold - warm) * t_num / temp_range
    // Round to nearest: add temp_range/2 before dividing
    auto interpolate = [&](uint32_t hours_warm, uint32_t hours_cold) -> uint32_t {
        const int32_t diff = static_cast<int32_t>(hours_cold) - static_cast<int32_t>(hours_warm);
        const int32_t interpolated = static_cast<int32_t>(hours_warm) + (diff * t_num + (diff >= 0 ? temp_range / 2 : -temp_range / 2)) / temp_range;
        return static_cast<uint32_t>(std::max(static_cast<int32_t>(0), interpolated));
    };

    *out_extended_hours = interpolate(extended_hours_warm, extended_hours_cold);
    *out_blocking_hours = interpolate(blocking_hours_warm, blocking_hours_cold);

    logger.tracefln(this->trace_buffer_index, "Heating curve: avg_temp=%.2f°C, extended=%lu h, blocking=%lu h",
                    avg_temp / 100.0f,
                    static_cast<unsigned long>(*out_extended_hours),
                    static_cast<unsigned long>(*out_blocking_hours));

    return true;
#else
    (void)out_extended_hours;
    (void)out_blocking_hours;
    logger.tracefln(this->trace_buffer_index, "Heating curve: Temperatures module not available.");
    return false;
#endif
}

void Heating::update_plan()
{
    const bool enable_heating_curve = config.get("enable_heating_curve")->asBool();
    const bool extended = config.get("extended")->asBool() || enable_heating_curve;
    const bool blocking = config.get("blocking")->asBool() || enable_heating_curve;

    uint32_t extended_hours;
    uint32_t blocking_hours;

    if (enable_heating_curve && get_heating_curve_hours(&extended_hours, &blocking_hours)) {
        // Heating curve successfully computed interpolated hours.
        // If a curve channel computes 0 hours, treat that channel as inactive.
        // (get_heating_curve_hours already did the interpolation and rounding.)
    } else {
        // Fall back to flat config values (also used when heating curve is disabled).
        extended_hours = config.get("extended_hours")->asUint();
        blocking_hours = config.get("blocking_hours")->asUint();
    }

    // Respect the per-channel enable flags when curve is not active
    if (!enable_heating_curve) {
        if (!config.get("extended")->asBool()) {
            extended_hours = 0;
        }
        if (!config.get("blocking")->asBool()) {
            blocking_hours = 0;
        }
    }

    const uint32_t min_hold_time  = config.get("min_hold_time")->asUint();
    const ControlPeriod control_period = config.get("control_period")->asEnum<ControlPeriod>();

    // Get plan arrays early so early-return paths can clear them
    auto plan_cheap     = plan.get("cheap");
    auto plan_expensive = plan.get("expensive");

    // Get price data metadata via API
    const Config *prices_state = api.getState("day_ahead_prices/prices", false);
    if (prices_state == nullptr) {
        plan_cheap->removeAll();
        plan_expensive->removeAll();
        return;
    }

    auto prices_arr = prices_state->get("prices");
    const size_t num_prices = prices_arr->count();

    // If no prices or no price-based control, clear the plan
    if (num_prices == 0 || (!extended && !blocking)) {
        plan_cheap->removeAll();
        plan_expensive->removeAll();
        return;
    }

    const uint32_t first_date = prices_state->get("first_date")->asUint();
    const bool is_15min = (prices_state->get("resolution")->asUint() == 0);
    const uint8_t resolution_minutes = is_15min ? 15 : 60;
    const uint8_t slots_per_hour = is_15min ? 4 : 1;

    uint8_t duration_hours = 0;
    switch(control_period) {
        case ControlPeriod::Hours24: duration_hours = 24; break;
        case ControlPeriod::Hours12: duration_hours = 12; break;
        case ControlPeriod::Hours8:  duration_hours = 8;  break;
        case ControlPeriod::Hours6:  duration_hours = 6;  break;
        case ControlPeriod::Hours4:  duration_hours = 4;  break;
        default: plan_cheap->removeAll(); plan_expensive->removeAll(); return;
    }

    const uint8_t min_block_15m = (min_hold_time + 14) / 15;
    const uint8_t slots_per_period = duration_hours * slots_per_hour;
    // get_cheap_1h_blocked always returns at 15-min resolution (duration_hours * 4 entries)
    const uint8_t data_slots = duration_hours * 4;

    // Resize plan arrays to match price array length
    plan_cheap->setCount(num_prices);
    plan_expensive->setCount(num_prices);

    // Initialize all to false
    for (size_t i = 0; i < num_prices; i++) {
        plan_cheap->get(i)->updateBool(false);
        plan_expensive->get(i)->updateBool(false);
    }

    // For each control period block that overlaps with the price range,
    // compute the plan and fill in the corresponding plan slots.
    //
    // Blocks are aligned to local midnight + multiples of duration_hours.
    // We find the first block start that is <= first_date, then iterate forward.

    // Convert first_date (UTC minutes) to a local time to find block alignment
    const time_t first_date_seconds = static_cast<time_t>(first_date) * 60;
    struct tm first_local;
    localtime_r(&first_date_seconds, &first_local);
    const uint16_t first_minutes_since_midnight = first_local.tm_hour * 60 + first_local.tm_min;
    const uint16_t duration_minutes = duration_hours * 60;

    // Find the local-time block that contains first_date
    const uint16_t block_offset = (first_minutes_since_midnight / duration_minutes) * duration_minutes;
    // first_date minus the offset within the block gives us the start of that block (in UTC minutes)
    const int32_t first_block_start = static_cast<int32_t>(first_date) - (first_minutes_since_midnight - block_offset);

    const int32_t price_end = static_cast<int32_t>(first_date) + static_cast<int32_t>(num_prices) * resolution_minutes;

    // Max data_slots is 24*4 = 96
    bool data[96];

    for (int32_t block_start = first_block_start; block_start < price_end; block_start += duration_minutes) {
        // Only process full blocks completely covered by price data
        const int32_t block_start_index = (block_start - static_cast<int32_t>(first_date)) / resolution_minutes;
        const int32_t block_end_index = block_start_index + slots_per_period;
        if (block_start_index < 0 || block_end_index > static_cast<int32_t>(num_prices)) {
            continue;
        }

        if (extended) {
            std::fill_n(data, data_slots, false);
            if (day_ahead_prices.get_cheap_1h_blocked(block_start, duration_hours, extended_hours, min_block_15m, data)) {
                if (is_15min) {
                    // 1:1 mapping: data has slots_per_period entries
                    for (uint8_t i = 0; i < slots_per_period; i++) {
                        plan_cheap->get(static_cast<uint32_t>(block_start_index + i))->updateBool(data[i]);
                    }
                } else {
                    // 4:1 mapping: data has duration_hours*4 entries, price array has duration_hours entries
                    for (uint8_t i = 0; i < slots_per_period; i++) {
                        bool any = data[i*4] || data[i*4+1] || data[i*4+2] || data[i*4+3];
                        plan_cheap->get(static_cast<uint32_t>(block_start_index + i))->updateBool(any);
                    }
                }
            }
        }

        if (blocking) {
            std::fill_n(data, data_slots, false);
            if (day_ahead_prices.get_expensive_1h_blocked(block_start, duration_hours, blocking_hours, min_block_15m, data)) {
                if (is_15min) {
                    for (uint8_t i = 0; i < slots_per_period; i++) {
                        plan_expensive->get(static_cast<uint32_t>(block_start_index + i))->updateBool(data[i]);
                    }
                } else {
                    for (uint8_t i = 0; i < slots_per_period; i++) {
                        bool any = data[i*4] || data[i*4+1] || data[i*4+2] || data[i*4+3];
                        plan_expensive->get(static_cast<uint32_t>(block_start_index + i))->updateBool(any);
                    }
                }
            }
        }
    }
}

void Heating::update()
{
    if (state.get("automation_override")->asBool()) {
        const uint64_t task_id = task_scheduler.currentTaskId();

        if (task_id != 0) {
            const auto result = task_scheduler.cancel(task_id);
            logger.printfln("Cancelled update task: %u", static_cast<unsigned>(result));
        }

        return;
    }

    // Only update if clock is synced. Heating control depends on time of day.
    struct timeval tv_now;
    if (!rtc.clock_synced(&tv_now)) {
        state.get("next_update")->updateUint(0);
        logger.tracefln(this->trace_buffer_index, "Clock not synced. Skipping update.");
        return;
    }

    // Update the plan state early (before the holding time guard) so that
    // the API always reflects the current config/prices, even while the
    // relay is being held steady.
    update_plan();

    if (startup_delay_task_id != 0) {
        logger.tracefln(this->trace_buffer_index, "Startup delay active. Skipping update.");
        return;
    }

    // TODO: Does §14 EnWG need a smaller update interval or is it enough to check it every minute?
    const bool     p14enwg        = config.get("p14enwg")->asBool();
    const uint32_t p14enwg_input  = config.get("p14enwg_input")->asUint();
    const uint32_t p14enwg_type   = config.get("p14enwg_type")->asUint();
    const uint32_t sg_ready0_type = config.get("sgr_blocking_type")->asUint();
    const uint32_t sg_ready1_type = config.get("sgr_extended_type")->asUint();

    // Check if §14 EnWG should be turned on
    bool p14enwg_on = false;
    if(p14enwg) {
        bool input_value = em_v2.get_input(p14enwg_input);

        if (p14enwg_type == 0) {
            p14enwg_on = input_value;
        } else {
            p14enwg_on = !input_value;
        }
    }

    // If p14enwg is triggered, we immediately set output 0 accordingly.
    // If it is not triggered it depends on the heating controller if it should be on or off.
    if (p14enwg_on) {
        state.get("p14enwg")->updateBool(true);
        state.get("sgr_blocking")->updateBool(true);
        logger.tracefln(this->trace_buffer_index, "§14 EnWG blocks heating. Turning on SG ready output 0 (%s).", sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
        em_v2.set_sg_ready_output(0, sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED);
    } else {
        state.get("p14enwg")->updateBool(false);
    }

    // Get values from config
    const uint8_t min_hold_time = config.get("min_hold_time")->asUint();
    const uint32_t minutes = rtc.timestamp_minutes();
    uint32_t remaining_holding_time = 0;
    if (minutes >= last_sg_ready_change) {
        uint32_t time_since_last_change = minutes - last_sg_ready_change;
        if(time_since_last_change < min_hold_time) {
            remaining_holding_time = min_hold_time - time_since_last_change;
        }
    }

    if (remaining_holding_time > 0) {
        if (state.get("next_update")->asUint() == 0) {
            state.get("next_update")->updateUint(rtc.timestamp_minutes() + remaining_holding_time);
            return;
        }

        logger.tracefln(this->trace_buffer_index, "Minimum control holding time not reached. Current time: %limin, last change: %limin, minimum holding time: %hhumin.", minutes, last_sg_ready_change, min_hold_time);
        return;
    }

    const uint32_t meter_slot_grid_power       = config.get("meter_slot_grid_power")->asUint();
    const bool     yield_forecast              = config.get("yield_forecast")->asBool();
    const uint32_t yield_forecast_threshold    = config.get("yield_forecast_threshold")->asUint();
    const bool     enable_heating_curve        = config.get("enable_heating_curve")->asBool();
    const bool     enable_daytime_extension    = config.get("enable_daytime_extension")->asBool();
    const bool     pv_excess_control           = config.get("pv_excess_control")->asBool();
    const uint32_t pv_excess_control_threshold = config.get("pv_excess_control_threshold")->asUint();
    const ControlPeriod control_period         = config.get("control_period")->asEnum<ControlPeriod>();

    // Determine effective extended/blocking hours, considering heating curve
    bool extended;
    uint32_t extended_hours;
    bool blocking;
    uint32_t blocking_hours;

    uint32_t curve_ext = 0, curve_blk = 0;
    if (enable_heating_curve && get_heating_curve_hours(&curve_ext, &curve_blk)) {
        // Heating curve is active: use interpolated values
        extended       = true;
        extended_hours = curve_ext;
        blocking       = true;
        blocking_hours = curve_blk;
    } else {
        // Flat config values (also fallback when curve data unavailable)
        extended       = config.get("extended")->asBool();
        extended_hours = config.get("extended_hours")->asUint();
        blocking       = config.get("blocking")->asBool();
        blocking_hours = config.get("blocking_hours")->asUint();
    }

    bool sg_ready0_on = false;
    bool sg_ready1_on = false;

    const bool sg_ready_output_1 = em_v2.get_sg_ready_output(1);

    if(!yield_forecast && !extended && !blocking && !pv_excess_control) {
        logger.tracefln(this->trace_buffer_index, "No control active.");
    } else {
        const time_t now = time(NULL);
        struct tm current_time;
        localtime_r(&now, &current_time);

        const uint16_t minutes_since_midnight = current_time.tm_hour * 60 + current_time.tm_min;
        if (minutes_since_midnight >= 24*60) {
            logger.tracefln(this->trace_buffer_index, "Too many minutes since midnight: %d.", minutes_since_midnight);
            return;
        }

        // PV excess handling for winter and summer
        auto handle_pv_excess = [&] () {
            if (!pv_excess_control) {
                return;
            }
            float watt_current = 0;
            MeterValueAvailability meter_availability = meters.get_power(meter_slot_grid_power, &watt_current);
            if (meter_availability != MeterValueAvailability::Fresh) {
                logger.tracefln(this->trace_buffer_index, "Meter value not available (meter %lu has availability %d). Ignoring PV excess control.", meter_slot_grid_power, static_cast<std::underlying_type<MeterValueAvailability>::type>(meter_availability));
            } else {
                const bool active_value = sg_ready1_type == HEATING_SG_READY_ACTIVE_CLOSED;
                if (sg_ready_output_1 == active_value) {
                    if ((-watt_current) > 0) {
                        logger.tracefln(this->trace_buffer_index, "Current PV excess is above threshold. Current PV excess: %dW, threshold: 0W (sgr1 is active).", (int)watt_current);
                        sg_ready1_on |= true;
                    }
                } else {
                    if ((-watt_current) > pv_excess_control_threshold) {
                        logger.tracefln(this->trace_buffer_index, "Current PV excess is above threshold. Current PV excess: %iW, threshold: %luW. (sgr1 is not active)", (int)watt_current, pv_excess_control_threshold);
                        sg_ready1_on |= true;
                    }
                }
            }
        };

        // Dynamic price handling for winter and summer
        auto handle_dynamic_price = [&] (const bool handle_extended, const bool handle_blocking) {
            if (!extended && !blocking) {
                return;
            }

            auto print_hours_today = [&] (const char *name, const bool *hours, const uint8_t duration) {
                char buffer[duration*4 + 1] = {'\0'};
                for (int i = 0; i < duration*4; i++) {
                    buffer[i] = hours[i] ? '1' : '0';
                }
                logger.tracefln(this->trace_buffer_index, "%s: %s", name, buffer);
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

            const uint8_t min_block_15m = (min_hold_time + 14) / 15; // ceil(min_hold_time / 15)

            if (handle_extended) {
                const bool data_available = day_ahead_prices.get_cheap_1h_blocked(start_time, duration, extended_hours, min_block_15m, data);
                if (!data_available) {
                    logger.tracefln(this->trace_buffer_index, "Cheap hours not available. Ignoring extended control.");
                } else {
                    print_hours_today("Cheap hours", data, duration);
                    if (data[current_index]) {
                        logger.tracefln(this->trace_buffer_index, "Current time is in cheap hours.");
                        sg_ready1_on |= true;
                    } else {
                        logger.tracefln(this->trace_buffer_index, "Current time is not in cheap hours.");
                        sg_ready1_on |= false;
                    }
                }
            }
            if (handle_blocking) {
                const bool data_available = day_ahead_prices.get_expensive_1h_blocked(start_time, duration, blocking_hours, min_block_15m, data);
                if (!data_available) {
                    logger.tracefln(this->trace_buffer_index, "Expensive hours not available. Ignoring blocking control.");
                } else {
                    print_hours_today("Expensive hours", data, duration);
                    if (data[current_index]) {
                        logger.tracefln(this->trace_buffer_index, "Current time is in expensive hours.");
                        sg_ready0_on |= true;
                    } else {
                        logger.tracefln(this->trace_buffer_index, "Current time is not in expensive hours.");
                        sg_ready0_on |= false;
                    }
                }
            }
        };

        auto is_expected_yield_high = [&] () {
            // we check the expected pv excess and unblock if it is below the threshold.
            if (!yield_forecast) {
                logger.tracefln(this->trace_buffer_index, "Yield forecast is inactive.");
                return false;
            }

            logger.tracefln(this->trace_buffer_index, "Yield forecast is active.");
            uint32_t wh_expected;

            if(!solar_forecast.get_wh_today().try_unwrap(&wh_expected)) {
                logger.tracefln(this->trace_buffer_index, "Expected PV yield not available. Ignoring yield forecast.");
            } else {
                if (wh_expected/1000 < yield_forecast_threshold) {
                    logger.tracefln(this->trace_buffer_index, "Expected PV yield %lukWh is below threshold of %lukWh.", wh_expected/1000, yield_forecast_threshold);
                } else {
                    logger.tracefln(this->trace_buffer_index, "Expected PV yield %lukWh is above or equal to threshold of %lukWh.", wh_expected/1000, yield_forecast_threshold);
                    return true;
                }
            }

            return false;
        };

        const bool pv_yield_high = is_expected_yield_high();
        if (pv_yield_high) {
            logger.tracefln(this->trace_buffer_index, "Day ahead prices are ignored for cheap hours because of expected PV yield.");
            handle_dynamic_price(false, blocking);
        } else {
            handle_dynamic_price(extended, blocking);
        }
        handle_pv_excess();
    }

    // Cold weather daytime restriction: If daily average temperature < 5°C,
    // restrict extended operation to the 09:00-18:00 window only.
    // Outside this window, price-based extended operation is suppressed.
    // Useful for air-to-air heat pumps that are very inefficient at low temperatures.
    if (enable_heating_curve && enable_daytime_extension) {
#if MODULE_TEMPERATURES_AVAILABLE()
        const int16_t today_avg = temperatures.get_today_avg();

        if (today_avg != INT16_MAX) {
            const int32_t avg_temp = static_cast<int32_t>(today_avg);

            if (avg_temp < DAYTIME_EXTENSION_TEMP_THRESHOLD) {
                const time_t now_dt = time(NULL);
                struct tm local_now;
                localtime_r(&now_dt, &local_now);
                const uint16_t minutes_now = local_now.tm_hour * 60 + local_now.tm_min;

                if ((minutes_now < DAYTIME_EXTENSION_START_MINUTES) || (minutes_now >= DAYTIME_EXTENSION_END_MINUTES)) {
                    logger.tracefln(this->trace_buffer_index,
                                    "Daytime restriction active: avg_temp=%.2f°C < 5°C, time %02d:%02d outside 09:00-18:00 window. Suppressing extended operation.",
                                    avg_temp / 100.0f,
                                    local_now.tm_hour,
                                    local_now.tm_min);
                    sg_ready1_on = false;
                } else {
                    logger.tracefln(this->trace_buffer_index,
                                    "Daytime restriction: avg_temp=%.2f°C < 5°C, time %02d:%02d within 09:00-18:00 window. Extended operation allowed.",
                                    avg_temp / 100.0f,
                                    local_now.tm_hour,
                                    local_now.tm_min);
                }
            } else {
                logger.tracefln(this->trace_buffer_index,
                                "Daytime restriction: avg_temp=%.2f°C >= 5°C threshold. No restriction applied.",
                                avg_temp / 100.0f);
            }
        } else {
            logger.tracefln(this->trace_buffer_index, "Daytime restriction: Temperature data not available. No restriction applied.");
        }
#else
        logger.tracefln(this->trace_buffer_index, "Daytime restriction: Temperatures module not available.");
#endif
    }

    // If p14enwg is triggered, we never turn output 1 (extended operation) on.
    if (p14enwg_on) {
        sg_ready1_on = false;
    // If sg_ready0 and sg_ready1 are both to be turned on (e.g. high price but enough sun for heating with pv excess),
    // we turn off sg_ready0. E.g. we prefer extended operation instead of blocking operation in this case.
    } else if (sg_ready0_on && sg_ready1_on) {
        sg_ready0_on = false;
    }

    if (sg_ready1_on) {
        state.get("sgr_extended")->updateBool(true);
        logger.tracefln(this->trace_buffer_index, "Heating decision: Turning on SG Ready output 1 (%s).", sg_ready1_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
        const bool new_value = sg_ready1_type == HEATING_SG_READY_ACTIVE_CLOSED;
        if (sg_ready_output_1 != new_value) {
            em_v2.set_sg_ready_output(1, new_value);
            last_sg_ready_change = rtc.timestamp_minutes();
            state.get("next_update")->updateUint(last_sg_ready_change + config.get("min_hold_time")->asUint());
        }
    } else {
        state.get("sgr_extended")->updateBool(false);
        logger.tracefln(this->trace_buffer_index, "Heating decision: Turning off SG Ready output 1 (%s).", sg_ready1_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
        const bool new_value = sg_ready1_type != HEATING_SG_READY_ACTIVE_CLOSED;
        if (sg_ready_output_1 != new_value) {
            em_v2.set_sg_ready_output(1, new_value);
            last_sg_ready_change = rtc.timestamp_minutes();
            state.get("next_update")->updateUint(last_sg_ready_change + config.get("min_hold_time")->asUint());
        }
    }

    uint32_t override_until = sgr_blocking_override.get("override_until")->asUint();
    if (override_until > 0) {
        uint32_t now = rtc.timestamp_minutes();
        if (now >= override_until) {
            logger.tracefln(this->trace_buffer_index, "Override time is over. Resetting override.");
            sgr_blocking_override.get("override_until")->updateUint(0);
        } else {
            logger.tracefln(this->trace_buffer_index, "Override time is active. Current time: %limin, override until: %limin.", now, override_until);
            sg_ready0_on = false;
        }
    }

    // If §14 EnWG is triggerend, we don't override it.
    if (!p14enwg_on) {
        const bool sg_ready_output_0 = em_v2.get_sg_ready_output(0);
        if (sg_ready0_on) {
            state.get("sgr_blocking")->updateBool(true);
            logger.tracefln(this->trace_buffer_index, "Heating decision: Turning on SG Ready output 0 (%s).", sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
            const bool new_value = sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED;
            if (sg_ready_output_0 != new_value) {
                em_v2.set_sg_ready_output(0, new_value);
                last_sg_ready_change = rtc.timestamp_minutes();
                state.get("next_update")->updateUint(last_sg_ready_change + config.get("min_hold_time")->asUint());
            }
        } else {
            state.get("sgr_blocking")->updateBool(false);
            logger.tracefln(this->trace_buffer_index, "Heating decision: Turning off SG Ready output 0 (%s).", sg_ready0_type == HEATING_SG_READY_ACTIVE_CLOSED ? "active closed" : "active open");
            const bool new_value = sg_ready0_type != HEATING_SG_READY_ACTIVE_CLOSED;
            if (sg_ready_output_0 != new_value) {
                em_v2.set_sg_ready_output(0, new_value);
                last_sg_ready_change = rtc.timestamp_minutes();
                state.get("next_update")->updateUint(last_sg_ready_change + config.get("min_hold_time")->asUint());
            }
        }
    }
}

bool Heating::must_delay_startup()
{
    if (config.get("extended")->asBool() || config.get("blocking")->asBool() || config.get("enable_heating_curve")->asBool()) {
        auto current_price_net = day_ahead_prices.get_current_price_net();
        if (current_price_net.is_none()) {
            return true;
        }
    }

    if (config.get("yield_forecast")->asBool()) {
        if (solar_forecast.get_cached_wh_today() < 0) {
            return true;
        }
    }

    return false;
}

EventResult Heating::check_startup_delay_event()
{
    // Startup delay already over
    if (startup_delay_task_id == 0) {
        logger.tracefln(this->trace_buffer_index, "Startup delay already over");
        return EventResult::Deregister;
    }

    // Continue startup delay
    if (must_delay_startup()) {
        logger.tracefln(this->trace_buffer_index, "Continue startup delay");
        return EventResult::OK;
    }

    // Cancel startup delay
    logger.tracefln(this->trace_buffer_index, "Cancel startup delay");
    task_scheduler.cancel(startup_delay_task_id);
    startup_delay_task_id = 0;

    task_scheduler.scheduleOnce([this]() {
        this->update();
    });

    return EventResult::Deregister;
}
