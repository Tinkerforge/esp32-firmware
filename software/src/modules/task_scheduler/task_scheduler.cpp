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

#include "event_log_prefix.h"
#include "main_dependencies.h"

static uint64_t last_task_id = 0;

Task::Task(std::function<void(void)> &&fn, uint64_t task_id, uint32_t first_run_delay_ms, uint32_t delay_ms, bool once) :
          fn(std::forward<std::function<void(void)>>(fn)),
          task_id(task_id),
          next_deadline_ms(millis() + first_run_delay_ms),
          delay_ms(delay_ms),
          awaited_by(nullptr),
          once(once),
          cancelled(false) {
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
    if (millis() > 0x7FFFFFFF) {
        // We are close to a timer overflow
        if (a->next_deadline_ms <= 0x7FFFFFFF && b->next_deadline_ms > 0x7FFFFFFF)
            // b is close to the overflow, a is behind the overflow
            return true;
        if (b->next_deadline_ms <= 0x7FFFFFFF && a->next_deadline_ms > 0x7FFFFFFF)
            // b is behind to the overflow, a is close to the overflow
            return false;
    }

    return a->next_deadline_ms >= b->next_deadline_ms;
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

void TaskScheduler::custom_loop()
{
    // We can't use defer to clean up currentTask on function level,
    // because we have to make sure currentTask is only written
    // while the task_mutex is locked.

    {
        std::lock_guard<std::mutex> l{this->task_mutex};
        if (tasks.empty()) {
            return;
        }

        if (!deadline_elapsed(tasks.top()->next_deadline_ms)) {
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

    // Run task without holding the lock.
    // This allows a task to schedule tasks (could also be done with a recursive mutex)
    // but also allows other threads to schedule tasks while one is executed.
    if (!this->currentTask->fn) {
        logger.printfln("Invalid task");
    } else {
        this->currentTask->fn();
    }

    {
        std::lock_guard<std::mutex> l{this->task_mutex};
        defer {this->currentTask = nullptr;};

        if (this->currentTask->awaited_by != nullptr) {
            xTaskNotifyGive(this->currentTask->awaited_by);
            this->currentTask->awaited_by = nullptr;
        }

        if (this->currentTask->once) {
            return;
        }

        // Check whether a repeated task was cancelled while it was being executed.
        if (this->currentTask->cancelled) {
            return;
        }

        this->currentTask->next_deadline_ms = millis() + this->currentTask->delay_ms;

        tasks.push(std::move(this->currentTask));
    }
}

uint64_t TaskScheduler::scheduleOnce(std::function<void(void)> &&fn, uint32_t delay_ms)
{
    std::lock_guard<std::mutex> l{this->task_mutex};
    uint64_t task_id = ++last_task_id;
    tasks.emplace(new Task(std::forward<std::function<void(void)>>(fn), task_id, delay_ms, 0, true));
    return task_id;
}

uint64_t TaskScheduler::scheduleWithFixedDelay(std::function<void(void)> &&fn, uint32_t first_delay_ms, uint32_t delay_ms)
{
    std::lock_guard<std::mutex> l{this->task_mutex};
    uint64_t task_id = ++last_task_id;
    tasks.emplace(new Task(std::forward<std::function<void(void)>>(fn), task_id, first_delay_ms, delay_ms, false));
    return task_id;
}

uint64_t TaskScheduler::scheduleWhenClockSynced(std::function<void(void)> &&fn)
{
    // Check once per second if clock is synced,
    // cancel task if it is and then call
    // the user supplied function
    return this->scheduleWithFixedDelay([fn, this]() {
        struct timeval tv_now;
        if (clock_synced(&tv_now)) {
            this->cancel(this->currentTask->task_id);
            fn();
        }
    }, 0, 1000);
}

TaskScheduler::CancelResult TaskScheduler::cancel(uint64_t task_id)
{
    std::lock_guard<std::mutex> l{this->task_mutex};
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

    std::lock_guard<std::mutex> l{this->task_mutex};
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
        std::lock_guard<std::mutex> l{this->task_mutex};
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
    return await(scheduleOnce(std::forward<std::function<void(void)>>(fn), 0), millis_to_wait);
}
