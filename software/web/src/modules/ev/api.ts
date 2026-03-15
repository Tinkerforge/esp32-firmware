export type seen_macs = {
    mac: string;
    last_seen: number;
}[];

export interface inject_ev {
    mac: string;
}

export interface inject_soc {
    soc: number;
}
