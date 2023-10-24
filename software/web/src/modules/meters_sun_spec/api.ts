
export interface scan {
    host: string
    port: number
}

export interface scan_progress {
    progress: number
}

export interface scan_result {
    host: string
    port: number
    manufacturer_name: string
    model_name: string
    options: string
    version: string
    serial_number: string
    device_address: number
    model_id: number
}

export interface scan_done {
    host: string
    port: number
}

export type scan_log = string;
