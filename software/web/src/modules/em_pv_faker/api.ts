export interface state {
    illuminance: number,
    fake_power:  number,
}

export interface config {
    auto_fake:   boolean,
    topic:       string,
    peak_power:  number,
    zero_at_lux: number,
    peak_at_lux: number,
}

export interface runtime_config {
    manual_power: number,
}
