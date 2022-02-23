interface Charge {
    timestamp_minutes: number
    charge_duration: number
    user_id: number
    energy_charged: number
}

export type last_charges = Charge[];

export interface current_charge {
    user_id: number,
    meter_start: number,
    evse_uptime_start: number,
    timestamp_minutes: number
}
