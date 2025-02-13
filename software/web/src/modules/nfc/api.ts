interface AuthorizedTag {
    user_id: number;
    tag_type: number;
    tag_id: string;
}

export interface config {
    authorized_tags: AuthorizedTag[];
    deadtime_post_start: number
}

interface SeenTag {
    tag_type: number;
    tag_id: string;
    last_seen: number;
}

export type seen_tags = SeenTag[];

export interface inject_tag {
    tag_type: number;
    tag_id: string;
}

export type inject_tag_start = inject_tag;

export type inject_tag_stop = inject_tag;
