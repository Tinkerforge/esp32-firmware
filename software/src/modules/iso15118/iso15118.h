/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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

#include "module.h"
#include "config.h"
#include "lwip/sockets.h"  // For struct pollfd
#include "qca700x.h"
#include "slac.h"
#include "sdp.h"
#include "common.h"
#include "din70121.h"
#include "iso2.h"
#include "iso20.h"

// Poll file descriptor indices (fixed positions in fds array)
static constexpr int FDS_TAP_INDEX    = 0;  // L2TAP for HomePlug/SLAC
static constexpr int FDS_SDP_INDEX    = 1;  // SDP UDP socket
static constexpr int FDS_LISTEN_INDEX = 2;  // DIN/ISO2/ISO20 TCP listen socket
static constexpr int FDS_ACTIVE_INDEX = 3;  // DIN/ISO2/ISO20 TCP active socket
static constexpr int FDS_COUNT        = 4;

class ISO15118 final : public IModule
{
private:
    ConfigRoot state_common;
    ConfigRoot state_din70121;
    ConfigRoot state_iso2;
    ConfigRoot state_iso20;

    size_t trace_buffer_index;
    size_t trace_buffer_index_ll;

    bool is_setup = false;
    uint64_t state_machine_task = 0;

public:
    ConfigRoot config;

    // Poll file descriptors for central I/O polling
    struct pollfd fds[FDS_COUNT];
    void set_poll_fd(int index, int fd) { fds[index].fd = fd; }

    ISO15118(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void state_machines_loop();
    [[gnu::format(__printf__, 2, 3)]] void trace(const char *fmt, ...);
    void trace_packet(const uint8_t *packet, const size_t packet_size);

    // Returns true if ISO 15118 is enabled (any of autocharge, read_soc, or charge_via_iso15118 is set)
    bool is_enabled() const {
        return config.get("autocharge")->asBool() ||
               config.get("read_soc")->asBool() ||
               config.get("charge_via_iso15118")->asBool();
    }

    QCA700x qca700x;
    SLAC slac;
    SDP sdp;
    Common common;
    DIN70121 din70121;
    ISO2 iso2;
    ISO20 iso20;

    bool debug_mode = false;
};
