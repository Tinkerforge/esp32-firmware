/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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

#include "esp32_ethernet_v2_co_bricklet.h"

#include <ctime>

#include "event_log_prefix.h"
#include "bindings/errors.h"
#include "build.h"
#include "musl_libc_timegm.h"

#include "warp_esp32_ethernet_v2_co_bricklet_firmware_bin.embedded.h"

#include "module_dependencies.h"


#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

ESP32EthernetV2CoBricklet::ESP32EthernetV2CoBricklet(): DeviceModule(warp_esp32_ethernet_v2_co_bricklet_firmware_bin_data,
                                                                     warp_esp32_ethernet_v2_co_bricklet_firmware_bin_length,
                                                                     "co",
                                                                     "WARP ESP32 Ethernet V2 Co Bricklet",
                                                                     "Co",
                                                                     [this](){this->setup_co();}) {}

void ESP32EthernetV2CoBricklet::pre_setup()
{
    this->DeviceModule::pre_setup();
    this->state = Config::Object({
        {"temperature", Config::Int(0, -8192, 12800)}
    });
}

void ESP32EthernetV2CoBricklet::setup()
{
    setup_co();

    if (!device_found) {
        return;
    }

    rtc.register_backend(this);

    task_scheduler.scheduleWithFixedDelay([this](){
        int16_t temperature = 0;
        const int rc = tf_warp_esp32_ethernet_v2_co_get_temperature(&device, &temperature);
        if (rc != TF_E_OK) {
            logger.printfln("Failed to get temperature: error %i", rc);
        } else {
            state.get("temperature")->updateInt(temperature);
        }
    }, 1_s, 1_s);

    initialized = true;
}


void ESP32EthernetV2CoBricklet::register_urls()
{
    api.addState("esp32/temperature", &state);
}

void ESP32EthernetV2CoBricklet::set_time(const tm &date_time, int32_t microseconds)
{
    const int rc = tf_warp_esp32_ethernet_v2_co_set_date_time(
        &device,
        static_cast<uint8_t >(date_time.tm_sec),
        static_cast<uint8_t >(date_time.tm_min),
        static_cast<uint8_t >(date_time.tm_hour),
        static_cast<uint8_t >(date_time.tm_mday - 1),
        static_cast<uint8_t >(date_time.tm_wday),
        static_cast<uint8_t >(date_time.tm_mon),
        static_cast<uint16_t>(date_time.tm_year)
    );

    if (rc != TF_E_OK) {
        logger.printfln("Failed to set datetime: error %i", rc);
    }

    return;
}

struct timeval ESP32EthernetV2CoBricklet::get_time()
{
    struct tm date_time;
    struct timeval time;
    time.tv_usec = 0;

    uint8_t tm_sec;
    uint8_t tm_min;
    uint8_t tm_hour;
    uint8_t tm_mday;
    uint8_t tm_wday;
    uint8_t tm_mon;
    uint16_t tm_year;

    const int rc = tf_warp_esp32_ethernet_v2_co_get_date_time(&device, &tm_sec, &tm_min, &tm_hour, &tm_mday, &tm_wday, &tm_mon, &tm_year);

    if (rc != TF_E_OK) {
        logger.printfln("Failed to get datetime: error %i", rc);
        time.tv_sec = 0;
        return time;
    }

    date_time.tm_sec  = tm_sec;
    date_time.tm_min  = tm_min;
    date_time.tm_hour = tm_hour;
    date_time.tm_mday = tm_mday + 1;
    date_time.tm_wday = tm_wday;
    date_time.tm_mon  = tm_mon;
    date_time.tm_year = tm_year;

    time.tv_sec = timegm(&date_time);

    return time;
}

void ESP32EthernetV2CoBricklet::setup_co()
{
    if (!this->DeviceModule::setup_device()) {
        return;
    }

    tf_warp_esp32_ethernet_v2_co_set_response_expected(&device, TF_WARP_ESP32_ETHERNET_V2_CO_FUNCTION_SET_DATE_TIME, true);

    initialized = true;
}

void ESP32EthernetV2CoBricklet::reset()
{
    DeviceModule::reset();
}

void ESP32EthernetV2CoBricklet::set_blue_led(const bool on)
{
    if (!initialized) {
        return;
    }

    const int rc = tf_warp_esp32_ethernet_v2_co_set_led(&device, on ? TF_WARP_ESP32_ETHERNET_V2_CO_LED_STATE_ON : TF_WARP_ESP32_ETHERNET_V2_CO_LED_STATE_OFF);
    if (rc != TF_E_OK) {
        logger.printfln("Failed to set led: error %i", rc);
    }
}


#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif