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
#include "language.h"
#include "../web_server/web_server.h"
#include "csv_charge_log.h"
#include "module_available.h"
#include "charge_tracker_defs.h"
#include "file_type.enum.h"
#include "csv_flavor.enum.h"
#include "generation_state.enum.h"
#include "charge_log_send_error.enum.h"

#if MODULE_REMOTE_ACCESS_AVAILABLE()
#include "../remote_access/remote_access_packets.h"

// States for the charge log send state machine
enum class ChargeLogSendState : uint8_t {
    Idle,                // Not sending
    RequestSent,         // Sent RequestChargeLogSend, waiting for potential Nack
    MetadataSent,        // Sent MetadataForChargeLog, waiting for Ack
    OpeningConnection,   // Opening TCP connection to server
    SendingData,         // Streaming charge log data over TCP
    WaitingForFinalAck,  // TCP closed, waiting for final Ack from server
    Done,                // Successfully completed
    Error,               // Failed
};

// Context for the packet-driven charge log send state machine.
struct ChargeLogSendContext {
    ChargeLogSendState state = ChargeLogSendState::Idle;
    int tcp_sock = -1;
    ChargeLogSendError error_code = ChargeLogSendError::Success;
    NackReason nack_reason = {};

    // Metadata needed for building MetadataForChargeLog packet
    uint8_t user_uuid[16] = {};
    uint8_t lang[2] = {};
    bool is_monthly_email = false;
    String filename;
    String display_name;
};
#endif

#define CHARGE_TRACKER_MAX_REPAIR 200
struct RemoteUploadRequest;
class ChargeLogGenerationLockHelper;
struct ChargeWithLocation;
struct ExportCharge;

class ChargeTracker final : public IModule
{
public:
    ChargeTracker(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

    uint32_t first_charge_record;
    uint32_t last_charge_record;

    bool startCharge(uint32_t timestamp_minutes, float meter_start, uint8_t user_id, uint32_t evse_uptime, uint8_t auth_type, Config::ConfVariant auth_info, const char *directory = nullptr);
    void endCharge(uint32_t charge_duration_seconds, float meter_end, const char *directory = nullptr);
    void removeOldRecords();
    bool setupRecords(const char *directory = nullptr);
    void updateState();
    bool is_user_tracked(uint8_t user_id);
    bool has_tracked_charges(uint32_t charger_uid);

    size_t completeRecordsInLastFile();
    bool currentlyCharging(const char *directory = nullptr);

    void readNRecords(File *f, size_t records_to_read, const char *directory = nullptr);

    void updateLastCharges(const char *directory);

    ExportCharge *getFilteredCharges(int user_filter, int device_filter, uint32_t start_timestamp_min, uint32_t end_timestamp_min, size_t *out_count);

#if MODULE_REMOTE_ACCESS_AVAILABLE()
    void start_charge_log_upload_for_user(const uint32_t cookie, const int user_filter = -2, const int device_filter = -2, const uint32_t start_timestamp_min = 0, const uint32_t end_timestamp_min = 0, const Language language = Language::German, const FileType file_type = FileType::PDF, const CSVFlavor csv_delimiter = CSVFlavor::Excel, std::unique_ptr<char[]> letterhead = nullptr, std::unique_ptr<ChargeLogGenerationLockHelper> generation_lock = nullptr, const String &remote_access_user_uuid = "");

    // Processes a pre-validated Ack or Nack management packet for the charge log send state machine.
    // Header checking (magic, packet type) is done by the remote_access module before calling this.
    // Returns true if the packet was consumed by the state machine.
    bool handle_charge_log_send_packet(PacketType type, NackReason nack_reason);

    // Initiates the charge log send state machine.
    // Stores the metadata context and sends RequestChargeLogSend with a config hash.
    // config_hash: SHA-256 hash of the generation config + month (32 bytes).
    // Returns true if the request was sent successfully.
    bool start_charge_log_send_sm(const char *filename, size_t filename_len,
                                  const char *display_name, size_t display_name_len,
                                  const uint8_t user_uuid[16],
                                  Language language,
                                  bool is_monthly_email,
                                  const uint8_t config_hash[32]);

    int generate_pdf(std::function<int(const void *buffer, size_t len)> &&callback, int user_filter, int device_filter, uint32_t start_timestamp_min, uint32_t end_timestamp_min, uint32_t current_timestamp_min, Language language, const char *letterhead, int letterhead_lines, WebServerRequest *request);

#endif

    ConfigRoot last_charges;
    ConfigRoot current_charge;
    ConfigRoot current_charges;
    ConfigRoot state;

    ConfigRoot config;
    ConfigRoot pdf_letterhead_config;

    std::mutex records_mutex;
    std::mutex pdf_mutex;

    ChargeLogSendContext charge_log_send_ctx;
    std::unique_ptr<RemoteUploadRequest> remote_upload_request;
    std::unique_ptr<ChargeLogGenerationLockHelper> pending_generation_lock;

private:
    bool repair_last(float, const char *);
    void repair_charges();
    std::vector<ChargeWithLocation> readLastChargesFromDirectory(const char *directory);
    bool getChargerChargeRecords(const char *directory, uint32_t *first_record, uint32_t *last_record);

    Config last_charges_prototype;
    Config current_charge_prototype;
    Config charge_log_send_prototype;

    std::vector<ChargeWithLocation> oldest_charges_per_directory;
    uint32_t total_charge_log_files;

#if MODULE_REMOTE_ACCESS_AVAILABLE()
    // Sends the MetadataForChargeLog packet using data stored in charge_log_send_ctx.
    bool send_metadata_from_ctx();

    // Monthly upload state machine: tries to start the send for the given user index.
    // Returns true if the send was started successfully, false if this user should be skipped.
    bool try_start_monthly_upload_for_user(int user_idx);
    // Called when all users have been processed or when there's nothing to upload.
    void finish_monthly_upload();

    uint64_t monthly_upload_task_id = 0;
    int8_t monthly_upload_user_idx = -1; // -1 = not uploading
    uint8_t monthly_upload_config_count = 0;
    micros_t monthly_upload_deadline = 0_us;
    uint32_t monthly_upload_start_timestamp_min = 0;
    uint32_t monthly_upload_end_timestamp_min = 0;
#endif
};

/**
 * Request parameters for asynchronous charge log upload operations to remote servers.
 *
 * This struct encapsulates all parameters needed for uploading charge logs to a remote
 * server via the management connection. It manages the complete lifecycle of an upload
 * operation, including time range filtering.
 *
 * The struct is typically passed as a unique_ptr.
 */
struct RemoteUploadRequest {
    /** Index of the remote upload configuration to use (0-based, -1 = all configs) */
    int8_t config_index = 0;

    /** Total number of configurations available */
    uint8_t config_count = 0;

    /** Cookie to correlate upload attempts with web interface events */
    uint32_t cookie = 0;

    /** User filter override (-2 = unknown user -1 = all users, 0+ = specific user) */
    int user_filter = -2;

    /** Device filter override (-2 = all chargers, -1 = local charger, 0+ = specific charger) */
    int device_filter = -2;

    /** Start of the time range to upload, in minutes since Unix epoch */
    uint32_t start_timestamp_min = 0;

    /** End of the time range to upload, in minutes since Unix epoch */
    uint32_t end_timestamp_min = 0;

    /** Language to use for the upload request */
    Language language = Language::German;

    /** File type to generate and upload */
    FileType file_type = FileType::PDF;

    /** CSV delimiter flavor to use if file_type is CSV */
    CSVFlavor csv_delimiter = CSVFlavor::Excel;

    /** Whether to use format overrides from the struct (true) or read from config (false) */
    bool use_format_overrides = false;

    /** Remote access user UUID for authentication */
    String remote_access_user_uuid = "";

    /** Letterhead for PDF generation */
    std::unique_ptr<char[]> letterhead = nullptr;

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

struct ChargeWithLocation {
    Charge charge;
    String directory;
    uint32_t file_index;
    uint32_t prev_known_timestamp_minutes;

    bool operator>(const ChargeWithLocation &other) const {
        uint32_t this_timestamp = charge.cs.timestamp_minutes != 0 ? charge.cs.timestamp_minutes : prev_known_timestamp_minutes;
        uint32_t other_timestamp = other.charge.cs.timestamp_minutes != 0 ? other.charge.cs.timestamp_minutes : other.prev_known_timestamp_minutes;

        if (this_timestamp == 0 && other_timestamp == 0) {
            return false;
        }

        return this_timestamp > other_timestamp;
    }
};

struct ExportCharge {
    Charge charge;
    uint32_t charger_uid;
    uint32_t prev_known_timestamp_minutes;

    bool operator>(const ExportCharge &other) const {
        uint32_t this_timestamp = charge.cs.timestamp_minutes != 0 ? charge.cs.timestamp_minutes : prev_known_timestamp_minutes;
        uint32_t other_timestamp = other.charge.cs.timestamp_minutes != 0 ? other.charge.cs.timestamp_minutes : other.prev_known_timestamp_minutes;

        if (this_timestamp == 0 && other_timestamp == 0) {
            return false;
        }

        return this_timestamp > other_timestamp;
    }
};

#include "module_available_end.h"
