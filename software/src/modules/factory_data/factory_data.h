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

#pragma once

#include "module.h"
#include "config.h"
#include "options.h"
#include "generated/sku_product.enum.h"
#include "generated/sku_model.enum.h"
#include "generated/sku_material.enum.h"
#include "generated/sku_type2.enum.h"
#include "generated/sku_engraving.enum.h"

#define FACTORY_DATA_SKU_STR_MAX_LEN 18 // WARP4-XX-XX-XXXX-X

#define FACTORY_DATA_NFC_TAG_MAX_COUNT 3

#define FACTORY_DATA_NFC_TAG_ID_LENGTH 10
#define FACTORY_DATA_NFC_TAG_ID_STRING_LENGTH (FACTORY_DATA_NFC_TAG_ID_LENGTH * 3 - 1)

class FactoryData final : public IModule
{
public:
    FactoryData() {}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    inline const char *get_sku() const            { return state.get("sku")->asEphemeralCStr(); }
    inline SKUProduct get_sku_product() const     { return state.get("sku_product")->asEnum<SKUProduct>(); }
    inline SKUModel get_sku_model() const         { return state.get("sku_model")->asEnum<SKUModel>(); }
    inline SKUMaterial get_sku_material() const   { return state.get("sku_material")->asEnum<SKUMaterial>(); }
    inline SKUType2 get_sku_type2() const         { return state.get("sku_type2")->asEnum<SKUType2>(); }
    inline SKUEngraving get_sku_engraving() const { return state.get("sku_engraving")->asEnum<SKUEngraving>(); }

    struct NFCTag {
        uint8_t tag_type = 255;
        char tag_id[FACTORY_DATA_NFC_TAG_ID_STRING_LENGTH + 1] = "";
    };

    bool read_factory_sku(char sku_str[FACTORY_DATA_SKU_STR_MAX_LEN + 1]);
    size_t read_factory_nfc_tags(NFCTag nfc_tags[FACTORY_DATA_NFC_TAG_MAX_COUNT]);

private:
    struct SKU {
        SKUProduct product = SKUProduct::Unknown;
        SKUModel model = SKUModel::Unknown;
        SKUMaterial material = SKUMaterial::Unknown;
        SKUType2 type2 = SKUType2::Unknown;
        SKUEngraving engraving = SKUEngraving::Unknown;
    };

    bool parse_sku(const char *sku_str, SKU *sku);
    void apply_config();

    ConfigRoot config;
    ConfigRoot state;
    Config nfc_tag_prototype;
#if OPTIONS_FACTORY_DATA_ENABLE_WRITE_API()
    ConfigRoot write_config;
#endif
};
