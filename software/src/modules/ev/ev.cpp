/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include "ev.h"

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"

#include "gcc_warnings.h"

#define EV_MAC_STRING_LENGTH 17

// Parse a MAC string "AA:BB:CC:DD:EE:FF" or pattern "AA:BB:CC:xx:xx:xx" into a byte array.
// If out_mask is not nullptr, "xx" bytes are accepted as wildcards (mac=0x00, mask=0x00).
// Returns false on invalid format.
static bool parse_mac(const char *str, uint8_t *out_mac, uint8_t *out_mask = nullptr)
{
    if (strlen(str) != EV_MAC_STRING_LENGTH) {
        return false;
    }

    for (int i = 0; i < EV_MAC_ADDRESS_LENGTH; i++) {
        char hi = str[i * 3];
        char lo = str[i * 3 + 1];

        if (i < 5 && str[i * 3 + 2] != ':') {
            return false;
        }

        // Check for wildcard "xx" or "XX"
        if ((hi == 'x' || hi == 'X') && (lo == 'x' || lo == 'X')) {
            if (out_mask == nullptr) {
                return false;
            }
            out_mac[i] = 0x00;
            out_mask[i] = 0x00;
            continue;
        }

        auto hex_nibble = [](char c, uint8_t &nibble) -> bool {
            if (c >= '0' && c <= '9') { nibble = static_cast<uint8_t>(c - '0'); return true; }
            if (c >= 'A' && c <= 'F') { nibble = static_cast<uint8_t>(c - 'A' + 10); return true; }
            if (c >= 'a' && c <= 'f') { nibble = static_cast<uint8_t>(c - 'a' + 10); return true; }
            return false;
        };

        uint8_t h, l;
        if (!hex_nibble(hi, h) || !hex_nibble(lo, l)) {
            return false;
        }
        out_mac[i] = static_cast<uint8_t>((h << 4) | l);
        if (out_mask != nullptr) {
            out_mask[i] = 0xFF;
        }
    }
    return true;
}

static void format_mac_string(const uint8_t *mac, char *out, size_t out_size)
{
    snprintf(out, out_size, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void Ev::pre_setup()
{
    ev_config_prototype = Config::Object({
        {"name",                Config::Str("", 0, 16)},
        {"mac",                 Config::Str("", 0, EV_MAC_STRING_LENGTH)},
        {"capacity",            Config::Float(0.0f)},
        {"charging_efficiency", Config::Float(EV_DEFAULT_CHARGING_EFFICIENCY)}
    });

    config = ConfigRoot{Config::Object({
        {"evs", Config::Array(
            {},
            &ev_config_prototype,
            0, EV_MAX_EVS,
            Config::type_id<Config::ConfObject>())
        }
    }), [](Config &update, ConfigSource source) -> String {
        Config *evs = static_cast<Config *>(update.get("evs"));
        size_t count = evs->count();

        for (size_t i = 0; i < count; i++) {
            String mac_str = evs->get(i)->get("mac")->asString();
            if (mac_str.length() == 0) {
                return "Empty MAC address is not allowed";
            }

            // Validate and normalize MAC format
            mac_str.toUpperCase();
            evs->get(i)->get("mac")->updateString(mac_str);

            uint8_t dummy_mac[EV_MAC_ADDRESS_LENGTH];
            uint8_t dummy_mask[EV_MAC_ADDRESS_LENGTH];
            if (!parse_mac(mac_str.c_str(), dummy_mac, dummy_mask)) {
                return String("Invalid MAC address format at index ") + static_cast<int>(i) + ". Expected format: AA:BB:CC:DD:EE:FF or AA:BB:CC:xx:xx:xx";
            }

            // Check for duplicates
            for (size_t j = i + 1; j < count; j++) {
                if (mac_str == evs->get(j)->get("mac")->asString()) {
                    return String("Duplicate MAC address at index ") + static_cast<int>(i) + " and " + static_cast<int>(j);
                }
            }
        }

        return "";
    }};

    state = Config::Object({
        {"active_ev_index",     Config::Int(-1, -1, EV_MAX_EVS - 1)},
        {"name",                Config::Str("", 0, 16)},
        {"mac",                 Config::Str("", 0, 17)},
        {"soc",                 Config::Float(NAN)},
        {"capacity",            Config::Float(NAN)},
        {"charging_efficiency", Config::Float(EV_DEFAULT_CHARGING_EFFICIENCY)}
    });

    inject_soc = Config::Object({
        {"soc", Config::Float(0.0f)}
    });

    seen_macs_prototype = Config::Object({
        {"mac",       Config::Str("", 0, EV_MAC_STRING_LENGTH)},
        {"last_seen", Config::Uint32(0)}
    });

    seen_macs = Config::Array(
        {},
        &seen_macs_prototype,
        0, EV_SEEN_MAC_COUNT,
        Config::type_id<Config::ConfObject>()
    );

    inject_ev = ConfigRoot{Config::Object({
        {"mac", Config::Str("", 0, EV_MAC_STRING_LENGTH)}
    }), [](Config &cfg, ConfigSource source) -> String {
        String mac_str = cfg.get("mac")->asString();
        mac_str.toUpperCase();
        cfg.get("mac")->updateString(mac_str);

        if (mac_str.length() == 0) {
            return "MAC address must not be empty.";
        }

        uint8_t dummy[EV_MAC_ADDRESS_LENGTH];
        if (!parse_mac(mac_str.c_str(), dummy)) {
            return "Invalid MAC address format. Expected format: AA:BB:CC:DD:EE:FF";
        }

        return "";
    }};
}

void Ev::setup()
{
    api.restorePersistentConfig("ev/config", &config);
    initialized = true;
}

void Ev::register_urls()
{
    api.addPersistentConfig("ev/config", &config);
    api.addState("ev/state", &state);
    api.addState("ev/seen_macs", &seen_macs);

    api.addCommand("ev/inject_soc", &inject_soc, {}, [this](Language /*language*/, String &errmsg) {
        float soc = inject_soc.get("soc")->asFloat();
        if ((soc < 0.0f) || (soc > 100.0f)) {
            errmsg = "SoC must be between 0 and 100";
            return;
        }
        set_soc(soc);
        session_updated(EVDataProtocol::None);
    }, true);

    api.addCommand("ev/inject_ev", &inject_ev, {"mac"}, [this](Language /*language*/, String &errmsg) {
        uint8_t mac[EV_MAC_ADDRESS_LENGTH];
        if (!parse_mac(inject_ev.get("mac")->asEphemeralCStr(), mac)) {
            errmsg = "Invalid MAC address format. Expected format: AA:BB:CC:DD:EE:FF";
            return;
        }
        on_ev_connected(mac);

#if MODULE_ISO15118_AVAILABLE()
        if (iso15118.is_enabled() && !iso15118.iec_temporary_active) {
            iso15118.begin_iec_transition();
        }
#endif
    }, true);
}

void Ev::register_events()
{
    event.registerEvent("evse/state", {"charger_state"}, [this](const Config *charger_state) {
        if (charger_state->asUint() != 0) {
            // EV connected: Start SoC estimation
            if (soc_estimation_task == 0) {
                soc_estimation_task = task_scheduler.scheduleWithFixedDelay([this]() {
                    update_estimated_soc();
                }, 1_s, 1_s);
            }
        } else {
            // EV disconnected (State A): Stop SoC estimation
            if (soc_estimation_task != 0) {
                task_scheduler.cancel(soc_estimation_task);
                soc_estimation_task = 0;
            }

            on_ev_disconnected();
        }
        return EventResult::OK;
    });
}

void Ev::on_ev_connected(const uint8_t mac[EV_MAC_ADDRESS_LENGTH])
{
    active_ev_index = -1;
    clear_session();

    // Track this MAC in the seen_macs list
    add_seen_mac_address(mac);

    // Find matching profile from config
    Config *evs = static_cast<Config *>(config.get("evs"));
    size_t count = evs->count();

    for (size_t i = 0; i < count; i++) {
        const String &mac_str = evs->get(i)->get("mac")->asString();
        if (mac_str.length() == 0) {
            continue;
        }

        uint8_t profile_mac[EV_MAC_ADDRESS_LENGTH];
        uint8_t profile_mask[EV_MAC_ADDRESS_LENGTH];
        if (!parse_mac(mac_str.c_str(), profile_mac, profile_mask)) {
            logger.printfln("Invalid MAC pattern in config at index %zu: '%s'", i, mac_str.c_str());
            continue;
        }

        bool match = true;
        for (int j = 0; j < EV_MAC_ADDRESS_LENGTH; j++) {
            if ((mac[j] & profile_mask[j]) != (profile_mac[j] & profile_mask[j])) {
                match = false;
                break;
            }
        }
        if (match) {
            active_ev_index = static_cast<int>(i);
            break;
        }
    }

    // Apply profile config to session
    if (active_ev_index >= 0) {
        const Config *entry = static_cast<const Config *>(evs->get(active_ev_index));
        float cap = entry->get("capacity")->asFloat();
        if (cap > 0.0f) {
            session.capacity = cap;
        }
        session.charging_efficiency = entry->get("charging_efficiency")->asFloat();

        logger.printfln("EV connected: Matched profile '%s' (index %d)",
                        entry->get("name")->asString().c_str(), active_ev_index);
    } else {
        char mac_fmt[18];
        format_mac_string(mac, mac_fmt, sizeof(mac_fmt));
        logger.printfln("EV connected: Unknown MAC %s", mac_fmt);
    }

    // Update API state
    char mac_fmt[18];
    format_mac_string(mac, mac_fmt, sizeof(mac_fmt));

    state.get("active_ev_index")->updateInt(active_ev_index);
    state.get("mac")->updateString(mac_fmt);
    if (active_ev_index >= 0) {
        state.get("name")->updateString(evs->get(active_ev_index)->get("name")->asString());
    } else {
        state.get("name")->updateString("");
    }
    state.get("soc")->updateFloat(session.soc);
    state.get("capacity")->updateFloat(session.capacity);
    state.get("charging_efficiency")->updateFloat(session.charging_efficiency);
}

void Ev::on_ev_disconnected()
{
    active_ev_index = -1;
    clear_session();

    // Update API state
    state.get("active_ev_index")->updateInt(-1);
    state.get("name")->updateString("");
    state.get("mac")->updateString("");
    state.get("soc")->updateFloat(NAN);
    state.get("capacity")->updateFloat(NAN);
    state.get("charging_efficiency")->updateFloat(EV_DEFAULT_CHARGING_EFFICIENCY);

    // Clear meter values
#if MODULE_METERS_ISO15118_AVAILABLE()
    meters_iso15118.clear_values();
#endif
}

void Ev::set_soc(float soc)
{
    session.energy_at_soc_reading = get_session_energy_kwh();
    session.soc_initial           = soc;
    session.soc                   = soc;

    state.get("soc")->updateFloat(soc);
}

void Ev::update_estimated_soc()
{
    if (isnan(session.soc_initial)) {
        return;
    }

    if (isnan(session.capacity) || (session.capacity <= 0.0f)) {
        return;
    }

    float energy_delta_kwh = get_session_energy_kwh() - session.energy_at_soc_reading;
    float soc = session.soc_initial + (energy_delta_kwh * session.charging_efficiency / session.capacity) * 100.0f;

    // Clamp to [0, 100]
    if (soc < 0.0f) {
        soc = 0.0f;
    } else if (soc > 100.0f) {
        soc = 100.0f;
    }

    session.soc = soc;
    state.get("soc")->updateFloat(soc);

#if MODULE_METERS_ISO15118_AVAILABLE()
    if (!isnan(session.soc)) {
        meters_iso15118.update_soc(session.soc);
    }
#endif
}

void Ev::session_updated(EVDataProtocol protocol)
{
    // Update API state with current session values
    state.get("soc")->updateFloat(session.soc);
    state.get("capacity")->updateFloat(session.capacity);

#if MODULE_METERS_ISO15118_AVAILABLE()
    meters_iso15118.update_values(
        session.soc,
        session.capacity,
        session.power,
        protocol
    );
#endif
}

float Ev::get_session_energy_kwh()
{
#if MODULE_CHARGE_TRACKER_AVAILABLE()
    float energy_now;
    if (evse_common.get_charger_meter_energy(&energy_now) == MeterValueAvailability::Unavailable) {
        return 0.0f;
    }
    float meter_start = charge_tracker.current_charge.get("meter_start")->asFloat();
    float session_energy = energy_now - meter_start;
    return std::max(0.0f, session_energy);
#else
    return 0.0f;
#endif
}

void Ev::clear_session()
{
    session = EVSession();
}

void Ev::add_seen_mac_address(const uint8_t mac[EV_MAC_ADDRESS_LENGTH])
{
    char mac_str[EV_MAC_STRING_LENGTH+1];
    format_mac_string(mac, mac_str, sizeof(mac_str));

    struct timeval tv;
    const uint32_t now =
#if MODULE_RTC_AVAILABLE()
        rtc.clock_synced(&tv) ? static_cast<uint32_t>(tv.tv_sec) :
#endif
        0;

    // Check if MAC already exists
    for (size_t i = 0; i < seen_macs.count(); i++) {
        if (seen_macs.get(i)->get("mac")->asString() == mac_str) {
            // MAC exists, update timestamp
            seen_macs.get(i)->get("last_seen")->updateUint(now);
            return;
        }
    }

    // MAC doesn't exist
    size_t target_idx;
    if (seen_macs.count() < EV_SEEN_MAC_COUNT) {
        // Add new entry
        seen_macs.add();
        target_idx = seen_macs.count() - 1;
    } else {
        // Array full, find oldest entry to replace
        target_idx = 0;
        uint32_t oldest_time = UINT32_MAX;

        for (size_t i = 0; i < seen_macs.count(); i++) {
            uint32_t entry_time = seen_macs.get(i)->get("last_seen")->asUint();
            if (entry_time < oldest_time) {
                oldest_time = entry_time;
                target_idx = i;
            }
        }
    }

    seen_macs.get(target_idx)->get("mac")->updateString(mac_str);
    seen_macs.get(target_idx)->get("last_seen")->updateUint(now);
}
