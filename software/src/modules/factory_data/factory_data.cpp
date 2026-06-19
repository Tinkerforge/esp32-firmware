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
#include <TFJson.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "options.h"
#include "tools/printf.h"
#include "tools/malloc.h"

#include "gcc_warnings.h"

static const char *validate_nfc_tags(Config *nfc_tags)
{
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
}

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
            Config::type_id<Config::ConfObject>()
        )},
    }),
    [this](Config &update, ConfigSource source) -> String {
        const char *sku_str = update.get("sku")->asEphemeralCStr();
        SKU sku;

        if (!parse_sku(sku_str, &sku)) {
            return "SKU is malformed";
        }

        return validate_nfc_tags(static_cast<Config *>(update.get("nfc_tags")));
    }};
}

void FactoryData::setup()
{
    data = perm_new_prefer<Data>(PSRAM, DRAM, _NONE);

    if (!read_sku(data->factory_sku_str)) {
        data->factory_sku_str[0] = '\0';
    }

    api.restorePersistentConfig("factory_data/config", &config);

    apply_config();

    initialized = true;
}

void FactoryData::register_urls()
{
    api.addPersistentConfig("factory_data/config", &config);
    api.addState("factory_data/state", &state);

    server.on("/factory_data/read", HTTP_GET, [this](WebServerRequest request) {
        char json_buf[256] = "";
        TFJsonSerializer json{json_buf, sizeof(json_buf)};

        char sku_str[FACTORY_DATA_SKU_STR_MAX_LEN + 1];

        if (!read_sku(sku_str)) {
            sku_str[0] = '\0';
        }

        json.addObject();
        json.addMemberString("sku", sku_str);

        NFCTag nfc_tags[FACTORY_DATA_NFC_TAG_MAX_COUNT];
        size_t nfc_tag_count = read_nfc_tags(nfc_tags);

        json.addMemberArray("nfc_tags");

        for (size_t i = 0; i < nfc_tag_count; ++i) {
            json.addObject();
            json.addMemberNumber("tag_type", nfc_tags[i].tag_type);
            json.addMemberString("tag_id", nfc_tags[i].tag_id);
            json.endObject();
        }

        json.endArray();
        json.endObject();
        json.end();

        return request.send_json(200, json);
    });

#if OPTIONS_FACTORY_DATA_ENABLE_WRITE_API()
    api.addCommand("factory_data/write", &write_config, {}, [this](Language language, String &errmsg) {
        fs::LittleFSFS factorydata_fs;

        {
            LogSilencer ls;

            if (!factorydata_fs.begin(true, "/factorydata", 10, "factorydata")) {
                errmsg = "Could not mount factorydata partition to write factory data";
                return;
            }
        }

        // SKU
        if (factorydata_fs.exists("/sku.tmp")) {
            factorydata_fs.remove("/sku.tmp");
        }

        const char *sku_str = write_config.get("sku")->asEphemeralCStr();
        size_t sku_str_len = strlen(sku_str);
        File sku_file = factorydata_fs.open("/sku.tmp", FILE_WRITE);
        size_t written = sku_file.write(reinterpret_cast<const uint8_t *>(sku_str), sku_str_len);
        sku_file.close();

        if (written != sku_str_len) {
            errmsg = "Could not write factory SKU file";
            factorydata_fs.end();
            return;
        }

        if (factorydata_fs.exists("/sku")) {
            factorydata_fs.remove("/sku");
        }

        if (!factorydata_fs.rename("/sku.tmp", "/sku")) {
            errmsg = "Could not rename factory SKU file";
            factorydata_fs.end();
            return;
        }

        // NFC tags
        if (factorydata_fs.exists("/nfc_tags.tmp")) {
            factorydata_fs.remove("/nfc_tags.tmp");
        }

        File nfc_tags_file = factorydata_fs.open("/nfc_tags.tmp", FILE_WRITE);
        static_cast<Config *>(write_config.get("nfc_tags"))->save_to_file(nfc_tags_file);
        nfc_tags_file.close();

        if (factorydata_fs.exists("/nfc_tags")) {
            factorydata_fs.remove("/nfc_tags");
        }

        if (!factorydata_fs.rename("/nfc_tags.tmp", "/nfc_tags")) {
            errmsg = "Could not rename factory NFC tags file";
            factorydata_fs.end();
            return;
        }

        factorydata_fs.end();

        // apply new factory data
        snprintf(data->factory_sku_str, sizeof(data->factory_sku_str), "%s", sku_str);
        apply_config();
    }, true);
#endif
}

bool FactoryData::read_sku(char sku_str[FACTORY_DATA_SKU_STR_MAX_LEN + 1])
{
    fs::LittleFSFS factorydata_fs;

    {
        LogSilencer ls;

        if (!factorydata_fs.begin(false, "/factorydata", 10, "factorydata")) {
            logger.printfln("Could not mount factorydata partition to read factory SKU");
            return false;
        }
    }

    if (!factorydata_fs.exists("/sku")) {
        factorydata_fs.end();
        logger.printfln("Factory SKU is missing");
        return false;
    }

    File sku_file = factorydata_fs.open("/sku");
    size_t read = sku_file.read(reinterpret_cast<uint8_t *>(sku_str), FACTORY_DATA_SKU_STR_MAX_LEN);
    sku_str[read] = '\0';
    sku_file.close();
    factorydata_fs.end();

    return read > 0;
}

size_t FactoryData::read_nfc_tags(NFCTag nfc_tags[FACTORY_DATA_NFC_TAG_MAX_COUNT])
{
    fs::LittleFSFS factorydata_fs;

    {
        LogSilencer ls;

        if (!factorydata_fs.begin(false, "/factorydata", 10, "factorydata")) {
            logger.printfln("Could not mount factorydata partition to read factory NFC tags");
            return 0;
        }
    }

    if (!factorydata_fs.exists("/nfc_tags")) {
        factorydata_fs.end();
        logger.printfln("Factory NFC tags are missing");
        return 0;
    }

    ConfigRoot nfc_tags_config = ConfigRoot{Config::Array(
        {},
        &nfc_tag_prototype,
        0, FACTORY_DATA_NFC_TAG_MAX_COUNT,
        Config::type_id<Config::ConfObject>()
    ),
    [this](Config &update, ConfigSource source) -> String {
        return validate_nfc_tags(&update);
    }};

    String error = nfc_tags_config.update_from_file(factorydata_fs.open("/nfc_tags"));

    factorydata_fs.end();

    if (!error.isEmpty()) {
        logger.printfln("Could not read factory NFC tags: %s", error.c_str());
        return 0;
    }

    size_t nfc_tags_count = nfc_tags_config.count();

    for (size_t i = 0; i < nfc_tags_count; ++i) {
        nfc_tags[i].tag_type = nfc_tags_config.get(i)->get("tag_type")->asUint8();
        snprintf(nfc_tags[i].tag_id, sizeof(nfc_tags[i].tag_id), "%s", nfc_tags_config.get(i)->get("tag_id")->asEphemeralCStr());
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

    snprintf(data->sku_str, sizeof(data->sku_str), "%s", sku_str);
    data->sku = sku;

    state.get("sku")->updateString(data->sku_str);
    state.get("sku_product")->updateEnum(data->sku.product);
    state.get("sku_model")->updateEnum(data->sku.model);
    state.get("sku_material")->updateEnum(data->sku.material);
    state.get("sku_type2")->updateEnum(data->sku.type2);
    state.get("sku_engraving")->updateEnum(data->sku.engraving);
}
