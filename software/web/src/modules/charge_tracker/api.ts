interface Charge {
    timestamp_minutes: number
    charge_duration: number
    user_id: number
    energy_charged: number
}

export interface config {
    electricity_price: number
    pdf_text: string
}

export type last_charges = Charge[];

export interface current_charge {
    user_id: number,
    meter_start: number,
    evse_uptime_start: number,
    timestamp_minutes: number
}

export interface state {
    tracked_charges: number,
    first_charge_timestamp: number,
}

export interface remove_all_charges {
    do_i_know_what_i_am_doing: boolean
}
