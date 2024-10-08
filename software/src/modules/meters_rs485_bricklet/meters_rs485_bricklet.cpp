/* esp32-firmware
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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

#define EVENT_LOG_PREFIX "meters_rs485_brk"

#include "meters_rs485_bricklet.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "rs485_bricklet_firmware_bin.embedded.h"

//#include "gcc_warnings.h"

MetersRS485Bricklet::MetersRS485Bricklet() : DeviceModule(rs485_bricklet_firmware_bin_data,
                                                          rs485_bricklet_firmware_bin_length,
                                                          "rs485",
                                                          "RS485",
                                                          "Modbus Meter",
                                                          [this](){this->setupRS485();}) {}

void MetersRS485Bricklet::pre_setup()
{
    this->DeviceModule::pre_setup();

    config_prototype = Config::Object({
        {"display_name", Config::Str("WARP Charger", 0, 32)},
        {"type_override", Config::Uint8(METER_TYPE_AUTO_DETECT)}
    });

    state_prototype = Config::Object({
        {"type",  Config::Uint(0)}  // 0 - not available, 1 - sdm72, 2 - sdm630, 3 - sdm72v2, ... see meter.h
    });

    errors_prototype = Config::Object({
        {"meter", Config::Uint32(0)},
        {"bricklet", Config::Uint32(0)},
        {"bricklet_reset", Config::Uint32(0)},
    });

    sdm630_reset_prototype = Config::Object({
        {"energy_total", Config::Float(0)},
        {"energy_import", Config::Float(0)},
        {"energy_export", Config::Float(0)}
    });

    meters.register_meter_generator(get_class(), this);
}

#define MODBUS_SLAVE_ADDRESS 1
#define MODBUS_MASTER_REQUEST_TIMEOUT 1000

void MetersRS485Bricklet::setupRS485()
{
    if (!this->DeviceModule::setup_device()) {
        return;
    }

    int result = tf_rs485_set_rs485_configuration(&device, 9600, TF_RS485_PARITY_NONE, TF_RS485_STOPBITS_1, TF_RS485_WORDLENGTH_8, TF_RS485_DUPLEX_HALF);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("RS485 set config failed (rc %d). Disabling energy meter support.", result);
        }
        return;
    }

    result = tf_rs485_set_modbus_configuration(&device, MODBUS_SLAVE_ADDRESS, MODBUS_MASTER_REQUEST_TIMEOUT);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("RS485 set modbus config failed (rc %d). Disabling energy meter support.", result);
        }
        return;
    }

    // Set mode to MODBUS_MASTER_RTU last:
    // checkRS485State will re-run setupRS485 if the mode is not MODBUS_MASTER_RTU.
    // This happens if one of the configuration functions fails
    // or the bricklet resets.
    result = tf_rs485_set_mode(&device, TF_RS485_MODE_MODBUS_MASTER_RTU);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("RS485 set mode failed (rc %d). Disabling energy meter support.", result);
        }
        return;
    }

    initialized = true;

    if (meter_instance == nullptr)
        return;

    meter_instance->setupMeter();
}

void MetersRS485Bricklet::checkRS485State()
{
    uint8_t mode = 0;
    int result = tf_rs485_get_mode(&device, &mode);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("Failed to get RS485 mode, rc: %d", result);
            if (meter_instance != nullptr)
                meter_instance->errors->get("bricklet")->updateUint(meter_instance->errors->get("bricklet")->asUint() + 1);
        }
        return;
    }
    if (mode != TF_RS485_MODE_MODBUS_MASTER_RTU) {
        logger.printfln("RS485 mode invalid (%u). Did the bricklet reset?", mode);
        if (meter_instance != nullptr)
            meter_instance->errors->get("bricklet_reset")->updateUint(meter_instance->errors->get("bricklet_reset")->asUint() + 1);
        setupRS485();
    } else if (meter_instance != nullptr && meter_instance->meter_in_use == nullptr) {
        // Bricklet is fine, but reading the energy meter's type failed.
        meter_instance->setupMeter();
    }
}

void MetersRS485Bricklet::setup()
{
    setupRS485();
    if (!device_found)
        return;

    task_scheduler.scheduleWithFixedDelay([this](){
        this->checkRS485State();
    }, 10_s, 10_s);
}

void MetersRS485Bricklet::register_urls()
{
    this->DeviceModule::register_urls();
}

void MetersRS485Bricklet::loop()
{
    this->DeviceModule::loop();
}

[[gnu::const]]
MeterClassID MetersRS485Bricklet::get_class() const
{
    return MeterClassID::RS485Bricklet;
}

IMeter *MetersRS485Bricklet::new_meter(uint32_t slot, Config *state, Config *errors)
{
    if (meter_instance != nullptr) {
        logger.printfln("Cannot create more than one meter of class ModbusRTU.");
        return nullptr;
    }
    // FIXME: sdm630_reset_prototype should be copied?
    meter_instance = new MeterRS485Bricklet(slot, this, &device, state, errors, &sdm630_reset_prototype);
    return meter_instance;
}

[[gnu::const]]
const Config *MetersRS485Bricklet::get_config_prototype()
{
    return &config_prototype;
}

[[gnu::const]]
const Config *MetersRS485Bricklet::get_state_prototype()
{
    return &state_prototype;
}

[[gnu::const]]
const Config *MetersRS485Bricklet::get_errors_prototype()
{
    return &errors_prototype;
}
