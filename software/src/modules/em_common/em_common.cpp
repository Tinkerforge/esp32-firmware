/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "em_common.h"

#include "event_log_prefix.h"
#include "bricklet_bindings_constants.h"
#include "bindings/errors.h"
#include "build.h"
#include "musl_libc_timegm.h"

#include "module_dependencies.h"

EMCommon::EMCommon()
{
#if MODULE_EM_V1_AVAILABLE()
    backend = &em_v1;
#elif MODULE_EM_V2_AVAILABLE()
    backend = &em_v2;
#else
    #error Unknown or missing Energy Manager module!
#endif
}

void EMCommon::setup()
{
    if (!backend->is_initialized())
        return;

    //backend->post_setup();
    initialized = true;

    api.addFeature("energy_manager");

    start_network_check_task();
}

void EMCommon::register_urls()
{
    api.addState("energy_manager/state", &state);
    api.addState("energy_manager/low_level_state", &low_level_state);
    api.addPersistentConfig("energy_manager/config", &config);
}

void EMCommon::set_time(const tm &tm)
{
    int rc = backend->wem_set_date_time(static_cast<uint8_t >(tm.tm_sec),
                                        static_cast<uint8_t >(tm.tm_min),
                                        static_cast<uint8_t >(tm.tm_hour),
                                        static_cast<uint8_t >(tm.tm_mday - 1),
                                        static_cast<uint8_t >(tm.tm_wday),
                                        static_cast<uint8_t >(tm.tm_mon),
                                        static_cast<uint16_t>(tm.tm_year - 100));
    check_bricklet_reachable(rc, "set_date_time");

    if (rc != TF_E_OK)
        logger.printfln("Failed to set datetime: error %i", rc);

    return;
}

timeval EMCommon::get_time()
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

    int rc = backend->wem_get_date_time(&tm_sec, &tm_min, &tm_hour, &tm_mday, &tm_wday, &tm_mon, &tm_year);
    check_bricklet_reachable(rc, "get_date_time");

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
    date_time.tm_year = tm_year + 100;

    time.tv_sec = timegm(&date_time);

    return time;
}

bool EMCommon::get_sdcard_info(struct sdcard_info *data)
{
    int rc = backend->wem_get_sd_information(
        &data->sd_status,
        &data->lfs_status,
        &data->sector_size,
        &data->sector_count,
        &data->card_type,
        &data->product_rev,
        data->product_name,
        &data->manufacturer_id
    );

    check_bricklet_reachable(rc, "get_sd_information");

    // Product name retrieved from the SD card is an unterminated 5-character string, so we have to terminate it here.
    data->product_name[sizeof(data->product_name) - 1] = 0;

    if (rc != TF_E_OK) {
        set_error(ERROR_FLAGS_SDCARD_MASK);
        logger.printfln("Failed to get SD card information. Error %i", rc);
        return false;
    }

    if (is_error(ERROR_FLAGS_SDCARD_BIT_POS))
        clr_error(ERROR_FLAGS_SDCARD_MASK);

    return true;
}

bool EMCommon::format_sdcard()
{
    uint8_t ret_format_status;
    int rc = backend->wem_format_sd(0x4223ABCD, &ret_format_status);
    check_bricklet_reachable(rc, "format_sd");

    return rc == TF_E_OK && ret_format_status == WEM_FORMAT_STATUS_OK;
}

uint16_t EMCommon::get_energy_meter_detailed_values(float *ret_values)
{
    uint16_t len = 0;
    int rc = backend->wem_get_energy_meter_detailed_values(ret_values, &len);
    check_bricklet_reachable(rc, "get_energy_meter_detailed_values");

    return rc == TF_E_OK ? len : 0;
}

bool EMCommon::reset_energy_meter_relative_energy()
{
    int rc = backend->wem_reset_energy_meter_relative_energy();
    check_bricklet_reachable(rc, "reset_energy_meter_relative_energy");
    return rc == TF_E_OK;
}

void EMCommon::clr_error(uint32_t error_mask)
{
    error_flags &= ~error_mask;
    state.get("error_flags")->updateUint(error_flags);

#if MODULE_EM_V1_AVAILABLE()
    em_v1.update_status_led();
#endif
}

bool EMCommon::is_error(uint32_t error_bit_pos) const
{
    return (error_flags >> error_bit_pos) & 1;
}

void EMCommon::set_error(uint32_t error_mask)
{
    error_flags |= error_mask;
    state.get("error_flags")->updateUint(error_flags);

#if MODULE_EM_V1_AVAILABLE()
    em_v1.update_status_led();
#endif
}

void EMCommon::set_config_error(uint32_t config_error_mask)
{
    config_error_flags |= config_error_mask;
    state.get("config_error_flags")->updateUint(config_error_flags);

    set_error(ERROR_FLAGS_BAD_CONFIG_MASK);
}

void EMCommon::check_bricklet_reachable(int rc, const char *context)
{
    if (rc == TF_E_OK) {
        consecutive_bricklet_errors = 0;
        if (!bricklet_reachable) {
            bricklet_reachable = true;
            clr_error(ERROR_FLAGS_BRICKLET_MASK);
            logger.printfln("Bricklet is reachable again.");
        }
    } else {
        if (rc == TF_E_TIMEOUT) {
            logger.printfln("%s: Bricklet access timed out.", context);
        } else {
            logger.printfln("%s: Bricklet access returned error %d.", context, rc);
        }
        if (bricklet_reachable && ++consecutive_bricklet_errors >= 8) {
            bricklet_reachable = false;
            set_error(ERROR_FLAGS_BRICKLET_MASK);
            logger.printfln("%s: Bricklet is unreachable.", context);
        }
    }
    low_level_state.get("consecutive_bricklet_errors")->updateUint(consecutive_bricklet_errors);
}

#if MODULE_AUTOMATION_AVAILABLE()
bool EMCommon::has_triggered(const Config *conf, void *data)
{
    return false;
}
#endif

void EMCommon::start_network_check_task()
{
    task_scheduler.scheduleWithFixedDelay([this]() {
        bool disconnected;
        do {
#if MODULE_ETHERNET_AVAILABLE()
            if (ethernet.get_connection_state() == EthernetState::Connected) {
                disconnected = false;
                break;
            }
#endif
#if MODULE_WIFI_AVAILABLE()
            if (wifi.get_connection_state() == WifiState::Connected) {
                disconnected = false;
                break;
            }
#endif
#if MODULE_ETHERNET_AVAILABLE()
            if (ethernet.is_enabled()) {
                disconnected = true;
                break;
            }
#endif
#if MODULE_WIFI_AVAILABLE()
            if (wifi.is_sta_enabled()) {
                disconnected = true;
                break;
            }
#endif
            disconnected = false;
        } while (0);

        if (disconnected) {
            set_error(ERROR_FLAGS_NETWORK_MASK);
        } else {
            if (is_error(ERROR_FLAGS_NETWORK_BIT_POS))
                clr_error(ERROR_FLAGS_NETWORK_MASK);
        }
    }, 5_s);
}
