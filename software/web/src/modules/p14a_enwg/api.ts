export interface config {
    enable: boolean;
    source: number;
    limit: number;
    active_on_close: boolean;
    input_index: number;
    this_charger: boolean;
    managed_chargers: boolean;
    heating: boolean;
    heating_max_power: number;
}

export interface state {
    active: boolean;
    limit: number;
}

export interface control {
    active: boolean;
    limit: number;
}

export interface control_update {
    active: boolean;
    limit: number;
}
