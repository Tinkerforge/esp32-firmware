export interface config {
    update_url: string,
    cert_id: number,
}

export interface state {
    publisher: string,
    check_timestamp: number,
    check_state: number,
    update_version: string,
    install_progress: number, // [0..100]
    install_state: number,
    running_partition: string,
    app0_state: string,
    app0_version: string,
    app1_state: string,
    app1_version: string,
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

// for historical reasons the flash_firmware API command is not
// inside the firmware_update/ module namespace. therefore the
// flash_firmware state stays outside too
//APIPath:
export interface flash_firmware_state {
    progress: number, // [0..100]
}
