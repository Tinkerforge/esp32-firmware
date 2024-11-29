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
import { Progress } from "../../ts/components/progress";
import { Upload } from "react-feather";
import { CheckState } from "./check_state.enum";
import { InstallState } from "./install_state.enum";

export function FirmwareUpdateNavbar() {
    return <NavbarItem name="firmware_update" module="firmware_update" title={__("firmware_update.navbar.firmware_update")} symbol={<Upload />} />;
}

interface FirmwareUpdateState {
    current_firmware: string,
    manual_install_in_progress: boolean,
    publisher: string,
    check_timestamp: number,
    check_state: number,
    update_version: string,
    install_state: number,
    install_progress: number,
};

export class FirmwareUpdate extends Component<{}, FirmwareUpdateState> {
    constructor() {
        super();

        this.state = {
            current_firmware: null,
            manual_install_in_progress: false,
            publisher: null,
            check_timestamp: 0,
            check_state: null,
            update_version: null,
            install_state: null,
            install_progress: 0,
        } as any;

        util.addApiEventListener('info/version', () => {
            let version = API.get('info/version');

            if (this.state.current_firmware != null && this.state.current_firmware != version.firmware) {
                window.location.reload();
            }

            this.setState({current_firmware: version.firmware});
        });

        util.addApiEventListener('firmware_update/state', () => {
            let state = API.get('firmware_update/state');

            if (state.install_state == InstallState.Rebooting) {
                util.postReboot(__("firmware_update.script.update_success"), __("util.reboot_text"));
            }

            this.setState({
                publisher: state.publisher,
                check_timestamp: state.check_timestamp,
                check_state: state.check_state,
                update_version: state.update_version,
                install_state: state.install_state,
                install_progress: state.install_progress,
            });
        });
    }

    async checkFirmware(f: File) {
        try {
            await util.upload(f.slice(0xd000 - 0x1000, 0xd000), "check_firmware", () => {})
        }
        catch (error) {
            let message = "";

            if (typeof error === "string") {
                message = error;
            }
            else if (error instanceof XMLHttpRequest) {
                try {
                    let response = JSON.parse(error.responseText);

                    if (response.error == InstallState.Downgrade) {
                        const modal = util.async_modal_ref.current;

                        if (await modal.show({
                                title: () => __("firmware_update.content.downgrade_title"),
                                body: () => __("firmware_update.content.downgrade_body")(response.firmware_version, response.installed_version),
                                no_text: () => __("firmware_update.content.abort_update"),
                                yes_text: () => __("firmware_update.content.confirm_downgrade"),
                                no_variant: "secondary",
                                yes_variant: "danger",
                            })) {
                            return true;
                        }
                    }
                    else {
                        message = translate_unchecked("firmware_update.script.install_state_" + response.error);
                    }
                }
                catch {
                    message = error.responseText;
                }
            }

            if (message != "") {
                util.add_alert("firmware_update_failed", "danger", () => __("firmware_update.script.update_fail"), () => message);
            }

            return false;
        }

        return true;
    }

    format_build_extra(version: string, publisher: string) {
        let result = '';

        try {
            let timestamp = parseInt(version.split('+')[1], 16);

            if (util.hasValue(timestamp) && !isNaN(timestamp)) {
                result += __("firmware_update.script.build_time")(util.timestamp_sec_to_date(timestamp));
            }
        } catch {
        }

        if (result.length > 0 && publisher.length > 0) {
            result += ', ';
        }

        if (publisher.length > 0) {
            result += __("firmware_update.script.publisher")(publisher);
        }

        if (result.length > 0) {
            result = ` (${result})`
        }

        return result;
    }

    render() {
        if (!util.render_allowed())
            return <SubPage name="firmware_update" />;

        return (
            <SubPage name="firmware_update">
                <PageHeader title={__("firmware_update.content.firmware_update")} />

                <FormRow label={__("firmware_update.content.current_version")}>
                    <InputText value={this.state.current_firmware + this.format_build_extra(this.state.current_firmware, this.state.publisher)}/>
                </FormRow>

                <FormRow label={__("firmware_update.content.manual_update")} label_muted={__("firmware_update.content.manual_update_muted")}>
                    <InputFile
                        browse={__("firmware_update.content.browse")}
                        select_file={__("firmware_update.content.select_file")}
                        upload={__("firmware_update.content.install_update")}
                        uploading={__("firmware_update.content.installing_update")}
                        url="/flash_firmware"
                        accept=".bin"
                        timeout_ms={120 * 1000}
                        onUploadStart={async (f) => {
                            this.setState({manual_install_in_progress: true});
                            util.remove_alert("firmware_update_failed");

                            if (!await this.checkFirmware(f)) {
                                this.setState({manual_install_in_progress: false});
                                return false;
                            }

                            if (util.remoteAccessMode) {
                                util.pauseiFrameSocket();
                            } else {
                                util.pauseWebSockets();
                            }

                            if (this.state.install_state == InstallState.InProgress) {
                                this.setState({install_state: InstallState.Aborted, install_progress: 0});
                            }

                            return true;
                        }}
                        onUploadSuccess={() => util.postReboot(__("firmware_update.script.update_success"), __("util.reboot_text"))}
                        onUploadError={async (error) => {
                            let message = "";

                            if (typeof error === "string") {
                                message = error;
                            }
                            else if (error instanceof XMLHttpRequest) {
                                try {
                                    let response = JSON.parse(error.responseText);

                                    if (response.error == InstallState.SignatureVerifyFailed) {
                                        const modal = util.async_modal_ref.current;

                                        if (await modal.show({
                                                title: () => __("firmware_update.content.signature_verify_failed_title"),
                                                body: () => __("firmware_update.content.signature_verify_failed_body")(response.actual_publisher, response.expected_publisher),
                                                no_text: () => __("firmware_update.content.abort_update"),
                                                yes_text: () => __("firmware_update.content.confirm_override"),
                                                no_variant: "secondary",
                                                yes_variant: "danger",
                                            })) {
                                            try {
                                                await API.call("firmware_update/override_signature", {cookie: response.cookie}, () => __("firmware_update.script.update_fail"));
                                            }
                                            catch {
                                                return;
                                            }

                                            util.postReboot(__("firmware_update.script.update_success"), __("util.reboot_text"));
                                            return;
                                        }
                                    }
                                    else {
                                        message = translate_unchecked("firmware_update.script.install_state_" + response.error);
                                    }
                                } catch {
                                    message = error.responseText;
                                }
                            }

                            if (message != "") {
                                util.add_alert("firmware_update_failed", "danger", () => __("firmware_update.script.update_fail"), () => message);
                            }

                            util.resumeWebSockets();
                            this.setState({manual_install_in_progress: false});
                        }}
                    />
                </FormRow>

                {API.hasFeature("firmware_update") ?
                    <>
                        <FormRow label={__("firmware_update.content.check_for_update")}>
                            <Button variant="primary"
                                    className="form-control"
                                    onClick={() => this.setState({check_state: CheckState.InProgress, install_state: InstallState.Idle, install_progress: 0}, () => API.call("firmware_update/check_for_update", null, () => ""))}
                                    disabled={this.state.check_state == CheckState.InProgress || this.state.install_state == InstallState.InProgress || this.state.manual_install_in_progress}>
                                {__("firmware_update.content.check_for_update")}
                                <span class="ml-2 spinner-border spinner-border-sm" role="status" style="vertical-align: middle;" hidden={this.state.check_state != CheckState.InProgress}></span>
                            </Button>
                        </FormRow>

                        {this.state.check_timestamp != 0 && this.state.check_state != CheckState.InProgress ? <>
                            <FormRow label={__("firmware_update.content.check_for_update_timestamp")}>
                                <InputText value={util.timestamp_sec_to_date(this.state.check_timestamp, "")} />
                            </FormRow>

                            {this.state.check_state != null && this.state.check_state != CheckState.Idle ?
                                <FormRow label={__("firmware_update.content.check_for_update_error")}>
                                    <InputText value={translate_unchecked(`firmware_update.script.check_state_${this.state.check_state}`)} />
                                </FormRow>
                                : <>
                                <FormRow label={__("firmware_update.content.available_update")}>
                                    <InputText value={
                                        this.state.update_version.length > 0
                                            ? this.state.update_version + this.format_build_extra(this.state.update_version, '')
                                            : __("firmware_update.content.no_update")}>
                                        {this.state.update_version.length > 0
                                            ? <div class="input-group-append">
                                                <Button variant="primary"
                                                        type="button"
                                                        onClick={() => this.setState({install_state: InstallState.InProgress, install_progress: 0}, () => API.call("firmware_update/install_firmware", {version: this.state.update_version}, () => __("firmware_update.script.install_failed")))}
                                                        disabled={this.state.install_state == InstallState.InProgress || this.state.manual_install_in_progress}>{__("firmware_update.content.install_update")}</Button>
                                            </div>
                                            : undefined
                                        }
                                    </InputText>
                                </FormRow>
                            </>}
                        </> : undefined}

                        {this.state.install_state != InstallState.Idle ? <>
                            {this.state.install_state == InstallState.InProgress
                                ? <FormRow label={__("firmware_update.content.install_progress")}>
                                      <Progress class="mb-1" progress={this.state.install_progress / 100} />
                                      <div class="mb-0">{__("firmware_update.content.installing_update")}</div>
                                  </FormRow>
                                : <FormRow label={__("firmware_update.content.install_complete")}>
                                      <InputText value={translate_unchecked("firmware_update.script.install_state_" + this.state.install_state)} />
                                  </FormRow>
                            }
                        </> : undefined}
                    </>
                    : undefined
                }
            </SubPage>
        );
    }
}

export function init() {
}
