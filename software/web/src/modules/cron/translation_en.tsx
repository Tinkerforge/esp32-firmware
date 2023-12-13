/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "cron": {
        "navbar": {
            "cron": "Automation"
        },
        "cron": {
            "clock": "Time",
            "time": "Time",
            "mday": "Day",
            "monday": "Monday",
            "tuesday": "Tuesday",
            "wednesday": "Wednesday",
            "thursday": "Thursday",
            "friday": "Friday",
            "saturday": "Saturday",
            "sunday": "Sunday",
            "every": "Every",
            "weekdays": "Weekdays",
            "weekends": "Weekends",
            "month_end": "Month End",
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
                    if (mday == 32) {
                        ret += "On the last day of the month";
                    } else {
                        ret += "Every " + mday + "th of the month";
                        if (mday >= 29) {
                            ret += " (only executed in months with " + mday + " days)";
                        }
                    }
                } else if (wday == 8) {
                    ret += "Weekdays";
                } else if (wday == 9) {
                    ret += "Weekends";
                } else if (wday != -1) {
                    ret += wdays[wday];
                }
                else {
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
            }/*NF*/
        },
        "content": {
            "cron": "Automation",
            "rule": "Rule",
            "condition": "Condition",
            "action": "Action",
            "add_rule_title": "Add rule",
            "add_rule_count": /*SFN*/(count: number, max: number) => count + " of " + max + " rules configured"/*NF*/,
            "edit_rule_title": "Edit rule",
            "select": "Select..."
        }
    }
}
