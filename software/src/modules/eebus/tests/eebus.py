#!/usr/bin/env -S uv run --group tests --script

import time
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite
else:
    import tinkerforge_util as tfutil

    tfutil.create_parent_module(__file__, "software")
    from software.test_runner.test_context import run_testsuite, TestContext


class EebusTest:
    # EEBUS state elements
    _eebus_config = {}
    _eebus_state = {}
    _eebus_usecase_state = {}


    def update_config(self, tc: TestContext):
        self._eebus_config = tc.api("eebus/config")
        self._eebus_state = tc.api("eebus/state")
        self._eebus_usecase_state = tc.api("eebus/usecases")

    def is_enabled(self):
        return self._eebus_config.get("enable", False)

    def get_ski(self, tc: TestContext):
        self.update_config(tc)
        if not self.is_enabled():
            tc.skip("Skipping get_ski. Eebus disabled")
        return self._eebus_state.get("ski", False)

    def toggle_eebus(self, tc: TestContext, on: bool):
        self.update_config(tc)
        self._eebus_config["enable"] = on
        tc.api("eebus/config_update", self._eebus_config)
        time.sleep(1) # give the module some time to process the change
        self.update_config(tc)

    def peer_in_config(self, peer_ski: str):
        for peer in self._eebus_config.get("peers", []):
            print(peer)
            if peer.get("ski") == peer_ski:
                return True
        return False

    def add_peer(self, tc: TestContext, peer_ski: str, peer_ip: str):
        tc.assert_true(self.is_enabled())
        self._eebus_config = tc.api("eebus/config")
        peer_data = {
            "ski": peer_ski,
            "ip": peer_ip,
            "port": 4712,
            "trusted": True,
            "persistent": True,
            "dns_name": "",
            "wss_path": "/ship/",
        }
        tc.api("eebus/add", peer_data)
        self.update_config(tc)

    def remove_peer(self, tc: TestContext, peer_ski: str):
        tc.assert_true(self.is_enabled())
        tc.api("eebus/remove", {"ski": peer_ski})
        self.update_config(tc)


EEbusTestSuite = EebusTest()

add_remove_test_ski = "DEADBEEF" + "0" * 32

def test_enable_eebus(tc: TestContext):
    """Test that EEBUS module can be enabled."""
    EEbusTestSuite.toggle_eebus(tc, True)
    tc.assert_eq(True, EEbusTestSuite.is_enabled())

def test_ski_generation(tc: TestContext):
    """Test that SKI is generated after enabling EEBUS."""
    ski = EEbusTestSuite.get_ski(tc)
    tc.assert_true(ski)
    tc.assert_true(len(ski) > 0)

def test_add_peer(tc: TestContext):
    """Test adding a peer to EEBUS."""
    EEbusTestSuite.add_peer(tc, add_remove_test_ski, "192.168.1.101")
    tc.assert_true(EEbusTestSuite.peer_in_config(add_remove_test_ski))

def test_remove_peer(tc: TestContext):
    """Test removing a peer from EEBUS."""
    # Ensure our peer is not in the list
    EEbusTestSuite.remove_peer(tc, add_remove_test_ski)
    tc.assert_eq(False, EEbusTestSuite.peer_in_config(add_remove_test_ski))

def test_disable_eebus(tc: TestContext):
    """Test that EEBUS module can be enabled."""
    EEbusTestSuite.toggle_eebus(tc, False)
    tc.assert_eq(False, EEbusTestSuite.is_enabled())


if __name__ == "__main__":
    run_testsuite(locals())
