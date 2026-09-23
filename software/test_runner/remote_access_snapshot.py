"""Restore relay-test configuration while leaving remote-access keys empty."""

import base64
import json
import os
import threading
import time
from dataclasses import dataclass
from urllib.error import HTTPError

from cryptography.hazmat.primitives.asymmetric.x25519 import X25519PrivateKey

from .test_context import TestContext


def _config_update(config: dict, *, enable: bool | None = None) -> dict:
    return {
        "enable": config["enable"] if enable is None else enable,
        "relay_host": config["relay_host"],
        "relay_port": config["relay_port"],
        "email": "",
        "cert_id": config["cert_id"],
        "mtu": config["mtu"],
    }


@dataclass
class RemoteAccessSnapshot:
    remote_config: dict
    charge_config: dict | None
    certs_state: dict
    cert_id: int

    @classmethod
    def capture(cls, tc: TestContext) -> "RemoteAccessSnapshot":
        remote_config = tc.api("remote_access/config")
        if remote_config["enable"]:
            tc.skip("Relay tests require remote access to start disabled")
        if remote_config["users"]:
            tc.skip("Relay tests require an empty remote-access user list and keys")
        try:
            charge_config = tc.api("charge_tracker/config")
        except HTTPError as exc:
            if exc.code != 404:
                raise
            charge_config = None

        certs_state = tc.api("certs/state")
        used_ids = {cert["id"] for cert in certs_state["certs"]}
        cert_id = next((i for i in range(7, -1, -1) if i not in used_ids), None)
        if cert_id is None:
            tc.skip("No free certificate slot for the mock relay")
        return cls(remote_config, charge_config, certs_state, cert_id)

    def clear_registration(self, tc: TestContext, server) -> None:
        """Replace test credentials with empty values, then delete all keys."""
        registered = threading.Event()

        def handler(_method: str, path: str, _body: bytes) -> tuple[int, str]:
            if "/api/selfdestruct" in path:
                return 200, "{}"
            if "/api/charger/add" in path or "/api/add_with_token" in path:
                registered.set()
                return 200, json.dumps({
                    "charger_uuid": "", "charger_password": "",
                    "management_pub": management_public,
                    "user_id": "",
                })
            return 404, '{"error":"not found"}'

        management_key = X25519PrivateKey.generate()
        management_private = base64.b64encode(management_key.private_bytes_raw()).decode()
        management_public = base64.b64encode(management_key.public_key().public_bytes_raw()).decode()
        user_public = base64.b64encode(X25519PrivateKey.generate().public_key().public_bytes_raw()).decode()
        server.set_response_fn(handler)

        for user in tc.api("remote_access/config")["users"]:
            tc.api("remote_access/remove_user", {"id": user["id"]}, timeout=5)

        # Let the asynchronous removal release the HTTPS client before reuse.
        tc.wait_for(lambda: _assert_no_users(tc), timeout=10)
        time.sleep(1)
        tc.api("remote_access/register", {
            "config": {
                "enable": True, "relay_host": tc.get_local_ip(),
                "relay_port": server.port, "email": "test@example.com",
                "cert_id": self.cert_id, "mtu": self.remote_config["mtu"],
            },
            "note": "", "public_key": user_public,
            "mgmt_charger_private": management_private,
            "mgmt_charger_public": management_public,
            "mgmt_psk": base64.b64encode(os.urandom(32)).decode(),
            "keys": [], "user_uuid": "null", "auth_token": "null",
        }, timeout=10)
        if not registered.wait(timeout=15):
            raise AssertionError("Cleanup registration did not reach the mock relay")

        def registration_finished():
            config = tc.api("remote_access/config")
            assert config["uuid"] == "" and len(config["users"]) == 1

        tc.wait_for(registration_finished, timeout=15)
        tc.api("remote_access/remove_user", {"id": 1}, timeout=5)
        tc.wait_for(lambda: _assert_no_users(tc), timeout=10)

    def restore(self, tc: TestContext) -> None:
        # Removing the suites' user also removes its WireGuard keys and the
        # management key via the public API.
        current = tc.api("remote_access/config")
        for user in current["users"]:
            tc.api("remote_access/remove_user", {"id": user["id"]}, timeout=5)
        tc.api("remote_access/config_update", _config_update(self.remote_config), timeout=3)
        if self.charge_config is not None:
            tc.api("charge_tracker/config_update", self.charge_config, timeout=5)

        current = tc.api("remote_access/config")
        if current["uuid"] != "" or current["password"] != "":
            raise AssertionError("Remote-access UUID or password was not cleared")
        # Credentials are intentionally cleared; compare all other settings.
        if {k: v for k, v in current.items() if k not in ("uuid", "password")} != {
                k: v for k, v in self.remote_config.items() if k not in ("uuid", "password")}:
            raise AssertionError("Remote-access config differs from its original state")
        if self.charge_config is not None and tc.api("charge_tracker/config") != self.charge_config:
            raise AssertionError("Charge-tracker config differs from its original state")
        if tc.api("certs/state") != self.certs_state:
            raise AssertionError("Certificate list differs from its original state")


def _assert_no_users(tc: TestContext) -> None:
    assert tc.api("remote_access/config")["users"] == []
