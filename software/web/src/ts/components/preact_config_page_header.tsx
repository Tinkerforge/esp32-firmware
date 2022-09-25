/* esp32-firmware
 * Copyright (C) 2022 Matthias Bolte <matthias@tinkerforge.com>
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

import { h, Component } from "preact";
import { __ } from "../translation";

export interface PreactConfigPageHeaderProps {
    title: string,
    config_form_id: string
    save_disabled: boolean
}

export class PreactConfigPageHeader extends Component<PreactConfigPageHeaderProps, any> {
    render(props: PreactConfigPageHeaderProps) {
        return (
            <div class="row sticky-under-top mb-3 pt-3">
                <div class="col-xl-8 d-flex justify-content-between pb-2 border-bottom tab-header-shadow">
                    <h1 class="h2" dangerouslySetInnerHTML={{__html: props.title}}></h1>
                    <button type="submit" form={props.config_form_id} class="btn btn-primary mb-2" disabled={props.save_disabled}>
                        {__("component.config_page_header.save")}
                        <span class="ml-2 spinner-border spinner-border-sm" role="status" style="vertical-align: middle;" hidden></span>
                    </button>
                </div>
            </div>
        );
    }
}
