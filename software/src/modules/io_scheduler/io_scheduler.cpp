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

#include "io_scheduler.h"

#include <sdkconfig.h>
#include <esp_http_server.h>
#include <esp_task.h>

#include "bindings/hal_common.h"
#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "tools/freertos.h"

#include "gcc_warnings.h"

#define IO_SCHEDULER_STACK_SIZE 2560

// The IO task switches between two priorities (pinned to core 1 together
// with the main loop):
//
// - IDLE (2, above the main loop, below httpd's 5): HAL polling and idle
//   sleeping. Preemptible by CPU-heavy work like TLS handshakes for
//   arbitrarily long stretches. Nothing that runs at IDLE
//   priority has a deadline.
// - JOB (8, above httpd, below tiT/WiFi): Executing scheduled jobs
//   (driven rounds, awaits). A started Bricklet request always completes
//   without beeing starved.
//
// During a TLS handshake the main loop (prio 1) is preempted as well and
// stops feeding driven rounds onto this scheduler, so the handshake gets
// core 1 almost exclusively, like in pre-io_scheduler firmwares,
// but without starving in-flight Bricklet requests into timeouts.
#define IO_SCHEDULER_TASK_PRIORITY_IDLE 2
#define IO_SCHEDULER_TASK_PRIORITY_JOB 8
#define IO_SCHEDULER_TASK_CORE 1

// Compile-time checks of the priority assumptions.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wuseless-cast"
static constexpr httpd_config_t default_httpd_config = HTTPD_DEFAULT_CONFIG();
static constexpr UBaseType_t httpd_task_priority = default_httpd_config.task_priority;
static constexpr UBaseType_t mqtt_task_priority = CONFIG_MQTT_TASK_PRIORITY;
static constexpr UBaseType_t tcpip_task_priority = ESP_TASK_TCPIP_PRIO;
static_assert(IO_SCHEDULER_TASK_PRIORITY_IDLE < httpd_task_priority, "IO scheduler IDLE priority must be below the httpd task priority");
static_assert(IO_SCHEDULER_TASK_PRIORITY_IDLE < mqtt_task_priority, "IO scheduler IDLE priority must be below the mqtt task priority");
static_assert(IO_SCHEDULER_TASK_PRIORITY_JOB > httpd_task_priority, "IO scheduler JOB priority must be above the httpd task priority");
static_assert(IO_SCHEDULER_TASK_PRIORITY_JOB > mqtt_task_priority, "IO scheduler JOB priority must be above the mqtt task priority");
static_assert(IO_SCHEDULER_TASK_PRIORITY_JOB < tcpip_task_priority, "IO scheduler JOB priority must be below the tiT (lwIP) task priority");
#pragma GCC diagnostic pop

extern TF_HAL hal;

void IoScheduler::pre_reboot()
{
    scheduler.pre_reboot();
}

std::function<void(void)> IoScheduler::make_driver(std::function<bool(void)> &&before_io,
                                                   std::function<void(void)> &&during_io,
                                                   std::function<void(void)> &&after_io,
                                                   const std::source_location &src_location)
{
    // Since all drivers are uncancelable there is no need for management of the state.
    // forward_list nodes never move, so raw pointers to the states stay valid.
    RoundState *state = &round_states.emplace_front(RoundState{std::move(before_io), std::move(during_io), std::move(after_io), src_location});

    // Runs on the main task with the cadence given to driveUncancelable.
    return [this, state]() {
        if (state->in_flight) {
            return;
        }

        if (state->before_io && !state->before_io()) {
            return;
        }

        state->in_flight = true;

        this->scheduleOnce([state]() {
            state->during_io();

            task_scheduler.scheduleOnce([state]() {
                if (state->after_io) {
                    state->after_io();
                }

                state->in_flight = false;
            }, 0_ms, state->src_location);
        }, 0_ms, state->src_location);
    };
}

uint64_t IoScheduler::driveUncancelable(std::function<bool(void)> &&before_io,
                                        std::function<void(void)> &&during_io,
                                        std::function<void(void)> &&after_io,
                                        millis_t first_delay_ms,
                                        millis_t delay_ms,
                                        const std::source_location &src_location)
{
    return task_scheduler.scheduleUncancelable(make_driver(std::move(before_io), std::move(during_io), std::move(after_io), src_location), first_delay_ms, delay_ms, src_location);
}

void IoScheduler::task_fn(void *arg)
{
    static_cast<IoScheduler *>(arg)->task_loop();
}

void IoScheduler::task_loop()
{
    scheduler.setOwnerTask(xTaskGetCurrentTaskHandle());

    for (;;) {
#if MODULE_WATCHDOG_AVAILABLE()
        if (watchdog_handle >= 0) {
            watchdog.reset(watchdog_handle);
        }
#endif

        // Poll SPITFP. This should basically only be for callbacks.
        // Since this uses DMA to transfer the whole packet once the length is known this is nearly thread-safe.
        // If we increase the amount of callbacks in the future, we should increase the task prio once we get a non-zero byte in the tick
        // and decrease it again once we message is completely received.
        tf_hal_tick(&hal, 0);

        if (scheduler.timeUntilNextTask(1_ms) == 0_us) {
            // Run one due job with its Bricklet transactions to guaranteed
            // completion, then drop back to IDLE priority.
            vTaskPrioritySet(nullptr, IO_SCHEDULER_TASK_PRIORITY_JOB);
            scheduler.custom_loop();
            vTaskPrioritySet(nullptr, IO_SCHEDULER_TASK_PRIORITY_IDLE);
        } else {
            vTaskDelay(1);
        }
    }
}

void IoScheduler::start_task()
{
    if (task_handle != nullptr) {
        esp_system_abort("IO task already started");
    }

#if MODULE_WATCHDOG_AVAILABLE()
    watchdog_handle = watchdog.add("io_scheduler", "IO task blocked", 30_s, 0_ms, true);
#endif

    auto err = xTaskCreatePinnedToCore(
        task_fn,
        "io_scheduler",
        IO_SCHEDULER_STACK_SIZE,
        this,
        IO_SCHEDULER_TASK_PRIORITY_IDLE,
        &task_handle,
        IO_SCHEDULER_TASK_CORE);

    if (err != pdPASS_nowarn) {
        esp_system_abortf<48>("Failed to create IO task: %i", err);
    }

#if MODULE_DEBUG_AVAILABLE()
    debug.register_task(task_handle, IO_SCHEDULER_STACK_SIZE);
#endif
}
