/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#include <stdint.h>
#include <TFModbusTCPClient.h>
#include <TFModbusTCPClientPool.h>

#include "config.h"
#include "options.h"
#include "language.h"
#include "generated/battery_modbus_tcp_table_id.enum.h"
#include "generated/kostal_plenticore_plus_g2_variant.enum.h"
#include "generated/kostal_plenticore_g3_variant.enum.h"
#include "modules/network_lib/generic_tcp_client_pool_connector.h"
#include "modules/batteries/ibattery.h"
#include "modules/modbus_tcp_client/generated/modbus_function_code.enum.h"

class BatteryModbusTCP final : protected GenericTCPClientPoolConnector, public IBattery
{
public:
    struct RegisterBlockSpec {
        ModbusFunctionCode function_code;
        uint16_t start_address;
        void *buffer;
        uint16_t values_count; // not bytes, but registers or coils
    };

    struct TableSpec {
        BatteryMode effective_mode;
        RegisterBlockSpec *register_blocks;
        size_t register_blocks_count;
    };

    typedef std::function<void(bool event_log, const char *fmt, va_list args)> VLogFLnFunction;
    typedef std::function<void(void)> WriterFailureFunction;
    typedef std::function<bool(void)> DiscoverCompleteFunction;

    struct WriterContext {
        Language language;
        uint64_t task_id = 0;
        BatteryModbusTCP *battery;
        uint32_t slot;
        TFModbusTCPSharedClient *client;
        uint8_t device_address;
        uint16_t transaction_id_mask;
        uint16_t repeat_interval; // seconds
        BatteryMode mode;
        TableSpec *table;
        size_t repeat_count = 0;
        size_t index = 0;
        size_t first_non_precondition_index = 0; // == index + 1 of the last register block with a read* function code, 0 == no precondition
        size_t last_precondition_not_met_index_plus_one = 0;
        VLogFLnFunction vlogfln;
        WriterFailureFunction failure;
        bool transact_pending = false;
        bool destroy_requested = false;
        bool precondition_met = false;
        bool test;
    };

    static const char *get_battery_mode_display_name(BatteryMode value, Language language = Language::English);

    static void load_custom_table(TableSpec **table_ptr, const Config *config);
    static void free_table(TableSpec *table);
    static WriterContext *create_writer(BatteryModbusTCP *battery, uint32_t slot, bool test, TFModbusTCPSharedClient *client, uint8_t device_address,
                                        uint16_t transaction_id_mask, uint16_t repeat_interval /*seconds*/,
                                        BatteryMode mode, TableSpec *table, VLogFLnFunction &&vlogfln,
                                        WriterFailureFunction &&failure, Language language = Language::English);
    static void destroy_writer(WriterContext *ctx);

    struct DiscoverContext {
        Language language;
        uint64_t task_id = 0;
        BatteryModbusTCP *battery;
        uint32_t slot;
        TFModbusTCPSharedClient *client;
        uint8_t device_address;
        uint16_t transaction_id_mask;
        VLogFLnFunction vlogfln;
        DiscoverCompleteFunction complete = nullptr;
        void *buffer = nullptr;
        bool transact_pending = false;
        bool destroy_requested = false;
        bool test;
    };

    static DiscoverContext *create_discover(BatteryModbusTCP *battery, uint32_t slot, bool test, TFModbusTCPSharedClient *client, uint8_t device_address,
                                            uint16_t transaction_id_mask, VLogFLnFunction &&vlogfln, Language language = Language::English);
    static void destroy_discover(DiscoverContext *ctx);
    static void discover_kostal_plenticore_plus_g2_variant(DiscoverContext *ctx, std::function<void(KostalPlenticorePlusG2Variant variant)> &&callback);
    static void discover_kostal_plenticore_g3_variant(DiscoverContext *ctx,  std::function<void(KostalPlenticoreG3Variant variant)> &&callback);

    BatteryModbusTCP(uint32_t slot_, Config *state_, Config *errors_, TFModbusTCPClientPool *pool_) :
        GenericTCPClientPoolConnector("batteries_mbtcp", format_battery_slot(slot_), pool_), slot(slot_), state(state_), errors(errors_) {}

    [[gnu::const]] BatteryClassID get_class() const override;
    void setup(const Config &ephemeral_config) override;
    void register_events() override;
    void pre_reboot() override;

    void set_mode(BatteryMode mode) override;

    void set_testing(bool testing);

    void set_state_mode(BatteryMode active_mode, BatteryMode effective_mode);
    void set_state_discovering(bool discovering);
    void set_state_checking(bool checking);

private:
    void connect_callback(TFGenericTCPClientConnectResult result, TFGenericTCPClientPoolShareLevel share_level) override;
    void disconnect_callback(TFGenericTCPClientDisconnectReason reason, TFGenericTCPClientPoolShareLevel share_level) override;
    void load_tables(const Config *table_config);
    void update_pending_mode();

    uint32_t slot;
    Config *state;
    Config *errors;

    BatteryModbusTCPTableID table_id;
    // FIXME: might be allocated. leaking if allocated, because as of right now battery instances don't get destroyed
    TableSpec *tables[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    uint8_t device_address;
    uint16_t transaction_id_mask = UINT16_MAX;
    uint16_t repeat_interval; // seconds
    BatteryMode requested_mode = BatteryMode::None;
    bool discover_pending = false;
    bool testing = false;
    BatteryMode pending_mode = BatteryMode::None;
    WriterContext *writer_ctx = nullptr;
    Config *discover_table_config = nullptr; // FIXME: leaking this, because as of right now battery instances don't get destroyed
    DiscoverContext *discover_ctx = nullptr;
    union {
        KostalPlenticorePlusG2Variant kostal_plenticore_plus_g2_variant;
        KostalPlenticoreG3Variant kostal_plenticore_g3_variant;
    };
};
