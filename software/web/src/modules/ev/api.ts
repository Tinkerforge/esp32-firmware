export interface config {
    evs: {
        name: string;
        mac: string;
        capacity: number;
        charging_efficiency: number;
        user_id: number;
    }[];
}

export interface state {
    active_ev_index: number;
    name: string;
    mac: string;
    soc: number;
    capacity: number;
    charging_efficiency: number;
}

export type seen_macs = {
    mac: string;
    seen_at: number;
}[];

export interface inject_ev {
    mac: string;
}

export interface inject_soc {
    soc: number;
}
