/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include "config.h"
#include "module.h"

class BrickletStressTest final : public IModule
{
public:
    BrickletStressTest() {}

    void pre_setup() override;
    void setup() override;
    void register_urls() override;

private:
    void run_round();

    ConfigRoot config;
    ConfigRoot state;

    static constexpr size_t MAX_DEVICES = 16;
    uint32_t device_uids[MAX_DEVICES];
    size_t device_count = 0;

    uint32_t nfc_uid = 0;
    char nfc_uid_str[7] = {0};
    bool nfc_mode_set = false;

    static constexpr uint32_t HIST_BOUNDS_US[] = {5000, 10000, 20000, 50000, 100000};
    static constexpr size_t HIST_BUCKETS = (sizeof(HIST_BOUNDS_US) / sizeof(HIST_BOUNDS_US[0])) + 1;

    uint32_t rounds = 0;
    uint32_t calls = 0;
    uint32_t errors = 0;
    uint32_t last_round_us = 0;
    uint32_t max_round_us = 0;
    uint32_t hist[HIST_BUCKETS] = {0};
};
