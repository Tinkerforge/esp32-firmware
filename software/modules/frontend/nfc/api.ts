interface AuthorizedTag {
    user_id: number,
    tag_type: number,
    tag_id: number[]
}

export interface config {
    authorized_tags: AuthorizedTag[]
}

interface SeenTag {
    tag_type: number,
    tag_id: number[]
    last_seen: number
}

export type seen_tags = SeenTag[];
