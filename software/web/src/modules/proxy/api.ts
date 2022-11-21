interface Device {
    uid: string,
    port: string,
    name: string
}


interface ErrorCounter {
    SpiTfpChecksum: number,
    SpiTfpFrame: number,
    TfpFrame: number,
    TfpUnexpected: number
}

export type devices = Device[];
export type error_counters = {[index:string]: ErrorCounter};

export interface config {
    authentication_secret: string
    listen_address: string
    listen_port: number
}
