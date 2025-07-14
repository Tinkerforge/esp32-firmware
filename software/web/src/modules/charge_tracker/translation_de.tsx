/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
let x = {
    "charge_tracker": {
        "status": {
            "last_charges": "Letzte Ladevorgänge",
            "current_charge": "Laufender Ladevorgang"
        },
        "navbar": {
            "charge_tracker": "Ladetracker"
        },
        "content": {
            "charge_tracker": "Ladetracker",
            "charge_log_file": "Ladelog",
            "download": "Ladelog herunterladen",
            "download_desc": "",
            "download_btn": "Ladelog als CSV herunterladen",
            "download_btn_pdf": "Ladelog als PDF herunterladen",

            "pdf_letterhead": "PDF-Briefkopf",
            "pdf_letterhead_muted": "",

            "tracked_charges": "Aufgezeichnete Ladevorgänge",
            "tracked_charges_muted": "",
            "first_charge_timestamp": "Ältester aufgezeichneter Ladevorgang",
            "first_charge_timestamp_muted": "",
            "user_filter": "Benutzerfilter",
            "user_filter_muted": "heruntergeladene Datei wird nur Ladevorgänge des ausgewählten Benutzers beinhalten",

            "date_filter": "Zeitfilter",
            "date_filter_muted": "heruntergeladene Datei wird nur Ladevorgänge in der ausgewählten Zeitspanne beinhalten",
            "from": "von",
            "to": "bis",
            "last_charges": "Letzte Ladevorgänge",
            "last_charges_desc": "bis zu 30 Ladevorgänge werden angezeigt",
            "remove": "Aufgezeichnete Ladevorgänge löschen",
            "remove_desc": "",
            "remove_btn": "Ladevorgänge löschen",

            "charge_tracker_remove_modal_text": <>Alle aufgezeichneten Ladevorgänge und der Benutzernamen-Verlauf werden gelöscht. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Danach wird {__("the_device")} neugestartet.</>,
            "abort_remove": "Abbrechen",
            "confirm_remove": "Löschen",
            "price": "Strompreis",
            "price_invalid": "Der Strompreis muss in Cent pro kWh, nicht in Euro pro kWh angegeben werden.",
            "price_not_dynamic_yet": /*FFN*/(dap_enabled: boolean) => <>
                Ladevorgänge werden immer mit dem hier konfigurierten Strompreis aufgezeichnet.<br/>
                {dap_enabled ?
                    <>Der dynamische Strompreis wird <strong>nicht</strong> aufgezeichnet!</>:
                    <>Ein dynamischer Strompreis (momentan nicht konfiguriert) wird <strong>nicht</strong> aufgezeichnet.</>}<br/>
                <>Es wird immer die gesamte ins Fahrzeug geladene Energie aufgezeichnet. Es wird <strong>nicht</strong> nach
                  Herkunft (Netzanschluss, Batteriespeicher, PV, etc.) unterschieden!</>
            </>/*NF*/,

            "file_type": "Dateiformat",
            "file_type_muted": "",
            "file_type_pdf": "PDF",
            "file_type_csv": "CSV",

            "csv_flavor": "CSV-Format",
            "csv_flavor_muted": "",
            "csv_flavor_excel": "Excel-kompatibel (; als Feldtrenner, Windows-1252-kodiert)",
            "csv_flavor_rfc4180": "Nach RFC4180 (, als Feldtrenner, UTF-8-kodiert)",

            "send_file_type": "Dateityp zum Senden",
            "enable_send": "Remote-Senden aktivieren",
            "send_user": "Benutzer zum Senden",
            "send_user_placeholder": "Fernzugriffsbenutzer",

            "user_filter_label": "Benutzerfilter",
            "file_type_label": "Dateityp",
            "english_label": "Englisch",
            "letterhead_label": "Briefkopf",
            "target_user": "Fernzugriffsbenutzer",
            "charge_log_send_config": "E-Mail-Versandkonfiguration",
            "charge_log_send_add_modal_title": "E-Mail-Versandkonfiguration hinzufügen",
            "charge_log_send_add_message": /*SFN*/ (have: number, max: number) => `${have} von ${max} Benutzern konfiguriert` /*NF*/
        },
        "automation": {
            "charge_tracker_reset": "Aufgezeichnete Ladevorgänge löschen",
            "automation_action_text": "lösche alle aufgezeichnete Ladevorgänge und starte neu."
        },
        "script": {
            "tracked_charge_count": /*SFN*/ (tracked: number, max_: number) => `${tracked} von maximal ${max_} (${Math.floor(100 * tracked / max_)} %)` /*NF*/,
            "csv_header_start": "Startzeit",
            "csv_header_display_name": "Anzeigename",
            "csv_header_username": "Benutzername",
            "csv_header_energy": "Geladene Energie in kWh",
            "csv_header_duration": "Ladedauer in s",
            "csv_header_meter_start": "Zählerstand Start",
            "csv_header_meter_end": "Zählerstand Ende",
            "csv_header_price": "Ladekosten in €; Arbeitspreis ",

            "unknown_user": "Unbekannter Benutzer",
            "unknown_users": "Unbekannte Benutzer",
            "deleted_user": "Gelöschter Benutzer",
            "deleted_users": "Gelöschte Benutzer",
            "all_users": "Alle Benutzer",
            "unknown_charge_start": "Unbekannter Startzeitpunkt",

            "remove_failed": "Löschen der aufgezeichneten Ladevorgänge fehlgeschlagen",
            "remove_init": "Lösche aufgezeichnete Ladevorgänge und starte neu...",
            "download_charge_log_failed": "Herunterladen der aufgezeichneten Ladevorgänge fehlgeschlagen",
            "download_usernames_failed": "Herunterladen der verwendeten Benutzernamen in aufgezeichneten Ladevorgängen fehlgeschlagen",

            "save_failed": "Speichern der Ladetracker-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "Ladetracker-Einstellungen"
        }
    }
}
