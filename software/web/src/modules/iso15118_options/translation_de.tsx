/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "iso15118": {
        "content": {
            "experimental_settings": "Experimentelle Fahrzeugkompatibilität",
            "experimental_warning": <><p>Diese experimentellen Einstellungen und ihre API sind instabil und können in zukünftigen Firmware-Versionen geändert oder entfernt werden.</p></>,
            "experimental_unplug": "Die experimentellen Einstellungen sind gesperrt, bis das Fahrzeug als abgesteckt erkannt wurde.",
            "ef_teardown": "E/F-Reset",
            "ef_teardown_desc": "CP-Signal vor AC-Laden zurücksetzen",
            "ef_teardown_help": "Vor dem normalen AC-Laden einen E/F-Reset des Control-Pilot-Signals durchführen. Dies betrifft auch den Wechsel zum AC-Laden nach Autocharge ohne Auslesen des Ladestands.",
            "nonegotiation_autocharge": "Autocharge-Variante",
            "nonegotiation_autocharge_desc": "Protokollaushandlung ablehnen",
            "nonegotiation_autocharge_help": "Bei Autocharge ohne Auslesen des Ladestands die Protokollaushandlung gezielt ablehnen, statt nach der Fahrzeugidentifikation aufzuhören. Dies gilt auch, wenn das Auslesen aus Kompatibilitätsgründen übersprungen wird. Bei aktivierter Neuverbindung gilt dies auch für die zweite Kommunikationsrunde, selbst ohne Autocharge.",
            "nonegotiation_after_soc": "Neuverbindung",
            "nonegotiation_after_soc_desc": "Nach SoC-Auslesen erneut verbinden",
            "nonegotiation_after_soc_help": "Nach der DC-Sitzung zum Auslesen des Ladestands eine neue Kommunikationsrunde versuchen, um den Wechsel zum AC-Laden zu testen. Beim SLAC-Matching ohne Bestätigung abbrechen. Bei aktivierter Autocharge-Variante stattdessen die Verbindung vollständig aufbauen und mit Failed_NoNegotiation antworten.",
            "ignore_soc_compatibility": "SoC immer lesen",
            "ignore_soc_compatibility_desc": "Kompatibilitätssperre ignorieren",
            "ignore_soc_compatibility_help": "Fahrzeugbezogene Kompatibilitätsausschlüsse beim Auslesen des Ladestands ignorieren. Die normale Einstellung Ladestand auslesen muss weiterhin aktiviert sein."
        },
        "script": {
            "experimental_save_failed": "Speichern der experimentellen ISO15118-Einstellungen fehlgeschlagen."
        }
    }
}
