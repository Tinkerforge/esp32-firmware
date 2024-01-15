/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "automation": {
        "navbar": {
            "automation": "Automatisierung"
        },
        "automation": {
            "clock": "Zeitpunkt",
            "time": "Uhrzeit",
            "mday": "Tag",
            "monday": "Montag",
            "tuesday": "Dienstag",
            "wednesday": "Mittwoch",
            "thursday": "Donnerstag",
            "friday": "Freitag",
            "saturday": "Samstag",
            "sunday": "Sonntag",
            "every": "Täglich",
            "weekdays": "Wochentags",
            "weekends": "Am Wochenende",
            "month_end": "Monatsende",
            "cron_translation_function": /*FFN*/(mday: number, wday: number, hour: number, minute: number) => {
                const wdays = [
                    "Sonntags",
                    "Montags",
                    "Dienstags",
                    "Mittwochs",
                    "Donnerstags",
                    "Freitags",
                    "Samstags",
                    "Sonntags"
                ];

                let day = <></>;
                if (mday != -1) {
                    if (mday == 32) {
                        day = <>Am <b>letzten Tag</b> des Monats</>;
                    } else {
                        day = <>Jeden <b>{mday}.</b> des Monats</>;
                        if (mday >= 29) {
                            day = <>{day} (wird nur in Monaten mit {mday} Tagen ausgeführt)</>;
                        }
                    }
                } else if (wday == 8) {
                    day = <><b>Wochentags</b></>;
                } else if (wday == 9) {
                    day = <><b>Am Wochenende</b></>;
                } else if (wday != -1) {
                    day = <><b>{wdays[wday]}</b></>;
                }
                else {
                    day = <>Täglich</>;
                }

                const date = new Date();
                let time = <></>;
                if (hour != -1 && minute != -1) {
                    date.setMinutes(minute);
                    date.setHours(hour);
                    let time_string = date.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
                    if (!time_string.endsWith("AM") && !time_string.endsWith("PM")) {
                        time_string += " Uhr";
                    }
                    time = <> um <b>{time_string}</b></>;
                } else if (hour != -1) {
                    date.setHours(hour);
                    const start = date.toLocaleTimeString([], { hour: "2-digit" });
                    date.setHours(hour + 1);
                    const end = date.toLocaleTimeString([], { hour: "2-digit" });
                    time = <>, minütlich von <b>{start}</b> bis <b>{end}</b></>
                } else if (minute != -1) {
                    if (minute == 0) {
                        time = <>, stündlich</>;
                    } else {
                        time = <>, stündlich zur <b>{minute}.</b> Minute</>;
                    }
                } else {
                    time = <>, minütlich</>
                }

                return <>{day}{time}, </>;
            }/*NF*/
        },
        "content": {
            "automation": "Automatisierung",
            "rule": "Regel",
            "condition": "Bedingung",
            "action": "Aktion",
            "add_rule_title": "Regel hinzufügen",
            "add_rule_count": /*SFN*/(count: number, max: number) => count + " von " + max + " Regeln konfiguriert"/*NF*/,
            "edit_rule_title": "Regel bearbeiten",
            "select": "Auswählen..."
        }
    }
}
