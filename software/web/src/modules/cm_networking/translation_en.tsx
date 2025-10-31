/** @jsxImportSource preact */
import { h } from "preact";
import { __ } from "../../ts/translation";
import { ConfigChargeMode } from "modules/cm_networking/config_charge_mode.enum";
let x = {
    "cm_networking": {
        "status": {
            "mode": "charge mode",
            "mode_fast": "Fast",
            "mode_off": "Off",
            "mode_pv": "PV",
            "mode_min_pv": "Min + PV",
            "mode_default": "Default mode",
            "mode_min": "Min",
            "mode_eco": "Eco",
            "mode_eco_pv": "Eco + PV",
            "mode_eco_min": "Eco + Min",
            "mode_eco_min_pv": "Eco + Min + PV",
            "mode_by_index": /*SFN*/ (mode: ConfigChargeMode, default_mode?: ConfigChargeMode) => {
                const modes = [
                    __("cm_networking.status.mode_fast"),
                    __("cm_networking.status.mode_off"),
                    __("cm_networking.status.mode_pv"),
                    __("cm_networking.status.mode_min_pv"),
                    __("cm_networking.status.mode_default"),
                    __("cm_networking.status.mode_min"),
                    __("cm_networking.status.mode_eco"),
                    __("cm_networking.status.mode_eco_pv"),
                    __("cm_networking.status.mode_eco_min"),
                    __("cm_networking.status.mode_eco_min_pv"),
                ];

                if (mode == 4 && default_mode !== undefined) {
                    return modes[mode] + " (" + modes[default_mode] + ")";
                }
                return modes[mode];
            }/*NF*/
        }
    }
}
