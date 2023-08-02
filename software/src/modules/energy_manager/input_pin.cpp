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
#include "module_dependencies.h"
#include "tools.h"
#include "energy_manager.h"

#include "gcc_warnings.h"

extern EnergyManager energy_manager;

InputPin::InputPin(uint32_t num_name, uint32_t num_logic, const ConfigRoot &conf, bool level_init) : prev_level(level_init)
{
    String pin_func_str            = String("input") + num_name + "_rule_then";
    String pin_limit_str           = String("input") + num_name + "_rule_then_limit";
    String pin_when_str            = String("input") + num_name + "_rule_is";
    String pin_on_high_str     = String("input") + num_name + "_rule_then_on_high";
    String pin_on_low_str    = String("input") + num_name + "_rule_then_on_low";
    uint32_t pin_conf_func         = conf.get(pin_func_str )->asUint();
    uint32_t pin_conf_limit_ma     = conf.get(pin_limit_str)->asUint();
    uint32_t pin_conf_when         = conf.get(pin_when_str)->asUint();
    uint32_t pin_conf_on_high  = conf.get(pin_on_high_str)->asUint();
    uint32_t pin_conf_on_low = conf.get(pin_on_low_str)->asUint();

    invert_pin = pin_conf_when == INPUT_CONFIG_WHEN_LOW;

    switch(pin_conf_func) {
        case INPUT_CONFIG_BLOCK_CHARGING:
            if (num_logic >= ARRAY_SIZE(energy_manager.charging_blocked.pin)) {
                logger.printfln("energy_manager/InputPin: num_logic too large for input %u: %u >= %u", num_name, num_logic, ARRAY_SIZE(energy_manager.charging_blocked.pin));
            } else {
                update_func = &InputPin::block_charging;
                out_dst = &(energy_manager.charging_blocked.pin[num_logic]);
            }
            break;
        case INPUT_CONFIG_LIMIT_MAX_CURRENT:
            update_func = &InputPin::limit_max_current;
            limit_ma = pin_conf_limit_ma;
            break;
        case INPUT_CONFIG_SWITCH_MODE:
            update_func = &InputPin::switch_mode;
            rising_mode  = pin_conf_on_high;
            falling_mode = pin_conf_on_low;
            break;
        default:
            logger.printfln("energy_manager/InputPin: Unknown INPUT_CONFIG type %u for input %u", pin_conf_func, num_name);
            /* FALLTHROUGH */
        case INPUT_CONFIG_DISABLED:
        case INPUT_CONFIG_CONTACTOR_CHECK:
            // update_func already set to nop.
            break;
    }

}

void InputPin::update(bool level)
{
    (this->*update_func)(level);
    prev_level = level;
}

void InputPin::nop(bool level)
{
    (void)level;
}

void InputPin::block_charging(bool level)
{
    *out_dst = level ^ invert_pin;
}

void InputPin::limit_max_current(bool level)
{
    if (level ^ invert_pin)
        energy_manager.limit_max_current(limit_ma);
}

void InputPin::switch_mode(bool level)
{
    // Only detect edges, do nothing if there's no change.
    if (level == prev_level)
        return;

    energy_manager.switch_mode(level ? rising_mode : falling_mode);
}
