/*export interface test {
    slot: number;
    host: string;
    port: number;
    cookie: number;
}*/

export interface test_continue {
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
