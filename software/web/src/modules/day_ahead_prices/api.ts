export interface config {
    enable: boolean;
    api_url: string;
    region: number;
    resolution: number;
    cert_id: number;
    grid_costs_and_taxes: number;
    supplier_markup: number;
    supplier_base_fee: number;
}

export interface state {
    last_sync: number;
    last_check: number;
    next_check: number;
    current_price: number;
}

export interface prices {
    first_date: number;
    resolution: number;
    prices: number[];
}