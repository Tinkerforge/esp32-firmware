export interface scan {
    host: string;
    port: number;
    device_address_first: number;
    device_address_last: number;
    cookie: number;
}

export interface scan_continue {
    cookie: number;
}

export interface scan_abort {
    cookie: number;
}

export interface scan_progress {
    cookie: number;
    progress: number;
}

export interface scan_result {
    cookie: number;
    manufacturer_name: string;
    model_name: string;
    serial_number: string;
    device_address: number;
    model_id: number;
    model_instance: number;
}

export interface scan_done {
    cookie: number;
}

export interface scan_log {
    cookie: number;
    message: string;
}
