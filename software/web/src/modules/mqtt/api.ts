export interface config {
    enable_mqtt: boolean,
    broker_host: string,
    broker_port: number,
    broker_username: string,
    broker_password: string,
    global_topic_prefix: string
    client_name: string,
    interval: number
}

export interface auto_discovery_config {
    enable_auto_discovery: boolean,
    auto_discovery_prefix: string
}

export interface state {
    connection_state: number
    last_error: number
}
