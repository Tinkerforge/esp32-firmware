/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "solar_forecast": {
        "status": {},
        "navbar": {
            "solar_forecast": "Solar Forecast"
        },
        "content": {
            "solar_forecast": "Solar Forecast",
            "enable_solar_forecast": "Solar forecast enabled",
            "enable_solar_forecast_muted": /*FFN*/(api_url: string) => <>The forecast is queried from <a href={api_url}>{api_url.replace("https://", "").replace(/\/$/, "")}</a>.</>/*NF*/,
            "planes": "PV planes",
            "plane": "PV plane",
            "no_data": "No data available!",
            "loading": "Loading data...",
            "plane_config_name": "Display name",
            "plane_config_latitude": "Latitude",
            "plane_config_latitude_muted": "e.g. 51.8846°",
            "plane_config_longitude": "Longitude",
            "plane_config_longitude_muted": "e.g. 8.6251°",
            "plane_config_declination": "Declination",
            "plane_config_declination_muted": "0° horizontal to 90° vertical",
            "plane_config_azimuth": "Azimuth",
            "plane_config_azimuth_muted": "-180° = north, -90° = East, 0° = south, 90° = west, 180° = north",
            "plane_config_kwp": "Peak power",
            "plane_config_kwp_muted": "Peak power of the PV plane",
            "add_plane_config_title": "Add PV plane",
            "add_plane_config_count": /*SFN*/(x: number, max: number) => x + " of " + max + " PV planes configured"/*NF*/,
            "add_plane_config_done": "All PV planes added (6 of 6)",
            "edit_plane_config_title": "Edit PV plane",
            "solar_forecast_desc": <>Automatic download of solar forecast data. {__("The_device")} must be connected to the Internet.</>,
            "table_name": "Name",
            "table_azimuth": "Azimuth",
            "table_declination": "Declination",
            "table_kwp": "kWp",
            "table_latitude": "Latitude",
            "table_longitude": "Longitude",
            "time": "Time of day",
            "solar_forecast_chart_heading": "Status",
            "solar_forecast_now_label": "Solar forecast from now",
            "solar_forecast_days_label": "Solar forecast",
            "solar_forecast_today_label": "Today",
            "solar_forecast_today_label_muted": "00:00 to 23:59",
            "solar_forecast_tomorrow_label": "Tomorrow",
            "solar_forecast_tomorrow_label_muted": "00:00 to 23:59",
            "time_to": "to",
            "unknown_not_yet": "Unknown (Solar forecast has not been queried yet)",
            "rate_limit_label": "Query limit",
            "rate_limit_label_muted": "How often can the solar forecast be queried from the API (in a 2-hour interval)",
            "remaining_requests_label": "Remaining queries",
            "remaining_requests_label_muted": "How many more times can the solar forecast be queried from the API in this interval",
            "next_api_call_label": "Next API call",
            "next_api_call_label_muted": "When will the solar forecast be queried next",
            "unknown": "Unknown",
            "not_set_for_this_plane": "Solar forecast has not yet been queried for this plane.",
            "address_of_pv_plane": "Address of the PV plane:",
            "last_update_attempt": "Last update attempt:",
            "last_successful_update": "Last successful update:",
            "next_update": "Next update:",
            "next_update_in": "Next update in",
            "next_update_in_help": "The solar forecast may be queried twelve times in a two-hour interval. Each plane is queried individually.",
            "remaining_queries": /*SFN*/(x: number, max: number) => "(Remaining queries: " + x + " of " + max + ")"/*NF*/,
            "not_yet_queried": "Not yet queried"
        },
        "script": {
            "power": "Power",
            "save_failed": "Failed to save the solar forecast settings"
        }
    }
}
