#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import time
import socket
from datetime import datetime, timedelta, timezone

from cryptography import x509
from cryptography.hazmat.primitives.asymmetric import ec, ed448
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.x509.oid import NameOID

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from _common import (
    CSMSSim,
    EVTestClient,
    ISO2,
    IsoTestEnvironment,
    LocalCSMSTls,
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
csms_tls = None
saved_certs = None


VARIABLES = [
    ("SeccId", None),
    ("CountryName", None),
    ("OrganizationName", None),
    ("Enabled", None),
    ("V2GCertificateInstallationEnabled", None),
    ("V2G20SECCLeafCryptoSuite", None),
    ("ISO15118EvseId", None),
    ("EnforceTlsEnabled", None),
    ("PrivateEnvironmentEnabled", None),
    ("PWMChargingFallbackTimeout", None),
]

IDENTITY_LIMITS = {
    "SeccId": (7, 64),
    "CountryName": (2, 2),
    "OrganizationName": (1, 64),
    "ISO15118EvseId": (7, 37),
}

BOOLEAN_VARIABLES = [
    "Enabled",
    "V2GCertificateInstallationEnabled",
    "EnforceTlsEnabled",
    "PrivateEnvironmentEnabled",
]


def get_variables(requests):
    assert csms is not None
    data = []
    for name, instance in requests:
        variable = {"name": name}
        if instance is not None:
            variable["instance"] = instance
        component = {"name": "ISO15118Ctrlr"}
        if name == "ProtocolSupported":
            component["evse"] = {"id": 1}
        data.append({"component": component, "variable": variable})
    return csms.call("GetVariables", {"getVariableData": data})["getVariableResult"]


def set_variable(name, value, instance=None):
    assert csms is not None
    variable = {"name": name}
    if instance is not None:
        variable["instance"] = instance
    component = {"name": "ISO15118Ctrlr"}
    if name == "ProtocolSupported":
        component["evse"] = {"id": 1}
    result = csms.call("SetVariables", {"setVariableData": [{
        "component": component,
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
        try:
            results = get_variables([(name, None) for name in saved_values])
            tc.assert_eq(["Accepted"] * len(saved_values), [r["attributeStatus"] for r in results])
            tc.assert_eq(list(saved_values.values()), [r["attributeValue"] for r in results])
        except Exception as e:
            errors.append(f"restored value readback: {e}")

    if errors:
        raise RuntimeError("Could not restore ISO15118Ctrlr values: " + "; ".join(errors))


def suite_setup(tc: TestContext):
    global environment, client, csms, saved_ocpp, test_ocpp, saved_values, pnc_supported, csms_tls, saved_certs
    environment = IsoTestEnvironment(tc)
    environment.start()
    client = EVTestClient(environment.host, environment.iface, environment.secc_ll)
    pnc_supported = "iso15118_pnc" in tc.api("info/features")
    saved_ocpp = tc.api("ocpp/config")
    saved_certs = tc.api("certs/state")
    local_ip = tc.get_local_ip()
    csms_tls = LocalCSMSTls(environment.host, local_ip)
    csms = CSMSSim(interactive=("SignCertificate", "NotifyReport", "NotifyEvent"),
                   certfile=str(csms_tls.certfile), keyfile=str(csms_tls.keyfile))
    test_ocpp = dict(saved_ocpp)
    test_ocpp.update({
        "enable": True,
        "protocol": 1,
        "url": f"wss://{local_ip}:{csms.port}",
        "identity": "warp4-c08-test",
        "enable_auth": True,
        "pass": "warp4-c08-test-password",
        "cert_id": csms_tls.cert_id,
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
            tc.assert_eq(saved_ocpp, tc.api("ocpp/config"))
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
    if csms_tls is not None:
        try:
            csms_tls.close()
            tc.assert_eq(saved_certs, tc.api("certs/state"))
        except Exception as e:
            errors.append(e)
    if errors:
        raise errors[0]


def test_protocol_supported(tc: TestContext):
    assert csms is not None
    results = get_variables([("ProtocolSupported", str(i)) for i in range(1, 5)])
    values = [result.get("attributeValue") for result in results]
    tc.assert_eq(["Accepted", "Accepted", "Accepted"], [r["attributeStatus"] for r in results[:3]])
    tc.assert_eq([
        "urn:din:70121:2012:MsgDef,2,0",
        "urn:iso:15118:2:2013:MsgDef,2,0",
        "urn:iso:std:iso:15118:-20:AC,1,0",
    ], values[:3])
    tc.assert_eq("UnknownVariable", results[3]["attributeStatus"])
    for instance, value in zip(("1", "2", "3"), values[:3]):
        tc.assert_eq("Rejected", set_variable("ProtocolSupported", "urn:example,1,0", instance))
        tc.assert_eq(value, get_variables([("ProtocolSupported", instance)])[0]["attributeValue"])
    for component, status in (
        ({"name": "ISO15118Ctrlr"}, "UnknownVariable"),
        ({"name": "ISO15118Ctrlr", "evse": {"id": 2}}, "UnknownComponent"),
    ):
        result = csms.call("GetVariables", {"getVariableData": [{
            "component": component,
            "variable": {"name": "ProtocolSupported", "instance": "1"},
        }]})["getVariableResult"][0]
        tc.assert_eq(status, result["attributeStatus"])


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


def test_device_model_report(tc: TestContext):
    assert csms is not None
    result = csms.call("GetBaseReport", {"requestId": 103, "reportBase": "FullInventory"})
    tc.assert_eq("Accepted", result["status"])
    entries = {}
    seq_no = 0
    while True:
        report, message_id = csms.expect("NotifyReport", timeout=30)
        csms.respond(message_id, {})
        tc.assert_eq(103, report["requestId"])
        tc.assert_eq(seq_no, report["seqNo"])
        seq_no += 1
        for entry in report.get("reportData", []):
            key = (entry["component"]["name"], entry["variable"]["name"], entry["variable"].get("instance"))
            # This suite inspects station-level and EVSE-1 ISO variables only.
            if key[0] in ("ISO15118Ctrlr", "SecurityCtrlr"):
                tc.assert_(key not in entries)
                entries[key] = entry
        if not report.get("tbc", False):
            break
    chain_size = entries[("SecurityCtrlr", "MaxCertificateChainSize", None)]
    tc.assert_eq("10000", chain_size["variableAttribute"][0]["value"])
    tc.assert_eq("ReadOnly", chain_size["variableAttribute"][0]["mutability"])
    tc.assert_eq(10000, chain_size["variableCharacteristics"]["maxLimit"])
    private = entries[("ISO15118Ctrlr", "PrivateEnvironmentEnabled", None)]
    tc.assert_eq("ReadWrite", private["variableAttribute"][0]["mutability"])
    tc.assert_eq(saved_values["PrivateEnvironmentEnabled"], private["variableAttribute"][0]["value"])

    for name, (minimum, maximum) in IDENTITY_LIMITS.items():
        entry = entries[("ISO15118Ctrlr", name, None)]
        tc.assert_("evse" not in entry["component"])
        characteristics = entry["variableCharacteristics"]
        tc.assert_eq("string", characteristics["dataType"])
        tc.assert_eq(minimum, characteristics["minLimit"])
        tc.assert_eq(maximum, characteristics["maxLimit"])
        attribute = entry["variableAttribute"][0]
        tc.assert_eq("ReadWrite", attribute["mutability"])
        tc.assert_eq(True, attribute["persistent"])
        tc.assert_eq(saved_values[name], attribute["value"])

    protocols = get_variables([("ProtocolSupported", str(i)) for i in range(1, 4)])
    for i, result in enumerate(protocols, 1):
        entry = entries[("ISO15118Ctrlr", "ProtocolSupported", str(i))]
        tc.assert_eq({"id": 1}, entry["component"]["evse"])
        tc.assert_eq("ReadOnly", entry["variableAttribute"][0]["mutability"])
        tc.assert_eq(result["attributeValue"], entry["variableAttribute"][0]["value"])
    tc.assert_(("ISO15118Ctrlr", "ProtocolSupported", "4") not in entries)
    suite = entries[("ISO15118Ctrlr", "V2G20SECCLeafCryptoSuite", None)]
    tc.assert_eq("OptionList", suite["variableCharacteristics"]["dataType"])
    tc.assert_eq("ecdsa_secp521r1_sha512,ed448", suite["variableCharacteristics"]["valuesList"])
    print("FullInventory: identity bounds/type/mutability/persistence, protocol EVSE association and crypto valuesList passed")


def test_identity_length_boundaries(tc: TestContext):
    # Hubject 3.2.3–3.2.6: both inclusive limits and adjacent invalid lengths.
    for name, (minimum, maximum) in IDENTITY_LIMITS.items():
        for length in sorted({minimum, maximum}):
            value = "DE" if name == "CountryName" else "Z" * length
            tc.assert_eq("Accepted", set_variable(name, value))
            tc.assert_eq(value, get_variables([(name, None)])[0]["attributeValue"])
        previous = get_variables([(name, None)])[0]["attributeValue"]
        for length in (minimum - 1, maximum + 1):
            tc.assert_eq("Rejected", set_variable(name, "Z" * length))
            tc.assert_eq(previous, get_variables([(name, None)])[0]["attributeValue"])
        print(f"{name}: inclusive {minimum}–{maximum} accepted, adjacent invalid lengths rejected without changing value")


def test_identity_persists_and_drives_csr_after_reboot(tc: TestContext):
    assert csms is not None
    tc.set_test_timeout(180)
    expected = {
        "SeccId": "DE*TFO*EC08SUBJECT",
        "CountryName": "DE",
        "OrganizationName": "C08 ISO Subject Organization",
        "ISO15118EvseId": "DE*TFO*EC08EVSE",
    }
    security_request = {"component": {"name": "SecurityCtrlr"}, "variable": {"name": "OrganizationName"}}
    security_before = csms.call("GetVariables", {"getVariableData": [security_request]})["getVariableResult"][0]
    tc.assert_eq("Accepted", security_before["attributeStatus"])
    tc.assert_(security_before["attributeValue"] != expected["OrganizationName"])
    for name, value in expected.items():
        tc.assert_eq("Accepted", set_variable(name, value))
    connection_count = csms.connection_count
    tc.reboot()
    csms.wait_for_connection(after=connection_count, timeout=60)
    tc.wait_for(lambda: tc.assert_(csms.connected.is_set()), timeout=5)
    results = get_variables([(name, None) for name in expected])
    tc.assert_eq(["Accepted"] * len(expected), [r["attributeStatus"] for r in results])
    tc.assert_eq(list(expected.values()), [r["attributeValue"] for r in results])
    security_after = csms.call("GetVariables", {"getVariableData": [security_request]})["getVariableResult"][0]
    tc.assert_eq(security_before, security_after)
    for trigger, certificate_type, dc in (
        ("SignV2GCertificate", "V2GCertificate", "CPO"),
        ("SignV2G20Certificate", "V2G20Certificate", "CSO"),
    ):
        tc.assert_eq("Accepted", csms.call("TriggerMessage", {"requestedMessage": trigger})["status"])
        request, message_id = csms.expect("SignCertificate", timeout=60)
        # Reject enrollment so the temporary pending key is removed.
        csms.respond(message_id, {"status": "Rejected"})
        tc.assert_eq(certificate_type, request["certificateType"])
        csr = x509.load_pem_x509_csr(request["csr"].encode())
        tc.assert_(csr.is_signature_valid)
        for oid, value in (
            (NameOID.COMMON_NAME, expected["SeccId"]),
            (NameOID.COUNTRY_NAME, expected["CountryName"]),
            (NameOID.ORGANIZATION_NAME, expected["OrganizationName"]),
            (NameOID.DOMAIN_COMPONENT, dc),
        ):
            tc.assert_eq([value], [attribute.value for attribute in csr.subject.get_attributes_for_oid(oid)])
        print(f"{certificate_type}: signed CSR uses persisted CN/C/O after reboot, DC={dc}")
    # A following request ensures the final rejection has been processed.
    tc.assert_eq(expected["SeccId"], get_variables([("SeccId", None)])[0]["attributeValue"])


def test_private_environment_persists_across_reboot(tc: TestContext):
    assert csms is not None
    expected = "false" if saved_values["PrivateEnvironmentEnabled"] == "true" else "true"
    tc.assert_eq("Accepted", set_variable("PrivateEnvironmentEnabled", expected))
    connection_count = csms.connection_count
    tc.reboot()
    csms.wait_for_connection(after=connection_count, timeout=60)
    result = get_variables([("PrivateEnvironmentEnabled", None)])[0]
    tc.assert_eq("Accepted", result["attributeStatus"])
    tc.assert_eq(expected, result["attributeValue"])


def test_missing_v2g_root_notify_event(tc: TestContext):
    # Hubject catalogue 47/E5: isolated untrusted issuer, no installed-root mutation.
    assert csms is not None
    tc.set_test_timeout(120)
    listing_request = {"certificateType": ["V2GRootCertificate", "V2GCertificateChain"]}
    before = csms.call("GetInstalledCertificateIds", listing_request)
    count = csms.call("GetVariables", {"getVariableData": [{
        "component": {"name": "SecurityCtrlr"}, "variable": {"name": "CertificateEntries"},
    }]})["getVariableResult"][0]["attributeValue"]
    security_before = len(csms.security_events)
    tc.assert_eq("Accepted", csms.call("TriggerMessage", {"requestedMessage": "SignV2G20Certificate"})["status"])
    request, mid = csms.expect("SignCertificate", timeout=60)
    csms.respond(mid, {"status": "Accepted"})
    try:
        csr = x509.load_pem_x509_csr(request["csr"].encode())
        tc.assert_(csr.is_signature_valid)
        key = ec.generate_private_key(ec.SECP256R1())
        subject = x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, "C10 uninstalled test root")])
        now = datetime.now(timezone.utc)
        root = (x509.CertificateBuilder().subject_name(subject).issuer_name(subject)
                .public_key(key.public_key()).serial_number(x509.random_serial_number())
                .not_valid_before(now - timedelta(days=1)).not_valid_after(now + timedelta(days=1))
                .add_extension(x509.BasicConstraints(ca=True, path_length=None), critical=True)
                .sign(key, hashes.SHA256()))
        leaf = (x509.CertificateBuilder().subject_name(csr.subject).issuer_name(subject)
                .public_key(csr.public_key()).serial_number(x509.random_serial_number())
                .not_valid_before(now - timedelta(days=1)).not_valid_after(now + timedelta(days=1))
                .add_extension(x509.BasicConstraints(ca=False, path_length=None), critical=True)
                .sign(key, hashes.SHA256()))
        ids = []
        for bundled in (False, True):
            chain = leaf.public_bytes(serialization.Encoding.PEM)
            if bundled:
                chain += root.public_bytes(serialization.Encoding.PEM)
            result = csms.call("CertificateSigned", {
                "certificateType": "V2G20Certificate", "requestId": request["requestId"],
                "certificateChain": chain.decode(),
            })
            tc.assert_eq("Rejected", result["status"])
            tc.assert_(result["statusInfo"]["reasonCode"] in ("NoTrustedRoot", "UntrustedChain"))
            event, event_mid = csms.expect("NotifyEvent", timeout=10)
            csms.respond(event_mid, {})
            tc.assert_eq(0, event["seqNo"])
            tc.assert_eq(False, event.get("tbc", False))
            tc.assert_eq(1, len(event["eventData"]))
            data = event["eventData"][0]
            tc.assert_eq({"name": "SecurityCtrlr"}, data["component"])
            tc.assert_eq({"name": "CertificateEntries"}, data["variable"])
            tc.assert_eq(count, data["actualValue"])
            tc.assert_eq("Alerting", data["trigger"])
            tc.assert_eq("HardWiredNotification", data["eventNotificationType"])
            tc.assert_eq(3, data["severity"])
            tc.assert_eq(result["statusInfo"]["reasonCode"], data["techCode"])
            tc.assert_eq("V2GCertificateChain installation failed because the corresponding V2G root was not found.", data["techInfo"])
            tc.assert_("variableMonitoringId" not in data)
            tc.assert_(isinstance(data["eventId"], int) and data["eventId"] >= 0)
            ids.append(data["eventId"])
            for stamp in (event["generatedAt"], data["timestamp"]):
                tc.assert_(abs((datetime.now(timezone.utc) - datetime.fromisoformat(stamp.replace("Z", "+00:00"))).total_seconds()) < 120)
            tc.assert_eq(before, csms.call("GetInstalledCertificateIds", listing_request))
            print(f"Missing-root rejection with bundled_root={bundled}: exact NotifyEvent payload and unchanged inventory passed")
        tc.assert_(ids[0] != ids[1])
        tc.assert_eq(security_before, len(csms.security_events))
    finally:
        # A new trigger aborts the old pending key; reject the new CSR as well.
        tc.assert_eq("Accepted", csms.call("TriggerMessage", {"requestedMessage": "SignV2G20Certificate"})["status"])
        _, cleanup_mid = csms.expect("SignCertificate", timeout=60)
        csms.respond(cleanup_mid, {"status": "Rejected"})
        tc.assert_eq(before, csms.call("GetInstalledCertificateIds", listing_request))


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


def test_v2g20_crypto_suite_persists_across_reboot(tc: TestContext):
    assert csms is not None

    cases = (
        ("ecdsa_secp521r1_sha512,ed448", ec.EllipticCurvePublicKey),
        ("ed448", ed448.Ed448PublicKey),
    )
    for expected, key_type in cases:
        tc.assert_eq("Accepted", set_variable("V2G20SECCLeafCryptoSuite", expected))
        connection_count = csms.connection_count
        tc.reboot()
        csms.wait_for_connection(after=connection_count, timeout=60)

        result = get_variables([("V2G20SECCLeafCryptoSuite", None)])[0]
        tc.assert_eq("Accepted", result["attributeStatus"])
        tc.assert_eq(expected, result["attributeValue"])

        trigger = csms.call("TriggerMessage", {
            "requestedMessage": "SignV2G20Certificate",
        })
        tc.assert_eq("Accepted", trigger["status"])
        request, message_id = csms.expect("SignCertificate", timeout=60)
        tc.assert_eq("V2G20Certificate", request["certificateType"])
        csr = x509.load_pem_x509_csr(request["csr"].encode())
        tc.assert_(csr.is_signature_valid)
        tc.assert_(isinstance(csr.public_key(), key_type))
        csms.respond(message_id, {"status": "Rejected"})


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


def test_enabled_controls_iso15118_runtime(tc: TestContext):
    assert environment is not None
    assert client is not None

    tc.assert_eq("Accepted", set_variable("Enabled", "true"))
    time.sleep(1)
    active = client.connect_raw()

    tc.assert_eq("Accepted", set_variable("Enabled", "false"))
    active.settimeout(5)
    try:
        tc.assert_eq(b"", active.recv(1))
    except ConnectionResetError:
        pass
    except socket.timeout:
        tc.assert_(False, "active ISO 15118 socket remained open after disabling the controller")
    finally:
        active.close()
    tc.assert_(sdp_request(environment.iface, timeout=2, expected_from=environment.secc_ll) is None)

    debug = tc.api("iso15118/debug")
    tc.api("iso15118/debug_update", dict(debug, enable=False), timeout=5)
    tc.api("iso15118/debug_update", dict(debug, enable=True), timeout=5)
    tc.assert_(sdp_request(environment.iface, timeout=2, expected_from=environment.secc_ll) is None)

    tc.assert_eq("Accepted", set_variable("Enabled", "true"))
    tc.wait_for(
        lambda: tc.assert_(
            sdp_request(environment.iface, timeout=1, expected_from=environment.secc_ll) is not None
        ),
        timeout=15,
    )


if __name__ == "__main__":
    run_testsuite(locals())
