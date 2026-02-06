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
import { __ } from "../../ts/translation";
import { h, Fragment } from "preact";
import { InputText } from "../../ts/components/input_text";
import { Alert, Button } from "react-bootstrap";
import { Save } from "react-feather";
import { ConfigComponent } from "../../ts/components/config_component";
import { StatusSection } from "../../ts/components/status_section";
import { PageHeader } from "../../ts/components/page_header";
import { FormRow } from "ts/components/form_row";
import { MobileStatusButton } from "../../ts/components/header_status";

export class DeviceNameStatus extends ConfigComponent<"info/display_name"> {
    constructor() {
        super('info/display_name', () => __("device_name.script.save_failed"));
    }

    render(props: {}, state: Readonly<API.getType['info/display_name']>) {
        if (!util.render_allowed() || !API.hasModule("device_name"))
            return <StatusSection name="device_name" class={util.is_native_median_app() ? "sticky-top-app" : "sticky-under-top"} />;

        document.title = API.get("info/display_name").display_name + " - " + __("main.title");

        const hide_save = state.display_name == API.get('info/display_name').display_name;

        const make_device_name_form = () => <form class="w-100" onSubmit={(e: Event) => {
            e.preventDefault();
            e.stopPropagation();
            if (!(e.target as HTMLFormElement).checkValidity())
                return;

            API.save("info/display_name", state, () => __("device_name.script.save_failed"));
        }}>
            <InputText class={hide_save ? "rounded-end" : undefined} maxLength={32} value={state.display_name} onValue={(v) => this.setState({display_name: v})} required>
                <Button type="submit" hidden={hide_save} disabled={hide_save}><Save/></Button>
            </InputText>
        </form>;

        const is_native_app = util.is_native_median_app();

        // In native app: always show status button header
        // In browser: show status button header on mobile (< md), standard PageHeader on desktop (>= md)
        // Note: In native app, main has px-4 padding, so we use negative margins (ms-n4, me-n4) to extend to edges
        const status_button_header = (
            <div class={is_native_app ? "row mb-3 sticky-top-app ms-n4 me-n4" : "d-md-none row mb-3 sticky-under-top"}>
                <div class={is_native_app ? "col ps-0 pe-0" : "col ps-0 pe-0"}>
                    <div class="d-flex align-items-stretch border-bottom">
                        <div class="flex-grow-1 d-flex align-items-center py-2 ps-3 pe-3">
                            {make_device_name_form()}
                        </div>
                        <MobileStatusButton />
                    </div>
                </div>
            </div>
        );

        const desktop_header = !is_native_app && (
            <div class="d-none d-md-block">
                <PageHeader title={__("device_name.status.status")} titleClass="col-4" childrenClass="col-8">
                    {make_device_name_form()}
                </PageHeader>
            </div>
        );

        return <StatusSection name="device_name" class={is_native_app ? "sticky-app" : "sticky-under-top"}>
            {status_button_header}
            {desktop_header}
        </StatusSection>;
    }
}

export function pre_init() {
}

export function init() {
}
