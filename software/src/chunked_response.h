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

#include <functional>
#include <WString.h>
#include <esp_err.h>
#include <assert.h>
#include <stdlib.h>
#include <mutex>
#include <condition_variable>

struct ChunkedResponseResult {
    inline operator bool()
    {
        return code == ESP_OK;
    }

    inline ChunkedResponseResult operator &=(const ChunkedResponseResult &other)
    {
        if (code == ESP_OK) {
            code = other.code;
            message = other.message;
        }

        return *this;
    }

    esp_err_t code = ESP_OK;
    const char *message = nullptr;
};

class IBaseChunkedResponse
{
public:
    virtual void begin(bool success) = 0;
    virtual void end(ChunkedResponseResult error = {}) = 0;
    virtual void alive() = 0;

    [[nodiscard]]
    ChunkedResponseResult write(const char *buf, size_t buf_size = std::numeric_limits<size_t>::max()) {
        if (buf_size == std::numeric_limits<size_t>::max()) {
            buf_size = strlen(buf);
        }

        return write_impl(buf, buf_size);
    }

protected:
    [[nodiscard]] virtual ChunkedResponseResult write_impl(const char *buf, size_t buf_size) = 0;
};

class QueuedChunkedResponse : public IBaseChunkedResponse
{
public:
    QueuedChunkedResponse(IBaseChunkedResponse *internal, uint32_t timeout_ms) : internal(internal), timeout_ms(timeout_ms) {}

    void begin(bool success);
    void end(ChunkedResponseResult result = {});
    void alive();

    [[nodiscard]] ChunkedResponseResult wait();

protected:
    [[nodiscard]] ChunkedResponseResult write_impl(const char *buf, size_t buf_size);

private:
    [[nodiscard]] ChunkedResponseResult call(std::function<ChunkedResponseResult(void)> &&local_function);

    IBaseChunkedResponse *internal;
    uint32_t timeout_ms;
    bool is_running = true;
    ChunkedResponseResult end_result;
    bool has_ended = false;

    bool have_function = false;
    std::function<ChunkedResponseResult(void)> function;

    bool have_result = false;
    ChunkedResponseResult result;

    std::mutex mutex;
    std::condition_variable condition;
};

class IChunkedResponse : public IBaseChunkedResponse
{
public:
    [[nodiscard]] [[gnu::format(__printf__, 2, 3)]] virtual ChunkedResponseResult writef(const char *fmt, ...) = 0;
    [[nodiscard]] [[gnu::format(__printf__, 2, 0)]] virtual ChunkedResponseResult vwritef(const char *fmt, va_list args) = 0;
    [[nodiscard]] virtual ChunkedResponseResult flush() = 0;

    void *metadata = nullptr;
};

class BufferedChunkedResponse : public IChunkedResponse
{
public:
    BufferedChunkedResponse(IBaseChunkedResponse *internal) : internal(internal) {};

    void begin(bool success);
    [[nodiscard]] [[gnu::format(__printf__, 2, 3)]] ChunkedResponseResult writef(const char *fmt, ...);
    [[nodiscard]] ChunkedResponseResult vwritef(const char *fmt, va_list args);
    [[nodiscard]] ChunkedResponseResult flush();
    void end(ChunkedResponseResult result);
    void alive();

protected:
    [[nodiscard]] ChunkedResponseResult write_impl(const char *buf, size_t buf_size);

private:
    size_t pending_free();
    char *pending_ptr();

    IBaseChunkedResponse *internal;
    char pending[1024];
    size_t pending_used = 0;
};
