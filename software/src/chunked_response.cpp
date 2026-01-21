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

#define EVENT_LOG_PREFIX "chunked_resp"

#include "chunked_response.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "event_log_prefix.h"
#include "main_dependencies.h"
#include "tools/printf.h"

void QueuedChunkedResponse::begin(bool success)
{
    (void)call([this, success]{internal->begin(success); return ChunkedResponseResult{};});
}

void QueuedChunkedResponse::alive()
{
    (void)call([this]{internal->alive(); return ChunkedResponseResult{};});
}

ChunkedResponseResult QueuedChunkedResponse::write_impl(const char *buf, size_t buf_size)
{
    return call([this, buf, buf_size]{return internal->write(buf, buf_size);});
}

void QueuedChunkedResponse::end(ChunkedResponseResult result)
{
    {
        std::lock_guard<std::mutex> guard(mutex);

        if (has_ended) {
            return;
        }
    }

    (void)call([this, &result]{internal->end(result); is_running = false; return ChunkedResponseResult{};});

    {
        std::lock_guard<std::mutex> guard(mutex);

        end_result = result;
        has_ended = true;
    }

    condition.notify_one();
}

ChunkedResponseResult QueuedChunkedResponse::wait()
{
    std::unique_lock<std::mutex> lock(mutex, std::defer_lock_t()); // don't lock immediatly

    while (is_running) {
        lock.lock();

        if (!condition.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this]{return have_function;})) {
            ChunkedResponseResult result{ESP_FAIL, "Condition timeout, no function"};

            internal->end(result);

            end_result = result;
            has_ended = true;

            lock.unlock();
            condition.notify_one();

            return end_result;
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
        end_result = {ESP_FAIL, "Condition timeout, has not ended"};
    }

    lock.unlock();

    return end_result;
}

ChunkedResponseResult QueuedChunkedResponse::call(std::function<ChunkedResponseResult(void)> &&local_function)
{
    std::unique_lock<std::mutex> lock(mutex);

    if (!condition.wait_for(lock, std::chrono::seconds(10), [this]{return !have_result || has_ended;})) {
        lock.unlock();

        return ChunkedResponseResult{ESP_FAIL, "Condition timeout, has result or has not ended"};
    }

    if (has_ended) {
        lock.unlock();

        return ChunkedResponseResult{ESP_FAIL, "Has ended"};
    }

    function = std::move(local_function);
    have_function = true;

    lock.unlock();
    condition.notify_one();

    lock.lock();

    if (!condition.wait_for(lock, std::chrono::seconds(10), [this]{return have_result || has_ended;})) {
        lock.unlock();

        return ChunkedResponseResult{ESP_FAIL, "Condition timeout, has no result or has not ended"};
    }

    if (has_ended) {
        lock.unlock();

        return ChunkedResponseResult{ESP_FAIL, "Has ended"};
    }

    ChunkedResponseResult local_result = result;
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

ChunkedResponseResult BufferedChunkedResponse::write_impl(const char *buf, size_t buf_size)
{
    if (buf_size <= pending_free()) {
        // buffer can be stored fully
        memcpy(pending_ptr(), buf, buf_size);

        pending_used += buf_size;

        return ChunkedResponseResult{};
    }

    if (pending_used > 0) {
        // buffer cannot be stored fully and there is already
        // stored data. fill up storage and write it
        size_t to_store = pending_free();

        memcpy(pending_ptr(), buf, to_store);

        pending_used += to_store;
        buf += to_store;
        buf_size -= to_store;

        ChunkedResponseResult result = flush();

        if (!result) {
            return result;
        }
    }

    if (buf_size <= pending_free()) {
        // remaining buffer can be stored fully
        memcpy(pending_ptr(), buf, buf_size);

        pending_used += buf_size;
    }
    else {
        // remaining buffer is bigger than storage, write it directly
        ChunkedResponseResult result = internal->write(buf, buf_size);

        if (!result) {
            return result;
        }
    }

    return ChunkedResponseResult{};
}

ChunkedResponseResult BufferedChunkedResponse::writef(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    ChunkedResponseResult result = vwritef(fmt, args);
    va_end(args);

    return result;
}

ChunkedResponseResult BufferedChunkedResponse::vwritef(const char *fmt, va_list args)
{
    va_list args_copy;

    va_copy(args_copy, args);
    int required_or_error = vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);

    if (required_or_error < 0) {
        logger.printfln("vsnprintf(1) failed: %d", required_or_error);

        return ChunkedResponseResult{ESP_FAIL, "vsnprintf(1) failed"};
    }

    // +1 because vsnprintf always returns the length it would
    // have written without the NUL terminator. but in case of the
    // free length being exactly the required length vsnprintf will
    // truncate the output by one byte to fit the NUL terminator
    size_t required = required_or_error + 1;

    if (required > sizeof(pending)) {
        logger.printfln("vsnprintf buffer too small, required: %zu, available: %zu\n", required, sizeof(pending));

        return ChunkedResponseResult{ESP_FAIL, "vsnprintf buffer too small"};
    }

    if (required > pending_free()) {
        ChunkedResponseResult result = flush();

        if (!result) {
            return result;
        }
    }

    int written_or_error = vsnprintf(pending_ptr(), pending_free(), fmt, args);

    if (written_or_error < 0) {
        logger.printfln("vsnprintf(2) failed: %d", written_or_error);

        return ChunkedResponseResult{ESP_FAIL, "vsnprintf(2) failed"};
    }

    pending_used += written_or_error;

    return ChunkedResponseResult{};
}

ChunkedResponseResult BufferedChunkedResponse::flush()
{
    if (pending_used > 0) {
        ChunkedResponseResult result = internal->write(pending, pending_used);

        if (!result) {
            return result;
        }

        pending_used = 0;
    }

    return ChunkedResponseResult{};
}

void BufferedChunkedResponse::end(ChunkedResponseResult result)
{
    if (pending_used > 0) {
        printf("end with unflushed data\n");
    }

    internal->end(result);
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
