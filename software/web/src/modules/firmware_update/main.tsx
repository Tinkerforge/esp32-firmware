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
    version: API.getType["info/version"],
};

export class FirmwareUpdate extends Component<{}, FirmwareUpdateState> {
    constructor() {
        super();

        this.state = {
            version: null,
        } as any;

        util.addApiEventListener('info/version', () => {
            let version = API.get('info/version');

            if (this.state.version != null && this.state.version.firmware != null && this.state.version.firmware != version.firmware) {
                window.location.reload();
            }

            this.setState({version: API.get('info/version')});
        });
    }

    async checkFirmware(f: File) {
        try {
            await util.upload(f.slice(0xd000 - 0x1000, 0xd000), "firmware_update/check_firmware", () => {})
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

    render() {
        if (!util.render_allowed())
            return <SubPage name="firmware_update" />;

        let build_time: string = '';

        try {
            let timestamp = parseInt(this.state.version.firmware.split('-')[1], 16);

            if (util.hasValue(timestamp) && !isNaN(timestamp)) {
                build_time = __("firmware_update.script.build_time_prefix") + util.timestamp_sec_to_date(timestamp) + __("firmware_update.script.build_time_suffix");
            }
        } catch {
        }

        return (
            <SubPage name="firmware_update">
                <PageHeader title={__("firmware_update.content.firmware_update")} />

                <FormRow label={__("firmware_update.content.current_firmware")}>
                    <InputText value={this.state.version.firmware + build_time}/>
                </FormRow>

                <FormRow label={__("firmware_update.content.firmware_update_label")} label_muted={__("firmware_update.content.firmware_update_desc")}>
                    <InputFile
                        browse={__("firmware_update.content.browse")}
                        select_file={__("firmware_update.content.select_file")}
                        upload={__("firmware_update.content.update")}
                        url="/firmware_update/flash_firmware"
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
            </SubPage>
        );
    }
}

export function init() {
}
