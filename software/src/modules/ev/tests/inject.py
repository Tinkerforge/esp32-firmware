#!/usr/bin/env -S uv run --group tests --script

# Tests the EV injection

import random
import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext


def _random_mac() -> str:
    octets = [random.randint(0x00, 0xFF) for _ in range(6)]
    # Clear multicast bit (bit 0 of first octet)
    octets[0] &= 0xFE
    return ":".join(f"{b:02X}" for b in octets)


def test_inject_random_mac(tc: TestContext):
    """Inject a random MAC via ev/inject_ev and verify it appears in state and seen_macs."""
    mac = _random_mac()

    tc.api("ev/inject_ev", {"mac": mac})

    def _check_injected():
        state = tc.api("ev/state")
        tc.assert_eq(mac.upper(), state.get("mac"))

        seen = tc.api("ev/seen_macs")
        seen_macs = [entry["mac"] for entry in seen]
        tc.assert_in(seen_macs, mac.upper())

    tc.wait_for(_check_injected)


if __name__ == "__main__":
    run_testsuite(locals())
