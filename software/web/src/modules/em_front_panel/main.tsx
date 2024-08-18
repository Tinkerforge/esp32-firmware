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

export function EMFrontPanelNavbar() {
    return (
        <NavbarItem
            name="front_panel"
            title={__("em_front_panel.navbar.em_front_panel")}
            hidden={false}
            symbol={<Monitor/>
            }
        />
    );
}

type EMFrontPanelConfig = API.getType["front_panel/config"];

interface EMFrontPanelState {
}

export class EMFrontPanel extends ConfigComponent<"front_panel/config", {}, EMFrontPanelState> {
    constructor() {
        super('front_panel/config',
              __("em_front_panel.script.save_failed"));
    }

    render(props: {}, state: EMFrontPanelState & EMFrontPanelConfig) {
        if (!util.render_allowed()) {
            return <SubPage name="front_panel" />;
        }

        return (
            <SubPage name="front_panel">
                <ConfigForm id="em_front_panel_config_form"
                            title={__("em_front_panel.content.em_front_panel")}
                            isModified={this.isModified()}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={undefined}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("em_front_panel.content.em_front_panel")} label_muted={__("em_front_panel.content.em_front_panel_muted")}>
                        <Switch desc={__("em_front_panel.content.em_front_panel_desc")}
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
