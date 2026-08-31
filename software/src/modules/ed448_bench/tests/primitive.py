#!/usr/bin/env -S uv run --locked --script

import json

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import TestContext, run_testsuite


def suite_setup(tc: TestContext):
    if not tc.device_type().is_warp(4):
        tc.skip_test("Ed448 target benchmark requires WARP4")
    tc.set_test_timeout(120)


def test_kat_and_benchmark(tc: TestContext):
    tc.api("ed448_bench/run", {"iterations": 20}, timeout=5)

    def finished():
        result = tc.api("ed448_bench/result")
        assert result["status"] in (2, 3), result
        return result

    result = tc.wait_for(finished, timeout=90, poll_delay=0.5)
    assert result["status"] == 2, result
    assert result["kat_passed"], result
    assert result["error"] == 0, result
    assert result["mbedtls_passed"], result
    assert result["mbedtls_error"] == 0, result
    assert 0 < result["stack_used"] < result["stack_size"], result
    assert result["derive_us"] > 0, result
    assert result["sign_us"] > 0, result
    assert result["verify_us"] > 0, result
    assert result["shake_us"] > 0, result
    tc.dbg("ED448_METRICS " + json.dumps(result, sort_keys=True))


if __name__ == "__main__":
    run_testsuite(locals())
