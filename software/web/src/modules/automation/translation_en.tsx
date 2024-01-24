/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "automation": {
        "navbar": {
            "automation": "Automation"
        },
        "automation": {
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
            "every_day": "Daily",
            "every_hour": "Every hour",
            "every_minute": "Every minute",
            "weekdays": "Weekdays",
            "weekends": "Weekends",
            "month_end": "Month End",
            "cron_translation_function": /*FFN*/(mday: number, wday: number, hour: number, minute: number) => {
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
                let day = <></>;
                if (mday != -1) {
                    if (mday == 32) {
                        day = <>On the <b>last day</b> of the month</>;
                    } else {
                        let extension = "th";
                        if (mday == 1 || mday == 21 || mday == 31) {
                            extension = "st";
                        } else if (mday == 2 || mday == 22) {
                            extension = "nd";
                        } else if (mday == 3 || mday == 23) {
                            extension = "rd";
                        }
                        day = <>Every <b>{mday}{extension}</b> of the month</>;
                        if (mday >= 29) {
                            day = <>{day} (only executed in months with {mday} days)</>;
                        }
                    }
                } else if (wday == 8) {
                    day = <><b>Weekdays</b></>;
                } else if (wday == 9) {
                    day = <><b>Weekends</b></>;
                } else if (wday != -1) {
                    day = <><b>{wdays[wday]}</b></>;
                }
                else {
                    day = <>Every day</>;
                }

                const date = new Date();
                let time = <></>;
                if (hour != -1 && minute != -1) {
                    date.setMinutes(minute);
                    date.setHours(hour);
                    let time_string = date.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
                    time = <> at <b>{time_string}</b></>;
                } else if (hour != -1) {
                    date.setHours(hour);
                    const start = date.toLocaleTimeString([], { hour: "2-digit" });
                    date.setHours(hour + 1);
                    const end = date.toLocaleTimeString([], { hour: "2-digit" });
                    time = <>, every minute from <b>{start}</b> until <b>{end}</b></>
                } else if (minute != -1) {
                    if (minute == 0) {
                        time = <>, every hour</>;
                    } else {
                        time = <>, every hour at minute <b>{minute}</b></>;
                    }
                } else {
                    time = <>, every minute</>
                }

                return <>{day}{time}, </>;
            }/*NF*/,

            "print_action": "Print to event log",
            "print_action_message": "Message",
            "print_action_text": /*FFN*/(message: string) => <>Show {message} in the <b>event log</b>.</> /*NF*/
        },
        "content": {
            "automation": "Automation",
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
