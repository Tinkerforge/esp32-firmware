
export interface config {
    enable: boolean,
    uuid: string,
    password: string,
    relay_host: string,
    relay_port: number,
    cert_id: number,
    users: user[],
}

interface user {
    id: number,
    email: string,
    uuid: string,
    public_key: string,
}

export interface remove_user {
    id: number,
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
    config: registration_config,
    login_key?: string,
    auth_token?: string,
    secret_key?: string,
    public_key?: string,
    user_uuid?: string,
    note: string,
    mgmt_charger_public: string,
    mgmt_charger_private: string,
    mgmt_psk: string,
    keys: KeyObject[],
}

export interface add_user {
    secret_key?: string,
    public_key?: string,
    note: string,
    email: string,
    login_key?: string,
    auth_token?: string,
    user_uuid?: string,
    wg_keys: KeyObject[],
}

interface registration_config {
    enable: boolean,
    relay_host: string,
    relay_port: number,
    email: string,
    cert_id: number,
}

export interface ping_state {
    packets_sent: number,
    packets_received: number,
    time_elapsed_ms: number,
}

export type start_ping = {}
export type stop_ping = {};

interface StateUser {
    state: number,
    user: number,
    connection: number,
    last_state_change: number,
}

export type state = StateUser[];
export type get_login_salt = registration_config;
export type get_secret_salt = registration_config;
export type config_update = registration_config;
export type login = register;
