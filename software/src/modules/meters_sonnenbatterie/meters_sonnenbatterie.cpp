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

#define EVENT_LOG_PREFIX "meters_sonnenbat"

#include <atomic>
#include <IPAddress.h> // FIXME: without this include here there is a problem with the IPADDR_NONE define in <lwip/ip4_addr.h>
#include <esp_http_client.h>
#include <ArduinoJson.hpp>

#include "generated/module_dependencies.h"
#include "meters_sonnenbatterie.h"

#include "event_log_prefix.h"
#include "modules/meters/generated/meter_location.enum.h"

#include "gcc_warnings.h"

static constexpr size_t API_RESPONSE_BUFFER_SIZE = 1024; // Usually about 672 needed

void MetersSonnenbatterie::pre_setup()
{
    meters.register_meter_generator(get_class(), this);

    trace_buffer_index = logger.alloc_trace_buffer("meters_sonnenbatterie");
}

void MetersSonnenbatterie::register_events()
{
    if (runtime_data == nullptr) {
        // No meter in use
        return;
    }

    // Delay starting the task until the network is marked as connected in the loop phase,
    // because the main loop won't process any packets before the loop phase.
    event.registerEvent("network/state", {"connected"}, [this](const Config *connected) {
        if (connected->asBool()) {
            if (this->runtime_data->sonnenbatterie_host == nullptr || strlen(this->runtime_data->sonnenbatterie_host) == 0) {
                logger.printfln("Sonnenbatterie host unset, cannot start");
                return EventResult::OK;
            }

            // Delay task creation a little to avoid the on-connect rush.
            task_scheduler.scheduleOnce([this]() {
                const TaskHandle_t task_handle = xTaskCreateStaticPinnedToCore(
                    sonnen_task,
                    "sonnen_task",
                    sizeof(this->runtime_data->xStack),
                    this->runtime_data,
                    uxTaskPriorityGet(nullptr) + 1, // sonnen_task should run with slightly higher priority so that the main task doesn't starve it.
                    this->runtime_data->xStack,
                    &this->runtime_data->xTaskBuffer,
                    1 // Run on same core as main application.
                );

#if MODULE_DEBUG_AVAILABLE()
                debug.register_task(task_handle, sizeof(this->runtime_data->xStack));
#else
                (void)task_handle;
#endif
            }, 1536_ms);

            return EventResult::Deregister;
        }

        return EventResult::OK;
    });
}

MeterClassID MetersSonnenbatterie::get_class() const
{
    return MeterClassID::Sonnenbatterie;
}

IMeter *MetersSonnenbatterie::new_meter(uint32_t slot, Config *state, Config * /*errors*/)
{
    if (runtime_data == nullptr) {
        void *data_ptr = heap_caps_malloc(sizeof(sonnen_runtime_data), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (data_ptr == nullptr) {
            logger.printfln("Failed to allocate runtime data");
            return nullptr;
        }

        void *api_buf = heap_caps_malloc_prefer(API_RESPONSE_BUFFER_SIZE, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (api_buf == nullptr) {
            free(data_ptr);
            logger.printfln("Failed to allocate API buffer");
            return nullptr;
        }

        runtime_data = new(data_ptr) sonnen_runtime_data{static_cast<char *>(api_buf), trace_buffer_index};
    }

    if (runtime_data->child_meter_count >= MAX_CHILD_METERS) {
        logger.printfln("Too many Sonnenbatterie meters. Only one Grid and one Battery allowed.");
        return nullptr;
    }

    MeterSonnenbatterie *meter = new(std::nothrow) MeterSonnenbatterie(slot);

    if (meter == nullptr) {
        logger.printfln("No memory to create new meter");
        return nullptr;
    }

    runtime_data->child_meters[runtime_data->child_meter_count] = meter;
    runtime_data->child_meter_states[runtime_data->child_meter_count] = state;

    runtime_data->child_meter_count++;

    return meter;
}

const Config *MetersSonnenbatterie::get_config_prototype()
{
    if (config_prototype.is_null()) {
        config_prototype = Config::Object({
            {"display_name",  Config::Str("", 0, 32)},
            {"location",      Config::Enum(MeterLocation::Unknown)},
            {"excluded",      Config::Bool(false)},
            {"host",          Config::Str("", 0, 64)},
            {"port",          Config::Uint16(80)},
            {"virtual_meter", Config::Uint8(0)},
        });
    }

    return &config_prototype;
}

const Config *MetersSonnenbatterie::get_state_prototype()
{
    if (state_prototype.is_null()) {
        state_prototype = Config::Object({
            {"disconnected", Config::Bool(false)},
        });
    }

    return &state_prototype;
}

const Config *MetersSonnenbatterie::get_errors_prototype()
{
    return Config::Null();
}

void MetersSonnenbatterie::apply_config(const Config *config)
{
    bool divergent_config = false;

    const String &new_host = config->get("host")->asString();
    const uint16_t new_port = config->get("port")->asUint16();

    if (runtime_data->sonnenbatterie_host == nullptr) {
        runtime_data->sonnenbatterie_host = perm_strdup(new_host.c_str());
    } else if (new_host != runtime_data->sonnenbatterie_host) {
        divergent_config = true;
    }

    if (runtime_data->sonnenbatterie_port == 0) {
        runtime_data->sonnenbatterie_port = new_port;
    } else if (new_port != runtime_data->sonnenbatterie_port) {
        divergent_config = true;
    }

    if (divergent_config) {
        logger.printfln("Configuration diverges between Sonnenbatterie meters; ignoring second Sonnenbatterie");
    }
}

void MetersSonnenbatterie::set_child_meter_disconnected_state(bool disconnected)
{
    task_scheduler.scheduleOnce([this, disconnected]() {
        for (uint16_t i = 0; i < runtime_data->child_meter_count; i++) {
            runtime_data->child_meter_states[i]->get("disconnected")->updateBool(disconnected);
        }
    });
}

bool MetersSonnenbatterie::get_value_from_doc(const JsonDocument &doc, const char *field_name, float *output_value)
{
    const auto &field = doc[field_name];
    if (field.isNull()) {
        logger.tracefln(trace_buffer_index, "Expected field '%s', not in API response.", field_name);
        return false;
    }
    if (!field.is<float>()) {
        logger.tracefln(trace_buffer_index, "Field '%s' in API response has unexpected type.", field_name);
        return false;
    }
    *output_value = field.as<float>();
    return true;
}

void MetersSonnenbatterie::process_data()
{
    struct MeterSonnenbatterie::sonnenbatterie_api_values api_values;
    DeserializationError::Code error_code;
    size_t missing_values = 0;

    do {
        StaticJsonDocument<JSON_OBJECT_SIZE(30)> doc;
        // Deserialize in zero-copy mode. Trashes response buffer.
        DeserializationError error = deserializeJson(doc, runtime_data->api_response_buffer, runtime_data->api_response_length);
        error_code = error.code();

        if (error_code != DeserializationError::Code::Ok) {
            if (error_code == DeserializationError::NoMemory) {
                logger.printfln("Failed to deserialize: JSON payload too long: %zu", runtime_data->api_response_length);
            } else {
                logger.printfln("Failed to deserialize: %s", error.c_str());
            }
            break;
        }

        //"Apparent_output":240
        //"BackupBuffer":"0"
        //"BatteryCharging":false
        //"BatteryDischarging":false
        //"Consumption_Avg":2710
        //"Consumption_W":2776
        if (!get_value_from_doc(doc, "Fac",             &api_values.f_ac            )) missing_values++;
        //"FlowConsumptionBattery":false
        //"FlowConsumptionGrid":false
        //"FlowConsumptionProduction":true
        //"FlowGridBattery":false
        //"FlowProductionBattery":false
        //"FlowProductionGrid":true
        if (!get_value_from_doc(doc, "GridFeedIn_W",    &api_values.grid_feed_w     )) missing_values++;
        //"IsSystemInstalled":1
        //"OperatingMode":"2"
        if (!get_value_from_doc(doc, "Pac_total_W",     &api_values.battery_out_w   )) missing_values++;
        //"Production_W":5699
        //"RSOC":100
        //"RemainingCapacity_Wh":10285
        //"Sac1":79
        //"Sac2":79
        //"Sac3":82
        //"SystemStatus":"OnGrid"
        //"Timestamp":"2023-09-10 12:52:15"
        if (!get_value_from_doc(doc, "USOC",            &api_values.soc             )) missing_values++;
        if (!get_value_from_doc(doc, "Uac",             &api_values.u_ac            )) missing_values++;
        if (!get_value_from_doc(doc, "Ubat",            &api_values.u_bat           )) missing_values++;
        //"dischargeNotAllowed":false
        //"generator_autostart":false
    } while (0);

    std::atomic_thread_fence(std::memory_order_release);
    runtime_data->api_response_length = 0;

    if (error_code == DeserializationError::Code::Ok && missing_values == 0) {
        for (size_t i = 0; i < runtime_data->child_meter_count; i++) {
            runtime_data->child_meters[i]->push_data(api_values);
        }
    } else {
        logger.printfln("Cannot process data; missing values: %zu", missing_values);
    }
}

[[gnu::noinline]]
esp_http_client_handle_t MetersSonnenbatterie::http_client_init(const sonnen_runtime_data *runtime_data)
{
    esp_http_client_config_t http_config;
    memset(&http_config, 0, sizeof(http_config));

    http_config.host = runtime_data->sonnenbatterie_host;
    http_config.port = runtime_data->sonnenbatterie_port;
    http_config.path = "/api/v2/status";
    http_config.transport_type = HTTP_TRANSPORT_OVER_TCP;

    esp_http_client_handle_t http_client = esp_http_client_init(&http_config);

    esp_http_client_delete_header(http_client, "User-Agent");

    return http_client;
}

[[gnu::noinline]]
static bool is_during_reboot_window()
{
    const time_t now = time(nullptr);
    struct tm tm;
    localtime_r(&now, &tm);
    const int hour = tm.tm_hour;

    return 2 <= hour && hour < 5;
}

[[gnu::noreturn]]
void MetersSonnenbatterie::sonnen_task(void *arg)
{
    sonnen_runtime_data *runtime_data = reinterpret_cast<sonnen_runtime_data *>(arg);
    esp_http_client_handle_t http_client = http_client_init(runtime_data);
    int last_status_code = 0;
    bool connection_error_printed = false;

    bool needs_long_delay = false;
    micros_t next_query_timestamp = now_us();

    for (;;) {
        bool needs_flush = false;

        do {
            std::atomic_thread_fence(std::memory_order_acquire);
            if (runtime_data->api_response_length > 0) {
                logger.tracefln(runtime_data->trace_buffer_index, "Previous response not processed yet");
                break;
            }

            int retries = 1;
retry_open: // label for evil goto

            int err = esp_http_client_open(http_client, 0);
            if (err != ESP_OK) {
                if (err == ESP_ERR_HTTP_CONNECT) {
                    if (!connection_error_printed) {
                        // Mark expected Sonnenbatterie reboots between 02:00 and 04:59.
                        if (is_during_reboot_window()) {
                            logger.printfln("Sonnenbatterie unreachable, might be nightly reboot");
                        } else {
                            logger.printfln("Sonnenbatterie unreachable");
                        }
                        connection_error_printed = true;
                        meters_sonnenbatterie.set_child_meter_disconnected_state(true);
                    }
                } else {
                    logger.printfln("Open failed: %s", esp_err_to_name(err));
                }
                break;
            }

            uint32_t eagain_count = 0;

retry_fetch_headers:
            const int content_length_s = static_cast<int>(esp_http_client_fetch_headers(http_client));
            const int status_code = esp_http_client_get_status_code(http_client);

            if (content_length_s <= 0) {
                if (content_length_s == -ESP_ERR_HTTP_EAGAIN) {
                    eagain_count++;

                    if (eagain_count != 1) {
                        logger.tracefln(runtime_data->trace_buffer_index, "EAGAIN %lu, status_code %i", eagain_count, status_code);
                    }

                    vTaskDelay_ms(100);
                    goto retry_fetch_headers;
                }

                int close_err = esp_http_client_close(http_client);
                if (close_err != ESP_OK) {
                    logger.printfln("Error during close: %i", close_err);
                }

                if (status_code == -1 && --retries >= 0) {
                    goto retry_open; // using evil goto
                }

                logger.printfln("Request failed: Content length: -0x%04x  Status code: %i  Connection closed", static_cast<unsigned>(-content_length_s), status_code);
                break;
            }

            const size_t content_length = static_cast<size_t>(content_length_s);

            needs_long_delay = false;

            if (status_code != 200) {
                if (status_code != last_status_code) {
                    bool log_error = true;

                    // Don't log status codes 500 and 502 while the Sonnenbatterie is restarting between 02:00 and 04:59.
                    if ((status_code == 500 || status_code == 502) && is_during_reboot_window()) {
                        log_error = false;
                    }

                    bool print_simple = true;
                    int read_bytes = 0;

                    if (content_length < API_RESPONSE_BUFFER_SIZE) {
                        read_bytes = esp_http_client_read_response(http_client, runtime_data->api_response_buffer, API_RESPONSE_BUFFER_SIZE - 1);
                    }

                    if (0 < read_bytes && read_bytes <= 100) {
                        if (status_code == 500 && read_bytes == 19 && strncmp(runtime_data->api_response_buffer, "{\"error\":\"no data\"}", 19) == 0) {
                            if (log_error) {
                                logger.printfln("Sonnenbatterie has no data");
                            } else {
                                logger.tracefln(runtime_data->trace_buffer_index, "Sonnenbatterie has no data");
                            }
                        } else if (read_bytes > 2 && strncmp(runtime_data->api_response_buffer, "{\"error\":\"", 10) == 0) {
                            if (log_error) {
                                logger.printfln("Request returned error: %i %.*s", status_code, read_bytes - 12, runtime_data->api_response_buffer + 10);
                            } else {
                                logger.tracefln(runtime_data->trace_buffer_index, "Request returned error: %i %.*s", status_code, read_bytes - 12, runtime_data->api_response_buffer + 10);
                            }
                        } else {
                            if (log_error) {
                                logger.printfln("Request returned status code %i: %.*s", status_code, read_bytes, runtime_data->api_response_buffer);
                            } else {
                                logger.tracefln(runtime_data->trace_buffer_index, "Request returned status code %i: %.*s", status_code, read_bytes, runtime_data->api_response_buffer);
                            }
                        }
                        print_simple = false;
                    }

                    if (print_simple) {
                        if (log_error) {
                            logger.printfln("Request returned status code %i. Content length: %zu", status_code, content_length);
                        } else {
                            logger.tracefln(runtime_data->trace_buffer_index, "Request returned status code %i. Content length: %zu", status_code, content_length);
                        }

                        if (read_bytes > 0) {
                            const size_t read_bytes_u = static_cast<size_t>(read_bytes);

                            if (read_bytes_u < API_RESPONSE_BUFFER_SIZE) {
                                runtime_data->api_response_buffer[read_bytes_u] = '\n';
                                if (log_error) {
                                    logger.print_plain(runtime_data->api_response_buffer, read_bytes_u + 1);
                                } else {
                                    logger.trace_plain(runtime_data->trace_buffer_index, runtime_data->api_response_buffer, read_bytes_u + 1);
                                }
                            }
                        }
                    }

                    last_status_code = status_code;
                }

                needs_flush = true;

                if (status_code == 502) {
                    needs_long_delay = true;
                }

                break;
            }

            bool log_data_receive = false;

            if (last_status_code != 200) {
                if (last_status_code >= 400) {
                    log_data_receive = true;
                }
                last_status_code = status_code;
            }

            if (content_length >= API_RESPONSE_BUFFER_SIZE) {
                logger.printfln("Content length %zu too large", content_length);
                needs_flush = true;
                break;
            }

            int read_bytes = esp_http_client_read_response(http_client, runtime_data->api_response_buffer, API_RESPONSE_BUFFER_SIZE);

            if (read_bytes != content_length_s) {
                logger.printfln("Read didn't get all data: %i/%i", read_bytes, content_length_s);
                if (read_bytes > 0) {
                    if (static_cast<unsigned>(read_bytes) < API_RESPONSE_BUFFER_SIZE) {
                        runtime_data->api_response_buffer[read_bytes] = '\n';
                        read_bytes++;
                    } else {
                        runtime_data->api_response_buffer[API_RESPONSE_BUFFER_SIZE - 1] = '\n';
                    }
                    logger.print_plain(runtime_data->api_response_buffer, static_cast<size_t>(read_bytes));
                }
                needs_flush = true;
                break;
            }

            if (connection_error_printed) {
                logger.printfln("Connected to Sonnenbatterie");
                connection_error_printed = false;
                meters_sonnenbatterie.set_child_meter_disconnected_state(false);
            } else {
                if (log_data_receive) {
                    logger.printfln("Received data from Sonnenbatterie");
                }
            }

            runtime_data->api_response_length = content_length;
            std::atomic_thread_fence(std::memory_order_release);

            task_scheduler.scheduleOnce([]() {
                meters_sonnenbatterie.process_data();
            });
        } while (0);

        if (needs_flush) {
            int flush_len = -1;
            int flush_ret = esp_http_client_flush_response(http_client, &flush_len);

            int read_len = 0;
            int reads = 0;
            for (;;) {
                int data_read = esp_http_client_read(http_client, runtime_data->api_response_buffer, API_RESPONSE_BUFFER_SIZE);
                if (data_read <= 0) {
                    break;
                }
                read_len += data_read;
                reads++;
            }

            logger.tracefln(runtime_data->trace_buffer_index, "Flushed %i, returned %i. Discarded %i with %i call(s).", flush_len, flush_ret, read_len, reads);
        }

        if (needs_long_delay) {
            next_query_timestamp += 10_s;
        } else {
            next_query_timestamp += 500_ms;
        }
        const micros_t now = now_us();
        // Casting the time difference microseconds to int32_t is safe because int32_t can hold +- 35 minutes.
        const int32_t delay_needed_us = static_cast<int32_t>(static_cast<int64_t>(next_query_timestamp - now));

        TickType_t delay_ticks;
        if (delay_needed_us < 250000) { // 250ms
            // Last request took too long, already too close or past next planned time.
            next_query_timestamp = now + 250_ms;
            delay_ticks = 250U / (1000 / configTICK_RATE_HZ); // 250ms
        } else {
            // delay_needed_us is safe to cast to unsigned here because it can't be negative.
            delay_ticks = static_cast<uint32_t>(delay_needed_us) / 1000 / (1000 / configTICK_RATE_HZ);
        }
        vTaskDelay(delay_ticks);
    }
}
