/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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
import { createRef, h } from "preact";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Monitor } from "react-feather";
import { Collapse } from "react-bootstrap";

export function EMDisplayNavbar() {
    return (
        <NavbarItem
            name="em_display"
            title={__("em_display.navbar.em_display")}
            hidden={false}
            symbol={<Monitor/>
            }
        />
    );
}

type EMDisplayConfig = API.getType["em_display/config"];

interface EMDisplayState {
}

export class EMDisplay extends ConfigComponent<"em_display/config", {}, EMDisplayState> {
    constructor() {
        super('em_display/config',
              __("em_display.script.save_failed"));
    }

    render(props: {}, state: EMDisplayState & EMDisplayConfig) {
        if (!util.render_allowed()) {
            return <SubPage name="em_display" />;
        }

        return (
            <SubPage name="em_display">
                <ConfigForm id="em_display_config_form"
                            title={__("em_display.content.em_display")}
                            isModified={this.isModified()}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={undefined}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("em_display.content.em_display")} label_muted={__("em_display.content.em_display_muted")}>
                        <Switch desc={__("em_display.content.em_display_desc")}
                                checked={state.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <Collapse in={state.enable}>
                        <div>
                        </div>
                    </Collapse>
                </ConfigForm>
        	</SubPage>
        );
    }
}

export function init() {
}
