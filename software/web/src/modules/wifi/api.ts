interface WifiInfo {
    ssid: string,
    bssid: string,
    rssi: number,
    channel: number,
    encryption: number
}

export type scan_results = WifiInfo[] | string;

export interface sta_config {
    enable_sta: boolean,
    ssid: string,
    bssid: number[],
    bssid_lock: boolean,
    passphrase: string,
    ip: number[],
    gateway: number[],
    subnet: number[],
    dns: number[],
    dns2: number[],
}

export interface ap_config {
    enable_ap: boolean,
    ap_fallback_only: boolean,
    ssid: string,
    hide_ssid: boolean,
    passphrase: string,
    channel: number,
    ip: number[],
    gateway: number[],
    subnet: number[]
}

export interface state {
    connection_state: number,
    ap_state: number,
    ap_bssid: string,
    sta_ip: number[],
    sta_rssi: number,
    sta_bssid: string
}
