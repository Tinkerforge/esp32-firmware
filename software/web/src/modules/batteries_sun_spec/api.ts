export interface test_start {
    slot: number;
    host: string;
    port: number;
    device_address: number;
    manufacturer_name: string;
    model_name: string;
    serial_number: string;
    model_instance: number;
    force_charge_rate: number;
    force_discharge_rate: number;
    mode: number;
    cookie: number;
}

export interface test_keep_alive {
    cookie: number;
}

export interface test_abort {
    cookie: number;
}

export interface test_done {
    cookie: number;
}

export interface test_log {
    cookie: number;
    message: string;
}
