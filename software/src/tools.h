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

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <new>
#include <mutex>
#include <esp_log.h>
#include <FS.h>
#include <driver/i2c.h>
#include <lwip/dns.h>
#include <TFTools/Micros.h>
#include <TFTools/Option.h>

#include "bindings/hal_common.h"

#define MACRO_NAME_TO_STRING(x) #x

// Indirection expands macro. See https://gcc.gnu.org/onlinedocs/gcc-3.4.3/cpp/Stringification.html
#define MACRO_VALUE_TO_STRING(x) MACRO_NAME_TO_STRING(x)

const char *tf_reset_reason();

bool a_after_b(uint32_t a, uint32_t b);
bool deadline_elapsed(uint32_t deadline_ms);

void read_efuses(uint32_t *ret_uid_num, char *ret_uid_str, char *ret_passphrase);

int check(int rc, const char *msg);

extern bool should_factory_reset_bricklets;
bool mount_or_format_spiffs();

int ensure_matching_firmware(TF_TFP *tfp, const char *name, const char *purpose, const uint8_t *firmware, size_t firmware_len, bool force);

int compare_version(uint8_t left_major, uint8_t left_minor, uint8_t left_patch, uint8_t left_beta /* 255 == no beta */, uint32_t left_timestamp,
                    uint8_t right_major, uint8_t right_minor, uint8_t right_patch, uint8_t right_beta /* 255 == no beta */, uint32_t right_timestamp);

bool for_file_in(const char *dir, bool (*callback)(File *open_file), bool skip_directories = true);

void remove_directory(const char *path);

bool is_in_subnet(IPAddress ip, IPAddress subnet, IPAddress to_check);
bool is_valid_subnet_mask(IPAddress subnet);

extern TaskHandle_t mainTaskHandle;
void set_main_task_handle();
inline bool running_in_main_task()
{
    return mainTaskHandle == xTaskGetCurrentTaskHandle();
}

void led_blink(int8_t led_pin, int interval, int blinks_per_interval, int off_time_ms);

uint16_t internet_checksum(const uint8_t *data, size_t length);

// Helper function to execute dns_gethostbyname in lwIP's TCPIP context.
err_t dns_gethostbyname_lwip_ctx(const char *hostname, ip_addr_t *addr, dns_found_callback found, void *callback_arg);
// Helper function to execute dns_gethostbyname_addrtype in lwIP's TCPIP context.
err_t dns_gethostbyname_addrtype_lwip_ctx(const char *hostname, ip_addr_t *addr, dns_found_callback found, void *callback_arg, u8_t dns_addrtype);

struct dns_gethostbyname_addrtype_lwip_ctx_async_data
{
    err_t err; // output
    ip_addr_t addr; // internal
    ip_addr_t *addr_ptr; // output
    std::function<void(dns_gethostbyname_addrtype_lwip_ctx_async_data *callback_arg)> found_callback; // input
    void *user; // input/output
};

void dns_gethostbyname_addrtype_lwip_ctx_async(const char *hostname,
                                               std::function<void(dns_gethostbyname_addrtype_lwip_ctx_async_data *callback_arg)> &&found_callback,
                                               dns_gethostbyname_addrtype_lwip_ctx_async_data *callback_arg,
                                               u8_t dns_addrtype);

void poke_localhost();

void trigger_reboot(const char *initiator, millis_t delay_ms = 0_ms);

time_t ms_until_datetime(int *year, int *month, int *day, int *hour, int *minutes, int *seconds);
time_t ms_until_time(int h, int m);

// Unchecked snprintf that returns size_t
[[gnu::format(__printf__, 2, 3)]]
size_t sprintf_u(char *buf, const char *fmt, ...);

// Unchecked snprintf that returns size_t
[[gnu::format(__printf__, 3, 4)]]
size_t snprintf_u(char *buf, size_t len, const char *fmt, ...);

// Unchecked vsnprintf that returns size_t
size_t vsnprintf_u(char *buf, size_t len, const char *fmt, va_list args);

class LogSilencer
{
public:
    LogSilencer();
    ~LogSilencer();

    vprintf_like_t old_fn;
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

void list_dir(fs::FS &fs, const char *dirname, uint8_t depth, uint8_t current_depth = 0);

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
int remove_separator(const char *const in, char *out);

// minimal C++11 allocator with debug output
template <class Tp>
struct DebugAlloc {
    typedef Tp value_type;
    DebugAlloc() = default;
    template <class T> DebugAlloc(const DebugAlloc<T>&) {}

    int counter = 0;

    Tp *allocate(std::size_t n)
    {
        n *= sizeof(Tp);
        printf("!!! %d allocating %u bytes (%u)\n", counter, n, sizeof(Tp));
        ++counter;
        return static_cast<Tp *>(::operator new(n));
    }
    void deallocate(Tp *p, std::size_t n)
    {
        printf("!!! %d deallocating %u bytes\n", counter, n * sizeof *p);
        ++counter;
        ::operator delete(p);
    }
};
template <class T, class U>
bool operator==(const DebugAlloc<T>&, const DebugAlloc<U>&) { return true; }
template <class T, class U>
bool operator!=(const DebugAlloc<T>&, const DebugAlloc<U>&) { return false; }

template <class Tp>
struct IRAMAlloc {
    typedef Tp value_type;
    IRAMAlloc() = default;
    template <class T> IRAMAlloc(const IRAMAlloc<T>&) {}

    Tp *allocate(std::size_t n)
    {
        return (Tp *) heap_caps_malloc(n * sizeof(Tp), MALLOC_CAP_32BIT);
    }
    void deallocate(Tp *p, std::size_t n)
    {
        heap_caps_free(p);
    }
};
template <class T, class U>
bool operator==(const IRAMAlloc<T>&, const IRAMAlloc<U>&) { return true; }
template <class T, class U>
bool operator!=(const IRAMAlloc<T>&, const IRAMAlloc<U>&) { return false; }

enum class BootStage {
    STATIC_INITIALIZATION,
    PRE_INIT,
    PRE_SETUP,
    SETUP,
    REGISTER_URLS,
    REGISTER_EVENTS,
    LOOP,
    PRE_REBOOT
};

extern BootStage boot_stage;

int strncmp_with_same_len(const char *left, const char *right, size_t right_len);

// C++17 adds this: https://en.cppreference.com/w/cpp/utility/as_const
template<class T>
constexpr const T& as_const(T& t) noexcept
{
    return t;
}

// This supports reading, writing and writing then reading a buffer from/to
// an I2C device. Call with a big enough command buffer (I2C_LINK_RECOMMENDED_SIZE(2))
// and use the returned handle with i2c_master_cmd_begin to actually send/receive data.
// The handle can be used multiple times to call i2c_master_cmd_begin.
// Free the handle with i2c_cmd_link_delete_static.
i2c_cmd_handle_t i2c_master_prepare_write_read_device(uint8_t device_address,
                                                      uint8_t *command_buffer, size_t command_buffer_size,
                                                      const uint8_t* write_buffer, size_t write_size,
                                                      uint8_t* read_buffer, size_t read_size);

time_t get_localtime_midnight_in_utc(time_t timestamp);
Option<time_t> get_localtime_today_midnight_in_utc();
