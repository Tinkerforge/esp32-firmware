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

#include <ctime>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "tools.h"
#include "musl_libc_timegm.h"

#include "gcc_warnings.h"

#define I2C_RTC_ADDRESS 0b1101000

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
// portTICK_PERIOD_MS expands to an old style cast.
static TickType_t i2c_timeout = 1000 / portTICK_PERIOD_MS;
#pragma GCC diagnostic pop

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
    rtc_cmd_handle = i2c_master_prepare_write_read_device(I2C_RTC_ADDRESS,
                                         rtc_cmd_buf, ARRAY_SIZE(rtc_cmd_buf),
                                         rtc_write_buf, ARRAY_SIZE(rtc_write_buf),
                                         rtc_read_buf, ARRAY_SIZE(rtc_read_buf));

    rtc_write_time_cmd_handle = i2c_master_prepare_write_read_device(I2C_RTC_ADDRESS,
                                         rtc_write_time_cmd_buf, ARRAY_SIZE(rtc_write_time_cmd_buf),
                                         rtc_write_time_write_buf, ARRAY_SIZE(rtc_write_time_write_buf),
                                         nullptr, 0);

    task_scheduler.scheduleWithFixedDelay([this](){
        // Enable battery switch-over in control register.
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_RTC_ADDRESS << 1) | I2C_MASTER_WRITE, 1)); // expect ack
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x02, 1));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0b00000000, 1));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        auto errRc = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, i2c_timeout);
        i2c_cmd_link_delete(cmd);
        if (errRc != 0) {
            logger.printfln("RTC write control reg failed: %d", errRc);
            return;
        }

        if (!initialized)
            rtc.register_backend(this);

        initialized = true; // FIXME: delayed initialization doesn't show in frontend
    }, 0, 60 * 1000);
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
    errRc = i2c_master_cmd_begin(I2C_MASTER_PORT, rtc_write_time_cmd_handle, i2c_timeout);
    if (errRc != 0) {
        logger.printfln("RTC write failed: %d", errRc);
    }
}

struct timeval WarpEsp32Rtc::get_time()
{
    struct timeval time;
    time.tv_sec = 0;
    time.tv_usec = 0;

    auto ret = i2c_master_cmd_begin(I2C_MASTER_PORT, rtc_cmd_handle, i2c_timeout);

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
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_RTC_ADDRESS << 1) | I2C_MASTER_WRITE, 1)); // expect ack
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, 1));

    // For a software reset, 01011000 (58h) must be sent to register Control_1
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0b01011000, 1));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    auto errRc = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, i2c_timeout);
    if (errRc != 0) {
        logger.printfln("RTC write control reg failed: %d", errRc);
    }
    i2c_cmd_link_delete(cmd);
}
