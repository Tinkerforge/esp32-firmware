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

import * as API  from "../../ts/api";
import * as util from "../../ts/util";
import { __ }    from "../../ts/translation";
import { h, Component } from "preact";
import { Button, ButtonGroup } from "react-bootstrap";
import { FormRow        } from "../../ts/components/form_row";
import { IndicatorGroup } from "../../ts/components/indicator_group";
import { StatusSection  } from "../../ts/components/status_section";

export class EnergyManagerStatus extends Component {
    render() {
        if (!util.render_allowed())
            return <StatusSection name="energy_manager" />

        if (!API.hasFeature("energy_manager")) {
            return <StatusSection name="energy_manager">
                <FormRow label={__("energy_manager.status.status")}>
                    <IndicatorGroup
                        value={0}
                        items={[
                            ["danger", __("energy_manager.status.no_bricklet")],
                        ]}
                    />
                </FormRow>
            </StatusSection>
        }

        let status = API.get('energy_manager/state');

        let error_flags_ok        = status.error_flags == 0;
        let error_flags_config    = status.error_flags & 0x80000000;
        let error_flags_internal  = status.error_flags & 0x7F000000;
        let error_flags_contactor = status.error_flags & 0x00010000;
        let error_flags_network   = status.error_flags & 0x00000002;

        return <StatusSection name="energy_manager">
            <FormRow label={__("energy_manager.status.status")}>
                <ButtonGroup className="flex-wrap w-100">
                    <Button disabled
                        variant={(error_flags_ok ? "" : "outline-") + "success"}>
                        {__("energy_manager.status.error_ok")}
                    </Button>
                    <Button disabled
                        variant={(error_flags_network ? "" : "outline-") + "warning"}>
                        {__("energy_manager.status.error_network")}
                    </Button>
                    <Button disabled
                        variant={(error_flags_contactor ? "" : "outline-") + "danger"}>
                        {__("energy_manager.status.error_contactor")}
                    </Button>
                    <Button disabled
                        variant={(error_flags_internal ? "" : "outline-") + "danger"}>
                        {__("energy_manager.status.error_internal")}
                    </Button>
                    <Button disabled
                        variant={(error_flags_config ? "" : "outline-") + "danger"}>
                        {__("energy_manager.status.error_config")}
                    </Button>
                </ButtonGroup>
            </FormRow>
        </StatusSection>
    }
}

export function init() {
}
