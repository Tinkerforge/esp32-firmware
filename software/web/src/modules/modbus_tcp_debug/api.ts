export interface transact_result {
    cookie: number;
    error: string;
    read_data: string;
}

export interface transact_transfer {
    cookie: number;
    direction: string;
    buffer: string;
}
