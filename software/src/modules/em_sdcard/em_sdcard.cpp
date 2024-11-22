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

#include "em_sdcard.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"

#include "gcc_warnings.h"

void EMSDcard::pre_setup()
{
    state = Config::Object({
        {"sd_status",       Config::Uint32(0)},
        {"lfs_status",      Config::Uint32(0)},
        {"card_type",       Config::Uint32(0)},
        {"sector_count",    Config::Uint32(0)},
        {"sector_size",     Config::Uint16(0)},
        {"manufacturer_id", Config::Uint8(0)},
        {"product_rev",     Config::Uint8(0)},
        {"product_name",    Config::Str(".....", 5, 5)},
    });
}

void EMSDcard::setup()
{
    if (!em_common.initialized)
        return;

    update_sdcard_info();

    task_scheduler.scheduleWithFixedDelay([this](){
        update_sdcard_info();
    }, 10_s, 10_s);

    initialized = true;
}

void EMSDcard::register_urls()
{
    api.addState("energy_manager/sdcard_state", &state);

    api.addCommand("energy_manager/sdcard_format", Config::Confirm(), {Config::confirm_key}, [this](String &errmsg) {
        if (!Config::Confirm()->get(Config::ConfirmKey())->asBool()) {
            errmsg = "SD card format NOT initiated";
            return;
        }

        logger.printfln("Formatting SD card...");
        if (!em_common.format_sdcard()) {
            errmsg = "Format request failed";
            return;
        }

        // Fake LittleFS state to display "Formatting..." message in frontend.
        state.get("lfs_status")->updateUint(256);
    }, true);
}

void EMSDcard::update_sdcard_info()
{
    struct sdcard_info data;

    if (em_common.get_sdcard_info(&data)) {
        state.get("sd_status")->updateUint(data.sd_status);
        state.get("lfs_status")->updateUint(data.lfs_status);
        state.get("card_type")->updateUint(data.card_type);
        state.get("sector_count")->updateUint(data.sector_count);
        state.get("sector_size")->updateUint(data.sector_size);
        state.get("manufacturer_id")->updateUint(data.manufacturer_id);
        state.get("product_rev")->updateUint(data.product_rev);
        state.get("product_name")->updateString(data.product_name);
    } else {
        state.get("sd_status")->updateUint(255);
        state.get("lfs_status")->updateUint(255);
        state.get("card_type")->updateUint(0);
        state.get("sector_count")->updateUint(0);
        state.get("sector_size")->updateUint(0);
        state.get("manufacturer_id")->updateUint(0);
        state.get("product_rev")->updateUint(0);
        state.get("product_name")->updateString("None.");
    }
}
