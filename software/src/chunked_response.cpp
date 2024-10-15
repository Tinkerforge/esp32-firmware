/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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

#include "chunked_response.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "event_log_prefix.h"
#include "main_dependencies.h"

void QueuedChunkedResponse::begin(bool success)
{
    call([this, success]{internal->begin(success); return true;});
}

void QueuedChunkedResponse::alive()
{
    call([this]{internal->alive(); return true;});
}

bool QueuedChunkedResponse::write_impl(const char *buf, size_t buf_size)
{
    return call([this, buf, buf_size]{return internal->write(buf, buf_size);});
}

void QueuedChunkedResponse::end(String error)
{
    {
        std::lock_guard<std::mutex> guard(mutex);

        if (has_ended) {
            return;
        }
    }

    call([this, &error]{internal->end(error); is_running = false; return true;});

    {
        std::lock_guard<std::mutex> guard(mutex);

        end_error = error;
        has_ended = true;
    }

    condition.notify_one();
}

String QueuedChunkedResponse::wait()
{
    std::unique_lock<std::mutex> lock(mutex, std::defer_lock_t()); // don't lock immediatly

    while (is_running) {
        lock.lock();

        if (!condition.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this]{return have_function;})) {
            String error = "condition timeout, no function";

            internal->end(error);

            end_error = error;
            has_ended = true;

            lock.unlock();
            condition.notify_one();

            return end_error;
        }

        result = function();
        have_result = true;

        function = nullptr;
        have_function = false;

        lock.unlock();
        condition.notify_one();
    }

    lock.lock();

    if (!condition.wait_for(lock, std::chrono::seconds(10), [this]{return has_ended;})) {
        end_error = "condition timeout, not ended";
    }

    lock.unlock();

    return end_error;
}

bool QueuedChunkedResponse::call(std::function<bool(void)> &&local_function)
{
    std::unique_lock<std::mutex> lock(mutex);

    if (!condition.wait_for(lock, std::chrono::seconds(10), [this]{return !have_result || has_ended;})) {
        lock.unlock();
        logger.printfln("Condition timeout, has result or not ended");

        return false;
    }

    if (has_ended) {
        lock.unlock();

        return false;
    }

    function = std::move(local_function);
    have_function = true;

    lock.unlock();
    condition.notify_one();

    lock.lock();

    if (!condition.wait_for(lock, std::chrono::seconds(10), [this]{return have_result || has_ended;})) {
        lock.unlock();
        logger.printfln("Condition timeout, no result or did not ended");

        return false;
    }

    if (has_ended) {
        lock.unlock();

        return false;
    }

    bool local_result = result;
    have_result = false;

    lock.unlock();

    return local_result;
}



void BufferedChunkedResponse::begin(bool success)
{
    internal->begin(success);
}

void BufferedChunkedResponse::alive()
{
    internal->alive();
}

bool BufferedChunkedResponse::write_impl(const char *buf, size_t buf_size)
{
    if (buf_size <= pending_free()) {
        // buffer can be stored fully
        memcpy(pending_ptr(), buf, buf_size);

        pending_used += buf_size;

        return true;
    }

    if (pending_used > 0) {
        // buffer cannot be stored fully and there is already
        // stored data. fill up storage and write it
        size_t to_store = pending_free();

        memcpy(pending_ptr(), buf, to_store);

        pending_used += to_store;
        buf += to_store;
        buf_size -= to_store;

        if (!flush()) {
            return false;
        }
    }

    if (buf_size <= pending_free()) {
        // remaining buffer can be stored fully
        memcpy(pending_ptr(), buf, buf_size);

        pending_used += buf_size;
    }
    else {
        // remaining buffer is bigger than storage, write it directly
        if (!internal->write(buf, buf_size)) {
            printf("internal write failed\n");
            return false;
        }
    }

    return true;
}

bool BufferedChunkedResponse::writef(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    int required_or_error = vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (required_or_error < 0) {
        printf("vsnprintf(1) failed: %d\n", required_or_error);
        return false;
    }

    // +1 because vsnprintf always returns the length it would
    // have written without the NUL terminator. but in case of the
    // free length being exactly the required length vsnprintf will
    // truncate the output by one byte to fit the NUL terminator
    size_t required = required_or_error + 1;

    if (required > sizeof(pending)) {
        printf("vsnprintf buffer too small, required: %zu, available: %zu\n", required, sizeof(pending));
        return false;
    }

    if (required > pending_free()) {
        if (!flush()) {
            return false;
        }
    }

    va_start(args, fmt);
    int written_or_error = vsnprintf(pending_ptr(), pending_free(), fmt, args);
    va_end(args);

    if (written_or_error < 0) {
        printf("vsnprintf(2) failed: %d\n", written_or_error);
        return false;
    }

    pending_used += written_or_error;

    return true;
}

bool BufferedChunkedResponse::flush()
{
    if (pending_used > 0) {
        if (!internal->write(pending, pending_used)) {
            printf("internal write failed\n");
            return false;
        }

        pending_used = 0;
    }

    return true;
}

void BufferedChunkedResponse::end(String error)
{
    if (pending_used > 0) {
        printf("end with unflushed data\n");
    }

    internal->end(error);
}

size_t BufferedChunkedResponse::pending_free()
{
    assert(pending_used <= sizeof(pending));

    return sizeof(pending) - pending_used;
}

char *BufferedChunkedResponse::pending_ptr()
{
    assert(pending_used <= sizeof(pending));

    return pending + pending_used;
}
