export interface ipv6_address {
    addr: string;
    flags: number;
}

export interface config {
    enable_ethernet: boolean;
    ip: string;
    gateway: string;
    subnet: string;
    dns: string;
    dns2: string;
    enable_ipv6: boolean;
    ip6: ipv6_address[];
}

export interface state {
    connection_state: number;
    connection_start: number;
    connection_end: number;
    mac: string;
    ip: string;
    subnet: string;
    ip6: ipv6_address[];
    full_duplex: boolean;
    link_speed: number;
    disable_countdown: number;
}
