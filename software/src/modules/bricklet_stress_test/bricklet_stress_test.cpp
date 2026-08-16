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

#include "bricklet_stress_test.h"

#include <TFTools/Micros.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"

#include "bindings/base58.h"
#include "bindings/bricklet_unknown.h"
#include "bindings/hal_common.h"
#include "tools/bricklets.h"

extern TF_HAL hal;

void BrickletStressTest::pre_setup()
{
    config = Config::Object({
        {"enable", Config::Bool(true)},
        {"interval_ms", Config::Uint(10, 1, 10000)},
    });

    state = Config::Object({
        {"device_count", Config::Uint32(0)},
        {"rounds", Config::Uint32(0)},
        {"calls", Config::Uint32(0)},
        {"errors", Config::Uint32(0)},
        {"last_round_us", Config::Uint32(0)},
        {"max_round_us", Config::Uint32(0)},
        {"hist_5ms", Config::Uint32(0)},
        {"hist_10ms", Config::Uint32(0)},
        {"hist_20ms", Config::Uint32(0)},
        {"hist_50ms", Config::Uint32(0)},
        {"hist_100ms", Config::Uint32(0)},
        {"hist_slow", Config::Uint32(0)},
    });
}

void BrickletStressTest::setup()
{
    api.restorePersistentConfig("bricklet_stress_test/config", &config);

    // Enumerate devices
    uint16_t index = 0;
    char uid_str[7] = {0};
    char port_name = '?';
    uint16_t device_id = 0;

    while (device_count < MAX_DEVICES && tf_hal_get_device_info(&hal, index, uid_str, &port_name, &device_id) == TF_E_OK) {
        index++;

        uint32_t uid_num = 0;
        if (tf_base58_decode(uid_str, &uid_num) != TF_E_OK) {
            logger.printfln("Could not decode UID %s", uid_str);
            continue;
        }

        device_uids[device_count] = uid_num;
        device_count++;

        logger.printfln("Stressing Bricklet %s (device id %u, port %c)", uid_str, device_id, port_name);
    }

    state.get("device_count")->updateUint(device_count);

    initialized = true;

    if (!config.get("enable")->asBool()) {
        logger.printfln("Disabled by config");
        return;
    }

    if (device_count == 0) {
        logger.printfln("No Bricklets found, nothing to stress");
        return;
    }

    const millis_t interval = millis_t{config.get("interval_ms")->asUint()};

    io_scheduler.driveUncancelable(
        nullptr,
        [this]() {
            run_round();
        },
        [this]() {
            state.get("rounds")->updateUint(rounds);
            state.get("calls")->updateUint(calls);
            state.get("errors")->updateUint(errors);
            state.get("last_round_us")->updateUint(last_round_us);
            state.get("max_round_us")->updateUint(max_round_us);
            state.get("hist_5ms")->updateUint(hist[0]);
            state.get("hist_10ms")->updateUint(hist[1]);
            state.get("hist_20ms")->updateUint(hist[2]);
            state.get("hist_50ms")->updateUint(hist[3]);
            state.get("hist_100ms")->updateUint(hist[4]);
            state.get("hist_slow")->updateUint(hist[5]);
        },
        interval, interval);
}

void BrickletStressTest::register_urls()
{
    api.addPersistentConfig("bricklet_stress_test/config", &config);
    api.addState("bricklet_stress_test/state", &state);
}

// Runs on the IO task.
void BrickletStressTest::run_round()
{
    const micros_t start = now_us();

    for (size_t i = 0; i < device_count; i++) {
        TF_TFP *tfp = tf_hal_get_tfp(&hal, &device_uids[i], nullptr, nullptr, false);
        if (tfp == nullptr) {
            errors++;
            continue;
        }

        // Temporarily detach a possibly bound device object.
        TFPSwap swap(tfp);
        TF_Unknown unknown;

        if (tf_unknown_create(&unknown, tfp) != TF_E_OK) {
            errors++;
            continue;
        }

        const int rc = tf_unknown_get_identity(&unknown, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        calls++;

        if (rc != TF_E_OK) {
            errors++;

            // Log the first few errors, then stay quiet to not distort the measurement with logging overhead.
            if (errors <= 10) {
                logger.printfln("get_identity failed for UID %lu: error %i", static_cast<unsigned long>(device_uids[i]), rc);
            }
        }

        tf_unknown_destroy(&unknown);
    }

    rounds++;

    const uint32_t duration_us = (now_us() - start).as<uint32_t>();
    last_round_us = duration_us;

    if (duration_us > max_round_us) {
        max_round_us = duration_us;
    }

    size_t bucket = 0;
    while (bucket < HIST_BUCKETS - 1 && duration_us >= HIST_BOUNDS_US[bucket]) {
        bucket++;
    }
    hist[bucket]++;
}
