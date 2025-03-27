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

// The ISO15118 class is the coordinator for all of the different versions and layers of the ISO 15118 standard.
// It coordinates between the following protocols:
// * SPI/QCA700x (Low level SPI communication with the QCA700x modem),
// * SLAC (Signal-Level-Attenuation-Characterization) as described in ISO 15118-3,
// * IPv6/ICMPv6/NDP (Neighbor Discorver Protoocol with Neighbor Solicitation/Advertisement),
// * IPv6/UDP/SDP (SECC Discovery Protocol),
// * IPv6/TCP/DIN-SPEC-70121,
// * IPv6/TCP/ISO-15118-2,
// * IPv6/TCP/ISO-15118-20
// and the EVSE that runs on a different board (the EVSE Bricklet).
// The EXI message encoding/decoding is based on libcbv2g.
// The objective is to *robustly* be able to
// * fingerprint the EV for authentication,
// * read SOC from the EV and
// * if it will be implemented by EVs, allow for bi-directional charging.
// To read the necessary data we may start a DC charge and end it again immediatily or similar, depending on the standards that the EV supports.
// The actual charging will mostly still be done through IEC 61851-1 (PWM on CP).

#include "iso15118.h"

#include <time.h>
#include <type_traits>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"

#include "qca700x.h"


void ISO15118::trace_array(const char *array_name, const uint8_t *array, const size_t array_size)
{
#if defined(BOARD_HAS_PSRAM)
    trace_ll("%s: %d bytes", array_name, array_size);
    for (size_t i = 0; i < array_size; i+=8) {
        switch(std::min(static_cast<size_t>(8), array_size - i)) {
            case 1: trace_ll("  %02x", array[i]); break;
            case 2: trace_ll("  %02x %02x", array[i], array[i+1]); break;
            case 3: trace_ll("  %02x %02x %02x", array[i], array[i+1], array[i+2]); break;
            case 4: trace_ll("  %02x %02x %02x %02x", array[i], array[i+1], array[i+2], array[i+3]); break;
            case 5: trace_ll("  %02x %02x %02x %02x %02x", array[i], array[i+1], array[i+2], array[i+3], array[i+4]); break;
            case 6: trace_ll("  %02x %02x %02x %02x %02x %02x", array[i], array[i+1], array[i+2], array[i+3], array[i+4], array[i+5]); break;
            case 7: trace_ll("  %02x %02x %02x %02x %02x %02x %02x", array[i], array[i+1], array[i+2], array[i+3], array[i+4], array[i+5], array[i+6]); break;
            case 8: trace_ll("  %02x %02x %02x %02x %02x %02x %02x %02x", array[i], array[i+1], array[i+2], array[i+3], array[i+4], array[i+5], array[i+6], array[i+7]); break;
            default: case 0: break;
        }
    }
#endif
}

void ISO15118::trace_ll(const char *fmt, ...)
{
#if defined(BOARD_HAS_PSRAM)
    va_list args;

    va_start(args, fmt);
    logger.vtracefln_plain(trace_buffer_index_ll, fmt, args);
    va_end(args);
#endif
}

void ISO15118::trace(const char *fmt, ...)
{
#if defined(BOARD_HAS_PSRAM)
    va_list args;

    va_start(args, fmt);
    logger.vtracefln_plain(trace_buffer_index, fmt, args);
    va_end(args);
#endif
}

void ISO15118::pre_setup()
{
    this->trace_buffer_index    = logger.alloc_trace_buffer("iso15118", 1 << 19);
    this->trace_buffer_index_ll = logger.alloc_trace_buffer("iso15118_ll", 1 << 19);

    config = ConfigRoot{Config::Object({
    }), [this](Config &update, ConfigSource source) -> String {
        return "";
    }};

    slac.pre_setup();
    sdp.pre_setup();
    common.pre_setup();
    din70121.pre_setup();
    iso2.pre_setup();
    iso20.pre_setup();
}

void ISO15118::setup()
{
    api.restorePersistentConfig("iso15118/config", &config);

    initialized = true;

    qca700x.setup_netif();
    sdp.setup_socket();
    common.setup_socket();
}

void ISO15118::register_urls()
{
    api.addPersistentConfig("iso15118/config", &config);
    api.addState("iso15118/state_slac",     &slac.api_state);
    api.addState("iso15118/state_sdp",      &sdp.api_state);
    api.addState("iso15118/state_common",   &common.api_state);
    api.addState("iso15118/state_din70121", &din70121.api_state);
    api.addState("iso15118/state_iso2",     &iso2.api_state);
    api.addState("iso15118/state_iso20",    &iso20.api_state);

    task_scheduler.scheduleWithFixedDelay([this]() {
        this->state_machines_loop();
    }, 1000_ms, 20_ms);
}

void ISO15118::state_machines_loop()
{
    // TODO: Only run the state machines that are needed depending on link state?
    qca700x.state_machine_loop();
    slac.state_machine_loop();
    sdp.state_machine_loop();
    common.state_machine_loop();
    // common calls din70121, iso2 and iso20 state machines
}