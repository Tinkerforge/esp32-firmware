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

#include "meters_mqtt_mirror.h"
#include "module_dependencies.h"

#include "gcc_warnings.h"

void MetersMqttMirror::pre_setup()
{
    config_prototype = ConfigRoot(Config::Object({
        {"display_name", Config::Str("Lokal", 0, 32)},
        {"auto",         Config::Bool(true)},
        {"meter_path",   Config::Str("", 3, 64)},
        {"value_ids",    Config::Str("", 0, 64)},
    }), [](Config &cfg, ConfigSource /*source*/) -> String {
        const String &global_topic_prefix = mqtt.config.get("global_topic_prefix")->asString();
        const String &meter_path = cfg.get("meter_path")->asString();

        if (meter_path.startsWith(global_topic_prefix))
            return "Cannot listen to itself: Source meter path cannot start with the topic prefix from the MQTT config.";

        return "";
    });

    meters.register_meter_generator(get_class(), this);
}

MeterClassID MetersMqttMirror::get_class() const
{
    return MeterClassID::MqttMirror;
}

IMeter *MetersMqttMirror::new_meter(uint32_t slot, Config * /*state*/, Config * /*errors*/)
{
    return new MeterMqttMirror(slot);
}

const Config *MetersMqttMirror::get_config_prototype()
{
    return &config_prototype;
}

const Config *MetersMqttMirror::get_state_prototype()
{
    return Config::Null();
}

const Config *MetersMqttMirror::get_errors_prototype()
{
    return Config::Null();
}
