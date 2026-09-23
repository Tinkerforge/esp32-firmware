#!/usr/bin/env -S uv run --group tests --script

"""Exercise manual and monthly charge-log uploads against a local relay."""

import base64
import json
import os
import queue
import struct
import subprocess
import threading
import time
import uuid

from cryptography.hazmat.primitives.asymmetric.x25519 import X25519PrivateKey
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext
from software.test_runner.remote_access_snapshot import RemoteAccessSnapshot
from software.src.modules.remote_access.tests.remote_access import (
    WG_CONTAINER_NAME, WireGuardTestPeer, _generate_wg_keypair, _generate_wg_psk,
)


CERT_ID = 2
USER_UUID = uuid.UUID("e4535915-0069-4b3c-9c7f-5a447e82cd08")
PACKET_ACK = 1
PACKET_NACK = 2
PACKET_METADATA = 3
PACKET_REQUEST = 4
NACK_BUSY = 0
NACK_ALREADY_SENT = 6

# The container owns the relay's tunnel interface. Adding .3 allows it to
# receive the device's charge-log UDP packets and TCP stream at their real IPs.
RELAY_SCRIPT = r"""
import base64
import json
import os
import select
import socket
import sys

udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp.bind(('10.123.123.3', 12345))
tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
tcp.bind(('10.123.123.3', 8080))
tcp.listen(1)
print(json.dumps({'kind': 'ready'}), flush=True)

client = None
size = 0
prefix = b''
pending = b''
while True:
    readers = [0, udp, tcp]
    if client is not None:
        readers.append(client)
    for source in select.select(readers, [], [])[0]:
        if source == 0:
            chunk = os.read(0, 4096)
            if not chunk:
                sys.exit(0)
            pending += chunk
            while b'\n' in pending:
                line, pending = pending.split(b'\n', 1)
                command = json.loads(line)
                payload = bytes.fromhex(command['packet'])
                udp.sendto(payload, ('10.123.123.2', 12345))
        elif source is udp:
            packet, _ = udp.recvfrom(2048)
            print(json.dumps({'kind': 'udp', 'packet': packet.hex()}), flush=True)
        elif source is tcp:
            client, _ = tcp.accept()
            size = 0
            prefix = b''
        else:
            chunk = client.recv(8192)
            if chunk:
                size += len(chunk)
                prefix += chunk[:max(0, 512 - len(prefix))]
            else:
                client.close()
                client = None
                print(json.dumps({'kind': 'tcp', 'size': size,
                                  'prefix': base64.b64encode(prefix).decode()}), flush=True)
"""


class ChargeLogRelay:
    def __init__(self):
        self.process: subprocess.Popen | None = None
        self.events: queue.Queue[dict] = queue.Queue()
        self.reader: threading.Thread | None = None

    def start(self):
        subprocess.run(
            ["docker", "exec", WG_CONTAINER_NAME, "ip",
             "addr", "add", "10.123.123.3/32", "dev", "wg0"],
            check=True, capture_output=True, text=True,
        )
        self.process = subprocess.Popen(
            ["docker", "exec", "-i", WG_CONTAINER_NAME, "python3", "-u", "-c", RELAY_SCRIPT],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            text=True, bufsize=1,
        )
        self.reader = threading.Thread(target=self._read_events, daemon=True)
        self.reader.start()
        self.next_event("ready", timeout=10)

    def _read_events(self):
        assert self.process is not None and self.process.stdout is not None
        for line in self.process.stdout:
            self.events.put(json.loads(line))

    def next_event(self, kind: str, timeout: float = 15) -> dict:
        try:
            event = self.events.get(timeout=timeout)
        except queue.Empty as exc:
            details = ""
            if self.process is not None and self.process.poll() is not None:
                details = f" (relay exited {self.process.returncode}: {self.process.stderr.read()})"
            raise AssertionError(f"Timed out waiting for relay {kind} event{details}") from exc
        assert event["kind"] == kind, f"Expected {kind}, received {event}"
        return event

    def send_response(self, packet_type: int, reason: int | None = None):
        assert self.process is not None and self.process.stdin is not None
        payload = bytes([reason]) if reason is not None else b""
        header = struct.pack("<HHHBB", 0x1234, len(payload), 0, 1, packet_type)
        self.process.stdin.write(json.dumps({"packet": (header + payload).hex()}) + "\n")
        self.process.stdin.flush()

    def stop(self):
        if self.process is None:
            return
        if self.process.stdin is not None:
            self.process.stdin.close()
        try:
            self.process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            self.process.terminate()
            self.process.wait(timeout=5)
        if self.reader is not None:
            self.reader.join(timeout=5)
        self.process = None


_server = None
_peer: WireGuardTestPeer | None = None
_relay: ChargeLogRelay | None = None
_original_charge_config: dict | None = None
_snapshot: RemoteAccessSnapshot | None = None


def suite_setup(tc: TestContext):
    global _server, _peer, _relay, _original_charge_config, _snapshot, CERT_ID
    tc.set_test_timeout(120)
    initial_state = tc.api("charge_tracker/state")  # Skip if no device is configured.
    tc.assert_eq(0, initial_state["generator_state"])
    tc.get_local_ip()
    charge_config = tc.api("charge_tracker/config")
    if "remote_upload_configs" not in charge_config:
        tc.skip("Firmware does not include charge-log sending")
    if charge_config["remote_upload_configs"]:
        tc.skip("Monthly uploads are configured and could interfere with this relay test")
    _original_charge_config = charge_config
    _snapshot = RemoteAccessSnapshot.capture(tc)
    CERT_ID = _snapshot.cert_id

    _server = tc.create_test_https_server(CERT_ID, "charge_log_send_test_cert")
    _peer = WireGuardTestPeer()
    _peer.generate_keys()
    try:
        _peer.start()
    except RuntimeError as exc:
        tc.skip(f"WireGuard setup not available: {exc}")

    _relay = ChargeLogRelay()
    _relay.start()

    registered = threading.Event()
    charger_uuid = str(uuid.uuid4())
    password = base64.b64encode(os.urandom(16)).decode()

    def https_handler(method: str, path: str, body: bytes) -> tuple[int, str]:
        if "/api/charger/add" in path or "/api/add_with_token" in path:
            registered.set()
            return 200, json.dumps({
                "charger_uuid": charger_uuid,
                "charger_password": password,
                "management_pub": _peer.relay_public,
                "user_id": str(USER_UUID),
            })
        if "/api/management" in path:
            return 200, json.dumps({
                "configured_users": [1],
                "configured_users_emails": ["test@example.com"],
                "configured_users_uuids": [str(USER_UUID)],
            })
        return 404, '{"error":"not found"}'

    _server.set_response_fn(https_handler)
    charger_private, charger_public = _generate_wg_keypair()
    web_private, web_public = _generate_wg_keypair()
    nacl_public = X25519PrivateKey.generate().public_key().public_bytes_raw()
    tc.api("remote_access/register", {
        "config": {
            "enable": True, "relay_host": tc.get_local_ip(), "relay_port": _server.port,
            "email": "test@example.com", "cert_id": CERT_ID, "mtu": 1240,
        },
        "note": "", "public_key": base64.b64encode(nacl_public).decode(),
        "mgmt_charger_private": _peer.device_private,
        "mgmt_charger_public": _peer.device_public,
        "mgmt_psk": _peer.psk,
        "keys": [{
            "charger_private": charger_private, "charger_public": charger_public,
            "psk": _generate_wg_psk(), "web_private": web_private,
            "web_public": web_public,
        }],
        "user_uuid": "null", "auth_token": "null",
    }, timeout=10)
    tc.assert_(registered.wait(timeout=15))

    def connected():
        assert tc.api("remote_access/state")[0]["state"] == 2

    tc.wait_for(connected, timeout=60)


def suite_teardown(tc: TestContext):
    global _server, _peer, _relay, _snapshot
    if _server is None:
        return

    errors: list[Exception] = []

    def attempt(fn):
        try:
            fn()
        except Exception as exc:
            errors.append(exc)

    attempt(lambda: tc.api("remote_access/config_update", {
        "enable": False, "relay_host": tc.get_local_ip(), "relay_port": _server.port,
        "email": "test@example.com", "cert_id": CERT_ID, "mtu": 1240,
    }, timeout=3))
    time.sleep(1)
    if _relay is not None:
        attempt(_relay.stop)
        _relay = None
    if _peer is not None:
        attempt(_peer.stop)
        _peer = None
    if _snapshot is not None:
        attempt(lambda: _snapshot.clear_registration(tc, _server))
    attempt(lambda: tc.api("certs/remove", {"id": CERT_ID}))
    attempt(_server.stop)
    _server = None
    if _snapshot is not None:
        attempt(lambda: _snapshot.restore(tc))
        _snapshot = None
    if errors:
        raise RuntimeError(f"Charge-log teardown had {len(errors)} cleanup error(s)") from errors[0]


def _send_upload(tc: TestContext, file_type: str):
    tc.api(f"charge_tracker/send_charge_log_{file_type}", {
        "api_not_final_acked": True,
        "user_filter": -2, "device_filter": -2,
        "start_timestamp_min": 0, "end_timestamp_min": 0,
        "current_timestamp_min": int(time.time() // 60),
        "language": 1, "csv_delimiter": 0,
        "letterhead": "Charge log test", "persist_letterhead": False,
        "cookie": 42, "remote_access_user_uuid": str(USER_UUID),
    }, timeout=10)


def _next_packet(tc: TestContext, packet_type: int, timeout: float = 15) -> bytes:
    assert _relay is not None
    packet = bytes.fromhex(_relay.next_event("udp", timeout)["packet"])
    tc.assert_ge(8, len(packet))
    magic, length, seq, version, actual_type = struct.unpack_from("<HHHBB", packet)
    tc.assert_eq((0x1234, len(packet) - 8, 0, 1, packet_type),
                 (magic, length, seq, version, actual_type))
    return packet[8:]


def _request(tc: TestContext, file_type: str) -> bytes:
    _send_upload(tc, file_type)
    digest = _next_packet(tc, PACKET_REQUEST)
    tc.assert_eq(32, len(digest))
    return digest


def _metadata(tc: TestContext, suffix: str, monthly: bool = False):
    assert _relay is not None
    _relay.send_response(PACKET_ACK)
    payload = _next_packet(tc, PACKET_METADATA)
    tc.assert_eq(USER_UUID.bytes, payload[:16])
    filename_len, display_name_len = struct.unpack_from("<HH", payload, 16)
    tc.assert_eq(b"en", payload[20:22])
    tc.assert_eq(1 if monthly else 0, payload[22])
    tc.assert_eq(23 + filename_len + display_name_len, len(payload))
    filename = payload[23:23 + filename_len].decode()
    display_name = payload[23 + filename_len:].decode()
    tc.assert_(filename.endswith(suffix))
    tc.assert_(len(display_name) > 0)


def _receive_file(tc: TestContext) -> dict:
    assert _relay is not None
    _relay.send_response(PACKET_ACK)  # Metadata accepted.
    _relay.send_response(PACKET_ACK)  # Open the data connection.
    return _relay.next_event("tcp", timeout=60)


def _wait_until_ready(tc: TestContext, timeout: float = 10):
    def check():
        state = tc.api("charge_tracker/state")
        assert state["generator_state"] == 0, state

    tc.wait_for(check, timeout=timeout)


def teardown(tc: TestContext):
    # Every exchange must release the generation lock, including Nack paths.
    try:
        _wait_until_ready(tc, timeout=2)
    except AssertionError:
        # A failed assertion can leave a request awaiting a relay response.
        assert _relay is not None
        _relay.send_response(PACKET_NACK, NACK_BUSY)
        _wait_until_ready(tc, timeout=5)


def test_csv_upload(tc: TestContext):
    tc.set_test_timeout(120)
    assert _relay is not None
    _request(tc, "csv")
    _metadata(tc, ".csv")
    received = _receive_file(tc)
    data = base64.b64decode(received["prefix"])
    tc.assert_(received["size"] >= len(data) > 0)
    tc.assert_(data.startswith(b"sep=;\r\n"))
    tc.assert_(b"Start time" in data)
    time.sleep(0.5)  # TCP EOF precedes the firmware's WaitingForFinalAck assignment.
    _relay.send_response(PACKET_ACK)


def test_pdf_upload(tc: TestContext):
    tc.set_test_timeout(120)
    assert _relay is not None
    _request(tc, "pdf")
    _metadata(tc, ".pdf")
    received = _receive_file(tc)
    data = base64.b64decode(received["prefix"])
    tc.assert_(received["size"] > 100)
    tc.assert_(data.startswith(b"%PDF-"))
    time.sleep(0.5)
    _relay.send_response(PACKET_ACK)


def test_already_sent_skips_transfer(tc: TestContext):
    assert _relay is not None
    _request(tc, "csv")
    _relay.send_response(PACKET_NACK, NACK_ALREADY_SENT)
    # An AlreadySent reply finishes this upload. A new request must be accepted
    # without any metadata or TCP connection from the first attempt.
    _wait_until_ready(tc)
    _request(tc, "csv")
    _relay.send_response(PACKET_NACK, NACK_BUSY)


def test_busy_request_can_be_retried(tc: TestContext):
    assert _relay is not None
    digest = _request(tc, "csv")
    _relay.send_response(PACKET_NACK, NACK_BUSY)
    _wait_until_ready(tc)
    tc.assert_eq(digest, _request(tc, "csv"))
    _relay.send_response(PACKET_NACK, NACK_ALREADY_SENT)


def test_metadata_rejection_can_be_retried(tc: TestContext):
    assert _relay is not None
    _request(tc, "csv")
    _metadata(tc, ".csv")
    _relay.send_response(PACKET_NACK, NACK_BUSY)
    _wait_until_ready(tc)
    _request(tc, "csv")
    _relay.send_response(PACKET_NACK, NACK_ALREADY_SENT)


def _start_monthly_upload(tc: TestContext, file_type: int):
    assert _original_charge_config is not None

    users = tc.api("remote_access/config")["users"]
    tc.assert_(any(user["id"] == 1 and user["uuid"] == str(USER_UUID) for user in users))

    config = dict(_original_charge_config)
    config["remote_upload_configs"] = [{
        "user_id": 1,
        "file_type": file_type,
        "language": 1,
        "letterhead": "Monthly charge log test",
        "user_filter": -2,
        "device_filter": -2,
        "csv_delimiter": 0,
        "last_upload_timestamp_min": 0,
    }]
    tc.api("charge_tracker/config_update", config, timeout=5)
    tc.assert_eq(0, tc.api("charge_tracker/config")["remote_upload_configs"][0]["last_upload_timestamp_min"])
    tc.reboot()

    digest = _next_packet(tc, PACKET_REQUEST, timeout=90)
    tc.assert_eq(32, len(digest))
    _metadata(tc, ".pdf" if file_type == 0 else ".csv", monthly=True)


def _finish_monthly_upload(tc: TestContext):
    assert _relay is not None
    time.sleep(0.5)
    _relay.send_response(PACKET_ACK)

    def timestamp_updated():
        config = tc.api("charge_tracker/config")
        timestamp = config["remote_upload_configs"][0]["last_upload_timestamp_min"]
        assert timestamp > 0, config

    tc.wait_for(timestamp_updated, timeout=15)
    _wait_until_ready(tc)


def test_monthly_csv_upload(tc: TestContext):
    tc.set_test_timeout(180)
    _start_monthly_upload(tc, file_type=1)
    received = _receive_file(tc)
    data = base64.b64decode(received["prefix"])
    tc.assert_(received["size"] >= len(data) > 0)
    tc.assert_(data.startswith(b"sep=;\r\n"))
    tc.assert_(b"Start time" in data)
    _finish_monthly_upload(tc)


def test_monthly_pdf_upload(tc: TestContext):
    tc.set_test_timeout(180)
    _start_monthly_upload(tc, file_type=0)
    received = _receive_file(tc)
    data = base64.b64decode(received["prefix"])
    tc.assert_(received["size"] > 100)
    tc.assert_(data.startswith(b"%PDF-"))
    _finish_monthly_upload(tc)


if __name__ == "__main__":
    run_testsuite(locals())
