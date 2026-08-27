#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import time

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from _common import (
    CSMSSim,
    EVTestClient,
    ISO2,
    IsoTestEnvironment,
    SDP_SECURITY_NO_TLS,
    SDP_SECURITY_TLS,
    managed_socket,
    sdp_request,
)


environment = None
client = None
csms = None
saved_ocpp = None
test_ocpp = None
saved_values = {}
pnc_supported = False


VARIABLES = [
    ("Enabled", None),
    ("V2GCertificateInstallationEnabled", None),
    ("ISO15118EvseId", None),
    ("EnforceTlsEnabled", None),
    ("PrivateEnviromentEnabled", None),
    ("PWMChargingFallbackTimeout", None),
]

BOOLEAN_VARIABLES = [
    "Enabled",
    "V2GCertificateInstallationEnabled",
    "EnforceTlsEnabled",
    "PrivateEnviromentEnabled",
]


def get_variables(requests):
    assert csms is not None
    data = []
    for name, instance in requests:
        variable = {"name": name}
        if instance is not None:
            variable["instance"] = instance
        data.append({"component": {"name": "ISO15118Ctrlr"}, "variable": variable})
    return csms.call("GetVariables", {"getVariableData": data})["getVariableResult"]


def set_variable(name, value, instance=None):
    assert csms is not None
    variable = {"name": name}
    if instance is not None:
        variable["instance"] = instance
    result = csms.call("SetVariables", {"setVariableData": [{
        "component": {"name": "ISO15118Ctrlr"},
        "variable": variable,
        "attributeValue": value,
    }]})
    return result["setVariableResult"][0]["attributeStatus"]


def wait_for_ocpp_disconnected(timeout=30):
    assert csms is not None
    deadline = time.monotonic() + timeout
    while csms.connected.is_set() and time.monotonic() < deadline:
        time.sleep(0.1)
    if csms.connected.is_set():
        raise TimeoutError("WARP4 did not disconnect from CSMSSim")


def connect_test_ocpp(tc: TestContext):
    assert csms is not None
    assert test_ocpp is not None
    tc.api("ocpp/config_update", test_ocpp, timeout=5)
    if not csms.connected.wait(timeout=60):
        raise TimeoutError("WARP4 did not connect to CSMSSim")


def reconnect_test_ocpp(tc: TestContext):
    assert test_ocpp is not None
    disabled = dict(test_ocpp)
    disabled["enable"] = False
    tc.api("ocpp/config_update", disabled, timeout=5)
    wait_for_ocpp_disconnected()
    connect_test_ocpp(tc)


def restore_values(tc: TestContext):
    if not saved_values:
        return

    errors = []
    assert csms is not None
    if not csms.connected.is_set():
        try:
            connect_test_ocpp(tc)
        except Exception as e:
            errors.append(f"could not reconnect to restore values: {e}")

    if csms.connected.is_set():
        for name, value in saved_values.items():
            try:
                status = set_variable(name, value)
                if status != "Accepted":
                    errors.append(f"ISO15118Ctrlr.{name}: {status}")
            except Exception as e:
                errors.append(f"ISO15118Ctrlr.{name}: {e}")

    if errors:
        raise RuntimeError("Could not restore ISO15118Ctrlr values: " + "; ".join(errors))


def suite_setup(tc: TestContext):
    global environment, client, csms, saved_ocpp, test_ocpp, saved_values, pnc_supported
    environment = IsoTestEnvironment(tc)
    environment.start()
    client = EVTestClient(environment.host, environment.iface, environment.secc_ll)
    pnc_supported = "iso15118_pnc" in tc.api("info/features")
    saved_ocpp = tc.api("ocpp/config")
    csms = CSMSSim()
    test_ocpp = dict(saved_ocpp)
    test_ocpp.update({
        "enable": True,
        "protocol": 1,
        "url": f"ws://{tc.get_local_ip()}:{csms.port}",
        "enable_auth": False,
    })
    connect_test_ocpp(tc)
    variables = list(VARIABLES)
    if pnc_supported:
        variables.append(("ContractCertificateInstallationEnabled", None))
    results = get_variables(variables)
    for (name, _), result in zip(variables, results):
        if result["attributeStatus"] != "Accepted":
            raise RuntimeError(f"Could not save ISO15118Ctrlr.{name}: {result['attributeStatus']}")
    saved_values = {
        name: result["attributeValue"]
        for (name, _), result in zip(variables, results)
    }


def setup(tc: TestContext):
    assert environment is not None
    environment.reset_session()


def teardown(tc: TestContext):
    restore_values(tc)


def suite_teardown(tc: TestContext):
    errors = []
    try:
        restore_values(tc)
    except Exception as e:
        errors.append(e)
    if saved_ocpp is not None:
        try:
            disabled = dict(test_ocpp if test_ocpp is not None else saved_ocpp)
            disabled["enable"] = False
            tc.api("ocpp/config_update", disabled, timeout=5)
            time.sleep(1)
            tc.api("ocpp/config_update", saved_ocpp, timeout=5)
        except Exception as e:
            errors.append(e)
    if csms is not None:
        try:
            csms.stop()
        except Exception as e:
            errors.append(e)
    if environment is not None:
        try:
            environment.stop()
        except Exception as e:
            errors.append(e)
    if errors:
        raise errors[0]


def test_protocol_supported(tc: TestContext):
    results = get_variables([("ProtocolSupported", str(i)) for i in range(1, 5)])
    values = [result.get("attributeValue") for result in results]
    tc.assert_eq(["Accepted", "Accepted", "Accepted"], [r["attributeStatus"] for r in results[:3]])
    tc.assert_eq([
        "urn:din:70121:2012:MsgDef,2,0",
        "urn:iso:15118:2:2013:MsgDef,2,0",
        "urn:iso:std:iso:15118:-20:AC,1,0",
    ], values[:3])
    tc.assert_eq("UnknownVariable", results[3]["attributeStatus"])


def test_controller_values_available(tc: TestContext):
    results = get_variables([
        ("PWMChargingFallbackTimeout", None),
        ("ISO15118EvseId", None),
        ("EnforceTlsEnabled", None),
    ])
    tc.assert_(all(result["attributeStatus"] == "Accepted" for result in results))
    tc.assert_eq("7", results[0]["attributeValue"])
    tc.assert_search(r"^[A-Z0-9*]{7,37}$", results[1]["attributeValue"])
    tc.assert_in(["true", "false"], results[2]["attributeValue"])

    contract = get_variables([("ContractCertificateInstallationEnabled", None)])[0]
    tc.assert_eq("Accepted" if pnc_supported else "UnknownVariable", contract["attributeStatus"])


def test_evseid_set_and_read_back(tc: TestContext):
    expected = "DE*TNK*E123456"
    tc.assert_eq("Accepted", set_variable("ISO15118EvseId", expected))
    result = get_variables([("ISO15118EvseId", None)])[0]
    tc.assert_eq("Accepted", result["attributeStatus"])
    tc.assert_eq(expected, result["attributeValue"])


def test_variable_validation_and_persistence(tc: TestContext):
    tc.assert_eq("Rejected", set_variable("ISO15118EvseId", "Z" * 6))
    tc.assert_eq("Rejected", set_variable("ISO15118EvseId", "Z" * 38))

    boolean_variables = list(BOOLEAN_VARIABLES)
    if pnc_supported:
        boolean_variables.append("ContractCertificateInstallationEnabled")
    for name in boolean_variables:
        tc.assert_eq("Rejected", set_variable(name, "maybe"))

    tc.assert_eq("Rejected", set_variable("PWMChargingFallbackTimeout", "0"))
    tc.assert_eq(
        "Rejected",
        set_variable("ProtocolSupported", "urn:example,1,0", instance="1"),
    )

    expected = {
        name: "false" if saved_values[name] == "true" else "true"
        for name in boolean_variables
    }
    expected["ISO15118EvseId"] = "DE*ICE*E*1234567890*1"
    expected["PWMChargingFallbackTimeout"] = (
        "15" if saved_values["PWMChargingFallbackTimeout"] != "15" else "16"
    )
    for name, value in expected.items():
        tc.assert_eq("Accepted", set_variable(name, value))

    reconnect_test_ocpp(tc)

    results = get_variables([(name, None) for name in expected])
    tc.assert_(all(result["attributeStatus"] == "Accepted" for result in results))
    tc.assert_eq(list(expected.values()), [result["attributeValue"] for result in results])


def test_enforce_tls_controls_sdp(tc: TestContext):
    assert environment is not None
    tc.assert_eq("Accepted", set_variable("EnforceTlsEnabled", "true"))
    time.sleep(1)
    tc.assert_(sdp_request(environment.iface, SDP_SECURITY_NO_TLS) is None)
    response = sdp_request(environment.iface, SDP_SECURITY_TLS)
    tc.assert_(response is not None)
    tc.assert_eq(SDP_SECURITY_TLS, response["security"])

    tc.assert_eq("Accepted", set_variable("EnforceTlsEnabled", "false"))
    time.sleep(1)
    response = sdp_request(environment.iface, SDP_SECURITY_NO_TLS)
    tc.assert_(response is not None)
    tc.assert_eq(SDP_SECURITY_NO_TLS, response["security"])


if __name__ == "__main__":
    run_testsuite(locals())
