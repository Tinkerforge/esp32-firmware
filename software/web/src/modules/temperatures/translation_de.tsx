/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
import { toLocaleFixed } from "../../ts/util";
let x = {
    "temperatures": {
        "status": {},
        "navbar": {
            "temperatures": "Temperaturen"
        },
        "content": {
            "temperatures": "Temperaturen",
            "enable_temperatures": "Temperaturvorhersage aktiviert",
            "enable_temperatures_help": <>
                <p>Lädt Außentemperaturvorhersagen (Min, Max, Tagesmittel) für die konfigurierten GPS-Koordinaten (Breiten-/Längengrad) herunter. Die Daten werden alle 6 Stunden von einer Wetter-API abgerufen und liefern Vorhersagen für heute und morgen.</p>
                <p>Die Tagesmitteltemperatur wird vom Heizungsmodul für die Temperatur-Heizkurve verwendet, um die Stunden für erweiterten und blockierenden Betrieb automatisch an die Außenbedingungen anzupassen.</p>
            </>,
            "temperatures_desc": <>Automatischer Download von Temperaturvorhersagen für heute und morgen. {__("The_device")} muss mit dem Internet verbunden sein.</>,
            "temperatures_push_desc": <>Temperaturvorhersagen für heute und morgen werden über die API bereitgestellt. Keine Internetverbindung erforderlich.</>,
            "source": "Quelle",
            "source_weather_service": "Automatisch über Wetterdienst",
            "source_weather_service_desc": /*FFN*/(api_url: string) => <>Die Daten werden von <a href={api_url}>{api_url.replace("https://", "").replace(/\/$/, "")}</a> abgerufen.</>/*NF*/,
            "source_push": "Push über API",
            "source_push_desc": <>Im Push-Modus werden Temperaturdaten über den API-Endpunkt <a href="https://docs.warp-charger.com/docs/interfaces/mqtt_http/api_reference/temperatures" target="_blank">temperatures/temperatures_update</a> bereitgestellt.</>,
            "latitude": "Breitengrad",
            "latitude_muted": "in Grad (z.B. 51,9035)",
            "longitude": "Längengrad",
            "longitude_muted": "in Grad (z.B. 8,6720)",
            "today": "Heute",
            "tomorrow": "Morgen",
            "min_temp": "Min",
            "max_temp": "Max",
            "avg_temp": "Mittel",
            "temperature": "Temperatur",
            "temperature_forecast": "Temperaturvorhersage",
            "temperature_degc": "Temperatur [°C]",
            "current_temperature": "Außentemperatur",
            "time": "Uhrzeit",
            "now": "Jetzt",
            "loading": "Lade Daten...",
            "last_update": "Letzte Aktualisierung",
            "next_update": "Nächste Aktualisierung",
            "no_data": "Keine Daten verfügbar",
            "not_configured": "Standort nicht konfiguriert",
            "unknown": "Unbekannt"
        },
        "automation": {
            "trigger_text": /*FFN*/(type: number, comparison: number, value: number) => {
                const type_names = [
                    "aktuelle Außentemperatur",
                    "heutige Mindesttemperatur",
                    "heutige Durchschnittstemperatur",
                    "heutige Höchsttemperatur",
                    "morgige Mindesttemperatur",
                    "morgige Durchschnittstemperatur",
                    "morgige Höchsttemperatur",
                ];
                const type_name = type_names[type] ?? "unbekannt";
                const comp = comparison == 0 ? "größer" : "kleiner";
                return (<>Wenn die {type_name} <b>{comp}</b> als <b>{toLocaleFixed(value / 10, 1)} °C</b> ist,{" "}</>)
            }/*NF*/,
            "temperature_type": "Temperaturwert",
            "type_current": "Aktuelle Außentemperatur",
            "type_today_min": "Heutiges Minimum",
            "type_today_avg": "Heutiger Durchschnitt",
            "type_today_max": "Heutiges Maximum",
            "type_tomorrow_min": "Morgiges Minimum",
            "type_tomorrow_avg": "Morgiger Durchschnitt",
            "type_tomorrow_max": "Morgiges Maximum",
            "comparison": "Vergleich",
            "comparison_greater_than": "Größer als",
            "comparison_less_than": "Kleiner als",
            "value": "Schwellwert",
            "current_temperature": "Temperatur"
        },
        "script": {
            "save_failed": "Speichern der Temperatureinstellungen fehlgeschlagen"
        }
    }
}
