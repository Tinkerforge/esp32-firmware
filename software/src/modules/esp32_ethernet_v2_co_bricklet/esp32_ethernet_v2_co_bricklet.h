/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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
#include "build.h"
#include "bindings/bricklet_warp_esp32_ethernet_v2_co.h"
#include "modules/rtc/rtc.h"
#include "options.h"

#include "module_available.h"

class ESP32EthernetV2CoBricklet final : public DeviceModule<TF_WARPESP32EthernetV2Co,
                                              tf_warp_esp32_ethernet_v2_co_create,
                                              tf_warp_esp32_ethernet_v2_co_get_bootloader_mode,
                                              tf_warp_esp32_ethernet_v2_co_reset,
                                              tf_warp_esp32_ethernet_v2_co_destroy,
                                              OPTIONS_PRODUCT_ID_IS_WARP4()>,
                                        public IRtcBackend
{
public:
    ESP32EthernetV2CoBricklet();

    void setup() override;
    void pre_setup() override;
    void register_urls() override;

    void setup_co();

    // IRtcBackend implementation
    void set_time(const tm &time, int32_t microseconds) override;
    struct timeval get_time() override;
    void reset() override;

    void set_blue_led(const bool on);

    ConfigRoot state;
};

#include "module_available_end.h"
