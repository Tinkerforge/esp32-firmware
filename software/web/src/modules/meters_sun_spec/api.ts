
export interface scan {
    host: string
    port: number
}

export interface scan_result {
    host: string
    port: number
    display_name: string
    device_address: number
    model_id: number
}

export interface scan_done {
    host: string
    port: number
}

export type scan_log = string;
