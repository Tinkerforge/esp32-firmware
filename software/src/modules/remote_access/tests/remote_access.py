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
# - Service-token (support account) registration and removal
#
# A local HTTPS server is started to simulate the relay server for most
# tests. For full registration tests, a WireGuard peer runs inside a Docker
# container to simulate the relay's WireGuard endpoint. This requires
# Docker to be installed and the host kernel to support WireGuard
# (Linux 5.6+ or wireguard-dkms).
#
# Service-token tests point the device at the production relay
# (my.warp-charger.com) instead of the local mock server. They require
# signed firmware (signature_sodium_public_key_length != 0) and a relay
# that is willing to issue service tokens for the test device.

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
from urllib.error import HTTPError
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

    def set_management_peer(self, charger_public: str, psk: str) -> None:
        """Replace the container's management-tunnel peer with the real device keys.

        ``WireGuardTestPeer.generate_keys`` allocates a placeholder charger keypair
        that the container is initially configured with. The device, however,
        generates its own WireGuard keys on-board during /remote_access/register
        (see ``generate_wg_key`` in remote_access.cpp) and uses those for the
        actual handshake. This method swaps the placeholder peer out for one
        configured with the device-generated public key and PSK so the
        handshake can succeed.

        ``wg(8)`` requires the preshared key to be passed as a file path
        (the ``preshared-key`` flag), not as an inline argument, so we write
        the PSK to a temp file inside the container and reference it.
        """
        subprocess.run(
            ["docker", "exec", WG_CONTAINER_NAME,
             "sh", "-c", f"printf '%s' '{psk}' > /tmp/wg_mgmt_psk"],
            check=True, capture_output=True, text=True,
        )
        # Remove the placeholder peer that was added during start().
        # Capture the placeholder public key first; self.device_public is
        # overwritten with the real device key below.
        placeholder_peer = self.device_public
        subprocess.run(
            ["docker", "exec", WG_CONTAINER_NAME,
             "wg", "set", "wg0", "peer", placeholder_peer, "remove"],
            check=True, capture_output=True, text=True,
        )
        # Add the real device peer with its on-board generated public key + PSK.
        subprocess.run(
            [
                "docker", "exec", WG_CONTAINER_NAME,
                "wg", "set", "wg0",
                "peer", charger_public,
                "preshared-key", "/tmp/wg_mgmt_psk",
                "allowed-ips", f"{WG_DEVICE_IP}/32",
            ],
            check=True, capture_output=True, text=True,
        )
        # Keep self.device_public in sync with what the container is now
        # actually configured for so is_peer_connected() (and any future
        # caller that asks "who is the active peer?") sees the real key.
        self.device_public = charger_public

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
    1. Generate the relay-side WireGuard keys (the device generates its own
       on-board keys, so the test only needs keys for the relay side).
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

    # 1. Generate WireGuard keys (relay side only; the device generates its own
    #    private/public/PSK on-board during the /remote_access/register call).
    wg = WireGuardTestPeer()
    wg.generate_keys()

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

    # 4. Call the device's /remote_access/register endpoint. WireGuard keys are
    #    no longer supplied by the client: the device generates them itself
    #    and ships the (sealed) relay-side private keys plus the device-side
    #    public keys in the registration request to the relay.
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
        "user_uuid": "null",
        "auth_token": "null",
    }

    tc.api("remote_access/register", registration_body, timeout=10)

    # 5. Wait for the relay to receive the registration request
    if not registration_received.wait(timeout=15):
        raise AssertionError("Device did not send registration request to mock relay")

    # 6. Swap the container's management peer for the device-generated keys.
    # The device generates its own WireGuard keypair on-board during
    # /remote_access/register (see generate_wg_key in remote_access.cpp) and
    # sends charger.charger_pub + charger.psk in plain text inside the
    # /api/charger/add body. Without this step the container is still
    # configured with the test's placeholder keys and the WireGuard handshake
    # never completes.
    for entry in _get_request_log():
        if "/api/charger/add" in entry["path"] or "/api/add_with_token" in entry["path"]:
            try:
                body = json.loads(entry["body"])
                charger_pub = body["charger"]["charger_pub"]
                psk = body["charger"]["psk"]
            except (json.JSONDecodeError, KeyError, TypeError) as exc:
                raise AssertionError(
                    f"Could not extract management tunnel keys from registration body: {exc}"
                ) from exc
            wg.set_management_peer(charger_pub, psk)
            break
    else:
        raise AssertionError("No /api/charger/add request found in request log")

    return wg


# NOTE: User-level WireGuard tunnel tests were removed. After WireGuard key
# generation moved to the device, the test helper can no longer recover the
# user-tunnel PSK (it is sealed with the relay's NaCl seal key in transit).
# To re-add user-tunnel tests, read the keys from the device's persisted state
# via /remote_access/state instead of from the registration request body.


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
    """Build a config update payload for the local mock relay."""
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


def _point_at_local_relay(tc: TestContext, *, enable: bool = False) -> None:
    """Configure the device to use the local HTTPS mock relay."""
    tc.api(
        "remote_access/config_update",
        _make_config_update(tc, enable=enable),
        timeout=3,
    )


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


# ---------------------------------------------------------------------------
# Service-token helpers
# ---------------------------------------------------------------------------

# Production relay used for service-token tests. The device must be configured
# to point at this host before running the tests, otherwise the device cannot
# fetch /api/auth/service_token and registration will fail.
PRODUCTION_RELAY_HOST = "my.warp-charger.com"
PRODUCTION_RELAY_PORT = 443

# cert_id = -1 selects the system default CA bundle, which trusts the public CA
# that signs my.warp-charger.com's certificate.
PRODUCTION_CERT_ID = -1

# Registration state values (from Registration State.uint8.enum, 0-indexed)
REG_STATE_NONE = 0
REG_STATE_IN_PROGRESS = 1
REG_STATE_SUCCESS = 2
REG_STATE_ERROR = 3


def _point_at_production_relay(tc: TestContext, *, enable: bool = False) -> None:
    """Reconfigure the device so it talks to the production relay (my.warp-charger.com).

    The device keeps whatever charger UUID/password/MTU it already had. The
    relay_host/relay_port/cert_id are overwritten so any subsequent relay
    traffic goes to the production relay over the system-trusted CA bundle.

    enable is False by default: most service-token tests trigger
    service_token_register themselves, which flips enable=true via the
    parse_service_token() path; starting enabled here would otherwise kick
    off the periodic management polling against the production relay.
    """
    tc.api(
        "remote_access/config_update",
        {
            "enable": enable,
            "relay_host": PRODUCTION_RELAY_HOST,
            "relay_port": PRODUCTION_RELAY_PORT,
            "cert_id": PRODUCTION_CERT_ID,
            "email": "test@example.com",
            "mtu": 1240,
        },
        timeout=3,
    )


def _get_registration_state(tc: TestContext) -> dict:
    """Fetch the remote_access/registration_state."""
    return tc.api("remote_access/registration_state")


def _get_service_token_user_uuid(tc: TestContext) -> str:
    """Return the current service_token_user_uuid from the device config."""
    return tc.api("remote_access/config").get("service_token_user_uuid", "") or ""


def _get_service_token_timestamp_minutes(tc: TestContext) -> int:
    """Return the current service_token_timestamp_minutes from the device config.

    This is the wall-clock minute (epoch / 60) when the service-token
    registration was last refreshed. The device uses it together with the
    24 h deadline in setup() to schedule the automatic removal task on the
    next boot.
    """
    return tc.api("remote_access/config").get("service_token_timestamp_minutes", 0) or 0


def _service_token_feature_available(tc: TestContext) -> bool:
    """Return True if the /remote_access/service_token_register endpoint is registered.

    The endpoint is only compiled in when the firmware has a sodium public key
    embedded (signature_sodium_public_key_length != 0). On signed firmware that
    sodium key is also baked into firmware_update/state.publisher (via
    signature_publisher); on unsigned firmware the field is the empty string.

    We probe via firmware_update/state instead of calling
    /remote_access/service_token_register directly: that endpoint kicks off an
    HTTPS GET to my.warp-charger.com in fetch_service_token() and the device
    has only one AsyncHTTPSClient, so probing and then actually registering
    races and the second call hits "AsyncHTTPSClient busy".
    """
    try:
        state = tc.api("firmware_update/state")
    except Exception:
        # If we cannot reach firmware_update/state at all the device is in
        # such a bad state that no remote-access test could pass; skip.
        return False
    return bool(state.get("publisher"))


def _register_service_token(tc: TestContext, *, timeout: float = 30.0) -> None:
    """Trigger /remote_access/service_token_register and wait for success."""
    tc.http_request("PUT", "/remote_access/service_token_register", timeout=timeout)

    def _check_success():
        state = _get_registration_state(tc)
        if state["state"] != REG_STATE_SUCCESS:
            raise AssertionError(
                f"Expected registration state=Success, got state={state['state']} message={state.get('message', '')!r}"
            )

    tc.wait_for(_check_success, timeout=timeout)


def _wait_for_remote_access_idle(tc: TestContext, *, timeout: float = 15.0) -> None:
    """Wait until the firmware is ready to accept another remote_access HTTPS request.

    The module shares a single AsyncHTTPSClient between all requests
    (self-destruct, /api/auth/service_token, register, add_user, etc.). Tests
    that issue requests back-to-back have to make sure the previous request
    has fully released the client, otherwise the new request fails with
    ESP_ERR_NOT_SUPPORTED / "AsyncHTTPSClient busy".

    The registration state is set to InProgress at request start and only
    updated when the next-stage callback runs, so a non-InProgress value is a
    reliable signal that the shared client is idle.
    """
    def _check():
        state = _get_registration_state(tc)
        if state.get("state") == REG_STATE_IN_PROGRESS:
            raise AssertionError(
                f"remote_access still busy: registration_state={state!r}"
            )

    tc.wait_for(_check, timeout=timeout)


def _reset_registration_state(tc: TestContext, *, timeout: float = 5.0) -> None:
    """Clear the registration_state enum back to None."""
    try:
        tc.api("remote_access/reset_registration_state", timeout=timeout)
    except (TimeoutError, OSError):
        pass


def _clear_service_token_registration(tc: TestContext) -> None:
    """Reset the device's remote_access state so a fresh service-token
    registration can run.

    The service-token flow in firmware (parse_service_token) takes a
    different relay path depending on whether users[] is empty:

    * empty users[]   -> register_with_relay() POSTs /api/add_with_token
                         using the auth_token from the signed service_token
                         (the device's own credentials are not used).
    * non-empty users -> allow_user_at_relay() PUTs /api/allow_user with
                         the device's uuid+password. The production relay
                         only accepts this when the device is registered with
                         it; if the device was previously registered against
                         the local mock relay (as happens when service-token
                         tests run after a full_registration test in the same
                         suite) the production relay returns 401, which the
                         firmware surfaces as "ESP_ERR_NOT_SUPPORTED
                         (error code 10)".

    To get the device into the empty-users[] state, remove every user. The
    last removal fires /api/selfdestruct. Point the device at the local mock
    relay first so the self-destruct is observable via the test server's
    request log, and wait for the request to be received before continuing.
    """
    # Point the device at the local mock relay so the eventual
    # /api/selfdestruct is observable (and so any retry cannot reach the
    # production relay).
    _point_at_local_relay(tc, enable=False)
    # Give the firmware time to apply the new config and stop talking to the
    # previous relay.
    time.sleep(1)
    _wait_for_remote_access_idle(tc, timeout=20)

    config = tc.api("remote_access/config")
    users = list(config.get("users", []))
    if not users:
        _reset_registration_state(tc)
        return

    for user in users:
        user_id = user.get("id")
        if user_id is None or user_id == 255:
            continue
        _clear_request_log()
        try:
            tc.api("remote_access/remove_user", {"id": user_id}, timeout=10)
        except (TimeoutError, OSError):
            pass
        # Wait for the local mock to receive the relay-side request
        # (resolve_management for non-last removals, /api/selfdestruct for
        # the last one). The default response handler logs every request.
        try:
            tc.wait_for(
                lambda: bool(_get_request_log()),
                timeout=10,
            )
        except Exception:
            pass
        # Give the firmware time to receive and process the response, and
        # for the shared AsyncHTTPSClient to be released. The
        # /api/management and /api/selfdestruct requests do not transition
        # registration_state, so we cannot rely on the idle probe here.
        time.sleep(2)
        _wait_for_remote_access_idle(tc, timeout=20)

    # After the self-destruct, the firmware's apply_config() runs and may
    # reschedule the management loop. Give it time to settle.
    time.sleep(1)
    _wait_for_remote_access_idle(tc, timeout=20)
    _reset_registration_state(tc)


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

    try:
        _point_at_local_relay(tc, enable=False)
    except (TimeoutError, OSError):
        pass

    # Restore the complete original config, including any persisted
    # service-token tracking fields and users.
    if _original_config is not None:
        try:
            tc.api("remote_access/config_update", _original_config, timeout=3)
        except (TimeoutError, OSError):
            pass

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
    _point_at_local_relay(tc, enable=False)
    time.sleep(1)


def teardown(tc: TestContext) -> None:
    """Disable remote access after each test to leave clean state."""
    global _wg_peer

    try:
        _point_at_local_relay(tc, enable=False)
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
    """After the WireGuard tunnel is lost, the module should keep re-resolving
    the management connection over multiple rounds, even after a successful
    HTTP request that ultimately fails to establish the WireGuard tunnel.

    Unlike test_reconnection (which only verifies one re-resolve attempt after
    a single 60 s management timeout), this test asserts that the device
    continues to retry across consecutive 60 s cycles.
    """
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

    # Round 1: device should detect the tunnel loss and attempt to re-resolve.
    # Use a generous timeout (>60 s timeout + jitter) so the assertion actually
    # observes the device's behaviour rather than timing out prematurely.
    def _check_reconnect_attempt():
        log = _get_request_log()
        management_reqs = [r for r in log if "/api/management" in r["path"]]
        if not management_reqs:
            raise AssertionError("No management re-resolve request after tunnel loss")

    tc.wait_for(_check_reconnect_attempt, timeout=200, poll_delay=5.0)

    # Round 2: after the next cycle the device should keep retrying even when
    # the previous HTTP request succeeded but the WireGuard tunnel could not
    # be rebuilt (the local mock relay is still up; only the WG interface is gone).
    _clear_request_log()
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
# Service-token tests
# ---------------------------------------------------------------------------
#
# These tests exercise the support-account registration/removal flow against a
# real relay (typically my.warp-charger.com). They require:
#
#   1. Signed firmware (signature_sodium_public_key_length != 0) so the
#      /remote_access/service_token_register endpoint is compiled in.
#   2. The relay currently configured on the device must be reachable and
#      willing to issue a service token for it.
#
# All tests skip if the endpoint is not registered (e.g. on unsigned firmware).

def test_service_token_register_sets_tracking(tc: TestContext) -> None:
    """After successful service-token registration the device must populate
    service_token_user_uuid so the user can be removed automatically later."""
    if not _service_token_feature_available(tc):
        tc.skip("Service-token endpoint not compiled in (firmware is likely unsigned)")
        return

    tc.set_test_timeout(60)

    # Point the device at the production relay before exercising the flow.
    _clear_service_token_registration(tc)
    _point_at_production_relay(tc, enable=False)

    # This issues a GET /api/auth/service_token
    # against my.warp-charger.com and then adds the user through
    # /api/add_with_token.
    _register_service_token(tc, timeout=30)

    # service_token_user_uuid must now be non-empty.
    uuid = _get_service_token_user_uuid(tc)
    tc.assert_ne("", uuid)


def test_service_token_register_enables_remote_access(tc: TestContext) -> None:
    """A successful service-token registration must flip enable=true (it replaces
    the manual config_update step)."""
    if not _service_token_feature_available(tc):
        tc.skip("Service-token endpoint not compiled in (firmware is likely unsigned)")
        return

    tc.set_test_timeout(60)

    _clear_service_token_registration(tc)
    _point_at_production_relay(tc, enable=False)


    _register_service_token(tc, timeout=30)
    # The registration handler updates enable=true in-memory and parse_registration()
    # persists it via API::writeConfig() before it sets registration_state=Success.
    # _register_service_token() only waits for the latter, so there is a small
    # window where state is Success but the writeConfig has not been picked up by
    # the state reader yet (or the WebServer handler is running on a different
    # task that hasn't released the lock). Poll until enable is actually True.

    tc.wait_for(lambda: tc.assert_true(tc.api("remote_access/config").get("enable")), timeout=10)


def test_service_token_register_idempotent(tc: TestContext) -> None:
    """Calling /remote_access/service_token_register again while a service user is
    already registered must be a no-op: it must not re-issue a token and must
    keep the existing tracking UUID intact. It must, however, refresh the
    deadline so the automatic removal is rescheduled 24 h into the future."""
    if not _service_token_feature_available(tc):
        tc.skip("Service-token endpoint not compiled in (firmware is likely unsigned)")
        return

    tc.set_test_timeout(120)

    _clear_service_token_registration(tc)
    _point_at_production_relay(tc, enable=False)

    # Register the service user the first time so we can exercise the
    # "already registered" branch on the second call.
    _register_service_token(tc, timeout=30)

    # The second call should not change the tracked UUID: the existing-user
    # branch in handle_service_token_register() returns early and only
    # refreshes the timestamp and re-schedules the removal.
    first_uuid = _get_service_token_user_uuid(tc)
    tc.assert_ne("", first_uuid)

    # Capture the deadline timestamp before the second call so we can verify
    # that the second call moves it forward. The field is stored as epoch
    # minutes (epoch / 60), so two calls within the same minute would be
    # indistinguishable.
    initial_ts = _get_service_token_timestamp_minutes(tc)
    tc.assert_gt(0, initial_ts)  # Must be set after the first registration.

    time.sleep(61)  # Wait for a minute to ensure the timestamp can advance.

    # The second call should not change the tracked UUID: the existing-user
    # branch in handle_service_token_register() returns early and only
    # refreshes the timestamp and re-schedules the removal.
    tc.api("remote_access/service_token_register", {}, timeout=10)

    def _check_uuid_unchanged():
        uuid = _get_service_token_user_uuid(tc)
        if uuid != first_uuid:
            raise AssertionError(
                f"Expected service_token_user_uuid={first_uuid!r}, got {uuid!r}"
            )

    tc.wait_for(_check_uuid_unchanged, timeout=10)

    # The removal deadline must have been pushed forward: the timestamp
    # that drives setup()'s 24 h removal scheduler should now be later than
    # the one we observed after the initial registration.
    def _check_deadline_moved():
        now_ts = _get_service_token_timestamp_minutes(tc)
        if now_ts <= initial_ts:
            raise AssertionError(
                f"service_token_timestamp_minutes did not advance: initial={initial_ts}, now={now_ts}"
            )

    tc.wait_for(_check_deadline_moved, timeout=10)


def test_service_token_removal_clears_tracking(tc: TestContext) -> None:
    """Manually removing the service-token user via /remote_access/remove_user
    must cancel the pending automatic removal (via cancel_service_token_removal())
    and clear the tracking UUID. The retry loop in remove_service_token_user()
    must not re-populate the tracking field after this.
    """
    if not _service_token_feature_available(tc):
        tc.skip("Service-token endpoint not compiled in (firmware is likely unsigned)")
        return

    tc.set_test_timeout(120)

    _clear_service_token_registration(tc)
    _point_at_production_relay(tc, enable=False)

    # Register first so we have a service-token user to remove.
    _register_service_token(tc, timeout=30)
    uuid = _get_service_token_user_uuid(tc)
    tc.assert_ne("", uuid)

    # Look up the user id by uuid and remove it via the regular endpoint.
    cfg = tc.api("remote_access/config")
    user_id = None
    for u in cfg["users"]:
        if u["uuid"] == uuid:
            user_id = u["id"]
            break
    tc.assert_ne(None, user_id)

    tc.api("remote_access/remove_user", {"id": user_id}, timeout=10)

    # The tracking UUID must be cleared promptly. The retry loop in
    # remove_service_token_user() must terminate on a manual removal.
    def _check_cleared():
        if _get_service_token_user_uuid(tc) != "":
            raise AssertionError("service_token_user_uuid not cleared after manual removal")

    tc.wait_for(_check_cleared, timeout=20)

    # Give the retry loop time to misbehave (it must not re-populate the
    # field). The first backoff is 1 minute; 5 s is well below it.
    time.sleep(5)
    tc.assert_eq("", _get_service_token_user_uuid(tc))


if __name__ == "__main__":
    run_testsuite(locals())
