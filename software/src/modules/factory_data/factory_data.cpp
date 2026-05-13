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

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "tools/malloc.h"

static const char *factory_sku_str = "WARP4-CP-SS-2275-W"; // FIXME: read this from the actual factorydata partition

void FactoryData::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"sku_override", Config::Str("", 0, SKU_STR_LEN)},
    }), [this](Config &update, ConfigSource source) -> String {
        const char *sku_str = update.get("sku_override")->asEphemeralCStr();
        SKU sku;

        if (strlen(sku_str) > 0) {
            if (!parse_sku(sku_str, &sku)) {
                return "SKU override is malformed";
            }

            if (source != ConfigSource::File) {
                logger.printfln("Using SKU override: %s", sku_str);
            }
        }
        else {
            sku_str = factory_sku_str;

            if (!parse_sku(sku_str, &sku)) {
                return "Factory SKU is malformed";
            }

            if (source != ConfigSource::File) {
                logger.printfln("Using factory SKU: %s", sku_str);
            }
        }

        update_sku(sku_str, &sku);
        return "";
    }};

    state = Config::Object({
        {"sku", Config::Str("", 0, SKU_STR_LEN)},
        {"sku_product", Config::Enum(SKUProduct::Unknown)},
        {"sku_generation", Config::Enum(SKUGeneration::Unknown)},
        {"sku_model", Config::Enum(SKUModel::Unknown)},
        {"sku_material", Config::Enum(SKUMaterial::Unknown)},
        {"sku_type2_power", Config::Enum(SKUType2Power::Unknown)},
        {"sku_type2_length", Config::Enum(SKUType2Length::Unknown)},
        {"sku_engraving", Config::Enum(SKUEngraving::Unknown)},
    });
}

void FactoryData::setup()
{
    data = perm_new_prefer<Data>(PSRAM, DRAM, _NONE);

    api.restorePersistentConfig("factory_data/config", &config);

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
        sku_str = factory_sku_str;
        sku_valid = parse_sku(sku_str, &sku);

        if (!sku_valid) {
            logger.printfln("Factory SKU is malformed: %s", sku_str);
        }
        else {
            logger.printfln("Using factory SKU: %s", sku_str);
        }
    }

    if (!sku_valid) {
        logger.printfln("No valid SKU available");
    }
    else {
        update_sku(sku_str, &sku);
    }

    initialized = true;
}

void FactoryData::register_urls()
{
    api.addPersistentConfig("factory_data/config", &config);
    api.addState("factory_data/state", &state);
}

bool FactoryData::parse_sku(const char *sku_str, SKU *sku)
{
    // product generation   model   material   type2-power type2-length   engraving
    //
    //    WARP          4 -    CS -       SS -          11           50 -         W
    //                         CP         PC            11           75           C
    //                         CE                       22           50
    //                                                  22           75
    //                                                   C            C

    const char *p = sku_str;

    if (strncmp(p, "WARP", 4) == 0) {
        sku->product = SKUProduct::WARP;
        p += 4;
    }
    else {
        return false;
    }

    if (*p == '4') {
        sku->generation = SKUGeneration::Fourth;
        p += 1;
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
        sku->type2_power = SKUType2Power::KW11;
        sku->type2_length = SKUType2Length::M5p0;
        p += 4;
    }
    else if (strncmp(p, "1175", 4) == 0) {
        sku->type2_power = SKUType2Power::KW11;
        sku->type2_length = SKUType2Length::M7p5;
        p += 4;
    }
    else if (strncmp(p, "2250", 4) == 0) {
        sku->type2_power = SKUType2Power::KW22;
        sku->type2_length = SKUType2Length::M5p0;
        p += 4;
    }
    else if (strncmp(p, "2275", 4) == 0) {
        sku->type2_power = SKUType2Power::KW22;
        sku->type2_length = SKUType2Length::M7p5;
        p += 4;
    }
    else if (strncmp(p, "CC", 2) == 0) {
        sku->type2_power = SKUType2Power::Custom;
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

void FactoryData::update_sku(const char *sku_str, SKU *sku)
{
    snprintf(data->sku_str, std::size(data->sku_str), "%s", sku_str);
    data->sku = *sku;

    state.get("sku")->updateString(data->sku_str);
    state.get("sku_product")->updateEnum(data->sku.product);
    state.get("sku_generation")->updateEnum(data->sku.generation);
    state.get("sku_model")->updateEnum(data->sku.model);
    state.get("sku_material")->updateEnum(data->sku.material);
    state.get("sku_type2_power")->updateEnum(data->sku.type2_power);
    state.get("sku_type2_length")->updateEnum(data->sku.type2_length);
    state.get("sku_engraving")->updateEnum(data->sku.engraving);
}
