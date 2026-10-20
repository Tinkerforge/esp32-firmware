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

#include <stdint.h>
#include <TFTools/Micros.h>
#include <lwip/ip_addr.h>

#include "module.h"
#include "language.h"
#include "config.h"
#include "tools/net.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class PortScanner final : public IModule
{
public:
    PortScanner() {}

    // for IModule
    void pre_setup() override;
    void register_urls() override;
    void loop() override;

private:
    enum class ScanState : uint8_t {
        Connect,
        Connecting,
        PortOpen,
        PortClosed,
        Next,
        Done,
    };

    void scan_flush_log();
    [[gnu::format(__printf__, 2, 3)]] void scan_printfln(const char *fmt, ...);

    ConfigRoot scan_start_config;
    ConfigRoot scan_keep_alive_config;
    ConfigRoot scan_abort_config;

    struct Scan {
        Language language;
        micros_t last_keep_alive = 0_us;
        ip4_addr_t ip4_first;
        ip4_addr_t ip4_last;
        uint16_t port;
        uint32_t cookie;
        ScanState state = ScanState::Connect;
        bool abort = false;
        bool connected = false;
        ip4_addr_t ip4;
        char ip4_str[INET_ADDRSTRLEN];
        int socket_fd = -1;
        micros_t connect_deadline;
        char printfln_buffer[512] = "";
        micros_t printfln_last_flush = 0_us;
        size_t printfln_buffer_used = 0;
    };

    Scan *scan = nullptr;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
