#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
# ruff: noqa: I001

import contextlib
import json
from datetime import datetime, timedelta, timezone
import ipaddress
from pathlib import Path
import socket
import ssl
import subprocess
import tempfile
import threading
import time

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.x509.oid import ExtendedKeyUsageOID, NameOID

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.src.modules.iso15118.tests._common import CSMSSim
from software.test_runner.test_context import TestContext, run_testsuite


IDENTITY = "warp4-tls-security-events-test"
PASSWORD = "tls-security-events-password"

saved_ocpp = None
tmpdir = None
cert_id = None
cert_added = False
local_ip = None
server_cert = None
server_key = None
bogus_server_cert = None
bogus_server_key = None
date_server_certs = {}
name_server_certs = {}


def generate_certificates(ip: str):
    global tmpdir, server_cert, server_key

    tmpdir = tempfile.TemporaryDirectory(prefix="ocpp-tls-security-events-")
    directory = Path(tmpdir.name)

    def make_ca(name: str):
        key = directory / f"{name}-key.pem"
        cert = directory / f"{name}.pem"
        subprocess.run([
            "openssl", "req", "-x509", "-newkey", "ec",
            "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
            "-keyout", key, "-out", cert, "-days", "365",
            "-subj", f"/CN={name}",
            "-addext", "basicConstraints=critical,CA:TRUE",
        ], check=True, capture_output=True)
        return key, cert

    def make_server(name: str, ca_key: Path, ca_cert: Path):
        key = directory / f"{name}-key.pem"
        csr = directory / f"{name}.csr"
        cert = directory / f"{name}.pem"
        ext = directory / f"{name}-ext.cnf"
        ext.write_text(f"subjectAltName=IP:{ip}\n")
        subprocess.run([
            "openssl", "req", "-newkey", "ec",
            "-pkeyopt", "ec_paramgen_curve:P-256", "-nodes",
            "-keyout", key, "-out", csr, "-subj", f"/CN={name}",
        ], check=True, capture_output=True)
        subprocess.run([
            "openssl", "x509", "-req", "-in", csr,
            "-CA", ca_cert, "-CAkey", ca_key, "-CAcreateserial",
            "-out", cert, "-days", "365", "-sha256", "-extfile", ext,
        ], check=True, capture_output=True)
        return cert, key

    ca_key, ca_cert = make_ca("WARP4 TLS security event test CA")
    bogus_ca_key, bogus_ca_cert = make_ca("WARP4 untrusted test CA")
    server_cert, server_key = make_server("ocpp-tls-security-events", ca_key, ca_cert)
    bogus_cert, bogus_key = make_server("ocpp-untrusted", bogus_ca_key, bogus_ca_cert)

    # Keep issuer, server key, SAN and usage valid; vary only the leaf validity.
    issuer = x509.load_pem_x509_certificate(ca_cert.read_bytes())
    issuer_key = serialization.load_pem_private_key(ca_key.read_bytes(), password=None)
    key = serialization.load_pem_private_key(server_key.read_bytes(), password=None)
    now = datetime.now(timezone.utc)
    for label, start, end in [
        ("expired", now - timedelta(days=2), now - timedelta(days=1)),
        ("future", now + timedelta(days=1), now + timedelta(days=2)),
    ]:
        cert = (x509.CertificateBuilder()
                .subject_name(x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, "ocpp-date-test")]))
                .issuer_name(issuer.subject).public_key(key.public_key())
                .serial_number(x509.random_serial_number())
                .not_valid_before(start).not_valid_after(end)
                .add_extension(x509.BasicConstraints(ca=False, path_length=None), critical=True)
                .add_extension(x509.SubjectAlternativeName([x509.IPAddress(ipaddress.ip_address(ip))]), critical=False)
                .add_extension(x509.ExtendedKeyUsage([ExtendedKeyUsageOID.SERVER_AUTH]), critical=False)
                .sign(issuer_key, hashes.SHA256()))
        path = directory / f"server-{label}.pem"
        path.write_bytes(cert.public_bytes(serialization.Encoding.PEM))
        date_server_certs[label] = path
    # Trusted, currently valid identities: vary only CN and SAN. An IP
    # endpoint must match an iPAddress SAN, not a DNS SAN containing IP text.
    wrong_ip = "192.0.2.1" if ip != "192.0.2.1" else "192.0.2.2"
    for label, cn, names in [
        ("matching_ip_san", "different.example.invalid", [x509.IPAddress(ipaddress.ip_address(ip))]),
        ("wrong_ip_san", "different.example.invalid", [x509.IPAddress(ipaddress.ip_address(wrong_ip))]),
        ("matching_cn_wrong_san", ip, [x509.IPAddress(ipaddress.ip_address(wrong_ip))]),
        ("ip_text_dns_san", "different.example.invalid", [x509.DNSName(ip)]),
        ("matching_cn_no_san", ip, []),
    ]:
        cert = (x509.CertificateBuilder()
                .subject_name(x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, cn)]))
                .issuer_name(issuer.subject).public_key(key.public_key())
                .serial_number(x509.random_serial_number())
                .not_valid_before(now - timedelta(days=1)).not_valid_after(now + timedelta(days=2))
                .add_extension(x509.BasicConstraints(ca=False, path_length=None), critical=True)
                .add_extension(x509.ExtendedKeyUsage([ExtendedKeyUsageOID.SERVER_AUTH]), critical=False))
        if names:
            cert = cert.add_extension(x509.SubjectAlternativeName(names), critical=False)
        cert = cert.sign(issuer_key, hashes.SHA256())
        path = directory / f"server-name-{label}.pem"
        path.write_bytes(cert.public_bytes(serialization.Encoding.PEM))
        name_server_certs[label] = path
    return ca_cert, bogus_cert, bogus_key


def configure(tc: TestContext, url: str):
    assert saved_ocpp is not None
    config = dict(saved_ocpp)
    config.update({
        "enable": True,
        "protocol": 1,
        "url": url,
        "identity": IDENTITY,
        "enable_auth": True,
        "pass": PASSWORD,
        "cert_id": cert_id,
    })
    tc.api("ocpp/config_update", config, timeout=5)


def reconnect_and_assert(tc: TestContext, event_type: str, port: int):
    assert server_cert is not None
    assert server_key is not None
    csms = CSMSSim(port=port, certfile=str(server_cert), keyfile=str(server_key))
    if not csms.connected.wait(timeout=60):
        csms.stop()
        raise TimeoutError("WARP4 did not reconnect to the valid CSMS")
    try:
        tc.wait_for(
            lambda: tc.assert_eq(
                1,
                sum(event.get("type") == event_type for event in csms.security_events),
            ),
            timeout=30,
        )
        # Allow the remaining queued calls to drain before checking duplicates.
        time.sleep(1)
        events = [event for event in csms.security_events if event.get("type") == event_type]
        tc.assert_eq(1, len(events))
        timestamp = datetime.fromisoformat(events[0]["timestamp"].replace("Z", "+00:00"))
        tc.assert_(abs((datetime.now(timezone.utc) - timestamp).total_seconds()) < 120)
        print(f"Recovered CSMS received exactly one {event_type} event at {events[0]['timestamp']}")
    finally:
        csms.stop()


def run_openssl_failure(tc: TestContext, event_type: str, options: list[str]):
    assert local_ip is not None
    assert server_cert is not None
    assert server_key is not None

    port = tc.find_free_port(19443)
    process = subprocess.Popen(
        ["openssl", "s_server", "-accept", str(port), "-cert", server_cert,
         "-key", server_key, "-quiet", *options],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        text=True,
    )
    try:
        time.sleep(1)
        if process.poll() is not None:
            raise RuntimeError(f"openssl s_server failed: {process.stderr.read()}")
        configure(tc, f"wss://{local_ip}:{port}")
        # Cover the automatic 10 second reconnect and prove failure-streak
        # deduplication before the queued event is delivered.
        time.sleep(12)
    finally:
        process.terminate()
        with contextlib.suppress(subprocess.TimeoutExpired):
            process.wait(timeout=5)
        if process.poll() is None:
            process.kill()
            process.wait(timeout=5)
    reconnect_and_assert(tc, event_type, port)


def suite_setup(tc: TestContext):
    global saved_ocpp, cert_id, cert_added, local_ip, bogus_server_cert, bogus_server_key

    if not tc.device_type().is_warp(4):
        tc.skip("TLS security event test requires a WARP4")

    saved_ocpp = tc.api("ocpp/config")
    disabled = dict(saved_ocpp)
    disabled["enable"] = False
    tc.api("ocpp/config_update", disabled, timeout=5)
    time.sleep(1)
    # Security-event timestamps are compared with host UTC. Establish that
    # precondition even when this suite is started immediately after flashing.
    tc.wait_for(lambda: tc.assert_(abs(tc.api("ntp/state")["time"] - int(time.time() / 60)) <= 1), timeout=30)

    local_ip = tc.get_local_ip()
    ca_cert, bogus_cert, bogus_key = generate_certificates(local_ip)
    used_cert_ids = {cert["id"] for cert in tc.api("certs/state")["certs"]}
    cert_id = next((candidate for candidate in range(7, -1, -1) if candidate not in used_cert_ids), None)
    if cert_id is None:
        tc.skip("TLS security event test needs one free certificate slot")
    tc.api("certs/add", {
        "id": cert_id,
        "name": "OCPP TLS security event test",
        "cert": ca_cert.read_text(),
    })
    cert_added = True

    bogus_server_cert = bogus_cert
    bogus_server_key = bogus_key


def suite_teardown(tc: TestContext):
    errors = []

    def api_with_retry(path, payload):
        last_error = None
        for _ in range(3):
            try:
                return tc.api(path, payload, timeout=15)
            except Exception as e:  # noqa: BLE001
                last_error = e
                time.sleep(1)
        raise last_error

    if saved_ocpp is not None:
        with contextlib.suppress(Exception):
            current = tc.api("ocpp/config")
            current["enable"] = False
            api_with_retry("ocpp/config_update", current)
            time.sleep(1)
        try:
            api_with_retry("ocpp/config_update", saved_ocpp)
            tc.assert_eq(saved_ocpp, tc.api("ocpp/config"))
        except Exception as e:  # noqa: BLE001
            errors.append(e)
    if cert_added:
        try:
            api_with_retry("certs/remove", {"id": cert_id})
            tc.assert_not(any(cert["id"] == cert_id for cert in tc.api("certs/state")["certs"]))
        except Exception as e:  # noqa: BLE001
            errors.append(e)
    if tmpdir is not None:
        tmpdir.cleanup()
    if errors:
        raise errors[0]
    print("Restored OCPP configuration and removed the temporary trust certificate")


def test_invalid_csms_certificate(tc: TestContext):
    assert local_ip is not None
    assert bogus_server_cert is not None
    assert bogus_server_key is not None
    port = tc.find_free_port(19443)
    invalid_csms = CSMSSim(port=port, certfile=str(bogus_server_cert), keyfile=str(bogus_server_key))
    try:
        configure(tc, f"wss://{local_ip}:{port}")
        time.sleep(12)
    finally:
        invalid_csms.stop()
    reconnect_and_assert(tc, "InvalidCsmsCertificate", port)


def run_date_failure(tc: TestContext, label: str, expected_alert: str):
    tc.set_test_timeout(120)
    # Give each date case a fresh startup context. Live reconfiguration is
    # covered separately below. Configure promptly after reboot to regress
    # delayed startup
    # replacing an already-started client and losing its queued security event.
    tc.reboot()
    tc.wait_for(lambda: tc.assert_(tc.api("ntp/state")["time"] >= int(time.time() / 60) - 1), timeout=30)
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(date_server_certs[label], server_key)
    # Avoid an old endpoint reconnect racing the new configuration after boot.
    port = tc.find_free_port(19543 if label == "expired" else 19643)
    observed = []

    def accept_tls(listener):
        try:
            raw, _ = listener.accept()
            with raw:
                raw.settimeout(15)
                with context.wrap_socket(raw, server_side=True):
                    observed.append("invalid certificate accepted")
        except Exception as exc:
            observed.append(exc)

    # Observe an actual authenticated TLS rejection. Merely seeing no WebSocket
    # connection would also pass if the charger never attempted to connect.
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listener:
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listener.bind(("0.0.0.0", port))
        listener.listen(1)
        listener.settimeout(30)
        worker = threading.Thread(target=accept_tls, args=(listener,), daemon=True)
        worker.start()
        try:
            configure(tc, f"wss://{local_ip}:{port}")
        finally:
            worker.join(timeout=50)
        tc.assert_not(worker.is_alive())
    tc.assert_eq(1, len(observed))
    print(f"CSMS {label} TLS observation: {observed[0]!r}")
    tc.assert_(isinstance(observed[0], ssl.SSLError))
    tc.assert_(expected_alert in str(observed[0]).upper())
    print(f"CSMS {label} certificate rejected: {observed[0]}")
    time.sleep(1)
    trace = tc.http_request("GET", "/trace_log", timeout=15).decode(errors="replace")
    tc.assert_("TLS connection failed: InvalidCsmsCertificate" in trace)
    reconnect_and_assert(tc, "InvalidCsmsCertificate", port)


def test_expired_csms_certificate(tc: TestContext):
    run_date_failure(tc, "expired", "CERTIFICATE_EXPIRED")


def test_future_csms_certificate(tc: TestContext):
    run_date_failure(tc, "future", "CERTIFICATE_UNKNOWN")


def run_name_failure(tc: TestContext, label: str):
    tc.set_test_timeout(90)
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(name_server_certs[label], server_key)
    observed = []

    def accept_tls(listener):
        try:
            raw, _ = listener.accept()
            with raw:
                raw.settimeout(15)
                with context.wrap_socket(raw, server_side=True) as tls:
                    observed.append(tls.recv(4096))
        except Exception as exc:
            observed.append(exc)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listener:
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listener.bind(("0.0.0.0", 0))
        port = listener.getsockname()[1]
        listener.listen(1)
        listener.settimeout(30)
        worker = threading.Thread(target=accept_tls, args=(listener,), daemon=True)
        worker.start()
        try:
            configure(tc, f"wss://{local_ip}:{port}")
        finally:
            worker.join(timeout=50)
        tc.assert_not(worker.is_alive())
    tc.assert_eq(1, len(observed))
    tc.assert_(isinstance(observed[0], ssl.SSLError))
    tc.assert_("BAD_CERTIFICATE" in str(observed[0]).upper())
    print(f"CSMS identity {label} rejected: {observed[0]}")
    reconnect_and_assert(tc, "InvalidCsmsCertificate", port)


def test_csms_name_matching_ip_san_different_cn(tc: TestContext):
    server = CSMSSim(certfile=str(name_server_certs["matching_ip_san"]), keyfile=str(server_key))
    try:
        configure(tc, f"wss://{local_ip}:{server.port}")
        tc.assert_(server.connected.wait(timeout=30))
        response = server.call("GetVariables", {
            "getVariableData": [{"component": {"name": "OCPPCommCtrlr"},
                                 "variable": {"name": "HeartbeatInterval"}}],
        }, timeout=10)
        tc.assert_eq(1, len(response["getVariableResult"]))
        tc.assert_eq([], server.security_events)
        print("Matching IP SAN accepted despite different CN; OCPP request/response passed")
    finally:
        server.stop()


def test_csms_name_wrong_ip_san(tc: TestContext):
    run_name_failure(tc, "wrong_ip_san")


def test_csms_name_matching_cn_wrong_san(tc: TestContext):
    run_name_failure(tc, "matching_cn_wrong_san")


def test_csms_name_ip_text_dns_san(tc: TestContext):
    run_name_failure(tc, "ip_text_dns_san")


def test_csms_name_matching_cn_no_san(tc: TestContext):
    run_name_failure(tc, "matching_cn_no_san")


def test_ocpp16_retains_legacy_date_policy(tc: TestContext):
    tc.set_test_timeout(90)
    tc.reboot()
    # HTTP may be reachable before the network module reports readiness. The
    # legacy stack's reconnect interval can exceed this TLS listener's timeout.
    tc.wait_for(lambda: tc.assert_(tc.api("network/state")["connected"]), timeout=30)
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(date_server_certs["expired"], server_key)
    observed = []
    port = tc.find_free_port(19743)

    def accept_tls(listener):
        try:
            raw, _ = listener.accept()
            with raw:
                raw.settimeout(15)
                with context.wrap_socket(raw, server_side=True) as tls:
                    # Application data proves the device accepted the server
                    # certificate; server-side handshake completion alone does not.
                    observed.append(tls.recv(4096))
        except Exception as exc:
            observed.append(exc)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listener:
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listener.bind(("0.0.0.0", port))
        listener.listen(1)
        listener.settimeout(30)
        worker = threading.Thread(target=accept_tls, args=(listener,), daemon=True)
        worker.start()
        try:
            config = dict(saved_ocpp, enable=True, protocol=0, url=f"wss://{local_ip}:{port}",
                          identity=IDENTITY, enable_auth=True, **{"pass": PASSWORD, "cert_id": cert_id})
            tc.api("ocpp/config_update", config, timeout=5)
        finally:
            worker.join(timeout=50)
        tc.assert_not(worker.is_alive())
    tc.assert_eq(1, len(observed))
    if isinstance(observed[0], Exception):
        print(f"OCPP 1.6 TLS failure: {observed[0]!r}")
    tc.assert_(isinstance(observed[0], bytes) and observed[0].startswith(b"GET "))
    print("OCPP 1.6 sent its WebSocket upgrade over TLS with an expired trusted server certificate")


def test_live_reconfiguration_with_incoming_messages(tc: TestContext):
    tc.set_test_timeout(180)
    boot_id = tc.api("event_log/boot_id")
    servers = [CSMSSim(certfile=str(server_cert), keyfile=str(server_key)) for _ in range(2)]
    try:
        configure(tc, f"wss://{local_ip}:{servers[0].port}")
        tc.assert_(servers[0].connected.wait(timeout=30))
        for cycle in range(8):
            old = servers[cycle % 2]
            new = servers[(cycle + 1) % 2]
            after = new.connection_count
            stop = threading.Event()

            def incoming():
                from websockets.exceptions import ConnectionClosed
                try:
                    sequence = 0
                    while not stop.is_set():
                        old.ws.send(json.dumps([2, f"reconfigure-{cycle}-{sequence}", "GetVariables", {
                            "getVariableData": [{"component": {"name": "OCPPCommCtrlr"},
                                                 "variable": {"name": "HeartbeatInterval"}}],
                        }]))
                        sequence += 1
                        time.sleep(0.02)
                except ConnectionClosed:
                    pass

            worker = threading.Thread(target=incoming, daemon=True)
            worker.start()
            try:
                configure(tc, f"wss://{local_ip}:{new.port}")
                # API responsiveness must survive client teardown while RX is active.
                tc.api("ocpp/state", timeout=3)
                new.wait_for_connection(after=after, timeout=15)
                response = new.call("GetVariables", {
                    "getVariableData": [{"component": {"name": "OCPPCommCtrlr"},
                                         "variable": {"name": "HeartbeatInterval"}}],
                }, timeout=5)
                tc.assert_eq(1, len(response["getVariableResult"]))
                tc.assert_eq(boot_id, tc.api("event_log/boot_id"))
                print(f"Live reconfiguration {cycle + 1}/8 completed without reboot")
            finally:
                stop.set()
                worker.join(timeout=5)
    finally:
        for server in servers:
            server.stop()


def test_receive_buffer_chunks_and_peer_close(tc: TestContext):
    tc.set_test_timeout(120)
    boot_id = tc.api("event_log/boot_id")
    for cycle in range(4):
        server = CSMSSim(certfile=str(server_cert), keyfile=str(server_key))
        try:
            configure(tc, f"wss://{local_ip}:{server.port}")
            tc.assert_(server.connected.wait(timeout=15))
            # Whitespace expands a valid request beyond the transport RX buffer
            # without changing the OCPP payload or exceeding its parser capacity.
            for suffix in ("large", "small"):
                message_id = f"chunks-{cycle}-{suffix}"
                frame = json.dumps([2, message_id, "GetVariables", {
                    "getVariableData": [{"component": {"name": "OCPPCommCtrlr"},
                                         "variable": {"name": "HeartbeatInterval"}}],
                }])
                if suffix == "large":
                    frame = frame[:1] + " " * 12000 + frame[1:]
                server.ws.send(frame)
            for suffix in ("large", "small"):
                message_id, response = server.responses.get(timeout=10)
                tc.assert_eq(f"chunks-{cycle}-{suffix}", message_id)
                tc.assert_eq(1, len(response["getVariableResult"]))
            server.ws.close()
            tc.api("ocpp/state", timeout=3)
            tc.assert_eq(boot_id, tc.api("event_log/boot_id"))
            print(f"RX chunk reassembly, ordered replies and peer close {cycle + 1}/4 passed")
        finally:
            server.stop()


def test_invalid_tls_version(tc: TestContext):
    run_openssl_failure(tc, "InvalidTLSVersion", ["-tls1_1", "-cipher", "ALL:@SECLEVEL=0"])


def test_invalid_tls_cipher_suite(tc: TestContext):
    run_openssl_failure(tc, "InvalidTLSCipherSuite", ["-tls1_2", "-cipher", "AES128-SHA256:@SECLEVEL=0"])


def test_csms_rejects_ecdhe_ecdsa_cbc(tc: TestContext):
    # Matches the server's EC certificate and the compiled library's supported
    # key exchange. Rejection must come from the OCPP-specific AEAD allow list.
    run_openssl_failure(tc, "InvalidTLSCipherSuite", ["-tls1_2", "-cipher", "ECDHE-ECDSA-AES128-SHA256:@SECLEVEL=0"])


if __name__ == "__main__":
    run_testsuite(locals())
