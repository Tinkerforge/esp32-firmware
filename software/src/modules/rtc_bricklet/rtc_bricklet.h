/* esp32-firmware
 * Copyright (C) 2022 - 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#pragma once

#include "device_module.h"
#include "config.h"
#include "bindings/bricklet_real_time_clock_v2.h"
#include "real_time_clock_v2_bricklet_firmware_bin.embedded.h"
#include "../rtc/rtc.h"

class RtcBricklet : public DeviceModule<TF_RealTimeClockV2,
                                        real_time_clock_v2_bricklet_firmware_bin_data,
                                        real_time_clock_v2_bricklet_firmware_bin_length,
                                        tf_real_time_clock_v2_create,
                                        tf_real_time_clock_v2_get_bootloader_mode,
                                        tf_real_time_clock_v2_reset,
                                        tf_real_time_clock_v2_destroy>, public IRtcBackend
{
public:
    RtcBricklet(): DeviceModule("rtc", "Real Time Clock 2.0", "Real Time Clock 2.0", std::bind(&RtcBricklet::setup_rtc, this)) {};
    void setup();

    void setup_rtc();
    void set_time(const timeval &time);
    void set_time(const tm &time);
    void update_system_time();
    void reset();
    struct timeval get_time();
};
