export interface config {
    update_url: string,
    cert_id: number,
}

export interface state {
    check_timestamp: number,
    check_error: string,
    update_version: string,
}

export interface check_for_update {
}

export interface install_firmware {
    version: string,
}

export interface override_signature {
    cookie: number,
}
