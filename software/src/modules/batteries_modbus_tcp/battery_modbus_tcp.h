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
#include "battery_modbus_tcp_table_id.enum.h"
#include "modules/api/language.enum.h"
#include "modules/network_lib/generic_tcp_client_pool_connector.h"
#include "modules/batteries/ibattery.h"
#include "modules/modbus_tcp_client/modbus_function_code.enum.h"

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
        RegisterBlockSpec *register_blocks;
        size_t register_blocks_count;
    };

    typedef std::function<void(bool error, const char *fmt, va_list args)> TableWriterVLogFLnFunction;
    typedef std::function<void(void)> TableWriterFinishedFunction;

    struct TableWriter {
        Language language;
        uint64_t task_id = 0;
        uint32_t slot;
        TFModbusTCPSharedClient *client = nullptr;
        uint8_t device_address = 0;
        BatteryMode mode = BatteryMode::None;
        uint16_t repeat_interval; // seconds
        TableSpec *table = nullptr;
        size_t repeat_count = 0;
        size_t index = 0;
        TableWriterVLogFLnFunction vlogfln;
        TableWriterFinishedFunction finished;
        bool transact_pending = false;
        bool delete_requested = false;
        bool test;
    };

    static void load_custom_table(TableSpec **table_ptr, const Config *config);
    static void free_table(TableSpec *table);
    static TableWriter *create_table_writer(uint32_t slot, bool test, TFModbusTCPSharedClient *client, uint8_t device_address, uint16_t repeat_interval /*seconds*/,
                                            BatteryMode mode, TableSpec *table, TableWriterVLogFLnFunction &&vlogfln, TableWriterFinishedFunction &&finished,
                                            Language language = Language::English);
    static void destroy_table_writer(TableWriter *writer);

    BatteryModbusTCP(uint32_t slot_, Config *state_, Config *errors_, TFModbusTCPClientPool *pool_) :
        GenericTCPClientPoolConnector("batteries_mbtcp", format_battery_slot(slot_), pool_), slot(slot_), state(state_), errors(errors_) {}

    [[gnu::const]] BatteryClassID get_class() const override;
    void setup(const Config &ephemeral_config) override;
    void register_events() override;
    void pre_reboot() override;

    bool supports_mode(BatteryMode mode) const override;
    void set_mode(BatteryMode mode) override;

    void set_paused(bool paused);

private:
    void connect_callback(TFGenericTCPClientConnectResult result) override;
    void disconnect_callback(TFGenericTCPClientDisconnectReason reason) override;
    void update_active_mode();

    uint32_t slot;
    Config *state;
    Config *errors;

    BatteryModbusTCPTableID table_id;
    // FIXME: might be allocated. leaking if allocated, because as of right now battery instances don't get destroyed
    TableSpec *tables[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    uint8_t device_address;
    uint16_t repeat_interval; // seconds
    BatteryMode requested_mode = BatteryMode::None;
    bool paused = false;
    bool finished = false;
    BatteryMode active_mode = BatteryMode::None;
    TableWriter *active_writer = nullptr;
};
