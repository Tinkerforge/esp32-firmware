import { P14aEnwgSource } from "./p14a_enwg_source.enum";

export type SourceInput = [
    P14aEnwgSource.Input,
    {
        limit_on_close: boolean;
        limit_w: number;
        input_index: number;
    },
];

type SourceEEBus = [
    P14aEnwgSource.EEBus,
    {},
];

type SourceAPI = [
    P14aEnwgSource.API,
    {},
];

export interface config {
    enable: boolean;
    source: SourceInput | SourceEEBus | SourceAPI;
    limit_charger: boolean;
    limit_charge_manager: boolean;
    limit_heating: boolean;
    heating_max_power: number;
}

export interface state {
    active: boolean;
    limit_w: number;
}

export interface control {
    active: boolean;
    limit_w: number;
}

export interface control_update {
    active: boolean;
    limit_w: number;
}
