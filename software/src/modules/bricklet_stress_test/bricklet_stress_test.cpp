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
#include "bindings/bricklet_nfc.h"
#include "bindings/bricklet_unknown.h"
#include "bindings/hal_common.h"
#include "tools/bricklets.h"

extern TF_HAL hal;

// #define TF_SPITFP_DEBUG_ERROR_DUMP
// Drain SPITFP error dumps recorded in src/bindings/spitfp.c
#ifdef TF_SPITFP_DEBUG_ERROR_DUMP
typedef struct {
    uint32_t time_us;
    char port;
    uint8_t kind; // 0 = frame error, 1 = checksum error
    uint8_t packet_len;
    uint8_t expected_checksum;
    uint8_t real_checksum;
    uint8_t used;
    uint8_t data[84]; // TF_PACKET_BUFFER_SIZE
} TF_SPITFPDebugDump;

extern "C" uint32_t tf_spitfp_debug_dump_count(void);
extern "C" bool tf_spitfp_debug_dump_get(uint32_t seq, TF_SPITFPDebugDump *out);

static void drain_spitfp_debug_dumps()
{
    static uint32_t consumed = 0;

    uint32_t produced = tf_spitfp_debug_dump_count();

    if (produced - consumed > 16) {
        logger.printfln("SPITFP_ERR %lu dumps lost", static_cast<unsigned long>(produced - consumed - 16));
        consumed = produced - 16;
    }

    while (consumed != produced) {
        TF_SPITFPDebugDump dump;
        bool valid = tf_spitfp_debug_dump_get(consumed, &dump);
        consumed++;

        if (!valid) {
            continue;
        }

        logger.printfln("SPITFP_ERR port=%c t=%lu type=%s len=%u cksum=%02X/%02X used=%u",
                        dump.port, static_cast<unsigned long>(dump.time_us),
                        dump.kind == 0 ? "frame" : "checksum",
                        dump.packet_len, dump.expected_checksum, dump.real_checksum, dump.used);

        char line[3 * 32 + 1];

        for (uint8_t offset = 0; offset < dump.used; offset += 32) {
            uint8_t count = static_cast<uint8_t>(std::min(32, dump.used - offset));
            char *p = line;

            for (uint8_t i = 0; i < count; ++i) {
                p += snprintf(p, 4, "%02X ", dump.data[offset + i]);
            }

            *(p > line ? p - 1 : p) = '\0';

            logger.printfln("SPITFP_ERR port=%c data[%02u]: %s", dump.port, offset, line);
        }
    }
}
#endif // TF_SPITFP_DEBUG_ERROR_DUMP

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

        if (device_id == TF_NFC_DEVICE_IDENTIFIER && nfc_uid == 0) {
            nfc_uid = uid_num;
            strncpy(nfc_uid_str, uid_str, sizeof(nfc_uid_str) - 1);
        }

        logger.printfln("Stressing Bricklet %s (device id %u, port %c)", uid_str, device_id, port_name);
    }

    state.get("device_count")->updateUint(device_count);

    initialized = true;

#ifdef TF_SPITFP_DEBUG_ERROR_DUMP
    task_scheduler.scheduleWithFixedDelay([]() {
        drain_spitfp_debug_dumps();
    }, 200_ms, 200_ms);
#endif

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

    if (nfc_uid != 0 && !nfc_mode_set) {
        // Simple mode makes the Bricklet continuously poll for tags.
        TF_TFP *tfp = tf_hal_get_tfp(&hal, &nfc_uid, nullptr, nullptr, false);

        if (tfp != nullptr) {
            TFPSwap swap(tfp);
            TF_NFC nfc;

            if (tf_nfc_create(&nfc, nfc_uid_str, &hal) == TF_E_OK) {
                const int rc = tf_nfc_set_mode(&nfc, TF_NFC_MODE_SIMPLE);

                if (rc == TF_E_OK) {
                    logger.printfln("NFC Bricklet %s set to simple mode", nfc_uid_str);
                    nfc_mode_set = true;
                } else {
                    logger.printfln("Setting NFC Bricklet %s to simple mode failed: error %i", nfc_uid_str, rc);
                    nfc_uid = 0; // Don't retry, don't poll.
                }

                tf_nfc_destroy(&nfc);
            }
        }
    }

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

        if (device_uids[i] == nfc_uid) {
            TF_NFC nfc;

            if (tf_nfc_create(&nfc, nfc_uid_str, &hal) != TF_E_OK) {
                errors++;
                continue;
            }

            uint8_t tag_id_data[10];
            const int nfc_rc = tf_nfc_simple_get_tag_id_low_level(&nfc, 0, nullptr, nullptr, tag_id_data, nullptr);
            calls++;

            if (nfc_rc != TF_E_OK) {
                errors++;

                if (errors <= 10) {
                    logger.printfln("simple_get_tag_id failed for UID %lu: error %i", static_cast<unsigned long>(nfc_uid), nfc_rc);
                }
            }

            tf_nfc_destroy(&nfc);
        }
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
