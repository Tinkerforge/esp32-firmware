/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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

#include "em_front_panel.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "bindings/errors.h"
#include "tools.h"
#include "warp_front_panel_bricklet_firmware_bin.embedded.h"

#define UPDATE_INTERVAL 1000

extern EMFrontPanel em_front_panel;

EMFrontPanel::EMFrontPanel() : DeviceModule(warp_front_panel_bricklet_firmware_bin_data,
                                            warp_front_panel_bricklet_firmware_bin_length,
                                            "front_panel",
                                            "WARP Front Panel",
                                            "Front Panel",
                                            [this](){this->setup_bricklet();}) {}

void EMFrontPanel::pre_setup()
{
    this->DeviceModule::pre_setup();

    config = ConfigRoot{Config::Object({
            {"enable", Config::Bool(true)}
        }), [this](Config &cfg, ConfigSource source) -> String {
            return "";
        }
    };

    uint8_t index = 0;
    for (FrontPanelTile &tile : tiles) {
        tile.config = ConfigRoot{Config::Object({
            {"type", Config::Uint8(0)},
            {"parameter", Config::Uint8(0)},
        }), [this, index](Config &update, ConfigSource source) -> String {

            return "";
        }};

        tile.index = index++;
    }
}

void EMFrontPanel::setup_bricklet()
{
    if (!this->DeviceModule::setup_device()) {
        return;
    }

    initialized = true;
    api.addFeature("front_panel");
}

void EMFrontPanel::check_bricklet_state()
{
    uint32_t index = 0;
    int result = tf_warp_front_panel_get_display_page_index(&device, &index);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("Failed to call front panel function, rc: %d", result);
        }
        return;
    }
}

void EMFrontPanel::setup()
{
    setup_bricklet();
    if (!device_found)
        return;

    api.restorePersistentConfig("front_panel/config", &config);
    for (FrontPanelTile &tile : tiles) {
        api.restorePersistentConfig("front_panel/tiles/" + String(tile.index) + "/config", &tile.config);
    }

    task_scheduler.scheduleWithFixedDelay([this](){
        this->check_bricklet_state();
    }, 5 * 60 * 1000, 5 * 60 * 1000);
}

void EMFrontPanel::register_urls()
{
    api.addPersistentConfig("front_panel/config", &config);
    for (FrontPanelTile &tile : tiles) {
        api.addPersistentConfig("front_panel/tiles/" + String(tile.index) + "/config", &tile.config);
    }

    task_scheduler.scheduleWithFixedDelay([this]() {
        this->update();
    }, 100, UPDATE_INTERVAL);

    this->DeviceModule::register_urls();
}

void EMFrontPanel::loop()
{
    this->DeviceModule::loop();
}

void EMFrontPanel::update()
{
    int result = tf_warp_front_panel_set_display_wifi_setup_1(
        &device,
        wifi.get_ap_ip(),
        wifi.get_ap_ssid()
    );

    result = tf_warp_front_panel_set_display_wifi_setup_2(
        &device,
        wifi.get_ap_passphrase()
    );

    EthernetState ethernet_state = ethernet.get_connection_state();
    WifiState wifi_state = wifi.get_connection_state();
    int wifi_rssi = wifi.get_sta_rssi();
    time_t now = time(nullptr);
    struct tm tm;
    localtime_r(&now, &tm);
    result = tf_warp_front_panel_set_status_bar(
        &device,
        static_cast<std::underlying_type<EthernetState>::type>(ethernet_state), // Ethernet status
        (wifi_rssi + 127) |  (static_cast<std::underlying_type<WifiState>::type>(wifi_state) << 16), // Wifi status
        tm.tm_hour, // Hours
        tm.tm_min, // Minutes
        tm.tm_sec // Seconds
    );
}