export interface config {
    enable_ethernet: boolean;
    ip: string;
    gateway: string;
    subnet: string;
    dns: string;
    dns2: string;
    ipv6: {
        ip: string;
        gateway: string;
        subnet: string;
        dns?: string;
        dns2?: string;
    }
    enable_ipv6: boolean;
}

export interface state {
    connection_state: number;
    connection_start: number;
    connection_end: number;
    mac: string;
    ip: string;
    subnet: string;
    full_duplex: boolean;
    link_speed: number;
    ip6_link_local: string;
    ip6_global: string;
}
