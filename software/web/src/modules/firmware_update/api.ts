export interface config {
    update_url: string,
    cert_id: number,
}

export interface state {
    publisher: string,
    check_timestamp: number,
    check_state: number,
    update_version: string,
    install_progress: number,
    install_state: number,
    rolled_back_version: string,
}

export interface check_for_update {
}

export interface install_firmware {
    version: string,
}

export interface override_signature {
    cookie: number,
}

export interface clear_rolled_back_version {
}

export interface validate {
}
