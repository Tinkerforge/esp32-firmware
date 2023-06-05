/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "config.h"
#include "module.h"

#define PV_FILTER_PERIOD_MS 250

class EmPvFaker final : public IModule
{
public:
    EmPvFaker(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void onMqttConnect();
    bool onMqttMessage(char *topic, size_t topic_len, char *data, size_t data_len, bool retain);

    ConfigRoot state;

private:
    ConfigRoot config;
    ConfigRoot runtime_config;
    ConfigRoot runtime_config_update;
};
