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

#pragma once

#include "device_module.h"
#include "config.h"
#include "build.h"
#include "bindings/bricklet_warp_front_panel.h"
#include "module_available.h"

#define FRONT_PANEL_TILES 6
#define TILE_TYPES 8

class FrontPanel : public DeviceModule<TF_WARPFrontPanel,
                                       tf_warp_front_panel_create,
                                       tf_warp_front_panel_get_bootloader_mode,
                                       tf_warp_front_panel_reset,
                                       tf_warp_front_panel_destroy>
{
public:
    enum class LEDPattern : uint8_t {
        Off       = 0,
        On        = 1,
        Blinking  = 2,
        Breathing = 3,
    };
    enum class LEDColor : uint8_t {
        Green  = 0,
        Red    = 1,
        Yellow = 2,
    };

    FrontPanel();

    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void loop() override;
    void setup_bricklet();
    void check_bricklet_state();
    int set_led(const LEDPattern pattern, const LEDColor color);
    int get_led(LEDPattern *pattern, LEDColor *color);

private:
    enum class TileType : uint8_t {
        EmptyTile           = 0,
        Wallbox             = 1,
        ChargeManagement    = 2,
        Meter               = 3,
        DayAheadPrices      = 4,
        SolarForecast       = 5,
        EnergyManagerStatus = 6,
        HeatingStatus       = 7,
    };

    enum class DAPType : uint8_t {
        CurrentPrice = 0,
        AveragePriveToday = 1,
        AveragePriveTomorrow = 2,
    };

    enum class SFType : uint8_t {
        ForecastToday = 0,
        ForecastTomorrow = 1,
    };

    void update();
    void update_wifi();
    void update_status_bar();
    void update_front_page();
    void update_led();
    int update_front_page_empty_tile(const uint8_t index, const TileType type, const uint8_t param);
    int update_front_page_wallbox(const uint8_t index, const TileType type, const uint8_t param);
    int update_front_page_charge_management(const uint8_t index, const TileType type, const uint8_t param);
    int update_front_page_meter(const uint8_t index, const TileType type, const uint8_t param);
    int update_front_page_day_ahead_prices(const uint8_t index, const TileType type, const DAPType param);
    int update_front_page_solar_forecast(const uint8_t index, const TileType type, const SFType param);
    int update_front_page_energy_manager_status(const uint8_t index, const TileType type, const uint8_t param);
    int update_front_page_heating_status(const uint8_t index, const TileType type, const uint8_t param);
    int set_display_front_page_icon_with_check(const uint32_t icon_index, bool active, const uint32_t sprite_index, const char *text_1, const uint8_t font_index_1, const char *text_2, const uint8_t font_index_2);

    const char* get_i18n_string(const char *key_en, const char *key_de);
    void check_flash_metadata();

    String watt_value_to_display_string(const int32_t w);
    String watt_hour_value_to_display_string(const uint32_t wh);
    String price_value_to_display_string(const int32_t price);

    ConfUnionPrototype<TileType> tile_prototypes[TILE_TYPES];
    Config config_tiles_prototype;
    ConfigRoot config;
};

#include "module_available_end.h"
