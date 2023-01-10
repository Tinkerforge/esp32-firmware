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

    String pin_func_str = String("input") + num_name + "_config";
    String pin_when_str = String("input") + num_name + "_config_when";
    uint32_t pin_conf_function  = conf.get(pin_func_str )->asUint();
    uint32_t pin_conf_when      = conf.get(pin_when_str)->asUint();

    switch(pin_conf_function) {
        case INPUT_CONFIG_BLOCK_CHARGING:
            update_func = &InputPin::block_charging;
            invert_pin = pin_conf_when == INPUT_CONFIG_WHEN_LOW;
            out_dst = &(energy_manager.charging_blocked.pin[num_logic]);
            break;
        case INPUT_CONFIG_EXCESS_CHARGING:
            update_func = & InputPin::switch_excess_charging;
            invert_pin = pin_conf_when == INPUT_CONFIG_WHEN_LOW;
            out_dst = &(energy_manager.excess_charging_enable);
            break;
        default:
            logger.printfln("energy_manager/InputPin: Unknown INPUT_CONFIG type %u for input %u", pin_conf_function, num_name);
            /* FALLTHROUGH */
        case INPUT_CONFIG_DISABLED:
        case INPUT_CONFIG_CONTACTOR_CHECK:
            update_func = &InputPin::nop;
            break;
    }
};

void InputPin::update(bool level)
{
    (this->*update_func)(level);
}

void InputPin::nop(bool level)
{
    (void)level;
};

void InputPin::block_charging(bool level)
{
    *(uint8_t*)out_dst = level ^ invert_pin;
};

void InputPin::switch_excess_charging(bool level)
{
    *(bool*)out_dst = level ^ invert_pin;
};
