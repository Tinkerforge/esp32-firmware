export interface config {
    enable: boolean,
    use_dhcp: boolean,
    timezone: string,
    server: string
    server2: string
}

export interface state {
    synced: boolean
}
