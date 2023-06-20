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

#include "api.h"
#include "modules.h"
#include "task_scheduler.h"

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
    if (!energy_manager.initialized)
        return;

    update_sdcard_info();

    task_scheduler.scheduleWithFixedDelay([this](){
        update_sdcard_info();
    }, 10 * 1000, 10 * 1000); // 10s

    initialized = true;
}

void EMSDcard::register_urls()
{
    api.addState("energy_manager/sdcard_state", &state, {}, 1000);

    api.addRawCommand("energy_manager/sdcard_format", [this](char *c, size_t s) -> String {
        StaticJsonDocument<16> doc;
        DeserializationError error = deserializeJson(doc, c, s);

        if (error) {
            return String("Failed to deserialize string: ") + error.c_str();
        }

        if (!doc["do_i_know_what_i_am_doing"].is<bool>()) {
            return "You don't seem to know what you are doing";
        }

        if (!doc["do_i_know_what_i_am_doing"].as<bool>()) {
            return "SD card format NOT initiated";
        }

        logger.printfln("em_sdcard: Formatting SD card...");
        if (!energy_manager.format_sdcard())
            return "Format request failed";

        // Fake LittleFS state to display "Formatting..." message in frontend.
        state.get("lfs_status")->updateUint(256);

        return "";
    }, true);
}

void EMSDcard::update_sdcard_info()
{
    struct sdcard_info data;

    if (energy_manager.get_sdcard_info(&data)) {
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
