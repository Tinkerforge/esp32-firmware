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

#include "meters.h"
#include "meter_class_none.h"

#include "api.h"
#include "event_log.h"
#include "tools.h"

#include "gcc_warnings.h"

static MeterGeneratorNone meter_generator_none;

void Meters::pre_setup()
{
    generators.reserve(METER_CLASSES);
    register_meter_generator(METER_CLASS_NONE, &meter_generator_none);
}

void Meters::setup()
{
    generators.shrink_to_fit();

    // Create config prototypes, depending on available generators.
    uint8_t class_count = static_cast<uint8_t>(generators.size());
    ConfUnionPrototype *config_prototypes = new ConfUnionPrototype[class_count];

    for (uint32_t i = 0; i < class_count; i++) {
        const auto &generator_tuple = generators[i];
        uint8_t meter_class = static_cast<uint8_t>(std::get<0>(generator_tuple));
        auto meter_generator = std::get<1>(generator_tuple);
        config_prototypes[i] = {meter_class, *meter_generator->get_config_prototype()};
    }

    for (uint32_t i = 0; i < METER_SLOTS; i++) {
        // Initialize config.
        config_unions[i] = Config::Union(
            *get_generator_for_class(METER_CLASS_NONE)->get_config_prototype(),
            METER_CLASS_NONE,
            config_prototypes,
            class_count
        );

        // Load config.
        char path_buf[32];
        snprintf(path_buf, ARRAY_SIZE(path_buf), "meters/%u/config", i);
        api.restorePersistentConfig(path_buf, &config_unions[i]);

        uint32_t configured_meter_class = config_unions[i].getTag();

        // Generator might be a NONE class generator if the requested class is not available.
        MeterGenerator *generator = get_generator_for_class(configured_meter_class);

        // Initialize state to match (loaded) config.
        states[i] = *generator->get_state_prototype();

        // Create meter from config.
        const Config *meter_conf = static_cast<const Config *>(config_unions[i].get());
        Config *meter_state = &states[i];

        IMeter *meter = new_meter_of_class(configured_meter_class, meter_state, meter_conf);
        if (!meter) {
            logger.printfln("meters: Failed to create meter of class %u.", configured_meter_class);
            continue;
        }
        meter->setup();
        meters[i] = meter;
    }

    initialized = true;
}

void Meters::register_urls()
{
    char path_buf[32];

    for (uint32_t i = 0; i < ARRAY_SIZE(states); i++) {
        snprintf(path_buf, ARRAY_SIZE(path_buf), "meters/%u/state", i);
        api.addState(path_buf, &states[i], {}, 1000);

        snprintf(path_buf, ARRAY_SIZE(path_buf), "meters/%u/config", i);
        api.addPersistentConfig(path_buf, &config_unions[i], {}, 1000);

        if (meters[i]) {
            snprintf(path_buf, ARRAY_SIZE(path_buf), "meters/%u", i);
            meters[i]->register_urls(path_buf);
        }
    }
}

void Meters::register_meter_generator(uint32_t meter_class, MeterGenerator *generator)
{
    for (const auto &generator_tuple : generators) {
        uint32_t known_class = std::get<0>(generator_tuple);
        if (meter_class == known_class) {
            logger.printfln("meters: Tried to register meter generator for already registered meter class %u.", meter_class);
            return;
        }
    }

    generators.push_back({meter_class, generator});
}

MeterGenerator *Meters::get_generator_for_class(uint32_t meter_class)
{
    for (auto generator_tuple : generators) {
        uint32_t known_class = std::get<0>(generator_tuple);
        if (meter_class == known_class) {
            return std::get<1>(generator_tuple);
        }
    }

    if (meter_class == METER_CLASS_NONE) {
        logger.printfln("meters: No generator for dummy meter available. This is probably fatal.");
        return nullptr;
    }

    logger.printfln("meters: No generator for meter class %u.", meter_class);
    return get_generator_for_class(METER_CLASS_NONE);
}

IMeter *Meters::new_meter_of_class(uint32_t meter_class, Config *state, const Config *config)
{
    MeterGenerator *generator = get_generator_for_class(meter_class);

    if (!generator)
        return nullptr;

    return generator->new_meter(state, config);
}

IMeter *Meters::get_meter(uint32_t slot)
{
    if (slot >= METER_SLOTS)
        return nullptr;

    return meters[slot];
}

uint32_t Meters::get_meters(uint32_t meter_class, IMeter **found_meters, uint32_t found_meters_capacity)
{
    uint32_t found_count = 0;
    for (uint32_t i = 0; i < ARRAY_SIZE(meters); i++) {
        if (meters[i]->get_class() == meter_class) {
            if (found_count < found_meters_capacity) {
                found_meters[found_count] = meters[i];
            }
            found_count++;
        }
    }
    return found_count;
}
