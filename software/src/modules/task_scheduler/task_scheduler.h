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

#include <Arduino.h>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <time.h>
#include <iostream>

#include "module.h"
#include "tools.h"

struct Task {
    std::function<void(void)> fn;
    uint64_t task_id;
    uint32_t next_deadline_ms;
    uint32_t delay_ms;
    TaskHandle_t awaited_by;
    bool once;
    bool cancelled;

    Task(std::function<void(void)> &&fn, uint64_t task_id, uint32_t first_run_delay_ms, uint32_t delay_ms, bool once);
};

bool compare(const std::unique_ptr<Task> &a, const std::unique_ptr<Task> &b);

class TaskQueue : public std::priority_queue<std::unique_ptr<Task>, std::vector<std::unique_ptr<Task>>, decltype(&compare)>
{
    using std::priority_queue<std::unique_ptr<Task>, std::vector<std::unique_ptr<Task>>, decltype(&compare)>::priority_queue;

public:
    bool removeByTaskID(uint64_t task_id);
    Task *findByTaskID(uint64_t task_id);

    std::unique_ptr<Task> top_and_pop()
    {
        std::pop_heap(c.begin(), c.end(), comp);
        std::unique_ptr<Task> value = std::move(c.back());
        c.pop_back();
        return value;
    }
};

class TaskScheduler final : public IModule
{
public:
    TaskScheduler() : tasks(&compare) {}

    void custom_loop();
    uint64_t currentTaskId();

    enum class CancelResult {
        // Task not found in task queue
        NotFound,
        // Task found in and removed from task queue
        Cancelled,
        // Task is currently being executed. Flagged to cancel
        // before being inserted into task queue again.
        // Don't remove task resources yet!
        // A well-written single shot task will remove its resources
        // To remove a repeated task's resources, schedule a task
        // (that will be executed after the currently executed task in any case)
        WillBeCancelled
    };

    CancelResult cancel(uint64_t task_id);
    uint64_t scheduleOnce(std::function<void(void)> &&fn, uint32_t delay_ms);
    uint64_t scheduleWithFixedDelay(std::function<void(void)> &&fn, uint32_t first_delay_ms, uint32_t delay_ms);
    uint64_t scheduleWhenClockSynced(std::function<void(void)> &&fn);

    enum class AwaitResult {
        Done,
        Timeout,
        Error
    };
    AwaitResult await(uint64_t task_id, uint32_t millis_to_wait = 10000);
    AwaitResult await(std::function<void(void)> &&fn, uint32_t millis_to_wait = 10000);

private:
    std::mutex task_mutex;
    TaskQueue tasks;
    std::unique_ptr<Task> currentTask = nullptr;
};
