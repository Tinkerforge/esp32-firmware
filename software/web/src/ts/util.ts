import $ from "jquery";

import * as API from "./api";

declare function __(s: string): string;

export function reboot() {
    $.ajax({
        url: '/reboot',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(null),
        success: () => postReboot(__("util.reboot_title"), __("util.reboot_text"))
    });
}

export function update_button_group(button_group_id: string, index_to_select: number, text_replacement?: string) {
    let buttons = $(`#${button_group_id} :button`);
    let color_suffixes = Array(buttons.length);
    for (let i = 0; i < buttons.length; ++i) {
        let classes = buttons[i].classList;
        for (let j = 0; j < classes.length; ++j) {
            if (classes[j].substring(0, 4) != "btn-")
                continue
            let splt = classes[j].split("-");
            color_suffixes[i] = splt[splt.length - 1];
        }
    }

    for (let i = 0; i < buttons.length; ++i) {
        buttons[i].classList.remove("btn-" + color_suffixes[i]);
        buttons[i].classList.add("btn-outline-" + color_suffixes[i]);
    }

    buttons[index_to_select].classList.remove("btn-outline-" + color_suffixes[index_to_select]);
    buttons[index_to_select].classList.add("btn-" + color_suffixes[index_to_select]);
    if (text_replacement != null)
        buttons[index_to_select].innerHTML = text_replacement;
}

export function add_alert(id: string, cls: string, title: string, text: string) {
    let to_add = `<div id="alert_${id}" class="alert ${cls} alert-dismissible fade show custom-alert" role="alert" style="line-height: 1.5rem;">
    <strong>${title}</strong> ${text}
    <button type="button" class="close" data-dismiss="alert" aria-label="Close">
    <span aria-hidden="true">&times;</span>
    </button>
</div>`;

    if(!document.getElementById(`alert_${id}`)) {
        $('#alert_placeholder').append(to_add);
    } else {
        $(`#alert_${id}`).replaceWith(to_add);
    }
}

export function remove_alert(id: string) {
    $(`#alert_${id}`).remove();
}

export function format_timespan(secs: number) {
    let days = 0;
    let hours = 0;
    let mins = 0;

    let dayString = "";
    let hourString = "";
    let minString = "";
    let secString = "";

    if (secs >= 60 * 60 * 24) {
        days = Math.floor(secs / (60 * 60 * 24));
        if (days > 1) {
            dayString = days + ` ${__("days")}, `;
        } else if (days > 0) {
            dayString = days + ` ${__("day")}, `;
        }
        secs %= 60 * 60 * 24;
    }

    if (secs >= 60 * 60) {
        hours = Math.floor(secs / (60 * 60));
        if (hours >= 10) {
            hourString = hours + ":";
        } else {
            hourString = "0" + hours + ":";
        }
        secs %= 60 * 60;
    } else {
        hourString = "00:";
    }

    if (secs >= 60) {
        mins = Math.floor(secs / 60);
        if (mins >= 10) {
            minString = mins + ":";
        } else {
            minString = "0" + mins + ":";
        }
        secs %= 60;
    } else {
        minString = "00:";
    }

    if (secs >= 10) {
        secString = secs.toString();
    } else {
        secString = "0" + secs;
    }

    return dayString + hourString + minString + secString;
}

export function toLocaleFixed(i: number, fractionDigits: number) {
    return i.toLocaleString(undefined, {
        minimumFractionDigits: fractionDigits,
        maximumFractionDigits: fractionDigits
    });
}

export function setNumericInput(id: string, i: number, fractionDigits: number) {
    // Firefox does not localize numbers with a fractional part correctly.
    // OTOH Webkit based browsers (correctly) expect setting the value to a non-localized number.
    // Unfortunately, setting the value to a localized number (i.e. with , instead of . for German)
    // does not raise an exception, instead only a warning on the console is shown.
    // So to make everyone happy, we use user agent detection.
    if (navigator.userAgent.includes("Gecko/")) {
        (<HTMLInputElement> document.getElementById(id)).value = toLocaleFixed(i, fractionDigits);
    } else {
        (<HTMLInputElement> document.getElementById(id)).value = i.toFixed(fractionDigits);
    }
}

export function toggle_password_fn(input_name: string) {
    return (ev: Event) => {
        let input = <HTMLInputElement>$(input_name)[0];
        let x = <HTMLInputElement>ev.target;

        if (x.checked)
            input.type = 'text';
        else
            input.type = 'password';
    }
}

export function clear_password_fn(input_name: string, to_be_cleared: string = __("util.to_be_cleared"), unchanged: string = __("util.unchanged")) {
    return (ev: Event) => {
        let x = <HTMLInputElement>ev.target;
        if (x.checked) {
            $(input_name).val('');
            $(input_name).attr('placeholder', to_be_cleared);
        } else {
            $(input_name).attr('placeholder', unchanged);
        }

        $(input_name).prop("disabled", x.checked);
    }
}

let wsReconnectTimeout: number = null;
let wsReconnectCallback: () => void = null;
let ws: WebSocket = null;

const RECONNECT_TIME = 12000;

let eventTarget: API.APIEventTarget = null;

export function setupEventSource(first: boolean, keep_as_first: boolean, continuation: (ws: WebSocket, eventTarget: API.APIEventTarget) => void) {
    if (!first) {
        add_alert("event_connection_lost", "alert-warning",  __("util.event_connection_lost_title"), __("util.event_connection_lost"))
    }
    console.log("Connecting to web socket");
    if (ws != null) {
        ws.close();
    }
    ws = new WebSocket((location.protocol == 'https:' ? 'wss://' : 'ws://') + location.host + '/ws');
    eventTarget = new API.APIEventTarget();

    if (wsReconnectTimeout != null) {
        clearTimeout(wsReconnectTimeout);
    }
    wsReconnectCallback = () => setupEventSource(keep_as_first ? first : false, keep_as_first, continuation)
    wsReconnectTimeout = window.setTimeout(wsReconnectCallback, RECONNECT_TIME);

    ws.onmessage = (e: MessageEvent) => {
        if(!keep_as_first)
            remove_alert("event_connection_lost");

        if (wsReconnectTimeout != null) {
            window.clearTimeout(wsReconnectTimeout);
            wsReconnectTimeout = null;
        }
        wsReconnectTimeout = window.setTimeout(wsReconnectCallback, RECONNECT_TIME);

        let topics = [];
        for (let item of e.data.split("\n")) {
            if (item == "")
                continue;
            let obj = JSON.parse(item);
            if (!("topic" in obj) || !("payload" in obj)) {
                console.log("Received malformed event", obj);
                return;
            }

            topics.push(obj["topic"]);
            API.update(obj["topic"], obj["payload"]);
        }

        for (let topic of topics) {
            API.trigger(topic, eventTarget);
        }
    }

    continuation(ws, eventTarget);
}

export function pauseWebSockets() {
    ws.close();
    if (wsReconnectTimeout != null) {
        clearTimeout(wsReconnectTimeout);
    }
}

export function resumeWebSockets() {
    wsReconnectTimeout = window.setTimeout(wsReconnectCallback, RECONNECT_TIME);
}

export function postReboot(alert_title: string, alert_text: string) {
    ws.close();
    clearTimeout(wsReconnectTimeout);
    add_alert("reboot", "alert-success", alert_title, alert_text);
    // Wait 5 seconds before starting the reload/reconnect logic, to make sure the reboot has actually started yet.
    // Else it sometimes happens, that we reconnect _before_ the reboot starts.
    window.setTimeout(() => whenLoggedInElseReload(() =>
        setupEventSource(true, true, (ws, eventSource) => {
                // It is a bit of a hack to use version here, but
                // as opposed to keep-alive, version was already there in the first version.
                // so this will even work if downgrading to an version older than
                // 1.1.0
                console.log("setting up...");
                eventSource.addEventListener('info/version', function (e) {
                    console.log("reloading");
                    window.location.reload();
                }, false);})
    ), 5000);
}

let loginReconnectTimeout: number = null;

export function ifLoggedInElse(if_continuation: () => void, else_continuation: () => void) {
    $.ajax({url: "/login_state", timeout:3000}).done(function(data, statusText, xhr) {
        if (data == "Logged in") {
            if_continuation();
        } else {
            else_continuation();
        }
    }).fail(function(xhr, statusText, errorThrown) {
        if (xhr.status == 404) {
            if_continuation();
        }
    });
}

export function ifLoggedInElseReload(continuation: () => void) {
    ifLoggedInElse(continuation, function() {
        window.location.reload();
    });
}

export function whenLoggedInElseReload(continuation: () => void) {
    if (loginReconnectTimeout != null) {
        clearTimeout(loginReconnectTimeout);
        loginReconnectTimeout = null;
    }
    if (wsReconnectTimeout != null) {
        clearTimeout(wsReconnectTimeout);
        wsReconnectTimeout = null;
    }
    loginReconnectTimeout = window.setTimeout(
        () => ifLoggedInElseReload(
            () => {clearTimeout(loginReconnectTimeout); continuation();}),
        RECONNECT_TIME);

    ifLoggedInElseReload(() => {clearTimeout(loginReconnectTimeout); continuation();});
}

// Password inputs use the empty string as the "unchanged" value.
// However the API expects a null value if the value should not be changed.
// If the input is disabled, the clear toggle was set.
// Return an empty string to remove the stored password in this case.
export function passwordUpdate(input_selector: string) {
    let input = $(input_selector);
    if (input.length == 0) {
        console.error(`Input with selector ${input_selector} not found.`);
    }
    if (input.attr("disabled"))
        return "";

    let value = input.val().toString();

    return value.length > 0 ? value : null;
}

function iso8601ButLocal(date: Date) {
    const offset = date.getTimezoneOffset() * 60 * 1000;
    const local =  date.getTime() - offset;
    const dateLocal = new Date(local);
    return dateLocal.toISOString().slice(0, -1);
}

export function downloadToFile(content: BlobPart, filename_prefix: string, extension: string, contentType: string) {
    const a = document.createElement('a');
    const file = new Blob([content], {type: contentType});
    let t = iso8601ButLocal(new Date()).replace(/:/gi, "-").replace(/\./gi, "-");
    let name = API.get('info/name').name;

    a.href= URL.createObjectURL(file);
    a.download = filename_prefix + "-" + name + "-" + t + "." + extension;
    a.click();

    URL.revokeObjectURL(a.href);
};

export function getShowRebootModalFn(changed_value_name: string) {
    return () => {
        $('#reboot_content_changed').html(changed_value_name);
        $('#reboot').modal('show');
    }
}
