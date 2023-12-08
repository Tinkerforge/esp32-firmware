interface AuthorizedTag {
    user_id: number;
    tag_type: number;
    tag_id: string;
}

export interface config {
    authorized_tags: AuthorizedTag[];
}

interface SeenTag {
    tag_type: number;
    tag_id: string;
    last_seen: number;
}

export type seen_tags = SeenTag[];
