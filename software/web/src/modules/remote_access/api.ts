
export interface config {
    enable: boolean,
    email: string,
    password: string,
    relay_host: string,
    relay_port: number,
    cert_id: number,
}

interface KeyObject {
    charger_public: string,
    charger_private: string,
    web_public: string,
    web_private: string,
    psk: string,
}

export enum RegistrationState {
    None,
    InProgress,
    Success,
    Error,
}

export interface registration_state {
    state: RegistrationState,
    message: string,
}

export interface register {
    config: config,
    login_key: string,
    secret_key: string,
    mgmt_charger_public: string,
    mgmt_charger_private: string,
    mgmt_psk: string,
    keys: KeyObject[],
}
