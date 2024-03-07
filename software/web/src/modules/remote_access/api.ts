
export interface config {
    enable: boolean,
    email: string,
    password: string,
    relay_host: string,
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

export interface test {

}

export interface test1 {
    
}