/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
import { __, removeUnicodeHacks } from "../../ts/translation";
let x = {
    "system": {
        "status": {
            "system": "System",
            "last_reset": /*FFN*/(reason: number, version: string) => {
                let reason_table = [
                    "wegen unbekannter Ursache",
                    "wegen dem Einschalten der Stromversorgung",
                    "mittels externem Eingang",
                    "mittels Software-Reset",
                    "wegen eines Software-Absturzes",
                    "wegen des Interrupt-Watchdog",
                    "wegen des Task-Watchdogs",
                    "wegen einem anderem Watchdog",
                    "nach Verlassen des Deep-Sleep-Modus",
                    "wegen eines Brownouts",
                    "mittels SDIO",
                    "mittels USB-Peripheral",
                    "mittels JTAG",
                    "wegen einem eFuse-Fehler",
                    "wegen einer Power-Glitch-Erkennung",
                    "wegen einer blockierten CPU",
                ];

                let reason_str = reason_table[reason];

                if (reason_str === undefined) {
                    reason_str = reason_table[0];
                }

                let result = [<>__("The_device") wurde {reason_str} unerwartet neu gestartet.</>];

                if (options.SUPPORT_EMAIL.length > 0) {
                    result.push(<> Bitte einen <a href="#event_log">Debug-Report</a> herunterladen und an <a href={removeUnicodeHacks(`mailto:${options.SUPPORT_EMAIL}?subject=${options.PRODUCT_NAME} mit Firmware ${version} wurde unerwartet neu gestartet`)}>{options.SUPPORT_EMAIL}</a> schicken.</>);
                }

                return <>{result}</>;
            }/*NF*/,
            "hide_last_reset_warning_failed": "Entfernen der Warnung fehlgeschlagen"
        },
        "navbar": {
            "system": "Einstellungen"
        },
        "content": {
            "system": "System",

            "factory_reset_desc": null,
            "factory_reset": "Zurücksetzen auf Werkszustand",
            "factory_reset_modal_text": null,

            "config_reset_desc": null,
            "config_reset": "Konfiguration zurücksetzen",

            "config_reset_modal_text": null,
            "confirm_config_reset": "Konfiguration zurücksetzen",

            "abort_reset": "Abbrechen",
            "confirm_factory_reset": "Zurücksetzen auf Werkszustand",
            "reboot": "Neu starten",
            "reboot_desc": null,

            "system_language": "Systemsprache",
            "system_language_help": "Setzt die Systemsprache auf die angegebene Sprache. Wenn \"Browser-Sprache automatisch erkennen\" deaktiviert ist, dann werden die Spracheinstellungen des Browsers ignoriert und das Webinterface nutzt auch die angegebene Systemsprache.",
            "system_language_warning": /*SFN*/(browser_locale: string, browser_language_name: string, web_interface_language: string, web_interface_language_name: string) =>
                `Die Sprache der Benutzeroberfläche des Browsers ist auf ${browser_language_name} [${browser_locale}] eingestellt. Zahlen, Uhrzeiten und Daten werden in ${browser_language_name} angezeigt. Während die Sprache des Webinterfaces auf ${web_interface_language_name} [${web_interface_language}] eingestellt ist. Texte werden in ${web_interface_language_name} angezeigt.`/*NF*/,
            "german": "Deutsch",
            "english": "Englisch",
            "detect_browser_language": "Browser-Sprache automatisch erkennen"
        },
        "script": {
            "factory_reset_init": "Formatiere Konfigurationspartition und starte neu...",
            "factory_reset_error": "Zurücksetzen auf Werkszustand fehlgeschlagen",

            "config_reset_init": "Setze Konfiguration zurück und starte neu...",
            "config_reset_error": "Zurücksetzen der Konfiguration fehlgeschlagen",

            "save_failed": "Speichern der Systemeinstellungen fehlgeschlagen"
        }
    }
}
