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
import { __ } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";

import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { Button } from "react-bootstrap";

export class Debug extends Component<{}, API.getType['debug/state']> {
    constructor() {
        super();

        util.addApiEventListener('debug/state', () => {
            this.setState(API.get('debug/state'));
        });

    }

    render(props: {}, state: Readonly<API.getType['debug/state']>) {
        if (!util.allow_render)
            return (<></>);

        return (
            <>
                <PageHeader title={__("debug.content.debug")} />

                <FormRow label={__("debug.content.uptime")}>
                    <InputText value={util.format_timespan(Math.round(state.uptime / 1000))}/>
                </FormRow>

                <FormRow label={__("debug.content.heap_free")}>
                    <InputText value={state.free_heap}/>
                </FormRow>

                <FormRow label={__("debug.content.heap_block")}>
                    <InputText value={state.largest_free_heap_block}/>
                </FormRow>

                <FormRow label={__("debug.content.psram_free")}>
                    <InputText value={state.free_psram}/>
                </FormRow>

                <FormRow label={__("debug.content.psram_block")}>
                    <InputText value={state.largest_free_psram_block}/>
                </FormRow>

                <FormRow label={__("debug.content.websocket_connection")} label_muted={__("debug.content.websocket_connection_muted")}>
                    <div class="input-group pb-2">
                        <Button variant="primary" className="form-control rounded-right mr-2" onClick={util.pauseWebSockets}>{__("debug.content.websocket_pause")}</Button>
                        <Button variant="primary" className="form-control rounded-left" onClick={util.resumeWebSockets}>{__("debug.content.websocket_resume")}</Button>
                    </div>
                </FormRow>
            </>
        )
    }
}

render(<Debug />, $('#debug')[0]);

export function init() {}

export function add_event_listeners(source: API.APIEventTarget) {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-debug').prop('hidden', !module_init.debug);
}
