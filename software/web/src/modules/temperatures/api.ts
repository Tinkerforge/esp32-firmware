export interface config {
    enable: boolean;
    source: number;
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
    first_date: number;
    temperatures: number[];
}
