/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "solar_forecast": {
        "status": {},
        "navbar": {
            "solar_forecast": "PV-Ertragsprognose"
        },
        "content": {
            "solar_forecast": "PV-Ertragsprognose",
            "enable_solar_forecast": "PV-Ertragsprognose aktiviert",
            "enable_solar_forecast_muted": /*FFN*/(api_url: string) => <>Die Prognose wird von <a href={api_url}>{api_url.replace("https://", "").replace(/\/$/, "")}</a> abgefragt.</>/*NF*/,
            "planes": "PV-Flächen",
            "plane": "PV-Fläche",
            "no_data": "Keine Daten vorhanden!",
            "loading": "Lade Daten...",
            "plane_config_name": "Anzeigename",
            "plane_config_latitude": "Breitengrad",
            "plane_config_latitude_muted": "z.B. 51,8846°",
            "plane_config_longitude": "Längengrad",
            "plane_config_longitude_muted": "z.B. 8,6251°",
            "plane_config_declination": "Neigung",
            "plane_config_declination_muted": "0° horizontal bis 90° vertikal",
            "plane_config_azimuth": "Azimut",
            "plane_config_azimuth_muted": "-180° = Norden, -90° = Osten, 0° = Süden, 90° = Westen, 180° = Norden",
            "plane_config_kwp": "Peak-Nennleistung",
            "plane_config_kwp_muted": "Peak-Leistung der PV-Fläche",
            "add_plane_config_title": "PV-Fläche hinzufügen",
            "add_plane_config_count": /*SFN*/(x: number, max: number) => x + " von " + max + " PV-Flächen konfiguriert"/*NF*/,
            "add_plane_config_done": "Alle PV-Flächen hinzugefügt (6 von 6)",
            "edit_plane_config_title": "PV-Fläche bearbeiten",
            "solar_forecast_desc": <>Lädt automatisch die PV-Ertragsprognose herunter. Für diese Funktion muss {__("the_device")} Internetzugriff haben.</>,
            "table_name": "Name",
            "table_azimuth": "Azimut",
            "table_declination": "Neigung",
            "table_kwp": "kWp",
            "table_latitude": "Breitengrad",
            "table_longitude": "Längengrad",
            "time": "Uhrzeit",
            "solar_forecast_chart_heading": "Status",
            "solar_forecast_now_label": "PV-Ertragsprognose ab jetzt",
            "solar_forecast_days_label": "PV-Ertragsprognose",
            "solar_forecast_today_label": "Heute",
            "solar_forecast_today_label_muted": "00:00 bis 23:59",
            "solar_forecast_tomorrow_label": "Morgen",
            "solar_forecast_tomorrow_label_muted": "00:00 bis 23:59",
            "time_to": "bis",
            "unknown_not_yet": "Unbekannt (PV-Ertragsprognose wurde noch nicht abgefragt)",
            "rate_limit_label": "Abfragebegrenzung",
            "rate_limit_label_muted": "Wie oft darf die PV-Ertragsprognose von der API abgefragt werden (in einem 2-Stunden-Intervall)",
            "remaining_requests_label": "Verbleibende Abfragen",
            "remaining_requests_label_muted": "Wie oft darf die PV-Ertragsprognose von der API in diesem Intervall noch abgefragt werden",
            "next_api_call_label": "Nächster API-Aufruf",
            "next_api_call_label_muted": "Wann wird die PV-Ertragsprognose das nächste mal abgefragt",
            "unknown": "Unbekannt",
            "not_set_for_this_plane": "PV-Ertragsprognose für diese Fläche wurde noch nicht abgefragt.",
            "address_of_pv_plane": "Adresse der PV-Fläche:",
            "last_update_attempt": "Letzter Abfrageversuch:",
            "last_successful_update": "Letzte erfolgreiche Abfrage:",
            "next_update": "Nächste Abfrage:",
            "next_update_in": "Nächste Abfrage in",
            "next_update_in_help": "Die PV-Ertragsprognose darf zwölf mal in einem Intervall von zwei Stunden abgefragt werden. Jede Fläche wird einzeln abgefragt.",
            "remaining_queries": /*SFN*/(x: number, max: number) => "(verbleibende Abfragen: " + x + " von " + max + ")"/*NF*/,
            "not_yet_queried": "Noch nicht abgefragt"
        },
        "script": {
            "power": "Leistung",
            "save_failed": "Speichern der PV-Ertragsprognose-Einstellungen fehlgeschlagen"
        }
    }
}
