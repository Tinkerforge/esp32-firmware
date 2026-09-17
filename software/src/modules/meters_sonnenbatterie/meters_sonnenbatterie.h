/* esp32-firmware
 * Copyright (C) 2026 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "meter_sonnenbatterie.h"

#include <esp_task.h>
#include <stdint.h>

#include "config.h"
#include "modules/meters/imeter.h"
#include "modules/meters/imeter_generator.h"
#include "module.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

typedef struct esp_http_client *esp_http_client_handle_t;

class MetersSonnenbatterie final : public IModule, public IMeterGenerator
{
public:
    // for IModule
    void pre_setup() override;
    void register_events() override;

    // for MeterGenerator
    [[gnu::const]] MeterClassID get_class() const override;
    IMeter *new_meter(uint32_t slot, Config *state, Config *errors) override;
    [[gnu::const]] const Config *get_config_prototype() override;
    [[gnu::const]] const Config *get_state_prototype()  override;
    [[gnu::const]] const Config *get_errors_prototype() override;

    void apply_config(const Config *config);

private:
    static const size_t MAX_CHILD_METERS = 2;

    struct sonnen_runtime_data {
        sonnen_runtime_data(char *api_response_buffer_ , size_t trace_buffer_index_) : api_response_buffer(api_response_buffer_), trace_buffer_index(trace_buffer_index_) {}

        char *api_response_buffer = nullptr;
        size_t api_response_length = 0;

        size_t trace_buffer_index;

        MeterSonnenbatterie *child_meters[MAX_CHILD_METERS];
        Config *child_meter_states[MAX_CHILD_METERS];
        uint16_t child_meter_count = 0;

        uint16_t sonnenbatterie_port = 0;
        const char *sonnenbatterie_host = nullptr;

        StaticTask_t xTaskBuffer;
        StackType_t xStack[2816];
    };

    void set_child_meter_disconnected_state(bool disconnected);
    bool get_value_from_doc(const JsonDocument &doc, const char *field_name, float *output_value);
    void process_data();
    static esp_http_client_handle_t http_client_init(const sonnen_runtime_data *task_data);
    static void sonnen_task(void *arg);

    Config config_prototype;
    Config state_prototype;
    sonnen_runtime_data *runtime_data = nullptr;
    size_t trace_buffer_index;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
