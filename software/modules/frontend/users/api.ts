interface User {
    id: number,
    roles: number,
    current: number,
    display_name: string,
    username: string,
    digest_hash: string,
}

export interface config {
    users: User[],
    next_user_id: number,
    http_auth_enabled: boolean
}

export interface charge_info {
    id: number,
    meter_start: number,
    evse_uptime_start: number
}
