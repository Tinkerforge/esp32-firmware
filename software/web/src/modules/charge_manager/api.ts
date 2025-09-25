import { ConfigChargeMode } from "modules/charge_manager/config_charge_mode.enum";
import { ZeroPhaseDecision } from "./zero_phase_decision.union";
import { OnePhaseDecision } from "./one_phase_decision.union";
import { ThreePhaseDecision } from "./three_phase_decision.union";
import { CurrentDecision } from "./current_decision.union";

import { CASState } from "./cas_state.enum";
import { CASError } from "./cas_error.enum";

type ChargerState = {
    s: CASState,
    e: CASError,
    ac: number,
    ap: number,
    sc: number,
    sp: number,
    lu: number,
    n: string,
    u: number,
    d0: ZeroPhaseDecision,
    d1: OnePhaseDecision,
    d3: ThreePhaseDecision,
    dc: CurrentDecision,
};

interface ServCharger {
    hostname: string;
    ip: string;
    display_name: string;
    error: number;
    proxy_of?: string;
}

export type scan_result = ServCharger[] | string;

export type state = {
    state: number,
    uptime: number,
    chargers: ChargerState[],
    l_raw: number[],
    l_min: number[],
    l_spread: number[],
    l_max_pv: number,
    alloc: number[]
}

interface ChargerConfig {
    host: string,
    name: string,
    rot: number
}

export interface config {
    enable_charge_manager: boolean,
    enable_watchdog: boolean,
    verbose: boolean,
    default_available_current: number,
    maximum_available_current: number,
    minimum_current_auto: boolean,
    minimum_current: number,
    minimum_current_1p: number,
    minimum_current_vehicle_type: number,
    requested_current_threshold: number,
    requested_current_margin: number,
    chargers: ChargerConfig[]
}

export interface available_current {
    current: number
}

export interface scan {
}

interface ChargerLowLevelState {
    b: number
    rc: number
    ae: number
    ls: number
    lp: number
    lw: number
    ip: number
}

export interface low_level_state {
    last_hyst_reset: number,
    wnd_min: number[],
    wnd_max: number[],
    chargers: ChargerLowLevelState[]
}

export interface low_level_config {
    global_hysteresis: number
    alloc_energy_rot_thres: number
    wakeup_time: number
    plug_in_time: number
    enable_current_factor_pct: number
    allocation_interval: number
}

export type charge_modes = ConfigChargeMode[];
export type supported_charge_modes = ConfigChargeMode[];

//APIPath:power_manager/
export interface charge_mode {
    mode: ConfigChargeMode;
}
