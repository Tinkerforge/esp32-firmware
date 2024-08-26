
export interface config {
    enable: boolean,
    email: string,
    password: string,
    relay_host: string,
    relay_host_port: number,
    cert_id: number,
}

interface KeyObject {
    charger_public: string,
    charger_private: string,
    web_public: string,
    web_private: string,
    psk: string,
}

export interface register {
    config: config,
    login_key: string,
    secret: string,
    secret_key: string,
    secret_nonce: string,
    mgmt_charger_public: string,
    mgmt_charger_private: string,
    mgmt_psk: string,
    keys: KeyObject[],
}
