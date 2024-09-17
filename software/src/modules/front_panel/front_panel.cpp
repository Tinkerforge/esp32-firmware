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

#include "front_panel.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "bindings/errors.h"
#include "tools.h"
#include "warp_front_panel_bricklet_firmware_bin.embedded.h"
#include "sprite_defines.h"
#include "font_defines.h"
#include "semantic_version.h"
#include "modules/charge_manager/charge_manager_private.h"
#include "metadata.h"

#define UPDATE_INTERVAL 1000
#define PAGE_FRONT_TEXT_MAX_CHAR 6
#define TILE_TYPES 8


#if MODULE_CM_NETWORKING_AVAILABLE()
#define FRONT_PANEL_CONTROLLED_CHARGES (MAX_CONTROLLED_CHARGERS - 1)
#else
#define FRONT_PANEL_CONTROLLED_CHARGES 0
#endif

#if MODULE_METERS_AVAILABLE()
#define FRONT_PANEL_METERS_SLOTS (METERS_SLOTS - 1)
#else
#define FRONT_PANEL_METERS_SLOTS 0
#endif

FrontPanel::FrontPanel() : DeviceModule(warp_front_panel_bricklet_firmware_bin_data,
                                        warp_front_panel_bricklet_firmware_bin_length,
                                        "front_panel",
                                        "WARP Front Panel",
                                        "Front Panel",
                                        [this](){this->setup_bricklet();}) {}

void FrontPanel::pre_setup()
{
    this->DeviceModule::pre_setup();

    ConfUnionPrototype<TileType> *tile_prototypes = new ConfUnionPrototype<TileType>[TILE_TYPES]{
        {TileType::EmptyTile,           *Config::Null()},
        {TileType::Wallbox,             Config::Uint(0, 0, FRONT_PANEL_CONTROLLED_CHARGES)},
        {TileType::ChargeManagement,    *Config::Null()},
        {TileType::Meter,               Config::Uint(0, 0, FRONT_PANEL_METERS_SLOTS)},
        {TileType::DayAheadPrices,      Config::Enum(DAPType::CurrentPrice, DAPType::CurrentPrice, DAPType::AveragePriveTomorrow)},
        {TileType::SolarForecast,       Config::Enum(SFType::ForecastToday, SFType::ForecastToday, SFType::ForecastTomorrow)},
        {TileType::EnergyManagerStatus, *Config::Null()},
        {TileType::HeatingStatus,       *Config::Null()},
    };

    Config *tile_union = new Config{Config::Union<TileType>(
        *Config::Null(),
        TileType::EmptyTile,
        tile_prototypes,
        TILE_TYPES
    )};

    config = ConfigRoot{Config::Object({
            {"enable", Config::Bool(true)},
            {"tiles", Config::Array({
                    *tile_union,
                    *tile_union,
                    *tile_union,
                    *tile_union,
                    *tile_union,
                    *tile_union
                }, tile_union, FRONT_PANEL_TILES, FRONT_PANEL_TILES, Config::type_id<Config::ConfUnion>())}
        }), [this](Config &cfg, ConfigSource source) -> String {
            // Schedule check_bricklet_state() here, since this checks
            // if the display needs to be turned on/off.
            task_scheduler.scheduleOnce([this](){
                this->check_bricklet_state();
            }, 0);

            return "";
        }
    };

    config.get("tiles")->get(0)->changeUnionVariant(TileType::Wallbox);
    config.get("tiles")->get(1)->changeUnionVariant(TileType::DayAheadPrices);
    config.get("tiles")->get(2)->changeUnionVariant(TileType::HeatingStatus);
    config.get("tiles")->get(3)->changeUnionVariant(TileType::Meter);
    config.get("tiles")->get(4)->changeUnionVariant(TileType::SolarForecast);
    config.get("tiles")->get(5)->changeUnionVariant(TileType::EnergyManagerStatus);
}

void FrontPanel::setup_bricklet()
{
    if (!this->DeviceModule::setup_device()) {
        return;
    }

    initialized = true;
    api.addFeature("front_panel");
}

void FrontPanel::check_bricklet_state()
{
    if (!initialized) {
        return;
    }

    const bool enable = config.get("enable")->asBool();
    uint8_t display = 0;
    int result = tf_warp_front_panel_get_display(&device, &display, nullptr);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("Failed to call get_display: %d", result);
        }
        return;
    }

    if ((display == TF_WARP_FRONT_PANEL_DISPLAY_OFF) && enable) {
        result = tf_warp_front_panel_set_display(&device, TF_WARP_FRONT_PANEL_DISPLAY_AUTOMATIC);
        if (result != TF_E_OK) {
            logger.printfln("Failed to call set_display(%d): %d", TF_WARP_FRONT_PANEL_DISPLAY_AUTOMATIC, result);
        }
    } else if((display == TF_WARP_FRONT_PANEL_DISPLAY_AUTOMATIC) && !enable) {
        result = tf_warp_front_panel_set_display(&device, TF_WARP_FRONT_PANEL_DISPLAY_OFF);
        if (result != TF_E_OK) {
            logger.printfln("Failed to call set_display(%d): %d", TF_WARP_FRONT_PANEL_DISPLAY_OFF, result);
        }
    }
}

void FrontPanel::setup()
{
    setup_bricklet();
    if (!device_found) {
        return;
    }

    api.restorePersistentConfig("front_panel/config", &config);

    task_scheduler.scheduleWithFixedDelay([this](){
        this->check_bricklet_state();
    }, 5 * 60 * 1000, 5 * 60 * 1000);

    task_scheduler.scheduleOnce([this](){
        this->check_flash_metadata();
    }, 15*1000); // We assume that the Bricklet has booted and read the metadata after 15s
}

void FrontPanel::register_urls()
{
    api.addPersistentConfig("front_panel/config", &config);

    task_scheduler.scheduleWithFixedDelay([this]() {
        this->update();
    }, 100, UPDATE_INTERVAL);

    this->DeviceModule::register_urls();
}

void FrontPanel::loop()
{
    this->DeviceModule::loop();
}

int FrontPanel::set_led(const LEDPattern pattern, const LEDColor color)
{
    int result = tf_warp_front_panel_set_led_state(
        &device,
        static_cast<std::underlying_type<LEDPattern>::type>(pattern),
        static_cast<std::underlying_type<LEDColor>::type>(color)
    );

    if (result != TF_E_OK) {
        logger.printfln("Failed to call set_led_state: %d", result);
    }

    return result;
}

int FrontPanel::get_led(LEDPattern *pattern, LEDColor *color)
{
    uint8_t pattern_raw = 0;
    uint8_t color_raw   = 0;
    int result = tf_warp_front_panel_get_led_state(
        &device,
        &pattern_raw,
        &color_raw
    );

    if (result == TF_E_OK) {
        *pattern = static_cast<LEDPattern>(pattern_raw);
        *color   = static_cast<LEDColor>(color_raw);
    } else {
        logger.printfln("Failed to call set_led_state: %d", result);
    }

    return result;
}

void FrontPanel::update_wifi()
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

void FrontPanel::update_status_bar()
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

int FrontPanel::set_display_front_page_icon_with_check(const uint32_t icon_index, bool active, const uint32_t sprite_index, const char *text_1, const uint8_t font_index_1, const char *text_2, const uint8_t font_index_2)
{
    // Always fill text with spaces, such that if a new string is
    // shorter than the previous one, the old characters are overwritten.
    char checked_text_1[PAGE_FRONT_TEXT_MAX_CHAR + 1] = "      ";
    char checked_text_2[PAGE_FRONT_TEXT_MAX_CHAR + 1] = "      ";
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

int FrontPanel::update_front_page_empty_tile(const uint8_t index, const TileType type, const uint8_t param)
{
    return set_display_front_page_icon_with_check(
        index,
        false,
        SPRITE_ICON_EMPTY,
        "",
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        "",
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int FrontPanel::update_front_page_wallbox(const uint8_t index, const TileType type, const uint8_t param)
{
    String str1 = "Box " + String(param);
    String str2 = "-- kW";
#if MODULE_CHARGE_MANAGER_AVAILABLE()
    size_t charger_count = charge_manager.charger_count;
    if (charger_count > 0) {
        auto &charger = charge_manager.charger_state[param];
        const int32_t watt = charger.power_total_sum/charger.power_total_count;
        str2 = watt_value_to_display_string(watt);
    }
#endif

    return set_display_front_page_icon_with_check(
        index,
        true,
        SPRITE_ICON_TYPE2,
        str1.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        str2.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int FrontPanel::update_front_page_charge_management(const uint8_t index, const TileType type, const uint8_t param)
{
    String str1 = "WB 0x";
    String str2 = "-- kW";
#if MODULE_CHARGE_MANAGER_AVAILABLE()
    size_t charger_count = charge_manager.charger_count;
    if (charger_count > 0) {
        str1 = "WB " + String(charger_count) + "x";

        // ma*0.23 = W
        const int32_t watt = charge_manager.get_allocated_currents()->pv*0.23;
        str2 = watt_value_to_display_string(watt);
    }
#endif

    return set_display_front_page_icon_with_check(
        index,
        true,
        SPRITE_ICON_CHARGE_MANAGEMENT,
        str1.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        str2.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int FrontPanel::update_front_page_meter(const uint8_t index, const TileType type, const uint8_t param)
{
    String str1 = "Bezug";
    String str2 = "-- kW";
    uint32_t icon_index = SPRITE_ICON_ENERGY;

#if MODULE_METERS_AVAILABLE()
    float watt = 0;
    MeterValueAvailability meter_availability = meters.get_power_real(param, &watt);
    if (meter_availability == MeterValueAvailability::Fresh) {
        if (watt > 0) {
            icon_index = SPRITE_ICON_ENERGY_IMPORT;
        } else if (watt == 0) {
            // Different str1 here if no energy is flowing?
        } else if (watt < 0) {
            icon_index = SPRITE_ICON_ENERGY_EXPORT;
            watt = -watt;
            str1 = "Einsp.";
        }

        str2 = watt_value_to_display_string(watt);
    }
#endif

    return set_display_front_page_icon_with_check(
        index,
        true,
        icon_index,
        str1.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        str2.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int FrontPanel::update_front_page_day_ahead_prices(const uint8_t index, const TileType type, const DAPType param)
{
    String str1 = "Preis";
    String str2 = "-- ct";

#if MODULE_DAY_AHEAD_PRICES_AVAILABLE()
    DataReturn<int32_t> price = {false, 0};
    switch (param) {
        case DAPType::CurrentPrice:
            str1 = "Preis";
            price = day_ahead_prices.get_current_price();
            break;
        case DAPType::AveragePriveToday:
            str1 = "Heute";
            price = day_ahead_prices.get_average_price_today();
            break;
        case DAPType::AveragePriveTomorrow:
            str1 = "Morgen";
            price = day_ahead_prices.get_average_price_tomorrow();
            break;
    }

    if (price.data_available) {
        str2 = price_value_to_display_string(price.data / 1000);
    }
#endif

    return set_display_front_page_icon_with_check(
        index,
        true,
        SPRITE_ICON_WALLET_EURO,
        str1.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        str2.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int FrontPanel::update_front_page_solar_forecast(const uint8_t index, const TileType type, const SFType param)
{
    String str1 = "------";
    String str2 = "-- kWh";
    uint32_t icon_index = SPRITE_ICON_SUN;

#if MODULE_SOLAR_FORECAST_AVAILABLE()
    DataReturn<uint32_t> kwh{};
    switch (param) {
        case SFType::ForecastToday:
            str1 = "Heute";
            kwh = solar_forecast.get_kwh_today();
            break;
        case SFType::ForecastTomorrow:
            str1 = "Morgen";
            kwh = solar_forecast.get_kwh_tomorrow();
            break;
    }

    if (kwh.data_available) {
        str2 = watt_hour_value_to_display_string(kwh.data);
        // If 5 kWh is a high or a low value of course depends on the size of the pv system.
        // On average, 5 kWh in a day sounds low enough to show some clouds.
        if (kwh.data <= 5) {
            icon_index = SPRITE_ICON_CLOUD_SUN;
        }
    }
#endif

    return set_display_front_page_icon_with_check(
        index,
        true,
        icon_index,
        str1.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        str2.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int FrontPanel::update_front_page_energy_manager_status(const uint8_t index, const TileType type, const uint8_t param)
{
    const SemanticVersion version;
    String str1 = "FW Ver";
    String str2 = String(version.major) + '.' + String(version.minor) + '.' + String(version.patch);

    // TODO: Show error instead if energy manager has an error?

    return tf_warp_front_panel_set_display_front_page_icon(
        &device,
        index,
        true,
        SPRITE_ICON_WRENCH,
        str1.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        str2.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

int FrontPanel::update_front_page_heating_status(const uint8_t index, const TileType type, const uint8_t param)
{
    const SemanticVersion version;
    String str1 = "SG Rdy";
    String str2 = "--";
    uint32_t icon_index = SPRITE_ICON_HEATING;

#if MODULE_HEATING_AVAILABLE()
    if (heating.is_active()) {
        // First check for SG Ready output 0 (§14EnWG blocked or not)
        // §14EnWG has priority if active. Afterwards check for heating overdrive
        if (heating.is_sg_ready_output0_closed()) {
            str1 = "14EnWG";
            str2 = "Block.";
            icon_index = SPRITE_ICON_HEATING_OFF;
        } else {
            const bool closed1 = heating.is_sg_ready_output1_closed();
            str2 = closed1 ? "Ein" : "Aus";
            icon_index = closed1 ? SPRITE_ICON_HEATING_HOT : SPRITE_ICON_HEATING;
        }
    }
#endif

    return tf_warp_front_panel_set_display_front_page_icon(
        &device,
        index,
        true,
        icon_index,
        str1.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK,
        str2.c_str(),
        FONT_24PX_FREEMONO_WHITE_ON_BLACK
    );
}

void FrontPanel::update_front_page()
{
    auto tiles = config.get("tiles");
    for (size_t i = 0; i < tiles->count(); ++i) {
        auto tile = tiles->get(i);
        TileType type = tile->getTag<TileType>();

        int result = TF_E_OK;

        switch (type) {
            case TileType::EmptyTile:
                result = update_front_page_empty_tile(i, type, 0);
                break;
            case TileType::Wallbox:
                result = update_front_page_wallbox(i, type, tile->get()->asUint());
                break;
            case TileType::ChargeManagement:
                result = update_front_page_charge_management(i, type, 0);
                break;
            case TileType::Meter:
                result = update_front_page_meter(i, type, tile->get()->asUint());
                break;
            case TileType::DayAheadPrices:
                result = update_front_page_day_ahead_prices(i, type, tile->get()->asEnum<DAPType>());
                break;
            case TileType::SolarForecast:
                result = update_front_page_solar_forecast(i, type, tile->get()->asEnum<SFType>());
                break;
            case TileType::EnergyManagerStatus:
                result = update_front_page_energy_manager_status(i, type, 0);
                break;
            case TileType::HeatingStatus:
                result = update_front_page_heating_status(i, type, 0);
                break;
            default:
                logger.printfln("Unknown tile type: %d", static_cast<std::underlying_type<TileType>::type>(type));
                break;
        }

        if (result != TF_E_OK) {
            logger.printfln("Failed to call set_display_front_page_icon: %d", result);
        }
    }
}

void FrontPanel::update_led()
{
    // Go trough possible states by decreasing priority and return after LED is set

    // Check if Wifi is enabled but not connected
    if (wifi.get_connection_state() == WifiState::NotConnected) {
        set_led(LEDPattern::Blinking, LEDColor::Red);
        return;
    }

    // TODO: Add more error checks here

    // Check if something is charging
#if MODULE_CHARGE_MANAGER_AVAILABLE()
    size_t charger_count = charge_manager.charger_count;
    if (charger_count > 0) {
        const float current = charge_manager.get_allocated_currents()->pv; // mA
        if (current > 100) {
            set_led(LEDPattern::Breathing, LEDColor::Green);
            return;
        }
    }
#endif

    // Default is green
    set_led(LEDPattern::On, LEDColor::Green);
}

void FrontPanel::update()
{
    if (!initialized) {
        return;
    }

    update_wifi();
    update_status_bar();
    update_front_page();
    update_led();
}

String FrontPanel::watt_value_to_display_string(const int32_t watt)
{
    if (watt < 10000) {
        return String(watt) + " W";
    } else if (watt < (1000*1000)) {
        uint32_t kw = watt / 1000;
        return String(kw) + " kW";
    } else if (watt < (1000*1000*1000)) {
        uint32_t mw = watt / (1000*1000);
        return String(mw) + " MW";
    } else {
        return String(">1GW");
    }
}

String FrontPanel::watt_hour_value_to_display_string(const uint32_t kilo_watt_hour)
{
    if (kilo_watt_hour < 1000) {
        return String(kilo_watt_hour) + "kWh";
    } else if (kilo_watt_hour < (1000*10)) {
        uint32_t mwh = kilo_watt_hour / 1000;
        return String(mwh/10) + "." + String(mwh%10) + "MWh";
    } else if (kilo_watt_hour < (1000*1000)) {
        uint32_t mwh = kilo_watt_hour / 1000;
        return String(mwh) + "MWh";
    } else if(kilo_watt_hour < (1000*1000*10)) {
        uint32_t gwh = kilo_watt_hour / (1000*1000);
        return String(gwh/10) + "." + String(gwh%10) + "GWh";
    } else if (kilo_watt_hour < (1000*1000*1000)) {
        uint32_t gwh = kilo_watt_hour / (1000*1000);
        return String(gwh) + "GWh";
    } else {
        return String(">1 TWh"); // damn
    }
}

String FrontPanel::price_value_to_display_string(const int32_t price)
{
    if(price >= 100) {
        return String(price/100) + "." + String((price/10) % 10) + " €";
    } else if (price <= -100) {
        return "-" + String(abs(price)/100) + "." + String((abs(price)/10) % 10) + " €";
    } else {
        return String(price) + " ct";
    }
}

void FrontPanel::check_flash_metadata()
{
    // Not yet initialized, try again in 15s.
    if (!initialized) {
        task_scheduler.scheduleOnce([this](){
            this->check_flash_metadata();
        }, 15*1000);
        return;
    }

    uint32_t version_flash     = 0;
    uint32_t version_bricklet  = 0;
    uint32_t length_flash      = 0;
    uint32_t length_bricklet   = 0;
    uint32_t checksum_flash    = 0;
    uint32_t checksum_bricklet = 0;

    int result = tf_warp_front_panel_get_flash_metadata(
        &device,
        &version_flash,
        &version_bricklet,
        &length_flash,
        &length_bricklet,
        &checksum_flash,
        &checksum_bricklet
    );

    if (result != TF_E_OK) {
        logger.printfln("Failed to call get_flash_metadata: %d", result);
        return;
    }

    const bool metadata_ok = (version_flash  == METADATA_VERSION)  && (version_bricklet  == METADATA_VERSION) &&
                             (length_flash   == METADATA_LENGTH)   && (length_bricklet   == METADATA_LENGTH)  &&
                             (checksum_flash == METADATA_CHECKSUM) && (checksum_bricklet == METADATA_CHECKSUM);

    // Check if content of flash and expectation of the Bricklet and expections of the ESP32 match
    // If it doesn't match, we can't actually do anything. The front panel will still work, but
    // probably miss some icons or similar.
    // This print will help to identify which part is not up-to-date
    if (!metadata_ok) {
        logger.printfln(
            "Flash metadata mismatch. Version: %u vs %u vs %u, Length: %u vs %u vs %u, Checksum: %x vs %x vs %x",
            version_flash,  version_bricklet,  METADATA_VERSION,
            length_flash,   length_bricklet,   METADATA_LENGTH,
            checksum_flash, checksum_bricklet, METADATA_CHECKSUM
        );
    }
}