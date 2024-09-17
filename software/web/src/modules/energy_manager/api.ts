import { EMV1State, EMV2State, EMV1LowLevelState, EMV2LowLevelState } from "./types"

export interface low_level_state extends Partial<EMV1LowLevelState>, Partial<EMV2LowLevelState> {
    consecutive_bricklet_errors: number;
    input_voltage: number;
    uptime: number;
}

export interface state extends Partial<EMV1State>, Partial<EMV2State> {
    error_flags: number;
    config_error_flags: number;
    em_version: number;
}

export interface config {
    contactor_installed: boolean;
}

export interface reflash {
}

export interface reset {
}
