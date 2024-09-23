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

            "http": "HTTP-Nachricht empfangen",
            "http_method": "Erlaube Methode",
            "http_get": "GET",
            "http_post": "POST",
            "http_put": "PUT",
            "http_post_put": "POST oder PUT",
            "http_get_post_put": "GET, POST oder PUT",
            "http_url_suffix": "URL-Suffix",
            "http_payload": "Erwartete Nachricht",
            "http_match_any": "Jede Nachricht wird akzeptiert",
            "http_translation_function": /*FFN*/(method: number, url: string, payload: string) => {
                let methods = [
                    "GET",
                    "POST",
                    "PUT",
                    "POST- oder PUT",
                    "GET-, POST- oder PUT",
                ];

                let payload_str = <></>;
                if (payload.length == 0) {
                    payload_str = <>mit beliebigem Inhalt</>;
                } else {
                    payload_str = <>mit dem Inhalt "<b>{payload}</b>"</>;
                }

                let ret = <>Wenn eine HTTP-{methods[method]}-Anfrage {payload_str}</>;

                return <>
                    {ret} an URL <b><a href={url}>{url}</a></b> empfangen wird{", "}
                </>
            }/*NF*/,

            "print_action": "Gebe im Ereignis-Log aus",
            "print_action_message": "Nachricht",
            "print_action_text": /*FFN*/(message: string) => <>zeige "<b>{message}</b>" im <b>Ereignis-Log</b> an.</> /*NF*/

        },
        "content": {
            "automation": "Automatisierung",
            "rule": "Regel",
            "condition": "Bedingung",
            "action": "Aktion",
            "add_rule_title": "Regel hinzufügen",
            "add_rule_count": /*SFN*/(count: number, max: number) => count + " von " + max + " Regeln konfiguriert"/*NF*/,
            "edit_rule_title": "Regel bearbeiten",
            "select": "Auswählen...",
            "trigger_disabled": "Deaktiviert",
            "action_disabled": "Deaktiviert",
            "preview": "Vorschau"
        },
        "script": {
            "save_failed": "Speichern der Automatisierungs-Einstellungen fehlgeschlagen.",
            "reboot_content_changed": "Automatisierungs-Einstellungen"
        }
    }
}
