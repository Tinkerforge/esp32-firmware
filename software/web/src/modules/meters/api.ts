//APIPath:meters/
export interface _0_state {
    state: number,
    type: number
}

export type _0_value_ids = number[];

export type _0_values = number[];

export interface _0_phases {
    phases_active: boolean[],
    phases_connected: boolean[]
}

export interface _0_live {
    offset: number,
    samples_per_second: number,
    samples: number[],
}

export interface _0_live_samples {
    samples_per_second: number,
    samples: number[],
}

export interface _0_history {
    offset: number,
    samples: number[],
}

export interface _0_history_samples {
    samples: number[],
}

export interface _0_reset {
}
