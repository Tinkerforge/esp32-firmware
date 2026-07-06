#!/usr/bin/env -S uv run --group tests --script

# Tests for the remote access reconnection mechanism.
#
# These tests verify the reconnection state machine, including:
# - Connection state reporting via the API
# - Behavior on authentication failure (401)
# - Retry behavior after server errors
# - State reset when disabling the module
# - Timeout-based reconnection
# - Full registration with a functioning management connection
#
# A local HTTPS server is started to simulate the relay server.
# For full registration tests, a WireGuard peer runs inside a Docker
# container to simulate the relay's WireGuard endpoint. This requires
# Docker to be installed and the host kernel to support WireGuard
# (Linux 5.6+ or wireguard-dkms).

import base64
import json
import os
import shutil
import struct
import subprocess
import tempfile
import time
import threading
import uuid as uuid_mod

from cryptography.hazmat.primitives.asymmetric.x25519 import X25519PrivateKey
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

# Connection state values (from remote_access.cpp)
STATE_DISCONNECTED = 1
STATE_CONNECTED = 2

# Number of connection slots (MAX_USER_CONNECTIONS + 1 management slot)
NUM_CONNECTION_SLOTS = 6

# Cert ID used for the test CA certificate
TEST_CERT_ID = 2
TEST_CERT_NAME = "remote_access_test_cert"

# WireGuard management tunnel parameters (must match remote_access.cpp)
WG_MANAGEMENT_PORT = 51820
WG_RELAY_IP = "10.123.123.1"
WG_DEVICE_IP = "10.123.123.2"
WG_SUBNET = "24"

_original_config: dict | None = None
_server = None
_wg_peer: "WireGuardTestPeer | None" = None
_request_log: list[dict] = []
_request_log_lock = threading.Lock()


def _log_request(method: str, path: str, body: bytes) -> None:
    with _request_log_lock:
        _request_log.append({
            "method": method,
            "path": path,
            "body": body.decode("utf-8", errors="replace"),
            "time": time.monotonic(),
        })


def _get_request_log() -> list[dict]:
    with _request_log_lock:
        return list(_request_log)


def _clear_request_log() -> None:
    with _request_log_lock:
        _request_log.clear()


# ---------------------------------------------------------------------------
# WireGuard key helpers
# ---------------------------------------------------------------------------


def _generate_wg_keypair() -> tuple[str, str]:
    """Generate a WireGuard (X25519) key pair.

    Returns (private_key_b64, public_key_b64) each 44 chars base64.
    """
    priv = X25519PrivateKey.generate()
    priv_bytes = priv.private_bytes_raw()
    pub_bytes = priv.public_key().public_bytes_raw()
    return (
        base64.b64encode(priv_bytes).decode(),
        base64.b64encode(pub_bytes).decode(),
    )


def _generate_wg_psk() -> str:
    """Generate a random 32-byte pre-shared key, base64-encoded (44 chars)."""
    return base64.b64encode(os.urandom(32)).decode()


# ---------------------------------------------------------------------------
# WireGuard test peer (Docker container simulating the relay's WireGuard endpoint)
# ---------------------------------------------------------------------------

# Docker image used for the WireGuard container.
# Alpine-based, lightweight, only needs wireguard-tools and iproute2.
WG_DOCKER_IMAGE = "alpine:latest"
WG_CONTAINER_NAME = "wg-ra-test"


def _docker_available() -> bool:
    """Check if docker CLI is available and the daemon is running."""
    try:
        result = subprocess.run(
            ["docker", "info"],
            capture_output=True, text=True, timeout=10,
        )
        return result.returncode == 0
    except (FileNotFoundError, subprocess.TimeoutExpired):
        return False


class WireGuardTestPeer:
    """Runs a WireGuard peer inside a Docker container to simulate the relay.

    Requires:
    - Docker daemon running and accessible (no sudo needed for docker)
    - Host kernel with WireGuard support (Linux 5.6+ or wireguard-dkms)
    - Port 51820/UDP available on the host

    The container listens on port 51820/UDP with tunnel IP 10.123.123.1/24
    and accepts WireGuard connections from the device at 10.123.123.2.
    """

    def __init__(self) -> None:
        self.relay_private: str = ""
        self.relay_public: str = ""
        self.device_private: str = ""
        self.device_public: str = ""
        self.psk: str = ""
        self._container_running: bool = False
        self._tmpdir: str | None = None

    def generate_keys(self) -> None:
        """Generate all WireGuard keys for the management tunnel."""
        self.relay_private, self.relay_public = _generate_wg_keypair()
        self.device_private, self.device_public = _generate_wg_keypair()
        self.psk = _generate_wg_psk()

    def start(self) -> None:
        """Start a Docker container running a WireGuard peer.

        Raises RuntimeError if Docker is unavailable or setup fails.
        """
        if not _docker_available():
            raise RuntimeError(
                "Docker is not available. Ensure the Docker daemon is running "
                "and the current user has permission to use it."
            )

        # Remove any leftover container from a previous failed run
        subprocess.run(
            ["docker", "rm", "-f", WG_CONTAINER_NAME],
            capture_output=True, text=True,
        )

        self._tmpdir = tempfile.mkdtemp(prefix="wg_test_")

        # Write the WireGuard config file (wg-quick format)
        conf_content = (
            f"[Interface]\n"
            f"Address = {WG_RELAY_IP}/{WG_SUBNET}\n"
            f"ListenPort = {WG_MANAGEMENT_PORT}\n"
            f"PrivateKey = {self.relay_private}\n"
            f"\n"
            f"[Peer]\n"
            f"PublicKey = {self.device_public}\n"
            f"PresharedKey = {self.psk}\n"
            f"AllowedIPs = {WG_DEVICE_IP}/32\n"
        )
        conf_path = os.path.join(self._tmpdir, "wg0.conf")
        with open(conf_path, "w") as f:
            f.write(conf_content)

        try:
            # Start the container:
            # - NET_ADMIN capability to create WireGuard interface
            # - Map port 51820/UDP from host to container
            # - Mount the config directory
            # - Install wireguard-tools, bring up wg0, then sleep
            _ = subprocess.run(
                [
                    "docker", "run", "-d",
                    "--name", WG_CONTAINER_NAME,
                    "--cap-add=NET_ADMIN",
                    "-p", f"{WG_MANAGEMENT_PORT}:{WG_MANAGEMENT_PORT}/udp",
                    "-v", f"{self._tmpdir}:/etc/wireguard:ro",
                    WG_DOCKER_IMAGE,
                    "sh", "-c",
                    "apk add --no-cache wireguard-tools iproute2 python3 "
                    "&& wg-quick up wg0 "
                    "&& sleep infinity",
                ],
                check=True, capture_output=True, text=True,
            )
            self._container_running = True

            # Wait for WireGuard interface to come up inside the container
            self._wait_for_ready()

        except subprocess.CalledProcessError as exc:
            self.stop()
            raise RuntimeError(
                f"Failed to start WireGuard Docker container: {exc.stderr.strip()}"
            ) from exc
        except FileNotFoundError as exc:
            self.stop()
            raise RuntimeError(
                "Docker CLI not found. Install Docker to run WireGuard tests."
            ) from exc

    def _wait_for_ready(self, timeout: float = 30.0) -> None:
        """Wait until the WireGuard interface is up inside the container."""
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            try:
                result = subprocess.run(
                    ["docker", "exec", WG_CONTAINER_NAME, "wg", "show", "wg0"],
                    capture_output=True, text=True, timeout=5,
                )
                if result.returncode == 0 and "listening port" in result.stdout:
                    return
            except (subprocess.CalledProcessError, subprocess.TimeoutExpired):
                pass
            time.sleep(1)

        raise RuntimeError(
            f"WireGuard interface did not come up in container within {timeout}s"
        )

    def stop(self) -> None:
        """Stop and remove the Docker container, clean up temp files."""
        if self._container_running:
            try:
                subprocess.run(
                    ["docker", "rm", "-f", WG_CONTAINER_NAME],
                    capture_output=True, text=True, timeout=15,
                )
            except (FileNotFoundError, subprocess.TimeoutExpired):
                pass
            self._container_running = False

        if self._tmpdir:
            shutil.rmtree(self._tmpdir, ignore_errors=True)
            self._tmpdir = None

    def is_peer_connected(self) -> bool:
        """Check if the device peer has completed a WireGuard handshake."""
        if not self._container_running:
            return False
        try:
            result = subprocess.run(
                ["docker", "exec", WG_CONTAINER_NAME,
                 "wg", "show", "wg0", "latest-handshakes"],
                capture_output=True, text=True, timeout=5,
            )
            if result.returncode != 0:
                return False
            # Output format: "<public_key>\t<unix_timestamp>\n"
            for line in result.stdout.strip().splitlines():
                parts = line.split("\t")
                if len(parts) == 2 and parts[0] == self.device_public:
                    timestamp = int(parts[1])
                    return timestamp > 0
        except (subprocess.CalledProcessError, subprocess.TimeoutExpired,
                FileNotFoundError, ValueError):
            pass
        return False

    def send_management_command(self, seq_num: int, command_id: int,
                                connection_no: int, connection_uuid: bytes) -> None:
        """Send a management command packet to the device through the WireGuard tunnel.

        The packet is sent from the container's wg0 IP (10.123.123.1) to the
        device's inner socket at 10.123.123.2:12345.

        Packet layout is the same as the device's management protocol (see remote_access_packets.h):
        """
        assert len(connection_uuid) == 16, "connection_uuid must be exactly 16 bytes"
        header = struct.pack('<HHHBB', 0x1234, 24, seq_num, 0, 0)
        command = struct.pack('<ii16s', command_id, connection_no, connection_uuid)
        packet_hex = (header + command).hex()

        script = (
            "import socket; "
            "s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); "
            "s.bind(('10.123.123.1', 0)); "
            f"s.sendto(bytes.fromhex('{packet_hex}'), ('10.123.123.2', 12345)); "
            "s.close()"
        )
        subprocess.run(
            ["docker", "exec", WG_CONTAINER_NAME, "python3", "-c", script],
            check=True, capture_output=True, text=True,
        )

    def add_user_peer(self, charger_public: str, psk: str, conn_no: int) -> None:
        """Add a user-tunnel WireGuard peer to the container's wg0 interface.

        The device uses IP 10.123.{conn_no}.2 inside user tunnels, so that
        address is added to the peer's allowed-ips so WireGuard accepts the
        handshake from the right source.

        Note: wg(8) requires the preshared key to be passed as a file path
        (the `preshared-key` flag), not as an inline argument, so we write the
        PSK to a temp file in the container and reference it.
        """
        allowed_ip = f"10.123.{conn_no}.2/32"
        # Write the PSK to a temp file inside the container so `wg` can read
        # it via `preshared-key <path>`. Using a fixed name keeps cleanup simple.
        subprocess.run(
            ["docker", "exec", WG_CONTAINER_NAME,
             "sh", "-c", f"printf '%s' '{psk}' > /tmp/wg_user_psk"],
            check=True, capture_output=True, text=True,
        )
        subprocess.run(
            [
                "docker", "exec", WG_CONTAINER_NAME,
                "wg", "set", "wg0",
                "peer", charger_public,
                "preshared-key", "/tmp/wg_user_psk",
                "allowed-ips", allowed_ip,
            ],
            check=True, capture_output=True, text=True,
        )

    def is_user_peer_connected(self, charger_public: str) -> bool:
        """Return True if the given user peer has completed a WireGuard handshake."""
        if not self._container_running:
            return False
        try:
            result = subprocess.run(
                ["docker", "exec", WG_CONTAINER_NAME,
                 "wg", "show", "wg0", "latest-handshakes"],
                capture_output=True, text=True, timeout=5,
            )
            if result.returncode != 0:
                return False
            for line in result.stdout.strip().splitlines():
                parts = line.split("\t")
                if len(parts) == 2 and parts[0] == charger_public:
                    return int(parts[1]) > 0
        except (subprocess.CalledProcessError, subprocess.TimeoutExpired,
                FileNotFoundError, ValueError):
            pass
        return False


# ---------------------------------------------------------------------------
# Full registration helper
# ---------------------------------------------------------------------------


def _do_full_registration(tc: TestContext) -> WireGuardTestPeer:
    """Perform a full registration flow including WireGuard management connection.

    This simulates what the web frontend does during initial charger registration:
    1. Generate WireGuard keys for both sides (relay and device)
    2. Set up a WireGuard interface on the test machine (acting as relay)
    3. Configure the mock HTTPS server to handle relay registration + management
    4. Call the device's /remote_access/register endpoint
    5. Wait for the management connection to become STATE_CONNECTED

    Returns the WireGuardTestPeer instance (caller must call .stop() to clean up).

    Raises:
        RuntimeError: If WireGuard setup fails.
        AssertionError: If registration or connection fails within timeout.
    """
    global _wg_peer

    # 1. Generate WireGuard keys
    wg = WireGuardTestPeer()
    wg.generate_keys()

    # Also generate a user connection key set (required by the registration flow)
    user_charger_private, user_charger_public = _generate_wg_keypair()
    user_web_private, user_web_public = _generate_wg_keypair()
    user_psk = _generate_wg_psk()

    # Generate a NaCl X25519 key pair for encrypting charger name/note
    # (the device encrypts data with this; we don't need to decrypt it in the test)
    nacl_priv = X25519PrivateKey.generate()
    nacl_pub_b64 = base64.b64encode(nacl_priv.public_key().public_bytes_raw()).decode()

    # 2. Set up the WireGuard interface
    wg.start()
    _wg_peer = wg

    # 3. Configure mock HTTPS server to handle registration and management
    charger_uuid = str(uuid_mod.uuid4())
    charger_password = base64.b64encode(os.urandom(16)).decode()

    # Track registration state
    registration_received = threading.Event()

    def relay_handler(method: str, path: str, body: bytes) -> tuple[int, str]:
        _log_request(method, path, body)

        # Registration endpoint (called during /remote_access/register)
        if "/api/charger/add" in path or "/api/add_with_token" in path:
            registration_received.set()
            return (200, json.dumps({
                "charger_uuid": charger_uuid,
                "charger_password": charger_password,
                "management_pub": wg.relay_public,
                "user_id": str(uuid_mod.uuid4()),
            }))

        # Management endpoint (periodic check-in after registration)
        if "/api/management" in path:
            return (200, json.dumps({
                "configured_users": [1],
                "configured_users_emails": ["test@example.com"],
                "configured_users_uuids": [str(uuid_mod.uuid4())],
            }))

        # Fallback
        return (404, '{"error": "not found"}')

    _server.set_response_fn(relay_handler)

    # 4. Call the device's /remote_access/register endpoint
    registration_body = {
        "config": {
            "enable": True,
            "relay_host": tc.get_local_ip(),
            "relay_port": _server.port,
            "email": "test@example.com",
            "cert_id": TEST_CERT_ID,
            "mtu": 1240,
        },
        "note": "",
        "public_key": nacl_pub_b64,
        "mgmt_charger_private": wg.device_private,
        "mgmt_charger_public": wg.device_public,
        "mgmt_psk": wg.psk,
        "keys": [
            {
                "charger_private": user_charger_private,
                "charger_public": user_charger_public,
                "psk": user_psk,
                "web_private": user_web_private,
                "web_public": user_web_public,
            },
        ],
        "user_uuid": "null",
        "auth_token": "null",
    }

    tc.api("remote_access/register", registration_body, timeout=10)

    # 5. Wait for the relay to receive the registration request
    if not registration_received.wait(timeout=15):
        raise AssertionError("Device did not send registration request to mock relay")

    return wg


def _do_full_registration_with_user_tunnel(
        tc: TestContext) -> tuple["WireGuardTestPeer", str, str]:
    """Like _do_full_registration but wires up a user-level WireGuard tunnel too.

    The user tunnel reuses the relay's own WireGuard key pair as the web-side
    key, so the same container wg0 interface can respond to both the management
    handshake and any user-level handshake.  Only one user connection key set
    (connection_no=0, user_id=0) is provisioned.

    Returns:
        (wg, user_charger_public, user_psk)
        - wg                 : WireGuardTestPeer (call .stop() to clean up)
        - user_charger_public: base64 public key the device uses for user tunnel
        - user_psk           : base64 pre-shared key for the user tunnel

    The caller can set up the user peer on the container with:
        wg.add_user_peer(user_charger_public, user_psk, conn_no=0)
    and then trigger the connection with:
        wg.send_management_command(seq_num, 0, 0, conn_uuid)

    Raises:
        RuntimeError: If WireGuard setup fails.
        AssertionError: If registration fails within timeout.
    """
    global _wg_peer

    wg = WireGuardTestPeer()
    wg.generate_keys()

    # Generate keys for the charger side of the user tunnel.
    user_charger_private, user_charger_public = _generate_wg_keypair()
    user_psk = _generate_wg_psk()
    # The web (relay) side uses the same key as the management tunnel so the
    # existing wg0 container instance can complete the user-tunnel handshake.
    user_web_private = wg.relay_private
    user_web_public = wg.relay_public

    nacl_priv = X25519PrivateKey.generate()
    nacl_pub_b64 = base64.b64encode(nacl_priv.public_key().public_bytes_raw()).decode()

    wg.start()
    _wg_peer = wg

    charger_uuid = str(uuid_mod.uuid4())
    charger_password = base64.b64encode(os.urandom(16)).decode()
    registration_received = threading.Event()

    def relay_handler(method: str, path: str, body: bytes) -> tuple[int, str]:
        _log_request(method, path, body)
        if "/api/charger/add" in path or "/api/add_with_token" in path:
            registration_received.set()
            return (200, json.dumps({
                "charger_uuid": charger_uuid,
                "charger_password": charger_password,
                "management_pub": wg.relay_public,
                "user_id": str(uuid_mod.uuid4()),
            }))
        if "/api/management" in path:
            return (200, json.dumps({
                "configured_users": [1],
                "configured_users_emails": ["test@example.com"],
                "configured_users_uuids": [str(uuid_mod.uuid4())],
            }))
        return (404, '{"error": "not found"}')

    _server.set_response_fn(relay_handler)

    registration_body = {
        "config": {
            "enable": True,
            "relay_host": tc.get_local_ip(),
            "relay_port": _server.port,
            "email": "test@example.com",
            "cert_id": TEST_CERT_ID,
            "mtu": 1240,
        },
        "note": "",
        "public_key": nacl_pub_b64,
        "mgmt_charger_private": wg.device_private,
        "mgmt_charger_public": wg.device_public,
        "mgmt_psk": wg.psk,
        "keys": [
            {
                "charger_private": user_charger_private,
                "charger_public": user_charger_public,
                "psk": user_psk,
                "web_private": user_web_private,
                "web_public": user_web_public,
            },
        ],
        "user_uuid": "null",
        "auth_token": "null",
    }

    tc.api("remote_access/register", registration_body, timeout=10)

    if not registration_received.wait(timeout=15):
        raise AssertionError("Device did not send registration request to mock relay")

    return wg, user_charger_public, user_psk


def _wait_for_management_connected(tc: TestContext, *, timeout: float = 60.0) -> None:
    """Wait until the management connection slot (index 0) reports STATE_CONNECTED."""
    def _check():
        state = _get_connection_state(tc)
        if state[0]["state"] != STATE_CONNECTED:
            raise AssertionError(
                f"Management slot state={state[0]['state']}, expected {STATE_CONNECTED} (connected)"
            )

    tc.wait_for(_check, timeout=timeout)


def _make_management_response_ok() -> str:
    """Return a minimal valid management response for the V2 API."""
    return json.dumps({
        "configured_users": [],
        "configured_users_emails": [],
        "configured_users_uuids": [],
    })


def _make_response_fn(status: int = 200, body: str | None = None):
    """Create a response function that logs requests and returns the given status/body."""
    if body is None:
        body = _make_management_response_ok()

    def handler(method: str, path: str, req_body: bytes) -> tuple[int, str]:
        _log_request(method, path, req_body)
        return (status, body)

    return handler


def _make_config_update(tc: TestContext, enable: bool = True, **overrides) -> dict:
    """Build a registration config update payload."""
    cfg = {
        "enable": enable,
        "relay_host": tc.get_local_ip(),
        "relay_port": _server.port if _server else 443,
        "email": "test@example.com",
        "cert_id": TEST_CERT_ID,
        "mtu": 1240,
    }
    cfg.update(overrides)
    return cfg


def _get_connection_state(tc: TestContext) -> list[dict]:
    """Fetch the remote_access/state and return as list of dicts."""
    return tc.api("remote_access/state")


def _assert_all_disconnected(tc: TestContext) -> None:
    """Assert all connection slots report disconnected."""
    state = _get_connection_state(tc)
    for i, slot in enumerate(state):
        if slot["state"] != STATE_DISCONNECTED:
            raise AssertionError(
                f"Expected slot {i} state={STATE_DISCONNECTED} (disconnected), got {slot['state']}"
            )


def _wait_for_management_request(tc: TestContext, *, timeout: float = 45.0) -> None:
    """Wait until the device sends at least one request to our mock relay server."""
    def _check():
        log = _get_request_log()
        management_reqs = [r for r in log if "/api/management" in r["path"]]
        if not management_reqs:
            raise AssertionError("No management request received yet")

    tc.wait_for(_check, timeout=timeout)


def suite_setup(tc: TestContext) -> None:
    global _original_config, _server

    # Save the original config to restore later
    _original_config = tc.api("remote_access/config")

    # Create the HTTPS test server (generates self-signed cert and uploads it)
    _server = tc.create_test_https_server(TEST_CERT_ID, TEST_CERT_NAME)

    # Default: return a valid management response
    _server.set_response_fn(_make_response_fn(200))


def suite_teardown(tc: TestContext) -> None:
    global _server, _wg_peer

    # Disable remote access to stop any ongoing connection attempts
    try:
        tc.api("remote_access/config_update", _make_config_update(tc, enable=False), timeout=3)
    except (TimeoutError, OSError):
        pass

    # Give the device time to process the disable
    time.sleep(2)

    # Clean up the WireGuard interface
    if _wg_peer:
        _wg_peer.stop()
        _wg_peer = None

    # Clean up the cert
    try:
        tc.api("certs/remove", {"id": TEST_CERT_ID})
    except (TimeoutError, OSError):
        pass

    if _server:
        _server.stop()
        _server = None


def setup(tc: TestContext) -> None:
    """Reset state before each test."""
    _clear_request_log()

    # Ensure remote access is disabled before each test
    tc.api("remote_access/config_update", _make_config_update(tc, enable=False), timeout=3)
    time.sleep(1)


def teardown(tc: TestContext) -> None:
    """Disable remote access after each test to leave clean state."""
    global _wg_peer

    try:
        tc.api("remote_access/config_update", _make_config_update(tc, enable=False), timeout=3)
    except (TimeoutError, OSError):
        pass
    time.sleep(1)

    # Tear down per-test WireGuard interfaces
    if _wg_peer:
        _wg_peer.stop()
        _wg_peer = None


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------


def test_state_structure(tc: TestContext) -> None:
    """Verify the remote_access/state API returns correct structure."""
    state = _get_connection_state(tc)

    # Should have MAX_USER_CONNECTIONS + 1 = 6 slots
    tc.assert_eq(NUM_CONNECTION_SLOTS, len(state))

    # Each slot should have the expected fields
    for slot in state:
        tc.assert_("state" in slot)
        tc.assert_("user" in slot)
        tc.assert_("connection" in slot)
        tc.assert_("last_state_change" in slot)


def test_all_disconnected_when_disabled(tc: TestContext) -> None:
    """When remote access is disabled, all slots should be disconnected."""
    _assert_all_disconnected(tc)


def test_enable_triggers_management_request(tc: TestContext) -> None:
    """Enabling remote access should trigger a management request to the relay server."""
    tc.set_test_timeout(60)

    _server.set_response_fn(_make_response_fn(200))
    _clear_request_log()

    tc.api("remote_access/config_update", _make_config_update(tc, enable=True), timeout=3)

    # The device should send a management request within ~30s (periodic interval + jitter)
    _wait_for_management_request(tc, timeout=45)

    log = _get_request_log()
    management_reqs = [r for r in log if "/api/management" in r["path"]]
    tc.assert_ge(1, len(management_reqs))

    # Verify it was a PUT request
    tc.assert_eq("PUT", management_reqs[0]["method"])

    # Verify the body contains expected fields
    body = json.loads(management_reqs[0]["body"])
    tc.assert_("data" in body)


def test_auth_failure_disables_module(tc: TestContext) -> None:
    """A 401 response from the relay should disable remote access."""
    tc.set_test_timeout(60)

    # Configure mock to return 401 (authentication failure)
    _server.set_response_fn(_make_response_fn(401, '{"error":"unauthorized"}'))
    _clear_request_log()

    tc.api("remote_access/config_update", _make_config_update(tc, enable=True), timeout=3)

    # Wait for the management request to happen
    _wait_for_management_request(tc, timeout=45)

    # After auth failure, the module should disable itself
    def _check_disabled():
        cfg = tc.api("remote_access/config")
        if cfg["enable"] is True:
            raise AssertionError(f"Expected enable=False after 401, got enable={cfg['enable']}")

    tc.wait_for(_check_disabled, timeout=10)

    # All connections should remain disconnected
    _assert_all_disconnected(tc)


def test_server_error_retries(tc: TestContext) -> None:
    """After a server error (500), the module should retry on the next periodic tick."""
    tc.set_test_timeout(120)

    call_count = {"n": 0}
    call_count_lock = threading.Lock()

    def counting_handler(method: str, path: str, body: bytes) -> tuple[int, str]:
        _log_request(method, path, body)
        with call_count_lock:
            call_count["n"] += 1
            n = call_count["n"]

        if n <= 1:
            # First request: return server error
            return (500, '{"error":"internal server error"}')
        else:
            # Subsequent requests: return success
            return (200, _make_management_response_ok())

    _server.set_response_fn(counting_handler)
    _clear_request_log()

    tc.api("remote_access/config_update", _make_config_update(tc, enable=True), timeout=3)

    # Wait for at least 2 management requests (first fails, second succeeds on retry)
    def _check_retried():
        log = _get_request_log()
        management_reqs = [r for r in log if "/api/management" in r["path"]]
        if len(management_reqs) < 2:
            raise AssertionError(
                f"Expected at least 2 management requests (retry), got {len(management_reqs)}"
            )

    # The retry happens after ~30s, so total wait needs to be >60s
    tc.wait_for(_check_retried, timeout=90, poll_delay=2.0)

    # Verify the module is still enabled (did not give up)
    cfg = tc.api("remote_access/config")
    tc.assert_true(cfg["enable"])


def test_disable_resets_state(tc: TestContext) -> None:
    """Disabling remote access should reset all connection states to disconnected."""
    tc.set_test_timeout(60)

    _server.set_response_fn(_make_response_fn(200))
    _clear_request_log()

    # Enable remote access
    tc.api("remote_access/config_update", _make_config_update(tc, enable=True), timeout=3)

    # Wait for at least one management request
    _wait_for_management_request(tc, timeout=45)

    # Now disable
    tc.api("remote_access/config_update", _make_config_update(tc, enable=False), timeout=3)

    # All slots should be disconnected
    def _check_all_disconnected():
        _assert_all_disconnected(tc)

    tc.wait_for(_check_all_disconnected, timeout=5)


def test_enable_disable_enable_cycle(tc: TestContext) -> None:
    """Cycling enable -> disable -> enable should restart the connection flow cleanly."""
    tc.set_test_timeout(120)

    _server.set_response_fn(_make_response_fn(200))
    _clear_request_log()

    # First enable
    tc.api("remote_access/config_update", _make_config_update(tc, enable=True), timeout=3)
    _wait_for_management_request(tc, timeout=45)

    # Disable
    tc.api("remote_access/config_update", _make_config_update(tc, enable=False), timeout=3)
    time.sleep(2)
    _assert_all_disconnected(tc)

    # Clear log and re-enable
    _clear_request_log()
    tc.api("remote_access/config_update", _make_config_update(tc, enable=True), timeout=3)

    # Should make a new management request
    _wait_for_management_request(tc, timeout=45)

    log = _get_request_log()
    management_reqs = [r for r in log if "/api/management" in r["path"]]
    tc.assert_ge(1, len(management_reqs))


def test_states_initially_disconnected(tc: TestContext) -> None:
    """The management connection slot (index 0) should start as disconnected."""
    state = _get_connection_state(tc)
    tc.assert_eq(STATE_DISCONNECTED, state[0]["state"])
    tc.assert_eq(255, state[0]["user"])
    tc.assert_eq(255, state[0]["connection"])
    
    for i in range(1, NUM_CONNECTION_SLOTS):
        tc.assert_eq(STATE_DISCONNECTED, state[i]["state"])
        tc.assert_eq(255, state[i]["user"])
        tc.assert_eq(255, state[i]["connection"])


def test_config_preserved_after_server_error(tc: TestContext) -> None:
    """Server errors should not modify the stored config (uuid, users, etc.)."""
    tc.set_test_timeout(60)

    # Get config before enabling
    config_before = tc.api("remote_access/config")

    # Return 500 from server
    _server.set_response_fn(_make_response_fn(500, '{"error":"server error"}'))
    _clear_request_log()

    tc.api("remote_access/config_update", _make_config_update(tc, enable=True), timeout=3)
    _wait_for_management_request(tc, timeout=45)

    # Wait a bit for error processing
    time.sleep(3)

    # Config should be unchanged (except enable which was set to True)
    config_after = tc.api("remote_access/config")
    tc.assert_eq(config_before.get("uuid", ""), config_after.get("uuid", ""))
    tc.assert_eq(config_before.get("users", []), config_after.get("users", []))
    tc.assert_eq(True, config_after.get("enable", []))


def test_reconnection(tc: TestContext) -> None:
    """After management timeout (60s), the module should re-resolve and reconnect."""
    tc.set_test_timeout(400)

    # Perform a full registration so we have a working management connection
    try:
        wg = _do_full_registration(tc)
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")
        return

    # Wait for management connection to become connected
    _wait_for_management_connected(tc, timeout=60)

    # Now tear down the WireGuard interface to simulate connection loss
    wg.stop()
    _clear_request_log()

    # The device should detect the timeout (after ~60s) and attempt to re-resolve
    def _check_reconnect_attempt():
        log = _get_request_log()
        management_reqs = [r for r in log if "/api/management" in r["path"]]
        if not management_reqs:
            raise AssertionError("No management re-resolve request after timeout")

    tc.wait_for(_check_reconnect_attempt, timeout=300, poll_delay=5.0)

    # Verify the module is still enabled (it should keep retrying)
    cfg = tc.api("remote_access/config")
    tc.assert_true(cfg["enable"])

def test_fallback_reconnection(tc: TestContext) -> None:
    """After management timeout (60s), the module should re-resolve and reconnect."""
    tc.set_test_timeout(400)

    # Perform a full registration so we have a working management connection
    try:
        wg = _do_full_registration(tc)
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")
        return

    # Now tear down the WireGuard interface to simulate connection loss
    wg.stop()

    _clear_request_log()

    # The device should detect the timeout (after ~60s) and attempt to re-resolve
    def _check_reconnect_attempt():
        log = _get_request_log()
        management_reqs = [r for r in log if "/api/management" in r["path"]]
        if not management_reqs:
            raise AssertionError("No management re-resolve request after timeout")

    tc.wait_for(_check_reconnect_attempt, timeout=15, poll_delay=5.0)

    # It should retry after not being able to connect the management-connection after successful http-request
    _clear_request_log()

    # The device should detect the timeout (after ~60s) and attempt to re-resolve
    def _check_reconnect_attempt():
        log = _get_request_log()
        management_reqs = [r for r in log if "/api/management" in r["path"]]
        if not management_reqs:
            raise AssertionError("No management re-resolve request after timeout")

    tc.wait_for(_check_reconnect_attempt, timeout=200, poll_delay=5.0)

    # Verify the module is still enabled (it should keep retrying)
    cfg = tc.api("remote_access/config")
    tc.assert_true(cfg["enable"])

def test_full_registration_management_connected(tc: TestContext) -> None:
    """Full registration flow should result in a functioning management connection."""
    tc.set_test_timeout(90)

    try:
        _do_full_registration(tc)
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")
        return

    # Wait for the management connection to report STATE_CONNECTED
    _wait_for_management_connected(tc, timeout=60)

    # Verify the management slot shows connected
    state = _get_connection_state(tc)
    tc.assert_eq(STATE_CONNECTED, state[0]["state"])

    # Verify config was updated with the charger UUID
    cfg = tc.api("remote_access/config")
    tc.assert_true(cfg["enable"])
    tc.assert_ne("", cfg.get("uuid", ""))


def test_full_registration_wg_handshake(tc: TestContext) -> None:
    """After full registration, the WireGuard handshake should complete on the relay side."""
    tc.set_test_timeout(90)

    try:
        wg = _do_full_registration(tc)
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")
        return

    # Wait for the WireGuard handshake to complete on our side
    def _check_handshake():
        if not wg.is_peer_connected():
            raise AssertionError("WireGuard handshake not completed on relay side")

    tc.wait_for(_check_handshake, timeout=60, poll_delay=2.0)

    # Also verify via the device API
    _wait_for_management_connected(tc, timeout=10)


def test_full_registration_reconnect_after_disable_enable(tc: TestContext) -> None:
    """After full registration, disable/enable should reconnect management."""
    tc.set_test_timeout(120)

    try:
        _do_full_registration(tc)
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")
        return

    # Wait for management connection
    _wait_for_management_connected(tc, timeout=60)

    # Disable remote access
    tc.api("remote_access/config_update", _make_config_update(tc, enable=False), timeout=3)
    time.sleep(2)

    # Verify disconnected
    _assert_all_disconnected(tc)

    # Re-enable (keys are still stored on the device)
    _clear_request_log()
    tc.api("remote_access/config_update", _make_config_update(tc, enable=True), timeout=3)

    # Should re-establish management connection
    _wait_for_management_connected(tc, timeout=60)

    state = _get_connection_state(tc)
    tc.assert_eq(STATE_CONNECTED, state[0]["state"])


# ---------------------------------------------------------------------------
# User-level WireGuard tunnel tests
# ---------------------------------------------------------------------------

# Helpers
_CONN_COMMAND_CONNECT = 0
_CONN_COMMAND_DISCONNECT = 1


def _wait_for_any_user_slot_active(tc: TestContext, *, timeout: float = 15.0) -> None:
    """Wait until at least one user slot (index 1-5) has user != 255."""
    def _check():
        state = _get_connection_state(tc)
        active = [s for s in state[1:] if s["user"] != 255]
        if not active:
            raise AssertionError(f"No active user slot yet, state={state[1:]}")
    tc.wait_for(_check, timeout=timeout)


def test_user_connect_command_updates_state(tc: TestContext) -> None:
    """A Connect management command should populate a user slot with the right IDs."""
    tc.set_test_timeout(90)

    try:
        wg, user_charger_public, user_psk = _do_full_registration_with_user_tunnel(tc)
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")
        return

    _wait_for_management_connected(tc, timeout=60)

    conn_uuid = uuid_mod.uuid4()
    wg.send_management_command(
        seq_num=1, command_id=_CONN_COMMAND_CONNECT,
        connection_no=0, connection_uuid=conn_uuid.bytes,
    )

    # connect_remote_access() calls update_connection_state(conn_idx, user_id=0,
    # conn_id=0, Disconnected) after loading the key.  Wait for that.
    _wait_for_any_user_slot_active(tc, timeout=15)

    state = _get_connection_state(tc)
    active = [s for s in state[1:] if s["user"] != 255]
    tc.assert_eq(1, len(active))
    tc.assert_eq(0, active[0]["user"])
    tc.assert_eq(0, active[0]["connection"])


def test_user_full_wg_handshake(tc: TestContext) -> None:
    """After a Connect command the user WireGuard handshake should complete and
    the slot should transition to STATE_CONNECTED."""
    tc.set_test_timeout(120)

    try:
        wg, user_charger_public, user_psk = _do_full_registration_with_user_tunnel(tc)
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")
        return

    _wait_for_management_connected(tc, timeout=60)

    # Add the user peer BEFORE sending the Connect command so the container
    # can respond to the WireGuard handshake initiation immediately.
    wg.add_user_peer(user_charger_public, user_psk, conn_no=0)

    conn_uuid = os.urandom(16)
    wg.send_management_command(
        seq_num=1, command_id=_CONN_COMMAND_CONNECT,
        connection_no=0, connection_uuid=conn_uuid,
    )

    # Check handshake from the relay side.
    def _check_handshake():
        if not wg.is_user_peer_connected(user_charger_public):
            raise AssertionError("User WireGuard handshake not completed on relay side")

    tc.wait_for(_check_handshake, timeout=30, poll_delay=2.0)

    # Check that the device API also reflects Connected.
    def _check_slot_connected():
        state = _get_connection_state(tc)
        connected = [s for s in state[1:] if s["state"] == STATE_CONNECTED]
        if not connected:
            raise AssertionError(f"No user slot in Connected state, state={state[1:]}")

    tc.wait_for(_check_slot_connected, timeout=15)

    state = _get_connection_state(tc)
    connected = [s for s in state[1:] if s["state"] == STATE_CONNECTED]
    tc.assert_eq(1, len(connected))
    tc.assert_eq(0, connected[0]["user"])
    tc.assert_eq(0, connected[0]["connection"])


def test_user_disconnect_command_clears_state(tc: TestContext) -> None:
    """A Disconnect management command should tear down the tunnel and reset the slot."""
    tc.set_test_timeout(90)

    try:
        wg, user_charger_public, user_psk = _do_full_registration_with_user_tunnel(tc)
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")
        return

    _wait_for_management_connected(tc, timeout=60)

    conn_uuid = os.urandom(16)

    # Connect first.
    wg.send_management_command(
        seq_num=1, command_id=_CONN_COMMAND_CONNECT,
        connection_no=0, connection_uuid=conn_uuid,
    )
    _wait_for_any_user_slot_active(tc, timeout=15)

    # Now disconnect.
    wg.send_management_command(
        seq_num=2, command_id=_CONN_COMMAND_DISCONNECT,
        connection_no=0, connection_uuid=conn_uuid,
    )

    # All user slots should return to Disconnected with user=255 and connection=255.
    def _check_all_user_slots_cleared():
        state = _get_connection_state(tc)
        for i, slot in enumerate(state[1:], start=1):
            if slot["user"] != 255 or slot["connection"] != 255:
                raise AssertionError(
                    f"Slot {i} not cleared after Disconnect: {slot}"
                )
            if slot["state"] != STATE_DISCONNECTED:
                raise AssertionError(
                    f"Slot {i} not in Disconnected state after Disconnect: {slot}"
                )

    tc.wait_for(_check_all_user_slots_cleared, timeout=10)


def test_duplicate_seq_num_ignored(tc: TestContext) -> None:
    """A command with the same sequence number as the last processed command
    should be silently dropped."""
    tc.set_test_timeout(90)

    try:
        wg, _, _ = _do_full_registration_with_user_tunnel(tc)
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")
        return

    _wait_for_management_connected(tc, timeout=60)

    conn_uuid = os.urandom(16)

    # First Connect for connection_no=0 (seq=1).
    wg.send_management_command(
        seq_num=1, command_id=_CONN_COMMAND_CONNECT,
        connection_no=0, connection_uuid=conn_uuid,
    )
    _wait_for_any_user_slot_active(tc, timeout=15)

    # Second command with the SAME seq=1 but for connection_no=1 – must be ignored.
    conn_uuid_2 = os.urandom(16)
    wg.send_management_command(
        seq_num=1, command_id=_CONN_COMMAND_CONNECT,
        connection_no=1, connection_uuid=conn_uuid_2,
    )

    # Give the device time to process (or ignore) the duplicate.
    time.sleep(2)

    # Only connection_no=0 should be active; connection_no=1 must not have opened.
    state = _get_connection_state(tc)
    active = [s for s in state[1:] if s["user"] != 255]
    tc.assert_eq(1, len(active))
    tc.assert_eq(0, active[0]["connection"])


def test_out_of_order_command_ignored(tc: TestContext) -> None:
    """A command whose sequence number is not greater than the last seen value
    should be ignored, leaving the existing connection intact."""
    tc.set_test_timeout(90)

    try:
        wg, _, _ = _do_full_registration_with_user_tunnel(tc)
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")
        return

    _wait_for_management_connected(tc, timeout=60)

    conn_uuid = os.urandom(16)

    # Connect with seq=2; this sets in_seq_number=2 on the device.
    wg.send_management_command(
        seq_num=2, command_id=_CONN_COMMAND_CONNECT,
        connection_no=0, connection_uuid=conn_uuid,
    )
    _wait_for_any_user_slot_active(tc, timeout=15)

    # Send Disconnect with seq=1 (lower than in_seq_number=2) – must be ignored.
    wg.send_management_command(
        seq_num=1, command_id=_CONN_COMMAND_DISCONNECT,
        connection_no=0, connection_uuid=conn_uuid,
    )

    # Give the device time to process (or ignore) the out-of-order packet.
    time.sleep(2)

    # The connection for connection_no=0 should still be active.
    state = _get_connection_state(tc)
    active = [s for s in state[1:] if s["user"] != 255]
    tc.assert_ge(1, len(active))


def test_disable_closes_user_connections(tc: TestContext) -> None:
    """Disabling remote access while a user tunnel is open should tear it down
    and reset all user slots to Disconnected."""
    tc.set_test_timeout(90)

    try:
        wg, user_charger_public, user_psk = _do_full_registration_with_user_tunnel(tc)
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")
        return

    _wait_for_management_connected(tc, timeout=60)

    conn_uuid = os.urandom(16)
    wg.send_management_command(
        seq_num=1, command_id=_CONN_COMMAND_CONNECT,
        connection_no=0, connection_uuid=conn_uuid,
    )
    _wait_for_any_user_slot_active(tc, timeout=15)

    # Disable remote access.  apply_config() calls close_all_remote_connections().
    tc.api("remote_access/config_update", _make_config_update(tc, enable=False), timeout=3)

    def _check_all_disconnected():
        _assert_all_disconnected(tc)

    tc.wait_for(_check_all_disconnected, timeout=10)


if __name__ == "__main__":
    run_testsuite(locals())
