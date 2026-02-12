/* esp32-firmware
 * Copyright (C) 2026 Frederic Henrichs <frederic@tinkerforge.com>
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

#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Packet type definitions from backend/src/udp_server/packet.rs
enum class PacketType : uint8_t {
    ManagementCommand = 0x00,
    Ack = 0x01,
    Nack = 0x02,
    MetadataForChargeLog = 0x03,
    RequestChargeLogSend = 0x04,
};

// Nack reason from backend/src/udp_server/packet.rs
enum class NackReason : uint8_t {
    Busy = 0,
    TooManyRequests = 1,
    OngoingRequest = 2,
    Timeout = 3,
};

// Management command ID enum from backend/src/udp_server/packet.rs
enum management_command_id {
    Connect = 0,
    Disconnect = 1,
};

// Management packet header from backend/src/udp_server/packet.rs
// magic: 0x1234
// p_type: 0x00 - Management Command, 0x01 - Ack, 0x02 - Nack, 0x03 - Metadata for Charge Log
struct [[gnu::packed]] management_packet_header {
    uint16_t magic;
    uint16_t length;
    uint16_t seq_num;
    uint8_t version;
    PacketType type;
};

// Management command from backend/src/udp_server/packet.rs
struct [[gnu::packed]] management_command {
    management_command_id command_id;
    int32_t connection_no;
    uint8_t connection_uuid[16];
};

// Management command packet from backend/src/udp_server/packet.rs
struct [[gnu::packed]] management_command_packet {
    struct management_packet_header header;
    struct management_command command;
};

// Management response V2 from backend/src/udp_server/packet.rs
struct [[gnu::packed]] management_response_v2 {
    uint8_t charger_id[16];
    int32_t connection_no;
    uint8_t connection_uuid[16];
};

// Management response packet from backend/src/udp_server/packet.rs
struct [[gnu::packed]] management_response_packet {
    struct management_packet_header header;
    struct management_response_v2 data;
};

// Port discovery packet (ESP32-specific, not in backend)
struct [[gnu::packed]] port_discovery_packet {
    uint32_t charger_id;
    int32_t connection_no;
    uint8_t connection_uuid[16];
};

// Ack packet from backend/src/udp_server/packet.rs
struct [[gnu::packed]] ack_packet {
    management_packet_header header;
};

// Nack packet from backend/src/udp_server/packet.rs
struct [[gnu::packed]] nack_packet {
    management_packet_header header;
    NackReason reason;
};

// Charge log metadata packet from backend/src/udp_server/packet.rs
// This is a variable-length packet with the following structure:
// - header: management_packet_header (8 bytes)
// - user_uuid: uint8_t[16] (16 bytes)
// - filename_length: uint16_t (2 bytes)
// - display_name_length: uint16_t (2 bytes)
// - lang: uint8_t[2] (2 bytes)
// - is_monthly_email: uint8_t (1 byte)
// - filename: variable length (filename_length bytes)
// - display_name: variable length (display_name_length bytes)
struct [[gnu::packed]] charge_log_send_metadata_packet {
    // Returns total bytes written, or 0 if buffer is too small
    static size_t write_to_buffer(uint8_t *buf, size_t buf_size,
                                  const management_packet_header &header,
                                  const uint8_t user_uuid[16],
                                  const uint8_t lang[2],
                                  bool is_monthly_email,
                                  const char *filename, uint16_t filename_len,
                                  const char *display_name, uint16_t display_name_len)
    {
        size_t total_size = sizeof(management_packet_header) + 16 + sizeof(uint16_t) + sizeof(uint16_t) + 2 + 1 + filename_len + display_name_len;
        if (buf_size < total_size) {
            return 0;
        }

        size_t offset = 0;

        // Write header
        memcpy(buf + offset, &header, sizeof(header));
        offset += sizeof(header);

        // Write user_uuid
        memcpy(buf + offset, user_uuid, 16);
        offset += 16;

        // Write filename_length
        memcpy(buf + offset, &filename_len, sizeof(filename_len));
        offset += sizeof(filename_len);

        // Write display_name_length
        memcpy(buf + offset, &display_name_len, sizeof(display_name_len));
        offset += sizeof(display_name_len);

        // Write lang
        memcpy(buf + offset, lang, 2);
        offset += 2;

        // Write is_monthly_email
        buf[offset] = is_monthly_email ? 1 : 0;
        offset += 1;

        // Write filename
        memcpy(buf + offset, filename, filename_len);
        offset += filename_len;

        // Write display_name
        memcpy(buf + offset, display_name, display_name_len);
        offset += display_name_len;

        return offset;
    }
};
