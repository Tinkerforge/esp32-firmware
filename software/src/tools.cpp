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

#include <Arduino.h>
#include <esp_efuse.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <soc/efuse_reg.h>

#include "event_log_prefix.h"
#include "main_dependencies.h"
#include "bindings/base58.h"

const char *tf_reset_reason()
{
    esp_reset_reason_t reason = esp_reset_reason();

    switch (reason) {
        case ESP_RST_POWERON:
            return "Reset due to power-on.";

        case ESP_RST_EXT:
            return "Reset by external pin.";

        case ESP_RST_SW:
            return "Software reset via esp_restart.";

        case ESP_RST_PANIC:
            return "Software reset due to exception/panic.";

        case ESP_RST_INT_WDT:
            return "Reset due to interrupt watchdog.";

        case ESP_RST_TASK_WDT:
            return "Reset due to task watchdog.";

        case ESP_RST_WDT:
            return "Reset due to some watchdog.";

        case ESP_RST_DEEPSLEEP:
            return "Reset after exiting deep sleep mode.";

        case ESP_RST_BROWNOUT:
            return "Brownout reset.";

        case ESP_RST_SDIO:
            return "Reset over SDIO.";

        default:
            return "Reset reason unknown.";
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

void read_efuses(uint32_t *ret_uid_num, char *ret_uid_str, char *ret_passphrase)
{
    uint32_t blocks[8] = {0};

    for (int32_t block3Address = EFUSE_BLK3_RDATA0_REG, i = 0; block3Address <= EFUSE_BLK3_RDATA7_REG; block3Address += 4, ++i) {
        blocks[i] = esp_efuse_read_reg(EFUSE_BLK3, i);
    }

    uint32_t passphrase[4] = {0};

    /*
    EFUSE_BLK_3 is 256 bit (32 byte, 8 blocks) long and organized as follows:
    block 0:
        Custom MAC CRC + MAC bytes 0 to 2
    block 1:
        Custom MAC bytes 3 to 5
        byte 3 - Wifi passphrase chunk 0 byte 0
    block 2:
        byte 0 - Wifi passphrase chunk 0 byte 1
        byte 1 - Wifi passphrase chunk 0 byte 2
        byte 2 - Wifi passphrase chunk 1 byte 0
        byte 3 - Wifi passphrase chunk 1 byte 1
    block 3:
        ADC 1 calibration data
    block 4:
        ADC 2 calibration data
    block 5:
        byte 0 - Wifi passphrase chunk 1 byte 2
        byte 1 - Wifi passphrase chunk 2 byte 0
        byte 2 - Wifi passphrase chunk 2 byte 1
        byte 3 - Custom MAC Version
    block 6:
        byte 0 - Wifi passphrase chunk 2 byte 2
        byte 1 - Wifi passphrase chunk 3 byte 0
        byte 2 - Wifi passphrase chunk 3 byte 1
        byte 3 - Wifi passphrase chunk 3 byte 2
    block 7:
        UID
    */

    passphrase[0] = ((blocks[1] & 0xFF000000) >> 24) | ((blocks[2] & 0x0000FFFF) << 8);
    passphrase[1] = ((blocks[2] & 0xFFFF0000) >> 16) | ((blocks[5] & 0x000000FF) << 16);
    passphrase[2] = ((blocks[5] & 0x00FFFF00) >> 8)  | ((blocks[6] & 0x000000FF) << 16);
    passphrase[3] =  (blocks[6] & 0xFFFFFF00) >> 8;
    *ret_uid_num = blocks[7];

    char buf[7] = {0};

    for (int i = 0; i < 4; ++i) {
        if (i != 0) {
            ret_passphrase[i * 5 - 1] = '-';
        }

        tf_base58_encode(passphrase[i], buf);

        if (strnlen(buf, ARRAY_SIZE(buf)) != 4) {
            logger.printfln("efuse error: malformed passphrase!");
        } else {
            memcpy(ret_passphrase + i * 5, buf, 4);
        }
    }

    tf_base58_encode(*ret_uid_num, ret_uid_str);
}

int vprintf_dev_null(const char *fmt, va_list args)
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

int compare_version(uint8_t left_major, uint8_t left_minor, uint8_t left_patch, uint8_t left_beta /* 255 == no beta */, uint32_t left_timestamp,
                    uint8_t right_major, uint8_t right_minor, uint8_t right_patch, uint8_t right_beta /* 255 == no beta */, uint32_t right_timestamp) {
    if (left_major > right_major)
        return 1;

    if (left_major < right_major)
        return -1;

    if (left_minor > right_minor)
        return 1;

    if (left_minor < right_minor)
        return -1;

    if (left_patch > right_patch)
        return 1;

    if (left_patch < right_patch)
        return -1;

    if (left_beta == 255 && right_beta != 255)
        return 1;

    if (left_beta != 255 && right_beta == 255)
        return -1;

    if (left_beta > right_beta)
        return 1;

    if (left_beta < right_beta)
        return -1;

    if (left_timestamp > right_timestamp)
        return 1;

    if (left_timestamp < right_timestamp)
        return -1;

    return 0;
}

TaskHandle_t mainTaskHandle;
void set_main_task_handle()
{
    mainTaskHandle = xTaskGetCurrentTaskHandle();
}

void led_blink(int8_t led_pin, int interval_ms, int blinks_per_interval, int off_time_ms)
{
    if (led_pin < 0)
        return;

    int t_in_second = now_us().to<millis_t>().as<int64_t>() % interval_ms;
    if (off_time_ms != 0 && (interval_ms - t_in_second <= off_time_ms)) {
        digitalWrite(led_pin, 1);
        return;
    }

    // We want blinks_per_interval blinks and blinks_per_interval pauses between them. The off_time counts as pause.
    int state_count = ((2 * blinks_per_interval) - (off_time_ms != 0 ? 1 : 0));
    int state_interval = (interval_ms - off_time_ms) / state_count;
    bool led = (t_in_second / state_interval) % 2 != 0;

    digitalWrite(led_pin, led);
}

uint16_t internet_checksum_u16(const uint16_t *data, size_t word_count)
{
    uint32_t checksum = 0xffff;

    for (size_t i = 0; i < word_count; i++) {
        checksum += data[i];
    }

    uint32_t carry = checksum >> 16;
    checksum = (checksum & 0xFFFF) + carry;
    return ~checksum;
}

void trigger_reboot(const char *initiator, millis_t delay_ms)
{
    logger.printfln("Reboot requested by %s.", initiator);
    task_scheduler.scheduleOnce([]() {
        ESP.restart();
    }, delay_ms);
}

size_t sprintf_u(char *buf, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int res = vsprintf(buf, fmt, args);
    va_end(args);

    return res < 0 ? 0 : static_cast<size_t>(res);
}

size_t snprintf_u(char *buf, size_t len, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int res = vsnprintf(buf, len, fmt, args);
    va_end(args);

    return res < 0 ? 0 : static_cast<size_t>(res);
}

size_t vsnprintf_u(char *buf, size_t len, const char *fmt, va_list args)
{
    int res = vsnprintf(buf, len, fmt, args);

    return res < 0 ? 0 : static_cast<size_t>(res);
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
    for (int i = 0; i < s; ++i) {
        if (in[i] == ':')
            continue;
        out[written] = in[i];
        ++written;
    }
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
