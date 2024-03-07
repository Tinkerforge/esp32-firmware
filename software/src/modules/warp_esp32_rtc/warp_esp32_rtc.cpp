/* esp32-firmware
 * Copyright (C) 2024 Erik Fleckstein <erik@tinkerforge.com>
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

#include "warp_esp32_rtc.h"
#include "module_dependencies.h"
#include "musl_libc_timegm.h"

#include "api.h"
#include "build.h"
#include <ctime>

#include "gcc_warnings.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
// portTICK_PERIOD_MS expands to an old style cast.
static TickType_t i2c_timeout = 1000 / portTICK_PERIOD_MS;
#pragma GCC diagnostic pop

bool WarpEsp32Rtc::update_system_time()
{
    // We have to make sure, we don't try to update the system clock
    // while NTP also sets the clock.
    // To prevent this, we skip updating the system clock if NTP
    // did update it while we were fetching the current time from the RTC.

    uint32_t count;
    {
        std::lock_guard<std::mutex> lock{ntp.mtx};
        count = ntp.sync_counter;
    }

    struct timeval t = this->get_time();
    if (t.tv_sec == 0 && t.tv_usec == 0)
        return false;

    {
        std::lock_guard<std::mutex> lock{ntp.mtx};
        if (count != ntp.sync_counter)
            // NTP has just updated the system time. We assume that this time is more accurate the the RTC's.
            return false;

        settimeofday(&t, nullptr);
        ntp.set_synced();
    }
    return true;
}

static uint8_t intToBCD(uint8_t num) {
	return static_cast<uint8_t>(((num / 10) << 4) | (num % 10));
}


static uint8_t bcdToInt(uint8_t bcd) {
	return static_cast<uint8_t>(((bcd >> 4) * 10) + (bcd & 0x0f));
}

static i2c_cmd_handle_t rtc_cmd_handle;
static uint8_t rtc_cmd_buf[I2C_LINK_RECOMMENDED_SIZE(2)] = {};
static uint8_t rtc_write_buf[1] = {0x03};
static uint8_t rtc_read_buf[7] = {};

static i2c_cmd_handle_t rtc_write_time_cmd_handle;
static uint8_t rtc_write_time_cmd_buf[I2C_LINK_RECOMMENDED_SIZE(1)] = {};
static uint8_t rtc_write_time_write_buf[8] = {};

void WarpEsp32Rtc::setup()
{
    if (!esp32_ethernet_brick.is_warp_esp_ethernet_brick)
        return;

    rtc_cmd_handle = i2c_master_prepare_write_read_device(I2C_RTC_ADDRESS,
                                         rtc_cmd_buf, ARRAY_SIZE(rtc_cmd_buf),
                                         rtc_write_buf, ARRAY_SIZE(rtc_write_buf),
                                         rtc_read_buf, ARRAY_SIZE(rtc_read_buf));

    rtc_write_time_cmd_handle = i2c_master_prepare_write_read_device(I2C_RTC_ADDRESS,
                                         rtc_write_time_cmd_buf, ARRAY_SIZE(rtc_write_time_cmd_buf),
                                         rtc_write_time_write_buf, ARRAY_SIZE(rtc_write_time_write_buf),
                                         nullptr, 0);


    // Enable battery switch-over in control register.
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_RTC_ADDRESS << 1) | I2C_MASTER_WRITE, 1)); // expect ack
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x02, 1));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0b00000000, 1));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    auto errRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, i2c_timeout);
    if (errRc != 0) {
        logger.printfln("RTC write control reg failed: %d", errRc);
    }
    i2c_cmd_link_delete(cmd);

    initialized = true;
}

void WarpEsp32Rtc::register_urls() {
    if (!esp32_ethernet_brick.is_warp_esp_ethernet_brick)
        return;

    rtc.register_backend(this);

    api.addCommand("scratch/foo", Config::Null(), {}, [this](){
        timeval t;
        t.tv_sec = 1678175475;
        t.tv_usec = 0;
        this->set_time(t);
    }, true);
}

void WarpEsp32Rtc::set_time(const tm &date_time)
{
    // Register 3 is the first time and date register.
    rtc_write_time_write_buf[0] = 0x03;
    rtc_write_time_write_buf[1] = intToBCD(static_cast<uint8_t>(date_time.tm_sec));
    rtc_write_time_write_buf[2] = intToBCD(static_cast<uint8_t>(date_time.tm_min));
    rtc_write_time_write_buf[3] = intToBCD(static_cast<uint8_t>(date_time.tm_hour));
    rtc_write_time_write_buf[4] = intToBCD(static_cast<uint8_t>(date_time.tm_mday));
    rtc_write_time_write_buf[5] = intToBCD(static_cast<uint8_t>(date_time.tm_wday));
    rtc_write_time_write_buf[6] = intToBCD(static_cast<uint8_t>(date_time.tm_mon + 1));
    rtc_write_time_write_buf[7] = intToBCD(static_cast<uint8_t>(date_time.tm_year - 100));

    esp_err_t errRc;
    errRc = i2c_master_cmd_begin(I2C_NUM_0, rtc_write_time_cmd_handle, i2c_timeout);
    if (errRc != 0) {
        logger.printfln("RTC write failed: %d", errRc);
    }
}

void WarpEsp32Rtc::set_time(const timeval &time)
{
    struct tm date_time;
    gmtime_r(&time.tv_sec, &date_time);

    set_time(date_time);
}

struct timeval WarpEsp32Rtc::get_time()
{
    struct timeval time;
    time.tv_sec = 0;
    time.tv_usec = 0;

    auto ret = i2c_master_cmd_begin(I2C_NUM_0, rtc_cmd_handle, i2c_timeout);

    if (ret != ESP_OK) {
        return time;
    }
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    tm.tm_sec  = bcdToInt(rtc_read_buf[0]);
    tm.tm_min  = bcdToInt(rtc_read_buf[1]);
    tm.tm_hour = bcdToInt(rtc_read_buf[2]);
    tm.tm_mday = bcdToInt(rtc_read_buf[3]);
    tm.tm_wday = bcdToInt(rtc_read_buf[4]);
    tm.tm_mon  = bcdToInt(rtc_read_buf[5]) - 1; // 0-11. The month on the PCF8523 is 1-12.
    tm.tm_year = bcdToInt(rtc_read_buf[6]) + 100; // Years since 1900

    time.tv_sec = timegm(&tm);
    time.tv_usec = 0;

    // Allow time to be 24h older than the build timestamp,
    // in case the RTC is set by hand to test something.
    // FIXME not Y2038-safe
    if (time.tv_sec < static_cast<time_t>(build_timestamp() - 24 * 3600)) {
        time.tv_sec = 0;
        time.tv_usec = 0;
    }

    return time;
}

void WarpEsp32Rtc::reset()
{
    // Enable battery switch-over in control register.
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_RTC_ADDRESS << 1) | I2C_MASTER_WRITE, 1)); // expect ack
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, 1));

    // For a software reset, 01011000 (58h) must be sent to register Control_1
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0b01011000, 1));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    auto errRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, i2c_timeout);
    if (errRc != 0) {
        logger.printfln("RTC write control reg failed: %d", errRc);
    }
    i2c_cmd_link_delete(cmd);
}
