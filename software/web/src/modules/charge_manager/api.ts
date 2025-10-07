import { ConfigChargeMode } from "modules/cm_networking/config_charge_mode.enum";
import { ZeroPhaseDecision } from "./zero_phase_decision.union";
import { OnePhaseDecision } from "./one_phase_decision.union";
import { ThreePhaseDecision } from "./three_phase_decision.union";
import { CurrentDecision } from "./current_decision.union";

import { CASState } from "./cas_state.enum";
import { CASError } from "./cas_error.enum";

type ChargerState = {
    /** state */
    s: CASState,
    /** error */
    e: CASError,
    /** allocated_current */
    ac: number,
    /** allocated_phases */
    ap: number,
    /** supported_current */
    sc: number,
    /** supported_phases */
    sp: number,
    /** last_update */
    lu: number,
    /** name */
    n: string,
    /** uid */
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
    /** bits
        0 cp_disc_supported
        1 cp_disc_state
        2 phase_switch_supported
        3 phases (2) */
    b: number
    /** requested_current */
    rc: number
    /** allocated_energy */
    ae: number
    /** last_switch_on */
    ls: number
    /** just_plugged_in_timestamp */
    lp: number
    /** last_wakeup */
    lw: number
    /** use_supported_current */
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
