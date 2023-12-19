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

#include "config.h"

#define CHARGE_MANAGER_PORT 34127
#define CHARGE_MANAGEMENT_PORT (CHARGE_MANAGER_PORT + 1)

// Increment when changing packet structs
#define CM_COMMAND_VERSION 1
#define CM_STATE_VERSION 2

// Minimum protocol version supported
#define CM_COMMAND_VERSION_MIN 1
#define CM_STATE_VERSION_MIN 1

#define CM_PACKET_MAGIC 34127

#define CM_NETWORKING_ERROR_NO_ERROR 0
#define CM_NETWORKING_ERROR_UNREACHABLE 1
#define CM_NETWORKING_ERROR_INVALID_HEADER 2
#define CM_NETWORKING_ERROR_NOT_MANAGED 3

struct [[gnu::packed]] cm_packet_header {
    uint16_t magic;
    uint16_t length;
    uint16_t seq_num;
    uint8_t version;
    uint8_t padding;
};

#define CM_PACKET_HEADER_LENGTH (sizeof(cm_packet_header))
static_assert(CM_PACKET_HEADER_LENGTH == 8);

#define CM_COMMAND_FLAGS_CPPDISC_BIT_POS 6
#define CM_COMMAND_FLAGS_CPPDISC_MASK (1 << CM_COMMAND_FLAGS_CPPDISC_BIT_POS)
#define CM_COMMAND_FLAGS_CPPDISC_IS_SET(FLAGS) (((FLAGS) & CM_COMMAND_FLAGS_CPPDISC_MASK) != 0)

struct [[gnu::packed]] cm_command_v1 {
    uint16_t allocated_current;
    /* command_flags
    bit 6 - control pilot permanently disconnected
    Other bits must be sent unset and ignored on reception.
    */
    uint8_t command_flags;
    uint8_t padding;
};

#define CM_COMMAND_V1_LENGTH (sizeof(cm_command_v1))
static_assert(CM_COMMAND_V1_LENGTH == 4);

struct [[gnu::packed]] cm_command_packet {
    cm_packet_header header;
    cm_command_v1 v1;
};

#define CM_COMMAND_PACKET_LENGTH (sizeof(cm_command_packet))
static_assert(CM_COMMAND_PACKET_LENGTH == 12);

#define CM_FEATURE_FLAGS_CP_DISCONNECT_BIT_POS 6
#define CM_FEATURE_FLAGS_CP_DISCONNECT_MASK (1 << CM_FEATURE_FLAGS_CP_DISCONNECT_BIT_POS)
#define CM_FEATURE_FLAGS_CP_DISCONNECT_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_CP_DISCONNECT_MASK) != 0)
#define CM_FEATURE_FLAGS_EVSE_BIT_POS 5
#define CM_FEATURE_FLAGS_EVSE_MASK (1 << CM_FEATURE_FLAGS_EVSE_BIT_POS)
#define CM_FEATURE_FLAGS_EVSE_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_EVSE_MASK) != 0)
#define CM_FEATURE_FLAGS_NFC_BIT_POS 4
#define CM_FEATURE_FLAGS_NFC_MASK (1 << CM_FEATURE_FLAGS_NFC_BIT_POS)
#define CM_FEATURE_FLAGS_NFC_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_NFC_MASK) != 0)
#define CM_FEATURE_FLAGS_METER_ALL_VALUES_BIT_POS 3
#define CM_FEATURE_FLAGS_METER_ALL_VALUES_MASK (1 << CM_FEATURE_FLAGS_METER_ALL_VALUES_BIT_POS)
#define CM_FEATURE_FLAGS_METER_ALL_VALUES_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_METER_ALL_VALUES_MASK) != 0)
#define CM_FEATURE_FLAGS_METER_PHASES_BIT_POS 2
#define CM_FEATURE_FLAGS_METER_PHASES_MASK (1 << CM_FEATURE_FLAGS_METER_PHASES_BIT_POS)
#define CM_FEATURE_FLAGS_METER_PHASES_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_METER_PHASES_MASK) != 0)
#define CM_FEATURE_FLAGS_METER_BIT_POS 1
#define CM_FEATURE_FLAGS_METER_MASK (1 << CM_FEATURE_FLAGS_METER_BIT_POS)
#define CM_FEATURE_FLAGS_METER_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_METER_MASK) != 0)
#define CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_BIT_POS 0
#define CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_MASK (1 << CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_BIT_POS)
#define CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_MASK) != 0)

#define CM_STATE_FLAGS_MANAGED_BIT_POS 7
#define CM_STATE_FLAGS_MANAGED_MASK (1 << CM_STATE_FLAGS_MANAGED_BIT_POS)
#define CM_STATE_FLAGS_MANAGED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_MANAGED_MASK) != 0)
#define CM_STATE_FLAGS_CP_DISCONNECTED_BIT_POS 6
#define CM_STATE_FLAGS_CP_DISCONNECTED_MASK (1 << CM_STATE_FLAGS_CP_DISCONNECTED_BIT_POS)
#define CM_STATE_FLAGS_CP_DISCONNECTED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_CP_DISCONNECTED_MASK) != 0)
#define CM_STATE_FLAGS_L1_CONNECTED_BIT_POS 5
#define CM_STATE_FLAGS_L1_CONNECTED_MASK (1 << CM_STATE_FLAGS_L1_CONNECTED_BIT_POS)
#define CM_STATE_FLAGS_L1_CONNECTED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L1_CONNECTED_MASK) != 0)
#define CM_STATE_FLAGS_L2_CONNECTED_BIT_POS 4
#define CM_STATE_FLAGS_L2_CONNECTED_MASK (1 << CM_STATE_FLAGS_L2_CONNECTED_BIT_POS)
#define CM_STATE_FLAGS_L2_CONNECTED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L2_CONNECTED_MASK) != 0)
#define CM_STATE_FLAGS_L3_CONNECTED_BIT_POS 3
#define CM_STATE_FLAGS_L3_CONNECTED_MASK (1 << CM_STATE_FLAGS_L3_CONNECTED_BIT_POS)
#define CM_STATE_FLAGS_L3_CONNECTED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L3_CONNECTED_MASK) != 0)
#define CM_STATE_FLAGS_CONNECTED_BIT_POS 3
#define CM_STATE_FLAGS_CONNECTED_MASK (0x7 << CM_STATE_FLAGS_CONNECTED_BIT_POS)
#define CM_STATE_FLAGS_CONNECTED_GET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_CONNECTED_MASK) >> 3)
#define CM_STATE_FLAGS_L1_ACTIVE_BIT_POS 2
#define CM_STATE_FLAGS_L1_ACTIVE_MASK (1 << CM_STATE_FLAGS_L1_ACTIVE_BIT_POS)
#define CM_STATE_FLAGS_L1_ACTIVE_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L1_ACTIVE_MASK) != 0)
#define CM_STATE_FLAGS_L2_ACTIVE_BIT_POS 1
#define CM_STATE_FLAGS_L2_ACTIVE_MASK (1 << CM_STATE_FLAGS_L2_ACTIVE_BIT_POS)
#define CM_STATE_FLAGS_L2_ACTIVE_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L2_ACTIVE_MASK) != 0)
#define CM_STATE_FLAGS_L3_ACTIVE_BIT_POS 0
#define CM_STATE_FLAGS_L3_ACTIVE_MASK (1 << CM_STATE_FLAGS_L3_ACTIVE_BIT_POS)
#define CM_STATE_FLAGS_L3_ACTIVE_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L3_ACTIVE_MASK) != 0)
#define CM_STATE_FLAGS_ACTIVE_BIT_POS 0
#define CM_STATE_FLAGS_ACTIVE_MASK (0x7 << CM_STATE_FLAGS_ACTIVE_BIT_POS)
#define CM_STATE_FLAGS_ACTIVE_GET(FLAGS) ((FLAGS) & CM_STATE_FLAGS_ACTIVE_MASK)

struct [[gnu::packed]] cm_state_v1 {
    /* feature_flags
    bit 6 - has cp_disconnect
    bit 5 - has evse
    bit 4 - has nfc
    bit 3 - has meter_all_values
    bit 2 - has meter_phases
    bit 1 - has meter
    bit 0 - has button_configuration
    Other bits must be sent unset and ignored on reception.
    */
    uint32_t feature_flags;
    uint32_t esp32_uid;
    uint32_t evse_uptime;
    uint32_t charging_time;
    uint16_t allowed_charging_current;
    uint16_t supported_current;

    uint8_t iec61851_state;
    uint8_t charger_state;
    uint8_t error_state;
    /* state_flags
    bit 7 - managed
    bit 6 - control_pilot_permanently_disconnected
    bit 5 - L1_connected
    bit 4 - L2_connected
    bit 3 - L3_connected
    bit 2 - L1_active
    bit 1 - L2_active
    bit 0 - L3_active
    */
    uint8_t state_flags;
    float line_voltages[3];
    float line_currents[3];
    float line_power_factors[3];
    float power_total;
    float energy_rel;
    float energy_abs;
};

#define CM_STATE_V1_LENGTH (sizeof(cm_state_v1))
static_assert(CM_STATE_V1_LENGTH == 72, "Unexpected CM_STATE_V1_LENGTH");

struct [[gnu::packed]] cm_state_v2 {
    uint32_t time_since_state_change;
};

#define CM_STATE_V2_LENGTH (sizeof(cm_state_v2))
static_assert(CM_STATE_V2_LENGTH == 4, "Unexpected CM_STATE_V2_LENGTH");

struct [[gnu::packed]] cm_state_packet {
    cm_packet_header header;
    cm_state_v1 v1;
    cm_state_v2 v2;
};

#define CM_STATE_PACKET_LENGTH (sizeof(cm_state_packet))
static_assert(CM_STATE_PACKET_LENGTH == 84, "Unexpected CM_STATE_PACKET_LENGTH");
