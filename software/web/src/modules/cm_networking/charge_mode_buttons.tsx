/* esp32-firmware
 * Copyright (C) 2025 Erik Fleckstein <erik@tinkerforge.com>
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
import { __ } from "../../ts/translation";
import { h } from "preact";
import { Button } from "react-bootstrap";
import { CheckCircle, Circle } from "react-feather";
import { ConfigChargeMode } from "./config_charge_mode.enum";

/*
selected disabled   variant   clickable   symbol
    0       0       primary     yes     Circle
    0       1       secondary   no      Circle
    1       0       success     yes     Circle
    1       1       success     no      CheckCircle
*/

function ChargeModeButton(props: {selected: boolean, disabled: boolean, mode: ConfigChargeMode, name: string, setMode: (x: ConfigChargeMode) => void}) {
    return <div class="col text-nowrap"><Button
        className="d-flex align-items-center justify-content-center w-100"
        variant={props.selected ? "success" : (props.disabled ? "secondary" : "primary")}
        disabled={props.disabled}
        onClick={() => props.setMode(props.mode)}>
        {(props.selected && props.disabled) ? <CheckCircle size="20"/> : <Circle size="20"/>} <span class="ms-2">{props.name}</span>
    </Button></div>;
}

export function ChargeModeButtons(props: {mode: ConfigChargeMode, setMode: (x: ConfigChargeMode) => void, supportedModes: readonly ConfigChargeMode[], modeEnabled: boolean}) {
    const {mode, setMode, supportedModes, modeEnabled} = props;

    const all_buttons = [
        ConfigChargeMode.Off,
        ConfigChargeMode.PV,
        ConfigChargeMode.Min,
        ConfigChargeMode.MinPV,
        ConfigChargeMode.Eco,
        ConfigChargeMode.EcoPV,
        ConfigChargeMode.EcoMin,
        ConfigChargeMode.EcoMinPV,
        ConfigChargeMode.Fast,
    ].map(m =>
        <ChargeModeButton mode={m}
                          selected={m == mode}
                          disabled={supportedModes.indexOf(m) < 0 || (m == mode && !modeEnabled) }
                          setMode={setMode}
                          name={__("cm_networking.status.mode_by_index")(m)}/>);

    let modes = supportedModes.slice();

    // Add another button if a non-supported charge mode was selected via the API.
    if (mode != ConfigChargeMode.Default && modes.indexOf(mode) == -1)
        modes.push(mode);

    let buttons = all_buttons.filter(b => modes.indexOf(b.props.mode) >= 0);

    return <div class="row g-2">{buttons}</div>;
}
