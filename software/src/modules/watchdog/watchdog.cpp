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

#include "watchdog.h"

#include <esp_task.h>
#include <esp_task_wdt.h>
#include <mutex>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"

#define WATCHDOG_MAX_REGS 10
// Highest observed stack usage was 768.
#define WATCHDOG_STACK_SIZE 1024

static StaticTask_t xTaskBuffer;
static StackType_t xStack[WATCHDOG_STACK_SIZE];
static TaskHandle_t xTask;

struct watchdog_reg {
    const char *name = nullptr;
    const char *message = nullptr;
    micros_t last_reset = 0_us;
    micros_t timeout = 0_us;
    // This timeout must be reached before the watchdog logic starts checking for timeouts.
    // Prevents boot-loops if a watchdog is triggered immediately after booting.
    micros_t initial_deadline = 0_us;
};

static std::mutex regs_mutex{};
static watchdog_reg regs[WATCHDOG_MAX_REGS];
static size_t regs_used = 0;

void watchdog_task(void *arg)
{
    esp_task_wdt_add(xTask);
    for (;;) {
        {
            std::lock_guard<std::mutex> l{regs_mutex};
            for (int i = 0; i < regs_used; ++i) {
                // Timeout 0 means that this registration is not initialized or disabled.
                if (regs[i].timeout == 0_us)
                    continue;

                // Don't trigger the watchdog if the initial deadline has not been reached.
                if (!deadline_elapsed(regs[i].initial_deadline))
                    continue;

                if (!deadline_elapsed(regs[i].last_reset + regs[i].timeout))
                    continue;

                esp_system_abort(regs[i].message);
            }
        }
        esp_task_wdt_reset();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void Watchdog::pre_setup()
{
    // See https://docs.espressif.com/projects/esp-idf/en/v4.4.3/esp32/api-guides/performance/speed.html#choosing-application-task-priorities

    // Use a static task so that the watchdog is always able to start,
    // even if (for some reason) allocations fail.
    // Pin the task to core 1:
    // We dont want to disturb the built-in tasks on core 0 (i.e. WiFi, Bluetooth, TCP/IP, etc)
    // It is then safe to give the task max priority
    xTask = xTaskCreateStaticPinnedToCore(
        watchdog_task,
        "watchdog_task",
        WATCHDOG_STACK_SIZE,
        nullptr,
        ESP_TASK_PRIO_MAX - 1,
        xStack,
        &xTaskBuffer,
        1);
}

void Watchdog::setup()
{
    initialized = true;

#if MODULE_DEBUG_AVAILABLE()
    debug.register_task(xTask, WATCHDOG_STACK_SIZE);
#endif
}

int Watchdog::add(const char *name, const char *message, millis_t timeout, millis_t initial_deadline, bool force)
{
    // This makes sure that we don't reboot too often, giving the user a chance to connect
    // to the web interface to diagnose and potentially fix the issue that will trigger the watchdog.
    if (!force && initial_deadline < 30_min) {
        logger.printfln("Can't register %s to watchdog: Initial deadline %lu not allowed as it is less than 30 minutes", name, initial_deadline.as<uint32_t>());
        return -1;
    }

    // Some tasks (for example firmware updates via a crappy WiFi connection) can take quite some time.
    if (!force && timeout < 5_min) {
        logger.printfln("Can't register %s to watchdog: Timeout %lu not allowed as it is less than 5 minutes", name, timeout.as<uint32_t>());
        return -1;
    }

    std::lock_guard<std::mutex> l{regs_mutex};
    if (regs_used >= WATCHDOG_MAX_REGS) {
        logger.printfln("Can't register %s to watchdog: All registrations used.", name);
        return -1;
    }

    regs[regs_used].name = name;
    regs[regs_used].message = message;
    regs[regs_used].timeout = timeout;
    regs[regs_used].initial_deadline = initial_deadline;
    regs[regs_used].last_reset = now_us();
    ++regs_used;
    return regs_used - 1;
}

void Watchdog::reset(int handle)
{
    std::lock_guard<std::mutex> l{regs_mutex};
    if (handle >= regs_used || handle < 0) {
        return;
    }

    regs[handle].last_reset = now_us();
}

// When the ESP WDT triggers, store triggered task data in RTC memory that ends up in the core dump.
COREDUMP_RTC_DATA_ATTR char wdt_info[256];
static size_t wdt_info_len = 0;

static void wdt_msg_handler(void *, const char *msg)
{
    while (wdt_info_len < (sizeof(wdt_info) - 1) && *msg != '\0') {
        wdt_info[wdt_info_len++] = *msg++;
    }
    wdt_info[wdt_info_len] = '\0';
}

void esp_task_wdt_isr_user_handler()
{
    esp_task_wdt_print_triggered_tasks(wdt_msg_handler, nullptr, nullptr);
}
