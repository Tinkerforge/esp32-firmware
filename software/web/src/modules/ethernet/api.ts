export interface config {
    enable_ethernet: boolean,
    ip: number[],
    gateway: number[],
    subnet: number[],
    dns: number[],
    dns2: number[],
}

export interface state {
    connection_state: number,
    ip: number[],
    full_duplex: boolean,
    link_speed: number,
}
