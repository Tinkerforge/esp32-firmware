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

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "generated/client_error.enum.h"
#include "generated/config_charge_mode.enum.h"
#include "generated/cm_auth_feedback.enum.h"
#include "generated/cm_auth_type.enum.h"

#if defined(BOARD_HAS_PSRAM)
// Also change charge_tracker_max_tracked_charges when changing this!
#define MAX_CONTROLLED_CHARGERS 64
#else
#define MAX_CONTROLLED_CHARGERS 10
#endif

#define CHARGE_MANAGER_PORT 34127
#define CHARGE_MANAGEMENT_PORT (CHARGE_MANAGER_PORT + 1)

// Increment when changing packet structs
#define CM_COMMAND_VERSION 5
#define CM_STATE_VERSION 6

// Minimum protocol version supported
#define CM_COMMAND_VERSION_MIN 1
#define CM_STATE_VERSION_MIN 1

#define CM_PACKET_MAGIC 34127

struct cm_packet_header {
    uint16_t magic;
    uint16_t length;
    uint16_t seq_num;
    uint8_t version;
    uint8_t padding;
};

#define CM_PACKET_HEADER_LENGTH (sizeof(cm_packet_header))
static_assert(CM_PACKET_HEADER_LENGTH == 8, "Unexpected CM_PACKET_HEADER_LENGTH");

#define CM_COMMAND_FLAGS_IGNORE_ALLOCATION_BIT_POS 7
#define CM_COMMAND_FLAGS_IGNORE_ALLOCATION_MASK (1u << CM_COMMAND_FLAGS_IGNORE_ALLOCATION_BIT_POS)
#define CM_COMMAND_FLAGS_IGNORE_ALLOCATION_IS_SET(FLAGS) (((FLAGS) & CM_COMMAND_FLAGS_IGNORE_ALLOCATION_MASK) != 0)

#define CM_COMMAND_FLAGS_CPDISC_BIT_POS 6
#define CM_COMMAND_FLAGS_CPDISC_MASK (1u << CM_COMMAND_FLAGS_CPDISC_BIT_POS)
#define CM_COMMAND_FLAGS_CPDISC_IS_SET(FLAGS) (((FLAGS) & CM_COMMAND_FLAGS_CPDISC_MASK) != 0)

#define CM_COMMAND_FLAGS_CENTRAL_USER_MANAGEMENT_BIT_POS 5
#define CM_COMMAND_FLAGS_CENTRAL_USER_MANAGEMENT_MASK (1u << CM_COMMAND_FLAGS_CENTRAL_USER_MANAGEMENT_BIT_POS)
#define CM_COMMAND_FLAGS_CENTRAL_USER_MANAGEMENT_IS_SET(FLAGS) (((FLAGS) & CM_COMMAND_FLAGS_CENTRAL_USER_MANAGEMENT_MASK) != 0)

#define CM_COMMAND_FLAGS_CENTRAL_CHARGE_LOGGING_BIT_POS 4
#define CM_COMMAND_FLAGS_CENTRAL_CHARGE_LOGGING_MASK (1u << CM_COMMAND_FLAGS_CENTRAL_CHARGE_LOGGING_BIT_POS)
#define CM_COMMAND_FLAGS_CENTRAL_CHARGE_LOGGING_IS_SET(FLAGS) (((FLAGS) & CM_COMMAND_FLAGS_CENTRAL_CHARGE_LOGGING_MASK) != 0)

struct cm_command_v1 {
    uint16_t allocated_current;
    /* command_flags
    bit 7    - ignore allocation: If set, don't update allocated_current, cp_disconnect flag and allocated_phases. Added with cm_command_v3.
    bit 6    - control pilot permanently disconnected
    bit 5    - central user management enabled
    bit 4    - central charge-logging enabled
    */
    uint8_t command_flags;
    uint8_t _padding; // In use for cm_command_v2 allocated phases
};

struct cm_command_v2 {
    uint16_t _padding_0; // In use for cm_command_v1 allocated_current
    uint8_t _padding_1;  // In use for cm_command_v1 command_flags
    int8_t allocated_phases; // Was padding in CM_COMMAND_VERSION 1
};

struct cm_command_v3 {
    uint8_t charge_mode;
    // This layout allows us to add more supported charge modes with cm_command_v4 if necessary
    uint8_t supported_charge_modes[2]; // Bitmask of ConfigChargeModes. Currently there are 9 ConfigChargeModes; if this packet is a v3 packet, the upper 7 bits must be send as 0 and not be interpreted!
    uint8_t _padding;
};

struct cm_command_v4 {
    CMAuthFeedback auth_feedback;
    uint8_t _padding[3]; // In use for cm_command_v5 ev_charging_efficiency and ev_capacity
};

// Charge parameters of the connected EV.
// The client uses them to estimate the current SoC (reported back via cm_state_v6).
// Redefines cm_command_v4._padding; cm_command_packet holds a union of v4 and v5.
struct cm_command_v5 {
    uint8_t _padding;               // In use for cm_command_v4 auth_feedback
    uint8_t ev_charging_efficiency; // percent, 0 = unknown
    uint16_t ev_capacity;           // 0.1 kWh, 0 = unknown
};

// Before adding cm_command_v4: Check whether more (supported) charge modes are to be added in the foreseeable future!

#define CM_COMMAND_V1_LENGTH (sizeof(cm_command_v1))
static_assert(CM_COMMAND_V1_LENGTH == 4, "Unexpected CM_COMMAND_V1_LENGTH");

#define CM_COMMAND_V2_LENGTH (sizeof(cm_command_v2))
static_assert(CM_COMMAND_V2_LENGTH == 4, "Unexpected CM_COMMAND_V2_LENGTH");
static_assert(sizeof(cm_command_v1::_padding) == sizeof(cm_command_v2::allocated_phases), "Unexpected size of cm_command_v2.phases");
static_assert(offsetof(cm_command_v1, _padding) == offsetof(cm_command_v2, allocated_phases), "Unexpected offset of cm_command_v2.phases");

#define CM_COMMAND_V3_LENGTH (sizeof(cm_command_v3))
static_assert(CM_COMMAND_V3_LENGTH == 4, "Unexpected CM_COMMAND_V3_LENGTH");
static_assert(to_underlying(ConfigChargeMode::_max) == 9);
#define CM_COMMAND_V3_MAX_CONFIG_CHARGE_MODE 9

#define CM_COMMAND_V4_LENGTH (sizeof(cm_command_v4))
static_assert(CM_COMMAND_V4_LENGTH == 4, "Unexpected CM_COMMAND_V4_LENGTH");

#define CM_COMMAND_V5_LENGTH (sizeof(cm_command_v5))
static_assert(CM_COMMAND_V5_LENGTH == 4, "Unexpected CM_COMMAND_V5_LENGTH");
static_assert(CM_COMMAND_V5_LENGTH == CM_COMMAND_V4_LENGTH, "cm_command_v5 must redefine cm_command_v4's padding without changing the size");
static_assert(offsetof(cm_command_v5, _padding) == offsetof(cm_command_v4, auth_feedback), "Unexpected offset of cm_command_v5._padding");

struct cm_command_packet {
    cm_packet_header header;
    union {
        cm_command_v1 v1;
        cm_command_v2 v2;
    };
    cm_command_v3 v3;
    union {
        cm_command_v4 v4;
        cm_command_v5 v5;
    };
};

#define CM_COMMAND_PACKET_LENGTH (sizeof(cm_command_packet))
static_assert(CM_COMMAND_PACKET_LENGTH == 20, "Unexpected CM_COMMAND_PACKET_LENGTH");

#define CM_FEATURE_FLAGS_URGENT_BIT_POS 31
#define CM_FEATURE_FLAGS_URGENT_MASK (1u << CM_FEATURE_FLAGS_URGENT_BIT_POS)
#define CM_FEATURE_FLAGS_URGENT_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_URGENT_MASK) != 0)
#define CM_FEATURE_FLAGS_REQUEST_REALLOCATION_BIT_POS 30
#define CM_FEATURE_FLAGS_REQUEST_REALLOCATION_MASK (1u << CM_FEATURE_FLAGS_REQUEST_REALLOCATION_BIT_POS)
#define CM_FEATURE_FLAGS_REQUEST_REALLOCATION_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_REQUEST_REALLOCATION_MASK) != 0)

#define CM_FEATURE_FLAGS_PHASE_SWITCH_BIT_POS 7
#define CM_FEATURE_FLAGS_PHASE_SWITCH_MASK (1u << CM_FEATURE_FLAGS_PHASE_SWITCH_BIT_POS)
#define CM_FEATURE_FLAGS_PHASE_SWITCH_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_PHASE_SWITCH_MASK) != 0)
#define CM_FEATURE_FLAGS_CP_DISCONNECT_BIT_POS 6
#define CM_FEATURE_FLAGS_CP_DISCONNECT_MASK (1u << CM_FEATURE_FLAGS_CP_DISCONNECT_BIT_POS)
#define CM_FEATURE_FLAGS_CP_DISCONNECT_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_CP_DISCONNECT_MASK) != 0)
#define CM_FEATURE_FLAGS_EVSE_BIT_POS 5
#define CM_FEATURE_FLAGS_EVSE_MASK (1u << CM_FEATURE_FLAGS_EVSE_BIT_POS)
#define CM_FEATURE_FLAGS_EVSE_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_EVSE_MASK) != 0)
#define CM_FEATURE_FLAGS_NFC_BIT_POS 4
#define CM_FEATURE_FLAGS_NFC_MASK (1u << CM_FEATURE_FLAGS_NFC_BIT_POS)
#define CM_FEATURE_FLAGS_NFC_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_NFC_MASK) != 0)
#define CM_FEATURE_FLAGS_METER_ALL_VALUES_BIT_POS 3
#define CM_FEATURE_FLAGS_METER_ALL_VALUES_MASK (1u << CM_FEATURE_FLAGS_METER_ALL_VALUES_BIT_POS)
#define CM_FEATURE_FLAGS_METER_ALL_VALUES_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_METER_ALL_VALUES_MASK) != 0)
#define CM_FEATURE_FLAGS_METER_PHASES_BIT_POS 2
#define CM_FEATURE_FLAGS_METER_PHASES_MASK (1u << CM_FEATURE_FLAGS_METER_PHASES_BIT_POS)
#define CM_FEATURE_FLAGS_METER_PHASES_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_METER_PHASES_MASK) != 0)
#define CM_FEATURE_FLAGS_METER_BIT_POS 1
#define CM_FEATURE_FLAGS_METER_MASK (1u << CM_FEATURE_FLAGS_METER_BIT_POS)
#define CM_FEATURE_FLAGS_METER_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_METER_MASK) != 0)
#define CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_BIT_POS 0
#define CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_MASK (1u << CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_BIT_POS)
#define CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_IS_SET(FLAGS) (((FLAGS) & CM_FEATURE_FLAGS_BUTTON_CONFIGURATION_MASK) != 0)

#define CM_STATE_FLAGS_MANAGED_BIT_POS 7
#define CM_STATE_FLAGS_MANAGED_MASK (1u << CM_STATE_FLAGS_MANAGED_BIT_POS)
#define CM_STATE_FLAGS_MANAGED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_MANAGED_MASK) != 0)
#define CM_STATE_FLAGS_CP_DISCONNECTED_BIT_POS 6
#define CM_STATE_FLAGS_CP_DISCONNECTED_MASK (1u << CM_STATE_FLAGS_CP_DISCONNECTED_BIT_POS)
#define CM_STATE_FLAGS_CP_DISCONNECTED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_CP_DISCONNECTED_MASK) != 0)
#define CM_STATE_FLAGS_L1_CONNECTED_BIT_POS 5
#define CM_STATE_FLAGS_L1_CONNECTED_MASK (1u << CM_STATE_FLAGS_L1_CONNECTED_BIT_POS)
#define CM_STATE_FLAGS_L1_CONNECTED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L1_CONNECTED_MASK) != 0)
#define CM_STATE_FLAGS_L2_CONNECTED_BIT_POS 4
#define CM_STATE_FLAGS_L2_CONNECTED_MASK (1u << CM_STATE_FLAGS_L2_CONNECTED_BIT_POS)
#define CM_STATE_FLAGS_L2_CONNECTED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L2_CONNECTED_MASK) != 0)
#define CM_STATE_FLAGS_L3_CONNECTED_BIT_POS 3
#define CM_STATE_FLAGS_L3_CONNECTED_MASK (1u << CM_STATE_FLAGS_L3_CONNECTED_BIT_POS)
#define CM_STATE_FLAGS_L3_CONNECTED_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L3_CONNECTED_MASK) != 0)
#define CM_STATE_FLAGS_CONNECTED_BIT_POS 3
#define CM_STATE_FLAGS_CONNECTED_MASK (0x7 << CM_STATE_FLAGS_CONNECTED_BIT_POS)
#define CM_STATE_FLAGS_CONNECTED_GET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_CONNECTED_MASK) >> 3)
#define CM_STATE_FLAGS_L1_ACTIVE_BIT_POS 2
#define CM_STATE_FLAGS_L1_ACTIVE_MASK (1u << CM_STATE_FLAGS_L1_ACTIVE_BIT_POS)
#define CM_STATE_FLAGS_L1_ACTIVE_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L1_ACTIVE_MASK) != 0)
#define CM_STATE_FLAGS_L2_ACTIVE_BIT_POS 1
#define CM_STATE_FLAGS_L2_ACTIVE_MASK (1u << CM_STATE_FLAGS_L2_ACTIVE_BIT_POS)
#define CM_STATE_FLAGS_L2_ACTIVE_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L2_ACTIVE_MASK) != 0)
#define CM_STATE_FLAGS_L3_ACTIVE_BIT_POS 0
#define CM_STATE_FLAGS_L3_ACTIVE_MASK (1u << CM_STATE_FLAGS_L3_ACTIVE_BIT_POS)
#define CM_STATE_FLAGS_L3_ACTIVE_IS_SET(FLAGS) (((FLAGS) & CM_STATE_FLAGS_L3_ACTIVE_MASK) != 0)
#define CM_STATE_FLAGS_ACTIVE_BIT_POS 0
#define CM_STATE_FLAGS_ACTIVE_MASK (0x7 << CM_STATE_FLAGS_ACTIVE_BIT_POS)
#define CM_STATE_FLAGS_ACTIVE_GET(FLAGS) ((FLAGS) & CM_STATE_FLAGS_ACTIVE_MASK)

struct cm_state_v1 {
    /* feature_flags
    bit 31 - "urgent": Request immediate response after processing this packet
    bit 30 - "request_reallocation": Request reallocation between processing this packet and generating the response
    [...]
    bit 7 - has phase_switch
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
    uint32_t car_stopped_charging;
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

struct cm_state_v2 {
    uint32_t time_since_state_change;
};

#define CM_STATE_V2_LENGTH (sizeof(cm_state_v2))
static_assert(CM_STATE_V2_LENGTH == 4, "Unexpected CM_STATE_V2_LENGTH");

struct cm_state_v3 {
    // bit 3: currently switching
    // bit 2: can switch phases now
    // bit 0-1: phases "connected"
    uint8_t phases;
    uint8_t padding[3]; // padding[0] in use for cm_state_v6 ev_soc
};

#define CM_STATE_V3_CURRENTLY_SWITCHING_BIT_POS 3
#define CM_STATE_V3_CURRENTLY_SWITCHING_MASK (1u << CM_STATE_V3_CURRENTLY_SWITCHING_BIT_POS)
#define CM_STATE_V3_CURRENTLY_SWITCHING_IS_SET(PHASES) (((PHASES) & CM_STATE_V3_CURRENTLY_SWITCHING_MASK) != 0)
#define CM_STATE_V3_CAN_PHASE_SWITCH_BIT_POS 2
#define CM_STATE_V3_CAN_PHASE_SWITCH_MASK (1u << CM_STATE_V3_CAN_PHASE_SWITCH_BIT_POS)
#define CM_STATE_V3_CAN_PHASE_SWITCH_IS_SET(PHASES) (((PHASES) & CM_STATE_V3_CAN_PHASE_SWITCH_MASK) != 0)
#define CM_STATE_V3_PHASES_CONNECTED_MASK 0x03u
#define CM_STATE_V3_PHASES_CONNECTED_GET(PHASES) ((PHASES) & CM_STATE_V3_PHASES_CONNECTED_MASK)

#define CM_STATE_V3_LENGTH (sizeof(cm_state_v3))
static_assert(CM_STATE_V3_LENGTH == 4, "Unexpected CM_STATE_V3_LENGTH");

struct cm_state_v4 {
    uint8_t requested_charge_mode;
    uint8_t _padding[3]; // in use for cm_state_v5 auth_method and last_seen_s
};

#define CM_STATE_V4_LENGTH (sizeof(cm_state_v4))
static_assert(CM_STATE_V4_LENGTH == 4, "Unexpected CM_STATE_V4_LENGTH");

#define CM_AUTH_INFO_TAG_ID_STRING_LEN (10 * 3 )

struct cm_auth_info {
    uint8_t _padding; // in use for cm_state_v4 requested_charge_mode
    CMAuthType auth_method;
    uint16_t last_seen_s;

    // Tagged by auth_method
    union {
        struct {
            uint8_t tag_type;
            uint8_t tag_id_len;
            uint8_t tag_id[10];
        } nfc;
        struct {
            uint8_t mac[6];
        } ev;
    };
};

inline bool operator!=(const cm_auth_info &first, const cm_auth_info &second)
{
    if (first.auth_method != second.auth_method) {
        return true;
    }

    if (first.last_seen_s > second.last_seen_s) {
        return true;
    }

    uint8_t offset = sizeof(uint8_t) + sizeof(CMAuthType) + sizeof(uint16_t);
    uint8_t cropped_length = sizeof(cm_auth_info) - offset;
    if (memcmp((uint8_t *)&first + offset, (uint8_t *)&second + offset, cropped_length) != 0) {
        return true;
    }

    return false;
}
#define CM_AUTH_INFO_LENGTH (sizeof(cm_auth_info))
static_assert(CM_AUTH_INFO_LENGTH == 16, "Unexpected CM_AUTH_INFO_LENGTH");

struct cm_state_v5 {
    cm_auth_info auth_info[3];
};

#define CM_STATE_V5_LENGTH (sizeof(cm_state_v5))
static_assert(CM_STATE_V5_LENGTH == 48, "Unexpected CM_STATE_V5_LENGTH");

#define CM_STATE_V6_EV_SOC_UNKNOWN 255

// Current SoC of the connected EV, estimated by the client from the
// charge parameters received via cm_command_v5 (or known locally),
// or with ISO 15118-20 reported directly by the EV.
// Note: senders with version < 6 don't zero cm_state_v3.padding, so this
// field must only be interpreted if header.version >= 6.
struct cm_state_v6 {
    uint8_t _padding; // In use for cm_state_v3 phases
    uint8_t ev_soc;   // percent (0-100), 255 = unknown
    uint8_t _padding2[2];
};

#define CM_STATE_V6_LENGTH (sizeof(cm_state_v6))
static_assert(CM_STATE_V6_LENGTH == 4, "Unexpected CM_STATE_V6_LENGTH");
static_assert(CM_STATE_V6_LENGTH == CM_STATE_V3_LENGTH, "cm_state_v6 must redefine cm_state_v3's padding without changing the size");
static_assert(offsetof(cm_state_v6, _padding) == offsetof(cm_state_v3, phases), "Unexpected offset of cm_state_v6._padding");

struct cm_state_packet {
    cm_packet_header header;
    cm_state_v1 v1;
    cm_state_v2 v2;
    union {
        cm_state_v3 v3;
        cm_state_v6 v6;
    };
    union {
        cm_state_v4 v4;
        cm_state_v5 v5;
    };
};

#define CM_STATE_PACKET_LENGTH (sizeof(cm_state_packet))
static_assert(CM_STATE_PACKET_LENGTH == 136, "Unexpected CM_STATE_PACKET_LENGTH");

#include "generated/client_error.enum.h"

// #include "generated/module_available_end.h"
