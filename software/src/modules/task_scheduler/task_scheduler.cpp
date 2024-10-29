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

#include "task_scheduler.h"

#undef scheduleOnce
#undef scheduleWithFixedDelay
#undef scheduleWhenClockSynced
#undef scheduleWallClock
#undef await

#include "event_log_prefix.h"
#include "module_dependencies.h"

static uint64_t last_task_id = 0;

thread_local const char *_task_scheduler_file;
thread_local int _task_scheduler_line;

Task::Task(std::function<void(void)> &&fn, uint64_t task_id, micros_t first_run_delay, micros_t delay, const char *file, int line, bool once) :
        fn(std::move(fn)),
        task_id(task_id),
        next_deadline(now_us() + first_run_delay),
        delay(delay),
        awaited_by(nullptr),
        file(file),
        line(line),
        once(once),
        cancelled(false) {
}

WallClockTask::WallClockTask(std::unique_ptr<Task> &&runner_task, uint64_t task_id, minutes_t interval_minutes, bool run_on_first_sync) :
        runner_task(std::move(runner_task)),
        task_id(task_id),
        interval_minutes(interval_minutes),
        run_on_first_sync(run_on_first_sync) {

}

/*
A Compare type providing a strict weak ordering.

Note that the Compare parameter is defined such that it returns true
if its first argument comes before its second argument in a weak ordering.
But because the priority queue outputs largest elements first,
the elements that "come before" are actually output last.
That is, the front of the queue contains the "last" element
according to the weak ordering imposed by Compare.
(https://en.cppreference.com/w/cpp/container/priority_queue)
*/
bool compare(const std::unique_ptr<Task> &a, const std::unique_ptr<Task> &b)
{
    return a->next_deadline >= b->next_deadline;
}

// https://stackoverflow.com/a/36711682
bool TaskQueue::removeByTaskID(uint64_t task_id)
{
    // The queue is locked if this function is called.

    auto it = std::find_if(this->c.begin(), this->c.end(), [task_id](const std::unique_ptr<Task> &t){return t->task_id == task_id;});

    if (it == this->c.end()) {
        // not found
        return false;
    }

    if (it == this->c.begin()) {
        this->pop();
        return true;
    }

    // remove element and re-heap
    this->c.erase(it);
    std::make_heap(this->c.begin(), this->c.end(), this->comp);
    return true;
}

Task *TaskQueue::findByTaskID(uint64_t task_id)
{
    auto it = std::find_if(this->c.begin(), this->c.end(), [task_id](const std::unique_ptr<Task> &t){return t->task_id == task_id;});

    if (it == this->c.end()) {
        // not found
        return nullptr;
    }

    return it->get();
}

COREDUMP_RTC_DATA_ATTR const char *task_fn_file;
COREDUMP_RTC_DATA_ATTR int task_fn_line;

void TaskScheduler::custom_loop()
{
    // We can't use defer to clean up currentTask on function level,
    // because we have to make sure currentTask is only written
    // while the task_mutex is locked.

    {
        std::lock_guard<std::mutex> lock{this->task_mutex};
        if (tasks.empty()) {
            return;
        }

        if (!deadline_elapsed(tasks.top()->next_deadline)) {
            return;
        }

        this->currentTask = tasks.top_and_pop();

        if (this->currentTask->cancelled) {
            if (this->currentTask->awaited_by != nullptr) {
                xTaskNotifyGive(this->currentTask->awaited_by);
                this->currentTask->awaited_by = nullptr;
            }

            this->currentTask = nullptr;
            return;
        }
    }

    task_fn_file = this->currentTask->file;
    task_fn_line = this->currentTask->line;

    // Run task without holding the lock.
    // This allows a task to schedule tasks (could also be done with a recursive mutex)
    // but also allows other threads to schedule tasks while one is executed.
    if (!this->currentTask->fn) {
        logger.printfln("Invalid task");
    } else {
        this->currentTask->fn();
    }

    task_fn_file = nullptr;
    task_fn_line = 0;

    {
        std::lock_guard<std::mutex> lock{this->task_mutex};
        defer {this->currentTask = nullptr;};

        if (this->currentTask->awaited_by != nullptr) {
            xTaskNotifyGive(this->currentTask->awaited_by);
            this->currentTask->awaited_by = nullptr;
        }

        if (this->currentTask->once) {
            if (!IS_WALL_CLOCK_TASK_ID(this->currentTask->task_id))
                return;

            for (auto &wall_clock_task : this->wall_clock_tasks) {
                if (wall_clock_task.task_id != this->currentTask->task_id)
                    continue;
                wall_clock_task.runner_task = std::move(this->currentTask);
                return;
            }
            return;
        }

        // Check whether a repeated task was cancelled while it was being executed.
        if (this->currentTask->cancelled) {
            return;
        }

        this->currentTask->next_deadline = now_us() + this->currentTask->delay;

        tasks.push(std::move(this->currentTask));
    }
}

uint64_t TaskScheduler::scheduleOnce(std::function<void(void)> &&fn, millis_t delay_ms)
{
    std::lock_guard<std::mutex> lock{this->task_mutex};
    uint64_t task_id = ++last_task_id;
    tasks.emplace(new Task(std::move(fn), task_id, delay_ms, 0_us, _task_scheduler_file, _task_scheduler_line, true));
    return task_id;
}

uint64_t TaskScheduler::scheduleWithFixedDelay(std::function<void(void)> &&fn, millis_t first_delay_ms, millis_t delay_ms)
{
    std::lock_guard<std::mutex> lock{this->task_mutex};
    uint64_t task_id = ++last_task_id;
    tasks.emplace(new Task(std::move(fn), task_id, first_delay_ms, delay_ms, _task_scheduler_file, _task_scheduler_line, false));
    return task_id;
}

uint64_t TaskScheduler::scheduleWhenClockSynced(std::function<void(void)> &&fn)
{
    // Check once per second if clock is synced,
    // cancel task if it is and then call
    // the user supplied function
    return this->scheduleWithFixedDelay([fn, this]() {
        struct timeval tv_now;
        if (rtc.clock_synced(&tv_now)) {
            this->cancel(this->currentTask->task_id);
            fn();
        }
    }, 0_ms, 1_s);
}

uint64_t TaskScheduler::scheduleWallClock(std::function<void(void)> &&fn, minutes_t interval_minutes, millis_t execution_delay_ms, bool run_on_first_sync)
{
    uint64_t task_id;
    {
        std::lock_guard<std::mutex> lock{this->task_mutex};
        task_id = ++last_task_id | (1ull << 63ull);
        auto runner_task = std::unique_ptr<Task>(new Task(std::move(fn), task_id, 0_us, execution_delay_ms, _task_scheduler_file, _task_scheduler_line, true));

        wall_clock_tasks.emplace_back(std::move(runner_task), task_id, interval_minutes, run_on_first_sync);
    }

    if (!wall_clock_worker_started) {
        wall_clock_worker_started = true;
        this->scheduleWithFixedDelay([this](){this->wall_clock_worker();}, 0_ms, 1_s); // TODO: measure how long the worker takes in the common case! Then decide oversampling interval.
    }

    return task_id;
}

TaskScheduler::CancelResult TaskScheduler::cancel(uint64_t task_id)
{
    std::lock_guard<std::mutex> lock{this->task_mutex};
    if (IS_WALL_CLOCK_TASK_ID(task_id)) {
        size_t i = 0;
        bool task_scheduled = false;
        for (; i < wall_clock_tasks.size(); ++i) {
            if (wall_clock_tasks[i].task_id != task_id)
                continue;
            task_scheduled = !(bool)(wall_clock_tasks[i].runner_task);
            break;
        }
        wall_clock_tasks.erase(wall_clock_tasks.begin() + i);
        if (!task_scheduled)
            return TaskScheduler::CancelResult::Cancelled;
    }

    if (this->currentTask && this->currentTask->task_id == task_id) {
        this->currentTask->cancelled = true;
        return TaskScheduler::CancelResult::WillBeCancelled;
    }
    else
        return tasks.removeByTaskID(task_id) ? TaskScheduler::CancelResult::Cancelled : TaskScheduler::CancelResult::NotFound;
}

uint64_t TaskScheduler::currentTaskId()
{
    // currentTaskId is intended to write a self-canceling task.
    // Don't allow other threads to cancel tasks without knowing their ID.
    if (!running_in_main_task()) {
        logger.printfln("Calling TaskScheduler::currentTask is only allowed in the main thread!");
        return 0;
    }

    std::lock_guard<std::mutex> lock{this->task_mutex};
    if (this->currentTask != nullptr)
        return this->currentTask->task_id;
    return 0;
}

TaskScheduler::AwaitResult TaskScheduler::await(uint64_t task_id, uint32_t millis_to_wait)
{
    if (millis_to_wait == 0) {
        logger.printfln("Calling TaskScheduler::await with millis_to_wait == 0 is not allowed. This is not scheduleOnce!");
        return TaskScheduler::AwaitResult::Error;
    }

    TaskHandle_t thisThread = xTaskGetCurrentTaskHandle();

    if (mainTaskHandle == thisThread) {
        logger.printfln("Calling TaskScheduler::await is not allowed in the main thread!");
        return TaskScheduler::AwaitResult::Error;
    }

    {
        std::lock_guard<std::mutex> lock{this->task_mutex};
        // The awaited task either
        // - is in the queue
        // - is currently running, i.e. not in the queue but in this->currentTask
        // - or was already executed, canceled or not yet created,
        //   i.e. not in the queue and not in this->currentTask
        Task *task = nullptr;

        if (this->currentTask != nullptr && this->currentTask->task_id == task_id)
            task = this->currentTask.get();
        else
            task = tasks.findByTaskID(task_id);

        if (task == nullptr)
            return TaskScheduler::AwaitResult::Done;

        if (!task->once) {
            logger.printfln("Calling TaskScheduler::await is not allowed for a non-single-shot task");
            return TaskScheduler::AwaitResult::Error;
        }

        if (task->awaited_by != nullptr) {
            logger.printfln("Task is already awaited by another thread!");
            return TaskScheduler::AwaitResult::Error;
        }

        xTaskNotifyStateClear(thisThread);
        task->awaited_by = thisThread;
    }

    if (ulTaskNotifyTake(true, pdMS_TO_TICKS(millis_to_wait)) == 0) {
        switch (this->cancel(task_id)) {
            case TaskScheduler::CancelResult::WillBeCancelled:
                esp_system_abort("Awaited task timed out and can't be cancelled. Giving up.");
                return TaskScheduler::AwaitResult::Timeout;
            case TaskScheduler::CancelResult::Cancelled:
                return TaskScheduler::AwaitResult::Timeout;
            case TaskScheduler::CancelResult::NotFound:
                return TaskScheduler::AwaitResult::Done;
        }
    }

    return TaskScheduler::AwaitResult::Done;
}

TaskScheduler::AwaitResult TaskScheduler::await(std::function<void(void)> &&fn, uint32_t millis_to_wait)
{
    return await(scheduleOnce(std::move(fn), 0_ms), millis_to_wait);
}

void TaskScheduler::wall_clock_worker() {
    static int last_minute = -1;

    timeval tv;
    if (!rtc.clock_synced(&tv))
        return;

    tm time_struct;
    gmtime_r(&tv.tv_sec, &time_struct);

    if (time_struct.tm_min == last_minute)
        return;

    auto minutes_since_midnight = minutes_t{time_struct.tm_hour * 60 + time_struct.tm_min};

    std::lock_guard<std::mutex> lock{this->task_mutex};
    auto now = now_us();

    for (auto &task : wall_clock_tasks) {
        if (last_minute == -1 && !task.run_on_first_sync)
            continue;

        if (last_minute != -1 && (minutes_since_midnight % task.interval_minutes) != 0_m)
            continue;

        if(!task.runner_task) {
            logger.printfln("Attempted to schedule WallClockTask execution but runner_task is invalid. Is this task still enqueued?");
            logger.printfln("    task_id=%llu interval_minutes=%u run_on_first_sync=%d", task.task_id, (uint32_t)(int64_t)task.interval_minutes, task.run_on_first_sync);
            continue;
        }

        task.runner_task->next_deadline = now + task.runner_task->delay;
        tasks.emplace(std::move(task.runner_task));
    }

    last_minute = time_struct.tm_min;
}

TaskScheduler *TaskScheduler::_task_scheduler_context(const char *f, int l) {
    _task_scheduler_file = f;
    _task_scheduler_line = l;
    return this;
}
