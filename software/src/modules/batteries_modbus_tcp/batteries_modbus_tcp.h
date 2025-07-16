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

#include "config.h"
#include "module.h"
#include "battery_modbus_tcp_table_id.enum.h"
#include "modules/batteries/ibattery_generator.h"
#include "modules/modbus_tcp_client/modbus_function_code.enum.h"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#include "gcc_warnings.h"
#pragma GCC diagnostic ignored "-Weffc++"
#endif

class BatteriesModbusTCP final : public IModule, public IBatteryGenerator
{
public:
    struct RegisterBlockSpec {
        ModbusFunctionCode function_code;
        uint16_t start_address;
        void *values_buffer;
        uint16_t values_count; // not bytes, but registers or coils
    };

    struct TableSpec {
        uint8_t device_address;
        RegisterBlockSpec *register_blocks;
        size_t register_blocks_count;
    };

    static TableSpec *init_table(const Config *config);
    static void free_table(TableSpec *table);

    // for IModule
    void pre_setup() override;
    void register_urls() override;

    // for IBatteryGenerator
    [[gnu::const]] BatteryClassID get_class() const override;
    virtual IBattery *new_battery(uint32_t slot, Config *state, Config *errors) override;
    [[gnu::const]] virtual const Config *get_config_prototype() override;
    [[gnu::const]] virtual const Config *get_state_prototype() override;
    [[gnu::const]] virtual const Config *get_errors_prototype() override;
    virtual String validate_config(Config &update, ConfigSource source) override;

private:
    void write_next();
    void release_client();

    Config config_prototype;
    Config table_custom_register_block_prototype;
    std::vector<ConfUnionPrototype<BatteryModbusTCPTableID>> table_prototypes;
    //Config errors_prototype;

    ConfigRoot execute_config;
    std::vector<ConfUnionPrototype<BatteryModbusTCPTableID>> execute_table_prototypes;
    TFGenericTCPSharedClient *execute_client = nullptr;
    uint32_t execute_cookie;
    TableSpec *execute_table = nullptr;
    size_t execute_index;
};

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
