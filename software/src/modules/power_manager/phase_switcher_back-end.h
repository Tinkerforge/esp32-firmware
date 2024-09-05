/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    //#include "gcc_warnings.h"
#endif

class PhaseSwitcherBackend
{
public:
    enum class SwitchingState
    {
        Ready = 0,
        Busy,
        Error,
    };

    virtual ~PhaseSwitcherBackend() = default;

    virtual uint32_t get_phase_switcher_priority() = 0;
    virtual bool phase_switching_capable() = 0;
    virtual bool can_switch_phases_now(bool wants_3phase) = 0;
    virtual bool requires_cp_disconnect() = 0;
    virtual bool get_is_3phase() = 0;
    virtual SwitchingState get_phase_switching_state() = 0;
    virtual bool switch_phases_3phase(bool wants_3phase) = 0;
    virtual bool is_external_control_allowed() = 0;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
