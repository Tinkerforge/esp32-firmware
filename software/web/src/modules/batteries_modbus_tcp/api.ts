/*export interface test_start {
    slot: number;
    host: string;
    port: number;
    table: ...
    cookie: number;
}*/

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
