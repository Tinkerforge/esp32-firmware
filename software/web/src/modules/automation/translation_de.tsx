/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "automation": {
        "navbar": {
            "automation": "Automatisierung"
        },
        "automation": {
            "cron": "Zeitpunkt",
            "cron_time": "Uhrzeit",
            "cron_mday": "Tag",
            "cron_monday": "Montag",
            "cron_tuesday": "Dienstag",
            "cron_wednesday": "Mittwoch",
            "cron_thursday": "Donnerstag",
            "cron_friday": "Freitag",
            "cron_saturday": "Samstag",
            "cron_sunday": "Sonntag",
            "cron_every_day": "Täglich",
            "cron_every_hour": "Stündlich",
            "cron_every_minute": "Minütlich",
            "cron_weekdays": "Wochentags",
            "cron_weekends": "Am Wochenende",
            "cron_month_end": "Monatsende",
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
                        time = <>, zur vollen Stunde</>;
                    } else {
                        time = <>, <b>{minute}</b> {minute == 1 ? "Minute" : "Minuten"} nach jeder vollen Stunde</>;
                    }
                } else {
                    time = <>, minütlich</>
                }

                return <>{day}{time}, </>;
            }/*NF*/,

            "print_action": "Gebe im Ereignislog aus",
            "print_action_message": "Nachricht",
            "print_action_text": /*FFN*/(message: string) => <>zeige "{message}" im <b>Ereignislog</b> an.</> /*NF*/

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
