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
#include "bindings/bricklet_real_time_clock_v2.h"
#include "modules/rtc/rtc.h"

class RtcBricklet final : public DeviceModule<TF_RealTimeClockV2,
                                              tf_real_time_clock_v2_create,
                                              tf_real_time_clock_v2_get_bootloader_mode,
                                              tf_real_time_clock_v2_reset,
                                              tf_real_time_clock_v2_destroy,
                                              false>,
                          public IRtcBackend
{
public:
    RtcBricklet();

    void setup() override;

    void setup_rtc();

    // IRtcBackend implementation
    void set_time(const tm &time, int microseconds) override;
    struct timeval get_time() override;
    void reset() override;
};
