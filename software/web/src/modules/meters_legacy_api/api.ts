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

export type all_values = number[];

export interface reset {

}
