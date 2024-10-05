//APIPath:energy_manager/

export interface history_wallbox_5min_changed {
    uid: number;
    year: number;
    month: number;
    day: number;
    hour: number;
    minute: number;
    flags: number;
    power: number;
}

export interface history_wallbox_daily_changed {
    uid: number;
    year: number;
    month: number;
    day: number;
    energy: number;
}

export interface history_energy_manager_5min_changed {
    year: number;
    month: number;
    day: number;
    hour: number;
    minute: number;
    flags: number;
    power: number[];
    price: number;
}

export interface history_energy_manager_daily_changed {
    year: number;
    month: number;
    day: number;
    energy_import: number[];
    energy_export: number[];
    price_min: number;
    price_avg: number;
    price_max: number;
}
