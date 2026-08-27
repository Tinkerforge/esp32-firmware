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


def test_tls13_ticket_resumption(tc: TestContext):
    assert client is not None
    context = client.tls13_context()

    with managed_socket(client.connect_tls(context)) as tls:
        tc.assert_false(tls.session_reused)
        result = client.sap(tls, [ISO20_AC])
        tc.assert_eq("OK_SuccessfulNegotiation", result["ResponseCode"])
        tc.assert_eq(2, result["SchemaID"])
        session = tls.session

    time.sleep(1)
    tc.assert_(session is not None and session.has_ticket)
    tc.assert_ge(20, session.ticket_lifetime_hint)
    tc.assert_le(86400, session.ticket_lifetime_hint)

    with managed_socket(client.connect_tls(context, session=session)) as tls:
        tc.assert_(tls.session_reused)
        tc.assert_eq("TLSv1.3", tls.version())
        result = client.sap(tls, [ISO20_AC, ISO2])
        tc.assert_eq("Failed_NoNegotiation", result["ResponseCode"])

    time.sleep(1)
    with managed_socket(client.connect_tls(context)) as tls:
        tc.assert_false(tls.session_reused)
        result = client.sap(tls, [ISO20_AC])
        tc.assert_eq("OK_SuccessfulNegotiation", result["ResponseCode"])
        tc.assert_eq(2, result["SchemaID"])


def test_tls12_not_resumed(tc: TestContext):
    assert client is not None
    context = client.tls12_context()
    with managed_socket(client.connect_tls(context)) as tls:
        tc.assert_eq("TLSv1.2", tls.version())
        result = client.sap(tls, [ISO2])
        tc.assert_eq("OK_SuccessfulNegotiation", result["ResponseCode"])
        tc.assert_eq(1, result["SchemaID"])
        session = tls.session

    time.sleep(1)
    tc.assert_(session is None or not session.has_ticket)
    if session is not None:
        with managed_socket(client.connect_tls(context, session=session)) as tls:
            tc.assert_false(tls.session_reused)
            result = client.sap(tls, [ISO2])
            tc.assert_eq("OK_SuccessfulNegotiation", result["ResponseCode"])
            tc.assert_eq(1, result["SchemaID"])


if __name__ == "__main__":
    run_testsuite(locals())
