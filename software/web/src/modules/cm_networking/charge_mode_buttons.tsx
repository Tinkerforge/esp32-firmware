import * as API from "../../ts/api";
import { __ } from "../../ts/translation";
import { h } from "preact";
import { ButtonGroup, Button } from "react-bootstrap";
import { CheckCircle, Circle } from "react-feather";
import { ConfigChargeMode } from "./config_charge_mode.enum";

function ChargeModeButton(props: {current_mode: ConfigChargeMode, disabled?: boolean, mode: ConfigChargeMode, name: string, setMode: (x: ConfigChargeMode) => void}) {
    const selected = props.current_mode == props.mode;
    const disabled = props.disabled;
    return <Button
        style="display: flex;align-items: center;justify-content: center;"
        className="m-1 rounded-left rounded-right"
        variant={selected ? "success" : (disabled ? "secondary" : "primary")}
        disabled={disabled || selected}
        onClick={() => props.setMode(props.mode)}>
        {selected ? <CheckCircle size="20"/> : <Circle size="20"/>} <span>&nbsp;&nbsp;</span><span>{props.name}</span>
    </Button>
}

export function ChargeModeButtons(props: {mode: ConfigChargeMode, setMode: (x: ConfigChargeMode) => void, supportedModes: readonly ConfigChargeMode[]}) {
    let {mode, setMode, supportedModes} = props;

    const all_buttons = [
        <ChargeModeButton setMode={setMode} current_mode={mode} mode={ConfigChargeMode.Off}      name={__("cm_networking.status.mode_off")}/>,
        <ChargeModeButton setMode={setMode} current_mode={mode} mode={ConfigChargeMode.PV}       name={__("cm_networking.status.mode_pv")}/>,
        <ChargeModeButton setMode={setMode} current_mode={mode} mode={ConfigChargeMode.Min}      name={__("cm_networking.status.mode_min")}/>,
        <ChargeModeButton setMode={setMode} current_mode={mode} mode={ConfigChargeMode.MinPV}    name={__("cm_networking.status.mode_min_pv")}/>,
        <ChargeModeButton setMode={setMode} current_mode={mode} mode={ConfigChargeMode.Eco}      name={__("cm_networking.status.mode_eco")}/>,
        <ChargeModeButton setMode={setMode} current_mode={mode} mode={ConfigChargeMode.EcoPV}    name={__("cm_networking.status.mode_eco_pv")}/>,
        <ChargeModeButton setMode={setMode} current_mode={mode} mode={ConfigChargeMode.EcoMin}   name={__("cm_networking.status.mode_eco_min")}/>,
        <ChargeModeButton setMode={setMode} current_mode={mode} mode={ConfigChargeMode.EcoMinPV} name={__("cm_networking.status.mode_eco_min_pv")}/>,
        <ChargeModeButton setMode={setMode} current_mode={mode} mode={ConfigChargeMode.Fast}     name={__("cm_networking.status.mode_fast")}/>
    ];

    let modes = supportedModes.slice();

    // Add another button if a non-supported charge mode was selected via the API.
    if (mode != ConfigChargeMode.Default && modes.indexOf(mode) == -1)
        modes.push(mode);

    let buttons = all_buttons.filter(b => modes.indexOf(b.props.mode) >= 0);

    return <ButtonGroup className="flex-wrap m-n1" style="width: calc(100% + 0.5rem);">
        {buttons}
    </ButtonGroup>
}
