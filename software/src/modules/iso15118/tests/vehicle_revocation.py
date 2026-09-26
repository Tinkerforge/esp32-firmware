#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""HUB20-432-002/004/008: observe revoked-leaf timing and TLS alerts.

Current-behavior regressions, not a pass of
TC_HU_SECC_ISO20_TLS_revokedClientCert_001's handshake-alert expectation.
"""

import ssl
import time

import vehicle_validation as validation
from vehicle_validation import TestContext, certificate_hash_data, vehicle
from software.test_runner.test_context import run_testsuite

suite_setup = validation.suite_setup
suite_teardown = validation.suite_teardown
setup = validation.setup
teardown = validation.teardown


def revoked_leaf(tc, delayed):
    tc.set_test_timeout(120)
    env = validation.environment
    label = "revoked-leaf-delayed" if delayed else "revoked-leaf-prompt"
    path, certs = env.chain(label)
    events = []
    start = time.monotonic()

    def record(event, **detail):
        events.append(dict(seconds=round(time.monotonic() - start, 6), event=event, **detail))
        env.write_json(label + ".json", events)

    def callback(connection, direction, version, content_type, message_type, data):
        if content_type in (21, 22):
            record("tls-message", direction=direction, content_type=int(content_type),
                   message_type=int(message_type), data=data.hex())

    with env.connect(path, message_callback=callback) as tls:
        tc.assert_eq("TLSv1.3", tls.version())
        record("client-handshake-returned")
        request, message = env.csms.expect("GetCertificateChainStatus", timeout=30)
        record("M07-request", request=request)
        expected = [certificate_hash_data(cert, issuer)
                    for cert, issuer in zip(certs, certs[1:] + [env.pki["vehicle"][0]])]
        tc.assert_eq(expected, [item["certificateHashData"] for item in request["certificateStatusRequests"]])
        tc.assert_(all(item["source"] == "OCSP" and item["urls"] == [validation.fixtures.URL]
                       for item in request["certificateStatusRequests"]))

        def respond():
            response = vehicle.chain_status_response(request, ["Revoked", "Good", "Good"])
            record("M07-response", response=response)
            env.csms.respond(message, response)

        if not delayed:
            respond()
            # Read before sending application data, to avoid masking a rejection
            # alert with a concurrent application write. A timeout is an observed
            # open connection here, never evidence of successful rejection.
            tls.settimeout(2)
            try:
                received = tls.recv(1)
            except TimeoutError:
                record("no-alert-or-close-within-two-seconds")
            else:
                record("unexpected-read", data=received.hex())
                tc.assert_(False)
            finally:
                tls.settimeout(15)

        vehicle.sap_iso20(tls)
        record("authenticated-SAP-response")
        session = vehicle.session_setup(tls)
        tc.assert_(int(session, 16) != 0)
        tc.assert_eq("OK", vehicle.authorization_setup(tls, session)["ResponseCode"])
        if delayed:
            pending = vehicle.authorization(tls, session)
            record("authorization-pending", response=pending)
            tc.assert_eq("OK", pending["ResponseCode"])
            tc.assert_eq("Ongoing", pending["EVSEProcessing"])
            respond()
        final = vehicle.final_authorization(tls, session)
        record("authorization-result", response=final)
        tc.assert_(final["ResponseCode"].startswith("FAILED"))
        # Do not use tls_closed(), which also treats a timeout as closure.
        tls.settimeout(5)
        try:
            closed = tls.recv(1)
        except ssl.SSLError as exc:
            record("TLS-read-error", error=str(exc))
            raise
        record("EOF", data=closed.hex())
        tc.assert_eq(b"", closed)
        alerts = [event for event in events if event["event"] == "tls-message"
                  and event["direction"] == "read" and event["content_type"] == 21]
        tc.assert_not(any(event["data"] == "022c" for event in alerts))
    env.record(label + " current behavior (catalogue alert mismatch)", {"alerts": alerts})
    time.sleep(2)


def test_revoked_leaf_prompt_response(tc: TestContext):
    revoked_leaf(tc, delayed=False)


def test_revoked_leaf_delayed_response(tc: TestContext):
    revoked_leaf(tc, delayed=True)


def test_valid_leaf_recovers(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    path, certs = env.chain("after-revoked-leaf")
    env.positive("Good leaf authorizes after revoked-leaf sessions", path, certs)


if __name__ == "__main__":
    run_testsuite(locals())
