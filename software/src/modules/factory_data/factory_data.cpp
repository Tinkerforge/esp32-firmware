/* esp32-firmware
 * Copyright (C) 2026 Matthias Bolte <matthias@tinkerforge.com>
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

#include "factory_data.h"

#include <LittleFS.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "options.h"
#include "tools/malloc.h"

void FactoryData::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"sku_override", Config::Str("", 0, SKU_STR_MAX_LEN)},
    }), [this](Config &update, ConfigSource source) -> String {
        const char *sku_str = update.get("sku_override")->asEphemeralCStr();
        SKU sku;

        if (strlen(sku_str) > 0 && !parse_sku(sku_str, &sku)) {
            return "SKU override is malformed";
        }

        if (source != ConfigSource::File) {
            task_scheduler.scheduleOnce([this]() {
                apply_config();
            });
        }

        return "";
    }};

    state = Config::Object({
        {"sku", Config::Str("", 0, SKU_STR_MAX_LEN)},
        {"sku_product", Config::Enum(SKUProduct::Unknown)},
        {"sku_model", Config::Enum(SKUModel::Unknown)},
        {"sku_material", Config::Enum(SKUMaterial::Unknown)},
        {"sku_type2", Config::Enum(SKUType2::Unknown)},
        {"sku_engraving", Config::Enum(SKUEngraving::Unknown)},
    });

    write_sku_config = ConfigRoot{Config::Object({
        {"sku", Config::Str("", 0, SKU_STR_MAX_LEN)},
    }), [this](Config &update, ConfigSource source) -> String {
        const char *sku_str = update.get("sku")->asEphemeralCStr();
        SKU sku;

        if (!parse_sku(sku_str, &sku)) {
            return "SKU is malformed";
        }

        return "";
    }};
}

void FactoryData::setup()
{
    data = perm_new_prefer<Data>(PSRAM, DRAM, _NONE);

    fs::LittleFSFS factorydata;

    if (!factorydata.begin(false, "/factorydata", 10, "factorydata")) {
        logger.printfln("Could not mount factorydata partition");
    }
    else {
        File sku = factorydata.open("/sku");
        size_t read = sku.read(reinterpret_cast<uint8_t *>(data->factory_sku_str), std::size(data->factory_sku_str));

        sku.close();
        factorydata.end();

        if (read == 0) {
            logger.printfln("Could not read factory SKU");
        }

        data->factory_sku_str[read] = '\0';
    }

    api.restorePersistentConfig("factory_data/config", &config);

    apply_config();

    initialized = true;
}

void FactoryData::register_urls()
{
    api.addPersistentConfig("factory_data/config", &config);
    api.addState("factory_data/state", &state);

#if OPTIONS_FACTORY_DATA_ENABLE_WRITE_API()
    api.addCommand("factory_data/write_sku", &write_sku_config, {}, [this](Language language, String &errmsg) {
        const char *sku_str = write_sku_config.get("sku")->asEphemeralCStr();
        size_t sku_str_len = strlen(sku_str);

        fs::LittleFSFS factorydata;

        if (!factorydata.begin(true, "/factorydata", 10, "factorydata")) {
            errmsg = "Could not mount factorydata partition";
        }
        else {
            File sku = factorydata.open("/sku", FILE_WRITE);
            size_t written = sku.write(reinterpret_cast<const uint8_t *>(sku_str), sku_str_len);

            sku.close();
            factorydata.end();

            if (written != sku_str_len) {
                errmsg = "Could not write factory SKU";
            }
            else {
                snprintf(data->factory_sku_str, std::size(data->factory_sku_str), "%s", sku_str);
                apply_config();
            }
        }
    }, true);
#endif
}

bool FactoryData::parse_sku(const char *sku_str, SKU *sku)
{
    // product   model   material   type2   engraving
    //
    //   WARP4 -    CS -       SS -  1150 -         W
    //              CP         PC    1175           C
    //              CE               2250
    //                               2275
    //                                 CC

    const char *p = sku_str;

    if (strncmp(p, "WARP4", 5) == 0) {
        sku->product = SKUProduct::WARP4;
        p += 5;
    }
    else {
        return false;
    }

    if (*p++ != '-') {
        return false;
    }

    if (strncmp(p, "CS", 2) == 0) {
        sku->model = SKUModel::Smart;
        p += 2;
    }
    else if (strncmp(p, "CP", 2) == 0) {
        sku->model = SKUModel::Pro;
        p += 2;
    }
    else if (strncmp(p, "CE", 2) == 0) {
        sku->model = SKUModel::Eichrecht;
        p += 2;
    }
    else {
        return false;
    }

    if (*p++ != '-') {
        return false;
    }

    if (strncmp(p, "SS", 2) == 0) {
        sku->material = SKUMaterial::StainlessSteel;
        p += 2;
    }
    else if (strncmp(p, "PC", 2) == 0) {
        sku->material = SKUMaterial::PowderCoated;
        p += 2;
    }
    else {
        return false;
    }

    if (*p++ != '-') {
        return false;
    }

    if (strncmp(p, "1150", 4) == 0) {
        sku->type2 = SKUType2::Cable1150;
        p += 4;
    }
    else if (strncmp(p, "1175", 4) == 0) {
        sku->type2 = SKUType2::Cable1175;
        p += 4;
    }
    else if (strncmp(p, "2250", 4) == 0) {
        sku->type2 = SKUType2::Cable2250;
        p += 4;
    }
    else if (strncmp(p, "2275", 4) == 0) {
        sku->type2 = SKUType2::Cable2275;
        p += 4;
    }
    else if (strncmp(p, "CC", 2) == 0) {
        sku->type2 = SKUType2::CableCustom;
        p += 2;
    }
    else {
        return false;
    }

    if (*p++ != '-') {
        return false;
    }

    if (*p == 'W') {
        sku->engraving = SKUEngraving::WARP;
        p += 1;
    }
    else if (*p == 'C') {
        sku->engraving = SKUEngraving::Custom;
        p += 1;
    }
    else {
        return false;
    }

    if (*p != '\0') {
        return false;
    }

    return true;
}

void FactoryData::apply_config()
{
    SKU sku;
    const char *sku_str = config.get("sku_override")->asEphemeralCStr();
    bool sku_valid = false;

    if (strlen(sku_str) > 0) {
        sku_valid = parse_sku(sku_str, &sku);

        if (!sku_valid) {
            logger.printfln("Ignoring malformed SKU override: %s", sku_str);
        }
        else {
            logger.printfln("Using SKU override: %s", sku_str);
        }
    }

    if (!sku_valid) {
        sku_str = data->factory_sku_str;

        if (strlen(sku_str) == 0) {
            logger.printfln("Factory SKU is empty");
        }
        else {
            sku_valid = parse_sku(sku_str, &sku);

            if (!sku_valid) {
                logger.printfln("Factory SKU is malformed: %s", sku_str);
            }
            else {
                logger.printfln("Using factory SKU: %s", sku_str);
            }
        }
    }

    if (!sku_valid) {
        logger.printfln("No valid SKU available");

        sku_str = "";
        sku = SKU();
    }

    snprintf(data->sku_str, std::size(data->sku_str), "%s", sku_str);
    data->sku = sku;

    state.get("sku")->updateString(data->sku_str);
    state.get("sku_product")->updateEnum(data->sku.product);
    state.get("sku_model")->updateEnum(data->sku.model);
    state.get("sku_material")->updateEnum(data->sku.material);
    state.get("sku_type2")->updateEnum(data->sku.type2);
    state.get("sku_engraving")->updateEnum(data->sku.engraving);
}
