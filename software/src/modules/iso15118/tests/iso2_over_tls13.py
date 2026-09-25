#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""HUB20-538-002 / HUB20-432-004: ISO-2 sessions over mutual TLS 1.3."""

import time

import vehicle_validation as validation
from vehicle_validation import TestContext, common, vehicle, certificate_hash_data
from software.test_runner.test_context import run_testsuite

suite_setup = validation.suite_setup
suite_teardown = validation.suite_teardown
setup = validation.setup
teardown = validation.teardown


def run_session(tc, label, statuses, *, tls12=False, status_required=True):
    from iso15118.shared.messages.enums import Namespace
    env = validation.environment
    path, certs = env.chain(label)
    messages = []
    with env.connect(path, tls12=tls12) as tls:
        tc.assert_eq("TLSv1.2" if tls12 else "TLSv1.3", tls.version())
        sap = vehicle.exchange(tls, "supportedAppProtocolReq", {"AppProtocol": [common.ISO2]}, Namespace.SAP, 0x8001)
        tc.assert_eq("OK_SuccessfulNegotiation", sap["supportedAppProtocolRes"]["ResponseCode"])
        tc.assert_eq(common.ISO2["SchemaID"], sap["supportedAppProtocolRes"]["SchemaID"])
        if status_required:
            request, mid = env.csms.expect("GetCertificateChainStatus", timeout=30)
            expected = [certificate_hash_data(cert, issuer) for cert, issuer in zip(certs, certs[1:] + [env.pki["vehicle"][0]])]
            tc.assert_eq(expected, [item["certificateHashData"] for item in request["certificateStatusRequests"]])
        else:
            try:
                unexpected, mid = env.csms.expect("GetCertificateChainStatus", timeout=2)
            except TimeoutError:
                pass
            else:
                env.csms.respond(mid, vehicle.chain_status_response(unexpected, ["Unknown"] * 3))
                tc.assert_(False)
        session = "0000000000000000"

        def exchange(name, body):
            response = vehicle.exchange(tls, "V2G_Message", {
                "Header": {"SessionID": session}, "Body": {name + "Req": body},
            }, Namespace.ISO_V2_MSG_DEF, 0x8001)["V2G_Message"]
            messages.append({"request": name, "response": response})
            env.write_json(f"iso2-tls13-{label}.json", messages)
            if name != "SessionSetup":
                tc.assert_eq(session, response["Header"]["SessionID"])
            return response

        response = exchange("SessionSetup", {"EVCCID": "020000000001"})
        tc.assert_eq("OK_NewSessionEstablished", response["Body"]["SessionSetupRes"]["ResponseCode"])
        session = response["Header"]["SessionID"]
        tc.assert_(int(session, 16) != 0)
        discovery = exchange("ServiceDiscovery", {})["Body"]["ServiceDiscoveryRes"]
        tc.assert_eq("OK", discovery["ResponseCode"])
        tc.assert_("ExternalPayment" in discovery["PaymentOptionList"]["PaymentOption"])
        selection = exchange("PaymentServiceSelection", {
            "SelectedPaymentOption": "ExternalPayment",
            "SelectedServiceList": {"SelectedService": [{"ServiceID": discovery["ChargeService"]["ServiceID"]}]},
        })["Body"]["PaymentServiceSelectionRes"]
        tc.assert_eq("OK", selection["ResponseCode"])
        if status_required:
            try:
                pending = exchange("Authorization", {})["Body"]["AuthorizationRes"]
                tc.assert_eq("OK", pending["ResponseCode"])
                tc.assert_eq("Ongoing", pending["EVSEProcessing"])
            finally:
                env.csms.respond(mid, vehicle.chain_status_response(request, statuses))
        deadline = time.monotonic() + 10
        while True:
            auth = exchange("Authorization", {})["Body"]["AuthorizationRes"]
            if auth["EVSEProcessing"] != "Ongoing" or time.monotonic() >= deadline:
                break
            time.sleep(.5)
        if statuses == ["Good"] * 3:
            tc.assert_eq("OK", auth["ResponseCode"])
            tc.assert_eq("Finished", auth["EVSEProcessing"])
            stopped = exchange("SessionStop", {"ChargingSession": "Terminate"})["Body"]["SessionStopRes"]
            tc.assert_eq("OK", stopped["ResponseCode"])
        else:
            tc.assert_(auth["ResponseCode"].startswith("FAILED"))
            tc.assert_(vehicle.tls_closed(tls))
        env.record(f"ISO2 {tls.version()} {label}", auth)
    time.sleep(2)


def test_good_status_and_subsequent_session(tc: TestContext):
    tc.set_test_timeout(180)
    for label in ("good-first", "good-subsequent"):
        run_session(tc, label, ["Good"] * 3)


def test_revoked_intermediate(tc: TestContext):
    tc.set_test_timeout(120)
    run_session(tc, "revoked", ["Good", "Revoked", "Good"])


def test_missing_intermediate_status(tc: TestContext):
    tc.set_test_timeout(120)
    run_session(tc, "missing", ["Good", None, "Good"])


def test_recovery_after_rejected_sessions(tc: TestContext):
    tc.set_test_timeout(120)
    run_session(tc, "recovery", ["Good"] * 3)


def test_iso2_tls12_authorization(tc: TestContext):
    tc.set_test_timeout(120)
    run_session(tc, "tls12", ["Good"] * 3, tls12=True, status_required=False)


def test_private_non_pnc_waiver(tc: TestContext):
    tc.set_test_timeout(120)
    env = validation.environment
    saved = env.variable("PrivateEnvironmentEnabled")
    try:
        env.variable("PrivateEnvironmentEnabled", "true")
        run_session(tc, "private-waiver", ["Good"] * 3, status_required=False)
    finally:
        env.variable("PrivateEnvironmentEnabled", saved)


if __name__ == "__main__":
    run_testsuite(locals())
