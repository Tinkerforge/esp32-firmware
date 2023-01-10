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

#include "event_log.h"
#include "input_pin.h"
#include "modules.h"

InputPin::InputPin(uint32_t num_name, uint32_t num_logic, const ConfigRoot &conf)
{
    this->num_name = num_name;
    this->num_logic = num_logic;

    String pin_func_str     = String("input") + num_name + "_config";
    String pin_limit_str    = String("input") + num_name + "_config_limit";
    String pin_when_str     = String("input") + num_name + "_config_when";
    uint32_t pin_conf_func  = conf.get(pin_func_str )->asUint();
    int32_t  pin_conf_limit = conf.get(pin_limit_str)->asInt();
    uint32_t pin_conf_when  = conf.get(pin_when_str)->asUint();

    invert_pin = pin_conf_when == INPUT_CONFIG_WHEN_LOW;

    switch(pin_conf_func) {
        case INPUT_CONFIG_BLOCK_CHARGING:
            update_func = &InputPin::block_charging;
            out_dst = &(energy_manager.charging_blocked.pin[num_logic]);
            break;
        case INPUT_CONFIG_EXCESS_CHARGING:
            update_func = &InputPin::switch_excess_charging;
            out_dst = &(energy_manager.excess_charging_enable);
            break;
        case INPUT_CONFIG_LIMIT_MAX_CURRENT:
            update_func = &InputPin::limit_max_current;
            limit = pin_conf_limit >= 0 ? pin_conf_limit : 0;
            break;
        case INPUT_CONFIG_OVERRIDE_GRID_DRAW:
            update_func = &InputPin::override_grid_draw;
            limit = pin_conf_limit;
            break;
        default:
            logger.printfln("energy_manager/InputPin: Unknown INPUT_CONFIG type %u for input %u", pin_conf_func, num_name);
            /* FALLTHROUGH */
        case INPUT_CONFIG_DISABLED:
        case INPUT_CONFIG_CONTACTOR_CHECK:
            update_func = &InputPin::nop;
            break;
    }
}

void InputPin::update(bool level)
{
    (this->*update_func)(level);
}

void InputPin::nop(bool level)
{
    (void)level;
}

void InputPin::block_charging(bool level)
{
    *(uint8_t*)out_dst = level ^ invert_pin;
}

void InputPin::switch_excess_charging(bool level)
{
    *(bool*)out_dst = level ^ invert_pin;
}

void InputPin::limit_max_current(bool level)
{
    if (level ^ invert_pin)
        energy_manager.limit_max_current((uint32_t)limit);
}

void InputPin::override_grid_draw(bool level)
{
    if (level ^ invert_pin)
        energy_manager.override_grid_draw(limit);
}
