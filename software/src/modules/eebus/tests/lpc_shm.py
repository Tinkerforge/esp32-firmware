#!/usr/bin/env -S uv run --group tests --script

# LPC with an Energy Guard test peer that imitates the SMA Sunny Home Manager 2.0:
# Energy Guard actors on several entities, only Generic client features, the heartbeat
# on DeviceDiagnosis feature 1000 and no device in the discovery feature addresses.
#
# Regression tests for the heartbeat lookup and IG-LPC 1.1.0 section 3.8 (multiple Energy Guard instances).
# The tests depend on each other and run in order.

import time
from urllib.error import HTTPError

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext
from software.src.modules.eebus.tests._common import *
from software.src.modules.eebus.tests._eebus_peer import (
    EebusPeer, LpcEnergyGuard, shm_like_layout,
    ERROR_NO_ERROR, ERROR_BINDING_REQUIRED,
)

peer: EebusPeer | None = None
lpc: LpcEnergyGuard | None = None
other: LpcEnergyGuard | None = None
removed_peers: list[dict] = []


def suite_setup(tc: TestContext):
    global peer, lpc, other, removed_peers
    enable_eebus(tc)
    if "lpc" not in tc.api("eebus/usecases"):
        tc.skip("LPC not available on this device")
    removed_peers = isolate_eebus_peers(tc)

    peer = start_test_peer(tc, shm_like_layout(4))
    peer.discover()
    lpc = LpcEnergyGuard(peer, [1])
    lpc.find_servers()
    other = LpcEnergyGuard(peer, [2])
    other.find_servers()


def suite_teardown(tc: TestContext):
    stop_test_peer(tc, peer)
    restore_eebus_peers(tc, removed_peers)


def test_no_heartbeat_subscription_before_binding(tc: TestContext):
    """With several Energy Guard instances the device does not know which heartbeat to use before the bindings (IG-LPC 3.8)."""
    time.sleep(5)
    tc.assert_eq([], peer.heartbeat_subscribers())


def test_subscriptions(tc: TestContext):
    """Subscriptions from the Generic client feature are accepted."""
    for client, server, t in ((lpc.lc_client, lpc.lc_server, "LoadControl"), (lpc.dc_client, lpc.dc_server, "DeviceConfiguration")):
        tc.assert_eq(ERROR_NO_ERROR, peer.subscribe(client, server, t))


def test_binding_from_other_entity_rejected(tc: TestContext):
    """The bindings on LoadControl and DeviceConfiguration have to originate from the same entity (IG-LPC 3.8)."""
    tc.assert_eq(ERROR_NO_ERROR, peer.bind(lpc.lc_client, lpc.lc_server, "LoadControl"))
    tc.assert_ne(ERROR_NO_ERROR, peer.bind(other.dc_client, other.dc_server, "DeviceConfiguration"))
    time.sleep(2)
    tc.assert_eq([], peer.heartbeat_subscribers())


def test_binding_selects_heartbeat(tc: TestContext):
    """Once both bindings come from the same entity, the device subscribes to the heartbeat on that entity (feature 1000)."""
    tc.assert_eq(ERROR_NO_ERROR, peer.bind(lpc.dc_client, lpc.dc_server, "DeviceConfiguration"))
    sub = peer.wait_for_heartbeat_subscription()
    tc.assert_eq([1], sub["serverAddress"]["entity"])
    tc.assert_eq(1000, sub["serverAddress"]["feature"])
    tc.assert_eq(1, len(peer.heartbeat_subscribers()))


def test_write_limit(tc: TestContext):
    """The bound Energy Guard instance can take control."""
    lpc.read_ids()
    peer.send_heartbeat()
    tc.assert_eq(ERROR_NO_ERROR, lpc.write_limit(0, active=False))
    wait_for_lpc_state(tc, LPC_UNLIMITED_CONTROLLED)
    tc.assert_eq(ERROR_NO_ERROR, lpc.write_limit(4200, active=True, duration=60))
    tc.assert_eq(4200, wait_for_lpc_state(tc, LPC_LIMITED)["current_limit"])


def test_write_from_unbound_instance(tc: TestContext):
    """Another Energy Guard instance of the same device is not bound and cannot write."""
    other.limit_id = lpc.limit_id
    tc.assert_eq(ERROR_BINDING_REQUIRED, other.write_limit(0, active=False))
    tc.assert_eq(LPC_LIMITED, lpc_state(tc)["usecase_state"])


def test_deactivate(tc: TestContext):
    tc.assert_eq(ERROR_NO_ERROR, lpc.write_limit(0, active=False))
    wait_for_lpc_state(tc, LPC_UNLIMITED_CONTROLLED)


def test_first_write_right_after_connect(tc: TestContext):
    """Like the Sunny Home Manager 2.0: Bind LoadControl and write the first limit right after connecting, before sending
    any heartbeat and before binding DeviceConfiguration. The device reads the heartbeats of all Energy Guard instances
    after the use case discovery, so the write follows a heartbeat and is accepted (LPC 2.2, IG-LPC 2.11)."""
    global peer, lpc
    stop_test_peer(tc, peer)
    peer = None
    # Restart the use cases: "init", no heartbeat received so far
    set_eebus_enabled(tc, False)
    time.sleep(1)
    set_eebus_enabled(tc, True)
    time.sleep(2)
    wait_for_lpc_state(tc, LPC_INIT)

    peer = start_test_peer(tc, shm_like_layout(4))
    peer.discover()
    lpc = LpcEnergyGuard(peer, [1])
    lpc.find_servers()
    tc.assert_eq(ERROR_NO_ERROR, peer.bind(lpc.lc_client, lpc.lc_server, "LoadControl"))
    lpc.read_ids()
    tc.assert_true(len(peer.received("read", "deviceDiagnosisHeartbeatData")) > 0)
    tc.assert_eq([], peer.heartbeat_subscribers())
    tc.assert_eq(ERROR_NO_ERROR, lpc.write_limit(0, active=False))
    wait_for_lpc_state(tc, LPC_UNLIMITED_CONTROLLED)
    tc.assert_eq(ERROR_NO_ERROR, peer.bind(lpc.dc_client, lpc.dc_server, "DeviceConfiguration"))
    peer.wait_for_heartbeat_subscription()


if __name__ == "__main__":
    run_testsuite(locals())
