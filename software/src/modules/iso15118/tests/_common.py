from __future__ import annotations

import asyncio
import base64
import contextlib
import json
import os
from pathlib import Path
import queue
import socket
import ssl
import struct
import subprocess
import tempfile
import threading
import time
import uuid
from typing import TYPE_CHECKING
import urllib.request

if TYPE_CHECKING:
    from software.test_runner.test_context import TestContext


TEST_DIR = Path(__file__).resolve().parent
ISO_DIR = TEST_DIR.parent
TOOLS_DIR = ISO_DIR / "tools"
CERTS_DIR = TOOLS_DIR / "certs" / "output"

SDP_SECURITY_TLS = 0x00
SDP_SECURITY_NO_TLS = 0x10

ISO2 = {
    "ProtocolNamespace": "urn:iso:15118:2:2013:MsgDef",
    "VersionNumberMajor": 2,
    "VersionNumberMinor": 0,
    "SchemaID": 1,
    "Priority": 2,
}
ISO20_AC = {
    "ProtocolNamespace": "urn:iso:std:iso:15118:-20:AC",
    "VersionNumberMajor": 1,
    "VersionNumberMinor": 0,
    "SchemaID": 2,
    "Priority": 1,
}

V2GTP_ISO20_COMMON = 0x8002
V2GTP_ISO20_AC = 0x8003


def route_interface(host: str) -> str:
    result = subprocess.run(
        ["ip", "route", "get", host],
        check=True,
        capture_output=True,
        text=True,
    ).stdout
    try:
        return result.split(" dev ", 1)[1].split()[0]
    except IndexError as e:
        raise RuntimeError(f"No interface in route to {host}: {result.strip()}") from e


def local_ip_towards(host: str) -> str:
    result = subprocess.run(
        ["ip", "route", "get", host],
        check=True,
        capture_output=True,
        text=True,
    ).stdout
    try:
        return result.split(" src ", 1)[1].split()[0]
    except IndexError as e:
        raise RuntimeError(f"No source address in route to {host}: {result.strip()}") from e


def api_get(host: str, path: str):
    with urllib.request.urlopen(f"http://{host}/{path}", timeout=5) as response:
        return json.loads(response.read())


def api_put(host: str, path: str, payload):
    request = urllib.request.Request(
        f"http://{host}/{path}",
        data=json.dumps(payload).encode(),
        headers={"Content-Type": "application/json"},
        method="PUT",
    )
    with urllib.request.urlopen(request, timeout=5) as response:
        return response.read().decode()


def enable_debug_mode(host: str):
    api_put(host, "iso15118/debug_update", {"enable": True, "current": 6000, "phases": 3})


def disable_debug_mode(host: str):
    api_put(host, "iso15118/debug_update", {"enable": False, "current": 6000, "phases": 3})


def require_iso_tls_config(host: str):
    if not api_get(host, "iso15118/config")["charge_via_iso15118"]:
        raise SystemExit("iso15118/config charge_via_iso15118 is disabled")


def sdp_request(
    iface: str,
    security: int = SDP_SECURITY_TLS,
    timeout: float = 3,
    expected_from: str | None = None,
):
    ifindex = socket.if_nametoindex(iface)
    payload = struct.pack("!BB", security, 0x00)
    request = struct.pack("!BBHI", 0x01, 0xFE, 0x9000, len(payload)) + payload

    with socket.socket(socket.AF_INET6, socket.SOCK_DGRAM) as sock:
        sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_MULTICAST_IF, ifindex)
        sock.settimeout(timeout)
        sock.sendto(request, ("ff02::1", 15118, 0, ifindex))
        deadline = time.monotonic() + timeout
        while True:
            try:
                data, addr = sock.recvfrom(64)
            except TimeoutError:
                return None
            if expected_from is None or addr[0].split("%", 1)[0].lower() == expected_from.lower():
                break
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                return None
            sock.settimeout(remaining)

    if len(data) != 28:
        raise ValueError(f"Unexpected SDP response length {len(data)}: {data.hex()}")
    ip_words = struct.unpack("<4I", data[8:24])
    ip_bytes = b"".join(struct.pack("<I", word) for word in ip_words)
    port, response_security, transport = struct.unpack("!HBB", data[24:28])
    return {
        "from": addr[0],
        "secc_ll": socket.inet_ntop(socket.AF_INET6, ip_bytes),
        "port": port,
        "security": response_security,
        "transport": transport,
    }


def connect_secc(host: str, iface: str | None = None, timeout: float = 20):
    iface = iface or route_interface(host)
    response = sdp_request(iface)
    if response is None:
        raise TimeoutError("ISO 15118 SDP request timed out")
    ifindex = socket.if_nametoindex(iface)
    sock = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    sock.connect((response["secc_ll"], response["port"], 0, ifindex))
    return sock


class IsoTestEnvironment:
    def __init__(self, tc: TestContext):
        self.tc = tc
        if tc._esp_host is None:
            tc.skip("ESP Host not passed")
        self.host = tc._esp_host
        self.iface = route_interface(self.host)
        self.secc_ll = None
        self.saved_config = None
        self.saved_debug = None

    def start(self):
        if not self.tc.device_type().is_warp(4):
            self.tc.skip("ISO 15118 debug tests require a WARP4")
        self.saved_config = self.tc.api("iso15118/config")
        if not self.saved_config["charge_via_iso15118"]:
            config = dict(self.saved_config)
            config["charge_via_iso15118"] = True
            self.tc.api("iso15118/config", config, timeout=5)
        self.saved_debug = self.tc.api("iso15118/debug")
        if not self.saved_debug["enable"]:
            self.tc.api("iso15118/debug_update", {
                "enable": True,
                "current": self.saved_debug["current"],
                "phases": self.saved_debug["phases"],
            }, timeout=5)
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            response = sdp_request(self.iface, timeout=1, expected_from=self.secc_ll)
            if response is not None:
                self.secc_ll = response["secc_ll"]
                return
        raise TimeoutError("ISO 15118 debug SDP did not become ready")

    def stop(self):
        errors = []
        if self.saved_debug is not None:
            try:
                self.tc.api("iso15118/debug_update", self.saved_debug, timeout=15)
            except Exception as e:
                errors.append(e)
        if self.saved_config is not None:
            try:
                self.tc.api("iso15118/config", self.saved_config, timeout=15)
            except Exception as e:
                errors.append(e)
        if errors:
            raise errors[0]

    def reset_session(self):
        current = self.tc.api("iso15118/debug")
        self.tc.api("iso15118/debug_update", {
            "enable": False,
            "current": current["current"],
            "phases": current["phases"],
        }, timeout=5)
        time.sleep(0.2)
        self.tc.api("iso15118/debug_update", {
            "enable": True,
            "current": current["current"],
            "phases": current["phases"],
        }, timeout=5)
        deadline = time.monotonic() + 10
        while time.monotonic() < deadline:
            if sdp_request(self.iface, timeout=1, expected_from=self.secc_ll) is not None:
                return
        raise TimeoutError("ISO 15118 debug SDP did not restart")


class EVTestClient:
    def __init__(self, host: str, iface: str, secc_ll: str | None = None):
        self.host = host
        self.iface = iface
        self.secc_ll = secc_ll
        self._codec = None

    def codec(self):
        if self._codec is None:
            from iso15118.shared.exificient_exi_codec import ExificientEXICodec
            from iso15118.shared.settings import load_shared_settings

            load_shared_settings()
            self._codec = ExificientEXICodec()
        return self._codec

    def connect_raw(self, timeout: float = 30):
        response = sdp_request(self.iface, expected_from=self.secc_ll)
        if response is None:
            raise TimeoutError("ISO 15118 SDP request timed out")
        ifindex = socket.if_nametoindex(self.iface)
        sock = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        sock.settimeout(timeout)
        sock.connect((response["secc_ll"], response["port"], 0, ifindex))
        return sock

    def tls12_context(self):
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        context.maximum_version = ssl.TLSVersion.TLSv1_2
        context.set_ciphers("ECDHE-ECDSA-AES128-SHA256")
        context.check_hostname = False
        context.verify_mode = ssl.CERT_REQUIRED
        context.load_verify_locations(cafile=str(CERTS_DIR / "iso2/certs/v2gRootCACert.pem"))
        return context

    def tls13_context(self):
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        context.minimum_version = ssl.TLSVersion.TLSv1_3
        context.check_hostname = False
        context.verify_mode = ssl.CERT_REQUIRED
        context.load_verify_locations(cafile=str(CERTS_DIR / "iso20/certs/v2gRootCACert.pem"))
        context.load_cert_chain(
            certfile=str(CERTS_DIR / "iso20/certs/oemCertChain.pem"),
            keyfile=str(CERTS_DIR / "iso20/private_keys/oemLeaf.key"),
            password="12345",
        )
        return context

    def connect_tls(self, context: ssl.SSLContext, session=None):
        return context.wrap_socket(self.connect_raw(), session=session)

    @staticmethod
    def _recv_exact(sock, size: int):
        data = bytearray()
        while len(data) < size:
            chunk = sock.recv(size - len(data))
            if not chunk:
                raise ConnectionError(f"Connection closed after {len(data)} of {size} bytes")
            data.extend(chunk)
        return bytes(data)

    def exchange(self, sock, request, namespace, payload_type: int = 0x8001):
        payload = self.codec().encode(json.dumps(request), namespace)
        sock.sendall(struct.pack("!BBHI", 0x01, 0xFE, payload_type, len(payload)) + payload)
        header = self._recv_exact(sock, 8)
        if header[:2] != b"\x01\xfe":
            raise ValueError(f"Unexpected V2GTP header: {header.hex()}")
        response_payload_type = struct.unpack("!H", header[2:4])[0]
        if response_payload_type != payload_type:
            raise ValueError(
                f"Unexpected V2GTP payload type 0x{response_payload_type:04x}, "
                f"expected 0x{payload_type:04x}"
            )
        payload_len = struct.unpack("!I", header[4:8])[0]
        return json.loads(self.codec().decode(self._recv_exact(sock, payload_len), namespace))

    def sap(self, sock, protocols):
        from iso15118.shared.messages.enums import Namespace

        response = self.exchange(
            sock,
            {"supportedAppProtocolReq": {"AppProtocol": protocols}},
            Namespace.SAP,
        )
        return response["supportedAppProtocolRes"]


class CSMSSim:
    def __init__(
        self,
        port: int = 0,
        interactive=(),
        *,
        certfile: str | None = None,
        keyfile: str | None = None,
        expected_basic_auth: tuple[str, str] | None = None,
    ):
        from websockets.sync.server import serve

        self.interactive = set(interactive)
        self.expected_basic_auth = expected_basic_auth
        self.requests = queue.Queue()
        self.pending_requests = []
        self.responses = queue.Queue()
        self.security_events = []
        self.connected = threading.Event()
        self.authorization = None
        self.current_time_offset_s = 0
        self.ws = None
        ssl_context = None
        if certfile is not None:
            ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
            ssl_context.load_cert_chain(certfile, keyfile)
        self.server = serve(
            self._handler,
            "0.0.0.0",
            port,
            ssl=ssl_context,
            process_request=self._process_request if expected_basic_auth is not None else None,
            select_subprotocol=lambda connection, protocols: "ocpp2.1",
        )
        self.port = self.server.socket.getsockname()[1]
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()

    def _process_request(self, connection, request):
        from websockets.http11 import Response

        self.authorization = request.headers.get("Authorization")
        user, password = self.expected_basic_auth
        expected = "Basic " + base64.b64encode(f"{user}:{password}".encode()).decode()
        if self.authorization == expected:
            return None
        return Response(401, "Unauthorized", request.headers, b"")

    def _handler(self, ws):
        self.ws = ws
        self.connected.set()
        try:
            for raw in ws:
                message = json.loads(raw)
                if message[0] == 2:
                    _, message_id, action, payload = message
                    if action in self.interactive:
                        self.requests.put((action, payload, message_id))
                    elif action == "BootNotification":
                        self.respond(message_id, {
                            "currentTime": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime(time.time() + self.current_time_offset_s)),
                            "interval": 300,
                            "status": "Accepted",
                        })
                    elif action == "Heartbeat":
                        self.respond(message_id, {
                            "currentTime": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime(time.time() + self.current_time_offset_s)),
                        })
                    elif action == "SecurityEventNotification":
                        self.security_events.append(payload)
                        self.respond(message_id, {})
                    else:
                        self.respond(message_id, {})
                elif message[0] == 3:
                    self.responses.put((message[1], message[2]))
        finally:
            self.connected.clear()

    def respond(self, message_id, payload):
        if self.ws is None:
            raise ConnectionError("CSMS has no connected charging station")
        self.ws.send(json.dumps([3, message_id, payload]))

    def call(self, action, payload, timeout: float = 30):
        if self.ws is None:
            raise ConnectionError("CSMS has no connected charging station")
        message_id = str(uuid.uuid4())
        self.ws.send(json.dumps([2, message_id, action, payload]))
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            try:
                response_id, response = self.responses.get(timeout=deadline - time.monotonic())
            except queue.Empty:
                break
            if response_id == message_id:
                return response
        raise TimeoutError(f"No response to {action}")

    def expect(self, action, timeout: float = 60):
        return self.expect_any({action}, timeout)[1:]

    def expect_any(self, actions, timeout: float = 60):
        deadline = time.monotonic() + timeout
        for index, request in enumerate(self.pending_requests):
            if request[0] in actions:
                return self.pending_requests.pop(index)
        while time.monotonic() < deadline:
            try:
                request = self.requests.get(timeout=deadline - time.monotonic())
            except queue.Empty:
                break
            if request[0] in actions:
                return request
            self.pending_requests.append(request)
        raise TimeoutError(f"No request for {', '.join(sorted(actions))}")

    def stop(self):
        self.server.shutdown()
        self.thread.join(timeout=5)


class LocalCSMSTls:
    def __init__(self, charger: str, server_ip: str):
        self.charger = charger
        self.cert_id = None
        self._tmpdir = tempfile.TemporaryDirectory(prefix="iso15118-csms-tls-")
        directory = Path(self._tmpdir.name)
        ca_key = directory / "ca-key.pem"
        ca_cert = directory / "ca.pem"
        server_csr = directory / "server.csr"
        self.certfile = directory / "server.pem"
        self.keyfile = directory / "server-key.pem"
        ext = directory / "server-ext.cnf"

        try:
            subprocess.run([
                "openssl", "req", "-x509", "-newkey", "ec",
                "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
                "-keyout", ca_key, "-out", ca_cert, "-days", "365",
                "-subj", "/CN=ISO 15118 test CSMS CA",
                "-addext", "basicConstraints=critical,CA:TRUE",
            ], check=True, capture_output=True)
            subprocess.run([
                "openssl", "req", "-newkey", "ec",
                "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
                "-keyout", self.keyfile, "-out", server_csr,
                "-subj", "/CN=iso15118-test-csms",
            ], check=True, capture_output=True)
            ext.write_text(f"subjectAltName=IP:{server_ip}\n")
            subprocess.run([
                "openssl", "x509", "-req", "-in", server_csr,
                "-CA", ca_cert, "-CAkey", ca_key, "-CAcreateserial",
                "-out", self.certfile, "-days", "365", "-sha256", "-extfile", ext,
            ], check=True, capture_output=True)

            used = {cert["id"] for cert in api_get(charger, "certs/state")["certs"]}
            self.cert_id = next((candidate for candidate in range(7, -1, -1) if candidate not in used), None)
            if self.cert_id is None:
                raise RuntimeError("ISO 15118 certificate tests need one free TLS certificate slot")
            api_put(charger, "certs/add", {
                "id": self.cert_id,
                "name": "ISO 15118 test CSMS",
                "cert": ca_cert.read_text(),
            })
        except Exception:
            self.close()
            raise

    def close(self):
        if self.cert_id is not None:
            api_put(self.charger, "certs/remove", {"id": self.cert_id})
            self.cert_id = None
        if self._tmpdir is not None:
            self._tmpdir.cleanup()
            self._tmpdir = None


class EVSim:
    def __init__(
        self,
        iface: str,
        protocol: str = "ISO_15118_20_AC",
        charge_loop_cycles: int = 1,
        *,
        iso20_soc: int | None = None,
        iso20_capacity_wh: int | None = None,
        iso20_power_w: int | None = None,
        pause_after_charge_loop: bool = False,
    ):
        self.iface = iface
        self.protocol = protocol
        self.charge_loop_cycles = charge_loop_cycles
        self.iso20_soc = iso20_soc
        self.iso20_capacity_wh = iso20_capacity_wh
        self.iso20_power_w = iso20_power_w
        self.pause_after_charge_loop = pause_after_charge_loop
        self.charge_loop_reached = threading.Event()
        self._charge_loop_resume = threading.Event()
        self._pki = None

    async def _run(self):
        from iso15118.evcc import EVCCHandler
        from iso15118.evcc.controller.simulator import SimEVController
        from iso15118.evcc.evcc_config import EVCCConfig
        from iso15118.shared.exificient_exi_codec import ExificientEXICodec
        from iso15118.shared.messages.iso15118_20.common_types import RationalNumber
        from iso15118.shared.settings import load_shared_settings

        use_tls13 = self.protocol.startswith("ISO_15118_20")
        self._pki = self._create_pki(use_tls13)
        os.environ["NETWORK_INTERFACE"] = self.iface
        os.environ["PKI_PATH"] = str(self._pki)
        os.environ["ENABLE_TLS_1_3"] = "true" if use_tls13 else "false"
        load_shared_settings()

        if use_tls13:
            from iso15118.evcc.transport import tcp_client
            from iso15118.shared import security

            original = security.get_ssl_context

            def get_ssl_context(server_side):
                context = original(server_side)
                if context is not None and not server_side:
                    context.set_ecdh_curve("X448")
                return context

            security.get_ssl_context = get_ssl_context
            tcp_client.get_ssl_context = get_ssl_context

        energy_service = "AC" if self.protocol.endswith("AC") or self.protocol == "ISO_15118_2" else "DC"
        transfer_mode = "AC_three_phase_core" if energy_service == "AC" else "DC_core"
        config = EVCCConfig(**{
            "supportedProtocols": [self.protocol],
            "supportedEnergyServices": [energy_service],
            "energyTransferMode": transfer_mode,
            "isCertInstallNeeded": False,
            "useTls": True,
            "enforceTls": True,
            "chargeLoopCycle": self.charge_loop_cycles,
            "sdpRetryCycles": 3,
            "maxContractCerts": 3,
            "maxSupportingPoints": 1024,
        })
        config.load_raw_values()

        outer = self

        class RunnerSimEVController(SimEVController):
            def __init__(self):
                super().__init__(config)
                self._pause_on_next_continue = False
                if outer.iso20_soc is not None:
                    self._soc = outer.iso20_soc

            async def get_display_params(self):
                params = await super().get_display_params()
                if outer.iso20_capacity_wh is not None:
                    params.battery_energy_capacity = RationalNumber(
                        exponent=0,
                        value=outer.iso20_capacity_wh,
                    )
                return params

            async def get_ac_charge_loop_params_v20(
                self,
                control_mode,
                selected_service,
            ):
                params = await super().get_ac_charge_loop_params_v20(
                    control_mode,
                    selected_service,
                )
                if outer.iso20_power_w is not None:
                    params.ev_present_active_power = RationalNumber(
                        exponent=0,
                        value=outer.iso20_power_w,
                    )
                self._pause_on_next_continue = True
                return params

            async def continue_charging(self):
                if (
                    outer.pause_after_charge_loop
                    and self._pause_on_next_continue
                    and not outer.charge_loop_reached.is_set()
                ):
                    outer.charge_loop_reached.set()
                    while not outer._charge_loop_resume.is_set():
                        await asyncio.sleep(0.05)
                    return False
                return await super().continue_charging()

        controller = RunnerSimEVController() if use_tls13 else SimEVController(config)
        handler = EVCCHandler(config, self.iface, ExificientEXICodec(), controller)
        await handler.start()

    def run(self, timeout: float = 120):
        try:
            asyncio.run(asyncio.wait_for(self._run(), timeout))
        finally:
            if self._pki is not None:
                import shutil
                shutil.rmtree(self._pki, ignore_errors=True)
                self._pki = None

    def resume_charge_loop(self):
        self._charge_loop_resume.set()

    @staticmethod
    def _create_pki(use_tls13: bool):
        source = CERTS_DIR / ("iso20" if use_tls13 else "iso2")
        root = Path(tempfile.mkdtemp(prefix="iso15118-evsim-"))
        cert_dir = root / "iso15118_2/certs"
        key_dir = root / "iso15118_2/private_keys"
        cert_dir.mkdir(parents=True)
        key_dir.mkdir(parents=True)
        for path in (source / "certs").iterdir():
            (cert_dir / path.name).symlink_to(path)
        for path in (source / "private_keys").iterdir():
            (key_dir / path.name).symlink_to(path)
        return root


@contextlib.contextmanager
def managed_socket(sock):
    try:
        yield sock
    finally:
        sock.close()
