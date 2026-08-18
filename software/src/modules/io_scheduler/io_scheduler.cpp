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

#define IO_SCHEDULER_STACK_SIZE 3584

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
static constexpr UBaseType_t tcpip_task_priority = ESP_TASK_TCPIP_PRIO;
static_assert(IO_SCHEDULER_TASK_PRIORITY_IDLE < httpd_task_priority, "IO scheduler IDLE priority must be below the httpd task priority");
static_assert(IO_SCHEDULER_TASK_PRIORITY_JOB > httpd_task_priority, "IO scheduler JOB priority must be above the httpd task priority");
static_assert(IO_SCHEDULER_TASK_PRIORITY_JOB < tcpip_task_priority, "IO scheduler JOB priority must be below the tiT (lwIP) task priority");
#ifdef CONFIG_MQTT_TASK_PRIORITY // Not defined on platforms whose sdkconfig predates the esp-mqtt task priority option (WARP1)
static constexpr UBaseType_t mqtt_task_priority = CONFIG_MQTT_TASK_PRIORITY;
static_assert(IO_SCHEDULER_TASK_PRIORITY_IDLE < mqtt_task_priority, "IO scheduler IDLE priority must be below the mqtt task priority");
static_assert(IO_SCHEDULER_TASK_PRIORITY_JOB > mqtt_task_priority, "IO scheduler JOB priority must be above the mqtt task priority");
#endif
#pragma GCC diagnostic pop

extern TF_HAL hal;

void IoScheduler::pre_reboot()
{
    rebooting = true;
    scheduler.pre_reboot();
}

uint64_t IoScheduler::driveUncancelableImpl(RoundStateBase *state, millis_t first_delay_ms, millis_t delay_ms, const std::source_location &src_location)
{
    // Runs on the main task with the cadence given to driveUncancelable.
    // [this, state] fits into std::function's inline buffer, so there is no allocation.
    return task_scheduler.scheduleUncancelable([this, state]() { this->drive(state); }, first_delay_ms, delay_ms, src_location);
}

// Runs on the main task. RoundStates are never freed, so the raw pointer stays valid.
void IoScheduler::drive(RoundStateBase *state)
{
    if (state->in_flight) {
        return;
    }

    if (!state->run_before_io()) {
        return;
    }

    state->in_flight = true;

    this->scheduleOnce([state]() {
        state->run_during_io();

        task_scheduler.scheduleOnce([state]() {
            state->run_after_io();
            state->in_flight = false;
        }, 0_ms, state->src_location);
    }, 0_ms, state->src_location);
}

bool IoScheduler::await_impl(void (*invoke)(void *callable), void *callable, millis_t millis_to_wait, const std::source_location &src_location)
{
    if (rebooting) {
        return false;
    }

    AwaitRequest request{invoke, callable, xTaskGetCurrentTaskHandle(), false};

    // No stale notification can be pending here. Every earlier await on this
    // task either consumed its notification or aborted.
    xTaskNotifyStateClear_nowarn(nullptr);

    TickType_t ticks_left = pdMS_TO_TICKS_nowarn(millis_to_wait.as<uint32_t>());

    // Post the request. If the slot is occupied by another awaiter's request,
    // retry until the deadline. Once the request is posted only the IO task
    // may remove it from the slot (or this task again, on timeout below).
    AwaitRequest *expected = nullptr;

    while (!pending_await.compare_exchange_strong(expected, &request)) {
        expected = nullptr;

        if (ticks_left == 0) {
            return false;
        }

        vTaskDelay(1);
        ticks_left--;
    }

    if (ulTaskNotifyTake_nowarn(true, ticks_left) == 0) {
        expected = &request;

        if (pending_await.compare_exchange_strong(expected, nullptr)) {
            // The IO task never picked up the request, fn was not and will not be executed.
            return false;
        }

        // The IO task picked up the request but did not signal completion in
        // time. fn and the request live on this task's stack, so returning now
        // would let the IO task access a dead stack frame. Abort, like
        // TaskScheduler::await does for awaited tasks that can't be cancelled.
        esp_system_abortf<192>("Awaited IO job %s:%lu timed out. Giving up.", src_location.file_name(), src_location.line());
    }

    return request.executed;
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

        AwaitRequest *request = pending_await.exchange(nullptr);

        if (request != nullptr) {
            vTaskPrioritySet(nullptr, IO_SCHEDULER_TASK_PRIORITY_JOB);

            if (!rebooting) {
                request->invoke(request->callable);
                request->executed = true;
            }

            // The request lives on the awaiting task's stack and must not be
            // accessed anymore once the caller was notified.
            xTaskNotifyGive_nowarn(request->caller);

            vTaskPrioritySet(nullptr, IO_SCHEDULER_TASK_PRIORITY_IDLE);
            continue; // Another awaiter might be waiting for the slot.
        }

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
