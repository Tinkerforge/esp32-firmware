export interface config {
    enable_mqtt: boolean;
    broker_host: string;
    broker_port: number;
    broker_username: string;
    broker_password: string;
    global_topic_prefix: string;
    client_name: string;
    interval: number;
    protocol: number;
    cert_id: number;
    client_cert_id: number;
    client_key_id: number;
    path: string;
}

export interface auto_discovery_config {
    auto_discovery_mode: number;
    auto_discovery_prefix: string;
}

export interface state {
    connection_state: number;
    last_error: number;
}
