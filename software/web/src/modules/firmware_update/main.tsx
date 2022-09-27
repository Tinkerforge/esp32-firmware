/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

import $ from "../../ts/jq";

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, render, Fragment, Component} from "preact";
import { __, translate_unchecked } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";

import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputFile } from "../../ts/components/input_file";
import { Button } from "react-bootstrap";

type FirmwareUpdateConfig = API.getType['info/version'];

export class FirmwareUpdate extends Component<{}, FirmwareUpdateConfig> {
    constructor() {
        super();
        util.eventTarget.addEventListener('info/version', () => {
            let newState = API.get('info/version');
            if (this.state != null && this.state.firmware != null && this.state.firmware != newState.firmware)
                window.location.reload();

            this.setState(API.get('info/version'));
        });
    }

    async checkFirmware(f: File) {
        try {
            await util.upload(f.slice(0xd000 - 0x1000, 0xd000), "check_firmware", () => {})
        } catch (error) {
            if (typeof error === "string") {
                util.add_alert("firmware_update_failed","alert-danger", __("firmware_update.script.flash_fail"), error);
            } else if (error instanceof XMLHttpRequest) {
                let xhr = error;

                if (xhr.status == 423) {
                    util.add_alert("firmware_update_failed", "alert-danger", __("firmware_update.script.flash_fail"), __("firmware_update.script.vehicle_connected"));
                    return false;
                }

                try {
                    let e = JSON.parse(xhr.responseText)
                    let error_message = translate_unchecked(e["error"])
                    if (e["error"] == "firmware_update.script.downgrade") {
                        error_message = error_message.replace("%fw%", e["fw"]).replace("%installed%", e["installed"]);

                        const modal = util.async_modal_ref.current;
                        if(!await modal.show({
                                title: __("firmware_update.content.downgrade"),
                                body: error_message,
                                no_text: __("firmware_update.content.abort_downgrade"),
                                yes_text: __("firmware_update.content.confirm_downgrade"),
                                no_variant: "secondary",
                                yes_variant: "danger"
                            }))
                            return false;
                    } else {
                        util.add_alert("firmware_update_failed","alert-danger", __("firmware_update.script.flash_fail"), error_message);
                        return false;
                    }
                } catch {
                    util.add_alert("firmware_update_failed","alert-danger", __("firmware_update.script.flash_fail"), xhr.responseText);
                    return false;
                }
            }
        }

        util.pauseWebSockets();
        return true;
    }

    render(props: {}, state: Readonly<FirmwareUpdateConfig>) {
        if (!state)
            return (<></>);

        // TODO: why not use the charge tracker module here?
        let show_config_reset = false;
        if (API.get('info/modules')?.hasOwnProperty("users") && (API.get('info/modules') as any).users)
            show_config_reset = true;

        return (
            <>
                <PageHeader title={__("firmware_update.content.firmware_update")} />

                <FormRow label={__("firmware_update.content.current_firmware")}>
                    <InputText value={state.firmware}/>
                </FormRow>

                <FormRow label={__("firmware_update.content.firmware_update_label")} label_muted={__("firmware_update.content.firmware_update_desc")}>
                    <InputFile
                        browse={__("firmware_update.content.browse")}
                        select_file={__("firmware_update.content.select_file")}
                        upload={__("firmware_update.content.update")}
                        url="/flash_firmware"

                        timeout_ms={120 * 1000}
                        onUploadStart={async (f) => this.checkFirmware(f)}
                        onUploadSuccess={() => util.postReboot(__("firmware_update.script.flash_success"), __("util.reboot_text"))}
                        onUploadError={error => {
                            if (typeof error === "string") {
                                util.add_alert("firmware_update_failed","alert-danger", __("firmware_update.script.flash_fail"), error);
                            } else if (error instanceof XMLHttpRequest) {
                                let xhr = error;

                                if (xhr.status == 423)
                                    util.add_alert("firmware_update_failed", "alert-danger", __("firmware_update.script.flash_fail"), __("firmware_update.script.vehicle_connected"));
                                else {
                                    let txt = xhr.responseText.startsWith("firmware_update.") ? translate_unchecked(xhr.responseText) : (xhr.responseText ?? xhr.response);
                                    util.add_alert("firmware_update_failed","alert-danger", __("firmware_update.script.flash_fail"), txt);
                                }
                            }
                            util.resumeWebSockets();
                        }}
                    />
                </FormRow>

                <FormRow label={__("firmware_update.content.reboot")} label_muted={__("firmware_update.content.reboot_desc")}>
                    <Button variant="primary" className="form-control" onClick={util.reboot}>{__("firmware_update.content.reboot")}</Button>
                </FormRow>

                <FormRow label={__("firmware_update.content.current_spiffs")}>
                    <InputText value={state.config}/>
                </FormRow>

                {show_config_reset ?
                    <FormRow label={__("firmware_update.content.config_reset")} label_muted={__("firmware_update.content.config_reset_desc")}>
                        <Button variant="danger" className="form-control" onClick={async () => {
                                const modal = util.async_modal_ref.current;
                                if (!await modal.show({
                                        title: __("firmware_update.content.config_reset"),
                                        body: __("firmware_update.content.config_reset_modal_text"),
                                        no_text: __("firmware_update.content.abort_reset"),
                                        yes_text: __("firmware_update.content.confirm_config_reset"),
                                        no_variant: "secondary",
                                        yes_variant: "danger"
                                    }))
                                    return;

                                try {
                                    await util.put("/config_reset", {"do_i_know_what_i_am_doing": true});
                                    util.postReboot(__("firmware_update.script.config_reset_init"), __("util.reboot_text"));
                                } catch (error) {
                                    util.add_alert("config_reset_failed", "alert-danger", __("firmware_update.script.config_reset_error"), error);
                                }
                            }}>{__("firmware_update.content.config_reset")}</Button>
                    </FormRow>
                    : ""
                }

                <FormRow label={__("firmware_update.content.factory_reset")} label_muted={__("firmware_update.content.factory_reset_desc")}>
                    <Button variant="danger" className="form-control" onClick={async () => {
                        const modal = util.async_modal_ref.current;
                        if (!await modal.show({
                                title: __("firmware_update.content.factory_reset"),
                                body: __("firmware_update.content.factory_reset_modal_text"),
                                no_text: __("firmware_update.content.abort_reset"),
                                yes_text: __("firmware_update.content.confirm_factory_reset"),
                                no_variant: "secondary",
                                yes_variant: "danger"
                            }))
                            return;

                        try {
                            await util.put("/factory_reset", {"do_i_know_what_i_am_doing": true});
                            util.postReboot(__("firmware_update.script.factory_reset_init"), __("util.reboot_text"));
                        } catch (error) {
                            util.add_alert("factory_reset_failed", "alert-danger", __("firmware_update.script.factory_reset_error"), error);
                        }
                    }}>{__("firmware_update.content.factory_reset")}</Button>
                </FormRow>
            </>
        );
    }
}

render(<FirmwareUpdate/>, $('#flash')[0])

export function init() {}

export function add_event_listeners(source: API.APIEventTarget) {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-flash').prop('hidden', !module_init.firmware_update);
}
