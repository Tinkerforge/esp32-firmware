/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "tools.h"

#include <esp_system.h>
#include <esp_timer.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <lwip/inet_chksum.h>

#include "event_log_prefix.h"
#include "main_dependencies.h"

void vTaskDelay_ms(uint32_t delay_ms)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    // portTICK_PERIOD_MS expands to an old style cast.
    vTaskDelay(delay_ms / portTICK_PERIOD_MS);
#pragma GCC diagnostic pop
}

const char *tf_reset_reason(uint32_t *numeric_reason_out)
{
    esp_reset_reason_t reason = esp_reset_reason();

    if (numeric_reason_out) {
        *numeric_reason_out = static_cast<uint32_t>(reason);
    }

    switch (reason) {
        case ESP_RST_UNKNOWN:
            return "Reset reason reported as unknown";

        case ESP_RST_POWERON:
            return "Reset due to power-on";

        case ESP_RST_EXT:
            return "Reset by external pin";

        case ESP_RST_SW:
            return "Software reset via esp_restart";

        case ESP_RST_PANIC:
            return "Software reset due to exception/panic";

        case ESP_RST_INT_WDT:
            return "Reset due to interrupt watchdog";

        case ESP_RST_TASK_WDT:
            return "Reset due to task watchdog";

        case ESP_RST_WDT:
            return "Reset due to some watchdog";

        case ESP_RST_DEEPSLEEP:
            return "Reset after exiting deep sleep mode";

        case ESP_RST_BROWNOUT:
            return "Brownout reset";

        case ESP_RST_SDIO:
            return "Reset over SDIO";

        case ESP_RST_USB:
            return "Reset by USB peripheral";

        case ESP_RST_JTAG:
            return "Reset by JTAG";

        case ESP_RST_EFUSE:
            return "Reset due to efuse error";

        case ESP_RST_PWR_GLITCH:
            return "Reset due to power glitch detected";

        case ESP_RST_CPU_LOCKUP:
            return "Reset due to CPU lock up";

        default:
            return "Reset reason out of range";
    }
}

bool a_after_b(uint32_t a, uint32_t b)
{
    return ((uint32_t)(a - b)) < (UINT32_MAX / 2);
}

// implement TFTools/Micros.h now_us
micros_t now_us()
{
    return micros_t{esp_timer_get_time()};
}

// implement TFTools/Tools.h now_us
[[gnu::noreturn]] void system_abort(const char *message)
{
    esp_system_abort(message);
}

static int vprintf_dev_null(const char *fmt, va_list args)
{
    return 0;
}

LogSilencer::LogSilencer() : old_fn(nullptr)
{
    old_fn = esp_log_set_vprintf(vprintf_dev_null);
}

LogSilencer::~LogSilencer()
{
    esp_log_set_vprintf(old_fn);
}

TaskHandle_t mainTaskHandle;
void set_main_task_handle()
{
    mainTaskHandle = xTaskGetCurrentTaskHandle();
}

void led_blink(int8_t led_pin, int interval_ms, int blinks_per_interval, int off_time_ms)
{
    led_blink(led_pin, interval_ms, blinks_per_interval, off_time_ms, digitalWrite);
}

void led_blink(int8_t led_pin, int interval_ms, int blinks_per_interval, int off_time_ms, std::function<void(uint8_t, uint8_t)> led_write)
{
    if (led_pin < 0)
        return;

    int t_in_second = now_us().to<millis_t>().as<int64_t>() % interval_ms;
    if (off_time_ms != 0 && (interval_ms - t_in_second <= off_time_ms)) {
        led_write(led_pin, 1);
        return;
    }

    // We want blinks_per_interval blinks and blinks_per_interval pauses between them. The off_time counts as pause.
    int state_count = ((2 * blinks_per_interval) - (off_time_ms != 0 ? 1 : 0));
    int state_interval = (interval_ms - off_time_ms) / state_count;
    bool led = (t_in_second / state_interval) % 2 != 0;

    led_write(led_pin, led);
}

uint16_t internet_checksum(const void *data, size_t len)
{
    if (len > UINT16_MAX) {
        esp_system_abortf<96>("internet_checksum: length of %zu > %d not supported", len, UINT16_MAX);
    }

    return inet_chksum(data, static_cast<uint16_t>(len));
}

void trigger_reboot(const char *initiator, millis_t delay_ms)
{
    logger.printfln("Reboot requested by %s.", initiator);
    task_scheduler.scheduleOnce([]() {
        esp_restart();
    }, delay_ms);
}

bool Ownership::try_acquire(uint32_t owner_id)
{
    mutex.lock();

    if (owner_id == current_owner_id) {
        return true;
    }

    mutex.unlock();

    return false;
}

void Ownership::release()
{
    mutex.unlock();
}

uint32_t Ownership::current()
{
    return current_owner_id;
}

uint32_t Ownership::next()
{
    mutex.lock();

    uint32_t owner_id = ++current_owner_id;

    mutex.unlock();

    return owner_id;
}

OwnershipGuard::OwnershipGuard(Ownership *ownership, uint32_t owner_id) : ownership(ownership)
{
    acquired = ownership->try_acquire(owner_id);
}

OwnershipGuard::~OwnershipGuard()
{
    if (acquired) {
        ownership->release();
    }
}

bool OwnershipGuard::have_ownership()
{
    return acquired;
}

int remove_separator(const char *const in, char *out)
{
    int written = 0;
    size_t s = strlen(in);
    for (size_t i = 0; i < s; ++i) {
        if (in[i] == ':')
            continue;
        out[written] = in[i];
        ++written;
    }
    out[written] = '\0';
    return written;
}

int add_separator(const char *const in, size_t in_len, char *out)
{
    int written = 0;
    if (in_len == 0) {
        out[0] = '\0';
        return 0;
    }

    for (size_t i = 0; i < in_len; ++i) {
        out[written] = in[i];
        ++written;
        if (i % 2 == 1) {
            out[written] = ':';
            ++written;
        }
    }
    // Only to one char back if a ':' was inserted last.
    if (in_len % 2 == 0)
        --written;
    out[written] = '\0';
    return written;
}

int strncmp_with_same_len(const char *left, const char *right, size_t right_len)
{
    size_t left_len = strlen(left);
    if (left_len != right_len)
        return -1;
    return strncmp(left, right, right_len);
}

i2c_cmd_handle_t i2c_master_prepare_write_read_device(uint8_t device_address,
                                                      uint8_t *command_buffer, size_t command_buffer_size,
                                                      const uint8_t* write_buffer, size_t write_size,
                                                      uint8_t* read_buffer, size_t read_size) {
    bool write = write_buffer != nullptr && write_size > 0;
    bool read = read_buffer != nullptr && read_size > 0;
    if (!write && !read)
        return nullptr;

    esp_err_t err = ESP_OK;

    i2c_cmd_handle_t handle = i2c_cmd_link_create_static(command_buffer, command_buffer_size);
    assert(handle != NULL);

    if (write) {
        err = i2c_master_start(handle);
        if (err != ESP_OK) {
            goto error;
        }

        err = i2c_master_write_byte(handle, device_address << 1 | I2C_MASTER_WRITE, true);
        if (err != ESP_OK) {
            goto error;
        }

        err = i2c_master_write(handle, write_buffer, write_size, true);
        if (err != ESP_OK) {
            goto error;
        }
    }

    if (read) {
        err = i2c_master_start(handle);
        if (err != ESP_OK) {
            goto error;
        }

        err = i2c_master_write_byte(handle, device_address << 1 | I2C_MASTER_READ, true);
        if (err != ESP_OK) {
            goto error;
        }

        err = i2c_master_read(handle, read_buffer, read_size, I2C_MASTER_LAST_NACK);
        if (err != ESP_OK) {
            goto error;
        }
    }

    err = i2c_master_stop(handle);
    if (err != ESP_OK) {
        goto error;
    }

    return handle;

error:
    i2c_cmd_link_delete_static(handle);
    return nullptr;
}

time_t get_localtime_midnight_in_utc(time_t timestamp)
{
    // Local time for timestamp
    struct tm tm;
    localtime_r(&timestamp, &tm);

    // Local time to today midnight
    tm.tm_hour  =  0;
    tm.tm_min   =  0;
    tm.tm_sec   =  0;
    tm.tm_isdst = -1; // isdst = -1 => let mktime figure out if DST is in effect

    // Return midnight in UTC
    return mktime(&tm);
}

Option<time_t> get_localtime_today_midnight_in_utc()
{
    struct timeval tv;
    if (!rtc.clock_synced(&tv))
        return {};

    return get_localtime_midnight_in_utc(tv.tv_sec);
}

void ensure_running_in_main_task(std::function<void(void)> &&fn) {
    if (running_in_main_task()) {
        fn();
    } else {
        task_scheduler.await_or_die(std::move(fn));
    }
}
