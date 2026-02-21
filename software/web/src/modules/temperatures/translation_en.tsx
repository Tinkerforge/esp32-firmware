/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "temperatures": {
        "status": {},
        "navbar": {
            "temperatures": "Temperatures"
        },
        "content": {
            "temperatures": "Temperatures",
            "enable_temperatures": "Temperature forecast enabled",
            "enable_temperatures_help": <>
                <p>Downloads outdoor temperature forecasts (min, max, daily average) for the configured GPS coordinates (latitude/longitude). The data is fetched from a weather API every 6 hours and provides forecasts for today and tomorrow.</p>
                <p>The daily average temperature is used by the heating module's temperature heating curve to automatically adjust extended and blocking operation hours based on outdoor conditions.</p>
            </>,
            "temperatures_muted": /*FFN*/(api_url: string) => <>The data is fetched from <a href={api_url}>{api_url.replace("https://", "").replace(/\/$/, "")}</a>.</>/*NF*/,
            "temperatures_desc": <>Automatic download of temperature forecasts for today and tomorrow. {__("The_device")} must be connected to the Internet.</>,
            "latitude": "Latitude",
            "latitude_muted": "in degrees (e.g., 51.9035)",
            "longitude": "Longitude",
            "longitude_muted": "in degrees (e.g., 8.6720)",
            "status_section": "Status",
            "today": "Today",
            "tomorrow": "Tomorrow",
            "min_temp": "Min",
            "max_temp": "Max",
            "avg_temp": "Avg",
            "last_update": "Last update",
            "next_update": "Next update",
            "no_data": "No data available",
            "not_configured": "Location not configured",
            "unknown": "Unknown"
        },
        "script": {
            "save_failed": "Failed to save the temperature settings"
        }
    }
}
