export type EMV1LowLevelState = {
    contactor: boolean;
    led_rgb: number[];
    contactor_check_state: number;
}

export type EMV2LowLevelState = {
}

export type EMV1State = {
    phases_switched: number;
    input3_state: boolean;
    input4_state: boolean;
    relay_state: boolean;
}

export type EMV2State = {
    inputs: boolean[];
    sg_ready_outputs: boolean[];
    relays: boolean[];
}
