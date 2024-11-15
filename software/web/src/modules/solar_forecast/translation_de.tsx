/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "solar_forecast": {
        "status": {},
        "navbar": {
            "solar_forecast": "Solarprognose"
        },
        "content": {
            "solar_forecast": "Solarprognose",
            "enable_solar_forecast": "Solarprognose aktiviert",
            "planes": "PV-Flächen",
            "plane": "PV-Fläche",
            "no_data": "Keine Daten vorhanden!",
            "loading": "Lade Daten...",
            "plane_config_name": "Name",
            "plane_config_name_muted": "Anzeigename der PV-Fläche",
            "plane_config_latitude": "Breitengrad",
            "plane_config_latitude_muted": "z.B. 51,8847°",
            "plane_config_longitude": "Längengrad",
            "plane_config_longitude_muted": "z.B. 8,6225°",
            "plane_config_declination": "Neigung",
            "plane_config_declination_muted": "0° horizontal bis 90° vertikal",
            "plane_config_azimuth": "Azimut",
            "plane_config_azimuth_muted": "-180° = Norden, -90° = Osten, 0° = Süden, 90° = Westen, 180° = Norden",
            "plane_config_kwp": "kWp",
            "plane_config_kwp_muted": "Peak-Leistung der PV-Fläche",
            "add_plane_config_title": "Neue PV-Fläche",
            "add_plane_config_count": /*SFN*/(x: number, max: number) => x + " von " + max + " PV-Flächen konfiguriert"/*NF*/,
            "add_plane_config_done": "Alle PV-Flächen hinzugefügt (6 von 6)",
            "edit_plane_config_title": "PV-Fläche bearbeiten",
            "solar_forecast_desc": "Lädt automatisch die Solarprognose herunter. Für diese Funktion muss das Gerät Zugriff auf das Internet haben.",
            "solar_forecast_muted": "",
            "table_name": "Name",
            "table_azimuth": "Azimut",
            "table_declination": "Neigung",
            "table_kwp": "kWp",
            "table_latitude": "Breitengrad",
            "table_longitude": "Längengrad",
            "time": "Zeit",
            "solar_forecast_chart_heading": "Status",
            "solar_forecast_now_label": "Solarprognose ab jetzt",
            "solar_forecast_today_label": "Heute",
            "solar_forecast_today_label_muted": "00:00 bis 23:59",
            "solar_forecast_tomorrow_label": "Morgen",
            "solar_forecast_tomorrow_label_muted": "00:00 bis 23:59",
            "time_to": "bis",
            "unknown_not_yet": "Unbekannt (Solarprognose wurde noch nicht abgefragt)",
            "rate_limit_label": "Abfragebegrenzung",
            "rate_limit_label_muted": "Wie oft darf die Solarprognose von der API abgefragt werden (in einem 2-Stunden-Intervall)",
            "remaining_requests_label": "Verbleibende Abfragen",
            "remaining_requests_label_muted": "Wie oft darf die Solarprognose von der API in diesem Intervall noch abgefragt werden",
            "next_api_call_label": "Nächster API-Aufruf",
            "next_api_call_label_muted": "Wann wird die Solarprognose das nächste mal abgefragt",
            "unknown": "Unbekannt",
            "not_set_for_this_plane": "Solarprognose für diese Fläche wurde noch nicht abgefragt.",
            "address_of_pv_plane": "Adresse der PV-Fläche:",
            "last_update_attempt": "Letzter Abfrageversuch:",
            "last_successful_update": "Letzte erfolgreiche Abfrage:",
            "next_update": "Nächste Abfrage:",
            "resolution": "Genauigkeit von ca. 1km ist OK"
        },
        "script": {
            "power": "Leistung",
            "save_failed": "Speichern der Solarprognose-Einstellungen fehlgeschlagen",
            "reboot_content_changed": "Solarprognose-Einstellungen"
        }
    }
}
