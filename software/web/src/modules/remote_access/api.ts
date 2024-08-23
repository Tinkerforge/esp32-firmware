
export interface config {
    enable: boolean,
    email: string,
    password: string,
    relay_host: string,
    relay_host_port: number,
    cert_id: number,
}

export interface management_connection {
    private_key: string,
    psk: string,
    remote_public_key: string,
}

interface KeyObject {
    charger_public: string,
    connection_no: number,
    web_private: string,
    psk: string,
}

export interface register {
    login_key: string,
    remote_host: string,
    charger_pub: string,
    psk: string,
    id: string,
    name: string,
    secret: string,
    secret_key: string,
    secret_nonce: string,
    config: config,
    keys: KeyObject[],
}

export interface remote_connection_config {
    connections: management_connection[],
}

export interface test {

}

export interface test1 {

}

export interface test2 {

}
