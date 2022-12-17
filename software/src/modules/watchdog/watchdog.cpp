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

#include "tools.h"

#define WATCHDOG_MAX_REGS 10
#define WATCHDOG_STACK_SIZE 1000

static StaticTask_t xTaskBuffer;
static StackType_t xStack[ WATCHDOG_STACK_SIZE ];
static TaskHandle_t xTask;

struct watchdog_reg {
    const char *name = nullptr;
    const char *message = nullptr;
    uint32_t last_reset = 0;
    uint32_t timeout = 0;
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
            for(int i = 0; i < regs_used; ++i) {
                if (regs[i].timeout == 0)
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
    // It is then save to give the task max priority
    xTask = xTaskCreateStaticPinnedToCore(
        watchdog_task,
        "watchdog_task",
        WATCHDOG_STACK_SIZE,
        nullptr,
        ESP_TASK_PRIO_MAX,
        xStack,
        &xTaskBuffer,
        1);
}

void Watchdog::setup()
{
    initialized = true;
}

void Watchdog::register_urls()
{

}

void Watchdog::loop()
{

}

int Watchdog::add(const char *name, const char *message, uint32_t timeout_ms)
{
    // We can't watchdogs to trigger within the first five minutes after start-up,
    // as the uptime tracker writes only after five minutes.
    // The easiest way to ensure this is to disallow high frequency watchdogs completely.
    // This also makes sure that we don't reboot too often, giving the user a chance to connect
    // to the web interface to diagnose and potentially fix the issue.
    if (timeout_ms < 5 * 60 * 1000) {
        logger.printfln("Can't register %s to watchdog: Timeout %u not allowed as it is less than 5 minutes", name, timeout_ms);
    }

    std::lock_guard<std::mutex> l{regs_mutex};
    if (regs_used >= WATCHDOG_MAX_REGS) {
        logger.printfln("Can't register %s to watchdog: All registrations used.", name);
        return -1;
    }

    regs[regs_used].name = name;
    regs[regs_used].message = message;
    regs[regs_used].timeout = timeout_ms;
    regs[regs_used].last_reset = millis();
    ++regs_used;
    return regs_used - 1;
}

void Watchdog::reset(int handle)
{
    std::lock_guard<std::mutex> l{regs_mutex};
    if (handle >= regs_used || handle < 0) {
        return;
    }

    regs[handle].last_reset = millis();
}
