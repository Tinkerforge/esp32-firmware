interface WifiInfo {
    ssid: string,
    bssid: string,
    rssi: number,
    channel: number,
    encryption: number
}

export enum EapConfigID {
    None = 0,
    TLS = 1,
    PEAP_TTLS = 2
}

type EapConfigNone = [
    EapConfigID.None,
    {}
]

export type EapConfigCertificate = [
    EapConfigID.TLS,
    {
        identity: string,
        ca_cert_id: number,
        client_cert_id: number,
        client_key_id: number
    }
]

export type EapConfigLogin = [
    EapConfigID.PEAP_TTLS,
    {
        identity: string,
        ca_cert_id: number,
        username: string,
        password: string
    }
]

export type scan_results = WifiInfo[] | string;

export interface sta_config {
    enable_sta: boolean,
    ssid: string,
    bssid: number[],
    bssid_lock: boolean,
    passphrase: string,
    ip: string,
    gateway: string,
    subnet: string,
    dns: string,
    dns2: string,
    wpa_eap_config: EapConfigNone | EapConfigCertificate | EapConfigLogin
}

export interface ap_config {
    enable_ap: boolean,
    ap_fallback_only: boolean,
    ssid: string,
    hide_ssid: boolean,
    passphrase: string,
    channel: number,
    ip: string,
    gateway: string,
    subnet: string
}

export interface state {
    connection_state: number,
    connection_start: number,
    connection_end: number,
    ap_state: number,
    ap_bssid: string,
    sta_ip: string,
    sta_subnet: string,
    sta_rssi: number,
    sta_bssid: string
}

export interface scan {

}
