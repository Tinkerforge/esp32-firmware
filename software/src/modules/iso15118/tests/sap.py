#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import time

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from _common import EVTestClient, ISO2, ISO20_AC, IsoTestEnvironment, managed_socket


environment = None
client = None


def suite_setup(tc: TestContext):
    global environment, client
    environment = IsoTestEnvironment(tc)
    environment.start()
    client = EVTestClient(environment.host, environment.iface, environment.secc_ll)


def setup(tc: TestContext):
    assert environment is not None
    environment.reset_session()


def suite_teardown(tc: TestContext):
    if environment is not None:
        environment.stop()


def sap(protocols):
    assert client is not None
    with managed_socket(client.connect_tls(client.tls12_context())) as tls:
        result = client.sap(tls, protocols)
    time.sleep(0.5)
    return result


def test_iso20_refused_after_tls12(tc: TestContext):
    result = sap([ISO20_AC])
    tc.assert_eq("Failed_NoNegotiation", result["ResponseCode"])


def test_iso2_selected_after_tls12(tc: TestContext):
    result = sap([ISO2, ISO20_AC])
    tc.assert_eq("OK_SuccessfulNegotiation", result["ResponseCode"])
    tc.assert_eq(1, result["SchemaID"])


def test_iso2_selected_after_tls13(tc: TestContext):
    assert environment is not None
    assert client is not None
    saved_ocpp = tc.api("ocpp/config")
    disabled = dict(saved_ocpp)
    disabled["enable"] = False
    try:
        tc.api("ocpp/config_update", disabled, timeout=5)
        time.sleep(1)
        environment.reset_session()
        with managed_socket(client.connect_tls(client.tls13_context())) as tls:
            tc.assert_eq("TLSv1.3", tls.version())
            result = client.sap(tls, [ISO2])
        tc.assert_eq("OK_SuccessfulNegotiation", result["ResponseCode"])
        tc.assert_eq(1, result["SchemaID"])
    finally:
        tc.api("ocpp/config_update", saved_ocpp, timeout=15)


def test_unknown_iso20_major_ignored(tc: TestContext):
    iso20_major2 = dict(ISO20_AC, VersionNumberMajor=2, SchemaID=3)
    result = sap([iso20_major2, ISO2])
    tc.assert_eq("OK_SuccessfulNegotiation", result["ResponseCode"])
    tc.assert_eq(1, result["SchemaID"])


def test_iso2_session_setup_returns_evseid(tc: TestContext):
    from iso15118.shared.messages.enums import Namespace

    assert client is not None
    with managed_socket(client.connect_tls(client.tls12_context())) as tls:
        result = client.sap(tls, [ISO2])
        tc.assert_eq("OK_SuccessfulNegotiation", result["ResponseCode"])
        response = client.exchange(tls, {
            "V2G_Message": {
                "Header": {"SessionID": "0000000000000000"},
                "Body": {"SessionSetupReq": {"EVCCID": "020000000001"}},
            },
        }, Namespace.ISO_V2_MSG_DEF)
    evseid = response["V2G_Message"]["Body"]["SessionSetupRes"]["EVSEID"]
    tc.assert_search(r"^[A-Z0-9*]{7,37}$", evseid)


if __name__ == "__main__":
    run_testsuite(locals())
