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

#include "em_common.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"

EMCommon::EMCommon()
{
#if MODULE_ENERGY_MANAGER_AVAILABLE()
    backend = &energy_manager;
//#elif MODULE_EVSE_V2_AVAILABLE()
//    backend = &em_v2;
#else
    #error Unknown or missing Energy Manager module!
#endif
}

void EMCommon::setup()
{
    if (!backend->is_initialized())
        return;

    //backend->post_setup();
    initialized = true;
}

bool EMCommon::device_module_is_in_bootloader(int rc)
{
    return backend->device_module_is_in_bootloader(rc);
}

uint32_t EMCommon::get_em_version()
{
    return backend->get_em_version();
}

void EMCommon::set_time(const tm &tm)
{
    backend->set_time(tm);
}

timeval EMCommon::get_time()
{
    return backend->get_time();
}

bool EMCommon::get_sdcard_info(struct sdcard_info *data)
{
    return backend->get_sdcard_info(data);
}

bool EMCommon::format_sdcard()
{
    return backend->format_sdcard();
}

uint16_t EMCommon::get_energy_meter_detailed_values(float *ret_values)
{
    return backend->get_energy_meter_detailed_values(ret_values);
}

bool EMCommon::reset_energy_meter_relative_energy()
{
    return backend->reset_energy_meter_relative_energy();
}

void EMCommon::clr_error(uint32_t error_mask)
{
    error_flags &= ~error_mask;
    state.get("error_flags")->updateUint(error_flags);

#if MODULE_ENERGY_MANAGER_AVAILABLE()
    energy_manager.update_status_led();
#endif
}

bool EMCommon::is_error(uint32_t error_bit_pos) const
{
    return (error_flags >> error_bit_pos) & 1;
}

void EMCommon::set_error(uint32_t error_mask)
{
    error_flags |= error_mask;
    state.get("error_flags")->updateUint(error_flags);

#if MODULE_ENERGY_MANAGER_AVAILABLE()
    energy_manager.update_status_led();
#endif
}

void EMCommon::set_config_error(uint32_t config_error_mask)
{
    config_error_flags |= config_error_mask;
    state.get("config_error_flags")->updateUint(config_error_flags);

    set_error(ERROR_FLAGS_BAD_CONFIG_MASK);
}

void EMCommon::check_bricklet_reachable(int rc, const char *context)
{
    if (rc == TF_E_OK) {
        consecutive_bricklet_errors = 0;
        if (!bricklet_reachable) {
            bricklet_reachable = true;
            clr_error(ERROR_FLAGS_BRICKLET_MASK);
            logger.printfln("Bricklet is reachable again.");
        }
    } else {
        if (rc == TF_E_TIMEOUT) {
            logger.printfln("%s: Bricklet access timed out.", context);
        } else {
            logger.printfln("%s: Bricklet access returned error %d.", context, rc);
        }
        if (bricklet_reachable && ++consecutive_bricklet_errors >= 8) {
            bricklet_reachable = false;
            set_error(ERROR_FLAGS_BRICKLET_MASK);
            logger.printfln("%s: Bricklet is unreachable.", context);
        }
    }
    low_level_state.get("consecutive_bricklet_errors")->updateUint(consecutive_bricklet_errors);
}

#if MODULE_AUTOMATION_AVAILABLE()
bool EMCommon::has_triggered(const Config *conf, void *data)
{
    return false;
}
#endif
