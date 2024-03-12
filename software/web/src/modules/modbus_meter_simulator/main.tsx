/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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
import { __ } from "../../ts/translation";
import { METERS_SLOTS } from "../../build";
import { h, Fragment } from "preact";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm      } from "../../ts/components/config_form";
import { FormRow         } from "../../ts/components/form_row";
import { InputSelect     } from "../../ts/components/input_select";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { LogOut } from "react-feather";

export function ModbusMeterSimulatorNavbar() {
    return <NavbarItem name="modbus_meter_simulator" module="modbus_meter_simulator" title={__("modbus_meter_simulator.navbar.entry")} symbol={<LogOut />} />;
}

export class ModbusMeterSimulator extends ConfigComponent<'modbus_meter_simulator/config'> {
    constructor() {
        super('modbus_meter_simulator/config',
            __("modbus_meter_simulator.script.save_failed"),
            __("modbus_meter_simulator.script.reboot_content_changed"));
    }

    render(props: {}, s: Readonly<API.getType['modbus_meter_simulator/config']>) {
        if (!util.render_allowed())
            return <SubPage name="modbus_meter_simulator" />;

        let meter_names: [string, string][] = [];
        for (let i = 0; i < METERS_SLOTS; i++) {
            const meter = API.get_unchecked(`meters/${i}/config`);
            if (meter[1]) {
                meter_names.push([i.toString(), meter[1].display_name]);
            }
        }

        return <SubPage name="modbus_meter_simulator">
            <ConfigForm id="modbus_meter_simulator_config_form" title={__("modbus_meter_simulator.content.page_header")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>

                <FormRow label={__("modbus_meter_simulator.content.meter_type")} label_muted={__("modbus_meter_simulator.content.meter_type_muted")}>
                    <InputSelect
                        items={[
                            ["0", __("modbus_meter_simulator.content.meter_type_no_meter")],
                            ["1", "SDM72DM"    ],
                            ["2", "SDM630"     ],
                            ["3", "SDM72DMv2"  ],
                            ["4", "SDM72CTM"   ],
                            ["5", "SDM630MCTv2"],
                        ]}
                        value={s.meter_type}
                        onValue={(v) => this.setState({meter_type: parseInt(v)})}
                    />
                </FormRow>

                <FormRow label={__("modbus_meter_simulator.content.source_meter_slot")}>
                    <InputSelect
                        items={meter_names}
                        value={s.source_meter_slot}
                        onValue={(v) => this.setState({source_meter_slot: parseInt(v)})}
                    />
                </FormRow>

            </ConfigForm>
        </SubPage>
    }
}

export function init() {
}
