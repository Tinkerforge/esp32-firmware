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

#include <forward_list>

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "module.h"
#include "modules/task_scheduler/task_scheduler.h"
#include "bindings/errors.h"

// The IO scheduler is driven by the main task. It only accepts one-shot
// jobs (scheduleOnce/await). There is deliberately no API to register
// self-rescheduling (periodic) tasks on it. The IO task runs at a higher
// priority than the main loop on the same core, so a periodic task living
// on the IO scheduler could starve the main loop on its own.
class IoScheduler final : public IModule
{
public:
    IoScheduler() {}

    void pre_reboot() override;

    void start_task();

    uint64_t scheduleOnce(std::function<void(void)> &&fn, millis_t delay_ms = 0_ms, const std::source_location &src_location = std::source_location::current())
    {
        return scheduler.scheduleOnce(std::move(fn), delay_ms, src_location);
    }

    uint64_t driveUncancelable(std::function<bool(void)> &&before_io,
                               std::function<void(void)> &&during_io,
                               std::function<void(void)> &&after_io,
                               millis_t first_delay_ms,
                               millis_t delay_ms,
                               const std::source_location &src_location = std::source_location::current());

    [[nodiscard]] bool await(std::function<void(void)> &&fn, millis_t millis_to_wait = 5_s, const std::source_location &src_location = std::source_location::current())
    {
        if (task_handle == nullptr || xTaskGetCurrentTaskHandle() == task_handle) {
            fn();
            return true;
        }

        return scheduler.await(std::move(fn), millis_to_wait, src_location);
    }

    // Runs a Bricklet call that returns a tf_* result code
    int hal_call(std::function<int(void)> &&fn, const std::source_location &src_location = std::source_location::current())
    {
        int rc = TF_E_TIMEOUT;

        if (!await([&fn, &rc]() { rc = fn(); }, 5_s, src_location)) {
            return TF_E_TIMEOUT;
        }

        return rc;
    }

    void assert_hal_access_allowed() const
    {
        if (task_handle == nullptr) {
            return;
        }

        if (xTaskGetCurrentTaskHandle() != task_handle) {
            esp_system_abort("Bricklet HAL access from a task other than the IO task! Un-migrated call site?");
        }
    }

private:
    [[noreturn]] void task_loop();
    [[noreturn]] static void task_fn(void *arg);

    struct RoundState {
        std::function<bool(void)> before_io;
        std::function<void(void)> during_io;
        std::function<void(void)> after_io;
        std::source_location src_location;
        bool in_flight = false; // Only accessed on the main task.
    };

    std::function<void(void)> make_driver(std::function<bool(void)> &&before_io,
                                          std::function<void(void)> &&during_io,
                                          std::function<void(void)> &&after_io,
                                          const std::source_location &src_location);

    TaskScheduler scheduler;

    std::forward_list<RoundState> round_states;

    TaskHandle_t task_handle = nullptr;
    int watchdog_handle = -1;
};
