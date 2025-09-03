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

#include "module_available.h"

#include "config.h"
#include "module.h"
#include "modules/cm_networking/cm_networking_defs.h"
#include "modules/power_manager/phase_switcher_back-end.h"
#include "TFTools/Micros.h"

class EMPhaseSwitcher final : public IModule,
                   public PhaseSwitcherBackend
{
public:
    EMPhaseSwitcher(){}

    // for IModule
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    // for PhaseSwitcherBackend
    uint32_t get_phase_switcher_priority() override {return 8;}
    bool phase_switching_capable() override;
    bool can_switch_phases_now(uint32_t phases_wanted) override;
    uint32_t get_phases() override;
    PhaseSwitcherBackend::SwitchingState get_phase_switching_state() override;
    bool switch_phases(uint32_t phases_wanted) override;
    bool is_external_control_allowed() override;

    bool is_proxy_mode_enabled();

    void filter_command_packet(size_t charger_idx, cm_command_packet *command_packet);
    void filter_state_packet(size_t charger_idx, cm_state_packet *state_packet);

private:
    enum class SwitchingState : uint8_t {
        Idle = 0,
        Stopping,
        DisconnectingCP,
        DisconnectingCPSettle,
        TogglingContactor,
        WaitUntilSwitched,
        WaitUntilCPReconnect,
        PostSwitchStateFaking,
    };

    PhaseSwitcherBackend::SwitchingState get_phase_switching_state_internal(bool ignore_contactor_error = false);
    bool switch_phases_internal(uint32_t phases_wanted);

    ConfigRoot charger_config;

    micros_t phase_switch_deadtime = 0_us;
    micros_t last_state_packet = -1_h; // System boot is time 0. Timestamps must be initialized with old enough values for correct deadline calculation after start-up.
    micros_t next_state_change_after;

    const char *charger_host;
    SwitchingState switching_state = SwitchingState::Idle;
    bool charger_usable = false;
    bool allocated_current_after_last_disconnect = true;
    bool skip_cp_disconnect = false;
    uint8_t controlled_charger_idx;
    uint8_t last_iec61851_state = 255;
    uint8_t last_charger_state = 255;
    uint8_t external_phase_override = 0;
};

#include "module_available_end.h"
