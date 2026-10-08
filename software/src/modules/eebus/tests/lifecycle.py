#!/usr/bin/env -S uv run --group tests --script

# Disabling EEBUS destroys all usecase objects. Deferred tasks of the usecases that are still pending at that
# time (e.g. the 10 s subscription result check, the 20 s ack cleanup, the 1 s usecase init) must not access
# the destroyed objects, neither after disabling nor after enabling EEBUS again.

import time

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext
from software.src.modules.eebus.tests._common import *
from software.src.modules.eebus.tests._eebus_peer import LpcEnergyGuard, energy_guard_layout

# Longest deferred usecase task (ack cleanup) plus margin
PENDING_TASK_WINDOW = 25

removed_peers: list[dict] = []
peer_skis: list[str] = []


def suite_setup(tc: TestContext):
    global removed_peers
    enable_eebus(tc)
    removed_peers = isolate_eebus_peers(tc)


def suite_teardown(tc: TestContext):
    set_eebus_enabled(tc, True)
    time.sleep(2)
    for ski in peer_skis:
        tc.api("eebus/remove", {"ski": ski})
    restore_eebus_peers(tc, removed_peers)


def connect_peer_with_pending_tasks(tc: TestContext):
    """Connects a peer and lets the device subscribe and bind, which schedules deferred tasks on the device."""
    peer = start_test_peer(tc, energy_guard_layout())
    peer_skis.append(peer.identity.ski)
    peer.discover()
    peer.wait_for_heartbeat_subscription()
    LpcEnergyGuard(peer).setup()
    return peer


def expect_no_reboot(tc: TestContext, uptime_before_ms: int, waited_s: float):
    tc.assert_ge(uptime_before_ms + waited_s * 1000, device_uptime_ms(tc))


def test_disable_with_pending_tasks(tc: TestContext):
    """Disabling EEBUS while usecase tasks are pending does not crash the device."""
    tc.set_test_timeout(120)
    uptime = device_uptime_ms(tc)
    start = time.monotonic()
    peer = connect_peer_with_pending_tasks(tc)

    set_eebus_enabled(tc, False)
    peer.close()
    time.sleep(PENDING_TASK_WINDOW)
    expect_no_reboot(tc, uptime, time.monotonic() - start)

    set_eebus_enabled(tc, True)
    time.sleep(2)
    tc.assert_true(tc.api("eebus/config")["enable"])


def test_reenable_with_pending_tasks(tc: TestContext):
    """Re-enabling EEBUS while tasks of the old usecases are pending: The old tasks must not run on the new usecases."""
    tc.set_test_timeout(120)
    uptime = device_uptime_ms(tc)
    start = time.monotonic()
    peer = connect_peer_with_pending_tasks(tc)

    set_eebus_enabled(tc, False)
    peer.close()
    time.sleep(0.5)
    set_eebus_enabled(tc, True)
    time.sleep(PENDING_TASK_WINDOW)
    expect_no_reboot(tc, uptime, time.monotonic() - start)

    # The new usecases work: A new peer can connect and take control
    peer = connect_peer_with_pending_tasks(tc)
    try:
        lpc = LpcEnergyGuard(peer)
        lpc.find_servers()
        lpc.read_ids()
        peer.send_heartbeat()
        tc.assert_eq(0, lpc.write_limit(0, active=False))
        wait_for_lpc_state(tc, LPC_UNLIMITED_CONTROLLED)
    finally:
        peer.close()


if __name__ == "__main__":
    run_testsuite(locals())
