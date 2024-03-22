
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

export interface remote_connection_config {
    connections: management_connection[],
}

export interface test {

}

export interface test1 {

}

export interface test2 {

}
