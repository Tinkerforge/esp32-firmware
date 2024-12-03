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

#define EVENT_LOG_PREFIX "em_phaseswitcher"

#include "em_phase_switcher.h"
#include "module_dependencies.h"

#include "event_log_prefix.h"
#include "TFTools/Micros.h"

#include "gcc_warnings.h"

void EMPhaseSwitcher::pre_setup()
{
    charger_config = Config::Object({
        {"idx",  Config::Uint8(255)},
        {"host", Config::Str("", 0, 32)},
        {"proxy_mode", Config::Bool(false)},
    });

    //state = Config::Object({
    //});

#if MODULE_AUTOMATION_AVAILABLE()
    //automation.register_trigger(
    //    AutomationTriggerID::EMInput,
    //    Config::Object({
    //        {"index",  Config::Uint(0, 0, 3)},
    //        {"closed", Config::Bool(false)},
    //    })
    //);

    //automation.register_action(
    //    AutomationActionID::EMRelaySwitch,
    //    Config::Object({
    //        {"index",  Config::Uint(0, 0, 1)},
    //        {"closed", Config::Bool(false)}
    //    }),
    //    [this](const Config *cfg) {
    //        const uint32_t index  = cfg->get("index" )->asUint();
    //        const bool     closed = cfg->get("closed")->asBool();
    //        this->set_relay_output(index, closed);
    //    }
    //);
#endif
}

void EMPhaseSwitcher::setup()
{
    initialized = 1;

    api.restorePersistentConfig("em_phase_switcher/charger_config", &charger_config);

    // Cache config
    controlled_charger_idx = static_cast<uint8_t>(charger_config.get("idx")->asUint());

    if (controlled_charger_idx == 255) {
        // Phase switching disabled
        return;
    }

    const size_t charger_count = charge_manager.config.get("chargers")->count();
    if (controlled_charger_idx >= charger_count) {
        logger.printfln("Controlled charger %u doesn't exist, have only %u", controlled_charger_idx, charger_count);
        return;
    }

    // Verify config
    const String &controlled_charger_host = charger_config.get("host")->asString();
    const String &cm_charger_host = charge_manager.get_charger_host(controlled_charger_idx);
    if (controlled_charger_host != cm_charger_host) {
        logger.printfln("Invalid charger config: No match with charge manager. Expected charger %hhu with host '%s', but host from charge manager is '%s'.", controlled_charger_idx, controlled_charger_host.c_str(), cm_charger_host.c_str());
        controlled_charger_idx = 255;
        return;
    }

    if (charger_config.get("proxy_mode")->asBool()) {
        charger_hostname = strdup(charge_manager.get_charger_host(0).c_str());

        cm_networking.register_manager(&charger_hostname, 1, nullptr, nullptr);
        cm_networking.register_client(nullptr);
    }
}

void EMPhaseSwitcher::register_urls()
{
    api.addPersistentConfig("em_phase_switcher/charger_config", &charger_config);
}

#if MODULE_AUTOMATION_AVAILABLE()
bool EMPhaseSwitcher::has_triggered(const Config *conf, void *data)
{
    const AutomationTriggerID trigger_id = conf->getTag<AutomationTriggerID>();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"

    switch (trigger_id) {
        default:
            break;
    }
#pragma GCC diagnostic pop

    logger.printfln("has_triggered called for unexpected trigger ID %u", static_cast<uint32_t>(trigger_id));
    return false;
}
#endif

bool EMPhaseSwitcher::is_proxy_mode_enabled()
{
    return charger_config.get("proxy_mode")->asBool();
}

void EMPhaseSwitcher::filter_command_packet(size_t charger_idx, cm_command_packet *command_packet)
{
    if (charger_idx != controlled_charger_idx) {
        return;
    }

    if (!charger_usable) {
        return;
    }

    uint32_t allocated_phases = static_cast<uint32_t>(command_packet->v2.allocated_phases); // Negative values become large positive values, which will be filtered out.
    if (allocated_phases == 0) {
        allocated_phases = 1;
    } else if (allocated_phases == 2) {
        // Cannot allocate 2 phases, must allocate 3 instead.
        allocated_phases = 3;
    } else if (allocated_phases > 3) {
        logger.printfln("cmd received unsupported allocated phases: %i", command_packet->v2.allocated_phases);
        allocated_phases = 1;
    } // 1 and 3 are ok

    const uint32_t current_phases = em_v1.get_phases();

    switch(switching_state) {
        case SwitchingState::Idle: {
            if (allocated_phases != current_phases) {
                if (!em_v1.phase_switching_capable()) {
                    logger.printfln("Phase switch wanted but not available. Check configuration.");
                } else if (!em_v1.can_switch_phases_now(allocated_phases)) {
                    // Can't switch to the requested phases at the moment. Try again later.
                    return;
                } else if (!deadline_elapsed(last_state_packet - 3500_ms)) {
                    logger.printfln("Charger state outdated. Last packet from %llims ago.", static_cast<int64_t>(now_us() - last_state_packet) / 1000);
                    return;
                } else {
                    command_packet->v1.allocated_current = 0;
                    switching_state = SwitchingState::Stopping;
                }
            }
            break;
        }
        case SwitchingState::Stopping: {
            command_packet->v1.allocated_current = 0;
            break;
        }
        case SwitchingState::DisconnectingCP:
        case SwitchingState::DisconnectingCPSettle: {
            command_packet->v1.allocated_current = 0;
            command_packet->v1.command_flags |= CM_COMMAND_FLAGS_CPDISC_MASK;
            break;
        }
        case SwitchingState::TogglingContactor: {
            command_packet->v1.allocated_current = 0;
            command_packet->v1.command_flags |= CM_COMMAND_FLAGS_CPDISC_MASK;

            if (em_v1.switch_phases(allocated_phases)) {
                switching_state = SwitchingState::WaitUntilSwitched;
            }
            break;
        }
        case SwitchingState::WaitUntilSwitched: {
            command_packet->v1.allocated_current = 0;
            command_packet->v1.command_flags |= CM_COMMAND_FLAGS_CPDISC_MASK;

            if (em_v1.get_phase_switching_state() == PhaseSwitcherBackend::SwitchingState::Ready) {
                if (em_v1.get_phases() == allocated_phases) {
                    switching_state = SwitchingState::WaitUntilCPReconnect;
                } else {
                    logger.printfln("Incorrect number of phases after switching, wanted %u. Trying again.", allocated_phases);
                    switching_state = SwitchingState::TogglingContactor;
                }
            }

            break;
        }
        case SwitchingState::WaitUntilCPReconnect: {
            if (deadline_elapsed(next_state_change_after)) {
                switching_state = SwitchingState::PostSwitchStateFaking;
                next_state_change_after += 10_s;
            } else {
                command_packet->v1.allocated_current = 0;
                command_packet->v1.command_flags |= CM_COMMAND_FLAGS_CPDISC_MASK;
            }
            break;
        }
        case SwitchingState::PostSwitchStateFaking: {
            if (deadline_elapsed(next_state_change_after)) {
                switching_state = SwitchingState::Idle;
            }
            break;
        }
        default:
            logger.printfln("Unexpected switching state for cmd: %u", static_cast<uint32_t>(switching_state));
            break;
    }
}

void EMPhaseSwitcher::filter_state_packet(size_t charger_idx, cm_state_packet *state_packet)
{
    if (charger_idx != controlled_charger_idx) {
        return;
    }

    last_state_packet = now_us();

    const uint8_t version = state_packet->header.version;
    const bool has_phase_switch = (state_packet->v1.feature_flags >> CM_FEATURE_FLAGS_PHASE_SWITCH_BIT_POS) & 1;
    const bool has_cp_disconnect = (state_packet->v1.feature_flags >> CM_FEATURE_FLAGS_CP_DISCONNECT_BIT_POS) & 1;
    const bool managed = (state_packet->v1.state_flags >> CM_STATE_FLAGS_MANAGED_BIT_POS) & 1;
    const uint8_t phases_connected = state_packet->v3.phases & CM_STATE_V3_PHASES_CONNECTED_MASK; // Statically configured supply phases

    if (version >= 3 && !has_phase_switch && has_cp_disconnect && managed && phases_connected == 3) {
        charger_usable = true;
    } else {
        if (version < 3)           logger.printfln("Downstream charger %zu not usable: charge protocol version is %hhu, need at least 3", charger_idx, version);
        if (has_phase_switch)      logger.printfln("Downstream charger %zu not usable: is phase-switching-capable", charger_idx);
        if (!has_cp_disconnect)    logger.printfln("Downstream charger %zu not usable: CP-disconnect not supported", charger_idx);
        if (!managed)              logger.printfln("Downstream charger %zu not usable: not managed", charger_idx);
        if (phases_connected != 3) logger.printfln("Downstream charger %zu not usable: not connected to three phases", charger_idx);
        charger_usable = false;
        return;
    }

    uint32_t em_phases = em_v1.get_phases();

    // Modify packet
    state_packet->v1.feature_flags |= CM_FEATURE_FLAGS_PHASE_SWITCH_MASK; // Fake phase-switching support
    state_packet->v3.phases = static_cast<uint8_t>((static_cast<uint32_t>(state_packet->v3.phases) & ~CM_STATE_V3_PHASES_CONNECTED_MASK) | em_phases | CM_STATE_V3_CAN_PHASE_SWITCH_MASK);

    switch(switching_state) {
        case SwitchingState::Idle: {
            last_iec61851_state = state_packet->v1.iec61851_state;
            last_charger_state  = state_packet->v1.charger_state;
            break;
        }
        case SwitchingState::Stopping: {
            const uint16_t allowed_charging_current = state_packet->v1.allowed_charging_current;
            if (allowed_charging_current == 0) {
                switching_state = SwitchingState::DisconnectingCP;
            }
            state_packet->v1.iec61851_state = last_iec61851_state;
            state_packet->v1.charger_state  = last_charger_state;
            break;
        }
        case SwitchingState::DisconnectingCP: {
            const bool cp_is_disconnected = (state_packet->v1.state_flags >> CM_STATE_FLAGS_CP_DISCONNECTED_BIT_POS) & 1;
            if (cp_is_disconnected) {
                next_state_change_after = now_us() + 4_s;
                switching_state = SwitchingState::DisconnectingCPSettle;
            }
            state_packet->v1.iec61851_state = last_iec61851_state;
            state_packet->v1.charger_state  = last_charger_state;
            break;
        }
        case SwitchingState::DisconnectingCPSettle: {
            switching_state = SwitchingState::TogglingContactor;
            state_packet->v1.iec61851_state = last_iec61851_state;
            state_packet->v1.charger_state  = last_charger_state;
            break;
        }
        case SwitchingState::TogglingContactor:
        case SwitchingState::WaitUntilSwitched:
        case SwitchingState::WaitUntilCPReconnect: {
            state_packet->v1.iec61851_state = last_iec61851_state;
            state_packet->v1.charger_state  = last_charger_state;
            break;
        }
        case SwitchingState::PostSwitchStateFaking: {
            if (state_packet->v1.iec61851_state == last_iec61851_state && state_packet->v1.charger_state == last_charger_state) {
                //logger.printfln("PostSwitchStateFaking: Done early");
                switching_state = SwitchingState::Idle;
            } else {
                state_packet->v1.iec61851_state = last_iec61851_state;
                state_packet->v1.charger_state  = last_charger_state;
            }
            break;
        }
        default:
            logger.printfln("Unexpected switching state for state: %u", static_cast<uint32_t>(switching_state));
            break;
    }
}
