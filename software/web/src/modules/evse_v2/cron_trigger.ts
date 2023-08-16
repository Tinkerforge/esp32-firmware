export interface EvseSdCronTrigger {
    0: 7,
    1: {
        high: boolean
    }
}

export interface EvseGpioCronTrigger {
    0: 8,
    1: {
        high: boolean
    }
}

export interface EvseButtonCronTrigger {
    0: 4,
    1: {
        button_pressed: boolean
    }
}