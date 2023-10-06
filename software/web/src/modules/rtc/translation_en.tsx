
/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "rtc": {
        "navbar": {
            "rtc": "Real-Time Clock"
        },
        "content": {
            "rtc": "Real-Time Clock",
            "clock": "Time",
            "time": "Time",
            "mday": "Day",
            "wday": "Weekday",
            "hour": "Hour",
            "minute": "Minute",
            "monday": "Monday",
            "tuesday": "Tuesday",
            "wednesday": "Wednesday",
            "thursday": "Thursday",
            "friday": "Friday",
            "saturday": "Saturday",
            "sunday": "Sunday",
            "every": "Every",
            "cron_translation_function": /*SFN*/(mday: number, wday: number, hour: number, minute: number) => {
                const wdays = [
                    "Sundays",
                    "Mondays",
                    "Tuesdays",
                    "Wednesdays",
                    "Thursdays",
                    "Fridays",
                    "Saturdays",
                    "Sundays"
                ];

                const date = new Date();

                let ret = "";
                if (mday != -1) {
                    ret += "Every " + mday + "th of the Month";
                } else if (wday != -1) {
                    ret += wdays[wday];
                } else {
                    ret += "Every day";
                }

                if (hour != -1 && minute != -1) {
                    date.setMinutes(minute);
                    date.setHours(hour);
                    ret += " at " + date.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
                } else if (hour != -1) {
                    date.setHours(hour)
                    ret += ", every minute from " + date.toLocaleTimeString([], { hour: "2-digit" });
                    date.setHours(hour + 1);
                    ret += " until " + date.toLocaleTimeString([], { hour: "2-digit" });
                } else if (minute != -1) {
                    ret += ", every hour at minute " + minute;
                } else {
                    ret += ", every minute";
                }

                return ret + ", ";
            }/*NF*/,
            "live_date": "Current time on real-time clock",
            "set_time": "Set time",
            "enable_auto_sync": "Automatic synchronization",
            "auto_sync_desc": "Automatically sets the real-time clock's time when loading the web interface and NTP is not synchronized."
        },
        "script": {
            "save_failed": "Failed to save the real-time clock settings.",
            "reboot_content_changed": "real-time clock settings"
        }
    }
}
