export interface config {
    update_url: string,
    cert_id: number,
}

export interface available_updates {
    timestamp: number,
    error: string,
    beta: string,
    release: string,
    stable: string,
}

export interface check_for_updates {
}
