//APIPath:meter/
export interface state {
    state: number,
    type: number
}

export interface values {
    power: number,
    energy_rel: number,
    energy_abs: number
}

export interface phases {
    phases_active: boolean[],
    phases_connected: boolean[]
}

interface WARP2MeterErrorCounters {
    local_timeout: number,
    global_timeout: number,
    illegal_function: number,
    illegal_data_access: number,
    illegal_data_value: number,
    slave_device_failure: number,
}

interface WARP1MeterErrorCounters {
    meter: number,
    bricklet: number,
    bricklet_reset: number,
}

export type all_values = number[];

export interface live {
    samples_per_second: number,
    samples: number[],
}

export interface live_samples {
    samples_per_second: number,
    samples: number[],
}

export interface history {
    offset: number,
    samples: number[],
}

export interface history_samples {
    samples: number[],
}

export interface reset {

}
