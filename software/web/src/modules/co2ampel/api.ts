//APIPath:co2/

export interface config {
    temperature_offset: number
}

export interface state {
    co2: number
    temperature: number
    humidity: number
    air_pressure: number
    led: number
}

export interface stop_blink {

}
