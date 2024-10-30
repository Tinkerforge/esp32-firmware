/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#include "meter_rct_power.h"

#include <lwip/sockets.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"

#if 0
#define debugfln(fmt, ...) logger.printfln(fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define debugfln(fmt, ...) do {} while (0)
#endif

static uint16_t crc16ccitt(uint8_t *buffer, size_t length)
{
    uint32_t checksum = 0xFFFF;

    for (size_t i = 0; i < length; ++i) {
        for (size_t k = 0; k < 8; ++k) {
            bool bit = (buffer[i] >> (7 - k) & 1) == 1;
            bool c15 = ((checksum >> 15) & 1) == 1;

            checksum <<= 1;

            if (c15 ^ bit) {
                checksum ^= 0x1021;
            }
        }

        checksum &= 0xFFFF;
    }

    return checksum & 0xFFFF;
}

const char *get_rct_power_client_transaction_result_name(RCTPowerClientTransactionResult result)
{
    switch (result) {
    case RCTPowerClientTransactionResult::Success:
        return "Success";

    case RCTPowerClientTransactionResult::InvalidArgument:
        return "InvalidArgument";

    case RCTPowerClientTransactionResult::Aborted:
        return "Aborted";

    case RCTPowerClientTransactionResult::NoTransactionAvailable:
        return "NoTransactionAvailable";

    case RCTPowerClientTransactionResult::NotConnected:
        return "NotConnected";

    case RCTPowerClientTransactionResult::DisconnectedByPeer:
        return "DisconnectedByPeer";

    case RCTPowerClientTransactionResult::SendFailed:
        return "SendFailed";

    case RCTPowerClientTransactionResult::ReceiveFailed:
        return "ReceiveFailed";

    case RCTPowerClientTransactionResult::Timeout:
        return "Timeout";

    case RCTPowerClientTransactionResult::ChecksumMismatch:
        return "ChecksumMismatch";
    }

    return "<Unknown>";
}

void RCTPowerClient::read(const RCTValueSpec *spec, micros_t timeout, RCTPowerClientTransactionCallback &&callback)
{
    if (spec == nullptr || timeout < 0_s || !callback) {
        callback(RCTPowerClientTransactionResult::InvalidArgument, NAN);
        return;
    }

    if (socket_fd < 0) {
        callback(RCTPowerClientTransactionResult::NotConnected, NAN);
        return;
    }

    RCTPowerClientTransaction **tail_ptr = &scheduled_transaction_head;
    size_t scheduled_transaction_count = 0;

    while (*tail_ptr != nullptr) {
        tail_ptr = &(*tail_ptr)->next;
        ++scheduled_transaction_count;
    }

    if (scheduled_transaction_count >= RCT_POWER_CLIENT_MAX_SCHEDULED_TRANSACTION_COUNT) {
        callback(RCTPowerClientTransactionResult::NoTransactionAvailable, NAN);
        return;
    }

    RCTPowerClientTransaction *transaction = new RCTPowerClientTransaction;

    transaction->spec     = spec;
    transaction->timeout  = timeout;
    transaction->callback = std::move(callback);
    transaction->next     = nullptr;

    *tail_ptr = transaction;
}

void RCTPowerClient::close_hook()
{
    last_received_byte = 0;
    bootloader_magic_number = 0;
    bootloader_last_detected = 0_s;

    reset_pending_response();
    finish_all_transactions(RCTPowerClientTransactionResult::Aborted);
}

void RCTPowerClient::tick_hook()
{
    check_pending_transaction_timeout();

    if (pending_transaction == nullptr && scheduled_transaction_head != nullptr) {
        pending_transaction          = scheduled_transaction_head;
        scheduled_transaction_head   = scheduled_transaction_head->next;
        pending_transaction->next    = nullptr;
        pending_transaction_deadline = calculate_deadline(pending_transaction->timeout);

        uint8_t request[8];

        request[0] = 1; // command: read
        request[1] = 4; // length
        request[2] = (uint8_t)((pending_transaction->spec->id >> 24) & 0xFF);
        request[3] = (uint8_t)((pending_transaction->spec->id >> 16) & 0xFF);
        request[4] = (uint8_t)((pending_transaction->spec->id >>  8) & 0xFF);
        request[5] = (uint8_t)((pending_transaction->spec->id >>  0) & 0xFF);

        uint32_t checksum = crc16ccitt(request, 6);

        request[6] = (checksum >> 8) & 0xFF;
        request[7] = (checksum >> 0) & 0xFF;

        uint8_t escaped_request[1 + 8 * 2] = "+";
        size_t escaped_request_length = 1;

        for (size_t i = 0; i < sizeof(request); ++i) {
            if (request[i] == '+' || request[i] == '-') {
                escaped_request[escaped_request_length++] = '-';
            }

            escaped_request[escaped_request_length++] = request[i];
        }

        if (!send(escaped_request, escaped_request_length)) {
            int saved_errno = errno;
            finish_pending_transaction(RCTPowerClientTransactionResult::SendFailed, NAN);
            disconnect(TFGenericTCPClientDisconnectReason::SocketSendFailed, saved_errno);
        }
    }
}

bool RCTPowerClient::receive_hook()
{
    micros_t deadline = calculate_deadline(10_ms);

    while (sizeof(pending_response) - pending_response_used > 0) {
        if (deadline_elapsed(deadline)) {
            return true;
        }

        uint8_t received_byte;
        ssize_t result = recv(socket_fd, &received_byte, 1, 0);

        if (result < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                disconnect(TFGenericTCPClientDisconnectReason::SocketReceiveFailed, errno);
            }

            return false;
        }

        if (result == 0) {
            disconnect(TFGenericTCPClientDisconnectReason::DisconnectedByPeer, -1);
            return false;
        }

        bootloader_magic_number = (bootloader_magic_number << 8) | received_byte;

        if (bootloader_magic_number == 0x50F705AB) {
            bootloader_last_detected = now_us();
        }

        debugfln("received_byte %u 0x%02x %s| last_received_byte %u 0x%02x %s",
                 received_byte, received_byte, received_byte == '+' ? "+ " : (received_byte == '-' ? "- " : ""),
                 last_received_byte, last_received_byte, last_received_byte == '+' ? "+ " : (last_received_byte == '-' ? "- " : ""));

        if (wait_for_start) {
            if (received_byte == '+' && last_received_byte != '-') {
                debugfln("Received expected start byte");
                wait_for_start = false;
            }
        }
        else if (received_byte == '+') {
            if (last_received_byte == '-') {
                pending_response[pending_response_used++] = received_byte;
            }
            else {
                debugfln("Received unexpected start byte, starting new response");
                pending_response_used = 0;
            }
        }
        else if (received_byte == '-') {
            if (last_received_byte == '-') {
                pending_response[pending_response_used++] = received_byte;
            }
        }
        else {
            pending_response[pending_response_used++] = received_byte;
        }

        last_received_byte = received_byte;

        if (pending_response_used == 1 && pending_response[0] != 5) {
            debugfln("Received response with unexpected command %u, ignoring response", pending_response[0]);
            reset_pending_response();
        }
        else if (pending_response_used == 2 && pending_response[1] != 8) {
            debugfln("Received response with unexpected length %u, ignoring response", pending_response[1]);
            reset_pending_response();
        }
    }

    uint32_t id = ((uint32_t)pending_response[2] << 24) |
                  ((uint32_t)pending_response[3] << 16) |
                  ((uint32_t)pending_response[4] <<  8) |
                  ((uint32_t)pending_response[5] <<  0);

    if (pending_transaction == nullptr || pending_transaction->spec->id != id) {
        reset_pending_response();
        return true;
    }

    uint16_t actual_checksum   = crc16ccitt(pending_response, pending_response_used - 2);
    uint16_t expected_checksum = ((uint16_t)pending_response[pending_response_used - 2] << 8) | pending_response[pending_response_used - 1];

    if (actual_checksum != expected_checksum) {
        debugfln("Received response [%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x] for ID 0x%08x with checksum mismatch (actual=0x%04x expected=0x%04x), ignoring response",
                 pending_response[0], pending_response[1], pending_response[2], pending_response[3], pending_response[4], pending_response[5],
                 pending_response[6], pending_response[7], pending_response[8], pending_response[9], pending_response[10], pending_response[11],
                 id, actual_checksum, expected_checksum);

        reset_pending_response();
        finish_pending_transaction(RCTPowerClientTransactionResult::ChecksumMismatch, NAN);
        return true;
    }

    union {
        float value;
        uint8_t bytes[4];
    } u;

    u.bytes[0] = pending_response[6 + 3];
    u.bytes[1] = pending_response[6 + 2];
    u.bytes[2] = pending_response[6 + 1];
    u.bytes[3] = pending_response[6 + 0];

    float value = u.value;

    if (value != 0.0f) { // Really compare exactly with 0.0f
        // Don't convert 0.0f into -0.0f if the scale factor is negative
        value *= pending_transaction->spec->scale_factor;
    }

    debugfln("Received response for ID 0x%08x with value %f [%f]", id, u.value, value);

    reset_pending_response();
    finish_pending_transaction(RCTPowerClientTransactionResult::Success, value);
    return true;
}

void RCTPowerClient::finish_pending_transaction(RCTPowerClientTransactionResult result, float value)
{
    if (pending_transaction != nullptr) {
        RCTPowerClientTransactionCallback callback = std::move(pending_transaction->callback);
        pending_transaction->callback = nullptr;

        delete pending_transaction;
        pending_transaction          = nullptr;
        pending_transaction_deadline = 0_s;

        callback(result, value);
    }
}

void RCTPowerClient::finish_all_transactions(RCTPowerClientTransactionResult result)
{
    finish_pending_transaction(result, NAN);

    RCTPowerClientTransaction *scheduled_transaction = scheduled_transaction_head;
    scheduled_transaction_head = nullptr;

    while (scheduled_transaction != nullptr) {
        RCTPowerClientTransactionCallback callback = std::move(scheduled_transaction->callback);
        scheduled_transaction->callback = nullptr;

        RCTPowerClientTransaction *scheduled_transaction_next = scheduled_transaction->next;

        delete scheduled_transaction;
        scheduled_transaction = scheduled_transaction_next;

        callback(result, NAN);
    }
}

void RCTPowerClient::check_pending_transaction_timeout()
{
    if (pending_transaction != nullptr && deadline_elapsed(pending_transaction_deadline)) {
        finish_pending_transaction(RCTPowerClientTransactionResult::Timeout, NAN);
    }
}

void RCTPowerClient::reset_pending_response()
{
    wait_for_start        = true;
    pending_response_used = 0;
}
