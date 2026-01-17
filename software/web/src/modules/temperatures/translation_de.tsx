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
            "temperatures_muted": /*FFN*/(api_url: string) => <>Die Daten werden von <a href={api_url}>{api_url.replace("https://", "").replace(/\/$/, "")}</a> abgerufen.</>/*NF*/,
            "temperatures_desc": <>Automatischer Download von Temperaturvorhersagen für heute und morgen. {__("The_device")} muss mit dem Internet verbunden sein.</>,
            "latitude": "Breitengrad",
            "latitude_muted": "in Grad (z.B. 51,9035)",
            "longitude": "Längengrad",
            "longitude_muted": "in Grad (z.B. 8,6720)",
            "status_section": "Status",
            "today": "Heute",
            "tomorrow": "Morgen",
            "min_temp": "Min",
            "max_temp": "Max",
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
