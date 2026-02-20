export interface config {
    enable: boolean;
    api_url: string;
    lat: number;
    lon: number;
    cert_id: number;
}

export interface state {
    last_sync: number;
    last_check: number;
    next_check: number;
}

export interface temperatures {
    today_date: number;
    today_min: number;
    today_max: number;
    today_avg: number;
    tomorrow_date: number;
    tomorrow_min: number;
    tomorrow_max: number;
    tomorrow_avg: number;
}
