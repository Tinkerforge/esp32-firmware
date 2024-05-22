/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "automation": {
        "navbar": {
            "automation": "Automation"
        },
        "automation": {
            "cron": "Time",
            "cron_time": "Time",
            "cron_mday": "Day",
            "cron_monday": "Monday",
            "cron_tuesday": "Tuesday",
            "cron_wednesday": "Wednesday",
            "cron_thursday": "Thursday",
            "cron_friday": "Friday",
            "cron_saturday": "Saturday",
            "cron_sunday": "Sunday",
            "cron_every_day": "Daily",
            "cron_every_hour": "Every hour",
            "cron_every_minute": "Every minute",
            "cron_weekdays": "Weekdays",
            "cron_weekends": "Weekends",
            "cron_month_end": "Month End",
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

            "http": "HTTP message received",
            "http_method": "Allowed method",
            "http_get": "GET",
            "http_post": "POST",
            "http_put": "PUT",
            "http_post_put": "POST or PUT",
            "http_get_post_put": "GET, POST or PUT",
            "http_url_suffix": "URL suffix",
            "http_payload": "Message",
            "http_match_any": "Accept any message",
            "http_translation_function": /*FFN*/(method: number, url: string, payload: string) => {
                let methods = [
                    "GET",
                    "POST",
                    "PUT",
                    "POST or PUT",
                    "GET, POST or PUT",
                ];

                let payload_str = <></>;
                if (payload.length == 0) {
                    payload_str = <>containing any message</>;
                } else {
                    payload_str = <>containing the message "<b>{payload}</b>"</>;
                }

                let ret = <>If a HTTP {methods[method]} request {payload_str}</>;

                return <>
                    {ret} is received on URL <b><a href={url}>{url}</a></b>{", "}
                </>
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
            "select": "Select...",
            "trigger_disabled": "Disabled",
            "action_disabled": "Disabled",
            "preview": "Preview"
        },
        "script": {
            "save_failed": "Failed to save automation settings.",
            "reboot_content_changed": "automation settings"
        }
    }
}
