#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""HUB20-432-004/009/010: unanswered M07 fails closed and recovers.

Observes the timing in TC_HU_SECC_ISO20_OCSPResponder_Unreach_001.
The workbook's exact 60-second verdict is recorded separately from fail-closed
behavior; a socket read timeout is never treated as successful TLS closure.
"""

import time

import vehicle_validation as validation
from vehicle_validation import TestContext, vehicle, certificate_hash_data
from software.test_runner.test_context import run_testsuite

suite_setup = validation.suite_setup
suite_teardown = validation.suite_teardown
setup = validation.setup
teardown = validation.teardown


def test_unanswered_m07_while_authorization_is_polled(tc: TestContext):
    tc.set_test_timeout(150)
    env = validation.environment
    path, certs = env.chain("unanswered-m07")
    events = []
    start = time.monotonic()

    def record(event, **detail):
        events.append(dict(seconds=round(time.monotonic() - start, 6), event=event, **detail))
        env.write_json("unanswered-m07.json", events)

    def callback(connection, direction, version, content_type, message_type, data):
        if content_type == 21:
            record("TLS-alert", direction=direction, data=data.hex())

    with env.connect(path, message_callback=callback) as tls:
        tc.assert_eq("TLSv1.3", tls.version())
        vehicle.sap_iso20(tls)
        request, message = env.csms.expect("GetCertificateChainStatus", timeout=30)
        requested = time.monotonic()
        record("M07-withheld", message_id=message, request=request)
        expected = [certificate_hash_data(cert, issuer)
                    for cert, issuer in zip(certs, certs[1:] + [env.pki["vehicle"][0]])]
        tc.assert_eq(expected, [entry["certificateHashData"] for entry in request["certificateStatusRequests"]])
        tc.assert_(all(entry["source"] == "OCSP" and entry["urls"] == [validation.fixtures.URL]
                       for entry in request["certificateStatusRequests"]))
        session = vehicle.session_setup(tls)
        tc.assert_eq("OK", vehicle.authorization_setup(tls, session)["ResponseCode"])
        authorization_start = time.monotonic()
        tls.settimeout(5)
        pending_count = 0
        while True:
            response = vehicle.authorization(tls, session)
            record("AuthorizationRes", response=response)
            tc.assert_not(response["ResponseCode"] == "OK" and response["EVSEProcessing"] == "Finished")
            if response["ResponseCode"].startswith("FAILED"):
                break
            tc.assert_eq("OK", response["ResponseCode"])
            tc.assert_eq("Ongoing", response["EVSEProcessing"])
            pending_count += 1
            tc.assert_(time.monotonic() - authorization_start < 65)
            time.sleep(1)
        tc.assert_(pending_count > 0)
        # Neither TimeoutError nor an unrelated SSLError counts as closure.
        tc.assert_eq(b"", tls.recv(1))
        closed = time.monotonic()
        record("EOF", after_m07_seconds=closed - requested,
               after_authorization_seconds=closed - authorization_start)
        tc.assert_(any(event["event"] == "TLS-alert" and event["direction"] == "read"
                       and event["data"] == "0100" for event in events))
        tc.assert_(closed - requested < 65)
        env.write_json("unanswered-m07-verdict.json", {
            "fail_closed": True, "authorization_polls": pending_count + 1,
            "close_after_m07_seconds": closed - requested,
            "close_after_authorization_seconds": closed - authorization_start,
            "within_workbook_60_second_window": 58 <= closed - authorization_start <= 65,
        })
    time.sleep(2)
    env.record("unanswered M07 fails closed (exact timing recorded separately)", {
        "close_after_m07_seconds": round(closed - requested, 3), "polls": pending_count + 1})


def test_good_chain_recovers_after_unanswered_m07(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    path, certs = env.chain("after-unanswered-m07")
    env.positive("Good M07 authorizes after preceding M07 timeout", path, certs)


if __name__ == "__main__":
    run_testsuite(locals())
