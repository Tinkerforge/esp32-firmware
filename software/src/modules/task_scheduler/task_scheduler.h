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
    micros_t next_deadline;
    micros_t delay;
    TaskHandle_t awaited_by;
    const char *file;
    int line;
    bool once;
    bool cancelled;

    Task(std::function<void(void)> &&fn, uint64_t task_id, micros_t first_run_delay, micros_t delay, const char *file, int line, bool once);
};

#define IS_WALL_CLOCK_TASK_ID(task_id) (task_id & (1ull << 63))

struct WallClockTask {
    // Is moved into the task queue to execute the WallClockTask.
    std::unique_ptr<Task> runner_task;
    // This is the runner_task's ID; duplicated to match currentTask against the WallClockTask IDs when moving the task back.
    // All WallClockTask IDs have the highest bit set.
    uint64_t task_id;

    // A WallClockTask is executed at midnight (UTC) and then once every interval_minutes.
    minutes_t interval_minutes;

    // Additionally run this task when the system clock is synced for the first time.
    bool run_on_first_sync;

    WallClockTask(std::unique_ptr<Task> &&runner_task, uint64_t task_id, minutes_t interval_minutes, bool run_on_first_sync);
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

    uint64_t scheduleOnce(std::function<void(void)> &&fn, millis_t delay_ms = 0_ms);

    // TODO Remove deprecated function. Marked as deprecated on 2024-10-09.
    [[gnu::deprecated("Use the millis_t overload of this function!")]]
    inline uint64_t scheduleOnce(std::function<void(void)> &&fn, uint32_t delay_ms) {return this->scheduleOnce(std::move(fn), millis_t{delay_ms});}

    inline uint64_t scheduleWithFixedDelay(std::function<void(void)> &&fn, millis_t delay_ms) {return this->scheduleWithFixedDelay(std::move(fn), millis_t{0}, delay_ms);}
    uint64_t scheduleWithFixedDelay(std::function<void(void)> &&fn, millis_t first_delay_ms, millis_t delay_ms);

    // TODO Remove deprecated function. Marked as deprecated on 2024-10-09.
    [[gnu::deprecated("Use the millis_t overload of this function!")]]
    inline uint64_t scheduleWithFixedDelay(std::function<void(void)> &&fn, uint32_t first_delay_ms, uint32_t delay_ms) {return this->scheduleWithFixedDelay(std::move(fn), millis_t{first_delay_ms}, millis_t{delay_ms});}

    uint64_t scheduleWhenClockSynced(std::function<void(void)> &&fn);

    uint64_t scheduleWallClock(std::function<void(void)> &&fn, minutes_t interval_minutes, millis_t execution_delay_ms, bool run_on_first_sync);

    enum class AwaitResult {
        Done,
        Timeout,
        Error
    };

    AwaitResult await(std::function<void(void)> &&fn, uint32_t millis_to_wait = 10000);

    TaskScheduler *_task_scheduler_context(const char *f, int l);

private:
    AwaitResult await(uint64_t task_id, uint32_t millis_to_wait = 10000);

    std::mutex task_mutex;
    TaskQueue tasks;
    std::unique_ptr<Task> currentTask = nullptr;

    std::vector<WallClockTask> wall_clock_tasks;
    bool wall_clock_worker_started = false;
    void wall_clock_worker();
    void run_wall_clock_task(uint64_t task_id);
};

#define scheduleOnce _task_scheduler_context(__FILE__, __LINE__)->scheduleOnce
#define scheduleWithFixedDelay _task_scheduler_context(__FILE__, __LINE__)->scheduleWithFixedDelay
#define scheduleWhenClockSynced _task_scheduler_context(__FILE__, __LINE__)->scheduleWhenClockSynced
#define scheduleWallClock _task_scheduler_context(__FILE__, __LINE__)->scheduleWallClock
#define await _task_scheduler_context(__FILE__, __LINE__)->await
