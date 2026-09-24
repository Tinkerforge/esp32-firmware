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

#include <stdint.h>
#include <TFModbusTCPClient.h>
#include <TFModbusTCPClientPool.h>

#include "config.h"
#include "options.h"
#include "language.h"
#include "modules/network_lib/generic_tcp_client_pool_connector.h"
#include "modules/batteries/ibattery.h"
#include "modules/sun_spec/sun_spec_resolver.h"

/*

Block
124:ChaGriSet = 0
124:StorCtl_Mod = 3 / 0b11
124:InWRte = 0 %
124:OutWRte = 0 %
124:InOutWRte_RvrtTms = 90 s


Normal
124:ChaGriSet = 0
124:StorCtl_Mod = 0 / 0b00
124:InWRte = 100 %
124:OutWRte = 100 %
124:InOutWRte_RvrtTms = 0 s


Block Discharge
124:ChaGriSet = 0
124:StorCtl_Mod = 2 / 0b10
124:InWRte = 100 %
124:OutWRte = 0 %
124:InOutWRte_RvrtTms = 90 s


Force Charge
124:ChaGriSet = 1
124:StorCtl_Mod = 3 / 0b11
124:InWRte = force_charge_rate
124:OutWRte = -force_charge_rate
124:InOutWRte_RvrtTms = 90 s

Fronius Anlagenüberwachung - Einstellungen - EVU-Editor - Batterie Ladung - Batterieladung aus EVU Netz erlauben


Block Charge
124:ChaGriSet = 0
124:StorCtl_Mod = 1 / 0b01
124:InWRte = 0 %
124:OutWRte = 100 %
124:InOutWRte_RvrtTms = 90 s


Force Discharge
124:ChaGriSet = 0
124:StorCtl_Mod = 3 / 0b11
124:InWRte = -force_discharge_rate
124:OutWRte = force_discharge_rate
124:InOutWRte_RvrtTms = 90 s

*/

class BatterySunSpec final : protected GenericTCPClientPoolConnector, public IBattery
{
public:
    typedef std::function<void(bool event_log, const char *fmt, va_list args)> VLogFLnFunction;
    typedef std::function<void(void)> WriterFailureFunction;
    typedef std::function<bool(void)> DiscoverCompleteFunction;

    enum class WriterState {
        Idle,
        ReadInOutWRteSF,
        WriteChaGriSet,
        WriteStorCtlMod,
        WriteInOutWRte,
        WriteInOutWRteRvrtTms,
        Done,
    };

    struct WriterContext {
        Language language;
        uint64_t task_id = 0;
        BatterySunSpec *battery;
        uint32_t slot;
        TFModbusTCPSharedClient *shared_client;
        uint8_t device_address;
        uint16_t start_address;
        int8_t force_charge_rate;
        int8_t force_discharge_rate;
        BatteryMode mode;
        size_t repeat_count = 0;
        WriterState state = WriterState::Idle;
        WriterState state_next = WriterState::ReadInOutWRteSF;
        uint16_t buffer[2];
        float in_out_w_rte_scale_factor;
        const char *log_prefix;
        VLogFLnFunction vlogfln;
        WriterFailureFunction failure;
        bool transact_pending = false;
        bool destroy_requested = false;
        bool test;
    };

    static WriterContext *create_writer(BatterySunSpec *battery, uint32_t slot, bool test, TFModbusTCPSharedClient *shared_client, uint8_t device_address,
                                        uint16_t start_address, int8_t force_charge_rate, int8_t force_discharge_rate, BatteryMode mode,
                                        const char *log_prefix, VLogFLnFunction &&vlogfln, WriterFailureFunction &&failure, Language language = Language::English);
    static void destroy_writer(WriterContext *ctx);

    BatterySunSpec(uint32_t slot_, Config *state_, Config *errors_, TFModbusTCPClientPool *pool_) :
        GenericTCPClientPoolConnector("batteries_sunspc", format_battery_slot(slot_), pool_), slot(slot_), state(state_), errors(errors_) {}

    [[gnu::const]] BatteryClassID get_class() const override;
    void setup(const Config &ephemeral_config) override;
    void register_events() override;
    void pre_reboot() override;

    void set_mode(BatteryMode mode) override;

    void set_testing(bool testing);

    void set_state_mode(BatteryMode mode);
    void set_state_resolving(bool resolving);

private:
    void connect_callback(TFGenericTCPClientConnectResult result, TFGenericTCPClientPoolShareLevel share_level) override;
    void disconnect_callback(TFGenericTCPClientDisconnectReason reason, TFGenericTCPClientPoolShareLevel share_level) override;
    void update_pending_mode();
    void resolve_result(SunSpecResolverCommonModel *common_model, uint16_t start_address, uint16_t block_length);

    uint32_t slot;
    Config *state;
    Config *errors;

    uint8_t device_address;
    String manufacturer_name;
    String model_name;
    String serial_number;
    uint16_t model_instance;
    int8_t force_charge_rate;
    int8_t force_discharge_rate;
    char trace_prefix[8 + 1]; // strlen("bXYr t0 ") == 8
    BatteryMode requested_mode = BatteryMode::None;
    bool resolve_pending = false;
    uint64_t resolve_start_delayed_task_id = 0;
    uint16_t resolved_address;
    bool testing = false;
    BatteryMode pending_mode = BatteryMode::None;
    WriterContext *writer_ctx = nullptr;
    SunSpecResolver *resolver = nullptr;
    micros_t last_connect = 0_us;
};
