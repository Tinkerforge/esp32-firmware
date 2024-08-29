/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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
#include <time.h>

#include "module_available.h"
#include "structs.h"

#include "module.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

class IEMBackend
{
    friend class EMCommon;

protected:
    IEMBackend() {}
    virtual ~IEMBackend() {}

    virtual bool is_initialized() const = 0;

    virtual uint32_t get_em_version() const = 0;

    virtual void set_time(const tm &tm) = 0;
    virtual timeval get_time() = 0;

    virtual bool get_sdcard_info(struct sdcard_info *data) = 0;
    virtual bool format_sdcard() = 0;

    virtual uint16_t get_energy_meter_detailed_values(float *ret_values) = 0;
    virtual bool reset_energy_meter_relative_energy() = 0;
};

class EMCommon final : public IModule
#if MODULE_AUTOMATION_AVAILABLE()
                       , public IAutomationBackend
#endif
{
    // It's a bit ugly that we have to declare all specific EM modules as friends here.
    // But this allows us to make the configs private, to enforce all access happens via the public methods below.
    friend class EnergyManager;

public:
    EMCommon();

    //void pre_setup() override;
    void setup() override;
    //void register_urls() override;

    uint32_t get_em_version();

    void set_time(const tm &tm);
    timeval get_time();

    bool get_sdcard_info(struct sdcard_info *data);
    bool format_sdcard();

    uint16_t get_energy_meter_detailed_values(float *ret_values);
    bool reset_energy_meter_relative_energy();

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

private:
    IEMBackend *backend = nullptr;
};
