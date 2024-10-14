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

#pragma once

#include <Arduino.h>
#include <assert.h>
#include <stdlib.h>
#include <mutex>
#include <condition_variable>

class IBaseChunkedResponse
{
public:
    virtual void begin(bool success) = 0;
    virtual void end(String error) = 0;
    virtual void alive() = 0;

    bool write(const char *buf, size_t buf_size = std::numeric_limits<size_t>::max()) {
        if (buf_size == std::numeric_limits<size_t>::max())
            buf_size = strlen(buf);

        return write_impl(buf, buf_size);
    }

protected:
    virtual bool write_impl(const char *buf, size_t buf_size) = 0;
};

class QueuedChunkedResponse : public IBaseChunkedResponse
{
public:
    QueuedChunkedResponse(IBaseChunkedResponse *internal, uint32_t timeout_ms) : internal(internal), timeout_ms(timeout_ms) {}

    void begin(bool success);
    void end(String error);
    void alive();

    String wait();

protected:
    bool write_impl(const char *buf, size_t buf_size);

private:
    bool call(std::function<bool(void)> &&local_function);

    IBaseChunkedResponse *internal;
    uint32_t timeout_ms;
    bool is_running = true;
    String end_error;
    bool has_ended = false;

    bool have_function = false;
    std::function<bool(void)> function;

    bool have_result = false;
    bool result;

    std::mutex mutex;
    std::condition_variable condition;
};

class IChunkedResponse : public IBaseChunkedResponse
{
public:
    virtual bool writef(const char *fmt, ...) = 0;
    virtual bool flush() = 0;

    void *metadata = nullptr;
};

class BufferedChunkedResponse : public IChunkedResponse
{
public:
    BufferedChunkedResponse(IBaseChunkedResponse *internal) : internal(internal) {};

    void begin(bool success);
    bool writef(const char *fmt, ...);
    bool flush();
    void end(String error);
    void alive();

protected:
    bool write_impl(const char *buf, size_t buf_size);

private:
    size_t pending_free();
    char *pending_ptr();

    IBaseChunkedResponse *internal;
    char pending[256];
    size_t pending_used = 0;
};
