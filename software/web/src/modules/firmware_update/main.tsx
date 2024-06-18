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
    check_is_pending: boolean,
    update_url: string,
    check_timestamp: number,
    check_error: string,
    beta_update: string,
    release_update: string,
    stable_update: string,
};

export class FirmwareUpdate extends Component<{}, FirmwareUpdateState> {
    constructor() {
        super();

        this.state = {
            current_firmware: null,
            check_is_pending: false,
            update_url: null,
            check_timestamp: 0,
            check_error: null,
            beta_update: null,
            release_update: null,
            stable_update: null,
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

        util.addApiEventListener('firmware_update/state', () => {
            let state = API.get('firmware_update/state');

            this.setState({
                check_is_pending: state.check_error == "pending",
                check_timestamp: state.check_timestamp,
                check_error: state.check_error,
                beta_update: state.beta_update,
                release_update: state.release_update,
                stable_update: state.stable_update,
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
                try {
                    let e = JSON.parse(error.responseText)
                    let error_message = translate_unchecked("firmware_update.script." + e["error"])
                    if (e["error"] == "downgrade") {
                        error_message = error_message.replace("%firmware%", e["firmware"]).replace("%installed%", e["installed"]);

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
                    util.add_alert("firmware_update_failed", "danger", __("firmware_update.script.update_fail"), error.responseText);
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

    get_update_component(version: string) {
        return <InputText value={
                version.length > 0
                    ? version + this.format_build_time(version)
                    : __("firmware_update.content.no_update")}>
                {version.length > 0
                    ? <div class="input-group-append">
                        <Button variant="primary" type="button" onClick={() => API.call("firmware_update/install_firmware", {version: version}, __("firmware_update.script.install_failed"))}>{__("firmware_update.content.install_update")}</Button>
                    </div>
                    : undefined
                }
            </InputText>;
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
                                let txt = error.responseText.startsWith("firmware_update.") ? translate_unchecked(error.responseText) : (error.responseText ?? error.response);
                                util.add_alert("firmware_update_failed", "danger", __("firmware_update.script.update_fail"), txt);
                            }
                            util.resumeWebSockets();
                        }}
                    />
                </FormRow>

                {this.state.update_url ?
                    <>
                        <FormRow label={__("firmware_update.content.check_for_update")}>
                            <Button variant="primary" className="form-control" onClick={() => this.setState({check_is_pending: true}, () => API.call("firmware_update/check_for_update", null, ""))} disabled={this.state.check_is_pending}>
                                {__("firmware_update.content.check_for_update")}
                                <span class="ml-2 spinner-border spinner-border-sm" role="status" style="vertical-align: middle;" hidden={!this.state.check_is_pending}></span>
                            </Button>
                        </FormRow>

                        {this.state.check_timestamp == 0 || this.state.check_is_pending ? undefined : <>
                            <FormRow label={__("firmware_update.content.check_for_update_timestamp")}>
                                <InputText value={util.timestamp_sec_to_date(this.state.check_timestamp, "")} />
                            </FormRow>

                            {this.state.check_error != null && this.state.check_error.length > 0 ?
                                <FormRow label={__("firmware_update.content.check_for_update_error")}>
                                    <InputText value={translate_unchecked("firmware_update.script." + this.state.check_error)} />
                                </FormRow>
                                : <>
                                <FormRow label={__("firmware_update.content.beta_update")}>
                                    {this.get_update_component(this.state.beta_update)}
                                </FormRow>

                                <FormRow label={__("firmware_update.content.release_update")}>
                                    {this.get_update_component(this.state.release_update)}
                                </FormRow>

                                <FormRow label={__("firmware_update.content.stable_update")}>
                                    {this.get_update_component(this.state.stable_update)}
                                </FormRow>
                            </>}
                        </>}
                    </>
                    : undefined
                }
            </SubPage>
        );
    }
}

export function init() {
}
