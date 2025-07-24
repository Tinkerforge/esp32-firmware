import { ConfigChargeMode } from "modules/charge_manager/config_charge_mode.enum";
import { AllocatorDecision } from "modules/charge_manager/allocator_decision.enum";
import { GlobalAllocatorDecision } from "modules/charge_manager/global_allocator_decision.enum";

type alloc_desc =
[ AllocatorDecision.None0 ] |
[ AllocatorDecision.WaitingForRotation0 ] |
[ AllocatorDecision.ShuttingDownUnknown0 ] |
[ AllocatorDecision.ShuttingDownNotActive0 ] |
[ AllocatorDecision.ShuttingDownRotatedForB10 ] |
[ AllocatorDecision.ShuttingDownRotatedForHigherPrio0 ] |
[ AllocatorDecision.ShuttingDownOffOrError0 ] |
[ AllocatorDecision.WelcomeChargeUntil2, timestamp_s: number ] |
[ AllocatorDecision.ShuttingDownPhaseOverload2, [phase: number, overload_ma: number ] ] |
[ AllocatorDecision.CantActivatePhaseMinimum3, [phase: number, required_ma: number, min_ma: number ] ] |
[ AllocatorDecision.Activating1, phase_alloc: number ] |
[ AllocatorDecision.PhaseSwitching0 ] |
[ AllocatorDecision.PhaseSwitchingBlockedUntil2, timestamp_s: number ] |
[ AllocatorDecision.WakingUp0 ];

type global_alloc_desc =
[ GlobalAllocatorDecision.None0 ] |
[ GlobalAllocatorDecision.NextRotationAt2, timestamp_s: number ] |
[ GlobalAllocatorDecision.PVExcessOverloadedHysteresisBlocksUntil3, [overload_ma: number, timestamp_s: number]] |
[ GlobalAllocatorDecision.HysteresisElapsesAt2, timestamp_s: number];

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
    d: alloc_desc
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
    alloc: number[],
    d: global_alloc_desc
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

//APIPath:power_manager/
export interface charge_mode {
    mode: ConfigChargeMode;
}
