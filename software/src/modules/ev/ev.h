/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include <math.h>

#include "module.h"
#include "config.h"

#include "generated/module_available.h"

#if MODULE_METERS_ISO15118_AVAILABLE()
#include "modules/meters_iso15118/generated/ev_data_source.enum.h"
#else
enum class EVDataProtocol : uint8_t {
    API = 1
};
#endif

#define EV_MAC_ADDRESS_LENGTH 6
#define EV_MAC_STRING_LENGTH 17 // "AA:BB:CC:DD:EE:FF"
#define EV_MAX_EVS 16
#define EV_SEEN_MAC_COUNT 8

static constexpr float EV_DEFAULT_CHARGING_EFFICIENCY = 0.92f;
static constexpr float EV_DEFAULT_CAPACITY            = 60.0f;

struct EVSession {
    float soc = NAN;                      // Current SOC (%)
    float soc_initial = NAN;              // Initial SOC for estimation (%)
    float energy_at_soc_reading = 0.0f;   // Energy at last SOC reading (kWh)
    float capacity = EV_DEFAULT_CAPACITY; // Battery capacity (kWh)
    float charging_efficiency = EV_DEFAULT_CHARGING_EFFICIENCY;
    float power = NAN;                    // EV current power (W, only iso15118-20)
};

class Ev final : public IModule
{
public:
    Ev(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

    void on_ev_connected(const uint8_t mac[EV_MAC_ADDRESS_LENGTH], bool injected = false);
    void on_ev_disconnected();

    int16_t get_user_id(const uint8_t mac[EV_MAC_ADDRESS_LENGTH]);
    void remove_user(uint8_t user_id);

    void set_soc(float soc); // Overwrites current and initial soc
    void update_estimated_soc();
    void session_updated(EVDataSource source); // Called after writing session fields.
    void add_seen_mac_address(const uint8_t mac[EV_MAC_ADDRESS_LENGTH]);

    EVSession session;

private:
    float get_session_energy_kwh();
    void clear_session();
    int find_matching_ev_index(const uint8_t mac[EV_MAC_ADDRESS_LENGTH]);

    ConfigRoot config;
    Config ev_config_prototype;
    ConfigRoot state;
    ConfigRoot inject_soc;
    ConfigRoot inject_ev;
    Config seen_macs_prototype;
    ConfigRoot seen_macs;
    Config auth_info;

    int active_ev_index = -1; // Index into config evs array, -1 = no match / unknown EV

    uint64_t soc_estimation_task = 0;
};

#include "generated/module_available_end.h"
