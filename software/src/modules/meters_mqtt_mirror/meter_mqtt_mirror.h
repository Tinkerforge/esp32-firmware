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

#include "modules/meters/imeter.h"
#include "config.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MeterMqttMirror final : public IMeter
{
public:
    MeterMqttMirror(uint32_t slot_) : slot(slot_) {}

    [[gnu::const]] MeterClassID get_class() const override;
    void setup(const Config &ephemeral_config) override;

    bool supports_power()         override {return true;}
    bool supports_energy_import() override {return true;}
    bool supports_energy_export() override {return true;}
    bool supports_currents()      override {return true;}

private:
    void onMessage(const char *topic, size_t topic_len, char *data, size_t data_len, void (MeterMqttMirror::*message_handler)(const JsonArrayConst &json_array));
    void handle_mqtt_value_ids(const JsonArrayConst &array);
    void handle_mqtt_values(const JsonArrayConst &array);

    uint32_t slot;
    uint32_t declared_values_count = 0;
    bool     accepting_updates = false;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
