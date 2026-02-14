#!/usr/bin/env python3
"""Test script for the heating module's DP-based plan computation.

Pushes known day-ahead prices to a WARP Energy Manager,
configures heating parameters, reads back the computed plan,
independently solves the same optimization problem using an
ILP reference solver (scipy.optimize.milp), and compares results.

The ILP formulation is a fundamentally different mathematical approach
from the firmware's block-based Dynamic Programming, making it a
truly independent cross-check.

Usage:
    python3 test_heating_plan.py <device-ip> [options]

Prerequisites on the device:
    - day_ahead_prices/config: enable=true, source=1 (Push)

Prerequisites on the host:
    - scipy (pip install scipy)

Example:
    python3 test_heating_plan.py 192.168.0.33
    python3 test_heating_plan.py 192.168.0.33 --resolution 60
    python3 test_heating_plan.py 192.168.0.33 --verbose
    python3 test_heating_plan.py --skip-device  # offline mode, ILP solver only
"""

import argparse
import json
import math
import random
import sys
import time
import urllib.request
import urllib.error
from dataclasses import dataclass, field
from typing import Optional

try:
    from scipy.optimize import milp, LinearConstraint, Bounds
    import numpy as np
except ImportError:
    print("ERROR: scipy is required. Install with: pip install scipy")
    sys.exit(1)


# ---------------------------------------------------------------------------
# Device communication helpers
# ---------------------------------------------------------------------------

def get_json(host: str, path: str, timeout: int = 5):
    """GET a JSON endpoint from the device."""
    url = f"http://{host}/{path}"
    req = urllib.request.Request(url)
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        return json.loads(resp.read())


def put_json(host: str, path: str, payload, timeout: int = 10):
    """PUT a JSON payload to the device. Returns (status_code, response_body)."""
    url = f"http://{host}/{path}"
    data = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(url, data=data, method="PUT")
    req.add_header("Content-Type", "application/json")
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            body = resp.read().decode("utf-8", errors="replace")
            return resp.status, body
    except urllib.error.HTTPError as e:
        body = e.read().decode("utf-8", errors="replace")
        return e.code, body


# ---------------------------------------------------------------------------
# ILP reference solver 
# ---------------------------------------------------------------------------

def ilp_select_slots(slot_prices: list, N: int, K: int, B: int,
                     find_expensive: bool) -> Optional[list]:
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

    all_constraints = []
    if A_ub.shape[0] > 0:
        all_constraints.append(LinearConstraint(A_ub, ub=b_ub))
    all_constraints.append(LinearConstraint(A_eq, lb=b_eq, ub=b_eq))

    result = milp(c, integrality=integrality, bounds=bounds,
                  constraints=all_constraints,
                  options={"time_limit": 30})

    if not result.success:
        return None

    x = [bool(round(v)) for v in result.x]

    # Verify sum
    if sum(x) != K:
        return None

    return x


# ---------------------------------------------------------------------------
# Validation helpers
# ---------------------------------------------------------------------------

def get_runs(selection: list) -> list:
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


def validate_solution(selection: list, N: int, K: int, B: int,
                      label: str = "") -> list:
    """Validate that a solution satisfies all constraints. Returns list of errors."""
    errors = []
    prefix = f"[{label}] " if label else ""

    if len(selection) != N:
        errors.append(f"{prefix}Length mismatch: got {len(selection)}, expected {N}")
        return errors

    actual_k = sum(selection)
    if actual_k != K:
        errors.append(f"{prefix}K mismatch: got {actual_k}, expected {K}")

    runs = get_runs(selection)
    for value, length in runs:
        if length < B:
            kind = "active" if value else "inactive"
            errors.append(f"{prefix}Run-length violation: {kind} run of length {length} < B={B}")

    return errors


def compute_cost(selection: list, prices: list) -> int:
    """Compute total cost of selected slots."""
    return sum(p for s, p in zip(selection, prices) if s)


# ---------------------------------------------------------------------------
# Price pattern generators
# ---------------------------------------------------------------------------

def make_flat(count: int, price: int = 5000) -> list:
    """All slots have the same price."""
    return [price] * count


def make_ascending(count: int, base: int = 1000, step: int = 100) -> list:
    """Monotonically increasing prices."""
    return [base + i * step for i in range(count)]


def make_descending(count: int, base: int = 10000, step: int = 100) -> list:
    """Monotonically decreasing prices."""
    return [base - i * step for i in range(count)]


def make_sawtooth(count: int, period: int = 8, low: int = 1000,
                  high: int = 9000) -> list:
    """Sawtooth wave: ramp up then drop."""
    prices = []
    for i in range(count):
        phase = i % period
        price = low + (high - low) * phase // (period - 1) if period > 1 else low
        prices.append(price)
    return prices


def make_sine(count: int, base: int = 5000, amplitude: int = 3000) -> list:
    """Sinusoidal price curve."""
    return [int(base + amplitude * math.sin(2 * math.pi * i / count))
            for i in range(count)]


def make_random(count: int, low: int = 500, high: int = 15000,
                seed: int = 42) -> list:
    """Random prices with fixed seed for reproducibility."""
    rng = random.Random(seed)
    return [rng.randint(low, high) for _ in range(count)]


def make_two_plateaus(count: int, low: int = 2000,
                      high: int = 8000) -> list:
    """First half cheap, second half expensive."""
    mid = count // 2
    return [low] * mid + [high] * (count - mid)


def make_realistic_de(count: int) -> list:
    """Realistic German day-ahead price pattern (ct/1000 per kWh).

    Simulates typical pattern: overnight low, morning peak, midday solar dip,
    evening peak, then decline.
    """
    hourly_pattern = [
        3200, 2800, 2500, 2300, 2400, 3000,   # 00-05: overnight low
        4500, 6200, 7800, 7200, 6500, 5800,   # 06-11: morning ramp + peak
        4200, 3500, 3200, 3800, 5200, 7500,   # 12-17: solar dip + evening ramp
        8500, 7800, 6200, 5000, 4200, 3600,   # 18-23: evening peak + decline
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


def make_negative_prices(count: int) -> list:
    """Pattern with some negative prices (grid pays you to consume)."""
    base_pattern = make_sine(count, base=2000, amplitude=4000)
    return [max(p, -2000) for p in base_pattern]


def make_spiky(count: int, base: int = 3000, spike: int = 25000,
               spike_positions: Optional[list] = None) -> list:
    """Mostly flat with extreme price spikes."""
    prices = [base] * count
    if spike_positions is None:
        spike_positions = [count // 4, count // 2, 3 * count // 4]
    for pos in spike_positions:
        if 0 <= pos < count:
            prices[pos] = spike
    return prices


# ---------------------------------------------------------------------------
# Test case definition
# ---------------------------------------------------------------------------

@dataclass
class TestCase:
    """A single test configuration."""
    name: str
    extended_hours: int
    blocking_hours: int
    control_period: int       # enum: 0=24h, 1=12h, 2=8h, 3=6h, 4=4h
    min_hold_time: int        # minutes (10-60)
    price_pattern: str        # name of pattern generator
    price_resolution: int     # 15 or 60 minutes
    price_hours: int = 48     # hours of price data
    extended: bool = True
    blocking: bool = True
    expect_fallback: bool = False  # if True, DP is expected to be infeasible
    pattern_seed: int = 42    # seed for random pattern generator

    @property
    def duration_hours(self) -> int:
        return {0: 24, 1: 12, 2: 8, 3: 6, 4: 4}[self.control_period]

    @property
    def min_block_15m(self) -> int:
        return (self.min_hold_time + 14) // 15


@dataclass
class TestResult:
    """Result of a single test case."""
    test: TestCase
    passed: bool
    errors: list = field(default_factory=list)
    ref_cheap_cost: Optional[int] = None
    ref_expensive_cost: Optional[int] = None
    dev_cheap_cost: Optional[int] = None
    dev_expensive_cost: Optional[int] = None


# ---------------------------------------------------------------------------
# Core test logic
# ---------------------------------------------------------------------------

PATTERN_GENERATORS = {
    "flat":          make_flat,
    "ascending":     make_ascending,
    "descending":    make_descending,
    "sawtooth":      make_sawtooth,
    "sine":          make_sine,
    "random":        make_random,
    "two_plateaus":  make_two_plateaus,
    "realistic_de":  make_realistic_de,
    "negative":      make_negative_prices,
    "spiky":         make_spiky,
}


def generate_prices(pattern_name: str, count: int,
                    seed: int = 42) -> list:
    """Generate a price array using the named pattern."""
    gen = PATTERN_GENERATORS.get(pattern_name)
    if gen is None:
        raise ValueError(f"Unknown pattern: {pattern_name}")
    if pattern_name == "random":
        return gen(count, seed=seed)
    return gen(count)


def get_control_period_blocks(duration_hours: int, first_date: int,
                              resolution_minutes: int,
                              num_prices: int,
                              tz_offset_minutes: int = 0) -> list:
    """Compute control period block boundaries, matching firmware logic.

    Returns list of (block_start_price_index, slots_per_period) for each
    block that is fully covered by price data.

    The firmware aligns blocks to local midnight + multiples of duration_hours.
    tz_offset_minutes is the UTC offset in minutes (e.g. 60 for CET).
    """
    slots_per_hour = 60 // resolution_minutes
    slots_per_period = duration_hours * slots_per_hour
    duration_minutes = duration_hours * 60

    # first_date is in UTC minutes. Convert to local time to find midnight alignment.
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


def run_reference_solver(prices: list, tc: TestCase, first_date: int,
                         verbose: bool = False,
                         tz_offset_minutes: int = 0):
    """Run the ILP reference solver for one test case.

    Returns (cheap_plan, expensive_plan) as bool arrays aligned to the
    price array (same length as prices).
    """
    N_prices = len(prices)
    resolution_minutes = tc.price_resolution
    is_15min = (resolution_minutes == 15)
    duration_hours = tc.duration_hours

    cheap_plan = [False] * N_prices
    expensive_plan = [False] * N_prices

    blocks = get_control_period_blocks(duration_hours, first_date,
                                       resolution_minutes, N_prices,
                                       tz_offset_minutes)

    B = tc.min_block_15m

    for block_start_idx, slots_per_period in blocks:
        block_prices = prices[block_start_idx:block_start_idx + slots_per_period]

        # When resolution is 60-min, the firmware runs DP at hourly granularity
        # (not on replicated 15-min sub-slots) to avoid sub-hour fragmentation.
        # The ILP reference must match this behavior.
        if is_15min:
            ilp_prices = block_prices
            N = len(ilp_prices)
            K_cheap = tc.extended_hours * 4
            K_expensive = tc.blocking_hours * 4
            ilp_B = B
        else:
            ilp_prices = block_prices  # hourly prices, no replication
            N = len(ilp_prices)  # = duration_hours
            K_cheap = tc.extended_hours
            K_expensive = tc.blocking_hours
            ilp_B = math.ceil(B / 4) if B > 0 else 0  # ceil(min_block_15m / 4)

        if verbose:
            print(f"  Block [{block_start_idx}..{block_start_idx + slots_per_period - 1}]: "
                  f"N={N}, K_cheap={K_cheap}, K_exp={K_expensive}, B={ilp_B}")

        # Solve cheap (extended)
        if tc.extended and K_cheap > 0:
            result = ilp_select_slots(ilp_prices, N, K_cheap, ilp_B,
                                      find_expensive=False)
            if result is not None:
                # Map back to price array resolution
                if is_15min:
                    for i in range(slots_per_period):
                        cheap_plan[block_start_idx + i] = result[i]
                else:
                    for i in range(slots_per_period):
                        cheap_plan[block_start_idx + i] = result[i]

        # Solve expensive (blocking)
        if tc.blocking and K_expensive > 0:
            result = ilp_select_slots(ilp_prices, N, K_expensive, ilp_B,
                                      find_expensive=True)
            if result is not None:
                if is_15min:
                    for i in range(slots_per_period):
                        expensive_plan[block_start_idx + i] = result[i]
                else:
                    for i in range(slots_per_period):
                        expensive_plan[block_start_idx + i] = result[i]

    return cheap_plan, expensive_plan


# ---------------------------------------------------------------------------
# Device interaction
# ---------------------------------------------------------------------------

def ensure_dap_push_mode(host: str, resolution: int) -> bool:
    """Ensure day_ahead_prices is enabled in Push mode with correct resolution."""
    config = get_json(host, "day_ahead_prices/config")
    resolution_enum = 0 if resolution == 15 else 1

    needs_update = (
        not config.get("enable", False) or
        config.get("source", 0) != 1 or
        config.get("resolution", 1) != resolution_enum
    )

    if needs_update:
        config["enable"] = True
        config["source"] = 1
        config["resolution"] = resolution_enum
        status, body = put_json(host, "day_ahead_prices/config_update", config)
        if status != 200:
            print(f"  ERROR: Failed to configure DAP push mode: HTTP {status}: {body}")
            return False
        time.sleep(1)

    return True


def push_prices(host: str, prices: list, resolution: int,
                first_date: int) -> bool:
    """Push price data to the device."""
    resolution_enum = 0 if resolution == 15 else 1
    payload = {
        "first_date": first_date,
        "resolution": resolution_enum,
        "prices": prices,
    }
    status, body = put_json(host, "day_ahead_prices/prices_update", payload)
    if status != 200:
        print(f"  ERROR: Failed to push prices: HTTP {status}: {body}")
        return False
    return True


def configure_heating(host: str, tc: TestCase) -> bool:
    """Configure the heating module for a test case."""
    config = get_json(host, "heating/config")
    config["extended"] = tc.extended
    config["extended_hours"] = tc.extended_hours
    config["blocking"] = tc.blocking
    config["blocking_hours"] = tc.blocking_hours
    config["control_period"] = tc.control_period
    config["min_hold_time"] = tc.min_hold_time

    status, body = put_json(host, "heating/config_update", config)
    if status != 200:
        print(f"  ERROR: Failed to configure heating: HTTP {status}: {body}")
        return False
    return True


def read_plan(host: str) -> Optional[dict]:
    """Read the heating plan from the device."""
    try:
        return get_json(host, "heating/plan")
    except Exception as e:
        print(f"  ERROR: Failed to read plan: {e}")
        return None


def wait_for_plan_update(host: str, max_wait: int = 90,
                         verbose: bool = False) -> Optional[dict]:
    """Wait for the plan to be computed (non-empty).

    The holding time guard in update() can delay update_plan() by up to
    min_hold_time minutes. We poll until the plan arrays are non-empty.
    """
    if verbose:
        print(f"  Waiting for plan update (max {max_wait}s)...")

    for elapsed in range(max_wait):
        plan = read_plan(host)
        if plan is None:
            time.sleep(1)
            continue

        cheap = plan.get("cheap", [])
        expensive = plan.get("expensive", [])
        if len(cheap) > 0 or len(expensive) > 0:
            if verbose:
                print(f"  Plan ready after {elapsed}s: "
                      f"cheap={len(cheap)} slots, expensive={len(expensive)} slots")
            return plan

        time.sleep(1)

    print(f"  WARNING: Plan still empty after {max_wait}s")
    return read_plan(host)


def wait_for_plan_change(host: str, old_plan: Optional[dict],
                         max_wait: int = 90,
                         verbose: bool = False) -> Optional[dict]:
    """Wait for the plan to change from old_plan.

    Polls the device until the plan differs from old_plan, indicating that
    update_plan() has run with the new config/prices.
    """
    if verbose:
        print(f"  Waiting for plan to change (max {max_wait}s)...")

    for elapsed in range(max_wait):
        time.sleep(1)
        plan = read_plan(host)
        if plan is None:
            continue

        # Consider it changed if it differs from the old plan
        if plan != old_plan:
            if verbose:
                print(f"  Plan changed after {elapsed + 1}s")
            return plan

    print(f"  WARNING: Plan unchanged after {max_wait}s")
    return read_plan(host)


# ---------------------------------------------------------------------------
# Test runner
# ---------------------------------------------------------------------------

def run_single_test(tc: TestCase, host: Optional[str],
                    verbose: bool = False,
                    tz_offset_minutes: int = 0) -> TestResult:
    """Run a single test case."""
    errors = []

    # Generate prices
    slots_per_hour = 60 // tc.price_resolution
    count = tc.price_hours * slots_per_hour
    prices = generate_prices(tc.price_pattern, count, seed=tc.pattern_seed)

    # Align first_date to local midnight (UTC midnight offset by tz)
    # so that control period blocks align cleanly with price data.
    now = time.time()
    # Local midnight in UTC seconds
    local_midnight_utc = (int(now) + tz_offset_minutes * 60) // 86400 * 86400 - tz_offset_minutes * 60
    first_date = local_midnight_utc // 60  # in UTC minutes

    # --- ILP reference solver ---
    ref_cheap, ref_expensive = run_reference_solver(
        prices, tc, first_date, verbose=verbose,
        tz_offset_minutes=tz_offset_minutes)

    # Validate ILP reference solution per block at 15-min resolution
    blocks = get_control_period_blocks(tc.duration_hours, first_date,
                                       tc.price_resolution, len(prices),
                                       tz_offset_minutes)
    is_15min = (tc.price_resolution == 15)
    B = tc.min_block_15m

    ref_cheap_cost = None
    ref_expensive_cost = None

    # When resolution is 60-min, the firmware runs DP at hourly granularity.
    # The reference ILP must match.
    if is_15min:
        ilp_B = B
    else:
        ilp_B = math.ceil(B / 4) if B > 0 else 0

    for block_start_idx, slots_per_period in blocks:
        block_prices = prices[block_start_idx:block_start_idx + slots_per_period]

        if is_15min:
            ilp_prices = block_prices
            N_ilp = len(ilp_prices)
            K_cheap = tc.extended_hours * 4
            K_expensive = tc.blocking_hours * 4
        else:
            ilp_prices = block_prices  # hourly prices directly
            N_ilp = len(ilp_prices)  # = duration_hours
            K_cheap = tc.extended_hours
            K_expensive = tc.blocking_hours

        if tc.extended and K_cheap > 0:
            result_ilp = ilp_select_slots(ilp_prices, N_ilp, K_cheap, ilp_B,
                                          find_expensive=False)
            if result_ilp is not None:
                errs = validate_solution(result_ilp, N_ilp, K_cheap, ilp_B,
                                         f"ref_cheap block@{block_start_idx}")
                errors.extend(errs)
                ref_cheap_cost = compute_cost(result_ilp, ilp_prices)
            else:
                if K_cheap >= ilp_B and (N_ilp - K_cheap) >= ilp_B:
                    errors.append(
                        f"[ref_cheap block@{block_start_idx}] "
                        f"ILP returned infeasible but K={K_cheap} >= B={ilp_B} "
                        f"and N-K={N_ilp - K_cheap} >= B={ilp_B}")

        if tc.blocking and K_expensive > 0:
            result_ilp = ilp_select_slots(ilp_prices, N_ilp, K_expensive, ilp_B,
                                          find_expensive=True)
            if result_ilp is not None:
                errs = validate_solution(result_ilp, N_ilp, K_expensive, ilp_B,
                                         f"ref_exp block@{block_start_idx}")
                errors.extend(errs)
                ref_expensive_cost = compute_cost(result_ilp, ilp_prices)
            else:
                if K_expensive >= ilp_B and (N_ilp - K_expensive) >= ilp_B:
                    errors.append(
                        f"[ref_exp block@{block_start_idx}] "
                        f"ILP returned infeasible but K={K_expensive} >= B={ilp_B} "
                        f"and N-K={N_ilp - K_expensive} >= B={ilp_B}")

    # --- Device comparison ---
    dev_cheap_cost = None
    dev_expensive_cost = None

    if host is not None:
        # Snapshot the plan before pushing changes so we can detect when it updates
        old_plan = read_plan(host)

        if not ensure_dap_push_mode(host, tc.price_resolution):
            errors.append("Failed to configure DAP push mode")
        elif not push_prices(host, prices, tc.price_resolution, first_date):
            errors.append("Failed to push prices")
        elif not configure_heating(host, tc):
            errors.append("Failed to configure heating")
        else:
            plan = wait_for_plan_change(host, old_plan, max_wait=90,
                                         verbose=verbose)

            if plan is None:
                errors.append("Failed to read plan from device")
            else:
                dev_cheap = plan.get("cheap", [])
                dev_expensive = plan.get("expensive", [])

                if len(dev_cheap) != len(prices):
                    errors.append(
                        f"Device cheap plan length {len(dev_cheap)} != "
                        f"price count {len(prices)}")
                if len(dev_expensive) != len(prices):
                    errors.append(
                        f"Device expensive plan length {len(dev_expensive)} != "
                        f"price count {len(prices)}")

                if (len(dev_cheap) == len(prices)
                        and len(dev_expensive) == len(prices)):
                    for block_start_idx, slots_per_period in blocks:
                        block_prices = prices[block_start_idx:
                                              block_start_idx + slots_per_period]

                        dev_block_cheap = dev_cheap[block_start_idx:
                                                    block_start_idx + slots_per_period]
                        ref_block_cheap = ref_cheap[block_start_idx:
                                                    block_start_idx + slots_per_period]
                        dev_block_exp = dev_expensive[block_start_idx:
                                                      block_start_idx + slots_per_period]
                        ref_block_exp = ref_expensive[block_start_idx:
                                                      block_start_idx + slots_per_period]

                        dev_cheap_k = sum(dev_block_cheap)
                        ref_cheap_k = sum(ref_block_cheap)
                        dev_exp_k = sum(dev_block_exp)
                        ref_exp_k = sum(ref_block_exp)

                        # Validate device solution constraints at the DP granularity
                        if is_15min:
                            val_N = len(block_prices)
                            val_B = B
                            val_K_cheap = tc.extended_hours * 4
                            val_K_exp = tc.blocking_hours * 4
                        else:
                            val_N = len(block_prices)  # hourly
                            val_B = ilp_B
                            val_K_cheap = tc.extended_hours
                            val_K_exp = tc.blocking_hours

                        if tc.extended and dev_cheap_k > 0:
                            errs = validate_solution(
                                dev_block_cheap, val_N, val_K_cheap, val_B,
                                f"dev_cheap block@{block_start_idx}")
                            errors.extend(errs)

                        if tc.blocking and dev_exp_k > 0:
                            errs = validate_solution(
                                dev_block_exp, val_N, val_K_exp, val_B,
                                f"dev_exp block@{block_start_idx}")
                            errors.extend(errs)

                        # Compare K counts
                        if tc.extended and dev_cheap_k != ref_cheap_k:
                            errors.append(
                                f"Cheap K mismatch block@{block_start_idx}: "
                                f"device={dev_cheap_k}, ref={ref_cheap_k}")

                        if tc.blocking and dev_exp_k != ref_exp_k:
                            errors.append(
                                f"Expensive K mismatch block@{block_start_idx}: "
                                f"device={dev_exp_k}, ref={ref_exp_k}")

                        # Compare costs: device should be no worse than ILP
                        if tc.extended and dev_cheap_k == ref_cheap_k:
                            d_cost = compute_cost(dev_block_cheap, block_prices)
                            r_cost = compute_cost(ref_block_cheap, block_prices)
                            dev_cheap_cost = d_cost
                            if d_cost > r_cost:
                                errors.append(
                                    f"Device cheap cost suboptimal "
                                    f"block@{block_start_idx}: "
                                    f"device={d_cost} > ILP={r_cost}")
                            elif d_cost != r_cost and verbose:
                                print(f"  Note: Equivalent cheap solutions "
                                      f"block@{block_start_idx}: "
                                      f"dev={d_cost}, ILP={r_cost}")

                        if tc.blocking and dev_exp_k == ref_exp_k:
                            d_cost = compute_cost(dev_block_exp, block_prices)
                            r_cost = compute_cost(ref_block_exp, block_prices)
                            dev_expensive_cost = d_cost
                            if d_cost < r_cost:
                                errors.append(
                                    f"Device expensive cost suboptimal "
                                    f"block@{block_start_idx}: "
                                    f"device={d_cost} < ILP={r_cost}")

    passed = len(errors) == 0
    return TestResult(
        test=tc, passed=passed, errors=errors,
        ref_cheap_cost=ref_cheap_cost,
        ref_expensive_cost=ref_expensive_cost,
        dev_cheap_cost=dev_cheap_cost,
        dev_expensive_cost=dev_expensive_cost,
    )


# ---------------------------------------------------------------------------
# Test suite
# ---------------------------------------------------------------------------

def build_test_suite(resolution: int = 15) -> list:
    """Build a comprehensive test suite."""
    tests = []
    test_id = 0

    def add(name, ext_h, blk_h, cp, mht, pattern, **kwargs):
        nonlocal test_id
        test_id += 1
        tests.append(TestCase(
            name=f"T{test_id:02d}_{name}",
            extended_hours=ext_h,
            blocking_hours=blk_h,
            control_period=cp,
            min_hold_time=mht,
            price_pattern=pattern,
            price_resolution=resolution,
            **kwargs,
        ))

    # --- Basic functionality ---
    add("basic_24h_30min", 3, 2, 0, 30, "sine")
    add("basic_12h_45min", 1, 1, 1, 45, "sine")
    add("basic_8h_15min",  2, 1, 2, 15, "sine")
    add("basic_6h_30min",  1, 1, 3, 30, "sine")
    add("basic_4h_10min",  1, 1, 4, 10, "sine")

    # --- Varying hold times (unique seeds to ensure plan changes between tests) ---
    add("hold_10min",  3, 2, 0, 10, "random", pattern_seed=100)
    add("hold_15min",  3, 2, 0, 15, "random", pattern_seed=101)
    add("hold_20min",  3, 2, 0, 20, "random", pattern_seed=102)
    add("hold_30min",  3, 2, 0, 30, "random", pattern_seed=103)
    add("hold_45min",  3, 2, 0, 45, "random", pattern_seed=104)
    add("hold_60min",  3, 2, 0, 60, "random", pattern_seed=105)

    # --- All price patterns ---
    for pattern in PATTERN_GENERATORS:
        add(f"pattern_{pattern}", 4, 3, 0, 30, pattern)

    # --- Edge cases: select all / none ---
    add("select_all_24h",  24, 0, 0, 15, "sine", blocking=False)
    add("block_all_24h",   0, 24, 0, 15, "sine", extended=False)
    add("full_coverage",   6, 6, 1, 30, "sine")

    # --- Tight feasibility ---
    # 4h period, 1h ext, hold=60 => K=4, B=4, N=16, N-K=12 => feasible
    add("tight_feasible",  1, 0, 4, 60, "random", blocking=False, pattern_seed=200)
    # 6h period, 1h ext, hold=60 => K=4, B=4, N=24, N-K=20 => feasible
    add("tight_6h",        1, 0, 3, 60, "random", blocking=False, pattern_seed=201)

    # --- Large K relative to N ---
    add("large_k_ext",  10, 2, 1, 30, "realistic_de")
    add("large_k_blk",  2, 10, 1, 30, "realistic_de")

    # --- Symmetric ---
    add("symmetric",  6, 6, 0, 30, "sine")

    # --- Only one mode ---
    add("only_extended",   4, 0, 0, 30, "sawtooth", blocking=False)
    add("only_blocking",   0, 4, 0, 30, "sawtooth", extended=False)

    # --- Flat prices (all equivalent cost) ---
    add("flat_prices",  3, 2, 0, 30, "flat")

    # --- 60-min resolution edge cases (only when running at 15-min default) ---
    if resolution == 15:
        tests.append(TestCase(
            name="T_60min_basic",
            extended_hours=3, blocking_hours=2,
            control_period=0, min_hold_time=30,
            price_pattern="sine",
            price_resolution=60,
        ))
        tests.append(TestCase(
            name="T_60min_random",
            extended_hours=4, blocking_hours=3,
            control_period=1, min_hold_time=45,
            price_pattern="random",
            price_resolution=60,
            pattern_seed=300,
        ))
        tests.append(TestCase(
            name="T_60min_realistic",
            extended_hours=6, blocking_hours=3,
            control_period=0, min_hold_time=30,
            price_pattern="realistic_de",
            price_resolution=60,
        ))

    return tests


def print_result(result: TestResult, verbose: bool = False):
    """Print a single test result."""
    tc = result.test
    status = "PASS" if result.passed else "FAIL"
    dur_h = tc.duration_hours
    B = tc.min_block_15m

    print(f"  [{status}] {tc.name}: "
          f"ext={tc.extended_hours}h blk={tc.blocking_hours}h "
          f"period={dur_h}h hold={tc.min_hold_time}min(B={B}) "
          f"pattern={tc.price_pattern} res={tc.price_resolution}min")

    if verbose:
        if result.ref_cheap_cost is not None:
            print(f"         ILP cheap cost: {result.ref_cheap_cost}")
        if result.ref_expensive_cost is not None:
            print(f"         ILP exp cost:   {result.ref_expensive_cost}")
        if result.dev_cheap_cost is not None:
            print(f"         Dev cheap cost: {result.dev_cheap_cost}")
        if result.dev_expensive_cost is not None:
            print(f"         Dev exp cost:   {result.dev_expensive_cost}")

    if not result.passed:
        for err in result.errors:
            print(f"         ERROR: {err}")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Test heating plan DP computation against ILP reference solver.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""\
Examples:
  # Run all tests against device
  python3 test_heating_plan.py 192.168.0.33

  # Run only ILP reference solver (no device needed)
  python3 test_heating_plan.py --skip-device

  # Verbose output with 60-min resolution
  python3 test_heating_plan.py 192.168.0.33 --resolution 60 --verbose
""",
    )
    parser.add_argument("host", nargs="?", default=None,
                        help="Device IP or hostname")
    parser.add_argument("--resolution", type=int, choices=[15, 60], default=15,
                        help="Price resolution in minutes (default: 15)")
    parser.add_argument("--verbose", "-v", action="store_true",
                        help="Verbose output")
    parser.add_argument("--skip-device", action="store_true",
                        help="Skip device tests, only run ILP reference solver")
    parser.add_argument("--test", "-t", type=str, default=None,
                        help="Run only tests matching this substring")
    args = parser.parse_args()

    if args.host is None and not args.skip_device:
        print("No host specified. Use --skip-device for offline mode.")
        parser.print_help()
        sys.exit(1)

    host = None if args.skip_device else args.host

    # Determine timezone offset for block alignment
    tz_offset_minutes = 0
    if host:
        try:
            ntp_config = get_json(host, "ntp/config")
            tz_name = ntp_config.get("timezone", "UTC")
            try:
                from zoneinfo import ZoneInfo
                from datetime import datetime, timezone
                tz = ZoneInfo(tz_name)
                now_dt = datetime.now(tz)
                tz_offset_minutes = int(now_dt.utcoffset().total_seconds()) // 60
            except ImportError:
                # Fallback: assume CET for Europe/Berlin
                if "Berlin" in tz_name or "Europe" in tz_name:
                    tz_offset_minutes = 60
                print(f"  Note: zoneinfo unavailable, using offset={tz_offset_minutes}min")
        except Exception:
            pass

    tests = build_test_suite(resolution=args.resolution)

    if args.test:
        tests = [t for t in tests if args.test.lower() in t.name.lower()]
        if not tests:
            print(f"No tests matching '{args.test}'")
            sys.exit(1)

    print(f"=== Heating Plan Test Suite (ILP Reference) ===")
    print(f"Device: {host or '(offline mode)'}")
    print(f"Resolution: {args.resolution}min")
    print(f"TZ offset: {tz_offset_minutes}min")
    print(f"Tests: {len(tests)}")
    print()

    if host:
        try:
            config = get_json(host, "heating/config")
            print(f"Device heating config: {json.dumps(config, indent=2)}")
            print()
        except Exception as e:
            print(f"ERROR: Cannot connect to device: {e}")
            sys.exit(1)

    results = []
    passed = 0
    failed = 0

    for i, tc in enumerate(tests):
        if args.verbose:
            print(f"\n--- Test {i+1}/{len(tests)}: {tc.name} ---")

        result = run_single_test(tc, host, verbose=args.verbose,
                                 tz_offset_minutes=tz_offset_minutes)
        results.append(result)

        if result.passed:
            passed += 1
        else:
            failed += 1

        print_result(result, verbose=args.verbose)

    print(f"\n{'='*60}")
    print(f"Results: {passed} passed, {failed} failed, {len(tests)} total")

    if failed > 0:
        print("\nFailed tests:")
        for r in results:
            if not r.passed:
                print(f"  - {r.test.name}")
                for err in r.errors:
                    print(f"    {err}")
        sys.exit(1)
    else:
        print("\nAll tests passed!")


if __name__ == "__main__":
    main()
