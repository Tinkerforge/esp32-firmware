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

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { h, Fragment, Component } from "preact";
import { __, translate_unchecked } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputFile } from "../../ts/components/input_file";
import { Button } from "react-bootstrap";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Upload } from "react-feather";

export function FirmwareUpdateNavbar() {
    return <NavbarItem name="firmware_update" module="firmware_update" title={__("firmware_update.navbar.firmware_update")} symbol={<Upload />} />;
}

interface FirmwareUpdateState {
    current_firmware: string,
    update_url: string,
    show_spinner: boolean,
    available_updates_timestamp: number,
    available_updates_cookie: number,
    available_beta_update: string,
    available_release_update: string,
    available_stable_update: string,
};

export class FirmwareUpdate extends Component<{}, FirmwareUpdateState> {
    constructor() {
        super();

        this.state = {
            current_firmware: null,
            update_url: null,
            show_spinner: false,
            available_updates_timestamp: 0,
            available_updates_cookie: null,
            available_beta_update: null,
            available_release_update: null,
            available_stable_update: null,
        } as any;

        util.addApiEventListener('info/version', () => {
            let version = API.get('info/version');

            if (this.state.current_firmware != null && this.state.current_firmware != version.firmware) {
                window.location.reload();
            }

            this.setState({current_firmware: version.firmware});
        });

        util.addApiEventListener('firmware_update/config', () => {
            let config = API.get('firmware_update/config');

            this.setState({update_url: config.update_url});
        });

        util.addApiEventListener('firmware_update/available_updates', () => {
            let available_updates = API.get('firmware_update/available_updates');

            if (this.state.available_updates_cookie != available_updates.cookie) {
                this.setState({show_spinner: false});
            }

            this.setState({
                available_updates_timestamp: available_updates.timestamp,
                available_updates_cookie: available_updates.cookie,
                available_beta_update: available_updates.beta,
                available_release_update: available_updates.release,
                available_stable_update: available_updates.stable,
            });
        });
    }

    async checkFirmware(f: File) {
        try {
            await util.upload(f.slice(0xd000 - 0x1000, 0xd000), "check_firmware", () => {})
        } catch (error) {
            if (typeof error === "string") {
                util.add_alert("firmware_update_failed", "danger", __("firmware_update.script.update_fail"), error);
            } else if (error instanceof XMLHttpRequest) {
                let xhr = error;

                if (xhr.status == 423) {
                    util.add_alert("firmware_update_failed", "danger", __("firmware_update.script.update_fail"), __("firmware_update.script.vehicle_connected"));
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
                                yes_variant: "danger",
                            }))
                            return false;
                    } else {
                        util.add_alert("firmware_update_failed", "danger", __("firmware_update.script.update_fail"), error_message);
                        return false;
                    }
                } catch {
                    util.add_alert("firmware_update_failed", "danger", __("firmware_update.script.update_fail"), xhr.responseText);
                    return false;
                }
            }
        }

        util.pauseWebSockets();
        return true;
    }

    format_build_time(version: string) {
        try {
            let timestamp = parseInt(version.split('+')[1], 16);

            if (util.hasValue(timestamp) && !isNaN(timestamp)) {
                return __("firmware_update.script.build_time")(util.timestamp_sec_to_date(timestamp));
            }
        } catch {
        }

        return ""
    }

    render() {
        if (!util.render_allowed())
            return <SubPage name="firmware_update" />;

        return (
            <SubPage name="firmware_update">
                <PageHeader title={__("firmware_update.content.firmware_update")} />

                <FormRow label={__("firmware_update.content.current_version")}>
                    <InputText value={this.state.current_firmware + this.format_build_time(this.state.current_firmware)}/>
                </FormRow>

                <FormRow label={__("firmware_update.content.manual_update")} label_muted={__("firmware_update.content.manual_update_muted")}>
                    <InputFile
                        browse={__("firmware_update.content.browse")}
                        select_file={__("firmware_update.content.select_file")}
                        upload={__("firmware_update.content.update")}
                        url="/flash_firmware"
                        accept=".bin"
                        timeout_ms={120 * 1000}
                        onUploadStart={async (f) => this.checkFirmware(f)}
                        onUploadSuccess={() => util.postReboot(__("firmware_update.script.update_success"), __("util.reboot_text"))}
                        onUploadError={error => {
                            if (typeof error === "string") {
                                util.add_alert("firmware_update_failed", "danger", __("firmware_update.script.update_fail"), error);
                            } else if (error instanceof XMLHttpRequest) {
                                let xhr = error;

                                if (xhr.status == 423)
                                    util.add_alert("firmware_update_failed", "danger", __("firmware_update.script.update_fail"), __("firmware_update.script.vehicle_connected"));
                                else {
                                    let txt = xhr.responseText.startsWith("firmware_update.") ? translate_unchecked(xhr.responseText) : (xhr.responseText ?? xhr.response);
                                    util.add_alert("firmware_update_failed", "danger", __("firmware_update.script.update_fail"), txt);
                                }
                            }
                            util.resumeWebSockets();
                        }}
                    />
                </FormRow>

                {this.state.update_url ?
                    <>
                        <FormRow label={__("firmware_update.content.check_for_updates")}>
                            <Button variant="primary" className="form-control" onClick={() => this.setState({show_spinner: true}, () => API.call("firmware_update/check_for_updates", null, ""))}>
                                {__("firmware_update.content.check_for_updates")}
                                <span class="ml-2 spinner-border spinner-border-sm" role="status" style="vertical-align: middle;" hidden={!this.state.show_spinner}></span>
                            </Button>
                        </FormRow>

                        <FormRow label={__("firmware_update.content.check_for_updates_timestamp")}>
                            <InputText value={util.timestamp_sec_to_date(this.state.available_updates_timestamp, "")}/>
                        </FormRow>

                        <FormRow label={__("firmware_update.content.available_beta_update")}>
                            <InputText value={
                                this.state.available_updates_timestamp == 0
                                ? ""
                                : (this.state.available_beta_update.length > 0
                                    ? this.state.available_beta_update + this.format_build_time(this.state.available_beta_update)
                                    : __("firmware_update.content.no_update"))}/>
                        </FormRow>

                        <FormRow label={__("firmware_update.content.available_release_update")}>
                            <InputText value={
                                this.state.available_updates_timestamp == 0
                                ? ""
                                : (this.state.available_release_update.length > 0
                                    ? this.state.available_release_update + this.format_build_time(this.state.available_release_update)
                                    : __("firmware_update.content.no_update"))}/>
                        </FormRow>

                        <FormRow label={__("firmware_update.content.available_stable_update")}>
                            <InputText value={
                                this.state.available_updates_timestamp == 0
                                ? ""
                                : (this.state.available_stable_update.length > 0
                                    ? this.state.available_stable_update + this.format_build_time(this.state.available_stable_update)
                                    : __("firmware_update.content.no_update"))}/>
                        </FormRow>
                    </>
                    : undefined
                }
            </SubPage>
        );
    }
}

export function init() {
}
