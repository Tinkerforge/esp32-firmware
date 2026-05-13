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
#include "generated/sku_product.enum.h"
#include "generated/sku_generation.enum.h"
#include "generated/sku_model.enum.h"
#include "generated/sku_material.enum.h"
#include "generated/sku_type2_power.enum.h"
#include "generated/sku_type2_length.enum.h"
#include "generated/sku_engraving.enum.h"

#define SKU_STR_LEN 18 // WARP4-XX-XX-XXXX-X

class FactoryData final : public IModule
{
public:
    FactoryData() {}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    inline const char *get_sku() const { return data->sku_str; }
    inline SKUProduct get_sku_product() const { return data->sku.product; }
    inline SKUGeneration get_sku_generation() const { return data->sku.generation; }
    inline SKUModel get_sku_model() const { return data->sku.model; }
    inline SKUMaterial get_sku_material() const { return data->sku.material; }
    inline SKUType2Power get_sku_type2_power() const { return data->sku.type2_power; }
    inline SKUType2Length get_sku_type2_length() const { return data->sku.type2_length; }
    inline SKUEngraving get_sku_engraving() const { return data->sku.engraving; }

private:
    struct SKU {
        SKUProduct product = SKUProduct::Unknown;
        SKUGeneration generation = SKUGeneration::Unknown;
        SKUModel model = SKUModel::Unknown;
        SKUMaterial material = SKUMaterial::Unknown;
        SKUType2Power type2_power = SKUType2Power::Unknown;
        SKUType2Length type2_length = SKUType2Length::Unknown;
        SKUEngraving engraving = SKUEngraving::Unknown;
    };

    struct Data {
        char sku_str[SKU_STR_LEN + 1] = "";
        SKU sku;
    };

    bool parse_sku(const char *sku_str, SKU *sku);
    void update_sku(const char *sku_str, SKU *sku);

    ConfigRoot config;
    ConfigRoot state;

    Data *data = nullptr;
};
