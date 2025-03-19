/* esp32-firmware
 * Copyright (C) 2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include <functional>
#include <stdint.h>

#include "module.h"
#include "config.h"
#include "TFTools/Micros.h"
#include "bindings/hal_common.h"

class DeviceModuleBase : public IModule
{
public:
    DeviceModuleBase(const uint8_t *firmware,
                     const size_t firmware_len,
                     const char *url_prefix,
                     const char *device_name,
                     const char *module_name,
                     std::function<void(void)> &&setup_function,
                     bool mandatory) :
        firmware(firmware),
        firmware_len(firmware_len),
        url_prefix(url_prefix),
        device_name(device_name),
        module_name(module_name),
        setup_function(std::move(setup_function)),
        mandatory(mandatory) {}

    virtual ~DeviceModuleBase() {}

    void pre_setup() override;
    void register_urls() override;

protected:
    uint16_t get_device_id();
    void update_identity(TF_TFP *tfp);
    bool is_in_bootloader(int rc);
    bool setup_device();
    virtual void reset() = 0;

private:
    virtual int init(const char *, TF_HAL *hal) = 0;
    virtual int destroy() = 0;
    virtual int get_bootloader_mode(uint8_t *mode) = 0;

    ConfigRoot identity;

    const uint8_t *firmware;
    const size_t firmware_len;
    const char *url_prefix;
    const char *device_name;
    const char *module_name;
    std::function<void(void)> setup_function;
    bool mandatory;

    bool log_message_printed = false;

    bool task_started = false;

protected:
    bool device_found = false;
};

template <typename DeviceT,
          int (*init_function)(DeviceT *, const char *, TF_HAL *),
          int (*get_bootloader_mode_function)(DeviceT *, uint8_t *),
          int (*reset_function)(DeviceT *),
          int (*destroy_function)(DeviceT *),
          bool is_mandatory = true>
class DeviceModule : public DeviceModuleBase
{
public:
    DeviceModule(const uint8_t *firmware,
                 const size_t firmware_len,
                 const char *url_prefix,
                 const char *device_name,
                 const char *module_name,
                 std::function<void(void)> &&setup_function) :
        DeviceModuleBase(firmware, firmware_len, url_prefix, device_name, module_name, std::move(setup_function), is_mandatory) {}

    void reset() override
    {
        reset_function(&device);
    }

    int init(const char *id, TF_HAL *hal) override
    {
        return init_function(&device, id, hal);
    }

    int destroy() override
    {
        return destroy_function(&device);
    }

    int get_bootloader_mode(uint8_t *mode) override
    {
        return get_bootloader_mode_function(&device, mode);
    }

    // Think before making the device handle public again.
    // Instead of the usual python-esque approach of
    // "We don't care if stuff should be private/protected,
    // if you break stuff while accessing members, keep the pieces.",
    // the reason to make this handle private is to make sure
    // that only the module that owns the device calls bindings functions directly.
    // This simplifies reimplementing modules for other hardware.
protected:
    DeviceT device;
};
