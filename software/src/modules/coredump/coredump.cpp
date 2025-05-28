/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "coredump.h"

#include <esp_core_dump.h>
#include <esp_flash.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "tools/string_builder.h"

#include "gcc_warnings.h"

// Pre- and postfix take up 54 characters.
COREDUMP_RTC_DATA_ATTR char tf_coredump_info[512];

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

Coredump::Coredump()
{
    StaticJsonDocument<500> tf_coredump_json;
    tf_coredump_json["firmware_version"] = build_version_full_str();
    tf_coredump_json["firmware_commit_id"] = build_commit_id_str();
    tf_coredump_json["firmware_file_name"] = build_filename_str();

    if (build_coredump_info(tf_coredump_json)) {
        setup_error = CoredumpSetupError::OK;
        return;
    }

    tf_coredump_json.remove("firmware_file_name");

    if (build_coredump_info(tf_coredump_json)) {
        setup_error = CoredumpSetupError::Truncated;
        return;
    }

    setup_error = CoredumpSetupError::BufferToSmall;
}

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

bool Coredump::build_coredump_info(JsonDocument &tf_coredump_json)
{
    String tf_coredump_json_string;
    serializeJson(tf_coredump_json, tf_coredump_json_string);

    constexpr const char *prefix = "___tf_coredump_info_start___";
    constexpr const char *suffix = "___tf_coredump_info_end___";
    constexpr size_t prefix_len = constexpr_strlen(prefix);
    constexpr size_t suffix_len = constexpr_strlen(suffix);

    if (prefix_len + tf_coredump_json_string.length() + suffix_len >= sizeof(tf_coredump_info))
        return false;

    StringWriter sw(tf_coredump_info, sizeof(tf_coredump_info));

    sw.puts(prefix,                          static_cast<ssize_t>(prefix_len));
    sw.puts(tf_coredump_json_string.c_str(), static_cast<ssize_t>(tf_coredump_json_string.length()));
    sw.puts(suffix,                          static_cast<ssize_t>(suffix_len));

    return true;
}

void Coredump::pre_init()
{
    if (esp_reset_reason() != ESP_RST_PANIC) {
        return;
    }

    char task_name[16];
    uint32_t exc_cause;
    char backtrace[192];
    StringWriter sw_bt(backtrace, ARRAY_SIZE(backtrace));

    { // Scope for summary
        esp_core_dump_summary_t summary;
        if (esp_core_dump_get_summary(&summary) != ESP_OK) {
            strlcpy(task_name, "<unknown>", sizeof(task_name));
            exc_cause = EXCCAUSE_EXCCAUSE_MASK + 1;
        } else {
            memcpy(task_name, summary.exc_task, sizeof(task_name));
            exc_cause = summary.ex_info.exc_cause;

            sw_bt.puts("Backtrace:");

            const uint32_t frame_count = std::min(summary.exc_bt_info.depth, static_cast<uint32_t>(ARRAY_SIZE(summary.exc_bt_info.bt)));
            const uint32_t *bt = summary.exc_bt_info.bt;
            for (uint32_t i = 0; i < frame_count; i ++) {
                sw_bt.printf(" 0x%08lx", bt[i]);
            }

            if (summary.exc_bt_info.corrupted) {
                sw_bt.puts(" |<-CORRUPTED\n");
            } else {
                sw_bt.putc('\n');
            }
        }
    }

    { // Scope for panic_reason
        char panic_reason[256];
        if (esp_core_dump_get_panic_reason(panic_reason, sizeof(panic_reason)) == ESP_OK) {
            logger.printfln("Task '%.16s' panicked: '%s'", task_name, panic_reason);
        } else {
            const char *exc_name = exc_cause < ARRAY_SIZE(exc_cause_table) ? exc_cause_table[exc_cause] : "<unknown>";
            logger.printfln("Task '%.16s' caused exception %lu: %s", task_name, exc_cause, exc_name);
        }
    }

    const size_t backtrace_length = sw_bt.getLength();
    if (backtrace_length > 0) {
        logger.print_plain(backtrace, backtrace_length);
    }
}

void Coredump::pre_setup()
{
    state = Config::Object({
        {"coredump_available", Config::Bool(false)}
    });

    if (setup_error == CoredumpSetupError::BufferToSmall) {
        logger.printfln("Buffer too small for any info data.");
    } else if (setup_error == CoredumpSetupError::Truncated) {
        logger.printfln("Buffer too small for all data; info data truncated.");
    }
}

void Coredump::setup()
{
    const esp_err_t status = esp_core_dump_image_check();

    switch(status) {
        case ESP_OK:
            state.get("coredump_available")->updateBool(true);
            break;
        case ESP_ERR_NOT_FOUND:
            break;
        case ESP_ERR_INVALID_SIZE:
            // Don't report this because it's the default error when a core dump was erased.
            break;
        case ESP_ERR_INVALID_CRC:
            // esp_core_dump_image_check() writes confusing log messages directly to the console when checksum verification fails.
            // Change this to a plain printf if it shouldn't be in the event log.
            logger.printfln("Core dump has invalid CRC");
            break;
        default:
            logger.printfln("Core dump unavailable: %s (0x%lx)", esp_err_to_name(status), static_cast<uint32_t>(status));
    }
}

void Coredump::register_urls()
{
    api.addState("coredump/state", &state);

    server.on_HTTPThread("/coredump/erase", HTTP_GET, [this](WebServerRequest request) {
        esp_core_dump_image_erase();

        if (esp_core_dump_image_check() == ESP_OK) {
            return request.send(503, "text/plain", "Error while erasing core dump");
        }

        task_scheduler.scheduleOnce([this]() {
            this->state.get("coredump_available")->updateBool(false);
        });

        return request.send(200);
    });

    server.on_HTTPThread("/coredump/coredump.elf", HTTP_GET, [this](WebServerRequest request) {
        constexpr size_t OFFSET_BEFORE_ELF_HEADER = 24;
        constexpr size_t BUFFER_SIZE = 2048;
        char buffer[BUFFER_SIZE];

        esp_err_t ret = esp_core_dump_image_check();
        if (ret != ESP_OK) {
            StringWriter sw(buffer, BUFFER_SIZE);
            sw.printf("No core dump image available: %s (0x%lX)", esp_err_to_name(ret), static_cast<uint32_t>(ret));
            return request.send(404, "text/plain", buffer, static_cast<ssize_t>(sw.getLength()));
        }

        size_t addr;
        size_t size;
        ret = esp_core_dump_image_get(&addr, &size);
        if (ret != ESP_OK) {
            StringWriter sw(buffer, BUFFER_SIZE);
            sw.printf("Failed to get core dump image: %s (0x%lX)", esp_err_to_name(ret), static_cast<uint32_t>(ret));
            return request.send(503, "text/plain", buffer, static_cast<ssize_t>(sw.getLength()));
        }

        request.beginChunkedResponse(200, "application/octet-stream");

        for (size_t i = 0; i < size; i += BUFFER_SIZE) {
            size_t to_send = std::min(BUFFER_SIZE, size - i);
            if (esp_flash_read(NULL, buffer, addr + i, to_send) != ESP_OK) {
                request.sendChunk("\n\nESP_FLASH_READ failed. Core dump truncated", HTTPD_RESP_USE_STRLEN);
                break;
            }
            request.sendChunk(buffer + (i == 0 ? OFFSET_BEFORE_ELF_HEADER : 0), static_cast<ssize_t>(to_send - (i == 0 ? OFFSET_BEFORE_ELF_HEADER : 0)));
        }

        return request.endChunkedResponse();
    });
}
