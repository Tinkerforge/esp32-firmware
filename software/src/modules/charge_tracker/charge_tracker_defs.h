/* esp32-firmware
 * Copyright (C) 2025 Frederic Henrichs <frederic@tinkerforge.com>
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
#include <cstdint>
#include <Arduino.h>

#include "language.h"
#include "modules/users/users.h"
#include "modules/web_server/web_server.h"

#define USER_FILTER_ALL_USERS -2
#define USER_FILTER_DELETED_USERS -1

#define DEVICE_FILTER_ALL_CHARGERS -2
#define DEVICE_FILTER_DELETED_CHARGERS -1

#define CHARGE_RECORD_FOLDER "/charge-records"

struct [[gnu::packed]] ChargeStart {
    uint32_t timestamp_minutes = 0;
    float meter_start = 0.0f;
    uint8_t user_id = 0;
};

static_assert(sizeof(ChargeStart) == 9, "Unexpected size of ChargeStart");

struct [[gnu::packed]] ChargeEnd {
    uint32_t charge_duration : 24;
    float meter_end = 0.0f;
};

static_assert(sizeof(ChargeEnd) == 7, "Unexpected size of ChargeEnd");

struct [[gnu::packed]] Charge {
    ChargeStart cs;
    ChargeEnd ce;
};

struct display_name_entry {
    void set(uint32_t len, char *buf) {
        this->length = len;
        const size_t copy_length = (len + 3) & -4; // Round up to IRAM-compatible multiple of 32bit.
        memcpy(this->name, buf, copy_length); // The compiler is hell-bent on using memcpy here, so let it have its way.
    }

    size_t get(char *buf) {
        if (this->length > 0) {
            // Can't use memcpy because that uses illegal IRAM reads when ret_buf is not aligned.
            uint32_t *src_start = this->name;
            uint32_t *src_end = src_start + (this->length + 3) / 4; // Round up to IRAM-compatible multiple of 32bit; copies some buffer slack behind termination that the caller must ignore.
            uint32_t *dst = reinterpret_cast<uint32_t *>(buf);

            while (src_start < src_end) {
                *dst++ = *src_start++;
            }
        }

        buf[this->length] = '\0';
        return this->length;
    }

    uint32_t length;
private:
    uint32_t name[32 / sizeof(uint32_t)]; // 32 == DISPLAY_NAME_LENGTH; can't include users.h here due to module_available guards. Validated by static_assert in charge_tracker.cpp.
};

struct charger_display_name_entry {
    void set(uint32_t charger_uid, uint32_t len, char *buf) {
        this->uid = charger_uid;
        this->length = len;
        const size_t copy_length = (len + 3) & -4; // Round up to IRAM-compatible multiple of 32bit.
        memcpy(this->name, buf, copy_length); // The compiler is hell-bent on using memcpy here, so let it have its way.
    }

    size_t get(char *buf) {
        if (this->length > 0) {
            // Can't use memcpy because that uses illegal IRAM reads when ret_buf is not aligned.
            uint32_t *src_start = this->name;
            uint32_t *src_end = src_start + (this->length + 3) / 4; // Round up to IRAM-compatible multiple of 32bit; copies some buffer slack behind termination that the caller must ignore.
            uint32_t *dst = reinterpret_cast<uint32_t *>(buf);

            while (src_start < src_end) {
                *dst++ = *src_start++;
            }
        }

        buf[this->length] = '\0';
        return this->length;
    }

    uint32_t uid;
    uint32_t length;
private:
    uint32_t name[32 / sizeof(uint32_t)]; // 32 == CHARGER_NAME_LENGTH; can't include charge_manager.h here due to module_available guards. Validated by static_assert in charge_tracker.cpp.
};

struct GenerationParams {
    GenerationParams() {}
    GenerationParams(const GenerationParams&) = delete;
    GenerationParams &operator=(const GenerationParams&) = delete;

    // Initialized in parse_request or manually
    int user = USER_FILTER_ALL_USERS;
    int device = DEVICE_FILTER_ALL_CHARGERS;
    uint32_t start_min = 0;
    uint32_t end_min = 0;
    uint32_t current_min = 0;
    Language language = Language::German;

    // Initialized in init()
    uint32_t electricity_price = 0;
    String display_name = "";
    String unique_device_name = "";
    uint8_t configured_users[MAX_ACTIVE_USERS] = {};
    display_name_entry *display_name_cache = nullptr;
    charger_display_name_entry *charger_display_name_cache = nullptr;

    size_t get_charger_display_name(uint32_t uid, char *buf) const;

    ~GenerationParams() {
        free_any(display_name_cache);
        free_any(charger_display_name_cache);
    }

    bool init();

    virtual bool parse_request(std::unique_ptr<char[]> &buf, StaticJsonDocument<192> &doc, WebServerRequest &request);
    bool include_charge(const Charge *charge) const;
};

struct PDFGenerationParams final : public GenerationParams {
    std::unique_ptr<char[]> letterhead_buf = nullptr;
    int letterhead_lines = 0;
    Config *pdf_letterhead_config = nullptr;

    PDFGenerationParams(Config *pdf_letterhead_config) : pdf_letterhead_config(pdf_letterhead_config) {}

    bool parse_request(std::unique_ptr<char[]> &buf, StaticJsonDocument<192> &doc, WebServerRequest &request) override;
};
