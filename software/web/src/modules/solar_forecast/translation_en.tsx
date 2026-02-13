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
            "source": "Source",
            "source_forecast_service": "Automatic via forecast service",
            "source_forecast_service_desc": /*FFN*/(api_url: string) => <>The forecast is queried from <a href={api_url}>{api_url.replace("https://", "").replace(/\/$/, "")}</a>.</>/*NF*/,
            "source_push": "Push via API",
            "source_push_desc": <>In push mode, forecast data is provided via the API endpoint <a href="https://docs.warp-charger.com/docs/interfaces/mqtt_http/api_reference/solar_forecast" target="_blank">solar_forecast/planes/X/forecast_update</a>.</>,
            "planes": "PV planes",
            "no_data": "No data available",
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
            "add_plane_config_message": /*SFN*/(have: number, max: number) => `${have} of ${max} PV planes configured`/*NF*/,
            "edit_plane_config_title": "Edit PV plane",
            "solar_forecast_desc": "Enable PV yield forecast.",
            "table_name": "Name",
            "table_azimuth": "Azimuth",
            "table_declination": "Declination",
            "table_kwp": "kWp",
            "table_latitude": "Latitude",
            "table_longitude": "Longitude",
            "time": "Time of day",
            "solar_forecast_now": "Solar forecast from now",
            "solar_forecast_days": "Solar forecast",
            "solar_forecast_days_muted": "00:00 to 23:59",
            "solar_forecast_today": "Today",
            "solar_forecast_tomorrow": "Tomorrow",
            "time_to": "to",
            "unknown": "Unknown",
            "not_set_for_this_plane": "Solar forecast has not yet been queried for this plane.",
            "server_error_404": "Forecast server reports 404: Invalid latitude or longitude",
            "server_error_422": "Forecast server reports 422: Invalid declination, azimuth or peak power",
            "server_error_other": "Forecast server reports error",
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
