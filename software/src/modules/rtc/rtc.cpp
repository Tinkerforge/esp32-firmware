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

#include "rtc.h"

#include <time.h>

#define TRACE_LOG_PREFIX nullptr

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "musl_libc_timegm.h"

static constexpr minutes_t RTC_TO_SYS_INTERVAL = 10_m;

static const char *get_quality_name(Rtc::Quality quality)
{
    switch (quality) {
    case Rtc::Quality::None:  return "None";
    case Rtc::Quality::RTC:   return "RTC";
    case Rtc::Quality::Low:   return "Low";
    case Rtc::Quality::High:  return "High";
    case Rtc::Quality::Force: return "Force";
    }

    return "<unknown>";
};

void IRtcBackend::set_time(const timeval &time)
{
    struct tm tm;
    gmtime_r(&time.tv_sec, &tm);
    set_time(tm, time.tv_usec);
}

void IRtcBackend::set_time(const tm &time, int32_t microseconds)
{
    struct tm copy = time;
    struct timeval timeval;
    timeval.tv_sec = timegm(&copy);
    timeval.tv_usec = microseconds;
    set_time(timeval);
}

void Rtc::pre_setup()
{
    this->trace_buf_index = logger.alloc_trace_buffer("rtc", 8192);

    time = Config::Object({
        {"year", Config::Uint16(0)},
        {"month", Config::Uint8(0)},
        {"day", Config::Uint8(0)},
        {"hour", Config::Uint8(0)},
        {"minute", Config::Uint8(0)},
        {"second", Config::Uint8(0)},
        {"weekday", Config::Uint8(0)},
    });

    time_update = time;

    config = Config::Object({
        {"auto_sync", Config::Bool(true)},
    });
}

void Rtc::setup()
{
    api.restorePersistentConfig("rtc/config", &config);

    initialized = true;
}

// Allow time to be 24h older than the build timestamp,
// in case the RTC is set by hand to test something.
// FIXME not Y2038-safe
static bool timestamp_acceptable(const struct timeval &time) {
    return time.tv_sec >= static_cast<time_t>(build_timestamp() - 25 * 60 * 60);
}

void Rtc::register_urls() {
    api.addState("rtc/time", &time, {}, {}, true);

    api.addPersistentConfig("rtc/config", &config);

    api.addCommand("rtc/time_update", &time_update, {}, [this](String &/*errmsg*/) {
        struct tm tm{};
        tm.tm_year = time_update.get("year")->asUint() - 1900;
        tm.tm_mon  = time_update.get("month")->asUint() - 1;
        tm.tm_mday = time_update.get("day")->asUint();
        tm.tm_hour = time_update.get("hour")->asUint();
        tm.tm_min  = time_update.get("minute")->asUint();
        tm.tm_sec  = time_update.get("second")->asUint();
        tm.tm_wday = time_update.get("weekday")->asUint();
        struct timeval timeval{};
        timeval.tv_sec = timegm(&tm);

        this->push_system_time(timeval, Rtc::Quality::Low);
    }, true);

    task_scheduler.scheduleWithFixedDelay([this]() {
        struct timeval tv{};
        gettimeofday(&tv, nullptr);

        if (!timestamp_acceptable(tv))
            return;

        struct tm tm{};
        gmtime_r(&tv.tv_sec, &tm);

        time.get("year")->updateUint(tm.tm_year + 1900);
        time.get("month")->updateUint(tm.tm_mon + 1);
        time.get("day")->updateUint(tm.tm_mday);
        time.get("hour")->updateUint(tm.tm_hour);

#if MODULE_NTP_AVAILABLE()
        if (time.get("minute")->updateUint(tm.tm_min))
            ntp.set_api_time(tv);
#else
        time.get("minute")->updateUint(tm.tm_min);
#endif

        time.get("second")->updateUint(tm.tm_sec);
        time.get("weekday")->updateUint(tm.tm_wday);
    }, 200_ms);

    task_scheduler.scheduleWithFixedDelay([this]() {
        update_system_time_from_rtc();
    }, RTC_TO_SYS_INTERVAL);
}

void Rtc::update_system_time_from_rtc() {
    std::lock_guard<std::recursive_mutex> lock{push_system_time_mutex};

    // Don't read back from the RTC if the last NTP sync was less than the sync interval ago
    // to prevent overwriting the NTP time with an RTC time that was not yet updated.
    if (!deadline_elapsed(last_sync + RTC_TO_SYS_INTERVAL)) {
        return;
    }

    for (auto *backend : backends) {
        auto t = backend->get_time();

        if (t.tv_sec == 0 && t.tv_usec == 0)
            continue;

        this->push_system_time(t, Quality::RTC);
        break;
    }
}

void Rtc::register_backend(IRtcBackend *_backend)
{
    if (_backend == nullptr)
        return;

    backends.push_back(_backend);

    // Only register the API if this is the first backend.
    if (backends.size() > 1)
        return;

    bool not_synced = last_sync < 0_us;
    update_system_time_from_rtc();
    if (not_synced && last_sync >= 0_us) {
        auto now = millis();
        auto secs = now / 1000;
        auto ms = now % 1000;
        logger.printfln("System time set from RTC at %lu,%03lu", secs, ms);
    }

    api.addFeature("rtc");
}

void Rtc::reset()
{
    for (auto *backend : backends)
        backend->reset();
}

void Rtc::update_rtc_from_system_time(int attempt) {
    struct timeval time;
    gettimeofday(&time, nullptr);

    if (time.tv_usec > 10000 && attempt < 3) {
        // Schedule this task to run directly after the next second.
        task_scheduler.scheduleOnce([this, attempt, time]() {
            this->update_rtc_from_system_time(attempt + 1);
        }, millis_t{((1000 * 1000) - time.tv_usec + 999) / 1000});
        return;
    }

    for (auto *backend : backends) {
        backend->set_time(time);
    }

    if (attempt > 0) {
        logger.tracefln(this->trace_buf_index, "Failed to hit < X.010s %d times.%s", attempt, attempt >= 3 ? " RTC time may not be precise" : "");
    }
}

bool Rtc::push_system_time(const timeval &time, Quality quality)
{
    if (time.tv_sec == 0 && time.tv_usec == 0)
        return false;

    if (!timestamp_acceptable(time))
        return false;

    micros_t max_age = 0_us;

    switch(quality) {
        case Quality::None:
            break;
        case Quality::RTC:
        case Quality::Low:
            max_age = 10_m;
            break;
        case Quality::High:
        case Quality::Force:
            max_age = 25_h;
            break;
    }

    {
        std::lock_guard<std::recursive_mutex> lock{push_system_time_mutex};

        if (quality < last_sync_quality && !deadline_elapsed(last_sync_ok_deadline)) {
            return false;
        }

        struct tm timeinfo = {};
        localtime_r(&time.tv_sec, &timeinfo);

        char buf[EVENT_LOG_TIMESTAMP_LENGTH + 1] = {};
        size_t written = strftime(buf, ARRAY_SIZE(buf), "%F %T", &timeinfo);
        snprintf(buf + written, EVENT_LOG_TIMESTAMP_LENGTH + 1 - written, ",%03ld", time.tv_usec / 1000);

        logger.tracefln(this->trace_buf_index, "Set time to %s at %lu. Quality %s", buf, millis(), get_quality_name(quality));

        settimeofday(&time, NULL);

        last_sync_quality = quality;
        last_sync = now_us();
        last_sync_ok_deadline = last_sync + max_age;
    }

    // Maybe we run in another thread.
    // Schedule this task to run directly after the next second.
    if (quality != Quality::RTC) {
        task_scheduler.scheduleOnce([this]() {
            this->update_rtc_from_system_time(0);
        }, millis_t{((1000 * 1000) - time.tv_usec + 999) / 1000});
    }
    return true;
}

bool Rtc::clock_synced(struct timeval *out_tv_now) {
    gettimeofday(out_tv_now, nullptr);
    return timestamp_acceptable(*out_tv_now);
}
