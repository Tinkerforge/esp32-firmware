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
