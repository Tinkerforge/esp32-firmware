"""Minimal EEBUS test peer (SHIP client + SPINE device) for the WARP EEBUS tests.

The peer connects to the SHIP server of the device under test, answers the
SPINE requests of the device (detailed discovery, use cases, subscriptions,
bindings, heartbeat reads) and lets a test send arbitrary SPINE messages.

Intentionally small and permissive: tests have to be able to send invalid or
unusual messages and to imitate peers that behave differently from eebus-go
(e.g. the SMA Sunny Home Manager).

    peer = EebusPeer("192.168.1.147", energy_guard_layout(), log=tc.dbg)
    tc.api("eebus/add", peer.add_peer_payload())
    peer.connect()
    peer.discover()
    lpc = LpcEnergyGuard(peer)
    lpc.setup()
    tc.assert_eq(ERROR_NO_ERROR, lpc.write_limit(4200, active=True, duration=300))
    peer.close()
"""

from __future__ import annotations

import datetime
import json
import os
import ssl
import tempfile
import threading
import time
from dataclasses import dataclass, field
from typing import Any, Callable

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import NameOID
from websockets.exceptions import ConnectionClosed
from websockets.sync.client import connect as ws_connect

SHIP_PORT = 4712
SPINE_SPEC_VERSION = "1.3.0"

SHIP_MSG_INIT = 0
SHIP_MSG_CONTROL = 1
SHIP_MSG_DATA = 2
SHIP_MSG_END = 3

# SPINE ErrorNumberType
ERROR_NO_ERROR = 0
ERROR_GENERAL = 1
ERROR_COMMAND_NOT_SUPPORTED = 6
ERROR_COMMAND_REJECTED = 7
ERROR_BINDING_REQUIRED = 9

NODE_MANAGEMENT = {"entity": [0], "feature": 0}


class PeerError(AssertionError):
    """Raised on timeouts and protocol errors. Derived from AssertionError, so the test runner reports a test failure."""


# ---------------------------------------------------------------------------
# EEBUS JSON (SHIP 13.4.1): Objects are encoded as arrays of single-key objects
# ---------------------------------------------------------------------------

def to_eebus(value: Any) -> Any:
    if isinstance(value, dict):
        return [{k: to_eebus(v)} for k, v in value.items()]
    if isinstance(value, list):
        return [to_eebus(v) for v in value]
    return value


def from_eebus(value: Any) -> Any:
    if isinstance(value, dict):
        return {k: from_eebus(v) for k, v in value.items()}
    if isinstance(value, list):
        # [] is an empty object (or an empty list, both are handled by as_list())
        if len(value) == 0:
            return {}
        if all(isinstance(v, dict) and len(v) == 1 for v in value):
            obj = {}
            for v in value:
                ((k, x),) = v.items()
                obj[k] = from_eebus(x)
            return obj
        return [from_eebus(v) for v in value]
    return value


def as_list(value: Any) -> list:
    """Normalizes a decoded list: None and {} become [], a single object becomes [object]."""
    if value is None or value == {}:
        return []
    if isinstance(value, list):
        return value
    return [value]


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def iso_duration(seconds: int) -> str:
    seconds = int(seconds)
    if seconds <= 0:
        return "PT0S"
    h, rest = divmod(seconds, 3600)
    m, s = divmod(rest, 60)
    return "PT" + (f"{h}H" if h else "") + (f"{m}M" if m else "") + (f"{s}S" if s else "")


def scaled_number(value: float | int) -> dict:
    if float(value).is_integer():
        return {"number": int(value), "scale": 0}
    scale = 0
    while not float(value).is_integer() and scale > -6:
        value *= 10
        scale -= 1
    return {"number": int(round(value)), "scale": scale}


def from_scaled_number(sn: dict) -> float:
    return sn.get("number", 0) * 10 ** sn.get("scale", 0)


def utc_timestamp() -> str:
    return datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def addr_str(a: dict | None) -> str:
    if not a:
        return "?"
    return f"{a.get('device', '')}/{a.get('entity', '?')}/{a.get('feature', '?')}"


def same_feature(a: dict | None, b: dict | None) -> bool:
    """Compares entity and feature; the device only if both addresses contain one."""
    if not a or not b:
        return False
    if a.get("device") and b.get("device") and a["device"] != b["device"]:
        return False
    return list(a.get("entity", [])) == list(b.get("entity", [])) and a.get("feature") == b.get("feature")


# ---------------------------------------------------------------------------
# Identity
# ---------------------------------------------------------------------------

@dataclass
class Identity:
    cert_pem: bytes
    key_pem: bytes
    ski: str

    @staticmethod
    def generate(common_name: str = "TestEnergyGuard") -> "Identity":
        """Self-signed EC P-256 certificate with Subject Key Identifier extension (required by SHIP)."""
        key = ec.generate_private_key(ec.SECP256R1())
        name = x509.Name([
            x509.NameAttribute(NameOID.COUNTRY_NAME, "DE"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Tinkerforge"),
            x509.NameAttribute(NameOID.COMMON_NAME, common_name),
        ])
        ski = x509.SubjectKeyIdentifier.from_public_key(key.public_key())
        now = datetime.datetime.now(datetime.timezone.utc)
        cert = (x509.CertificateBuilder()
                .subject_name(name)
                .issuer_name(name)
                .public_key(key.public_key())
                .serial_number(x509.random_serial_number())
                .not_valid_before(now - datetime.timedelta(days=1))
                .not_valid_after(now + datetime.timedelta(days=3650))
                .add_extension(x509.BasicConstraints(ca=True, path_length=None), critical=True)
                .add_extension(ski, critical=False)
                .sign(key, hashes.SHA256()))
        return Identity(
            cert.public_bytes(serialization.Encoding.PEM),
            key.private_bytes(serialization.Encoding.PEM, serialization.PrivateFormat.PKCS8, serialization.NoEncryption()),
            ski.digest.hex(),
        )


def ski_from_der(der: bytes) -> str | None:
    cert = x509.load_der_x509_certificate(der)
    try:
        return cert.extensions.get_extension_for_class(x509.SubjectKeyIdentifier).value.digest.hex()
    except x509.ExtensionNotFound:
        return None


# ---------------------------------------------------------------------------
# SHIP
# ---------------------------------------------------------------------------

class ShipClient:
    """SHIP client connection: TLS websocket, CMI, hello, protocol handshake, PIN (none), access methods."""

    def __init__(self, host: str, port: int, identity: Identity, ship_id: str, log: Callable[[str], None]):
        self.host = host
        self.port = port
        self.identity = identity
        self.ship_id = ship_id
        self.log = log
        self.ws = None
        self.remote_ski: str | None = None
        self.pending_data: list[dict] = []  # data messages received during the handshake
        self._send_lock = threading.Lock()

    def connect(self, timeout: float = 30):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        ctx.check_hostname = False
        ctx.verify_mode = ssl.CERT_NONE  # SHIP trust is based on the SKI, not on a CA
        ctx.minimum_version = ssl.TLSVersion.TLSv1_2
        ctx.maximum_version = ssl.TLSVersion.TLSv1_2
        with tempfile.TemporaryDirectory() as d:
            cert_path, key_path = os.path.join(d, "cert.pem"), os.path.join(d, "key.pem")
            with open(cert_path, "wb") as f:
                f.write(self.identity.cert_pem)
            with open(key_path, "wb") as f:
                f.write(self.identity.key_pem)
            ctx.load_cert_chain(cert_path, key_path)

        self.ws = ws_connect(f"wss://{self.host}:{self.port}/ship/", ssl=ctx, subprotocols=["ship"],
                             compression=None, open_timeout=timeout, ping_interval=None, close_timeout=2, max_size=None)
        try:
            der = self.ws.socket.getpeercert(binary_form=True)
            self.remote_ski = ski_from_der(der) if der else None
        except Exception:
            self.remote_ski = None

        self._handshake(timeout)

    # -- raw messages --

    def send(self, msg_type: int, obj: dict | None):
        payload = bytes([msg_type]) + (b"\x00" if obj is None else json.dumps(obj, separators=(",", ":")).encode())
        with self._send_lock:
            self.ws.send(payload)

    def send_control(self, key: str, value: dict):
        self.send(SHIP_MSG_CONTROL, {key: to_eebus(value)})

    def send_data(self, datagram: dict):
        self.send(SHIP_MSG_DATA, {"data": [{"header": [{"protocolId": "ee1.0"}]}, {"payload": {"datagram": to_eebus(datagram)}}]})

    def recv(self, timeout: float | None) -> tuple[int, dict | None]:
        """Returns (message type, decoded JSON). Raises TimeoutError."""
        raw = self.ws.recv(timeout=timeout)
        if isinstance(raw, str):
            raw = raw.encode()
        if len(raw) == 0:
            raise PeerError("SHIP: received empty message")
        msg_type = raw[0]
        if msg_type == SHIP_MSG_INIT:
            return msg_type, {"cmi": raw[1:]}
        return msg_type, from_eebus(json.loads(raw[1:]))

    # -- handshake --

    def _recv_control(self, deadline: float, key: str) -> dict:
        while True:
            left = deadline - time.monotonic()
            if left <= 0:
                raise PeerError(f"SHIP: timeout waiting for {key}")
            msg_type, msg = self.recv(left)
            if msg_type == SHIP_MSG_END:
                raise PeerError(f"SHIP: connection closed by peer while waiting for {key}: {msg}")
            if msg_type == SHIP_MSG_DATA:
                self.pending_data.append(msg)
                continue
            if msg and "accessMethodsRequest" in msg:
                self.send_control("accessMethods", {"id": self.ship_id})
                if key != "accessMethodsRequest":
                    continue
            if msg and key in msg:
                return msg[key]
            self.log(f"SHIP: ignoring {msg} while waiting for {key}")

    def _handshake(self, timeout: float):
        deadline = time.monotonic() + timeout

        # CMI (SHIP 13.4.3)
        with self._send_lock:
            self.ws.send(b"\x00\x00")
        msg_type, msg = self.recv(timeout)
        if msg_type != SHIP_MSG_INIT or msg["cmi"] != b"\x00":
            raise PeerError(f"SHIP: unexpected CMI answer {msg_type} {msg}")

        # Hello (SHIP 13.4.4.1)
        self.send_control("connectionHello", {"phase": "ready", "waiting": 60000})
        while True:
            hello = self._recv_control(time.monotonic() + 65, "connectionHello")
            phase = hello.get("phase")
            if phase == "ready":
                break
            if phase == "aborted":
                raise PeerError("SHIP: peer aborted hello (not trusted?)")
            if hello.get("prolongationRequest"):
                self.send_control("connectionHello", {"phase": "ready", "waiting": 60000})

        # Protocol handshake (SHIP 13.4.4.2)
        announce = {"handshakeType": "announceMax", "version": {"major": 1, "minor": 0}, "formats": {"format": ["JSON-UTF8"]}}
        self.send_control("messageProtocolHandshake", announce)
        selected = self._recv_control(deadline, "messageProtocolHandshake")
        if selected.get("handshakeType") != "select":
            raise PeerError(f"SHIP: expected protocol handshake select, got {selected}")
        self.send_control("messageProtocolHandshake", {"handshakeType": "select", "version": selected.get("version", {"major": 1, "minor": 0}), "formats": {"format": ["JSON-UTF8"]}})

        # PIN verification (SHIP 13.4.4.3): Send our state right away, the WARP as server waits for our next message.
        self.send_control("connectionPinState", {"pinState": "none"})
        pin = self._recv_control(deadline, "connectionPinState")
        if pin.get("pinState") not in ("none", "pinOk"):
            raise PeerError(f"SHIP: PIN required by peer: {pin}")

        # Access methods (SHIP 13.4.6)
        self.send_control("accessMethodsRequest", {})
        self.log(f"SHIP: connected to {self.host}:{self.port}, remote SKI {self.remote_ski}")

    def close(self, reason: str = "test finished"):
        if self.ws is None:
            return
        try:
            self.send(SHIP_MSG_END, {"connectionClose": to_eebus({"phase": "announce", "maxTime": 500, "reason": reason})})
            time.sleep(0.2)
        except Exception:
            pass
        try:
            self.ws.close()
        except Exception:
            pass
        self.ws = None


# ---------------------------------------------------------------------------
# SPINE device layout
# ---------------------------------------------------------------------------

@dataclass
class FeatureSpec:
    entity: list[int]
    feature: int
    type: str
    role: str
    functions: dict[str, list[str]] = field(default_factory=dict)  # function name -> operations ("read", "write")


@dataclass
class EntitySpec:
    address: list[int]
    type: str


@dataclass
class UseCaseSpec:
    entity: list[int]
    actor: str
    name: str
    version: str = "1.0.0"
    scenarios: list[int] = field(default_factory=lambda: [1, 2, 3, 4])
    available: bool = True


@dataclass
class Layout:
    device_type: str
    entities: list[EntitySpec]
    features: list[FeatureSpec]
    use_cases: list[UseCaseSpec]
    # The SMA Sunny Home Manager omits the device in the feature addresses of its detailed discovery data
    omit_device_in_feature_addresses: bool = False


NODE_MANAGEMENT_FUNCTIONS = {
    "nodeManagementDetailedDiscoveryData": ["read"],
    "nodeManagementUseCaseData": ["read"],
    "nodeManagementSubscriptionRequestCall": ["call"],
    "nodeManagementSubscriptionDeleteCall": ["call"],
    "nodeManagementBindingRequestCall": ["call"],
    "nodeManagementBindingDeleteCall": ["call"],
}

HEARTBEAT_FUNCTIONS = {"deviceDiagnosisHeartbeatData": ["read"]}


def energy_guard_layout(use_cases: tuple[str, ...] = ("limitationOfPowerConsumption",)) -> Layout:
    """Energy Guard like the eebus-go LPC/LPP implementation: One GridGuard entity with dedicated client features."""
    e = [1]
    return Layout(
        device_type="ElectricitySupplySystem",
        entities=[EntitySpec(e, "GridGuard")],
        features=[
            FeatureSpec(e, 1, "LoadControl", "client"),
            FeatureSpec(e, 2, "DeviceConfiguration", "client"),
            FeatureSpec(e, 3, "DeviceDiagnosis", "server", dict(HEARTBEAT_FUNCTIONS)),
            FeatureSpec(e, 4, "DeviceDiagnosis", "client"),
            FeatureSpec(e, 5, "ElectricalConnection", "client"),
        ],
        use_cases=[UseCaseSpec(e, "EnergyGuard", uc) for uc in use_cases],
    )


def shm_like_layout(energy_guard_entities: int = 4) -> Layout:
    """Imitates the SMA Sunny Home Manager 2.0: Energy Guards on several entities, only Generic client features,
    heartbeat on feature 1000 and no device in the discovery feature addresses."""
    entities, features, use_cases = [], [], []
    for i in range(1, energy_guard_entities + 1):
        entities.append(EntitySpec([i], "CEM"))
        features.append(FeatureSpec([i], 1, "Generic", "client"))
        features.append(FeatureSpec([i], 1000, "DeviceDiagnosis", "server", dict(HEARTBEAT_FUNCTIONS)))
        use_cases.append(UseCaseSpec([i], "EnergyGuard", "limitationOfPowerConsumption"))
        use_cases.append(UseCaseSpec([i], "EnergyGuard", "limitationOfPowerProduction"))
        use_cases.append(UseCaseSpec([i], "MonitoringAppliance", "monitoringOfPowerConsumption"))
    return Layout("EnergyManagementSystem", entities, features, use_cases, omit_device_in_feature_addresses=True)


# ---------------------------------------------------------------------------
# SPINE peer
# ---------------------------------------------------------------------------

@dataclass
class Msg:
    time: float
    header: dict
    cmd: dict

    @property
    def classifier(self) -> str:
        return self.header.get("cmdClassifier", "")

    @property
    def function(self) -> str:
        for k in self.cmd:
            if k not in ("function", "filter", "manufacturerSpecificExtension", "lastUpdateAt"):
                return k
        return self.cmd.get("function", "")

    @property
    def data(self) -> Any:
        return self.cmd.get(self.function)

    @property
    def source(self) -> dict:
        return self.header.get("addressSource", {})

    @property
    def destination(self) -> dict:
        return self.header.get("addressDestination", {})

    @property
    def error_number(self) -> int | None:
        if self.function != "resultData":
            return None
        return (self.data or {}).get("errorNumber")

    def __str__(self):
        return f"{self.classifier} {self.function} {addr_str(self.source)} -> {addr_str(self.destination)}"


class EebusPeer:
    def __init__(self, host: str, layout: Layout | None = None, *, port: int = SHIP_PORT, identity: Identity | None = None,
                 device: str = "d:_i:Tinkerforge_TestEnergyGuard", heartbeat_interval: float = 30.0,
                 log: Callable[[str], None] = print, verbose: bool = False):
        self.host = host
        self.port = port
        self.layout = layout or energy_guard_layout()
        self.identity = identity or Identity.generate()
        self.device = device
        self.log = log
        self.verbose = verbose

        # Behaviour switches for tests
        self.heartbeat_interval = heartbeat_interval
        self.heartbeat_enabled = True    # periodic heartbeat notifications
        self.answer_heartbeat_reads = True  # reads return the last heartbeat (stale while the heartbeat is stopped)
        self.accept_subscriptions = True  # subscriptions of the remote on our features
        self.heartbeat_counter = 0
        self._heartbeat = {"timestamp": utc_timestamp(), "heartbeatCounter": 0, "heartbeatTimeout": "PT2M"}

        self.ship: ShipClient | None = None
        self.messages: list[Msg] = []  # all received SPINE messages
        self.subscriptions: list[dict] = []  # subscriptionRequests of the remote on our features
        self.bindings: list[dict] = []  # bindingRequests of the remote on our features

        self.remote_device: str | None = None
        self.remote_features: list[dict] = []
        self.remote_entities: list[dict] = []
        self.remote_use_cases: list[dict] = []

        self._msg_counter = 0
        self._cv = threading.Condition()
        self._stop = threading.Event()
        self._threads: list[threading.Thread] = []
        self.closed_by_remote = False

    # -- setup --

    def add_peer_payload(self) -> dict:
        """Payload for eebus/add. Without IP the device does not try to connect to us, we connect to it."""
        return {"ski": self.identity.ski, "ip": "", "port": 0, "trusted": True, "persistent": False, "dns_name": "", "wss_path": "/ship/"}

    def connect(self, timeout: float = 30):
        self.ship = ShipClient(self.host, self.port, self.identity, f"Tinkerforge-TestPeer-{self.identity.ski[:8]}", self.log)
        self.ship.connect(timeout)
        self._stop.clear()
        for target in (self._rx_loop, self._heartbeat_loop):
            t = threading.Thread(target=target, daemon=True)
            t.start()
            self._threads.append(t)
        for data in self.ship.pending_data:
            self._handle_data(data)
        self.ship.pending_data.clear()

    def close(self):
        self._stop.set()
        if self.ship is not None:
            self.ship.close()
        for t in self._threads:
            t.join(timeout=3)
        self._threads.clear()

    # -- addresses --

    def local(self, entity: list[int], feature: int) -> dict:
        return {"device": self.device, "entity": list(entity), "feature": feature}

    def local_feature(self, address: dict) -> FeatureSpec | None:
        if list(address.get("entity", [])) == [0] and address.get("feature") == 0:
            return FeatureSpec([0], 0, "NodeManagement", "special", dict(NODE_MANAGEMENT_FUNCTIONS))
        for f in self.layout.features:
            if f.entity == list(address.get("entity", [])) and f.feature == address.get("feature"):
                return f
        return None

    def remote(self, entity: list[int], feature: int) -> dict:
        a = {"entity": list(entity), "feature": feature}
        if self.remote_device:
            a = {"device": self.remote_device} | a
        return a

    def remote_node_management(self) -> dict:
        return self.remote([0], 0)

    # -- sending --

    def _next_counter(self) -> int:
        with self._cv:
            self._msg_counter += 1
            return self._msg_counter

    def send(self, src: dict, dst: dict, classifier: str, cmd: dict, *, ack: bool = False, ref: int | None = None) -> int:
        counter = self._next_counter()
        header: dict[str, Any] = {"specificationVersion": SPINE_SPEC_VERSION, "addressSource": src, "addressDestination": dst, "msgCounter": counter}
        if ref is not None:
            header["msgCounterReference"] = ref
        header["cmdClassifier"] = classifier
        if ack:
            header["ackRequest"] = True
        if self.verbose:
            self.log(f"SPINE TX #{counter} {Msg(0, header, cmd)}")
        self.ship.send_data({"header": header, "payload": {"cmd": [cmd]}})
        return counter

    def request(self, src: dict, dst: dict, classifier: str, cmd: dict, *, timeout: float = 10) -> Msg:
        """Sends a read, write or call and waits for the reply or result."""
        since = len(self.messages)
        counter = self.send(src, dst, classifier, cmd, ack=classifier in ("write", "call"))
        expected = ("reply", "result") if classifier == "read" else ("result",)
        return self.wait_for(lambda m: m.header.get("msgCounterReference") == counter and m.classifier in expected,
                             timeout=timeout, since=since, what=f"answer to {classifier} {Msg(0, {}, cmd).function} #{counter}")

    def read(self, src: dict, dst: dict, function: str, *, timeout: float = 10) -> Any:
        m = self.request(src, dst, "read", {function: {}}, timeout=timeout)
        if m.classifier == "result":
            raise PeerError(f"read {function} from {addr_str(dst)} failed: {m.data}")
        return m.data

    def call(self, src: dict, dst: dict, cmd: dict, *, timeout: float = 10) -> int:
        """Sends a call and returns the error number of the result."""
        return self.request(src, dst, "call", cmd, timeout=timeout).error_number

    def write(self, src: dict, dst: dict, function: str, data: dict, *, filters: list[dict] | None = None, partial: bool = True, timeout: float = 10) -> int:
        """Sends a write and returns the error number of the result."""
        filters = list(filters or [])
        if partial:
            filters.append({"cmdControl": {"partial": {}}})
        cmd: dict[str, Any] = {"function": function}
        if filters:
            cmd["filter"] = filters
        cmd[function] = data
        return self.request(src, dst, "write", cmd, timeout=timeout).error_number

    # -- waiting --

    def wait_for(self, predicate: Callable[[Msg], bool], *, timeout: float = 10, since: int = 0, what: str = "message") -> Msg:
        deadline = time.monotonic() + timeout
        with self._cv:
            idx = since
            while True:
                while idx < len(self.messages):
                    m = self.messages[idx]
                    idx += 1
                    if predicate(m):
                        return m
                left = deadline - time.monotonic()
                if left <= 0:
                    raise PeerError(f"Timeout after {timeout} s waiting for {what}")
                self._cv.wait(left)

    def wait_until(self, condition: Callable[[], bool], *, timeout: float = 10, what: str = "condition"):
        deadline = time.monotonic() + timeout
        with self._cv:
            while not condition():
                left = deadline - time.monotonic()
                if left <= 0:
                    raise PeerError(f"Timeout after {timeout} s waiting for {what}")
                self._cv.wait(left)

    def received(self, classifier: str | None = None, function: str | None = None, since: int = 0) -> list[Msg]:
        with self._cv:
            return [m for m in self.messages[since:] if (classifier is None or m.classifier == classifier) and (function is None or m.function == function)]

    # -- receiving --

    def _rx_loop(self):
        while not self._stop.is_set():
            try:
                msg_type, msg = self.ship.recv(0.5)
            except TimeoutError:
                continue
            except ConnectionClosed:
                if not self._stop.is_set():
                    self.log("SHIP: connection closed by remote")
                    self.closed_by_remote = True
                with self._cv:
                    self._cv.notify_all()
                return
            except Exception as e:
                if not self._stop.is_set():
                    self.log(f"SHIP: receive error: {e!r}")
                return
            try:
                if msg_type == SHIP_MSG_DATA:
                    self._handle_data(msg)
                elif msg_type == SHIP_MSG_CONTROL:
                    if msg and "accessMethodsRequest" in msg:
                        self.ship.send_control("accessMethods", {"id": self.ship.ship_id})
                elif msg_type == SHIP_MSG_END:
                    close = (msg or {}).get("connectionClose", {})
                    self.log(f"SHIP: connectionClose {close}")
                    if close.get("phase") == "announce":
                        self.ship.send(SHIP_MSG_END, {"connectionClose": to_eebus({"phase": "confirm"})})
                    self.closed_by_remote = True
            except Exception as e:
                self.log(f"SPINE: error handling {msg}: {e!r}")

    def _handle_data(self, msg: dict):
        datagram = (msg.get("data", {}).get("payload", {}) or {}).get("datagram", {})
        header = datagram.get("header", {})
        cmds = as_list((datagram.get("payload", {}) or {}).get("cmd"))
        for cmd in cmds:
            m = Msg(time.monotonic(), header, cmd if isinstance(cmd, dict) else {})
            if self.verbose:
                self.log(f"SPINE RX #{header.get('msgCounter')} {m}")
            self._dispatch(m)
            with self._cv:
                self.messages.append(m)
                self._cv.notify_all()

    def _answer(self, m: Msg, classifier: str, cmd: dict):
        src = {"device": self.device, "entity": m.destination.get("entity"), "feature": m.destination.get("feature")}
        self.send(src, m.source, classifier, cmd, ref=m.header.get("msgCounter"))

    def _result(self, m: Msg, error_number: int, description: str | None = None):
        data: dict[str, Any] = {"errorNumber": error_number}
        if description:
            data["description"] = description
        self._answer(m, "result", {"resultData": data})

    def _dispatch(self, m: Msg):
        if self.remote_device is None and m.source.get("device"):
            self.remote_device = m.source["device"]

        classifier, function = m.classifier, m.function
        if classifier == "read":
            reply = self._read_reply(m)
            if reply is None:
                self._result(m, ERROR_COMMAND_NOT_SUPPORTED, f"{function} not supported")
            elif reply is not False:
                self._answer(m, "reply", {function: reply})
            return

        if classifier == "call":
            self._result(m, *self._handle_call(m))
            return

        if classifier == "write":
            self._result(m, ERROR_COMMAND_NOT_SUPPORTED, "no writable functions")
            return

        if classifier in ("reply", "notify") and m.header.get("ackRequest"):
            self._result(m, ERROR_NO_ERROR)

    def _read_reply(self, m: Msg) -> Any:
        """Returns the reply data, None for "not supported", False for "do not answer"."""
        feature = self.local_feature(m.destination)
        if feature is None or m.function not in feature.functions:
            return None
        if m.function == "nodeManagementDetailedDiscoveryData":
            return self.detailed_discovery_data()
        if m.function == "nodeManagementUseCaseData":
            return self.use_case_data()
        if m.function == "deviceDiagnosisHeartbeatData":
            return self.heartbeat_data() if self.answer_heartbeat_reads else False
        return None

    def _handle_call(self, m: Msg) -> tuple[int, str | None]:
        data = m.data or {}
        if m.function == "nodeManagementSubscriptionRequestCall":
            req = data.get("subscriptionRequest", {})
            if not self.accept_subscriptions:
                return ERROR_COMMAND_REJECTED, "subscriptions disabled by test"
            if self.local_feature(req.get("serverAddress", {})) is None:
                return ERROR_GENERAL, "unknown server feature"
            with self._cv:
                self.subscriptions.append(req)
                self._cv.notify_all()
            self.log(f"SPINE: {addr_str(req.get('clientAddress'))} subscribed to {addr_str(req.get('serverAddress'))} ({req.get('serverFeatureType')})")
            return ERROR_NO_ERROR, None
        if m.function == "nodeManagementSubscriptionDeleteCall":
            req = data.get("subscriptionDelete", {})
            with self._cv:
                self.subscriptions = [s for s in self.subscriptions if not (same_feature(s.get("clientAddress"), req.get("clientAddress")) and same_feature(s.get("serverAddress"), req.get("serverAddress")))]
            return ERROR_NO_ERROR, None
        if m.function == "nodeManagementBindingRequestCall":
            with self._cv:
                self.bindings.append(data.get("bindingRequest", {}))
                self._cv.notify_all()
            return ERROR_NO_ERROR, None
        if m.function == "nodeManagementBindingDeleteCall":
            req = data.get("bindingDelete", {})
            with self._cv:
                self.bindings = [b for b in self.bindings if not (same_feature(b.get("clientAddress"), req.get("clientAddress")) and same_feature(b.get("serverAddress"), req.get("serverAddress")))]
            return ERROR_NO_ERROR, None
        return ERROR_COMMAND_NOT_SUPPORTED, f"{m.function} not supported"

    # -- our data --

    def detailed_discovery_data(self) -> dict:
        L = self.layout

        def feature_address(f: FeatureSpec) -> dict:
            a = {"entity": f.entity, "feature": f.feature}
            return a if L.omit_device_in_feature_addresses else {"device": self.device} | a

        features = [FeatureSpec([0], 0, "NodeManagement", "special", dict(NODE_MANAGEMENT_FUNCTIONS))] + L.features
        feature_information = []
        for f in features:
            desc: dict[str, Any] = {"featureAddress": feature_address(f), "featureType": f.type, "role": f.role}
            if f.functions:
                desc["supportedFunction"] = [{"function": fn, "possibleOperations": {op: {} for op in ops if op != "call"}} for fn, ops in f.functions.items()]
            feature_information.append({"description": desc})

        return {
            "specificationVersionList": {"specificationVersion": [SPINE_SPEC_VERSION]},
            "deviceInformation": {"description": {"deviceAddress": {"device": self.device}, "deviceType": L.device_type, "networkFeatureSet": "smart"}},
            "entityInformation": [{"description": {"entityAddress": {"device": self.device, "entity": e.address}, "entityType": e.type}}
                                  for e in [EntitySpec([0], "DeviceInformation")] + L.entities],
            "featureInformation": feature_information,
        }

    def use_case_data(self) -> dict:
        infos: dict[tuple, dict] = {}
        for uc in self.layout.use_cases:
            key = (tuple(uc.entity), uc.actor)
            info = infos.setdefault(key, {"address": {"device": self.device, "entity": uc.entity}, "actor": uc.actor, "useCaseSupport": []})
            info["useCaseSupport"].append({"useCaseName": uc.name, "useCaseVersion": uc.version, "useCaseAvailable": uc.available,
                                           "scenarioSupport": uc.scenarios, "useCaseDocumentSubRevision": "release"})
        return {"useCaseInformation": list(infos.values())}

    def heartbeat_data(self) -> dict:
        """The last heartbeat. Like eebus-go, reads return unchanged data while the heartbeat is stopped."""
        return dict(self._heartbeat)

    # -- heartbeat --

    def heartbeat_subscribers(self) -> list[dict]:
        with self._cv:
            return [s for s in self.subscriptions if (f := self.local_feature(s.get("serverAddress", {}))) is not None and f.type == "DeviceDiagnosis"]

    def send_heartbeat(self):
        """Increments the heartbeat counter and notifies all heartbeat subscribers."""
        self.heartbeat_counter += 1
        self._heartbeat = {"timestamp": utc_timestamp(), "heartbeatCounter": self.heartbeat_counter, "heartbeatTimeout": "PT2M"}
        for sub in self.heartbeat_subscribers():
            server = sub["serverAddress"]
            self.send(self.local(server["entity"], server["feature"]), sub["clientAddress"], "notify", {"deviceDiagnosisHeartbeatData": self.heartbeat_data()})

    def _heartbeat_loop(self):
        while not self._stop.wait(self.heartbeat_interval):
            if self.heartbeat_enabled and not self.closed_by_remote:
                try:
                    self.send_heartbeat()
                except Exception as e:
                    self.log(f"SPINE: heartbeat failed: {e!r}")

    def wait_for_heartbeat_subscription(self, timeout: float = 20) -> dict:
        self.wait_until(lambda: len(self.heartbeat_subscribers()) > 0, timeout=timeout, what="heartbeat subscription of the remote")
        return self.heartbeat_subscribers()[0]

    # -- remote discovery --

    def discover(self, timeout: float = 10):
        """Reads the detailed discovery data and use cases of the remote device."""
        src = self.local([0], 0)
        data = self.read(src, self.remote_node_management(), "nodeManagementDetailedDiscoveryData", timeout=timeout)
        device = ((data.get("deviceInformation") or {}).get("description") or {}).get("deviceAddress", {}).get("device")
        if device:
            self.remote_device = device
        self.remote_entities = [e.get("description", {}) for e in as_list(data.get("entityInformation"))]
        self.remote_features = [f.get("description", {}) for f in as_list(data.get("featureInformation"))]
        data = self.read(src, self.remote_node_management(), "nodeManagementUseCaseData", timeout=timeout)
        self.remote_use_cases = as_list(data.get("useCaseInformation"))

    def find_remote_use_case(self, actor: str, name: str) -> dict | None:
        for info in self.remote_use_cases:
            if info.get("actor") != actor:
                continue
            for support in as_list(info.get("useCaseSupport")):
                if support.get("useCaseName") == name:
                    return info
        return None

    def find_remote_feature(self, feature_type: str, role: str, entity: list[int] | None = None) -> dict:
        for f in self.remote_features:
            a = f.get("featureAddress", {})
            if f.get("featureType") == feature_type and f.get("role") == role and (entity is None or list(a.get("entity", [])) == list(entity)):
                return self.remote(a["entity"], a["feature"])
        raise PeerError(f"remote has no {feature_type} {role} feature" + (f" on entity {entity}" if entity else ""))

    # -- node management --

    def subscribe(self, client: dict, server: dict, server_feature_type: str) -> int:
        req = {"clientAddress": client, "serverAddress": server, "serverFeatureType": server_feature_type}
        return self.call(client, self.remote_node_management(), {"nodeManagementSubscriptionRequestCall": {"subscriptionRequest": req}})

    def bind(self, client: dict, server: dict, server_feature_type: str) -> int:
        req = {"clientAddress": client, "serverAddress": server, "serverFeatureType": server_feature_type}
        return self.call(client, self.remote_node_management(), {"nodeManagementBindingRequestCall": {"bindingRequest": req}})

    def unbind(self, client: dict, server: dict) -> int:
        req = {"clientAddress": client, "serverAddress": server}
        return self.call(client, self.remote_node_management(), {"nodeManagementBindingDeleteCall": {"bindingDelete": req}})


# ---------------------------------------------------------------------------
# LPC Energy Guard
# ---------------------------------------------------------------------------

class LpcEnergyGuard:
    """Energy Guard side of the LPC use case on top of an EebusPeer."""

    def __init__(self, peer: EebusPeer, entity: list[int] = [1], lc_client: int | None = None, dc_client: int | None = None):
        self.peer = peer
        self.entity = list(entity)
        features = [f for f in peer.layout.features if f.entity == self.entity and f.role == "client"]

        def pick(t: str) -> int:
            for f in features:
                if f.type == t:
                    return f.feature
            for f in features:
                if f.type == "Generic":
                    return f.feature
            raise PeerError(f"layout has no {t} or Generic client on entity {entity}")

        self.lc_client = peer.local(self.entity, lc_client if lc_client is not None else pick("LoadControl"))
        self.dc_client = peer.local(self.entity, dc_client if dc_client is not None else pick("DeviceConfiguration"))
        ec_clients = [f.feature for f in features if f.type in ("ElectricalConnection", "Generic")]
        self.ec_client = peer.local(self.entity, ec_clients[0]) if ec_clients else None
        self.lc_server: dict | None = None
        self.dc_server: dict | None = None
        self.ec_server: dict | None = None
        self.limit_id: int | None = None
        self.failsafe_power_key: int | None = None
        self.failsafe_duration_key: int | None = None

    def find_servers(self):
        uc = self.peer.find_remote_use_case("ControllableSystem", "limitationOfPowerConsumption")
        if uc is None:
            raise PeerError("remote does not announce LPC as ControllableSystem")
        entity = uc.get("address", {}).get("entity")
        self.lc_server = self.peer.find_remote_feature("LoadControl", "server", entity)
        self.dc_server = self.peer.find_remote_feature("DeviceConfiguration", "server", entity)
        self.ec_server = self.peer.find_remote_feature("ElectricalConnection", "server", entity)

    def setup(self, *, subscribe: bool = True, bind: bool = True):
        """Finds the LPC features of the remote, subscribes, binds and reads the limit and key ids."""
        if not self.peer.remote_features:
            self.peer.discover()
        self.find_servers()
        for client, server, t in ((self.lc_client, self.lc_server, "LoadControl"), (self.dc_client, self.dc_server, "DeviceConfiguration")):
            if subscribe:
                err = self.peer.subscribe(client, server, t)
                if err != ERROR_NO_ERROR:
                    raise PeerError(f"subscription on {t} failed with error {err}")
            if bind:
                err = self.peer.bind(client, server, t)
                if err != ERROR_NO_ERROR:
                    raise PeerError(f"binding on {t} failed with error {err}")
        self.read_ids()

    def read_ids(self):
        desc = self.peer.read(self.lc_client, self.lc_server, "loadControlLimitDescriptionListData")
        for d in as_list(desc.get("loadControlLimitDescriptionData")):
            if d.get("limitDirection") == "consume" and d.get("scopeType") == "activePowerLimit":
                self.limit_id = d.get("limitId")
        if self.limit_id is None:
            raise PeerError(f"no consumption limit in {desc}")

        desc = self.peer.read(self.dc_client, self.dc_server, "deviceConfigurationKeyValueDescriptionListData")
        for d in as_list(desc.get("deviceConfigurationKeyValueDescriptionData")):
            if d.get("keyName") == "failsafeConsumptionActivePowerLimit":
                self.failsafe_power_key = d.get("keyId")
            elif d.get("keyName") == "failsafeDurationMinimum":
                self.failsafe_duration_key = d.get("keyId")
        if self.failsafe_power_key is None or self.failsafe_duration_key is None:
            raise PeerError(f"failsafe keys missing in {desc}")

    def write_limit(self, value: float | None = None, *, active: bool | None = None, duration: int | str | None = None,
                    delete_duration: bool = False, limit_id: int | None = None) -> int:
        """Writes the consumption limit. duration in seconds (or as ISO 8601 string). Returns the SPINE error number."""
        limit_id = self.limit_id if limit_id is None else limit_id
        data: dict[str, Any] = {"limitId": limit_id}
        if active is not None:
            data["isLimitActive"] = active
        if duration is not None:
            data["timePeriod"] = {"endTime": duration if isinstance(duration, str) else iso_duration(duration)}
        if value is not None:
            data["value"] = scaled_number(value)
        filters = []
        if delete_duration:
            filters.append({"cmdControl": {"delete": {}},
                            "loadControlLimitListDataSelectors": {"limitId": limit_id},
                            "loadControlLimitDataElements": {"timePeriod": {}}})
        return self.peer.write(self.lc_client, self.lc_server, "loadControlLimitListData", {"loadControlLimitData": [data]}, filters=filters)

    def write_failsafe(self, power: float | None = None, duration: int | str | None = None) -> int:
        """Writes the failsafe values. duration in seconds (or as ISO 8601 string). Returns the SPINE error number."""
        entries = []
        if power is not None:
            entries.append({"keyId": self.failsafe_power_key, "value": {"scaledNumber": scaled_number(power)}})
        if duration is not None:
            entries.append({"keyId": self.failsafe_duration_key, "value": {"duration": duration if isinstance(duration, str) else iso_duration(duration)}})
        return self.peer.write(self.dc_client, self.dc_server, "deviceConfigurationKeyValueListData", {"deviceConfigurationKeyValueData": entries})

    def read_limit(self) -> dict:
        data = self.peer.read(self.lc_client, self.lc_server, "loadControlLimitListData")
        for d in as_list(data.get("loadControlLimitData")):
            if d.get("limitId") == self.limit_id:
                return d
        raise PeerError(f"limit {self.limit_id} missing in {data}")

    def read_constraints(self) -> dict:
        """Scenario 4: Returns the characteristics of the remote by characteristicType."""
        data = self.peer.read(self.ec_client, self.ec_server, "electricalConnectionCharacteristicListData")
        return {d.get("characteristicType"): d for d in as_list((data or {}).get("electricalConnectionCharacteristicData"))}

    def read_failsafe(self) -> dict:
        data = self.peer.read(self.dc_client, self.dc_server, "deviceConfigurationKeyValueListData")
        return {d.get("keyId"): d.get("value") for d in as_list(data.get("deviceConfigurationKeyValueData"))}


# ---------------------------------------------------------------------------
# Manual smoke test: uv run --group tests _eebus_peer.py <host>
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    import sys
    import urllib.request

    host = sys.argv[1]

    def api(path, payload=None):
        req = urllib.request.Request(f"http://{host}/{path}", data=None if payload is None else json.dumps(payload).encode(),
                                     method="GET" if payload is None else "PUT", headers={"Content-Type": "application/json"})
        with urllib.request.urlopen(req, timeout=5) as r:
            body = r.read()
            return json.loads(body) if payload is None and body else None

    peer = EebusPeer(host, energy_guard_layout(), verbose=True, log=lambda s: print(time.strftime("%H:%M:%S"), s, flush=True))
    print("local SKI", peer.identity.ski, "device SKI", api("eebus/state")["ski"])
    api("eebus/add", peer.add_peer_payload())
    try:
        peer.connect()
        peer.discover()
        print("remote device", peer.remote_device)
        print("heartbeat subscription", peer.wait_for_heartbeat_subscription(30))
        lpc = LpcEnergyGuard(peer)
        lpc.setup()
        print("limit id", lpc.limit_id, "failsafe keys", lpc.failsafe_power_key, lpc.failsafe_duration_key)
        print("limit", lpc.read_limit(), "failsafe", lpc.read_failsafe())
        print("write deactivated 0 W ->", lpc.write_limit(0, active=False))
        time.sleep(1.5)
        print("lpc state", api("eebus/usecases")["lpc"])
    finally:
        peer.close()
        api("eebus/remove", {"ski": peer.identity.ski})
