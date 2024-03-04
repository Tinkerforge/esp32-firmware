export interface config {
    enable: boolean;
    make_default_interface: boolean;
    local_port: number;
    internal_ip: string;
    internal_subnet: string;
    internal_gateway: string;
    remote_host: string;
    remote_port: number;
    private_key: string;
    remote_public_key: string;
    preshared_key: string;
    allowed_ip: string;
    allowed_subnet: string;
}

export interface state {
    state: number;
}
