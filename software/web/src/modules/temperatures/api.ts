export interface config {
    enable: boolean;
    source: number;
    api_url: string;
    lat: number;
    long: number;
    cert_id: number;
}

export interface state {
    last_sync: number;
    last_check: number;
    next_check: number;
    current: number;
    today_min: number;
    today_max: number;
    today_avg: number;
    tomorrow_min: number;
    tomorrow_max: number;
    tomorrow_avg: number;
}

export interface temperatures {
    first_date: number;
    temperatures: number[];
}
