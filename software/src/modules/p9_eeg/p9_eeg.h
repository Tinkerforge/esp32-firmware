/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include "config.h"
#include "module.h"
#include "generated/p9_eeg_source.enum.h"
#include "generated/module_available.h"

#if MODULE_EM_V1_AVAILABLE()
#define P9_EEG_RELAY_COUNT 1
#else
#define P9_EEG_RELAY_COUNT 2
#endif

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

// Relay level that marks a relay as not used by §9 EEG
#define P9_EEG_RELAY_UNUSED 255

class P9Eeg final : public IModule
#if MODULE_AUTOMATION_AVAILABLE()
                  , public IAutomationBackend
#endif
{
private:
    enum class TriggerState : uint8_t {
        Unknown,
        NotLimited,
        Limited,
    };

    ConfigRoot config;
    ConfigRoot state;
    ConfigRoot control;

    bool eebus_available = false;
    bool eebus_limited = false;
    uint32_t eebus_limit_w = 0;

    bool api_control_written = false;

    // Relays that were driven by this module with the last applied config
    bool relay_owned[P9_EEG_RELAY_COUNT] = {};

    TriggerState last_trigger_state = TriggerState::Unknown;

    void update();
    void update_trigger(bool limited);
    void update_lpp_failsafe();
    void apply_relays(bool force);
    uint8_t level_for_limit(uint32_t limit_w, bool *reachable);
    bool get_relay(size_t index);
    void set_relay(size_t index, bool closed);

public:
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    inline bool is_enabled() { return config.get("enable")->asBool(); }

    // True if the relay (0-based) is controlled by §9 EEG and must not be switched by anything else
    bool is_relay_in_use(uint32_t index);

    // available = false: EEBUS has no LPP information (disabled or still starting)
    void set_eebus_limit(bool available, bool limited, uint32_t limit_w);

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif
};

#include "generated/module_available_end.h"
