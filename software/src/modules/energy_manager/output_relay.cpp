/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "WString.h"

#include "bindings/bricklet_warp_energy_manager.h"
#include "event_log.h"
#include "output_relay.h"
#include "modules.h"

static bool cmp_dummy(int32_t a, int32_t b)
{
    logger.printfln("energy_manager/RelayOutput: cmp_dummy(%i, %i) called. This should not happen.", a, b);
    return false;
}

/*
#define RELAY_CONFIG_IF_INPUT3          0
#define RELAY_CONFIG_IF_INPUT4          1
#define RELAY_CONFIG_IF_PHASE_SWITCHING 2
#define RELAY_CONFIG_IF_METER           3

#define RELAY_CONFIG_IS_HIGH            0
#define RELAY_CONFIG_IS_LOW             1
#define RELAY_CONFIG_IS_1PHASE          2
#define RELAY_CONFIG_IS_3PHASE          3
#define RELAY_CONFIG_IS_GOE_0KW         4
#define RELAY_CONFIG_IS_SOE_0KW         5

    uint8_t relay_config = energy_manager_config_in_use.get("relay_config")->asUint();
    if (relay_config == RELAY_CONFIG_RULE_BASED) {
        uint8_t relay_config_if = energy_manager_config_in_use.get("relay_config_if")->asUint();
        switch(relay_config_if) {
            case RELAY_CONFIG_IF_PHASE_SWITCHING: handle_relay_config_if_phase_switching(); break;
            case RELAY_CONFIG_IF_METER:           handle_relay_config_if_meter();           break;
            default: logger.printfln("Unknown RELAY_CONFIG_IF: %u", relay_config_if);       break;
        }
    }
*/
OutputRelay::OutputRelay(const ConfigRoot &conf)
{
    const uint32_t relay_conf_func  = conf.get("relay_config"     )->asUint();
    const uint32_t relay_conf_when  = conf.get("relay_config_when")->asUint();
    const uint32_t relay_conf_is    = conf.get("relay_config_is"  )->asUint();

    // Don't risk crashing on an invalid pointer, so make sure that the function pointers are always set to something sensible.
    update_func = &OutputRelay::nop;
    cmp_func = &cmp_dummy;

    switch(relay_conf_func) {
        case RELAY_CONFIG_RULE_BASED:
            switch(relay_conf_when) {
                case RELAY_CONFIG_IF_INPUT3:
                case RELAY_CONFIG_IF_INPUT4:
                    if (relay_conf_is == RELAY_CONFIG_IS_HIGH) {
                        ref_val = 1;
                    } else if (relay_conf_is == RELAY_CONFIG_IS_LOW) {
                        ref_val = 0;
                    } else {
                        logger.printfln("energy_manager/OutputRelay: Unknown RELAY_CONFIG_IS type %u", relay_conf_is);
                        break;
                    }
                    update_func = &OutputRelay::input_controlled;
                    if (relay_conf_when == RELAY_CONFIG_IF_INPUT3)
                        input_val = &(energy_manager.all_data.input[0]);
                    else
                        input_val = &(energy_manager.all_data.input[1]);
                    break;
                case RELAY_CONFIG_IF_PHASE_SWITCHING:
                    if (relay_conf_is == RELAY_CONFIG_IS_1PHASE) {
                        ref_val = 0;
                    } else if (relay_conf_is == RELAY_CONFIG_IS_3PHASE) {
                        ref_val = 1;
                    } else {
                        logger.printfln("energy_manager/OutputRelay: Unknown RELAY_CONFIG_IS type %u for phase switching mode", relay_conf_is);
                        break;
                    }
                    update_func = &OutputRelay::phase_switching_state;
                    break;
                case RELAY_CONFIG_IF_CONTACTOR_CHECK:
                    if (relay_conf_is == RELAY_CONFIG_IS_CONTACTOR_OK) {
                        // Checks against contactor_check_tripped, not contactor_check_state.
                        ref_val = 0;
                    } else if (relay_conf_is == RELAY_CONFIG_IS_CONTACTOR_FAIL) {
                        ref_val = 1;
                    } else {
                        logger.printfln("energy_manager/OutputRelay: Unknown RELAY_CONFIG_IS type %u for contactor check mode", relay_conf_is);
                        break;
                    }
                    update_func = &OutputRelay::contactor_check_tripped;
                    break;
                case RELAY_CONFIG_IF_POWER_AVAILABLE:
                    if (relay_conf_is == RELAY_CONFIG_IS_POWER_INSUFFIC) {
                        ref_val = 0;
                    } else if (relay_conf_is == RELAY_CONFIG_IS_POWER_SUFFIC) {
                        ref_val = 1;
                    } else {
                        logger.printfln("energy_manager/OutputRelay: Unknown RELAY_CONFIG_IS type %u for power available mode", relay_conf_is);
                        break;
                    }
                    update_func = &OutputRelay::contactor_check_tripped;
                    break;
                default:
                    logger.printfln("energy_manager/OutputRelay: Unknown RELAY_CONFIG_RULE type %u", relay_conf_when);
            }
            break;
        case RELAY_CONFIG_DISABLED:
            energy_manager.set_output(false);
            // update_func already set to nop.
            break;
        default:
            logger.printfln("energy_manager/OutputRelay: Unknown RELAY_CONFIG type %u", relay_conf_func);
            /* FALLTHROUGH */
        case RELAY_CONFIG_EXTERNAL:
            // update_func already set to nop.
            break;
    }
}

void OutputRelay::update()
{
    (this->*update_func)();
}

void OutputRelay::nop()
{
}

void OutputRelay::input_controlled()
{
    bool want_set = *input_val == ref_val;
    energy_manager.set_output(want_set);
}

void OutputRelay::phase_switching_state()
{
    bool want_set = energy_manager.is_3phase == ref_val;
    energy_manager.set_output(want_set);
}

void OutputRelay::contactor_check_tripped()
{
    bool want_set = energy_manager.contactor_check_tripped == ref_val;
    energy_manager.set_output(want_set);
}

void OutputRelay::power_sufficient()
{
    bool want_set = energy_manager.is_on_last == ref_val;
    energy_manager.set_output(want_set);
}
