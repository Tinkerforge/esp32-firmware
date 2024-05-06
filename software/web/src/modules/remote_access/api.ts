
export interface config {
    enable: boolean,
    email: string,
    password: string,
    relay_host: string,
    relay_host_port: number,
    self_signed_cert_id: number,
}

export interface management_connection {
    internal_ip: string,
    internal_subnet: string,
    internal_gateway: string,
    remote_internal_ip: string,
    remote_host: string,
    remote_port: number,
    local_port: number,
    private_key: string,
    remote_public_key: string,
}

interface KeyObject {
    charger_address: string,
    charger_public: string,
    connection_no: number,
    web_address: string,
    web_private: string,
    web_private_nonce: string,
}

export interface register {
    login_key: string,
    remote_host: string,
    remote_port: number,
    charger_pub: string,
    id: string,
    name: string,
    wg_charger_ip: string,
    wg_server_ip: string,
    secret: string,
    secret_key: string,
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
