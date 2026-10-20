export interface scan_start {
    ip4_first: string;
    ip4_last: string;
    port: number;
    cookie: number;
}

export interface scan_keep_alive {
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
    ip4: string;
    port: number;
    open: boolean;
}

export interface scan_done {
    cookie: number;
}

export interface scan_log {
    cookie: number;
    message: string;
}
