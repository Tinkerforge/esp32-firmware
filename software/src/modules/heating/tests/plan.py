#!/usr/bin/env -S uv run --group tests --script

# Tests for heating plan computation: ILP reference solver vs firmware DP.
#
# Each test generates a price pattern, runs an ILP reference solver
# (scipy.optimize.milp) per control-period block, validates constraints,
# and optionally compares against the firmware's DP-based plan on a device.

import math
import random
import time
from dataclasses import dataclass
from typing import Optional

from scipy.optimize import milp, LinearConstraint, Bounds
import numpy as np

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite
else:
    import tinkerforge_util as tfutil

    tfutil.create_parent_module(__file__, "software")
    from software.test_runner.test_context import run_testsuite, TestContext


# ILP reference solver
def _ilp_select_slots(
    slot_prices: list, N: int, K: int, B: int, find_expensive: bool = False
) -> Optional[list]:
    """Solve the slot selection problem as an Integer Linear Program.

    Uses scipy.optimize.milp. Returns None if the problem is infeasible.

    Selects exactly K slots out of N to minimize total cost (or maximize
    if find_expensive=True), subject to the constraint that all contiguous
    runs of selected AND unselected slots have length >= B.
    """
    if K == 0:
        return [False] * N
    if K == N:
        return [True] * N
    if K < B or (N - K) < B:
        return None

    # Objective: minimize sum(c[i] * x[i])
    c = np.array(slot_prices, dtype=float)
    if find_expensive:
        c = -c

    constraints_rows = []
    constraints_b = []

    # Minimum block length constraints using transition-based formulation.
    #
    # Interior constraints (for each d in 1..B-1):
    #   Position 0 as potential run start:
    #     Active start:   x[0] - x[d] <= 0   (if x[0]=1 then x[d]=1)
    #     Inactive start: x[d] - x[0] <= 0   (if x[0]=0 then x[d]=0)
    #   Position i >= 1:
    #     Active run start:   x[i] - x[i-1] - x[i+d] <= 0
    #     Inactive run start: x[i-1] - x[i] + x[i+d] <= 1
    #
    # Boundary constraints: no transition allowed at positions where fewer
    # than B slots remain, i.e. for positions i where N - i < B:
    #   x[i] = x[i-1]  (equivalently: x[i] - x[i-1] <= 0 AND x[i-1] - x[i] <= 0)

    for d in range(1, B):
        # i = 0: active start
        if d < N:
            row = [0.0] * N
            row[0] = 1.0
            row[d] = -1.0
            constraints_rows.append(row)
            constraints_b.append(0.0)

        # i = 0: inactive start
        if d < N:
            row = [0.0] * N
            row[d] = 1.0
            row[0] = -1.0
            constraints_rows.append(row)
            constraints_b.append(0.0)

        # i >= 1
        for i in range(1, N):
            if i + d >= N:
                continue
            # Active run start: x[i] - x[i-1] - x[i+d] <= 0
            row = [0.0] * N
            row[i] = 1.0
            row[i - 1] = -1.0
            row[i + d] = -1.0
            constraints_rows.append(row)
            constraints_b.append(0.0)

            # Inactive run start: x[i-1] - x[i] + x[i+d] <= 1
            row = [0.0] * N
            row[i - 1] = 1.0
            row[i] = -1.0
            row[i + d] = 1.0
            constraints_rows.append(row)
            constraints_b.append(1.0)

    # Boundary: no transitions allowed at positions where fewer than B
    # slots remain. Force x[i] == x[i-1] for i in [N-B+1 .. N-1].
    for i in range(max(1, N - B + 1), N):
        # x[i] - x[i-1] <= 0
        row = [0.0] * N
        row[i] = 1.0
        row[i - 1] = -1.0
        constraints_rows.append(row)
        constraints_b.append(0.0)

        # x[i-1] - x[i] <= 0
        row = [0.0] * N
        row[i - 1] = 1.0
        row[i] = -1.0
        constraints_rows.append(row)
        constraints_b.append(0.0)

    if len(constraints_rows) == 0:
        A_ub = np.zeros((0, N))
        b_ub = np.zeros(0)
    else:
        A_ub = np.array(constraints_rows, dtype=float)
        b_ub = np.array(constraints_b, dtype=float)

    # Equality constraint: sum(x) = K
    A_eq = np.ones((1, N), dtype=float)
    b_eq = np.array([K], dtype=float)

    bounds = Bounds(lb=0, ub=1)
    integrality = np.ones(N)  # all variables are integers

    all_constraints: list = []
    if A_ub.shape[0] > 0:
        all_constraints.append(LinearConstraint(A_ub, ub=b_ub))  # type: ignore[arg-type]
    all_constraints.append(LinearConstraint(A_eq, lb=b_eq, ub=b_eq))  # type: ignore[arg-type]

    result = milp(
        c,
        integrality=integrality,
        bounds=bounds,
        constraints=all_constraints,
        options={"time_limit": 30},
    )

    if not result.success:
        return None

    x = [bool(round(v)) for v in result.x]

    # Verify sum
    if sum(x) != K:
        return None

    return x


# Validation helpers
def _get_runs(selection: list) -> list:
    """Get contiguous runs as (value, length) pairs."""
    if not selection:
        return []
    runs = []
    current = selection[0]
    length = 1
    for i in range(1, len(selection)):
        if selection[i] == current:
            length += 1
        else:
            runs.append((current, length))
            current = selection[i]
            length = 1
    runs.append((current, length))
    return runs


def _validate_solution(
    selection: list, N: int, K: int, B: int, label: str = ""
) -> list:
    """Validate that a solution satisfies all constraints. Returns list of errors."""
    errors = []
    prefix = f"[{label}] " if label else ""

    if len(selection) != N:
        errors.append(f"{prefix}Length mismatch: got {len(selection)}, expected {N}")
        return errors

    actual_k = sum(selection)
    if actual_k != K:
        errors.append(f"{prefix}K mismatch: got {actual_k}, expected {K}")

    runs = _get_runs(selection)
    for value, length in runs:
        if length < B:
            kind = "active" if value else "inactive"
            errors.append(
                f"{prefix}Run-length violation: {kind} run of length {length} < B={B}"
            )

    return errors


def _compute_cost(selection: list, prices: list) -> int:
    return sum(p for s, p in zip(selection, prices) if s)


# Price pattern generators
def _make_flat(count: int, price: int = 5000) -> list:
    return [price] * count


def _make_ascending(count: int, base: int = 1000, step: int = 100) -> list:
    return [base + i * step for i in range(count)]


def _make_descending(count: int, base: int = 10000, step: int = 100) -> list:
    return [base - i * step for i in range(count)]


def _make_sawtooth(
    count: int, period: int = 8, low: int = 1000, high: int = 9000
) -> list:
    prices = []
    for i in range(count):
        phase = i % period
        price = low + (high - low) * phase // (period - 1) if period > 1 else low
        prices.append(price)
    return prices


def _make_sine(count: int, base: int = 5000, amplitude: int = 3000) -> list:
    return [
        int(base + amplitude * math.sin(2 * math.pi * i / count)) for i in range(count)
    ]


def _make_random(count: int, low: int = 500, high: int = 15000, seed: int = 42) -> list:
    rng = random.Random(seed)
    return [rng.randint(low, high) for _ in range(count)]


def _make_two_plateaus(count: int, low: int = 2000, high: int = 8000) -> list:
    mid = count // 2
    return [low] * mid + [high] * (count - mid)


def _make_realistic_de(count: int) -> list:
    hourly_pattern = [
        3200,
        2800,
        2500,
        2300,
        2400,
        3000,  # 00-05: overnight low
        4500,
        6200,
        7800,
        7200,
        6500,
        5800,  # 06-11: morning ramp + peak
        4200,
        3500,
        3200,
        3800,
        5200,
        7500,  # 12-17: solar dip + evening ramp
        8500,
        7800,
        6200,
        5000,
        4200,
        3600,  # 18-23: evening peak + decline
    ]
    prices = []
    slots_per_hour = max(1, count // 24)
    for h, p in enumerate(hourly_pattern):
        for _ in range(slots_per_hour):
            noise = int(200 * math.sin(len(prices) * 0.7))
            prices.append(p + noise)
            if len(prices) >= count:
                break
        if len(prices) >= count:
            break
    while len(prices) < count:
        prices.append(hourly_pattern[-1])
    return prices[:count]


def _make_negative_prices(count: int) -> list:
    base_pattern = _make_sine(count, base=2000, amplitude=4000)
    return [max(p, -2000) for p in base_pattern]


def _make_spiky(
    count: int,
    base: int = 3000,
    spike: int = 25000,
    spike_positions: Optional[list] = None,
) -> list:
    prices = [base] * count
    if spike_positions is None:
        spike_positions = [count // 4, count // 2, 3 * count // 4]
    for pos in spike_positions:
        if 0 <= pos < count:
            prices[pos] = spike
    return prices


_PATTERN_GENERATORS = {
    "flat": _make_flat,
    "ascending": _make_ascending,
    "descending": _make_descending,
    "sawtooth": _make_sawtooth,
    "sine": _make_sine,
    "random": _make_random,
    "two_plateaus": _make_two_plateaus,
    "realistic_de": _make_realistic_de,
    "negative": _make_negative_prices,
    "spiky": _make_spiky,
}


def _generate_prices(pattern_name: str, count: int, seed: int = 42) -> list:
    gen = _PATTERN_GENERATORS[pattern_name]
    if pattern_name == "random":
        return gen(count, seed=seed)
    return gen(count)


# Test case definition
@dataclass
class _TestCase:
    extended_hours: int
    blocking_hours: int
    control_period: int  # enum: 0=24h, 1=12h, 2=8h, 3=6h, 4=4h
    min_hold_time: int  # minutes (10-60)
    price_pattern: str  # key into _PATTERN_GENERATORS
    price_resolution: int = 15  # 15 or 60 minutes
    price_hours: int = 48  # hours of price data
    extended: bool = True
    blocking: bool = True
    pattern_seed: int = 42

    @property
    def duration_hours(self) -> int:
        return {0: 24, 1: 12, 2: 8, 3: 6, 4: 4}[self.control_period]

    @property
    def min_block_15m(self) -> int:
        return (self.min_hold_time + 14) // 15


# Block computation
def _get_control_period_blocks(
    duration_hours: int,
    first_date: int,
    resolution_minutes: int,
    num_prices: int,
    tz_offset_minutes: int = 0,
) -> list:
    """Compute control period block boundaries, matching firmware logic."""
    slots_per_hour = 60 // resolution_minutes
    slots_per_period = duration_hours * slots_per_hour
    duration_minutes = duration_hours * 60

    local_first_date = first_date + tz_offset_minutes
    minutes_since_midnight = local_first_date % 1440

    block_offset = (minutes_since_midnight // duration_minutes) * duration_minutes
    first_block_start_minutes = first_date - (minutes_since_midnight - block_offset)

    price_end_minutes = first_date + num_prices * resolution_minutes

    blocks = []
    block_start = first_block_start_minutes
    while block_start < price_end_minutes:
        block_start_index = (block_start - first_date) // resolution_minutes
        block_end_index = block_start_index + slots_per_period
        if block_start_index >= 0 and block_end_index <= num_prices:
            blocks.append((block_start_index, slots_per_period))
        block_start += duration_minutes

    return blocks


# Test data generation
def _generate_test_data(case: _TestCase, tz_offset_minutes: int = 0):
    """Generate prices and first_date for a test case.

    Returns (prices, first_date) where first_date is aligned to local midnight
    in UTC minutes.
    """
    slots_per_hour = 60 // case.price_resolution
    count = case.price_hours * slots_per_hour
    prices = _generate_prices(case.price_pattern, count, seed=case.pattern_seed)

    now = time.time()
    local_midnight_utc = (
        int(now) + tz_offset_minutes * 60
    ) // 86400 * 86400 - tz_offset_minutes * 60
    first_date = local_midnight_utc // 60  # in UTC minutes

    return prices, first_date


# Module state + suite setup/teardown
_tz_offset_minutes = 0
_original_heating_config = None
_original_dap_config = None


def suite_setup(tc: TestContext):
    global _tz_offset_minutes, _original_heating_config, _original_dap_config

    # Save original configs for teardown restoration.
    _original_heating_config = tc.api("heating/config")
    _original_dap_config = tc.api("day_ahead_prices/config")

    # Detect timezone offset from the device's NTP config.
    ntp_config = tc.api("ntp/config")
    tz_name = ntp_config.get("timezone", "UTC")
    try:
        from zoneinfo import ZoneInfo
        from datetime import datetime

        tz = ZoneInfo(tz_name)
        now_dt = datetime.now(tz)
        utc_offset = now_dt.utcoffset()
        if utc_offset is not None:
            _tz_offset_minutes = int(utc_offset.total_seconds()) // 60
    except ImportError:
        # Fallback: assume CET for Europe/Berlin-like timezones.
        if "Berlin" in tz_name or "Europe" in tz_name:
            _tz_offset_minutes = 60


def _run_and_validate_reference(
    tc: TestContext, case: _TestCase, prices: list, first_date: int
):
    """Run ILP reference solver per block and validate solutions.

    Returns (ref_cheap_plan, ref_expensive_plan) as bool arrays aligned to prices.
    """
    N_prices = len(prices)
    is_15min = case.price_resolution == 15
    B = case.min_block_15m
    ilp_B = B if is_15min else (math.ceil(B / 4) if B > 0 else 0)

    blocks = _get_control_period_blocks(
        case.duration_hours,
        first_date,
        case.price_resolution,
        N_prices,
        _tz_offset_minutes,
    )

    cheap_plan = [False] * N_prices
    expensive_plan = [False] * N_prices

    for block_start_idx, slots_per_period in blocks:
        block_prices = prices[block_start_idx : block_start_idx + slots_per_period]

        if is_15min:
            N = len(block_prices)
            K_cheap = case.extended_hours * 4
            K_expensive = case.blocking_hours * 4
        else:
            N = len(block_prices)
            K_cheap = case.extended_hours
            K_expensive = case.blocking_hours

        # Solve and validate cheap (extended)
        if case.extended and K_cheap > 0:
            result = _ilp_select_slots(
                block_prices, N, K_cheap, ilp_B, find_expensive=False
            )
            if result is not None:
                for err in _validate_solution(
                    result, N, K_cheap, ilp_B, f"ref_cheap block@{block_start_idx}"
                ):
                    tc.fail(err)
                for i in range(slots_per_period):
                    cheap_plan[block_start_idx + i] = result[i]
            else:
                if K_cheap >= ilp_B and (N - K_cheap) >= ilp_B:
                    tc.fail(
                        f"[ref_cheap block@{block_start_idx}] "
                        f"ILP returned infeasible but K={K_cheap} >= B={ilp_B} "
                        f"and N-K={N - K_cheap} >= B={ilp_B}"
                    )

        # Solve and validate expensive (blocking)
        if case.blocking and K_expensive > 0:
            result = _ilp_select_slots(
                block_prices, N, K_expensive, ilp_B, find_expensive=True
            )
            if result is not None:
                for err in _validate_solution(
                    result, N, K_expensive, ilp_B, f"ref_exp block@{block_start_idx}"
                ):
                    tc.fail(err)
                for i in range(slots_per_period):
                    expensive_plan[block_start_idx + i] = result[i]
            else:
                if K_expensive >= ilp_B and (N - K_expensive) >= ilp_B:
                    tc.fail(
                        f"[ref_exp block@{block_start_idx}] "
                        f"ILP returned infeasible but K={K_expensive} >= B={ilp_B} "
                        f"and N-K={N - K_expensive} >= B={ilp_B}"
                    )

    return cheap_plan, expensive_plan


def _compare_with_device(
    tc: TestContext,
    case: _TestCase,
    prices: list,
    first_date: int,
    ref_cheap: list,
    ref_expensive: list,
):
    """Push prices + config to device, wait for plan, compare with reference."""
    is_15min = case.price_resolution == 15
    B = case.min_block_15m
    ilp_B = B if is_15min else (math.ceil(B / 4) if B > 0 else 0)

    # Snapshot the current plan so we can detect when it changes.
    old_plan = tc.api("heating/plan")

    # Configure DAP push mode.
    dap_config = tc.api("day_ahead_prices/config")
    resolution_enum = 0 if case.price_resolution == 15 else 1
    dap_config["enable"] = True
    dap_config["source"] = 1  # Push
    dap_config["resolution"] = resolution_enum
    tc.api("day_ahead_prices/config_update", dap_config)

    # Push prices.
    tc.api(
        "day_ahead_prices/prices_update",
        {
            "first_date": first_date,
            "resolution": resolution_enum,
            "prices": prices,
        },
    )

    # Configure heating.
    # Zero out heating-curve endpoint hours so they don't conflict
    # with shorter control periods (the firmware validates that
    # extended + blocking <= control_period at each curve endpoint).
    heating_config = tc.api("heating/config")
    heating_config["extended"] = case.extended
    heating_config["extended_hours"] = case.extended_hours
    heating_config["blocking"] = case.blocking
    heating_config["blocking_hours"] = case.blocking_hours
    heating_config["control_period"] = case.control_period
    heating_config["min_hold_time"] = case.min_hold_time
    heating_config["enable_heating_curve"] = False
    heating_config["extended_hours_warm"] = 0
    heating_config["extended_hours_cold"] = 0
    heating_config["blocking_hours_warm"] = 0
    heating_config["blocking_hours_cold"] = 0
    tc.api("heating/config_update", heating_config)

    # Wait for the plan to change.
    def plan_changed():
        try:
            plan = tc.api("heating/plan")
        except Exception as e:
            raise AssertionError(f"Failed to read plan: {e}") from e
        if plan == old_plan:
            raise AssertionError("Plan has not changed yet")
        return plan

    plan = tc.wait_for(plan_changed, poll_delay=1)

    dev_cheap = plan.get("cheap", [])
    dev_expensive = plan.get("expensive", [])

    tc.assert_eq(len(prices), len(dev_cheap))
    tc.assert_eq(len(prices), len(dev_expensive))

    # Validate device solution and compare costs per block.
    blocks = _get_control_period_blocks(
        case.duration_hours,
        first_date,
        case.price_resolution,
        len(prices),
        _tz_offset_minutes,
    )

    for block_start_idx, slots_per_period in blocks:
        block_prices = prices[block_start_idx : block_start_idx + slots_per_period]

        if is_15min:
            val_N = len(block_prices)
            val_B = B
            val_K_cheap = case.extended_hours * 4
            val_K_exp = case.blocking_hours * 4
        else:
            val_N = len(block_prices)
            val_B = ilp_B
            val_K_cheap = case.extended_hours
            val_K_exp = case.blocking_hours

        dev_block_cheap = dev_cheap[
            block_start_idx : block_start_idx + slots_per_period
        ]
        dev_block_exp = dev_expensive[
            block_start_idx : block_start_idx + slots_per_period
        ]
        ref_block_cheap = ref_cheap[
            block_start_idx : block_start_idx + slots_per_period
        ]
        ref_block_exp = ref_expensive[
            block_start_idx : block_start_idx + slots_per_period
        ]

        # Validate device solution constraints.
        if case.extended and sum(dev_block_cheap) > 0:
            for err in _validate_solution(
                dev_block_cheap,
                val_N,
                val_K_cheap,
                val_B,
                f"dev_cheap block@{block_start_idx}",
            ):
                tc.fail(err)

        if case.blocking and sum(dev_block_exp) > 0:
            for err in _validate_solution(
                dev_block_exp,
                val_N,
                val_K_exp,
                val_B,
                f"dev_exp block@{block_start_idx}",
            ):
                tc.fail(err)

        # Compare K counts.
        if case.extended:
            tc.assert_eq(sum(ref_block_cheap), sum(dev_block_cheap))
        if case.blocking:
            tc.assert_eq(sum(ref_block_exp), sum(dev_block_exp))

        # Compare costs: device must be no worse than ILP.
        # For cheap: device cost <= ILP cost (lower is better).
        if case.extended and sum(dev_block_cheap) == sum(ref_block_cheap):
            dev_cost = _compute_cost(dev_block_cheap, block_prices)
            ref_cost = _compute_cost(ref_block_cheap, block_prices)
            tc.assert_le(ref_cost, dev_cost)

        # For expensive: device cost >= ILP cost (higher is better).
        if case.blocking and sum(dev_block_exp) == sum(ref_block_exp):
            dev_cost = _compute_cost(dev_block_exp, block_prices)
            ref_cost = _compute_cost(ref_block_exp, block_prices)
            tc.assert_ge(ref_cost, dev_cost)


def _run_test(tc: TestContext, case: _TestCase):
    prices, first_date = _generate_test_data(case, _tz_offset_minutes)

    ref_cheap, ref_expensive = _run_and_validate_reference(tc, case, prices, first_date)

    if tc._esp_host is not None:
        _compare_with_device(tc, case, prices, first_date, ref_cheap, ref_expensive)


# cp values: 0=24h, 1=12h, 2=8h, 3=6h, 4=4h
def _case(tc, ext_h, blk_h, cp, mht, pattern, **kwargs):
    _run_test(
        tc,
        _TestCase(
            extended_hours=ext_h,
            blocking_hours=blk_h,
            control_period=cp,
            min_hold_time=mht,
            price_pattern=pattern,
            **kwargs,
        ),
    )


# Basic functionality
def test_basic_24h_30min(tc: TestContext):
    _case(tc, 3, 2, 0, 30, "sine")


def test_basic_12h_45min(tc: TestContext):
    _case(tc, 1, 1, 1, 45, "sine")


def test_basic_8h_15min(tc: TestContext):
    _case(tc, 2, 1, 2, 15, "sine")


def test_basic_6h_30min(tc: TestContext):
    _case(tc, 1, 1, 3, 30, "sine")


def test_basic_4h_10min(tc: TestContext):
    _case(tc, 1, 1, 4, 10, "sine")


# Varying hold times
def test_hold_10min(tc: TestContext):
    _case(tc, 3, 2, 0, 10, "random", pattern_seed=100)


def test_hold_15min(tc: TestContext):
    _case(tc, 3, 2, 0, 15, "random", pattern_seed=101)


def test_hold_20min(tc: TestContext):
    _case(tc, 3, 2, 0, 20, "random", pattern_seed=102)


def test_hold_30min(tc: TestContext):
    _case(tc, 3, 2, 0, 30, "random", pattern_seed=103)


def test_hold_45min(tc: TestContext):
    _case(tc, 3, 2, 0, 45, "random", pattern_seed=104)


def test_hold_60min(tc: TestContext):
    _case(tc, 3, 2, 0, 60, "random", pattern_seed=105)


# Price patterns
def test_pattern_flat(tc: TestContext):
    _case(tc, 4, 3, 0, 30, "flat")


def test_pattern_ascending(tc: TestContext):
    _case(tc, 4, 3, 0, 30, "ascending")


def test_pattern_descending(tc: TestContext):
    _case(tc, 4, 3, 0, 30, "descending")


def test_pattern_sawtooth(tc: TestContext):
    _case(tc, 4, 3, 0, 30, "sawtooth")


def test_pattern_sine(tc: TestContext):
    _case(tc, 4, 3, 0, 30, "sine")


def test_pattern_random(tc: TestContext):
    _case(tc, 4, 3, 0, 30, "random")


def test_pattern_two_plateaus(tc: TestContext):
    _case(tc, 4, 3, 0, 30, "two_plateaus")


def test_pattern_realistic_de(tc: TestContext):
    _case(tc, 4, 3, 0, 30, "realistic_de")


def test_pattern_negative(tc: TestContext):
    _case(tc, 4, 3, 0, 30, "negative")


def test_pattern_spiky(tc: TestContext):
    _case(tc, 4, 3, 0, 30, "spiky")


# Edge cases: select all / none
def test_select_all_24h(tc: TestContext):
    _case(tc, 24, 0, 0, 15, "sine", blocking=False)


def test_block_all_24h(tc: TestContext):
    _case(tc, 0, 24, 0, 15, "sine", extended=False)


def test_full_coverage(tc: TestContext):
    _case(tc, 6, 6, 1, 30, "sine")


# Tight feasibility
# 4h period, 1h ext, hold=60 => K=4, B=4, N=16, N-K=12 => feasible
def test_tight_feasible(tc: TestContext):
    _case(tc, 1, 0, 4, 60, "random", blocking=False, pattern_seed=200)


# 6h period, 1h ext, hold=60 => K=4, B=4, N=24, N-K=20 => feasible
def test_tight_6h(tc: TestContext):
    _case(tc, 1, 0, 3, 60, "random", blocking=False, pattern_seed=201)


# Large K relative to N
def test_large_k_ext(tc: TestContext):
    _case(tc, 10, 2, 1, 30, "realistic_de")


def test_large_k_blk(tc: TestContext):
    _case(tc, 2, 10, 1, 30, "realistic_de")


# Symmetric
def test_symmetric(tc: TestContext):
    _case(tc, 6, 6, 0, 30, "sine")


# Only one mode
def test_only_extended(tc: TestContext):
    _case(tc, 4, 0, 0, 30, "sawtooth", blocking=False)


def test_only_blocking(tc: TestContext):
    _case(tc, 0, 4, 0, 30, "sawtooth", extended=False)


# Flat prices (all equivalent cost)
def test_flat_prices(tc: TestContext):
    _case(tc, 3, 2, 0, 30, "flat")


# 60-minute resolution
def test_60min_basic(tc: TestContext):
    _case(tc, 3, 2, 0, 30, "sine", price_resolution=60)


def test_60min_random(tc: TestContext):
    _case(tc, 4, 3, 1, 45, "random", price_resolution=60, pattern_seed=300)


def test_60min_realistic(tc: TestContext):
    _case(tc, 6, 3, 0, 30, "realistic_de", price_resolution=60)


def suite_teardown(tc: TestContext):
    if _original_heating_config is not None:
        tc.api("heating/config_update", _original_heating_config)
    if _original_dap_config is not None:
        tc.api("day_ahead_prices/config_update", _original_dap_config)


if __name__ == "__main__":
    run_testsuite(locals())
