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
            "temperatures_muted": /*FFN*/(api_url: string) => <>Die Daten werden von <a href={api_url}>{api_url.replace("https://", "").replace(/\/$/, "")}</a> abgerufen.</>/*NF*/,
            "temperatures_desc": <>Automatischer Download von Temperaturvorhersagen für heute und morgen. {__("The_device")} muss mit dem Internet verbunden sein.</>,
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
