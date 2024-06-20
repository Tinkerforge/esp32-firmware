from dataclasses import dataclass, field
from enum import IntEnum
from collections.abc import Callable
from copy import deepcopy
from time import sleep
from math import floor

GLOBAL_HYSTERESIS = 1#5 * 60
ALLOCATION_TIMEOUT_S = 1
SIM_SPEED_UP = 6000
SIM_LENGTH = 40000
PRINT_NTH = 100
ALLOCATED_ENERGY_ROTATION_THRESHOLD = 5 #kWh

force_print = False

#region Stuff

@dataclass
class Car:
    current: int = 32000
    phases: int = 3
    capacity: float = 30 # in kWh; if 0 car is full

class ChargerPhase(IntEnum):
    PV = 0
    P1 = 1
    P2 = 2
    P3 = 3

class GridPhase(IntEnum):
    PV = 0
    L1 = 1
    L2 = 2
    L3 = 3

def phase_rotation(x, y, z):
    return (0 << 6) | (x << 4)| (y << 2)| (z << 0)

class PhaseRotation(IntEnum):
    Unknown = 0
    NotApplicable = 1
    L123 = phase_rotation(GridPhase.L1,GridPhase.L2,GridPhase.L3)
    L132 = phase_rotation(GridPhase.L1,GridPhase.L3,GridPhase.L2)
    L231 = phase_rotation(GridPhase.L2,GridPhase.L3,GridPhase.L1)
    L213 = phase_rotation(GridPhase.L2,GridPhase.L1,GridPhase.L3)
    L321 = phase_rotation(GridPhase.L3,GridPhase.L2,GridPhase.L1)
    L312 = phase_rotation(GridPhase.L3,GridPhase.L1,GridPhase.L2)

def get_phase(rot: PhaseRotation, phase: ChargerPhase) -> GridPhase:
    assert(rot != PhaseRotation.Unknown)

    return GridPhase((rot >> (6 - 2 * phase)) & 0x3)

@dataclass
class Cost:
    pv: int = 0
    l1: int = 0
    l2: int = 0
    l3: int = 0

    def __getitem__(self, i: int | GridPhase) -> int:
        if isinstance(i, ChargerPhase):
            raise Exception("Don't index cost with ChargerPhase! Use get_phase to get GridPhase first!")
        return [self.pv, self.l1, self.l2, self.l3][i]

    def __setitem__(self, i: int | GridPhase, value: int):
        if isinstance(i, slice):
            for j, x in enumerate(range(i.start if i.start is not None else 0,
                                        i.stop if i.stop is not None else 4,
                                        i.step if i.step is not None else 1)):
                self[x] = value[j]
            return

        if isinstance(i, ChargerPhase):
            raise Exception("Don't index cost with ChargerPhase! Use get_phase to get GridPhase first!")
        if i == 0:
            self.pv = value
            return
        elif i == 1:
            self.l1 = value
            return
        elif i == 2:
            self.l2 = value
            return
        elif i == 3:
            self.l3 = value
            return

        raise Exception("Ups")

    def __add__(self, other: 'Cost') -> 'Cost':
        return Cost(self.pv + other.pv, self.l1 + other.l1, self.l2 + other.l2, self.l3 + other.l3)

    def __sub__(self, other: 'Cost') -> 'Cost':
        return Cost(self.pv - other.pv, self.l1 - other.l1, self.l2 - other.l2, self.l3 - other.l3)

def get_cost(current_to_allocate: int, phases_to_allocate: ChargerPhase, rot: PhaseRotation, allocated_current: int, allocated_phases: ChargerPhase) -> Cost:
    cost = Cost(0, 0, 0, 0)

    if allocated_current != 0:
        cost = get_cost(allocated_current, allocated_phases, rot, 0, ChargerPhase(0))
        for i in range(4):
            cost[i] = -cost[i]

    cost.pv += phases_to_allocate * current_to_allocate

    if rot == PhaseRotation.Unknown:
        cost.l1 += current_to_allocate
        cost.l2 += current_to_allocate
        cost.l3 += current_to_allocate
    else:
        for i in range(1, phases_to_allocate + 1):
            cost[get_phase(rot, ChargerPhase(i))] += current_to_allocate

    return cost

@dataclass
class CurrentLimits:
    raw: Cost = field(default_factory=Cost)
    filtered: Cost = field(default_factory=Cost)

def cost_exceeds_limits(cost: Cost, limits: CurrentLimits, stage: int) -> bool:
    phases_exceeded = any([limits.raw[x] < cost[x] for x in range(1, 4)])
    phases_filtered_exceeded = any([limits.filtered[x] < cost[x] for x in range(1, 4)])
    pv_excess_exceeded = limits.raw.pv < cost.pv
    pv_excess_filtered_exceeded = limits.filtered.pv < cost.pv

    if stage in (6, 7):
        return phases_exceeded or pv_excess_exceeded

    raise Exception("unknown stage")

def apply_cost(cost: Cost, limits: CurrentLimits) -> CurrentLimits:
    for i in range(4):
        limits.raw[i] -= cost[i]
        limits.filtered[i] -= cost[i]

    return limits

@dataclass
class ChargerState:
    name: str
    is_charging: bool = False
    wants_to_charge: bool = False
    wants_to_charge_low_priority: bool = False
    phase_switch_supported: bool = False
    phase_rotation: PhaseRotation = PhaseRotation.Unknown
    phases: int = 1
    supported_current: int = 0

    _current_allocation: int = 0
    _phase_allocation: int = 0
    _car: Car = field(default_factory=Car)
    _allocated_energy: float = 0
    _allocated_energy_this_rotation: float = 0

@dataclass
class CurrentAllocatorConfig:
    minimum_current_3p: int
    minimum_current_1p: int
    enable_current_factor: float = 1.5

@dataclass
class CurrentAllocatorState:
    global_hysteresis_elapsed: bool
    reset_global_hysteresis: bool

    _last_global_hysteresis_reset: int = -GLOBAL_HYSTERESIS - 1
    _control_window_min: Cost = field(default_factory=Cost)
    _control_window_max: Cost = field(default_factory=Cost)
    _t: int = 0

def flatten(x):
    return sum(x, [])

def sort(cs: list[ChargerState], group_fn: Callable[[ChargerState], int], key_fn: Callable[[ChargerState], int]) -> list[ChargerState]:
    x = {}
    for c in cs:
        g = group_fn(c)
        x.setdefault(g, []).append(c)
    for k in x:
        x[k] = sorted(x[k], key=key_fn)

    return flatten([x[k] for k in sorted(x.keys())])

#endregion

#region Stages

def is_active(cs: ChargerState):
    return cs._phase_allocation > 0 or cs.is_charging
    # TODO: implement other checks such as "Einschaltzeit > 0 < 1 min und State == B1 oder B2" here
    # Maybe also handle global hysteresis here? (ignore ALLOCATED_ENERGY_ROTATION_THRESHOLD if it is not elapsed)


def stage_1(limits: CurrentLimits, charger_state: list[ChargerState], cfg: CurrentAllocatorConfig, ca_state: CurrentAllocatorState):
    have_b1 = any(cs.wants_to_charge for cs in charger_state)

    for cs in charger_state:
        if is_active(cs) and (not have_b1 or cs._allocated_energy_this_rotation < ALLOCATED_ENERGY_ROTATION_THRESHOLD):
            cs._phase_allocation = cs.phases


def stage_2(limits: CurrentLimits, charger_state: list[ChargerState], cfg: CurrentAllocatorConfig, ca_state: CurrentAllocatorState):
    min_1p = cfg.minimum_current_1p
    min_3p = cfg.minimum_current_3p

    wnd_min = Cost()
    wnd_min_1p = Cost() # .pv is not used
    wnd_max = Cost()

    cs = [x for x in charger_state if x._phase_allocation > 0]

    for state in cs:
        if state.phase_rotation == PhaseRotation.Unknown:
            if state._phase_allocation == 1:
                wnd_min.pv += min_1p
                wnd_min.l1 += min_1p
                wnd_min.l2 += min_1p
                wnd_min.l3 += min_1p

                wnd_min_1p.l1 += min_1p
                wnd_min_1p.l2 += min_1p
                wnd_min_1p.l3 += min_1p
            else:
                wnd_min.pv += 3 * min_3p
                wnd_min.l1 += min_3p
                wnd_min.l2 += min_3p
                wnd_min.l3 += min_3p
        else:
            for i in range(1, 1 + state._phase_allocation):
                phase = get_phase(state.phase_rotation, ChargerPhase(i))
                wnd_min.pv += min_1p if state._phase_allocation == 1 else min_3p
                wnd_min[phase] += min_1p if state._phase_allocation == 1 else min_3p
                wnd_min_1p[phase] += min_1p if state._phase_allocation == 1 else 0

    current_avail_for_3p = 1e7
    for i in range(1, 4):
        avail_on_phase = min(limits.raw[i], limits.filtered[i]) - wnd_min_1p[i]
        current_avail_for_3p = min(current_avail_for_3p, avail_on_phase)

    for state in cs:
        if state._phase_allocation != 3:
            continue

        wnd_max += Cost(0,
                        state.supported_current,
                        state.supported_current,
                        state.supported_current)
        # It is sufficient to check one phase here, wnd_max should have the same value on every phase because only three phase chargers are included yet
        if wnd_max.l1 > current_avail_for_3p:
            wnd_max = Cost(wnd_max.l1 + (current_avail_for_3p - wnd_max.l1) * state._phase_allocation,
                           current_avail_for_3p,
                           current_avail_for_3p,
                           current_avail_for_3p)
            break

        wnd_max.pv += state.supported_current * state._phase_allocation

    for state in cs:
        if state._phase_allocation == 3 or state.phase_rotation != PhaseRotation.Unknown:
            continue

        # only 1p unknown rotated chargers left

        current = state.supported_current
        for i in range(1, 4):
            avail_on_phase = min(limits.raw[i], limits.filtered[i]) - wnd_max[i]
            current = min(current, avail_on_phase)

        wnd_max += Cost(current,
                        current,
                        current,
                        current)

    for state in cs:
        if state._phase_allocation == 3 or state.phase_rotation == PhaseRotation.Unknown:
            continue

        phase = get_phase(state.phase_rotation, ChargerPhase.P1)

        l = min(limits.raw[phase], limits.filtered[phase])
        current = min(l - wnd_max[phase], state.supported_current)

        wnd_max[phase] += current
        wnd_max.pv += current

    ca_state._control_window_min = wnd_min
    ca_state._control_window_max = wnd_max

def stage_3(limits: CurrentLimits, charger_state: list[ChargerState], cfg: CurrentAllocatorConfig, ca_state: CurrentAllocatorState):
    wnd_min = ca_state._control_window_min

    cs = [x for x in charger_state if x._phase_allocation > 0]

    cs = reversed(sort(cs,
              lambda x: 0,
              lambda x: x._allocated_energy))

    min_1p = cfg.minimum_current_1p
    min_3p = cfg.minimum_current_3p

    any_charger_shut_down = False

    # Maybe try to be more clever here:
    # - Shut down 1p chargers first if only one or two phase limits are below wnd_min
    # - Shut down 1p unknown rotated chargers last
    for p in range(1, 4):
        for state in cs:
            if limits.raw[p] >= wnd_min[p]:
                break

            if state._phase_allocation == 0:
                continue

            any_charger_shut_down = True

            if state.phases == 3:
                state._phase_allocation = 0
                wnd_min -= Cost(3*min_3p, min_3p, min_3p, min_3p)
            elif state.phase_rotation == PhaseRotation.Unknown:
                state._phase_allocation = 0
                wnd_min -= Cost(min_1p, min_1p, min_1p, min_1p)
            elif get_phase(state.phase_rotation, ChargerPhase.P1) == GridPhase(p):
                state._phase_allocation = 0

                wnd_min.pv -= min_1p
                wnd_min[p] -= min_1p


    # TODO: charger min active time
    for state in cs:
        if limits.raw.pv >= wnd_min.pv or limits.filtered.pv >= wnd_min.pv or not ca_state.global_hysteresis_elapsed:
            break

        if state._phase_allocation == 0:
            continue

        any_charger_shut_down = True

        if state.phases == 3:
            state._phase_allocation = 0
            wnd_min -= Cost(3*min_3p, min_3p, min_3p, min_3p)
        elif state.phase_rotation == PhaseRotation.Unknown:
            state._phase_allocation = 0
            wnd_min -= Cost(min_1p, min_1p, min_1p, min_1p)
        else:
            state._phase_allocation = 0
            wnd_min.pv -= min_1p

            phase = get_phase(state.phase_rotation, ChargerPhase.P1)
            wnd_min[phase] -= min_1p

    # Recalculating the window once is enough: We don't need an up to date max window in this stage. We could also skip recalculating wnd_min
    if any_charger_shut_down:
        stage_2(limits, charger_state, cfg, ca_state)

def stage_4(limits: CurrentLimits, charger_state: list[ChargerState], cfg: CurrentAllocatorConfig, ca_state: CurrentAllocatorState):
    wnd_min = ca_state._control_window_min
    wnd_max = ca_state._control_window_max

    cs = [x for x in charger_state if x._phase_allocation == 0 and (x.wants_to_charge or is_active(x))]

    cs = sort(cs,
              lambda x: 0,
              lambda x: x._allocated_energy)

    min_1p = cfg.minimum_current_1p
    min_3p = cfg.minimum_current_3p
    ena_1p = min_1p * cfg.enable_current_factor
    ena_3p = min_3p * cfg.enable_current_factor

    def can_activate(new_cost, new_enable_cost, is_unknown_rotated_1p_3p_switch=False):
        for i in range(1, 4):
            if new_cost[i] > 0 and wnd_max[i] < limits.raw[i] and wnd_max[i] < limits.filtered[i]:
                break
        else:
            # if new_cost.pv <= 0 or all(wnd_max[i] == limits.raw[i] and wnd_max[i] == limits.filtered[i] for i in range(1, 4) if new_cost[i] > 0):
            enable = is_unknown_rotated_1p_3p_switch and new_cost.pv >= 0 and wnd_max.pv < limits.raw.pv and wnd_max.pv < limits.filtered.pv
            if not enable:
                return False

        limit_exceeded = False
        for i in range(4):
            if new_cost[i] <= 0:
                continue
            required = wnd_min[i] * cfg.enable_current_factor + new_enable_cost[i]

            limit_exceeded |= limits.raw[i] < required
            limit_exceeded |= limits.filtered[i] < required
        if limit_exceeded:
            return False

        return True

    for state in cs:
        new_cost = Cost()
        new_enable_cost = Cost()

        activate_3p = state.phases == 3 and not state.phase_switch_supported

        if activate_3p:
            new_cost = Cost(3*min_3p, min_3p, min_3p, min_3p)
            new_enable_cost = Cost(3*ena_3p, ena_3p, ena_3p, ena_3p)
        elif state.phase_rotation == PhaseRotation.Unknown:
            # Try to enable switchable unknown rotated chargers with three phases first.
            if state.phase_switch_supported:
                new_cost = Cost(3*min_3p, min_3p, min_3p, min_3p)
                new_enable_cost = Cost(3*ena_3p, ena_3p, ena_3p, ena_3p)
                activate_3p = True
            if not state.phase_switch_supported or not can_activate(new_cost, new_enable_cost):
                new_cost = Cost(min_1p, min_1p, min_1p, min_1p)
                new_enable_cost = Cost(ena_1p, ena_1p, ena_1p, ena_1p)
                activate_3p = False
        else:
            phase = get_phase(state.phase_rotation, ChargerPhase.P1)

            new_cost.pv += min_1p
            new_cost[phase] += min_1p

            new_enable_cost.pv += ena_1p
            new_enable_cost[phase] += ena_1p

        if not can_activate(new_cost, new_enable_cost):
            continue

        state._phase_allocation = 3 if activate_3p else 1
        stage_2(limits, charger_state, cfg, ca_state)
        wnd_min = ca_state._control_window_min
        wnd_max = ca_state._control_window_max

    # FIXME: This will _only_ switch chargers to three-phase that were not active before!
    for state in cs:
        if state._phase_allocation != 1 or not state.phase_switch_supported:
            continue

        new_cost = Cost(3*min_3p-min_1p, min_3p, min_3p, min_3p)
        new_enable_cost = Cost(3*ena_3p-ena_1p, ena_3p, ena_3p, ena_3p)

        if state.phase_rotation == PhaseRotation.Unknown:
            for i in range(1, 4):
                new_cost[i] -= min_1p
                new_enable_cost[i] -= ena_1p
        else:
            phase = get_phase(state.phase_rotation, ChargerPhase.P1)
            # P1 is already active
            new_cost[phase] -= min_1p
            new_enable_cost[phase] -= ena_1p

        if not can_activate(new_cost, new_enable_cost, is_unknown_rotated_1p_3p_switch=True):
            continue

        state._phase_allocation = 3
        stage_2(limits, charger_state, cfg, ca_state)
        wnd_min = ca_state._control_window_min
        wnd_max = ca_state._control_window_max


def stage_5(limits: CurrentLimits, charger_state: list[ChargerState], cfg: CurrentAllocatorConfig, ca_state: CurrentAllocatorState):
    cs = [x for x in charger_state if x._phase_allocation > 0]

    cs = sort(cs,
              lambda x: 0,
              lambda x: x._allocated_energy)

    min_1p = cfg.minimum_current_1p
    min_3p = cfg.minimum_current_3p

    for state in cs:
        if state._phase_allocation == 3:
            cost = Cost(3*min_3p, min_3p, min_3p, min_3p)
        else:
            cost = Cost(min_1p, 0, 0, 0)
            if state.phase_rotation == PhaseRotation.Unknown:
                for i in range(1, 4):
                    cost[i] += min_1p
            else:
                phase = get_phase(state.phase_rotation, ChargerPhase.P1)
                cost[phase] += min_1p

        state._current_allocation = min_3p if state._phase_allocation == 3 else min_1p
        apply_cost(cost, limits)

def current_capacity(limits: CurrentLimits, state: ChargerState):
    capacity = state.supported_current - state._current_allocation

    if state._phase_allocation == 3 or state.phase_rotation == PhaseRotation.Unknown:
        return min(capacity, limits.raw.l1, limits.raw.l2, limits.raw.l3)

    for i in range(1, 1 + state._phase_allocation):
        phase = get_phase(state.phase_rotation, ChargerPhase(i))
        capacity = min(capacity, limits.raw[phase])

    return state._phase_allocation * capacity

def stage_6(limits: CurrentLimits, charger_state: list[ChargerState], cfg: CurrentAllocatorConfig, ca_state: CurrentAllocatorState):
    cs = [x for x in charger_state if x._current_allocation > 0 ]
    if len(cs) == 0:
        return

    cs = sort(cs,
              lambda x: 3 - x._phase_allocation,
              lambda x: 0)

    active_on_phase = Cost(pv=len(cs))
    for state in cs:
        if state._phase_allocation == 3 or state.phase_rotation == PhaseRotation.Unknown:
            active_on_phase.l1 += 1
            active_on_phase.l2 += 1
            active_on_phase.l3 += 1
        else:
            for i in range(1, 1 + state._phase_allocation):
                phase = get_phase(state.phase_rotation, ChargerPhase(i))
                active_on_phase[phase] += 1

    fair_current = floor(min([(limits.raw[i] / active_on_phase[i]) if active_on_phase[i] != 0 else 1e7 for i in range(0, 4)]))

    for state in cs:
        current = min(fair_current, current_capacity(limits, state))
        current += state._current_allocation

        cost = get_cost(current, ChargerPhase(state._phase_allocation), state.phase_rotation, state._current_allocation, ChargerPhase(state._phase_allocation))

        if cost_exceeds_limits(cost, limits, 6):
            continue

        apply_cost(cost, limits)
        state._current_allocation = current


def stage_7(limits: CurrentLimits, charger_state: list[ChargerState], cfg: CurrentAllocatorConfig, ca_state: CurrentAllocatorState):
    cs = [x for x in charger_state if x._current_allocation > 0 ]

    cs = sort(cs,
              lambda x: 3 - x._phase_allocation,
              lambda x: current_capacity(limits, x))


    for state in cs:
        current = min(limits.raw.pv, current_capacity(limits, state))
        current += state._current_allocation

        cost = get_cost(current, ChargerPhase(state._phase_allocation), state.phase_rotation, state._current_allocation, ChargerPhase(state._phase_allocation))

        if cost_exceeds_limits(cost, limits, 7):
            continue

        apply_cost(cost, limits)
        state._current_allocation = current


def print_allocation(t: int, charger_state: list[ChargerState], stage:int = None):
    if t % PRINT_NTH and not force_print != 0:
        return

    if stage is not None:
        print(f"Stage {stage}", end="  ")
    for state in charger_state:
        if state._phase_allocation == 0:
            print(" " * 11, end = " | ")
        else:
            print(f"{state._current_allocation/1000:>6.3f} @ {state._phase_allocation}p", end=" | ")
    print("")

def allocate_current(t: int, limits: CurrentLimits, limits_cpy: CurrentLimits, charger_state: list[ChargerState], cfg: CurrentAllocatorConfig, ca_state: CurrentAllocatorState):
    global force_print

    #if t == 816:
    #    force_print = True
    #    breakpoint()

    setup(limits, limits_cpy, charger_state, ca_state)
    ca_state._t = t

    ca_state.global_hysteresis_elapsed = t >= (ca_state._last_global_hysteresis_reset + GLOBAL_HYSTERESIS)
    ca_state.reset_global_hysteresis = False

    stage_1(limits_cpy, charger_state, cfg, ca_state)
    print_allocation(t, charger_state, 1)
    stage_2(limits_cpy, charger_state, cfg, ca_state)
    print_allocation(t, charger_state, 2)
    stage_3(limits_cpy, charger_state, cfg, ca_state)
    print_allocation(t, charger_state, 3)
    stage_4(limits_cpy, charger_state, cfg, ca_state)
    print_allocation(t, charger_state, 4)
    stage_5(limits_cpy, charger_state, cfg, ca_state)
    print_allocation(ca_state._t, charger_state, 5)
    stage_6(limits_cpy, charger_state, cfg, ca_state)
    print_allocation(ca_state._t, charger_state, 6)
    stage_7(limits_cpy, charger_state, cfg, ca_state)

    print_allocation(t, charger_state, 7)

    if ca_state.reset_global_hysteresis:
        ca_state._last_global_hysteresis_reset = t

    for state in charger_state:
        amps = state._current_allocation / 1000 * state._phase_allocation # mA / 1000 * phases = A
        charge = amps * ALLOCATION_TIMEOUT_S # A * s = As
        charge /= 3600 # Ah
        energy = 230 * charge # V * Ah = Wh
        energy /= 1000 # kWh

        state._allocated_energy += energy
        if state._current_allocation != 0:
            state._allocated_energy_this_rotation += energy
        else:
            state._allocated_energy_this_rotation = 0


    if t % PRINT_NTH and not force_print != 0:
        return
    print(f"I {t:5d}", end="  ")
    for x in charger_state:
        if x._car is not None:
            print(f"{x._allocated_energy:>4.1f}  {x._car.current // 1000}@{x._car.phases}p", end=" | ")
        else:
            print("  no  car  ", end = " | ")

    print("")
    print("")

    #if force_print:
    #    breakpoint()

#region Test



def setup(limits: CurrentLimits, limits_cpy: CurrentLimits, charger_state: ChargerState, ca_state: CurrentAllocatorState):
    limits_cpy.raw = deepcopy(limits.raw)
    limits_cpy.filtered = deepcopy(limits.filtered)

    for x in charger_state:
        x._current_allocation = 0
        x._phase_allocation = 0

def update_chargers(charger_state: list[ChargerState]):
    for c in charger_state:
        if c._current_allocation > 0:
            c.is_charging = True
            c.wants_to_charge = False
            c.wants_to_charge_low_priority = False
            if c.phase_switch_supported:
                c.phases = c._phase_allocation

        if c.is_charging and c._car is not None:
            amps = min(c._car.current, c._current_allocation) / 1000
            phases = min(c._car.phases, c.phases)

            c._car.capacity -= amps * phases * 230 / 3600 / 1000
            if c._car.capacity <= 0:
                c._car = None
                c.is_charging = False
                c._allocated_energy = 0

        if c._current_allocation == 0 and c.is_charging:
            c.is_charging = False
            c.wants_to_charge = c._car.capacity > 0

# ca_state = CurrentAllocatorState(False, False, Cost(0, 0, 0, 0))
# charger_state = [
#     ChargerState(name="Charger 0", phases=1, phase_switch_supported=False, phase_rotation=PhaseRotation.Unknown, is_charging=True,                  supported_current=10000),
#     ChargerState(name="Charger 1", phases=1, phase_switch_supported=False, phase_rotation=PhaseRotation.L123,    _car=None),
#     ChargerState(name="Charger 2", phases=1, phase_switch_supported=True,  phase_rotation=PhaseRotation.Unknown, wants_to_charge=True,              supported_current=12000),
#     ChargerState(name="Charger 3", phases=1, phase_switch_supported=True,  phase_rotation=PhaseRotation.L231,    is_charging=True,                  supported_current=13000),
#     ChargerState(name="Charger 4", phases=3, phase_switch_supported=False, phase_rotation=PhaseRotation.Unknown, wants_to_charge=True,              supported_current=14000),
#     ChargerState(name="Charger 5", phases=3, phase_switch_supported=False, phase_rotation=PhaseRotation.L132,    wants_to_charge_low_priority=True, supported_current=15000),
#     ChargerState(name="Charger 6", phases=3, phase_switch_supported=True,  phase_rotation=PhaseRotation.Unknown, is_charging=True,                  supported_current=16000),
#     ChargerState(name="Charger 7", phases=3, phase_switch_supported=True,  phase_rotation=PhaseRotation.L312,    _car=None),
# ]

# cfg = CurrentAllocatorConfig(6000, 6000, 9000)
# limits = CurrentLimits(raw=Cost(30000, 14000, 16000, 8000),
#                        filtered=Cost(30000, 14000, 16000, 8000),
#                        supply=Cost(0, 32000, 32000, 32000))
# limits_cpy = CurrentLimits()

# limits = CurrentLimits(raw=Cost(60000, 44000, 46000, 38000),
#                        filtered=Cost(60000, 44000, 46000, 38000),
#                        supply=Cost(0, 42000, 42000, 42000))

ca_state = CurrentAllocatorState(False, False, Cost(0, 0, 0, 0))
charger_state = [
    ChargerState(name="Charger 0", phases=3, phase_switch_supported=True, phase_rotation=PhaseRotation.L123, wants_to_charge=True, supported_current=32000),
    ChargerState(name="Charger 1", phases=3, phase_switch_supported=True, phase_rotation=PhaseRotation.L231, _car=None),#wants_to_charge=True, supported_current=32000),
    ChargerState(name="Charger 2", phases=3, phase_switch_supported=True, phase_rotation=PhaseRotation.L312, _car=None),#wants_to_charge=True, supported_current=32000),
    ChargerState(name="Charger 3", phases=3, phase_switch_supported=True, phase_rotation=PhaseRotation.L123, _car=None),
    ChargerState(name="Charger 4", phases=3, phase_switch_supported=True, phase_rotation=PhaseRotation.L231, _car=None),
    ChargerState(name="Charger 5", phases=3, phase_switch_supported=True, phase_rotation=PhaseRotation.L312, _car=None)
]

# charger_state = [
#     ChargerState(name="Charger 1", phases=3, phase_switch_supported=True,  phase_rotation=PhaseRotation.L123, wants_to_charge=True, supported_current=32000),
#     ChargerState(name="Charger 2", phases=3, phase_switch_supported=True,  phase_rotation=PhaseRotation.L231, _car=None),
# ]


cfg = CurrentAllocatorConfig(6000, 6000, 9000)
limits = CurrentLimits(raw=Cost(1e7, 32000, 32000, 32000),
                       filtered=Cost(1e7, 32000, 32000, 32000))
limits_cpy = CurrentLimits()

car_queue = [
    Car(16000, 3, 30),
    Car(32000, 1, 30),
    Car(),
    Car(32000, 1, 30),
    Car(),
]

for i in range(SIM_LENGTH):
    if i % ALLOCATION_TIMEOUT_S == 0:
        allocate_current(i, limits, limits_cpy, charger_state, cfg, ca_state)

    # setup(limits, limits_cpy, charger_state, ca_state)
    # stage_1(limits_cpy, charger_state, cfg, ca_state)

    update_chargers(charger_state)

    # print([x._current_allocation for x in charger_state])
    # print([x._phase_allocation for x in charger_state])
    # if i % 100 == 0:
    #     print([x._car for x in charger_state])
    #print(ca_state.allocated_minimum_current_packets)
    #print(limits_cpy)

    if i < len(charger_state) - 1:
        charger_state[i + 1].wants_to_charge = True
        charger_state[i + 1]._car = car_queue[i]
        charger_state[i + 1].supported_current = car_queue[i].current
        if car_queue[i].phases == 1:
            charger_state[i + 1].phases = 1
            charger_state[i + 1].phase_switch_supported = False

    # if len(car_queue) > 0 and i > 700 and i % 100 == 0:
    #     cs = next(x for x in charger_state if x._car is None)
    #     cs.wants_to_charge = True
    #     cs._car = car_queue[0]
    #     cs.supported_current = car_queue[0].current
    #     if car_queue[0].phases == 1:
    #         cs.phases = 1
    #         cs.phase_switch_supported = False
    #     car_queue.pop(0)

    #sleep(1 / SIM_SPEED_UP)

# limits.raw.l2 = 13000
# limits.raw.l3 = 18000

# setup(limits, limits_cpy, charger_state, ca_state)
# stage_1(limits_cpy, charger_state, cfg, ca_state)

# print([x._current_allocation for x in charger_state])
# print([x._phase_allocation for x in charger_state])
# print(ca_state.allocated_minimum_current_packets)
# print(limits_cpy)

# stage_2(limits_cpy, charger_state, cfg, ca_state)

# print([x._current_allocation for x in charger_state])
# print([x._phase_allocation for x in charger_state])
# print(ca_state.allocated_minimum_current_packets)
# print(limits_cpy)

#endregion

