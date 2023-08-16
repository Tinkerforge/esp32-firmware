export interface EvseCronAction {
    0: 3,
    1: {
        current: number
    }
}

export interface EvseLedCronAction {
    0: 4,
    1: {
        state: number,
        duration: number
    }
}