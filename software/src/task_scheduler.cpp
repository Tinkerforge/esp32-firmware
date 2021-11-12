/* esp32-lib
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

Task::Task(const char *task_name, std::function<void(void)> fn, uint32_t first_run_delay_ms, uint32_t delay_ms, bool once) :
          task_name(task_name),
          fn(std::move(fn)),
          next_deadline_ms(millis() + first_run_delay_ms),
          delay_ms(delay_ms),
          once(once) {

}

bool compare(const Task &a, const Task &b) {
    return a.next_deadline_ms >= b.next_deadline_ms;
}

void TaskScheduler::setup()
{
    initialized = true;
}

const char *current_scheduler_state = "init";
const char *current_scheduler_task = "init";

void TaskScheduler::register_urls()
{
    server.on("/scheduler/state", HTTP_GET, [](WebServerRequest request) {
        request.send(200, "text/html", String(current_scheduler_state).c_str());
    });

    server.on("/scheduler/task", HTTP_GET, [](WebServerRequest request) {
        request.send(200, "text/html", String(current_scheduler_task).c_str());
    });
}

void TaskScheduler::loop() {
    this->task_mutex.lock();
        current_scheduler_state = "checking for empty queue";
        if(tasks.empty()) {
            this->task_mutex.unlock();
            return;
        }
        current_scheduler_state = "top";
        auto &task_ref = tasks.top();
        current_scheduler_task = task_ref.task_name;
        if(!deadline_elapsed(task_ref.next_deadline_ms)) {
            this->task_mutex.unlock();
            current_scheduler_state = "not elapsed";
            return;
        }
        current_scheduler_state = "copying task";

        Task task = task_ref;
        tasks.pop();
    this->task_mutex.unlock();

    current_scheduler_state = "running task";

    if (!task.fn) {
        logger.printfln("Invalid task");
        delay(100);
        logger.printfln("task name is: %s!", task.task_name);
    } else
        task.fn();

    current_scheduler_state = "done running task";

    if(task.once) {
        current_scheduler_state = "task ran once";
        return;
    }
    current_scheduler_state = "pushing task";

    task.next_deadline_ms = millis() + task.delay_ms;
    {
        std::lock_guard<std::mutex> l{this->task_mutex};
        tasks.push(std::move(task));
    }

    current_scheduler_state = "end loop";
}


void TaskScheduler::scheduleOnce(const char *taskName, std::function<void(void)> &&fn, uint32_t delay){
    std::lock_guard<std::mutex> l{this->task_mutex};
    tasks.emplace(taskName, fn, delay, 0, true);
}

void TaskScheduler::scheduleWithFixedDelay(const char *taskName, std::function<void(void)> &&fn, uint32_t first_delay, uint32_t delay) {
    std::lock_guard<std::mutex> l{this->task_mutex};
    tasks.emplace(taskName, fn, first_delay, delay, false);
}

