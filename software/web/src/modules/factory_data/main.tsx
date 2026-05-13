/* esp32-firmware
 * Copyright (C) 2026 Matthias Bolte <matthias@tinkerforge.com>
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

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { h } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputNumber } from "../../ts/components/input_number";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Box } from "react-feather";

export function FactoryDataNavbar() {
    return <NavbarItem name="factory_data" module="factory_data" title="Factory Data" symbol={<Box />} />;
}

type FactoryDataConfig = API.getType["factory_data/config"];

export class FactoryData extends ConfigComponent<'factory_data/config'> {
    constructor() {
        super('factory_data/config',
              () => "Failed to save the factory data settings.");
    }

    render(props: {}, state: Readonly<FactoryDataConfig>) {
        if (!util.render_allowed())
            return <SubPage name="factory_data" />;

        const factory_data_state = API.get("factory_data/state");

        return (
            <SubPage name="factory_data" title="Factory Data">
                <SubPage.Status>
                    <FormRow label="SKU">
                        <InputText value={factory_data_state.sku} />
                    </FormRow>
                    <FormRow label="SKU product">
                        <InputText value={factory_data_state.sku_product} />
                    </FormRow>
                    <FormRow label="SKU generation">
                        <InputText value={factory_data_state.sku_generation} />
                    </FormRow>
                    <FormRow label="SKU model">
                        <InputText value={factory_data_state.sku_model} />
                    </FormRow>
                    <FormRow label="SKU material">
                        <InputText value={factory_data_state.sku_material} />
                    </FormRow>
                    <FormRow label="SKU type-2 power">
                        <InputText value={factory_data_state.sku_type2_power} />
                    </FormRow>
                    <FormRow label="SKU type-2 length">
                        <InputText value={factory_data_state.sku_type2_length} />
                    </FormRow>
                    <FormRow label="SKU engraving">
                        <InputText value={factory_data_state.sku_engraving} />
                    </FormRow>
                </SubPage.Status>

                <SubPage.Config id="factory_data_config_form"
                                isDirty={this.isDirty()}
                                onSave={this.save}
                                onDirtyChange={this.setDirty}>
                    <FormRow label="SKU override">
                        <InputText maxLength={18}
                                   value={state.sku_override}
                                   onValue={this.set("sku_override")}/>
                    </FormRow>

                </SubPage.Config>
            </SubPage>
        );
    }
}

export function pre_init() {
}

export function init() {
}
