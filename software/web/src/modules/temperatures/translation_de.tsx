/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
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
            "last_update": "Letzte Aktualisierung",
            "next_update": "Nächste Aktualisierung",
            "no_data": "Keine Daten verfügbar",
            "not_configured": "Standort nicht konfiguriert",
            "unknown": "Unbekannt"
        },
        "script": {
            "save_failed": "Speichern der Temperatureinstellungen fehlgeschlagen"
        }
    }
}
