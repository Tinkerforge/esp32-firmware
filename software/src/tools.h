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
#include <new>

#include "config.h"

#include "bindings/hal_common.h"

#include "esp_log.h"

#include "strong_typedef.h"

#define MACRO_NAME_TO_STRING(x) #x

// Indirection expands macro. See https://gcc.gnu.org/onlinedocs/gcc-3.4.3/cpp/Stringification.html
#define MACRO_VALUE_TO_STRING(x) MACRO_NAME_TO_STRING(x)

#ifdef __GNUC__
#define ATTRIBUTE_UNUSED __attribute__((unused))
#else
#define ATTRIBUTE_UNUSED
#endif

const char *tf_reset_reason();

bool a_after_b(uint32_t a, uint32_t b);
bool deadline_elapsed(uint32_t deadline_ms);

STRONG_INTEGER_TYPEDEF(int64_t, micros_t)

micros_t operator""_usec(unsigned long long int i);

micros_t now_us();
bool deadline_elapsed(micros_t deadline_us);

void read_efuses(uint32_t *ret_uid_num, char *ret_uid_str, char *ret_passphrase);

int check(int rc, const char *msg);

bool mount_or_format_spiffs(void);

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

    char *releaseOwnership(size_t *len) {
        char *p = const_cast<char *>(c_str());
        *len = length();

        // String::init is marked inline and cannot be called
        // from here. copy the body of String::init to here
        setSSO(false);
        setBuffer(nullptr);
        setCapacity(0);
        setLen(0);

        return p;
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

#ifndef HEAP_ALLOC_ARRAY_DEFINED
#define HEAP_ALLOC_ARRAY_DEFINED
template <typename T>
std::unique_ptr<T[]> heap_alloc_array(size_t n) {
    return std::unique_ptr<T[]>{new T[n]()};
}
#endif


class TFPSwap
{
public:
    TFPSwap(TF_TFP *tfp) :
        tfp(tfp),
        device(tfp->device),
        cb_handler(tfp->cb_handler)
    {
        tfp->device = nullptr;
        tfp->cb_handler = nullptr;
    }

    ~TFPSwap()
    {
        tfp->device = device;
        tfp->cb_handler = cb_handler;
    }

private:
    TF_TFP *tfp;
    void *device;
    TF_TFP_CallbackHandler cb_handler;
};

// Remove seperator for nfc tags
void remove_separator(const char * const in, char *out);

// minimal C++11 allocator with debug output
template <class Tp>
struct DebugAlloc {
    typedef Tp value_type;
    DebugAlloc() = default;
    template <class T> DebugAlloc(const DebugAlloc<T>&) {}

    int counter = 0;

    Tp* allocate(std::size_t n)
    {
        n *= sizeof(Tp);
        printf("!!! %d allocating %u bytes (%u)\n", counter, n, sizeof(Tp));
        ++counter;
        return static_cast<Tp*>(::operator new(n));
    }
    void deallocate(Tp* p, std::size_t n)
    {
        printf("!!! %d deallocating %u bytes\n", counter, n*sizeof*p);
        ++counter;
        ::operator delete(p);
    }
};
template <class T, class U>
bool operator==(const DebugAlloc<T>&, const DebugAlloc<U>&) { return true; }
template <class T, class U>
bool operator!=(const DebugAlloc<T>&, const DebugAlloc<U>&) { return false; }

enum class BootStage {
    STATIC_INITIALIZATION,
    PRE_INIT,
    PRE_SETUP,
    SETUP,
    REGISTER_URLS,
    REGISTER_EVENTS,
    LOOP
};

extern BootStage boot_stage;
