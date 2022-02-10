interface Charge {
    timestamp_minutes: number
    charge_duration: number
    user_id: number
    energy_charged: number
}

export type last_charges = Charge[];
