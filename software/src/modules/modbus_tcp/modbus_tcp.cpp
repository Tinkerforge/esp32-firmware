/* esp32-firmware
 * Copyright (C) 2022 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "modbus_tcp.h"

#include <Arduino.h>
#include <esp_modbus_common.h>
#include <esp_modbus_slave.h>
#include <esp_netif.h>
#include <math.h>

#include "event_log_prefix.h"
#include "build.h"
#include "string_builder.h"

#include "module_dependencies.h"

extern uint32_t local_uid_num;

// MODBUS TABLE CHANGELOG
// 1 - Initial release
// 2 - Add coils 1000, 1001
// 3 - Add phase switch, EVSE LED color, EVSE GPIOs, NFC tag injection
#define MODBUS_TABLE_VERSION 3

static uint8_t hextouint(const char c)
{
    uint8_t i = 0;
    if (c > 96)
        i = c - 'a' + 10;
    else if (c > 64)
        i = c - 'A' + 10;
    else
        i = c - '0';
    return i;
}

static uint32_t export_tag_id_as_uint32(const String &str)
{
    int str_idx = 0;
    char c[4];

    for (int i = 3; i >= 0; i--) {
        c[i] = hextouint(str[str_idx++]) << 4;
        c[i] |= hextouint(str[str_idx++]);
        str_idx++;
    }
    uint32_t *ret = reinterpret_cast<uint32_t *>(c);
    return *ret;
}

uint32_t keba_get_features()
{
    /*
        ABCDEF
        |||||0 - No RFID
        |||||1 - RFID
        ||||1 - Standard Energy Meter, not calibrated
        ||||2 - MID
        ||||3 - Eichrecht
        |||0 - x-series
        |||1 - c-series
        ||1 - 13 A
        ||2 - 16 A
        ||3 - 20 A
        ||4 - 32 A
        |0 - Socket
        |1 - Cable
        3 - KC-P30
    */
    static bool warned;
    uint32_t features = 310000; // Report KC-P30 with cable

    // Map current
    if (api.hasFeature("evse"))
    {
        switch (api.getState("evse/hardware_configuration")->get("jumper_configuration")->asUint())
        {
            case 2: // 13 A
                features += 1000;
                break;

            case 3: // 16 A
                features += 2000;
                break;

            case 4: // 20 A
                features += 3000;
                break;

            case 6: // 32 A
                features += 4000;
                break;

            case 0: // 6 A
            case 1: // 10 A
            case 5: // 25 A
            case 7: // Unused
            case 8: // not configured
            default:
                logger.printfln("No matching keba cable configuration! It will be set to 0!");
                break;
        }
    }
    // Report c-series:
    // x-series has a 4G modem and ISO15118 hardware support
    features += 100;
    if (api.hasFeature("meter")) {
        features += 20;
    } else {
        // All keba boxes have some kind of energy meter.
        // TODO: should we still report 10 (not calibrared meter) here for better compatibility?
        if (!warned) {
            logger.printfln("Wallbox has no meter. Errors are expected!");
            warned = true;
        }
    }

    if (api.hasFeature("nfc"))
        features += 1;
    return features;
}

ModbusTcp::ModbusTcp() : server(TFModbusTCPByteOrder::Network)
{
}

void ModbusTcp::pre_setup()
{
    config = Config::Object({
        {"enable", Config::Bool(false)},
        {"port", Config::Uint16(502)},
        {"table", Config::Enum(RegisterTable::WARP, RegisterTable::WARP, RegisterTable::KEBA)},
        {"send_illegal_data_address", Config::Bool(true)}
    });
}

// TODO this requires the NFC module!
static inline void fillTagCache(Option<NFC::tag_info_t> &tag) {
    if (tag.is_none()) {
        const auto &seen_tag = nfc.old_tags[0];
        const auto &injected_tag = nfc.old_tags[TAG_LIST_LENGTH - 1];
        if ((seen_tag.last_seen == 0 || seen_tag.last_seen > injected_tag.last_seen) && injected_tag.last_seen > 0)
            tag = {injected_tag};
        else
            tag = {seen_tag};

        int written = remove_separator(tag.unwrap().tag_id, tag.unwrap().tag_id);
        memset(tag.unwrap().tag_id + written, 0, sizeof(tag.unwrap().tag_id) - written);
        //swap_bytes(tag.unwrap().tag_id, 20);
    }
}

static inline uint32_t swapBytes(uint32_t x) {
    return ((x & 0x000000FF) << 24)
         | ((x & 0x0000FF00) << 8)
         | ((x & 0x00FF0000) >> 8)
         | ((x & 0xFF000000) >> 24);
}

static inline uint16_t swapBytes(uint16_t x) {
    return (x >> 8) | (x << 8);
}

#define FILL_FEATURE_CACHE(x) \
    if (!cache->has_feature_##x)\
        cache->has_feature_##x = api.hasFeature(#x);

#define REQUIRE(x) if(!cache->has_feature_##x) break

Option<ModbusTcp::TwoRegs> ModbusTcp::getWarpInputRegister(uint16_t reg, void *ctx_ptr) {
    struct Ctx{
        Option<float> energy_abs = {};
        Option<NFC::tag_info_t> tag = {};
    };
    Ctx *ctx = (Ctx*) ctx_ptr;

    ModbusTcp::TwoRegs val{0};

    bool report_illegal_data_address = false;

    switch (reg) {
        case 0: val.u = MODBUS_TABLE_VERSION; break;
        case 2: val.u = BUILD_VERSION_MAJOR; break;
        case 4: val.u = BUILD_VERSION_MINOR; break;
        case 6: val.u = BUILD_VERSION_PATCH; break;
        case 8: val.u = build_timestamp(); break;
        case 10: val.u = local_uid_num; break;
        case 12: val.u = (uint32_t)(int64_t)(now_us() / 1_s); break;

        case 1000: REQUIRE(evse); val.u = cache->evse_state->get("iec61851_state")->asUint(); break;
        case 1002: REQUIRE(evse); val.u = cache->evse_state->get("charger_state")->asUint(); break;
#if MODULE_CHARGE_TRACKER_AVAILABLE()
                   // We want to return UINT32_MAX if nobody is charging right now.
                   // If nobody is charging, the user_id is -1 which is UINT32_MAX when casted to uint32_t.
        case 1004: val.u = (uint32_t) cache->current_charge->get("user_id")->asInt(); break;
                    // timestamp_minutes is 0 if nobody is charging or if there was no time sync when the charge started.
        case 1006: val.u = cache->current_charge->get("timestamp_minutes")->asUint(); break;
        case 1008: REQUIRE(evse); {
                uint32_t now = cache->evse_ll_state->get("uptime")->asUint();
                uint32_t start = cache->current_charge->get("evse_uptime_start")->asUint();
                if (start == 0)
                    val.u = 0;
                else if (now >= start)
                    val.u = now - start;
                else
                    val.u = UINT32_MAX - start + now + 1;
            }
            break;
#endif
        case 1010: REQUIRE(evse); val.u = cache->evse_state->get("allowed_charging_current")->asUint(); break;
        //1012... handled below

        case 2000: REQUIRE(meter); val.u = cache->meter_state->get("type")->asUint(); break;
        case 2002: REQUIRE(meter); val.f = cache->meter_values->get("power")->asFloat(); break;
        case 2004: REQUIRE(meter); val.f = ctx->energy_abs.is_some() ? ctx->energy_abs.unwrap() : ctx->energy_abs.insert(cache->meter_values->get("energy_abs")->asFloat()); break;
        case 2006: REQUIRE(meter); val.f = cache->meter_values->get("energy_rel")->asFloat(); break;
#if MODULE_CHARGE_TRACKER_AVAILABLE()
        case 2008: REQUIRE(meter); {
                if (cache->current_charge->get("user_id")->asInt() == -1) {
                    val.f = 0;
                    break;
                }
                auto en_abs = ctx->energy_abs.is_some() ? ctx->energy_abs.unwrap() : ctx->energy_abs.insert(cache->meter_values->get("energy_abs")->asFloat());
                val.f = en_abs - cache->current_charge->get("meter_start")->asFloat();
            }
            break;
#endif
        //2100... handled below

        case 3100: val.u = power_manager.get_phases(); break;
        case 3102: REQUIRE(phase_switch); val.u = cache->power_manager_state->get("external_control")->asUint(); break;

        //4000... handled below
        case 4010: REQUIRE(nfc); {
                fillTagCache(ctx->tag);
                val.u = ctx->tag.unwrap().last_seen;
                // We want to support both 1 and 2 and '1' and '2' as injection types inject_tag_start/_stop.
                // Other values are interpreted as 0, i.e. inject_tag
                // "Reserve" by lying about a tags age if it is younger than 0x70 (112) ms.
                // This allows a user of the modbus API to read input registers 4000 - 4013 after presenting a physical tag
                // and then inject the same tag by writing the same data to holding registers 4000 - 4013.
                if (val.u != 0 && val.u < 0x70)
                    val.u += 0x70;
            } break;
        case 4012: REQUIRE(nfc); fillTagCache(ctx->tag); val.u = 0x30303000 + ('0' + ctx->tag.unwrap().tag_type); break;

        default: report_illegal_data_address = true; break;
    }

    // RANGES
    if (reg >= 1012 && reg < 1012 + 2 * CHARGING_SLOT_COUNT_SUPPORTED_BY_EVSE) {
        report_illegal_data_address = false;

        if (cache->has_feature_evse) {
            size_t slot_idx = (reg - 1012) / 2;
            if (slot_idx < cache->evse_slots->count()) {
                auto slot = cache->evse_slots->get(slot_idx);
                val.u = slot->get("active")->asBool() ? slot->get("max_current")->asUint() : 0xFFFFFFFF;
            } else {
                val.u = 0xFFFFFFFF;
            }
        }
    } else if (reg >= 2100 && reg < 2100 + 2 * 85) {
        report_illegal_data_address = false;

        if (cache->has_feature_meter_all_values) {
            auto value_idx = (reg - 2100) / 2;
            val.f = cache->meter_all_values->get(value_idx)->asFloat();
        }
    } else if (reg >= 4000 && reg < 4000 + NFC_TAG_ID_LENGTH) {
        report_illegal_data_address = false;

        if (cache->has_feature_nfc) {
            fillTagCache(ctx->tag);
            auto value_idx = (reg - 4000) * 2;
            memcpy(&val, ctx->tag.unwrap().tag_id + value_idx, 4);
            // the ID is already in network order, but this function should return values in host order.
            val.u = swapBytes(val.u);
        }
    }

    if (this->send_illegal_data_address && report_illegal_data_address)
        return {};

    return {val};
}

TFModbusTCPExceptionCode ModbusTcp::getWarpInputRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values) {
    struct {
        Option<float> energy_abs = {};
        Option<NFC::tag_info_t> tag = {};
    } ctx;

    FILL_FEATURE_CACHE(evse)
    FILL_FEATURE_CACHE(meter)
    FILL_FEATURE_CACHE(meter_all_values)
    FILL_FEATURE_CACHE(nfc)
    FILL_FEATURE_CACHE(phase_switch)

    int i = 0;
    while (i < data_count) {
        uint16_t reg = (i + start_address) & (~1);

        Option<TwoRegs> opt = this->getWarpInputRegister(reg, &ctx);
        if (opt.is_none())
            return TFModbusTCPExceptionCode::IllegalDataAddress;

        TwoRegs val = opt.unwrap();

        val.u = swapBytes(val.u);

        // TODO: comment
        if (i == 0 && (start_address % 2) == 1) {
            data_values[i] = val.regs.lower;
            ++i;
        } else if (i == data_count - 1) {
            data_values[i] = val.regs.upper;
            ++i;
        } else {
            data_values[i] = val.regs.upper;
            data_values[i + 1] = val.regs.lower;
            i += 2;
        }
    }

    return TFModbusTCPExceptionCode::Success;
}

Option<ModbusTcp::TwoRegs> ModbusTcp::getWarpHoldingRegister(uint16_t reg) {
    TwoRegs val{0};

    bool report_illegal_data_address = false;

    switch (reg) {
        case 0: val.u = 0; break;

        case 1000: REQUIRE(evse); {
                auto slot = cache->evse_slots->get(CHARGING_SLOT_MODBUS_TCP_ENABLE);
                val.u = slot->get("active")->asBool() ? slot->get("max_current")->asUint() : 0xFFFFFFFF;
            } break;
        case 1002: REQUIRE(evse); {
                auto slot = cache->evse_slots->get(CHARGING_SLOT_MODBUS_TCP);
                val.u = slot->get("active")->asBool() ? slot->get("max_current")->asUint() : 0xFFFFFFFF;
            } break;
                    // We want to return UINT32_MAX if the EVSE controls the LED.
                    // (uint32_t)(-1) is UINT32_MAX.
        case 1004: REQUIRE(evse); val.u = (uint32_t)cache->evse_indicator_led->get("indication")->asInt(); break;
        case 1006: REQUIRE(evse); val.u = cache->evse_indicator_led->get("duration")->asUint(); break;

        case 2000: REQUIRE(meter); val.u = 0; break;

        case 3100: REQUIRE(phase_switch); val.u = cache->power_manager_external_control->get("phases_wanted")->asUint(); break;

        default: report_illegal_data_address = true;
    }

    if (reg >= 4000 && reg < 4014) {
        report_illegal_data_address = false;

        if (cache->has_feature_nfc) {;
            // The NFC ID is already in network order but will be swapped.
            for(size_t i = 0; i < 4; ++i) {
                val.chars[i] = cache->nfc_tag_injection_buffer[(reg - 4000) * 2 + i];
            }
            val.u = swapBytes(val.u);
        }
        logger.printfln_debug("READ %u %.*s", reg, 4, val.chars);
    }

    if (this->send_illegal_data_address && report_illegal_data_address)
        return {};

    return {val};
}

TFModbusTCPExceptionCode ModbusTcp::getWarpHoldingRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values) {
    FILL_FEATURE_CACHE(evse)
    FILL_FEATURE_CACHE(meter)
    FILL_FEATURE_CACHE(nfc)

    int i = 0;
    while (i < data_count) {
        uint16_t reg = (i + start_address) & (~1);

        Option<ModbusTcp::TwoRegs> opt = this->getWarpHoldingRegister(reg);
        if (opt.is_none())
            return TFModbusTCPExceptionCode::IllegalDataAddress;

        auto val = opt.unwrap();

        val.u = swapBytes(val.u);

        // TODO: comment
        if (i == 0 && (start_address % 2) == 1) {
            data_values[i] = val.regs.lower;
            ++i;
        } else if (i == data_count - 1) {
            data_values[i] = val.regs.upper;
            ++i;
        } else {
            data_values[i] = val.regs.upper;
            data_values[i + 1] = val.regs.lower;
            i += 2;
        }
    }

    return TFModbusTCPExceptionCode::Success;
}

TFModbusTCPExceptionCode ModbusTcp::getWarpDiscreteInputs(uint16_t start_address, uint16_t data_count, uint8_t *data_values) {
    FILL_FEATURE_CACHE(evse)
    FILL_FEATURE_CACHE(meter)
    FILL_FEATURE_CACHE(meter_phases)
    FILL_FEATURE_CACHE(meter_all_values)
    FILL_FEATURE_CACHE(nfc)

    int i = 0;
    while (i < data_count) {
        size_t byte_idx = i / 8;
        size_t bit_idx = i % 8;

        bool result = false;

        switch (i + start_address) {
            case 0: result = cache->has_feature_evse; break;
            case 1: result = cache->has_feature_meter; break;
            case 2: result = cache->has_feature_meter_phases; break;
            case 3: result = cache->has_feature_meter_all_values; break;
            case 4: result = cache->has_feature_phase_switch; break;
            case 5: result = cache->has_feature_nfc; break;
            case 6: REQUIRE(evse); result = cache->evse_hardware_configuration->get("evse_version")->asUint() >= 20; break;
            case 7: REQUIRE(evse); {
                auto version = cache->evse_hardware_configuration->get("evse_version")->asUint();
                result = version >= 20 && version < 30;
            } break;
            case 8: REQUIRE(evse); {
                auto version = cache->evse_hardware_configuration->get("evse_version")->asUint();
                result = version >= 20 && version < 30;
            } break;

            case 1100: REQUIRE(evse); {
                auto version = cache->evse_hardware_configuration->get("evse_version")->asUint();
                if (version >= 20 && version < 30)
                    result = cache->evse_ll_state->get("gpio")->get(5)->asBool();
                else if (version >= 30)
                    result = cache->evse_ll_state->get("gpio")->get(18)->asBool();
            } break;
            case 1101: REQUIRE(evse); {
                auto version = cache->evse_hardware_configuration->get("evse_version")->asUint();
                if (version >= 20 && version < 30)
                    result = cache->evse_ll_state->get("gpio")->get(16)->asBool();
            } break;

            case 2100: REQUIRE(meter_phases); result = cache->meter_phases->get("phases_connected")->get(0)->asBool(); break;
            case 2101: REQUIRE(meter_phases); result = cache->meter_phases->get("phases_connected")->get(1)->asBool(); break;
            case 2102: REQUIRE(meter_phases); result = cache->meter_phases->get("phases_connected")->get(2)->asBool(); break;
            case 2103: REQUIRE(meter_phases); result = cache->meter_phases->get("phases_active")->get(0)->asBool(); break;
            case 2104: REQUIRE(meter_phases); result = cache->meter_phases->get("phases_active")->get(1)->asBool(); break;
            case 2105: REQUIRE(meter_phases); result = cache->meter_phases->get("phases_active")->get(2)->asBool(); break;

            default: if (this->send_illegal_data_address) return TFModbusTCPExceptionCode::IllegalDataAddress;
        }

        if (result)
            data_values[byte_idx] |= (1 << bit_idx);
        else
            data_values[byte_idx] &= ~(1 << bit_idx);
        ++i;
    }

    return TFModbusTCPExceptionCode::Success;
}

TFModbusTCPExceptionCode ModbusTcp::getWarpCoils(uint16_t start_address, uint16_t data_count, uint8_t *data_values) {
    FILL_FEATURE_CACHE(evse)

    int i = 0;
    while (i < data_count) {
        size_t byte_idx = i / 8;
        size_t bit_idx = i % 8;

        bool result = false;

        switch (i + start_address) {
            case 1000: REQUIRE(evse); {
                    auto slot = cache->evse_slots->get(CHARGING_SLOT_MODBUS_TCP_ENABLE);
                    result = !slot->get("active")->asBool() || slot->get("max_current")->asUint() > 0;
                } break;
            case 1001: REQUIRE(evse); {
                    auto slot = cache->evse_slots->get(CHARGING_SLOT_AUTOSTART_BUTTON);
                    result = !slot->get("active")->asBool() || slot->get("max_current")->asUint() > 0;
                } break;
            case 1100: REQUIRE(evse); {
                    if (cache->evse_gp_output != nullptr)
                        result = cache->evse_gp_output->get("gp_output")->asUint() > 0;
                } break;

            default: if (this->send_illegal_data_address) return TFModbusTCPExceptionCode::IllegalDataAddress;
        }

        if (result)
            data_values[byte_idx] |= (1 << bit_idx);
        else
            data_values[byte_idx] &= ~(1 << bit_idx);
        ++i;
    }

    return TFModbusTCPExceptionCode::Success;
}

TFModbusTCPExceptionCode ModbusTcp::setWarpCoils(uint16_t start_address, uint16_t data_count, uint8_t *data_values) {
    FILL_FEATURE_CACHE(evse)

    if (!cache->has_feature_evse || !cache->evse_slots->get(CHARGING_SLOT_MODBUS_TCP)->get("active")->asBool())
        return TFModbusTCPExceptionCode::Success;

    int i = 0;
    while (i < data_count) {
        size_t byte_idx = i / 8;
        size_t bit_idx = i % 8;

        bool coil = data_values[byte_idx] & (1 << bit_idx);

        switch (i + start_address) {
            case 1000: REQUIRE(evse); evse_common.set_modbus_enabled(coil); break;
            case 1001: REQUIRE(evse); {
                    String err = api.callCommand(coil ? "evse/start_charging" : "evse/stop_charging", nullptr);
                    if (err != "") {
                        logger.printfln("Failed to %s charging: %s", coil ? "start" : "stop", err.c_str());
                    }
                } break;

            case 1100: REQUIRE(evse); {
                    String err = api.callCommand("evse/gp_output_update", Config::ConfUpdateObject{{
                        {"gp_output", coil ? 1 : 0}
                    }});
                    if (err != "") {
                        logger.printfln("Failed to update GP output: %s", err.c_str());
                    }
                } break;

            default: if (this->send_illegal_data_address) return TFModbusTCPExceptionCode::IllegalDataAddress;
        }

        ++i;
    }

    return TFModbusTCPExceptionCode::Success;
}

TFModbusTCPExceptionCode ModbusTcp::setWarpHoldingRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values) {
    FILL_FEATURE_CACHE(evse)
    FILL_FEATURE_CACHE(meter)
    FILL_FEATURE_CACHE(phase_switch)
    FILL_FEATURE_CACHE(nfc)

    if (!cache->has_feature_evse || !cache->evse_slots->get(CHARGING_SLOT_MODBUS_TCP)->get("active")->asBool())
        return TFModbusTCPExceptionCode::Success;

    bool report_illegal_data_address = false;

    int i = 0;
    while (i < data_count) {
        uint16_t reg = (i + start_address) & (~1);

        TwoRegs val;

        if (i == 0 && (start_address % 2) == 1) {
            val.regs.lower = data_values[i];
            Option<TwoRegs> opt = this->getWarpHoldingRegister(reg);
            if (opt.is_none())
                return TFModbusTCPExceptionCode::IllegalDataAddress;

            auto old_val = opt.unwrap();
            old_val.u = swapBytes(old_val.u);
            val.regs.upper = old_val.regs.upper;
            ++i;
        } else if (i == data_count - 1) {
            val.regs.upper = data_values[i];
            Option<TwoRegs> opt = this->getWarpHoldingRegister(reg);
            if (opt.is_none())
                return TFModbusTCPExceptionCode::IllegalDataAddress;

            auto old_val = opt.unwrap();
            old_val.u = swapBytes(old_val.u);
            val.regs.lower = old_val.regs.lower;
            ++i;
        } else {
            val.regs.upper = data_values[i];
            val.regs.lower = data_values[i + 1];
            i += 2;
        }

        val.u = swapBytes(val.u);

        switch (reg) {
            case 0: if (val.u == 0x012EB007) trigger_reboot("Modbus TCP"); break;

            case 1000: REQUIRE(evse); evse_common.set_modbus_enabled(val.u > 0); break;
            case 1002: REQUIRE(evse); evse_common.set_modbus_current(val.u); break;
            case 1004: REQUIRE(evse); {
                    // Only accept evse led write if both indication and duration are written in one request.
                    // i was already incremented above, so we only need 2 more bytes.
                    if (data_count - i < 2) {
                        logger.printfln("Received write to EVSE LED indication but without duration! Please write all 4 registers starting at 1004 in one request.");
                        break;
                    }

                    uint16_t duration = swapBytes(data_values[i + 1]);
                    i += 2;

                    uint16_t h = 0;
                    uint8_t s = 0;
                    uint8_t v = 0;

                    if (data_count - i >= 6) {
                        h = swapBytes(data_values[i + 1]);
                        i += 2;
                        s = data_values[i + 1] >> 8;
                        i += 2;
                        v = data_values[i + 1] >> 8;
                        i += 2;
                    }
                    evse_led.set_api(EvseLed::Blink(val.u), duration, h, s, v);
                } break;
            case 1006: REQUIRE(evse); logger.printfln("Received write to EVSE LED duration but without indication! Please write all 4 registers starting at 1004 in one request."); break;
            case 1008: REQUIRE(evse); logger.printfln("Received write to EVSE LED hue but without indication! Please write all 10 registers starting at 1004 in one request."); break;
            case 1010: REQUIRE(evse); logger.printfln("Received write to EVSE LED saturation but without indication! Please write all 10 registers starting at 1004 in one request."); break;
            case 1012: REQUIRE(evse); logger.printfln("Received write to EVSE LED value but without indication! Please write all 10 registers starting at 1004 in one request."); break;

            case 2000: REQUIRE(meter); {
                    if (val.u == 0x3E12E5E7) {
                        String err = api.callCommand("meter/reset", {});
                        if (err != "") {
                            logger.printfln("Failed to reset energy meter: %s", err.c_str());
                        }
                    }
                } break;
            case 3100: REQUIRE(phase_switch); {
                    if (cache->power_manager_state->get("external_control")->asUint() == 0) {
                        String err = api.callCommand("power_manager/external_control_update", Config::ConfUpdateObject{{
                            {"phases_wanted", val.u}
                        }});
                        if (err != "") {
                            logger.printfln("Failed to switch phases: %s", err.c_str());
                        }
                    }
                } break;

            default: report_illegal_data_address = true;
        }

        if (reg >= 4000 && reg < 4014) {
            report_illegal_data_address = false;

            if (cache->has_feature_nfc) {
                // The NFC ID was already in network order but was swapped above.
                val.u = swapBytes(val.u);

                for(size_t j = 0; j < 4; ++j) {
                    cache->nfc_tag_injection_buffer[(reg - 4000) * 2 + j] = val.chars[j];
                }

                // Inject tag if the tag type register(s) were written, but only if either the lower or both registers were written.
                if (reg == 4012 && ((i + start_address == 4013) || data_count > 1)) {
                    char buf[NFC_TAG_ID_STRING_LENGTH + 1];
                    StringWriter sw{buf, ARRAY_SIZE(buf)};

                    for(size_t j = 0; j < NFC_TAG_ID_LENGTH; ++j) {
                        if (cache->nfc_tag_injection_buffer[2 * j] == 0)
                            break;
                        sw.putc(cache->nfc_tag_injection_buffer[2 * j]);
                        sw.putc(cache->nfc_tag_injection_buffer[2 * j + 1]);
                        sw.putc(':');
                    }
                    sw.setLength(sw.getLength() - 1);

                    // Accepts both 0 - 5 and '0' to '5'
                    uint32_t tag_type = cache->nfc_tag_injection_buffer[ARRAY_SIZE(cache->nfc_tag_injection_buffer) - 1] & 0x0F;
                    // Accepts both 0 - 2 and '0' to '2'
                    uint8_t injection_type = cache->nfc_tag_injection_buffer[ARRAY_SIZE(cache->nfc_tag_injection_buffer) - 5] & 0x0F;

                    const char *command = "nfc/inject_tag";
                    if (injection_type == 1)
                        command = "nfc/inject_tag_start";
                    else if (injection_type == 2)
                        command = "nfc/inject_tag_stop";

                    String err = api.callCommand(command, Config::ConfUpdateObject{{
                            {"tag_type", tag_type},
                            {"tag_id", CoolString{buf}}
                        }});
                    if (err != "") {
                        logger.printfln("Failed to inject tag: %s", err.c_str());
                    }
                    memset(cache->nfc_tag_injection_buffer, 0, sizeof(cache->nfc_tag_injection_buffer));
                }
            }
        }
    }

    return (this->send_illegal_data_address && report_illegal_data_address) ? TFModbusTCPExceptionCode::IllegalDataAddress : TFModbusTCPExceptionCode::Success;
}

TFModbusTCPExceptionCode ModbusTcp::setKebaHoldingRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values) {
    FILL_FEATURE_CACHE(evse)
    FILL_FEATURE_CACHE(phase_switch)

    if (!cache->has_feature_evse || !cache->evse_slots->get(CHARGING_SLOT_MODBUS_TCP)->get("active")->asBool())
        return TFModbusTCPExceptionCode::Success;

    int i = 0;
    while (i < data_count) {
        uint16_t reg = i + start_address;
        uint16_t val = swapBytes(data_values[i]);

        switch (reg) {
            case 5004: REQUIRE(evse); evse_common.set_modbus_current(val); break;
            case 5010: REQUIRE(evse); {
                    String err = api.callCommand("charge_limits/override_energy", Config::ConfUpdateObject{{
                        {"energy_wh", (uint32_t)(val * 10)}
                    }});
                    if (err != "")  {
                        logger.printfln("Failed to set energy limit: %s", err.c_str());
                    }
                }
                break;
            case 5012: break; // We can't unlock the connector.
            case 5014: REQUIRE(evse); evse_common.set_modbus_enabled(val > 0); break;
            case 5016: break; // We don't support the failsafe.
            case 5018: break; // We don't support the failsafe.
            case 5020: break; // We don't support the failsafe.

            case 5050: break; // We don't support setting the phase switch communication channel.
            case 5052: REQUIRE(phase_switch); {
                    if (cache->power_manager_state->get("external_control")->asUint() == 0) {
                        String err = api.callCommand("power_manager/external_control_update", Config::ConfUpdateObject{{
                            {"phases_wanted", val == 1 ? 3 : 1}
                        }});
                        if (err != "") {
                            logger.printfln("Failed to switch phases: %s", err.c_str());
                        }
                    }
                } break;

            default: if(this->send_illegal_data_address) return TFModbusTCPExceptionCode::IllegalDataAddress;
        }
        ++i;
    }

    return TFModbusTCPExceptionCode::Success;
}

TFModbusTCPExceptionCode ModbusTcp::setBenderHoldingRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values) {
    FILL_FEATURE_CACHE(evse)
    FILL_FEATURE_CACHE(phase_switch)

    if (!cache->has_feature_evse || !cache->evse_slots->get(CHARGING_SLOT_MODBUS_TCP)->get("active")->asBool())
        return TFModbusTCPExceptionCode::Success;

    int i = 0;
    while (i < data_count) {
        uint16_t reg = i + start_address;
        uint16_t val = swapBytes(data_values[i]);

        switch (reg) {
            case 124: REQUIRE(evse); evse_common.set_modbus_enabled(val == 0); break;
            case 1000: REQUIRE(evse); evse_common.set_modbus_current(val * 1000); break;
            default: if (this->send_illegal_data_address) return TFModbusTCPExceptionCode::IllegalDataAddress;
        }
        ++i;
    }

    return TFModbusTCPExceptionCode::Success;
}

TFModbusTCPExceptionCode ModbusTcp::getKebaHoldingRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values) {
    FILL_FEATURE_CACHE(evse)
    FILL_FEATURE_CACHE(meter)
    FILL_FEATURE_CACHE(meter_all_values)
    FILL_FEATURE_CACHE(nfc)

    int i = 0;
    while (i < data_count) {
        uint16_t reg = (i + start_address) & (~1);

        Option<ModbusTcp::TwoRegs> opt = this->getKebaHoldingRegister(reg);
        if (opt.is_none())
            return TFModbusTCPExceptionCode::IllegalDataAddress;

        auto val = opt.unwrap();

        val.u = swapBytes(val.u);

        // TODO: comment
        if (i == 0 && (start_address % 2) == 1) {
            data_values[i] = val.regs.lower;
            ++i;
        } else if (i == data_count - 1) {
            data_values[i] = val.regs.upper;
            ++i;
        } else {
            data_values[i] = val.regs.upper;
            data_values[i + 1] = val.regs.lower;
            i += 2;
        }
    }

    return TFModbusTCPExceptionCode::Success;
}

Option<ModbusTcp::TwoRegs> ModbusTcp::getKebaHoldingRegister(uint16_t reg) {
    TwoRegs val{0};

    switch (reg) {
        case 1000: REQUIRE(evse); {
                auto slot = cache->evse_slots->get(CHARGING_SLOT_MODBUS_TCP_ENABLE);
                auto iec_state = cache->evse_state->get("iec61851_state")->asUint();

                if (iec_state != 0 && slot->get("active")->asBool() && slot->get("max_current")->asUint() == 0)
                    // The charging process is temporarily interrupted because the temperature is too high or the wallbox is in suspended mode.
                    val.u = 5;
                else if (iec_state == 4)
                    // An error has occurred.
                    val.u =  4;
                else
                    // 0 is start-up, 1 to 3 are IEC states A to C
                    val.u = iec_state + 1;
            } break;
        case 1004: REQUIRE(evse); {
                auto iec_state = cache->evse_state->get("iec61851_state")->asUint();

                if (iec_state == 1)
                    // Cable is connected to the charging station and the electric vehicle (not locked)
                    val.u = 5;
                else if (iec_state == 2 || iec_state == 3)
                    // Cable is connected to the charging station and the electric vehicle and locked (charging)
                    val.u = 7;
                else
                    // Cable is connected to the charging station and locked (not to the electric vehicle).
                    val.u = 3;
            } break;
        case 1006: break; // error code
        case 1008: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_CURRENT_L1_A)->asFloat() * 1000); break;
        case 1010: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_CURRENT_L2_A)->asFloat() * 1000); break;
        case 1012: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_CURRENT_L3_A)->asFloat() * 1000); break;
        case 1014: val.u = local_uid_num; break;
        case 1016: val.u = keba_get_features(); break;
        case 1018: val.u = 0x30A1B00; break;  //3.10.27 is the last firmware version without support for the failsafe registers. We don't implement those, so report .27.
        case 1020: REQUIRE(meter); val.u = (uint32_t)(cache->meter_values->get("power")->asFloat() * 1000); break;
        case 1036: REQUIRE(meter); val.u = (uint32_t)(cache->meter_values->get("energy_abs")->asFloat() * 1000 * 10); break; // 0.1 Wh
        case 1040: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L1)->asFloat()); break;
        case 1042: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L2)->asFloat()); break;
        case 1044: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L3)->asFloat()); break;
        case 1046: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_TOTAL_SYSTEM_POWER_FACTOR)->asFloat() * 10); break; // in 0.1 %.

        case 1100: REQUIRE(evse); val.u = cache->evse_state->get("allowed_charging_current")->asUint(); break;
        case 1110: REQUIRE(evse); {
                auto incoming = cache->evse_slots->get(CHARGING_SLOT_INCOMING_CABLE)->get("max_current")->asUint();
                auto outgoing = cache->evse_slots->get(CHARGING_SLOT_OUTGOING_CABLE)->get("max_current")->asUint();
                val.u = min(incoming, outgoing);
            } break;

        case 1500: REQUIRE(nfc); {
                auto auth_type = cache->current_charge->get("authorization_type")->asUint();

                if (auth_type == 2 || auth_type == 3) {
                    const auto &tag_id = cache->current_charge->get("authorization_info")->get("tag_id")->asString();
                    val.u  = export_tag_id_as_uint32(tag_id);
                }
            } break;
#if MODULE_CHARGE_TRACKER_AVAILABLE()
        case 1502:  REQUIRE(meter); {
                if (cache->current_charge->get("user_id")->asInt() == -1) {
                    val.f = 0;
                    break;
                }
                auto en_abs = cache->meter_values->get("energy_abs")->asFloat(); // No need to cache: 1036 and 1502 can't be read in the same request.
                val.u = (uint32_t)((en_abs - cache->current_charge->get("meter_start")->asFloat()) * 1000 * 10); // 0.1 Wh
            } break;
#endif
        case 1550: val.u = cache->has_feature_phase_switch ? 3 : 0; break;
        case 1552: val.u = power_manager.get_phases(); break;
        case 1600: break; // failsafe
        case 1602: break; // failsafe

        default: if (this->send_illegal_data_address) {logger.printfln_debug("%d", this->send_illegal_data_address); return {};}
    }

    return {val};
}

TFModbusTCPExceptionCode ModbusTcp::getBenderHoldingRegisters(uint16_t start_address, uint16_t data_count, uint16_t *data_values) {
    FILL_FEATURE_CACHE(evse)
    FILL_FEATURE_CACHE(meter_all_values)

    int i = 0;
    while (i < data_count) {
        uint16_t reg = (i + start_address) & (~1);

        ModbusTcp::TwoRegs val = this->getBenderHoldingRegister(reg);

        val.u = swapBytes(val.u);

        // TODO: comment
        if (i == 0 && (start_address % 2) == 1) {
            data_values[i] = val.regs.lower;
            ++i;
        } else if (i == data_count - 1) {
            data_values[i] = val.regs.upper;
            ++i;
        } else {
            data_values[i] = val.regs.upper;
            data_values[i + 1] = val.regs.lower;
            i += 2;
        }
    }

    return TFModbusTCPExceptionCode::Success;
}

/* Bender docs:
   If there are gaps with undefined register numbers in this range, value '0' will be returned.
*/
ModbusTcp::TwoRegs ModbusTcp::getBenderHoldingRegister(uint16_t reg) {
    ModbusTcp::TwoRegs val{0};

    switch (reg) {
        case 100: val.u = 0x342E3430; break; // firmware version 4.40
        case 104: REQUIRE(evse); {
                switch (cache->evse_state->get("charger_state")->asUint()) {
                    case 0:
                        val.regs.lower = 0;
                        break;

                    case 4:
                        val.regs.lower = 4;
                        break;

                    default:
                        val.regs.lower = 1;
                        break;
                }
            } break;
        case 120: val.u = 0x302E3134; break; // protocol version 0.14
        case 122: REQUIRE(evse); val.regs.lower = cache->evse_state->get("iec61851_state")->asUint() + 1; val.regs.upper = cache->evse_state->get("iec61851_state")->asUint() + 10; break;//vehicle state hex
        case 124: REQUIRE(evse); {
                auto slot = cache->evse_slots->get(CHARGING_SLOT_MODBUS_TCP_ENABLE);
                if (!slot->get("active")->asBool()) {
                    val.regs.lower = 0;
                    break;
                }

                val.regs.lower =  slot->get("max_current")->asUint() >= 6000 ? 0 : 1; // 0 operative, 1 inoperative.
            } break;
        case 132: REQUIRE(evse); {
                auto incoming = cache->evse_slots->get(CHARGING_SLOT_INCOMING_CABLE)->get("max_current")->asUint();
                auto outgoing = cache->evse_slots->get(CHARGING_SLOT_OUTGOING_CABLE)->get("max_current")->asUint();
                val.regs.upper = min(incoming, outgoing) / 1000;
            } break;
        case 140: val.regs.upper = 0xEBEE; break;

        case 200: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_IMPORT_KWH_L1)->asFloat() * 1000); break;
        case 202: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_IMPORT_KWH_L2)->asFloat() * 1000); break;
        case 204: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_IMPORT_KWH_L3)->asFloat() * 1000); break;
        case 206: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_POWER_L1_W)->asFloat()); break;
        case 208: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_POWER_L2_W)->asFloat()); break;
        case 210: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_POWER_L3_W)->asFloat()); break;
        case 212: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_CURRENT_L1_A)->asFloat() * 1000); break;
        case 214: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_CURRENT_L2_A)->asFloat() * 1000); break;
        case 216: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_CURRENT_L3_A)->asFloat() * 1000); break;

        case 218: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_TOTAL_IMPORT_KWH)->asFloat() * 1000); break;
        case 220: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_TOTAL_SYSTEM_POWER_W)->asFloat()); break;

        case 222: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L1)->asFloat()); break;
        case 224: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L2)->asFloat()); break;
        case 226: REQUIRE(meter_all_values); val.u = (uint32_t)(cache->meter_all_values->get(METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L3)->asFloat()); break;

        case 706: REQUIRE(evse); val.u = cache->evse_state->get("allowed_charging_current")->asUint() / 1000; break;
#if MODULE_CHARGE_TRACKER_AVAILABLE()
        case 709:
        case 718: REQUIRE(evse); {
                uint32_t now = cache->evse_ll_state->get("uptime")->asUint();
                uint32_t start = cache->current_charge->get("evse_uptime_start")->asUint();
                if (start == 0)
                    val.u = 0;
                else if (now >= start)
                    val.u = now - start;
                else
                    val.u = UINT32_MAX - start + now + 1;
            } break;
        case 705:
        case 716: REQUIRE(meter); {
                if (cache->current_charge->get("user_id")->asInt() == -1) {
                    val.u = 0;
                    break;
                }
                auto en_abs = cache->meter_values->get("energy_abs")->asFloat();
                val.u = (en_abs - cache->current_charge->get("meter_start")->asFloat()) * 1000;
            }
            break;
#endif
        default: break;
    }

    return val;
}

void ModbusTcp::start_server() {
    cache = std::unique_ptr<Cache>(new Cache());
    fillCache();

    auto table = config.get("table")->asEnum<RegisterTable>();

    this->send_illegal_data_address = config.get("send_illegal_data_address")->asBool();

    server.start(
        0, config.get("port")->asUint(),
        [](uint32_t peer_address, uint16_t port) {
            logger.printfln("client connected: peer_address=%u port=%u", peer_address, port);
        },
        [](uint32_t peer_address, uint16_t port, TFModbusTCPServerDisconnectReason reason, int error_number) {
            logger.printfln("client disconnected: peer_address=%u port=%u reason=%s error_number=%d", peer_address, port, get_tf_modbus_tcp_server_client_disconnect_reason_name(reason), error_number);
        },
        [this, table](uint8_t unit_id, TFModbusTCPFunctionCode function_code, uint16_t start_address, uint16_t data_count, void *data_values) {
            switch(function_code) {
                case TFModbusTCPFunctionCode::ReadCoils:
                    switch (table) {
                        case RegisterTable::WARP:
                            return this->getWarpCoils(start_address, data_count, (uint8_t *) data_values);
                        case RegisterTable::BENDER:
                        case RegisterTable::KEBA:
                            break;
                    }
                    break;
                case TFModbusTCPFunctionCode::ReadDiscreteInputs:
                    switch (table) {
                        case RegisterTable::WARP:
                            return this->getWarpDiscreteInputs(start_address, data_count, (uint8_t *) data_values);
                        case RegisterTable::BENDER:
                        case RegisterTable::KEBA:
                            break;
                    }
                    break;
                case TFModbusTCPFunctionCode::ReadHoldingRegisters:
                    switch (table) {
                        case RegisterTable::WARP:
                            return this->getWarpHoldingRegisters(start_address, data_count, (uint16_t *) data_values);
                        case RegisterTable::BENDER:
                            return this->getBenderHoldingRegisters(start_address, data_count, (uint16_t *) data_values);
                        case RegisterTable::KEBA:
                            return this->getKebaHoldingRegisters(start_address, data_count, (uint16_t *) data_values);
                    }
                    break;
                case TFModbusTCPFunctionCode::ReadInputRegisters:
                    switch (table) {
                        case RegisterTable::WARP:
                            return this->getWarpInputRegisters(start_address, data_count, (uint16_t *) data_values);
                        case RegisterTable::BENDER:
                        case RegisterTable::KEBA:
                            break;
                    }
                    break;
                case TFModbusTCPFunctionCode::WriteMultipleCoils:
                    switch (table) {
                        case RegisterTable::WARP:
                            return this->setWarpCoils(start_address, data_count, (uint8_t *) data_values);
                        case RegisterTable::BENDER:
                        case RegisterTable::KEBA:
                            break;
                    }
                    break;
                case TFModbusTCPFunctionCode::WriteMultipleRegisters:
                    switch (table) {
                        case RegisterTable::WARP:
                            return this->setWarpHoldingRegisters(start_address, data_count, (uint16_t *) data_values);
                        case RegisterTable::BENDER:
                            return this->setBenderHoldingRegisters(start_address, data_count, (uint16_t *) data_values);
                        case RegisterTable::KEBA:
                            return this->setKebaHoldingRegisters(start_address, data_count, (uint16_t *) data_values);
                    }
                    break;
                case TFModbusTCPFunctionCode::WriteSingleCoil:
                case TFModbusTCPFunctionCode::WriteSingleRegister:
                    esp_system_abort("WriteSingleCoils/Registers should not be passed to receive_cb!");
                    break;
            }

            return TFModbusTCPExceptionCode::IllegalFunction;
        }
    );

    this->tick_task = task_scheduler.scheduleWithFixedDelay([this](){
        server.tick();
    }, 10_ms);
}

void ModbusTcp::stop_server() {
    // We are running in the main task -> cancel can't return WillBeCancelled
    task_scheduler.cancel(this->tick_task);

    server.stop();
    cache = nullptr;
}

void ModbusTcp::fillCache() {
    if (cache == nullptr)
        return;

    if (boot_stage < BootStage::REGISTER_EVENTS)
        return;

    cache->evse_state = api.getState("evse/state");
    cache->evse_slots = api.getState("evse/slots");
    cache->evse_ll_state = api.getState("evse/low_level_state");
    cache->evse_indicator_led = api.getState("evse/indicator_led");
    cache->evse_gp_output = api.getState("evse/gp_output", false); // Don't log if missing: Only WARP2 has this API.
    cache->evse_hardware_configuration = api.getState("evse/hardware_configuration");
    cache->current_charge = api.getState("charge_tracker/current_charge");
    cache->meter_state = api.getState("meter/state");
    cache->meter_values = api.getState("meter/values");
    cache->meter_phases = api.getState("meter/phases");
    cache->meter_all_values = api.getState("meter/all_values");
    cache->power_manager_state = api.getState("power_manager/state");
    cache->power_manager_external_control = api.getState("power_manager/external_control");
}

void ModbusTcp::register_events() {
    event.registerEvent("modbus_tcp/config", {}, [this](const Config *config) {
        this->stop_server();
        if (config->get("enable")->asBool())
            this->start_server();
        return EventResult::OK;
    });
}

void ModbusTcp::setup()
{
    api.restorePersistentConfig("modbus_tcp/config", &config);

    if (!config.get("enable")->asBool()) {
        initialized = true;
        return;
    }

    start_server();

    started = true;
    initialized = true;
}

void ModbusTcp::register_urls()
{
    api.addPersistentConfig("modbus_tcp/config", &config);
}

void ModbusTcp::pre_reboot()
{
    stop_server();
}
