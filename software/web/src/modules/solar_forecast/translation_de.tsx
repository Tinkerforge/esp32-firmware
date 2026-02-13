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
            "source": "Quelle",
            "source_forecast_service": "Automatisch über Prognosedienst",
            "source_forecast_service_desc": /*FFN*/(api_url: string) => <>Die Prognose wird von <a href={api_url}>{api_url.replace("https://", "").replace(/\/$/, "")}</a> abgefragt.</>/*NF*/,
            "source_push": "Push über API",
            "source_push_desc": <>Im Push-Modus werden Prognosedaten über den API-Endpunkt <a href="https://docs.warp-charger.com/docs/interfaces/mqtt_http/api_reference/solar_forecast" target="_blank">solar_forecast/planes/X/forecast_update</a> bereitgestellt.</>,
            "planes": "PV-Flächen",
            "no_data": "Keine Daten vorhanden",
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
            "add_plane_config_message": /*SFN*/(have: number, max: number) => `${have} von ${max} PV-Flächen konfiguriert`/*NF*/,
            "edit_plane_config_title": "PV-Fläche bearbeiten",
            "solar_forecast_desc": "PV-Ertragsprognose aktivieren.",
            "table_name": "Name",
            "table_azimuth": "Azimut",
            "table_declination": "Neigung",
            "table_kwp": "kWp",
            "table_latitude": "Breitengrad",
            "table_longitude": "Längengrad",
            "time": "Uhrzeit",
            "solar_forecast_now": "PV-Ertragsprognose ab jetzt",
            "solar_forecast_days": "PV-Ertragsprognose",
            "solar_forecast_days_muted": "00:00 bis 23:59",
            "solar_forecast_today": "Heute",
            "solar_forecast_tomorrow": "Morgen",
            "time_to": "bis",
            "unknown": "Unbekannt",
            "not_set_for_this_plane": "PV-Ertragsprognose für diese Fläche wurde noch nicht abgefragt.",
            "server_error_404": "Prognoseserver meldet 404: Ungültiger Breiten- oder Längengrad",
            "server_error_422": "Prognoseserver meldet 422: Ungültige Neigung, Azimut oder Peak-Nennleistung",
            "server_error_other": "Prognoseserver meldet Fehler",
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
