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

#include "warp_esp32_i2c.h"

#include "module_dependencies.h"

#include "gcc_warnings.h"

#define I2C_MASTER_SCL_IO 4
#define I2C_MASTER_SDA_IO 15

#define I2C_TMP1075N_ADDR 0b1001001

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
// portTICK_PERIOD_MS expands to an old style cast.
static TickType_t i2c_timeout = 1000 / portTICK_PERIOD_MS;
#pragma GCC diagnostic pop

static uint8_t tmp_cmd_buf[I2C_LINK_RECOMMENDED_SIZE(2)] = {};
static uint8_t tmp_read_buf[2] = {};

void WarpEsp32I2c::pre_setup()
{
    this->state = Config::Object({
        {"temperature", Config::Int(0, -8192, 12800)}
    });
}

void WarpEsp32I2c::setup()
{
    i2c_config_t conf;
    memset(&conf, 0, sizeof(i2c_config_t));
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;

    i2c_param_config(I2C_MASTER_PORT, &conf);
    i2c_driver_install(I2C_MASTER_PORT, conf.mode, 0, 0, 0);

    tmp_cmd_handle = i2c_master_prepare_write_read_device(I2C_TMP1075N_ADDR,
                                         tmp_cmd_buf, ARRAY_SIZE(tmp_cmd_buf),
                                         nullptr, 0,
                                         tmp_read_buf, ARRAY_SIZE(tmp_read_buf));

    initialized = true;

    task_scheduler.scheduleWithFixedDelay([this](){
        auto ret = i2c_master_cmd_begin(I2C_MASTER_PORT, tmp_cmd_handle, i2c_timeout);
        if(ret != ESP_OK)
            return;

        int temp = static_cast<int8_t>(tmp_read_buf[0]) << 4 | tmp_read_buf[1] >> 4; // Cast to int8_t for sign-extension.
        temp = temp * 25 / 4; // equivalent to * 6.25 centicelsius per LSB

        state.get("temperature")->updateInt(temp);
    }, 1000, 1000);
}

void WarpEsp32I2c::register_urls()
{
    api.addState("esp32/temperature", &state);
}
