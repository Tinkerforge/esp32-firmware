export interface config {
    enable: boolean,
    use_dhcp: boolean,
    timezone: string,
    server: string
}

export interface state {
    synced: boolean,
    server: string
}
