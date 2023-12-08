export interface scan {
    host: string;
    port: number;
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
    options: string;
    version: string;
    serial_number: string;
    device_address: number;
    model_id: number;
}

export interface scan_done {
    cookie: number;
}

export interface scan_log {
    cookie: number;
    message: string;
}
