import { ConfigChargeMode } from "modules/charge_manager/config_charge_mode.enum";
import { AllocatorDecision } from "modules/charge_manager/allocator_decision.enum";
import { GlobalAllocatorDecision } from "modules/charge_manager/global_allocator_decision.enum";

type alloc_desc =
{ d: AllocatorDecision.None0 } |
{ d: AllocatorDecision.WaitingForRotation0 } |
{ d: AllocatorDecision.ShuttingDownUnknown0 } |
{ d: AllocatorDecision.ShuttingDownNotActive0 } |
{ d: AllocatorDecision.ShuttingDownRotatedForB10 } |
{ d: AllocatorDecision.ShuttingDownRotatedForHigherPrio0 } |
{ d: AllocatorDecision.ShuttingDownOffOrError0 } |
{ d: AllocatorDecision.WelcomeChargeUntil2, d1: number, d2: number } |
{ d: AllocatorDecision.ShuttingDownPhaseOverload2, d1: number, d2: number } |
{ d: AllocatorDecision.CantActivatePhaseMinimum3, d1: number, d2: number, d3: number } |
{ d: AllocatorDecision.Activating1, d1: number } |
{ d: AllocatorDecision.PhaseSwitching0 } |
{ d: AllocatorDecision.PhaseSwitchingUnblockedAt2, d1: number, d2: number } |
{ d: AllocatorDecision.WakingUp0 };

type global_alloc_desc =
{ d: GlobalAllocatorDecision.None0 } |
{ d: GlobalAllocatorDecision.NextRotationAt2, d1: number, d2: number } |
{ d: GlobalAllocatorDecision.PVExcessOverloadedHysteresisElapsesAt3, d1: number, d2: number, d3: number } |
{ d: GlobalAllocatorDecision.HysteresisElapsesAt2, d1: number, d2: number };

type ChargerState = {
    s: number,
    e: number,
    ac: number,
    ap: number,
    sc: number,
    sp: number,
    lu: number,
    n: string,
    u: number
} & alloc_desc;

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
} & global_alloc_desc

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

//APIPath:power_manager/
export interface charge_mode {
    mode: ConfigChargeMode;
}
