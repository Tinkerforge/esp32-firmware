export interface config {
    update_url: string,
}

export interface available_updates {
    timestamp: number,
    cookie: number,
    beta: string,
    release: string,
    stable: string,
}

export interface check_for_updates {
}
