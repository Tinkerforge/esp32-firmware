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

#include "meter_pv_faker.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modules/meters/meter_value_id.h"
#include "tools.h"

#include "gcc_warnings.h"

#define INDEX_FAKE_POWER_TOTAL 0
#define INDEX_FAKE_ILLUMINATION 1

static const MeterValueID pv_faker_value_ids[2] = {
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::PowerDC,
};

MeterClassID MeterPvFaker::get_class() const
{
    return MeterClassID::PvFaker;
}

void MeterPvFaker::setup(const Config &ephemeral_config)
{
    // Cache config
    peak_power  = ephemeral_config.get("peak_power")->asUint() * (-1LL);
    zero_at_lux = ephemeral_config.get("zero_at_lux")->asUint();
    peak_at_lux = ephemeral_config.get("peak_at_lux")->asUint();
    peak_at_lux_shifted = peak_at_lux - zero_at_lux;

    if (zero_at_lux >= peak_at_lux) {
        logger.printfln("Lux value for zero production must be less than lux value for peak production.");
        return;
    }

    const String &illuminance_topic = ephemeral_config.get("topic")->asString();
    const String &limiter_topic = ephemeral_config.get("limiter_topic")->asString();
    const String &global_topic_prefix = mqtt.config.get("global_topic_prefix")->asString();

    if (illuminance_topic.startsWith(global_topic_prefix)) {
        logger.printfln("Cannot listen to itself: Illuminance topic cannot start with the topic prefix from the MQTT config.");
        return;
    }

    if (illuminance_topic.length() < 3) {
        logger.printfln("Illuminance topic too short: %u", illuminance_topic.length());
        return;
    }

    mqtt.subscribe(illuminance_topic, [this](const char *_topic, size_t _topic_len, char *data, size_t data_len) {
        StaticJsonDocument<JSON_OBJECT_SIZE(10)> doc;
        DeserializationError error = deserializeJson(doc, data, data_len);
        if (error) {
            logger.printfln("Failed to deserialize MQTT payload: %s", error.c_str());
            return;
        }

        uint32_t illuminance = doc["illuminance"].as<uint32_t>();
        this->update_illuminance(illuminance);
    }, Mqtt::Retained::Accept);

    if (limiter_topic.isEmpty()) {
        limited_power = peak_power;
    } else {
        if (limiter_topic.startsWith(global_topic_prefix)) {
            logger.printfln("Cannot listen to itself: Limiter topic cannot start with the topic prefix from the MQTT config.");
            return;
        }

        if (limiter_topic.length() < 3) {
            logger.printfln("Limter topic too short: %u", limiter_topic.length());
            return;
        }

        limited_power = 0;

        mqtt.subscribe(limiter_topic, [this](const char *_topic, size_t _topic_len, char *data, size_t data_len) {
            StaticJsonDocument<JSON_OBJECT_SIZE(10)> doc;
            DeserializationError error = deserializeJson(doc, data, data_len);
            if (error) {
                logger.printfln("Failed to deserialize MQTT payload: %s", error.c_str());
                return;
            }

            uint32_t illuminance = doc["illuminance"].as<uint32_t>();
            this->update_limit(illuminance);
        }, Mqtt::Retained::Accept);
    }

    meters.declare_value_ids(slot, pv_faker_value_ids, ARRAY_SIZE(pv_faker_value_ids));

    task_scheduler.scheduleWithFixedDelay([this]() {
        meters.update_all_values(this->slot, this->values);
    }, 2_s, 2_s);
}

void MeterPvFaker::update_illuminance(uint32_t illuminance)
{
    if (illuminance < zero_at_lux) {
        illuminance = zero_at_lux;
    } else if (illuminance > peak_at_lux) {
        illuminance = peak_at_lux;
    }

    int64_t illuminance_shifted = illuminance - zero_at_lux;

    float fake_power_w = static_cast<float>(limited_power * illuminance_shifted / peak_at_lux_shifted);

    values[INDEX_FAKE_POWER_TOTAL]  = fake_power_w;
    values[INDEX_FAKE_ILLUMINATION] = static_cast<float>(illuminance);

    meters.update_all_values(slot, values);
}

void MeterPvFaker::update_limit(uint32_t illuminance)
{
    if (illuminance < zero_at_lux) {
        illuminance = zero_at_lux;
    } else if (illuminance > peak_at_lux) {
        illuminance = peak_at_lux;
    }

    limited_power = peak_power * illuminance / peak_at_lux;
}
