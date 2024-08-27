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
#include "sprite_defines.h"
#include "font_defines.h"

#define UPDATE_INTERVAL 1000
#define PAGE_FRONT_TEXT_MAX_CHAR 6

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

void EMFrontPanel::update_wifi()
{
    int result = tf_warp_front_panel_set_display_wifi_setup_1(
        &device,
        wifi.get_ap_ip(),
        wifi.get_ap_ssid()
    );
    if (result != TF_E_OK) {
        logger.printfln("Failed to call set_display_wifi_setup_1: %d", result);
        return;
    }

    result = tf_warp_front_panel_set_display_wifi_setup_2(
        &device,
        wifi.get_ap_passphrase()
    );
    if (result != TF_E_OK) {
        logger.printfln("Failed to call set_display_wifi_setup_2: %d", result);
        return;
    }
}

void EMFrontPanel::update_status_bar()
{
    const EthernetState ethernet_state = ethernet.get_connection_state();
    const WifiState wifi_state         = wifi.get_connection_state();
    const int wifi_rssi                = wifi.get_sta_rssi();

    uint8_t hours   = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;

    struct timeval tv_now;
    if (clock_synced(&tv_now)) {
        time_t now = time(nullptr);
        struct tm tm;
        localtime_r(&now, &tm);
        hours   = tm.tm_hour;
        minutes = tm.tm_min;
        seconds = tm.tm_sec;
    }

    int result = tf_warp_front_panel_set_status_bar(
        &device,
        static_cast<std::underlying_type<EthernetState>::type>(ethernet_state),
        (wifi_rssi + 127) |  (static_cast<std::underlying_type<WifiState>::type>(wifi_state) << 16),
        hours,
        minutes,
        seconds
    );

    if (result != TF_E_OK) {
        logger.printfln("Failed to call set_status_bar: %d", result);
        return;
    }
}

int EMFrontPanel::set_display_front_page_icon_with_check(const uint32_t icon_index, bool active, const uint32_t sprite_index, const char *text_1, const uint8_t font_index_1, const char *text_2, const uint8_t font_index_2)
{
    // Always fill text with spaces, such that if a new string is
    // shorter than the previous one, the old characters are overwritten.
    char checked_text_1[PAGE_FRONT_TEXT_MAX_CHAR] = "     ";
    char checked_text_2[PAGE_FRONT_TEXT_MAX_CHAR] = "     ";
    strncpy(checked_text_1, text_1, PAGE_FRONT_TEXT_MAX_CHAR);
    strncpy(checked_text_2, text_2, PAGE_FRONT_TEXT_MAX_CHAR);

    return tf_warp_front_panel_set_display_front_page_icon(
        &device,
        icon_index,
        active,
        sprite_index,
        checked_text_1,
        font_index_1,
        checked_text_2,
        font_index_2
    );
}

int EMFrontPanel::update_front_page_empty_tile(const uint8_t index, const TileType type, const uint8_t param)
{
    return set_display_front_page_icon_with_check(
        index,
        false,
        0, // TODO: Make SPRITE_EMPTY?
        "",
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        "",
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int EMFrontPanel::update_front_page_wallbox(const uint8_t index, const TileType type, const uint8_t param)
{
    return set_display_front_page_icon_with_check(
        index,
        true,
        SPRITE_ICON_TYPE2,
        (String("Box ") + String(param)).c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        "2 kW", // TODO: Get value from load management
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int EMFrontPanel::update_front_page_charge_management(const uint8_t index, const TileType type, const uint8_t param)
{
    return set_display_front_page_icon_with_check(
        index,
        true,
        SPRITE_ICON_COG, // TODO: Make SPRITE_LOAD_MANAGEMENT
        "WB 3x", // TODO: Get value from load management
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        "42 kW", // TODO: Get value from load management
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int EMFrontPanel::update_front_page_meter(const uint8_t index, const TileType type, const uint8_t param)
{
    return set_display_front_page_icon_with_check(
        index,
        true,
        SPRITE_ICON_COG, // TODO: Make SPRITE_METER
        "Bezug", // TODO: Get value from load management
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        "10 kW", // TODO: Get value from load management
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int EMFrontPanel::update_front_page_day_ahead_prices(const uint8_t index, const TileType type, const uint8_t param)
{
    return set_display_front_page_icon_with_check(
        index,
        true,
        SPRITE_ICON_MONEY,
        "Preis",
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        "0,18 \x1F", // TODO: Get value from load management
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int EMFrontPanel::update_front_page_solar_forecast(const uint8_t index, const TileType type, const uint8_t param)
{
    return set_display_front_page_icon_with_check(
        index,
        true,
        SPRITE_ICON_SUN,
        "Morgen",
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        "135 kWh", // TODO: Get value from solar forecast
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int EMFrontPanel::update_front_page_energy_manager_status(const uint8_t index, const TileType type, const uint8_t param)
{
    return tf_warp_front_panel_set_display_front_page_icon(
        &device,
        index,
        true,
        SPRITE_ICON_COG, // TODO: WEM icon?
        "All OK", // TODO: Get status from WEM
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        "V2.3.1", // TODO: Get WEM firmware version
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}


void EMFrontPanel::update_front_page()
{
    for (FrontPanelTile &tile : tiles) {
        const uint8_t index = tile.index;
        const TileType type = TileType(tile.config.get("type")->asUint());
        const uint8_t param = tile.config.get("parameter")->asUint();
        int result = 0;
        switch (type) {
            case TileType::EmptyTile:           result = update_front_page_empty_tile(index, type, param);            break;
            case TileType::Wallbox:             result = update_front_page_wallbox(index, type, param);               break;
            case TileType::ChargeManagement:    result = update_front_page_charge_management(index, type, param);     break;
            case TileType::Meter:               result = update_front_page_meter(index, type, param);                 break;
            case TileType::DayAheadPrices:      result = update_front_page_day_ahead_prices(index, type, param);      break;
            case TileType::SolarForecast:       result = update_front_page_solar_forecast(index, type, param);        break;
            case TileType::EnergyManagerStatus: result = update_front_page_energy_manager_status(index, type, param); break;
            default:
                logger.printfln("Unknown tile type: %d", static_cast<std::underlying_type<TileType>::type>(type));
                break;
        }

        if (result != TF_E_OK) {
            logger.printfln("Failed to call set_display_front_page_icon: %d", result);
            return;
        }
    }
}

void EMFrontPanel::update()
{
    update_wifi();
    update_status_bar();
    update_front_page();
}
