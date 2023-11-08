/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "rtc": {
        "navbar": {
            "rtc": "Echtzeituhr"
        },
        "content": {
            "rtc": "Echtzeituhr",
            "live_date": "Aktuelle Zeit der Echtzeituhr",
            "set_time": "Zeit setzen",
            "enable_auto_sync": "Automatische Synchronisierung",
            "auto_sync_desc": "Setzt die Zeit der Echtzeituhr automatisch, wenn das Webinterface geladen wird und keine NTP-Synchronisierung besteht."
        },
        "cron": {
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
            "cron_translation_function": /*SFN*/(mday: number, wday: number, hour: number, minute: number) => {
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

                let ret = "";
                if (mday != -1) {
                    if (mday == 32) {
                        ret += "Am letzten Tag des Monats";
                    } else {
                        ret += "Jeden " + mday + ". des Monats";
                        if (mday >= 29) {
                            ret += " (wird nur in Monaten mit " + mday + " Tagen ausgeführt)";
                        }
                    }
                } else if (wday == 8) {
                    ret += "Wochentags";
                } else if (wday == 9) {
                    ret += "Am Wochenende";
                } else if (wday != -1) {
                    ret += wdays[wday];
                }
                else {
                    ret += "Täglich";
                }

                const date = new Date();

                if (hour != -1 && minute != -1) {
                    date.setMinutes(minute);
                    date.setHours(hour);
                    ret += " um " + date.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
                    if (!ret.endsWith("AM") && !ret.endsWith("PM")) {
                        ret += " Uhr";
                    }
                } else if (hour != -1) {
                    date.setHours(hour);
                    ret += ", minütlich von " + date.toLocaleTimeString([], { hour: "2-digit" }) + " bis ";
                    date.setHours(hour + 1);
                    ret += date.toLocaleTimeString([], { hour: "2-digit" });
                } else if (minute != -1) {
                    ret += ", stündlich zur " + minute + ". Minute";
                } else {
                    ret += ", minütlich";
                }

                return ret + ", ";
            }/*NF*/
        },
        "script": {
            "save_failed": "Speichern der Echtzeituhr-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "Echtzeituhr-Einstellungen"
        }
    }
}
