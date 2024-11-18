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
#include "TFTools/Micros.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

class EMPhaseSwitcher final : public IModule
#if MODULE_AUTOMATION_AVAILABLE()
                 , public IAutomationBackend
#endif
{
public:
    EMPhaseSwitcher(){}

    // for IModule
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    bool is_proxy_mode_enabled();

    void filter_command_packet(size_t charger_idx, cm_command_packet *command_packet);
    void filter_state_packet(size_t charger_idx, cm_state_packet *state_packet);

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

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

    ConfigRoot state;
    ConfigRoot charger_config;

    micros_t last_state_packet = -1_h; // System boot is time 0. Timestamps must be initialized with old enough values for correct deadline calculation after start-up.

    micros_t next_state_change_after;

    const char *charger_hostname;
    SwitchingState switching_state = SwitchingState::Idle;
    bool charger_usable = false;
    uint8_t controlled_charger_idx;
    uint8_t last_iec61851_state;
    uint8_t last_charger_state;
};

#include "module_available_end.h"
