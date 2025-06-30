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

// Stolen from evse_common.h, which cannot be included or depended on because the EM doesn't have any EVSE code.
#define CHARGER_STATE_NOT_PLUGGED_IN 0
#define CHARGER_STATE_READY_TO_CHARGE 2
#define IEC_STATE_B 1

#include "gcc_warnings.h"

void EMPhaseSwitcher::pre_setup()
{
    charger_config = Config::Object({
        {"idx",  Config::Uint8(255)},
        {"host", Config::Str("", 0, 32)},
        {"proxy_mode", Config::Bool(false)},
    });
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

    const bool proxy_mode = charger_config.get("proxy_mode")->asBool();

    if (proxy_mode) {
        if (controlled_charger_idx != 0) {
            logger.printfln("Invalid controlled charger index %hhu", controlled_charger_idx);
            return;
        }
        if (!charge_manager.is_only_proxy()) {
            logger.printfln("Charge manager not in proxy mode");
            return;
        }
    } else {
        const size_t charger_count = charge_manager.get_charger_count();
        if (controlled_charger_idx >= charger_count) {
            logger.printfln("Controlled charger %u doesn't exist, have only %u", controlled_charger_idx, charger_count);
            return;
        }
    }

    // Verify config
    const String &controlled_charger_host = charger_config.get("host")->asString();
    const String &cm_charger_host = charge_manager.get_charger_host(controlled_charger_idx);
    if (controlled_charger_host != cm_charger_host) {
        logger.printfln("Invalid charger config: No match with charge manager. Expected charger %hhu with host '%s', but host from charge manager is '%s'.", controlled_charger_idx, controlled_charger_host.c_str(), cm_charger_host.c_str());
        controlled_charger_idx = 255;
        return;
    }

    // Initialize phase request with current state if external control is enabled
    if (power_manager.get_phase_switching_mode() == PHASE_SWITCHING_EXTERNAL_CONTROL) {
        external_phase_override = static_cast<uint8_t>(get_phases());
    }

    if (proxy_mode) {
        charger_host = strdup(charge_manager.get_charger_host(0).c_str());

        cm_networking.register_manager(&charger_host, 1, nullptr, nullptr);
        cm_networking.register_client(nullptr);
    } else {
#if MODULE_POWER_MANAGER_AVAILABLE()
        power_manager.register_phase_switcher_backend(this);
#endif
    }
}

void EMPhaseSwitcher::register_urls()
{
    api.addPersistentConfig("em_phase_switcher/charger_config", &charger_config);
}

// for PhaseSwitcherBackend

bool EMPhaseSwitcher::phase_switching_capable()
{
    return em_v1.get_is_contactor_installed();
}

bool EMPhaseSwitcher::can_switch_phases_now(uint32_t phases_wanted)
{
    if (!em_v1.get_is_contactor_installed()) {
        return false;
    }

    if (get_phase_switching_state_internal(phases_wanted == 1) != PhaseSwitcherBackend::SwitchingState::Ready) {
        return false;
    }

    return true;
}

uint32_t EMPhaseSwitcher::get_phases()
{
    return em_v1.get_phases();
}

PhaseSwitcherBackend::SwitchingState EMPhaseSwitcher::get_phase_switching_state()
{
    PhaseSwitcherBackend::SwitchingState internal_state = get_phase_switching_state_internal();
    if (internal_state != PhaseSwitcherBackend::SwitchingState::Ready) {
        return internal_state;
    }

    return PhaseSwitcherBackend::SwitchingState::Ready;
}

bool EMPhaseSwitcher::switch_phases(uint32_t phases_wanted)
{
    if (phases_wanted > 3) {
        logger.printfln("Invalid phases wanted: %lu", phases_wanted);
        return false;
    }

    if (!em_v1.get_is_contactor_installed()) {
        logger.printfln("Requested phase switch without contactor installed.");
        return false;
    }

    if (get_phase_switching_state() != PhaseSwitcherBackend::SwitchingState::Ready) {
        logger.printfln("Requested phase switch while not ready.");
        return false;
    }

    external_phase_override = static_cast<uint8_t>(phases_wanted);

    // Skip global hysteresis so that a phase switch request is acted on immediately.
    charge_manager.skip_global_hysteresis();

    return true;
}

bool EMPhaseSwitcher::is_external_control_allowed()
{
    return charge_manager.get_charger_count() == 1;
}

PhaseSwitcherBackend::SwitchingState EMPhaseSwitcher::get_phase_switching_state_internal(bool ignore_contactor_error)
{
    if (!em_v1.get_is_contactor_installed()) {
        // Don't report an error when phase_switching_capable() is false.
        return PhaseSwitcherBackend::SwitchingState::Ready;
    }

    if (!ignore_contactor_error && em_v1.get_is_contactor_error()) {
        return PhaseSwitcherBackend::SwitchingState::Error;
    }

    if (phase_switch_deadtime_us == 0_us) {
        return PhaseSwitcherBackend::SwitchingState::Ready;
    }

    if (!deadline_elapsed(phase_switch_deadtime_us)) {
        return PhaseSwitcherBackend::SwitchingState::Busy;
    }

    phase_switch_deadtime_us = 0_us;

    return PhaseSwitcherBackend::SwitchingState::Ready;
}

bool EMPhaseSwitcher::switch_phases_internal(uint32_t phases_wanted)
{
    if (!em_v1.get_is_contactor_installed()) {
        logger.printfln("Requested phase switch without contactor installed.");
        return false;
    }

    if (get_phase_switching_state_internal(phases_wanted == 1) != PhaseSwitcherBackend::SwitchingState::Ready) {
        logger.printfln("Requested phase switch while not ready.");
        return false;
    }

    em_v1.set_contactor_for_em_phase_switcher(phases_wanted > 1);
    phase_switch_deadtime_us = now_us() + micros_t{2000000}; // 2s

    return true;
}

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

    if (em_v1.get_is_contactor_error()) {
        command_packet->v1.allocated_current = 0;
        allocated_phases = 1;
    } else if (allocated_phases == 0) {
        allocated_phases = 1;
    } else if (allocated_phases == 2) {
        // Cannot allocate 2 phases, must allocate 3 instead.
        allocated_phases = 3;
    } else if (allocated_phases > 3) {
        logger.printfln("cmd received unsupported allocated phases: %i", command_packet->v2.allocated_phases);
        allocated_phases = 1;
    } // 1 and 3 are ok

    const uint32_t current_phases = em_v1.get_phases();

    SwitchingState old_state = switching_state;

    switch(switching_state) {
        case SwitchingState::Idle: {
            if (allocated_phases != current_phases) {
                if (!phase_switching_capable()) {
                    logger.printfln("Phase switch wanted but not available. Check configuration.");
                } else if (!can_switch_phases_now(allocated_phases)) {
                    // Can't switch to the requested phases at the moment. Try again later.
                    return;
                } else if (!deadline_elapsed(last_state_packet - 3500_ms)) {
                    logger.printfln("Charger state outdated. Last packet from %llims ago.", (now_us() - last_state_packet).to<millis_t>().as<int64_t>());
                    return;
                } else {
                    if (allocated_current_after_last_disconnect) {
                        skip_cp_disconnect = false;
                        command_packet->v1.allocated_current = 0;
                        switching_state = SwitchingState::Stopping;
                        logger.printfln("Toggling with CP disconnect");
                    } else {
                        skip_cp_disconnect = true;
                        command_packet->v1.allocated_current = 0;
                        switching_state = SwitchingState::TogglingContactor;
                        logger.printfln("Free toggle without CP disconnect");
                    }
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
            if (!skip_cp_disconnect) {
                command_packet->v1.command_flags |= CM_COMMAND_FLAGS_CPDISC_MASK;
            }

            if (switch_phases_internal(allocated_phases)) {
                switching_state = SwitchingState::WaitUntilSwitched;
            }
            break;
        }
        case SwitchingState::WaitUntilSwitched: {
            command_packet->v1.allocated_current = 0;
            if (!skip_cp_disconnect) {
                command_packet->v1.command_flags |= CM_COMMAND_FLAGS_CPDISC_MASK;
            }

            if (get_phase_switching_state_internal(allocated_phases == 1) == PhaseSwitcherBackend::SwitchingState::Ready) {
                if (em_v1.get_phases() == allocated_phases) {
                    if (skip_cp_disconnect) {
                        switching_state = SwitchingState::Idle;
                    } else {
                        switching_state = SwitchingState::WaitUntilCPReconnect;
                    }
                } else {
                    logger.printfln("Incorrect number of phases after switching, wanted %lu. Trying again.", allocated_phases);
                    switching_state = SwitchingState::TogglingContactor;
                }
            }

            break;
        }
        case SwitchingState::WaitUntilCPReconnect: {
            if (deadline_elapsed(next_state_change_after)) {
                switching_state = SwitchingState::PostSwitchStateFaking;
                next_state_change_after += 25_s; // Only 25 seconds because the charge manager will complain about unresponsive EVSEs after 32 seconds.
            } else {
                command_packet->v1.allocated_current = 0;
                command_packet->v1.command_flags |= CM_COMMAND_FLAGS_CPDISC_MASK;
            }
            break;
        }
        case SwitchingState::PostSwitchStateFaking: {
            if (deadline_elapsed(next_state_change_after) || command_packet->v2.allocated_phases == 0) {
                switching_state = SwitchingState::Idle;
            }
            break;
        }
        default:
            logger.printfln("Unexpected switching state for cmd: %hhu", static_cast<uint8_t>(switching_state));
            break;
    }

    if (last_charger_state == CHARGER_STATE_NOT_PLUGGED_IN && command_packet->v1.allocated_current == 0) {
        // Not plugged in and no current allocated -> safe
        allocated_current_after_last_disconnect = false;
    } else {
        if ((old_state == SwitchingState::Idle && last_charger_state >= CHARGER_STATE_READY_TO_CHARGE) || command_packet->v1.allocated_current != 0) {
            // Charger ready or charging or allocating current -> not safe
            allocated_current_after_last_disconnect = true;
        }
    }

    if (switching_state != old_state) {
        logger.printfln("Now in state %hhu (command)", static_cast<uint8_t>(switching_state));
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

    if (version >= 3 && !has_phase_switch && has_cp_disconnect && managed) {
        charger_usable = true;
    } else {
        if (version < 3)           logger.printfln("Downstream charger %zu not usable: charge protocol version is %hhu, need at least 3", charger_idx, version);
        if (has_phase_switch)      logger.printfln("Downstream charger %zu not usable: is phase-switching-capable", charger_idx);
        if (!has_cp_disconnect)    logger.printfln("Downstream charger %zu not usable: CP-disconnect not supported", charger_idx);
        if (!managed)              logger.printfln("Downstream charger %zu not usable: not managed", charger_idx);
        charger_usable = false;
        return;
    }

    uint32_t em_phases = em_v1.get_phases();

    // Modify packet
    if (external_phase_override) {
        // Don't fake phase-switching support
        state_packet->v3.phases = static_cast<uint8_t>((static_cast<uint32_t>(state_packet->v3.phases) & ~CM_STATE_V3_PHASES_CONNECTED_MASK) | external_phase_override);
    } else {
        state_packet->v1.feature_flags |= CM_FEATURE_FLAGS_PHASE_SWITCH_MASK; // Fake phase-switching support
        state_packet->v3.phases = static_cast<uint8_t>((static_cast<uint32_t>(state_packet->v3.phases) & ~CM_STATE_V3_PHASES_CONNECTED_MASK) | em_phases | CM_STATE_V3_CAN_PHASE_SWITCH_MASK);
    }

    SwitchingState old_state = switching_state;

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
                next_state_change_after = now_us() + 5_s;
                switching_state = SwitchingState::DisconnectingCPSettle;

                // CP was just disconnected but no current was allocated yet.
                allocated_current_after_last_disconnect = false;
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
            // Continue faking the state while the vehicle is taking its time to start charging.
            if (state_packet->v1.iec61851_state == IEC_STATE_B) {
                state_packet->v1.iec61851_state = last_iec61851_state;
                state_packet->v1.charger_state  = last_charger_state;
            } else {
                //logger.printfln("PostSwitchStateFaking: Done early");
                switching_state = SwitchingState::Idle;
            }
            break;
        }
        default:
            logger.printfln("Unexpected switching state for state: %hhu", static_cast<uint8_t>(switching_state));
            break;
    }

    if (switching_state != old_state) {
        logger.printfln("Now in state %hhu (state)", static_cast<uint8_t>(switching_state));
    }
}
