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
#include "tools/printf.h"
#include "tools/malloc.h"

#include "gcc_warnings.h"

void FactoryData::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"sku_override", Config::Str("", 0, FACTORY_DATA_SKU_STR_MAX_LEN)},
    }),
    [this](Config &update, ConfigSource source) -> String {
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
        {"sku", Config::Str("", 0, FACTORY_DATA_SKU_STR_MAX_LEN)},
        {"sku_product", Config::Enum(SKUProduct::Unknown)},
        {"sku_model", Config::Enum(SKUModel::Unknown)},
        {"sku_material", Config::Enum(SKUMaterial::Unknown)},
        {"sku_type2", Config::Enum(SKUType2::Unknown)},
        {"sku_engraving", Config::Enum(SKUEngraving::Unknown)},
    });

    nfc_tag_prototype = Config::Object({
        {"tag_type", Config::Uint(0, 0, 5)},
        {"tag_id", Config::Str("", 0, FACTORY_DATA_NFC_TAG_ID_STRING_LENGTH)}
    });

    write_config = ConfigRoot{Config::Object({
        {"sku", Config::Str("", 0, FACTORY_DATA_SKU_STR_MAX_LEN)},
        {"nfc_tags", Config::Array(
            {},
            &nfc_tag_prototype,
            0, FACTORY_DATA_NFC_TAG_MAX_COUNT,
            Config::type_id<Config::ConfObject>())
        },
    }),
    [this](Config &update, ConfigSource source) -> String {
        const char *sku_str = update.get("sku")->asEphemeralCStr();
        SKU sku;

        if (!parse_sku(sku_str, &sku)) {
            return "SKU is malformed";
        }

        Config *nfc_tags = static_cast<Config *>(update.get("nfc_tags"));
        size_t nfc_tags_count = nfc_tags->count();

        for (size_t i = 0; i < nfc_tags_count; ++i) {
            String nfc_tag_id_copy = nfc_tags->get(i)->get("tag_id")->asString();
            nfc_tag_id_copy.toUpperCase();
            nfc_tags->get(i)->get("tag_id")->updateString(nfc_tag_id_copy);

            if (nfc_tag_id_copy.length() % 3 != 2) {
                return "NFC tag ID has unexpected length";
            }

            for (size_t k = 0; k < nfc_tag_id_copy.length(); ++k) {
                char c = nfc_tag_id_copy.charAt(k);

                if ((k % 3) != 2 && ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
                    continue;
                }

                if ((k % 3) == 2 && c == ':') {
                    continue;
                }

                return "NFC tag ID contains unexpected character";
            }
        }

        return "";
    }};
}

void FactoryData::setup()
{
    data = perm_new_prefer<Data>(PSRAM, DRAM, _NONE);

    fs::LittleFSFS factorydata_fs;
    bool factorydata_mounted;

    {
        LogSilencer ls;

        factorydata_mounted = factorydata_fs.begin(false, "/factorydata", 10, "factorydata");
    }

    if (!factorydata_mounted) {
        logger.printfln("Could not mount factorydata partition to read factory SKU");
    }
    else {
        File sku_file = factorydata_fs.open("/sku");
        size_t read = sku_file.read(reinterpret_cast<uint8_t *>(data->factory_sku_str), std::size(data->factory_sku_str));
        data->factory_sku_str[read] = '\0';
        sku_file.close();

        if (read == 0) {
            logger.printfln("Could not read factory SKU");
        }

        factorydata_fs.end();
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
    api.addCommand("factory_data/write", &write_config, {}, [this](Language language, String &errmsg) {
        fs::LittleFSFS factorydata_fs;
        bool factorydata_mounted;

        {
            LogSilencer ls;

            factorydata_mounted = factorydata_fs.begin(true, "/factorydata", 10, "factorydata");
        }

        if (!factorydata_mounted) {
            errmsg = "Could not mount factorydata partition";
        }
        else {
            const char *sku_str = write_config.get("sku")->asEphemeralCStr();
            size_t sku_str_len = strlen(sku_str);

            File sku_file = factorydata_fs.open("/sku", FILE_WRITE);
            size_t written = sku_file.write(reinterpret_cast<const uint8_t *>(sku_str), sku_str_len);
            sku_file.close();

            if (written != sku_str_len) {
                errmsg = "Could not write factory SKU";
                factorydata_fs.end();
                return;
            }

            Config *nfc_tags = static_cast<Config *>(write_config.get("nfc_tags"));
            size_t nfc_tags_count = nfc_tags->count();
            char nfc_tags_str[(1 + 1 + FACTORY_DATA_NFC_TAG_ID_STRING_LENGTH + 1) * FACTORY_DATA_NFC_TAG_MAX_COUNT + 1];
            size_t nfc_tags_str_len = 0;

            for (size_t i = 0; i < nfc_tags_count; ++i) {
                nfc_tags_str_len += snprintf_u(nfc_tags_str + nfc_tags_str_len, sizeof(nfc_tags_str) - nfc_tags_str_len, "%u %s\n",
                                               nfc_tags->get(i)->get("tag_type")->asUint8(),
                                               nfc_tags->get(i)->get("tag_id")->asEphemeralCStr());
            }

            File nfc_tags_file = factorydata_fs.open("/nfc_tags", FILE_WRITE);
            written = nfc_tags_file.write(reinterpret_cast<const uint8_t *>(nfc_tags_str), nfc_tags_str_len);
            nfc_tags_file.close();

            if (written != nfc_tags_str_len) {
                errmsg = "Could not write factory NFC tags";
                factorydata_fs.end();
                return;
            }

            snprintf(data->factory_sku_str, std::size(data->factory_sku_str), "%s", sku_str);
            apply_config();
            factorydata_fs.end();
        }
    }, true);
#endif
}

size_t FactoryData::read_nfc_tags(NFCTag *nfc_tags)
{
    fs::LittleFSFS factorydata_fs;

    {
        LogSilencer ls;

        if (!factorydata_fs.begin(false, "/factorydata", 10, "factorydata")) {
            logger.printfln("Could not mount factorydata partition to read factory NFC tags");
            return 0;
        }
    }

    File nfc_tags_file = factorydata_fs.open("/nfc_tags");
    char nfc_tags_str[(1 + 1 + FACTORY_DATA_NFC_TAG_ID_STRING_LENGTH + 1) * FACTORY_DATA_NFC_TAG_MAX_COUNT + 1];
    size_t read = nfc_tags_file.read(reinterpret_cast<uint8_t *>(nfc_tags_str), sizeof(nfc_tags_str));
    nfc_tags_str[read] = '\0';
    nfc_tags_file.close();
    factorydata_fs.end();

    if (read == 0) {
        return 0;
    }

    size_t nfc_tags_count = 0;
    char *p = nfc_tags_str;

    while (*p != '\0' && nfc_tags_count < FACTORY_DATA_NFC_TAG_MAX_COUNT) {
        if (*p < '0' || *p > '5') {
            logger.printfln("Factory NFC tags file is malformed, NFC tag type missing: %s", nfc_tags_str);
            return 0;
        }

        nfc_tags[nfc_tags_count].tag_type = static_cast<int8_t>(*p - '0');

        ++p;

        if (*p != ' ') {
            logger.printfln("Factory NFC tags file is malformed, space missing: %s", nfc_tags_str);
            return 0;
        }

        ++p;

        char *n = strchr(p, '\n');

        if (n == nullptr) {
            logger.printfln("Factory NFC tags file is malformed, newline missing: %s", nfc_tags_str);
            return 0;
        }

        size_t nfc_tag_id_len = static_cast<size_t>(n - p);

        if (nfc_tag_id_len == 0) {
            logger.printfln("Factory NFC tags file is malformed, NFC tag ID missing: %s", nfc_tags_str);
            return 0;
        }

        if (nfc_tag_id_len % 3 != 2) {
            logger.printfln("Factory NFC tags file is malformed, NFC tag ID has unexpected length: %s", nfc_tags_str);
            return 0;
        }

        for (size_t i = 0; i < nfc_tag_id_len; ++i) {
            char *q = p + i;

            *q = static_cast<char>(toupper(*q));

            char c = *q;

            if ((i % 3) != 2 && ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
                continue;
            }

            if ((i % 3) == 2 && c == ':') {
                continue;
            }

            logger.printfln("Factory NFC tags file is malformed, NFC tag ID has unexpected character: %s", nfc_tags_str);
            return 0;
        }

        snprintf(nfc_tags[nfc_tags_count].tag_id, sizeof(nfc_tags[nfc_tags_count].tag_id), "%.*s", static_cast<int>(nfc_tag_id_len), p);

        p = n + 1;
        ++nfc_tags_count;
    }

    if (*p != '\0') {
        logger.printfln("Factory NFC tags file has trailing data: %s", p);
    }

    return nfc_tags_count;
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
