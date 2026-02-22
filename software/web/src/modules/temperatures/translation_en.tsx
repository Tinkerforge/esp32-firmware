/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
import { toLocaleFixed } from "../../ts/util";
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
            "temperatures_desc": <>Automatic download of temperature forecasts for today and tomorrow. {__("The_device")} must be connected to the Internet.</>,
            "temperatures_push_desc": <>Temperature forecasts for today and tomorrow are provided via the API. No Internet connection required.</>,
            "source": "Source",
            "source_weather_service": "Automatic via weather service",
            "source_weather_service_desc": /*FFN*/(api_url: string) => <>The data is fetched from <a href={api_url}>{api_url.replace("https://", "").replace(/\/$/, "")}</a>.</>/*NF*/,
            "source_push": "Push via API",
            "source_push_desc": <>In push mode, temperature data is provided via the API endpoint <a href="https://docs.warp-charger.com/docs/interfaces/mqtt_http/api_reference/temperatures" target="_blank">temperatures/temperatures_update</a>.</>,
            "latitude": "Latitude",
            "latitude_muted": "in degrees (e.g., 51.9035)",
            "longitude": "Longitude",
            "longitude_muted": "in degrees (e.g., 8.6720)",
            "today": "Today",
            "tomorrow": "Tomorrow",
            "min_temp": "Min",
            "max_temp": "Max",
            "avg_temp": "Avg",
            "temperature": "Temperature",
            "temperature_forecast": "Temperature forecast",
            "temperature_degc": "Temperature [°C]",
            "current_temperature": "Outdoor temperature",
            "time": "Time of day",
            "now": "Now",
            "loading": "Loading data...",
            "last_update": "Last update",
            "next_update": "Next update",
            "no_data": "No data available",
            "not_configured": "Location not configured",
            "unknown": "Unknown"
        },
        "automation": {
            "trigger_text": /*FFN*/(type: number, comparison: number, value: number) => {
                const type_names = [
                    "current outdoor temperature",
                    "today's minimum temperature",
                    "today's average temperature",
                    "today's maximum temperature",
                    "tomorrow's minimum temperature",
                    "tomorrow's average temperature",
                    "tomorrow's maximum temperature",
                ];
                const type_name = type_names[type] ?? "unknown";
                const comp = comparison == 0 ? "greater" : "less";
                return (<>If the {type_name} is <b>{comp}</b> than <b>{toLocaleFixed(value / 10, 1)} °C</b>,{" "}</>)
            }/*NF*/,
            "temperature_type": "Temperature value",
            "type_current": "Current outdoor temperature",
            "type_today_min": "Today's minimum",
            "type_today_avg": "Today's average",
            "type_today_max": "Today's maximum",
            "type_tomorrow_min": "Tomorrow's minimum",
            "type_tomorrow_avg": "Tomorrow's average",
            "type_tomorrow_max": "Tomorrow's maximum",
            "comparison": "Comparison",
            "comparison_greater_than": "Greater than",
            "comparison_less_than": "Less than",
            "value": "Threshold",
            "current_temperature": "Temperature"
        },
        "script": {
            "save_failed": "Failed to save the temperature settings"
        }
    }
}
