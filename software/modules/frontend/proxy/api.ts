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
