/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "ove_r37": {
        "navbar": {
            "ove_r37": "OVE R 37"
        },
        "content": {
            "ove_r37": "OVE-Richtlinie R 37",
            "enabled": "OVE R 37 aktiviert",
            "enabled_muted": "Netzstützung gemäß OVE R 37 (Österreich)",
            "enabled_desc": "Aktiviert die OVE-R-37-Netzstützungsfunktionen (Unterspannungsauslösung, Zuschaltbedingungen, Symmetrie).",
            "undervoltage_threshold": "Unterspannungs-Auslöseschwelle",
            "undervoltage_threshold_muted": "(bei 230 V Nennspannung)",
            "undervoltage_threshold_help": <>
                <p>Die Ladung wird unterbrochen (Auslösung), wenn eine Phasenspannung länger als die Beobachtungszeit unter dieser Schwelle bleibt (OVE R 37 §5.9.8). Die Schwelle wird in 1/1000 pu der Nennspannung von 230 V angegeben, d.h. 800 = 0,80 pu ≈ 184 V.</p>
            </>,
            "undervoltage_observation_time": "Unterspannungs-Beobachtungszeit",
            "undervoltage_observation_time_muted": "Zeit unterhalb der Schwelle bis zur Auslösung (Standard 3000 ms)",
            "reconnect_wait_time": "Wiederzuschalt-Wartezeit",
            "reconnect_wait_time_muted": "Wartezeit bis zur Wiederaufnahme der Ladung nach einer Auslösung (0…300 s, Standard 60 s)",
            "start_delay": "Ladestart-Verzögerung",
            "start_delay_muted": "Verzögerung des Ladestarts (0…300 s, Standard 0 s)",
            "password": "Elektriker-Passwort",
            "password_muted": "Erforderlich, um die OVE-R-37-Konfiguration zu ändern",
            "password_placeholder": "Elektriker-Passwort",
            "password_required": "Zum Speichern der Konfiguration ist das Elektriker-Passwort erforderlich.",
            "state_disabled": "Deaktiviert",
            "state_normal": "Normal",
            "state_tripped": "Ausgelöst",
            "state_wait": "Warten auf Wiederzuschaltung",
            "state_ramp": "Hochlauf",
            "state_boot": "Warten nach Neustart",
            "trip_none": "Keine",
            "trip_undervoltage": "Unterspannung",
            "trip_overvoltage": "Überspannung",
            "trip_frequency": "Frequenz"
        },
        "status": {
            "state": "OVE-R-37-Zustand",
            "trip_reason": "Auslösegrund",
            "voltage_in_range": "Spannung im Bereich",
            "voltage_in_range_muted": "0,9-1,1 pu (207-253 V)",
            "voltage_in_range_help": <>
                <p>Ob alle angeschlossenen Phasenspannungen innerhalb des Betriebsbands von 0,9-1,1 pu liegen (OVE R 37 §5.9.9), also 207-253 V bei 230 V Nennspannung.</p>
                <p>Dies ist unabhängig von der Unterspannungs-Auslöseschwelle: Eine Spannung kann außerhalb dieses Bereichs liegen (z.B. 197 V), ohne auszulösen. Die Ladung wird erst unterbrochen, wenn die Spannung für die Beobachtungszeit unter die Auslöseschwelle fällt.</p>
            </>,
            "frequency_in_range": "Frequenz im Bereich",
            "frequency_in_range_muted": "49,9–50,1 Hz",
            "frequency_in_range_help": <>
                <p>Ob die Netzfrequenz innerhalb des Wiederzuschalt-Fensters von 49,9-50,1 Hz liegt (OVE R 37 §5.7.4.2). Dieses Band bestimmt, ob nach einer Auslösung wieder zugeschaltet werden darf.</p>
                <p>Die Frequenz selbst löst eine laufende Ladung nie aus (Ladelasten haben noch keine aktive Frequenzstützungs-Anforderung).</p>
            </>,
            "yes": "Ja",
            "no": "Nein",
            "no_data": "Keine Daten"
        },
        "script": {
            "save_failed": "Speichern der OVE-R-37-Konfiguration fehlgeschlagen"
        }
    }
}
