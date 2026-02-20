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
