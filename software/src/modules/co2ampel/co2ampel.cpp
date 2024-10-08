/* warp-charger
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "co2ampel.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "bindings/errors.h"

#include "gcc_warnings.h"

extern TF_HAL hal;

#define RED 3
#define YELLOW 2
#define GREEN 1
#define OFF 0

void Co2Ampel::pre_setup()
{
    config = Config::Object({
        {"temperature_offset", Config::Uint16(0)},
    });

    state = Config::Object({
        {"co2", Config::Uint16(0)},
        {"temperature", Config::Int16(0)},
        {"humidity", Config::Uint16(0)},
        {"air_pressure", Config::Uint16(0)},
        {"led", Config::Uint8(0)}
    });
}

#define DEFAULT_RED_LIMIT 1500
#define DEFAULT_YELLOW_LIMIT 1000
#define HYSTERESIS 10

static uint16_t red_limit = 1500;
static uint16_t yellow_limit = 1000;

static bool last_blink = false;

static bool blink_allowed = true;

static void touch_position_handler(TF_LCD128x64 *device, uint16_t pressure, uint16_t x,
                                   uint16_t y, uint32_t age, void *user_data) {
    blink_allowed = false;
}

void Co2Ampel::set_color(uint32_t c)
{
    switch (c) {
        case RED:
            check(tf_rgb_led_v2_set_rgb_value(&rgb, 255, 0, 0), "call set_rgb_value");
            break;
        case YELLOW:
            check(tf_rgb_led_v2_set_rgb_value(&rgb, 218, 255, 0), "call set_rgb_value");
            break;
        case GREEN:
            check(tf_rgb_led_v2_set_rgb_value(&rgb, 0, 255, 0), "call set_rgb_value");
            break;
        default:
        case OFF:
            check(tf_rgb_led_v2_set_rgb_value(&rgb, 0, 0, 0), "call set_rgb_value");
            break;
    }

    state.get("led")->updateUint(c);
}

void Co2Ampel::setup()
{
    int result = tf_co2_v2_create(&co2, nullptr, &hal);
    if (result != TF_E_OK) {
        logger.printfln("Failed to initialize CO2 Bricklet.");
        return;
    }

    result = tf_lcd_128x64_create(&lcd, nullptr, &hal);
    if (result != TF_E_OK) {
        logger.printfln("Failed to initialize LCD Bricklet.");
        return;
    }

    result = tf_rgb_led_v2_create(&rgb, nullptr, &hal);
    if (result != TF_E_OK) {
        logger.printfln("Failed to initialize RGB LED Bricklet.");
        return;
    }

    api.restorePersistentConfig("co2ampel/config", &config);

    tf_co2_v2_set_temperature_offset(&co2, static_cast<uint16_t>(config.get("temperature_offset")->asUint()));
    tf_lcd_128x64_clear_display(&lcd);
    tf_lcd_128x64_set_touch_position_callback_configuration(&lcd, 100, true);
    tf_lcd_128x64_register_touch_position_callback(&lcd, touch_position_handler, NULL);

    tf_rgb_led_v2_set_status_led_config(&rgb, TF_RGB_LED_V2_STATUS_LED_CONFIG_OFF);

    task_scheduler.scheduleWithFixedDelay([this]() {
        uint16_t co2_concentration, humidity, air_pressure; int16_t temperature;
        check(tf_co2_v2_get_all_values(&co2, &co2_concentration, &temperature, &humidity), "get all values");
        state.get("co2")->updateUint(co2_concentration);
        state.get("temperature")->updateInt(temperature);
        state.get("humidity")->updateUint(humidity);

        tf_co2_v2_get_air_pressure(&co2, &air_pressure);
        state.get("air_pressure")->updateUint(air_pressure);

        String test = String(static_cast<unsigned int>(co2_concentration)) + "ppm    ";
        //check(tf_lcd_128x64_write_line(&lcd, 0, 0, test.c_str()), "call write_line");
        check(tf_lcd_128x64_draw_text(&lcd, 0, 0, TF_LCD_128X64_FONT_18X32, TF_LCD_128X64_COLOR_BLACK, test.c_str()), "call draw_text");

        char temp_buf[12] = {0};
        size_t written = snprintf_u(temp_buf, ARRAY_SIZE(temp_buf), "%2.1f \xF8", static_cast<double>(temperature / 100.0f));
        temp_buf[written] = 'C';
        temp_buf[written+1] = ' ';
        check(tf_lcd_128x64_draw_text(&lcd, 0, 40, TF_LCD_128X64_FONT_6X24, TF_LCD_128X64_COLOR_BLACK, temp_buf), "call draw_text");

        char hum_buf[12] = {0};
        written = snprintf_u(hum_buf, ARRAY_SIZE(hum_buf), " %2.1f %%RH", static_cast<double>(humidity / 100.0f));
        check(tf_lcd_128x64_draw_text(&lcd, static_cast<uint8_t>(128 - written * 6), 40, TF_LCD_128X64_FONT_6X24, TF_LCD_128X64_COLOR_BLACK, hum_buf), "call draw_text");

        char t_buf[22] = {0};
        uint32_t t = millis();
        uint32_t d = t / (24 * 60 * 60 * 1000);
        t -= d * 24 * 60 * 60 * 1000;
        uint32_t h = t / (60 * 60 * 1000);
        t -= h * 60 * 60 * 1000;
        uint32_t m = t / (60 * 1000);
        t -= m * 60 * 1000;
        uint32_t s = t / 1000;

        snprintf(t_buf, ARRAY_SIZE(t_buf), "%02u %02u", d, h);
        tf_lcd_128x64_write_line(&lcd, 6, 8, t_buf);

        snprintf(t_buf, ARRAY_SIZE(t_buf), "%02u %02u", m, s);
        tf_lcd_128x64_write_line(&lcd, 7, 8, t_buf);

        if(co2_concentration > red_limit) {
            red_limit = DEFAULT_RED_LIMIT - HYSTERESIS;
            if (blink_allowed) {
                if(last_blink) {
                    set_color(RED);
                } else {
                    set_color(OFF);
                }
                tf_lcd_128x64_set_display_configuration(&lcd, 14, last_blink ? 100 : 0, false, true);
                last_blink = !last_blink;
            } else {
                tf_lcd_128x64_set_display_configuration(&lcd, 14, 100, false, true);
                set_color(RED);
            }
        } else {
            red_limit = DEFAULT_RED_LIMIT + HYSTERESIS;
            tf_lcd_128x64_set_display_configuration(&lcd, 14, 100, false, true);
            blink_allowed = true;

            if (co2_concentration > yellow_limit){
                yellow_limit = DEFAULT_YELLOW_LIMIT - HYSTERESIS;
                set_color(YELLOW);
            } else {
                yellow_limit = DEFAULT_YELLOW_LIMIT + HYSTERESIS;
                set_color(GREEN);
            }
        }
    }, 500_ms);

    initialized = true;
}

void Co2Ampel::register_urls()
{
    api.addPersistentConfig("co2ampel/config", &config);
    api.addState("co2ampel/state", &state);
    api.addCommand("co2ampel/stop_blink", Config::Null(), {}, [](String &/*errmsg*/) {
        blink_allowed = false;
    }, true);
}
