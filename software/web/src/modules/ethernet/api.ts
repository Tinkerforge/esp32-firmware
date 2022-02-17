export interface config {
    enable_ethernet: boolean,
    ip: string,
    gateway: string,
    subnet: string,
    dns: string,
    dns2: string,
}

export interface state {
    connection_state: number,
    ip: string,
    full_duplex: boolean,
    link_speed: number,
}
