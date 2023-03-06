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

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "config.h"

#include "bindings/hal_common.h"

#include "esp_log.h"

#define MACRO_NAME_TO_STRING(x) #x

// Indirection expands macro. See https://gcc.gnu.org/onlinedocs/gcc-3.4.3/cpp/Stringification.html
#define MACRO_VALUE_TO_STRING(x) MACRO_NAME_TO_STRING(x)

const char *tf_reset_reason();

bool a_after_b(uint32_t a, uint32_t b);
bool deadline_elapsed(uint32_t deadline_ms);

void read_efuses(uint32_t *ret_uid_num, char *ret_uid_str, char *ret_passphrase);

int check(int rc, const char *msg);

bool mount_or_format_spiffs(void);

String read_config_version();

int ensure_matching_firmware(TF_TFP *tfp, const char *name, const char *purpose, const uint8_t *firmware, size_t firmware_len, EventLog *logger, bool force);

int compare_version(uint8_t left_major, uint8_t left_minor, uint8_t left_patch,
                    uint8_t right_major, uint8_t right_minor, uint8_t right_patch);

bool clock_synced(struct timeval *out_tv_now);

uint32_t timestamp_minutes();

bool for_file_in(const char *dir, bool (*callback)(File *open_file), bool skip_directories = true);

void remove_directory(const char *path);

bool is_in_subnet(IPAddress ip, IPAddress subnet, IPAddress to_check);
bool is_valid_subnet_mask(IPAddress subnet);

void led_blink(int8_t led_pin, int interval, int blinks_per_interval, int off_time_ms);

uint16_t internet_checksum(const uint8_t* data, size_t length);

void trigger_reboot(const char *initiator);

time_t ms_until_datetime(int *year, int *month, int *day, int *hour, int *minutes, int *seconds);
time_t ms_until_time(int h, int m);

class LogSilencer
{
public:
    LogSilencer();
    ~LogSilencer();

    vprintf_like_t old_fn;
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

// Arduino String that allows accessing more methods.
class CoolString : public String
{
public:
    void setLength(int len)
    {
        setLen(len);
    }

    unsigned int getCapacity() {
        return capacity();
    }

    void shrinkToFit() {
        changeBuffer(len());
    }
};

void list_dir(fs::FS &fs, const char * dirname, uint8_t depth, uint8_t current_depth = 0);

template <typename T>
T clamp(T min, T val, T max)
{
    if (val < min) {
        return min;
    }

    if (val > max) {
        return max;
    }

    return val;
}

// https://stackoverflow.com/a/42060129
#ifndef defer
struct defer_dummy {};
template <class F> struct deferrer { F f; ~deferrer() { f(); } };
template <class F> deferrer<F> operator*(defer_dummy, F f) { return {f}; }
#define DEFER_(LINE) zz_defer##LINE
#define DEFER(LINE) DEFER_(LINE)
#define defer auto DEFER(__LINE__) = defer_dummy{} *[&]()
#endif

class Ownership
{
public:
    Ownership() {};

    bool try_acquire(uint32_t owner_id);
    void release();
    uint32_t current();
    uint32_t next();

private:
    uint32_t current_owner_id = 0;
    std::mutex mutex;
};

class OwnershipGuard
{
public:
    OwnershipGuard(Ownership *ownership, uint32_t owner_id);
    ~OwnershipGuard();

    bool have_ownership();

private:
    Ownership *ownership;
    bool acquired;
};
