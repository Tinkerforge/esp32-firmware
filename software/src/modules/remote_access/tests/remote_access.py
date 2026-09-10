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
# - User-level WireGuard tunnels driven by Connect/Disconnect management
#   commands (state updates, full handshake, disconnect cleanup, command
#   sequence-number handling, and disable-while-open)
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
import ctypes
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

# NaCl X25519 keypair used as the seal-receiver for the last
# _do_full_registration() call. We hold on to the private key so the
# user-tunnel helper can unseal the per-user WireGuard private keys and PSKs
# that the device ships (sealed) in the registration request body.
_last_registration_nacl_priv: X25519PrivateKey | None = None
_last_registration_nacl_pub_b64: str | None = None


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


# ---------------------------------------------------------------------------
# libsodium bindings (ctypes) for unsealing the user-tunnel WireGuard keys
# ---------------------------------------------------------------------------
#
# The device seals the relay-side WireGuard private key and shared PSK with
# the public key the test sends in the registration body (see
# crypto_box_seal in remote_access.cpp). We unseal them here so the test
# WireGuard container can act as the peer. crypto_box_seal_open is not
# exposed by the Python `cryptography` package, so we load the system
# libsodium via ctypes.

_sodium_lib: ctypes.CDLL | None = None


def _load_sodium() -> ctypes.CDLL:
    """Load system libsodium via ctypes (cached)."""
    global _sodium_lib
    if _sodium_lib is None:
        lib = ctypes.CDLL("libsodium.so.23")
        lib.sodium_init.restype = ctypes.c_int
        lib.sodium_init()

        lib.crypto_box_seal_open.restype = ctypes.c_int
        lib.crypto_box_seal_open.argtypes = [
            ctypes.c_char_p,  # m (output, plaintext)
            ctypes.c_char_p,  # c (input, ciphertext)
            ctypes.c_ulonglong,  # clen
            ctypes.c_char_p,  # pk (32 bytes)
            ctypes.c_char_p,  # sk (32 bytes)
        ]

        _sodium_lib = lib
    return _sodium_lib


def _sealed_box_open(ciphertext: bytes, pk: bytes, sk: bytes) -> bytes:
    """Open a crypto_box_seal ciphertext.

    Args:
        ciphertext: Sealed box (ephemeral pubkey || XSalsa20-Poly1305 ciphertext).
        pk: Receiver's X25519 public key (32 bytes).
        sk: Receiver's X25519 private key (32 bytes).

    Returns:
        Plaintext bytes.

    Raises:
        RuntimeError: If the box fails to open (e.g. wrong key or truncated input).
    """
    sodium = _load_sodium()
    out = ctypes.create_string_buffer(len(ciphertext))
    ret = sodium.crypto_box_seal_open(out, ciphertext, len(ciphertext), pk, sk)
    if ret != 0:
        raise RuntimeError(
            "crypto_box_seal_open failed (wrong key or malformed ciphertext)"
        )
    return out.raw[: len(ciphertext) - 48]


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

    def swap_wg0_private_key(self, relay_private: str) -> None:
        """Replace wg0's static private key at runtime via `wg set`.

        The container is initially brought up with the management tunnel's
        relay-side private key (``relay_private = wg.relay_private``).  Each
        user tunnel has its own device-generated relay-side key pair, so to
        complete a user-tunnel handshake the container must use that
        user-specific private key instead.  Calling this method invalidates
        any existing management-tunnel session (the device sees a peer
        handshake response with a different static public key) and is
        intended to be used right before driving a user-tunnel Connect.
        """
        subprocess.run(
            ["docker", "exec", WG_CONTAINER_NAME,
             "sh", "-c", f"printf '%s' '{relay_private}' > /tmp/wg0_priv"],
            check=True, capture_output=True, text=True,
        )
        subprocess.run(
            ["docker", "exec", WG_CONTAINER_NAME,
             "wg", "set", "wg0", "private-key", "/tmp/wg0_priv"],
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

    The NaCl X25519 keypair used to seal charger name/note (and the relay-side
    WireGuard private key + PSK for every user-tunnel slot) is also persisted
    in a module-level variable so _do_full_registration_with_user_tunnel can
    unseal the user-tunnel keys afterwards.

    Returns the WireGuardTestPeer instance (caller must call .stop() to clean up).

    Raises:
        RuntimeError: If WireGuard setup fails.
        AssertionError: If registration or connection fails within timeout.
    """
    global _wg_peer, _last_registration_nacl_priv, _last_registration_nacl_pub_b64

    # 1. Generate WireGuard keys (relay side only; the device generates its own
    #    private/public/PSK on-board during the /remote_access/register call).
    wg = WireGuardTestPeer()
    wg.generate_keys()

    # Generate a NaCl X25519 key pair for encrypting charger name/note and for
    # sealing the per-user-tunnel WireGuard private keys + PSKs. We keep it
    # around so _extract_user_tunnel_keys can unseal them.
    nacl_priv = X25519PrivateKey.generate()
    nacl_pub_b64 = base64.b64encode(nacl_priv.public_key().public_bytes_raw()).decode()
    _last_registration_nacl_priv = nacl_priv
    _last_registration_nacl_pub_b64 = nacl_pub_b64

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


# ---------------------------------------------------------------------------
# User-tunnel registration helper
# ---------------------------------------------------------------------------


def _extract_user_tunnel_keys(
        tc: TestContext, *, conn_no: int) -> tuple[str, str, str]:
    """Recover the WireGuard keys needed to act as the relay for one user tunnel.

    The device generates one user-tunnel key set per ``connection_no`` slot
    during /remote_access/register, seals the relay-side WireGuard private
    key and the shared PSK with the registration ``public_key`` (X25519), and
    ships the sealed blobs as arrays of bytes inside the registration body.
    We unseal them here using libsodium's ``crypto_box_seal_open`` and the
    X25519 keypair that was last passed to ``/remote_access/register`` via
    ``_do_full_registration``.

    The device stores every WireGuard key as a 44-char base64 string (see
    ``generate_wg_key`` in remote_access.cpp) and seals the wire-format
    strings directly, so the unsealed relay-side WireGuard private key and
    PSK are already in wire format and can be passed to ``wg set`` as-is.

    Returns:
        (charger_public_b64, relay_private_b64, psk_b64) for the requested
        ``connection_no``. ``relay_private_b64`` is the relay-side WireGuard
        private key for this tunnel; the device expects the matching public
        key (``relay_keys[i].pub``) on every handshake response it gets back,
        so the test container must use this key as its ``wg0`` private key
        (replacing the management-tunnel one) before driving the user tunnel.

    Raises:
        AssertionError: If the expected user-tunnel entry is missing from the
            registration body.
        RuntimeError: If the sealed box cannot be opened.
    """
    if _last_registration_nacl_priv is None:
        raise AssertionError(
            "No registration has been performed yet; call "
            "_do_full_registration first"
        )

    pk_bytes = _last_registration_nacl_priv.public_key().public_bytes_raw()
    sk_bytes = _last_registration_nacl_priv.private_bytes_raw()

    for entry in _get_request_log():
        path = entry["path"]
        if "/api/charger/add" not in path and "/api/add_with_token" not in path:
            continue
        try:
            body = json.loads(entry["body"])
        except json.JSONDecodeError:
            continue

        for key in body.get("keys", []):
            if int(key.get("connection_no", -1)) != conn_no:
                continue

            sealed_web_private = bytes(int(b) for b in key["web_private"])
            sealed_psk = bytes(int(b) for b in key["psk"])

            web_private = _sealed_box_open(sealed_web_private, pk_bytes, sk_bytes)
            psk_bytes = _sealed_box_open(sealed_psk, pk_bytes, sk_bytes)

            return (
                key["charger_public"],
                web_private.decode("ascii"),
                psk_bytes.decode("ascii"),
            )

        raise AssertionError(
            f"No entry for connection_no={conn_no} in registration keys"
        )

    raise AssertionError(
        "No /api/charger/add or /api/add_with_token request found in request log"
    )


def _do_full_registration_with_user_tunnel(
        tc: TestContext) -> tuple["WireGuardTestPeer", str, str, str]:
    """Like _do_full_registration but also recovers the user-tunnel-0 keys.

    Runs the full registration flow (including the management-tunnel
    handshake) and then unseals the first user tunnel (``connection_no=0``)
    using the NaCl X25519 keypair the device sealed against.  Tests can then
    drive Connect/Disconnect management commands and observe the WireGuard
    handshake via the test peer.

    Returns:
        (wg, charger_public, relay_private, psk) for the user tunnel at
        ``connection_no=0``. ``relay_private`` is the (unsealed) relay-side
        WireGuard private key for this tunnel; ``test_user_full_wg_handshake``
        swaps it onto the container's wg0 so the user-tunnel handshake can
        complete.

    Raises:
        RuntimeError: WireGuard setup fails.
        AssertionError: Registration does not complete within timeout, or the
            expected user-tunnel-0 entry is missing.
    """
    wg = _do_full_registration(tc)
    charger_public, relay_private, psk = _extract_user_tunnel_keys(tc, conn_no=0)
    return wg, charger_public, relay_private, psk


def _wait_for_management_connected(tc: TestContext, *, timeout: float = 60.0) -> None:
    """Wait until the management connection slot (index 0) reports STATE_CONNECTED."""
    tc.wait_for(
        lambda: tc.assert_eq(STATE_CONNECTED, _get_connection_state(tc)[0]["state"]),
        timeout=timeout,
    )


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
    tc.wait_for(
        lambda: tc.assert_true(any("/api/management" in r["path"] for r in _get_request_log())),
        timeout=timeout,
    )


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

    tc.wait_for(
        lambda: tc.assert_eq(REG_STATE_SUCCESS, _get_registration_state(tc)["state"]),
        timeout=timeout,
    )


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
    tc.wait_for(
        lambda: tc.assert_ne(REG_STATE_IN_PROGRESS, _get_registration_state(tc).get("state")),
        timeout=timeout,
    )


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
                lambda: tc.assert_true(bool(_get_request_log())),
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
    tc.wait_for(
        lambda: tc.assert_false(tc.api("remote_access/config")["enable"]),
        timeout=10,
    )

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
    # The retry happens after ~30s, so total wait needs to be >60s
    tc.wait_for(
        lambda: tc.assert_ge(2, sum(1 for r in _get_request_log() if "/api/management" in r["path"])),
        timeout=90,
        poll_delay=2.0,
    )

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
    tc.wait_for(lambda: _assert_all_disconnected(tc), timeout=5)


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
    tc.wait_for(
        lambda: tc.assert_true(any("/api/management" in r["path"] for r in _get_request_log())),
        timeout=300,
        poll_delay=5.0,
    )

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
    tc.wait_for(
        lambda: tc.assert_true(any("/api/management" in r["path"] for r in _get_request_log())),
        timeout=200,
        poll_delay=5.0,
    )

    # Round 2: after the next cycle the device should keep retrying even when
    # the previous HTTP request succeeded but the WireGuard tunnel could not
    # be rebuilt (the local mock relay is still up; only the WG interface is gone).
    _clear_request_log()
    tc.wait_for(
        lambda: tc.assert_true(any("/api/management" in r["path"] for r in _get_request_log())),
        timeout=200,
        poll_delay=5.0,
    )

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
    tc.wait_for(lambda: tc.assert_true(wg.is_peer_connected()), timeout=60, poll_delay=2.0)

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
#
# These tests verify that the Connect/Disconnect management commands drive
# the user tunnels correctly. The management connection is established by
# _do_full_registration_with_user_tunnel (full registration flow including
# the WireGuard management handshake), then we unseal the user-tunnel keys
# that the device shipped (sealed) in the registration body.

_CONN_COMMAND_CONNECT = 0
_CONN_COMMAND_DISCONNECT = 1


def _wait_for_any_user_slot_active(tc: TestContext, *, timeout: float = 15.0) -> None:
    """Wait until at least one user slot (index 1-5) has user != 255."""
    tc.wait_for(
        lambda: tc.assert_true(any(
            s["user"] != 255 for s in _get_connection_state(tc)[1:]
        )),
        timeout=timeout,
    )


def _assert_all_user_slots_cleared(tc: TestContext) -> None:
    """Assert every user slot (index 1-5) is reset to user=255, connection=255,
    and state=Disconnected."""
    for i, slot in enumerate(_get_connection_state(tc)[1:], start=1):
        if slot["user"] != 255 or slot["connection"] != 255:
            raise AssertionError(f"Slot {i} not cleared after Disconnect: {slot}")
        if slot["state"] != STATE_DISCONNECTED:
            raise AssertionError(
                f"Slot {i} not in Disconnected state after Disconnect: {slot}"
            )


def test_user_connect_command_updates_state(tc: TestContext) -> None:
    """A Connect management command should populate a user slot with the right IDs."""
    tc.set_test_timeout(90)

    try:
        wg, _charger_public, _relay_priv, _psk = _do_full_registration_with_user_tunnel(tc)
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

    active = [s for s in _get_connection_state(tc)[1:] if s["user"] != 255]
    tc.assert_eq(1, len(active))
    tc.assert_eq(0, active[0]["user"])
    tc.assert_eq(0, active[0]["connection"])


def test_user_full_wg_handshake(tc: TestContext) -> None:
    """After a Connect command the user WireGuard handshake should complete and
    the slot should transition to STATE_CONNECTED."""
    tc.set_test_timeout(120)

    try:
        wg, user_charger_public, user_relay_priv, user_psk = _do_full_registration_with_user_tunnel(tc)
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")
        return

    _wait_for_management_connected(tc, timeout=60)

    # The device generates its own relay-side key per user tunnel at
    # registration time. To complete the user-tunnel handshake, the
    # container's wg0 must use the user tunnel's relay-side private key
    # (not the management tunnel's). Sending the Connect command must
    # happen while the management tunnel is still up (we use it to
    # deliver the command) -- immediately afterwards we re-key wg0 and
    # the user tunnel handshake can then complete.  The management tunnel
    # is intentionally invalidated by the re-key; this test only cares
    # about the user tunnel.
    conn_uuid = os.urandom(16)
    wg.send_management_command(
        seq_num=1, command_id=_CONN_COMMAND_CONNECT,
        connection_no=0, connection_uuid=conn_uuid,
    )

    # Give the device a moment to start its user-tunnel WG interface
    # (DNS resolve + connect_remote_access); handshakes typically start
    # immediately afterwards.
    time.sleep(1)

    wg.swap_wg0_private_key(user_relay_priv)
    wg.add_user_peer(user_charger_public, user_psk, conn_no=0)

    # Wait for the WireGuard handshake from the relay side.
    tc.wait_for(
        lambda: tc.assert_true(wg.is_user_peer_connected(user_charger_public)),
        timeout=30, poll_delay=1.0,
    )

    # Wait for the device API to reflect STATE_CONNECTED.
    tc.wait_for(
        lambda: tc.assert_true(any(
            s["state"] == STATE_CONNECTED for s in _get_connection_state(tc)[1:]
        )),
        timeout=15,
    )

    connected = [s for s in _get_connection_state(tc)[1:] if s["state"] == STATE_CONNECTED]
    tc.assert_eq(1, len(connected))
    tc.assert_eq(0, connected[0]["user"])
    tc.assert_eq(0, connected[0]["connection"])


def test_user_disconnect_command_clears_state(tc: TestContext) -> None:
    """A Disconnect management command should tear down the tunnel and reset the slot."""
    tc.set_test_timeout(90)

    try:
        wg, _charger_public, _relay_priv, _psk = _do_full_registration_with_user_tunnel(tc)
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
    tc.wait_for(lambda: _assert_all_user_slots_cleared(tc), timeout=10)


def test_duplicate_seq_num_ignored(tc: TestContext) -> None:
    """A command with the same sequence number as the last processed command
    should be silently dropped."""
    tc.set_test_timeout(90)

    try:
        wg, _, _, _ = _do_full_registration_with_user_tunnel(tc)
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
    active = [s for s in _get_connection_state(tc)[1:] if s["user"] != 255]
    tc.assert_eq(1, len(active))
    tc.assert_eq(0, active[0]["connection"])


def test_out_of_order_command_ignored(tc: TestContext) -> None:
    """A command whose sequence number is not greater than the last seen value
    should be ignored, leaving the existing connection intact."""
    tc.set_test_timeout(90)

    try:
        wg, _, _, _ = _do_full_registration_with_user_tunnel(tc)
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

    # The connection for connection_no=0 should still be active (the
    # out-of-order Disconnect with seq=1 must be ignored because in_seq_number
    # is now 2).
    active = [s for s in _get_connection_state(tc)[1:] if s["user"] != 255]
    tc.assert_ge(1, len(active))
    tc.assert_eq(0, active[0]["connection"])


def test_disable_closes_user_connections(tc: TestContext) -> None:
    """Disabling remote access while a user tunnel is open should tear it down
    and reset all user slots to Disconnected."""
    tc.set_test_timeout(90)

    try:
        wg, _charger_public, _relay_priv, _psk = _do_full_registration_with_user_tunnel(tc)
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
    tc.wait_for(lambda: _assert_all_disconnected(tc), timeout=10)


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

    tc.wait_for(
        lambda: tc.assert_eq(first_uuid, _get_service_token_user_uuid(tc)),
        timeout=10,
    )

    # The removal deadline must have been pushed forward: the timestamp
    # that drives setup()'s 24 h removal scheduler should now be later than
    # the one we observed after the initial registration.
    tc.wait_for(
        lambda: tc.assert_gt(initial_ts, _get_service_token_timestamp_minutes(tc)),
        timeout=10,
    )


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
    tc.wait_for(
        lambda: tc.assert_eq("", _get_service_token_user_uuid(tc)),
        timeout=20,
    )

    # Give the retry loop time to misbehave (it must not re-populate the
    # field). The first backoff is 1 minute; 5 s is well below it.
    time.sleep(5)
    tc.assert_eq("", _get_service_token_user_uuid(tc))


# ---------------------------------------------------------------------------
# Auth-token (non-service-token) registration tests
# ---------------------------------------------------------------------------
#
# These tests verify that enabling remote access via a normal user
# auth-token -- the path taken when an existing warp-charger.com user adds a
# new charger to their account -- goes through the same /api/add_with_token
# endpoint as the service-token flow, but is parameterized with
# is_service_token=false. The two paths share the relay handshake but the
# auth-token one must NOT touch the service_token_user_uuid tracking field,
# because that field is only meaningful for support accounts that have to be
# removed automatically after 24 h.


def _do_auth_token_register(tc: TestContext, *, timeout: float = 15.0) -> str:
    """Register a new charger via /remote_access/register using an auth-token.

    Points the device at the local mock relay, issues the auth-token register
    call and waits for registration_state=Success. The mock returns the
    minimal response shape parse_registration() needs.

    Returns the user_id the relay assigned (taken from the response body), so
    callers can compare it against service_token_user_uuid.
    """
    # Generate a NaCl X25519 key pair that the device will seal the relay-side
    # WireGuard keys and charger name/note against. We hold on to it so we can
    # also verify the management-tunnel keys afterwards (mirrors the pattern in
    # _do_full_registration).
    global _last_registration_nacl_priv, _last_registration_nacl_pub_b64
    nacl_priv = X25519PrivateKey.generate()
    nacl_pub_b64 = base64.b64encode(nacl_priv.public_key().public_bytes_raw()).decode()
    _last_registration_nacl_priv = nacl_priv
    _last_registration_nacl_pub_b64 = nacl_pub_b64

    assigned_user_id = str(uuid_mod.uuid4())
    registration_received = threading.Event()

    def relay_handler(method: str, path: str, body: bytes) -> tuple[int, str]:
        _log_request(method, path, body)
        # The auth-token register path goes to /api/add_with_token (when
        # user_uuid and auth_token are both set in the body). It must NOT use
        # /api/charger/add -- if we observe /api/charger/add, the firmware
        # ignored the auth-token fields and the test is meaningless.
        if "/api/add_with_token" in path or "/api/charger/add" in path:
            registration_received.set()
            return (200, json.dumps({
                "charger_uuid": str(uuid_mod.uuid4()),
                "charger_password": base64.b64encode(os.urandom(16)).decode(),
                # management_pub is required by parse_registration() but we
                # don't drive the WireGuard handshake here; a syntactically
                # valid base64 32-byte X25519 public key is enough.
                "management_pub": base64.b64encode(os.urandom(32)).decode(),
                "user_id": assigned_user_id,
            }))
        # Subsequent periodic management polling (the device enables itself
        # once registration succeeds) must also get a valid response so the
        # connection state machine does not loop with errors.
        if "/api/management" in path:
            return (200, _make_management_response_ok())
        return (404, '{"error": "not found"}')

    _server.set_response_fn(relay_handler)

    _point_at_local_relay(tc, enable=False)
    _clear_request_log()

    register_body = {
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
        # Non-null user_uuid + auth_token selects the /api/add_with_token
        # (auth-token) path in register_urls(); see use_token_path at
        # remote_access.cpp:1345.
        "user_uuid": str(uuid_mod.uuid4()),
        "auth_token": "test-auth-token-" + os.urandom(8).hex(),
    }

    tc.api("remote_access/register", register_body, timeout=10)

    if not registration_received.wait(timeout=timeout):
        raise AssertionError("Device did not send registration request to mock relay")

    tc.wait_for(
        lambda: tc.assert_eq(REG_STATE_SUCCESS, _get_registration_state(tc)["state"]),
        timeout=timeout,
    )

    return assigned_user_id


def test_auth_token_register_enables_remote_access(tc: TestContext) -> None:
    """Enabling remote access via /remote_access/register with a normal
    auth_token must flip enable=true (the register endpoint applies the
    submitted config's enable flag via parse_registration()) and must NOT
    populate service_token_user_uuid.

    The service_token_user_uuid field is only written by the service-token
    branch of register_with_relay() / allow_user_at_relay() (the is_service_token
    argument). The auth-token branch is parameterized with is_service_token=false
    and must leave the tracking field empty, otherwise the 24 h automatic
    removal task would also remove this user's account.

    The invariant is checked both right after registration and after the
    device has had time to run at least one periodic management poll, so a
    bug that lazily populates the field later (for example from the management
    response) would still be caught.
    """
    tc.set_test_timeout(60)

    _clear_service_token_registration(tc)

    assigned_user_id = _do_auth_token_register(tc)

    # The relay's response must have populated a new user in the config.
    cfg = tc.wait_for(lambda: _get_remote_access_config_with_user(tc, assigned_user_id), timeout=10)

    # remote access must be enabled after a successful registration. The
    # config_update we sent inside the register body contains enable=true and
    # parse_registration() writes it back into the live config.
    tc.assert_true(cfg["enable"])

    # The newly-added user must be present (sanity check that the relay
    # response was actually applied, not that we just flipped enable locally).
    tc.assert_eq(1, len(cfg.get("users", [])))
    tc.assert_eq(assigned_user_id, cfg["users"][0]["uuid"])

    # The auth-token branch must NOT populate the service-token tracking
    # fields. Both must stay empty/zero: the user_uuid that came back from the
    # relay belongs to a real user, not to the support account, so it would be
    # a bug if the firmware treated it as a service-token user.
    tc.assert_eq("", _get_service_token_user_uuid(tc))
    tc.assert_eq(0, _get_service_token_timestamp_minutes(tc))

    # The auth-token register path goes straight to /api/add_with_token and
    # must not hit the production-only /api/auth/service_token endpoint that
    # service-token registrations use. If we observed such a request, the
    # firmware took the wrong branch.
    log = _get_request_log()
    service_token_requests = [r for r in log if "/api/auth/service_token" in r["path"]]
    tc.assert_eq(0, len(service_token_requests))

    # The relay must have observed the auth-token registration on the
    # /api/add_with_token endpoint, not /api/charger/add (which is the path
    # used when the client did not supply user_uuid + auth_token).
    add_with_token_requests = [r for r in log if "/api/add_with_token" in r["path"]]
    tc.assert_ge(1, len(add_with_token_requests))

    # /api/charger/add must NOT have been called: that endpoint is used when
    # the client supplies only secret_key/public_key without an auth_token,
    # i.e. a charger-only registration flow that knows nothing about a user
    # account. Using it here would mean the firmware ignored our
    # user_uuid + auth_token fields.
    charger_add_requests = [r for r in log if "/api/charger/add" in r["path"]]
    tc.assert_eq(0, len(charger_add_requests))

    # The body we received must contain the auth_token we submitted so the
    # relay can verify it.
    body = json.loads(add_with_token_requests[0]["body"])
    tc.assert_("token" in body)
    tc.assert_("user_id" in body)

    # The service-token tracking field must stay empty even after the device
    # has had a chance to do some work: wait for at least one periodic
    # management poll to arrive at the mock relay, then re-check both the
    # tracking UUID and the timestamp. This catches any race where the field
    # is populated lazily from a later code path (e.g. a buggy management
    # response handler that copies user_id into the tracking field).
    _wait_for_management_request(tc, timeout=30)
    tc.assert_eq("", _get_service_token_user_uuid(tc))
    tc.assert_eq(0, _get_service_token_timestamp_minutes(tc))


def _get_remote_access_config_with_user(tc: TestContext, user_uuid: str) -> dict:
    """Fetch remote_access/config and assert that ``user_uuid`` is in users[].

    Returns the full config dict so callers can chain more assertions on it
    without re-reading the API.
    """
    cfg = tc.api("remote_access/config")
    for user in cfg.get("users", []):
        if user.get("uuid") == user_uuid:
            return cfg
    raise AssertionError(
        f"User with uuid={user_uuid} not found in remote_access/config users[]"
    )


if __name__ == "__main__":
    run_testsuite(locals())
