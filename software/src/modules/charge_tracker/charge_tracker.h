/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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
#include "../web_server/web_server.h"
#include "async_https_client.h"
#include "csv_charge_log.h"
#include "module_available.h"
#include "charge_tracker_defs.h"
#include "file_type.enum.h"
#include "../system/language.enum.h"
#include "csv_flavor.enum.h"
#include "generation_state.enum.h"

#define CHARGE_TRACKER_MAX_REPAIR 200
#define MAX_RETRY_COUNT 10
#define BASE_RETRY_DELAY_MINUTES 5
struct RemoteUploadRequest;
class ChargeLogGenerationLockHelper;

class ChargeTracker final : public IModule
{
public:
    ChargeTracker(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    uint32_t first_charge_record;
    uint32_t last_charge_record;

    bool startCharge(uint32_t timestamp_minutes, float meter_start, uint8_t user_id, uint32_t evse_uptime, uint8_t auth_type, Config::ConfVariant auth_info);
    void endCharge(uint32_t charge_duration_seconds, float meter_end);
    void removeOldRecords();
    bool setupRecords();
    void updateState();
    bool is_user_tracked(uint8_t user_id);

    size_t completeRecordsInLastFile();
    bool currentlyCharging();

    void readNRecords(File *f, size_t records_to_read);

#if MODULE_REMOTE_ACCESS_AVAILABLE()
    void send_file(std::unique_ptr<RemoteUploadRequest> args);
    void upload_charge_logs(const int8_t retry_count = 0);
    void start_charge_log_upload_for_config(const uint8_t config_index, const uint32_t cookie, const int user_filter = -2, const uint32_t start_timestamp_min = 0, const uint32_t end_timestamp_min = 0, const Language language = Language::German, const FileType file_type = FileType::PDF, const CSVFlavor csv_delimiter = CSVFlavor::Excel, std::unique_ptr<char[]> letterhead = nullptr, std::unique_ptr<ChargeLogGenerationLockHelper> generation_lock = nullptr);
#endif

    ConfigRoot last_charges;
    ConfigRoot current_charge;
    ConfigRoot state;

    ConfigRoot config;
    ConfigRoot pdf_letterhead_config;

    std::mutex records_mutex;
    std::mutex pdf_mutex;

private:
    bool repair_last(float);
    void repair_charges();
    void generate_pdf(std::function<int(const void *buffer, size_t len)> &&callback, int user_filter, uint32_t start_timestamp_min, uint32_t end_timestamp_min, uint32_t current_timestamp_min, Language language, const char *letterhead, int letterhead_lines, WebServerRequest *request);

    Config last_charges_prototype;
    Config charge_log_send_prototype;
};

/**
 * Request parameters for asynchronous charge log upload operations to remote servers.
 *
 * This struct encapsulates all parameters needed for uploading charge logs to a remote
 * server via HTTPS. It supports retry logic with exponential backoff and manages the
 * complete lifecycle of an upload operation, including time range filtering and HTTP
 * client management.
 *
 * The struct is typically passed as a unique_ptr.
 */
struct RemoteUploadRequest {
    /** Index of the remote upload configuration to use (0-based, -1 = all configs) */
    int8_t config_index = 0;

    /** Current retry attempt count (0 = first attempt, incremented on each retry, -1 = no retries) */
    int8_t retry_count = -1;

    /** Total number of configurations available */
    uint8_t config_count = 0;

    /** Cookie to correlate upload attempts with web interface events */
    uint32_t cookie = 0;

    /** User filter override (-2 = unknown user -1 = all users, 0+ = specific user) */
    int user_filter = -2;

    /** Start of the time range to upload, in minutes since Unix epoch */
    uint32_t start_timestamp_min = 0;

    /** End of the time range to upload, in minutes since Unix epoch */
    uint32_t end_timestamp_min = 0;

    /** Delay before next retry attempt (used for exponential backoff scheduling) */
    millis_t next_retry_delay = millis_t(0);

    /** Language to use for the upload request */
    Language language = Language::German;

    /** File type to generate and upload */
    FileType file_type = FileType::PDF;

    /** CSV delimiter flavor to use if file_type is CSV */
    CSVFlavor csv_delimiter = CSVFlavor::Excel;

    /** Whether to use format overrides from the struct (true) or read from config (false) */
    bool use_format_overrides = false;

    /** Letterhead for PDF generation */
    std::unique_ptr<char[]> letterhead = nullptr;

    /** HTTP client instance for performing the upload request */
    std::unique_ptr<AsyncHTTPSClient> remote_client = nullptr;

    std::unique_ptr<ChargeLogGenerationLockHelper> generation_lock = nullptr;
};

/**
 * This helper struct provides a raii-style lock that ensures only one charge log generation
 * without the need of holding a mutex for the entire generation duration.
 */
class ChargeLogGenerationLockHelper {
    public:
        static std::unique_ptr<ChargeLogGenerationLockHelper> try_lock(GenerationState kind);

        ~ChargeLogGenerationLockHelper();
    private:
        static std::atomic<GenerationState> generation_lock_state;
};

#include "module_available_end.h"
