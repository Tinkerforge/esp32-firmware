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
#include "slac.h"
#include "sdp.h"
#include "common.h"
#include "din70121.h"
#include "iso2.h"
#include "iso20.h"

class ISO15118 final : public IModule
{
private:
    ConfigRoot config;
    ConfigRoot state_common;
    ConfigRoot state_din70121;
    ConfigRoot state_iso2;
    ConfigRoot state_iso20;

    size_t trace_buffer_index;
    size_t trace_buffer_index_ll;

public:
    ISO15118(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void state_machines_loop();
    void trace(const char *fmt, ...);
    void trace_ll(const char *fmt, ...);
    void trace_array(const char *array_name, const uint8_t *array, const size_t array_size);

    SLAC slac;
    SDP sdp;
    Common common;
    DIN70121 din70121;
    ISO2 iso2;
    ISO20 iso20;

    enum class ChargeType : uint8_t {
        DC_ReadSocOnce,
        DC_ReadSocInLoop,
        AC_Charging
    };
    ChargeType charge_type = ChargeType::DC_ReadSocInLoop;
};
