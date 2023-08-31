/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "charge_manager": {
        "status": {
            "charge_manager": null,
            "not_configured": "Deaktiviert",
            "manager": "Aktiviert",
            "error": "Fehler",
            "managed_boxes": "Kontrollierte Wallboxen",
            "available_current": "Verfügbarer Strom"
        },
        "navbar": {
            "charge_manager": null
        },
        "content": {
            "charge_manager": null,
            "enable_charge_manager": "Lastmanagementmodus",
            "enable_charge_manager_muted": <><a href="{{{manual_url}}}">siehe Betriebsanleitung für Details</a></>,
            "enable_watchdog": "Watchdog aktiviert",
            "enable_watchdog_muted": "nur bei API-Benutzung aktivieren (für den normalen Lastmanagement-Betrieb nicht notwendig!)",
            "enable_watchdog_desc": "Setzt den verfügbaren Strom auf die Voreinstellung, wenn er nicht spätestens alle 30 Sekunden aktualisiert wurde",
            "verbose": "Stromverteilungsprotokoll aktiviert",
            "verbose_desc": "Erzeugt Einträge im Ereignis-Log, wenn Strom umverteilt wird",
            "default_available_current": "Voreingestellt verfügbarer Strom",
            "default_available_current_muted": "wird nach Neustart des Lastmanagers verwendet",
            "default_available_current_invalid": "Der voreingestellt verfügbare Strom darf höchstens so groß sein wie der maximale Gesamtstrom.",
            "maximum_available_current": "Maximaler Gesamtstrom",
            "maximum_available_current_muted": "Maximal zulässiger Ladestrom aller Wallboxen in Summe. Dies ist üblicherweise die Nennbelastbarkeit der gemeinsamen Zuleitung.",
            "minimum_current_auto": "Automatischer minimaler Ladestrom",
            "minimum_current_auto_desc": "Minimaler Ladestrom wird abhängig vom gewählten Fahrzeugmodell eingestellt.",
            "minimum_current_vehicle_type": "Fahrzeugmodell",
            "minimum_current_vehicle_type_other": "Standard",
            "minimum_current_vehicle_type_zoe": "Renault ZOE R135, ZOE R110 oder Twingo Z.E.",
            "minimum_current": "Minimaler Ladestrom",
            "minimum_current_muted": "geringster Ladestrom, der vom Fahrzeug unterstützt wird",
            "minimum_current_1p": "Minimaler einphasiger Ladestrom",
            "minimum_current_1p_muted": "geringster Ladestrom, der vom Fahrzeug für einphasiges Laden unterstützt wird",
            "minimum_current_3p": "Minimaler dreiphasiger Ladestrom",
            "minimum_current_3p_muted": "geringster Ladestrom, der vom Fahrzeug für dreiphasiges Laden unterstützt wird",

            "requested_current_margin": "Spielraum des Phasenstroms",
            "requested_current_margin_muted": "",
            "requested_current_threshold": "Länge der Startphase",
            "requested_current_threshold_muted": "Wallboxen mit einem Stromzähler, der Phasenströme misst, werden nach Ablauf der Startphase auf den größten Phasenstrom plus den konfigurierten Spielraum limitiert. Damit kann der verfügbare Strom effizienter auf mehrere Wallboxen verteilt werden.",

            "configuration_mode": "Experteneinstellungen",
            "configuration_mode_muted": "",

            "table_charger_name": "Anzeigename",
            "table_charger_host": "Host",

            "add_charger_title": "Wallbox hinzufügen",
            "add_charger_name": "Anzeigename",
            "add_charger_host": "IP-Adresse oder Hostname",
            "add_charger_found": "Gefundene Wallboxen",
            "add_charger_count": /*SFN*/(x: number, max: number) => x + " von " + max + " Wallboxen konfiguriert"/*NF*/,

            "edit_charger_title": "Wallbox bearbeiten",
            "edit_charger_name": "Anzeigename",
            "edit_charger_host": "IP-Adresse oder Hostname",

            "multi_broadcast_modal_title": "Reservierte, Multi- oder Broadcast IP-Adresse(n) gefunden",
            "multi_broadcast_modal_body": "Folgende reservierte, Multi- oder Broadcast-IP-Adressen wurden gefunden: ",
            "multi_broadcast_modal_body_end": "Sind sie sicher, dass sie die Einstellungen übernehmen wollen?",
            "multi_broadcast_modal_save": "Übernehmen",
            "multi_broadcast_modal_cancel": "Abbrechen",

            "scan_error_1": "Firmwareversion inkompatibel",
            "scan_error_2": "Lastmanagement deaktiviert",

            "mode_disabled": "Deaktiviert",
            "mode_manager": "Lastmanager",
            "mode_managed": "Fremdgesteuert",

            "managed_boxes": "Kontrollierte Wallboxen",

            "host_exists": "Host existiert bereits",

            "set_charge_manager": "Lastmanagement"
        },
        "script": {
            "charge_state_0": "Kein Fahrzeug angeschlossen",
            "charge_state_1": "Warte auf Benutzerfreigabe",
            "charge_state_2": "Warte auf Manager-Freigabe",
            "charge_state_3": "Laden freigegeben",
            "charge_state_4": "Lädt",
            "charge_state_5": "Fehler",
            "charge_state_6": "Laden abgeschlossen",

            "charge_error_type_management": "Managementfehler",
            "charge_error_type_client": "Wallbox-Fehler",

            "charge_error_0": "OK",
            "charge_error_1": "Kommunikationsfehler",
            "charge_error_2": "Paketheader ungültig oder Firmware inkompatibel",
            "charge_error_3": "Lastmanagement deaktiviert",
            "charge_error_128": "Wallbox nicht erreichbar",
            "charge_error_129": "Ladecontroller nicht erreichbar",
            "charge_error_130": "Ladecontroller reagiert nicht",

            "charge_error_192": "OK",
            "charge_error_194": "Schalterfehler",
            "charge_error_195": "DC-Fehlerstromschutzfehler",
            "charge_error_196": "Schützfehler",
            "charge_error_197": "Fahrzeug-Kommunikationsfehler",

            "charge_state_blocked_by_other_box": "Blockiert",
            "charge_state_blocked_by_other_box_details": "Fehler bei anderer Wallbox",

            "ampere_allocated": "A zugeteilt",
            "ampere_supported": "A unterstützt",

            "last_update_prefix": "Gestört seit ",
            "last_update_suffix": "",
            "save_failed": "Speichern der Lastmanager-Konfiguration fehlgeschlagen",
            "set_available_current_failed": "Setzen des verfügbaren Stroms fehlgeschlagen",

            "reboot_content_changed": "Lastmanagement-Einstellungen",

            "scan_failed": "Scan fehlgeschlagen",

            "mode_explainer_0": "Dieser WARP Charger ist nicht Teil eines Lastmanagement-Verbunds mit anderen WARP Chargern und/oder einem WARP Energy Manager.",
            "mode_explainer_1": "Dieser WARP Charger ist Teil eines Lastmanagement-Verbunds mit anderen WARP Chargern und/oder einem WARP Energy Manager. Ein anderes Gerät steuert diesen Lastmanagement-Verbund, um sicherzustellen, dass nie mehr als der verfügbare Strom bezogen wird.",
            "mode_explainer_2": "Dieser WARP Charger steuert einen Lastmanagement-Verbund mit anderen WARP Chargern um sicherzustellen, dass nie mehr als der verfügbare Strom bezogen wird."
        }
    }
}
