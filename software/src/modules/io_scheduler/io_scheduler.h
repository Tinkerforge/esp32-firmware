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

#include <atomic>
#include <memory>
#include <type_traits>
#include <utility>

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

    // Registers an uncancelable driven round.
    // before_io and after_io run on the main task, during_io runs on the IO task.
    // Pass nullptr for before_io/after_io if unused.
    template <typename BeforeIo, typename DuringIo, typename AfterIo>
    uint64_t driveUncancelable(BeforeIo &&before_io,
                               DuringIo &&during_io,
                               AfterIo &&after_io,
                               millis_t first_delay_ms,
                               millis_t delay_ms,
                               const std::source_location &src_location = std::source_location::current())
    {
        RoundStateBase *state = new RoundState<std::decay_t<BeforeIo>, std::decay_t<DuringIo>, std::decay_t<AfterIo>>(
            std::forward<BeforeIo>(before_io),
            std::forward<DuringIo>(during_io),
            std::forward<AfterIo>(after_io),
            src_location
        );

        return driveUncancelableImpl(state, first_delay_ms, delay_ms, src_location);
    }

    // Runs fn on the IO task and blocks until it has completed. The callable stays on the caller's stack.
    template <typename F>
    [[nodiscard]] bool await(F &&fn, millis_t millis_to_wait = 5_s, const std::source_location &src_location = std::source_location::current())
    {
        if (task_handle == nullptr || xTaskGetCurrentTaskHandle() == task_handle) {
            fn();
            return true;
        }

        using Callable = std::remove_reference_t<F>;
        return await_impl(&IoScheduler::invoke_callable<Callable>, const_cast<void *>(static_cast<const void *>(std::addressof(fn))), millis_to_wait, src_location);
    }

    // Runs a Bricklet call that returns a tf_* result code
    template <typename F>
    int hal_call(F &&fn, const std::source_location &src_location = std::source_location::current())
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

    struct RoundStateBase {
        const std::source_location src_location;
        bool in_flight = false; // Only accessed on the main task.

        RoundStateBase(const std::source_location &src_location_) : src_location(src_location_) {}

        virtual bool run_before_io() = 0;
        virtual void run_during_io() = 0;
        virtual void run_after_io() = 0;

    protected:
        ~RoundStateBase() = default; // Round states are never destroyed.
    };

    // Stores the callables of a driven round in a single allocation.
    template <typename BeforeIo, typename DuringIo, typename AfterIo>
    struct RoundState final : public RoundStateBase {
        BeforeIo before_io;
        DuringIo during_io;
        AfterIo after_io;

        RoundState(BeforeIo before_io_, DuringIo during_io_, AfterIo after_io_, const std::source_location &src_location_) :
            RoundStateBase(src_location_),
            before_io(std::move(before_io_)),
            during_io(std::move(during_io_)),
            after_io(std::move(after_io_)) {}

        bool run_before_io() override
        {
            if constexpr (std::is_same_v<BeforeIo, std::nullptr_t>) {
                return true;
            } else {
                return before_io();
            }
        }

        void run_during_io() override
        {
            during_io();
        }

        void run_after_io() override
        {
            if constexpr (!std::is_same_v<AfterIo, std::nullptr_t>) {
                after_io();
            }
        }
    };

    // Handoff content for await(). Lives on the awaiting task's stack.
    struct AwaitRequest {
        void (*invoke)(void *callable);
        void *callable;
        TaskHandle_t caller;
        bool executed = false; // Written by the IO task before notifying the caller.
    };

    template <typename Callable>
    static void invoke_callable(void *callable)
    {
        (*static_cast<Callable *>(callable))();
    }

    uint64_t driveUncancelableImpl(RoundStateBase *state, millis_t first_delay_ms, millis_t delay_ms, const std::source_location &src_location);
    void drive(RoundStateBase *state);
    bool await_impl(void (*invoke)(void *callable), void *callable, millis_t millis_to_wait, const std::source_location &src_location);

    TaskScheduler scheduler;

    std::atomic<AwaitRequest *> pending_await{nullptr};
    std::atomic<bool> rebooting{false};

    TaskHandle_t task_handle = nullptr;
    int watchdog_handle = -1;
};
