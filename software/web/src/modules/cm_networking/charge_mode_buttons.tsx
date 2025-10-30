import * as API from "../../ts/api";
import { __ } from "../../ts/translation";
import { h } from "preact";
import { ButtonGroup, Button } from "react-bootstrap";
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
    return <Button
        style="display: flex;align-items: center;justify-content: center;"
        className="m-1 rounded-left rounded-right"
        variant={props.selected ? "success" : (props.disabled ? "secondary" : "primary")}
        disabled={props.disabled}
        onClick={() => props.setMode(props.mode)}>
        {(props.selected && props.disabled) ? <CheckCircle size="20"/> : <Circle size="20"/>} <span>&nbsp;&nbsp;</span><span>{props.name}</span>
    </Button>
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

    return <ButtonGroup className="flex-wrap m-n1" style="width: calc(100% + 0.5rem);">
        {buttons}
    </ButtonGroup>
}
