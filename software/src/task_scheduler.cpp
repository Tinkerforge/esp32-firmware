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

extern WebServer server;

Task::Task(std::function<void(void)> fn, uint32_t first_run_delay_ms, uint32_t delay_ms, bool once) :
          fn(std::move(fn)),
          next_deadline_ms(millis() + first_run_delay_ms),
          delay_ms(delay_ms),
          once(once) {

}

bool compare(const Task &a, const Task &b)
{
    return a.next_deadline_ms >= b.next_deadline_ms;
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
    this->task_mutex.lock();
        if(tasks.empty()) {
            this->task_mutex.unlock();
            return;
        }
        const auto &task_ref = tasks.top();

        if(!deadline_elapsed(task_ref.next_deadline_ms)) {
            this->task_mutex.unlock();
            return;
        }

        Task task = task_ref;
        tasks.pop();
    this->task_mutex.unlock();

    if (!task.fn) {
        logger.printfln("Invalid task");
    } else {
        task.fn();
    }

    if (task.once) {
        return;
    }

    task.next_deadline_ms = millis() + task.delay_ms;
    {
        std::lock_guard<std::mutex> l{this->task_mutex};
        tasks.push(std::move(task));
    }
}

void TaskScheduler::scheduleOnce(std::function<void(void)> &&fn, uint32_t delay_ms)
{
    std::lock_guard<std::mutex> l{this->task_mutex};
    tasks.emplace(fn, delay_ms, 0, true);
}

void TaskScheduler::scheduleWithFixedDelay(std::function<void(void)> &&fn, uint32_t first_delay_ms, uint32_t delay_ms)
{
    std::lock_guard<std::mutex> l{this->task_mutex};
    tasks.emplace(fn, first_delay_ms, delay_ms, false);
}
