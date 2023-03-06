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

#include "web_server.h"

// Global definition here to match the declaration in task_scheduler.h.
TaskScheduler task_scheduler;

Task::Task(std::function<void(void)> fn, uint32_t first_run_delay_ms, uint32_t delay_ms, bool once) :
          fn(std::move(fn)),
          next_deadline_ms(millis() + first_run_delay_ms),
          delay_ms(delay_ms),
          once(once) {

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
bool compare(const Task *a, const Task *b)
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

void TaskScheduler::pre_setup()
{

}

void TaskScheduler::setup()
{
    initialized = true;
}

void TaskScheduler::register_urls()
{
}

void TaskScheduler::loop()
{
    std::unique_ptr<Task> task;

    {
        std::lock_guard<std::mutex> l{this->task_mutex};
        if(tasks.empty()) {
            return;
        }

        if(!deadline_elapsed(tasks.top()->next_deadline_ms)) {
            return;
        }

        task = std::unique_ptr<Task>(tasks.top());
        tasks.pop();
    }

    if (!task->fn) {
        logger.printfln("Invalid task");
    } else {
        task->fn();
    }

    if (task->once) {
        return;
    }

    task->next_deadline_ms = millis() + task->delay_ms;
    {
        std::lock_guard<std::mutex> l{this->task_mutex};
        tasks.push(task.release());
    }
}

void TaskScheduler::scheduleOnce(std::function<void(void)> &&fn, uint32_t delay_ms)
{
    std::lock_guard<std::mutex> l{this->task_mutex};
    tasks.emplace(new Task(fn, delay_ms, 0, true));
}

void TaskScheduler::scheduleWithFixedDelay(std::function<void(void)> &&fn, uint32_t first_delay_ms, uint32_t delay_ms)
{
    std::lock_guard<std::mutex> l{this->task_mutex};
    tasks.emplace(new Task(fn, first_delay_ms, delay_ms, false));
}
