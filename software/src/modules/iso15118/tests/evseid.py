#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
"""B05/B06, ISO 15118 SessionSetup: configured EVSEID across protocols.

TC_HU_SECC_Connector_EVSE_Ids_Ocpp2_Keys_001 protocol-level coverage.
Ethernet debug mode does not exercise its physical charging loop.
"""

import time

import vehicle_validation as validation
from vehicle_validation import TestContext, common, vehicle, certificate_hash_data
from software.test_runner.test_context import run_testsuite

suite_teardown = validation.suite_teardown
setup = validation.setup
teardown = validation.teardown


def suite_setup(tc: TestContext):
    validation.suite_setup(tc)
    env = validation.environment
    env.variables["ISO15118EvseId"] = env.variable("ISO15118EvseId")
    env.write_json("variables-backup.json", env.variables)
    env.csms.interactive.add("NotifyReport")


def inventory(tc):
    env = validation.environment
    tc.assert_eq("Accepted", env.csms.call("GetBaseReport", {
        "requestId": 50, "reportBase": "FullInventory",
    })["status"])
    reports = []
    data = []
    while True:
        report, message = env.csms.expect("NotifyReport", timeout=60)
        env.csms.respond(message, {})
        tc.assert_eq(50, report["requestId"])
        tc.assert_eq(len(reports), report["seqNo"])
        reports.append(report)
        data.extend(report.get("reportData", []))
        if not report.get("tbc", False):
            break
    env.write_json("evseid-full-inventory.json", reports)
    return data


def variable(tc, component, value=None):
    env = validation.environment
    item = {"component": component, "variable": {"name": "ISO15118EvseId"}, "attributeType": "Actual"}
    if value is None:
        result = env.csms.call("GetVariables", {"getVariableData": [item]})["getVariableResult"][0]
    else:
        item["attributeValue"] = value
        result = env.csms.call("SetVariables", {"setVariableData": [item]})["setVariableResult"][0]
    env.write_json("evseid-last-variable-result.json", result)
    for field in ("component", "variable", "attributeType"):
        tc.assert_eq(item[field], result[field])
    tc.assert_eq("Accepted", result["attributeStatus"])
    return result.get("attributeValue")


def session(tc, label, expected, iso20):
    from iso15118.shared.messages.enums import Namespace
    env = validation.environment
    path, certs = env.chain(label)
    messages = []
    with env.connect(path if iso20 else None, tls12=not iso20) as tls:
        tc.assert_eq("TLSv1.3" if iso20 else "TLSv1.2", tls.version())
        protocol = vehicle.ISO20_AC if iso20 else common.ISO2
        sap = vehicle.exchange(tls, "supportedAppProtocolReq", {"AppProtocol": [protocol]},
                               Namespace.SAP, 0x8001)["supportedAppProtocolRes"]
        tc.assert_eq("OK_SuccessfulNegotiation", sap["ResponseCode"])
        tc.assert_eq(protocol["SchemaID"], sap["SchemaID"])
        if iso20:
            request, message = env.csms.expect("GetCertificateChainStatus", timeout=30)
            hashes = [certificate_hash_data(cert, issuer)
                      for cert, issuer in zip(certs, certs[1:] + [env.pki["vehicle"][0]])]
            tc.assert_eq(hashes, [item["certificateHashData"] for item in request["certificateStatusRequests"]])
            env.csms.respond(message, vehicle.chain_status_response(request, ["Good"] * 3))
        session_id = "0000000000000000"

        def exchange(name, body):
            if iso20:
                response = vehicle.exchange(tls, name + "Req", {"Header": vehicle.header(session_id), **body},
                                            Namespace.ISO_V20_COMMON_MSG, 0x8002)[name + "Res"]
                response_session = response["Header"]["SessionID"]
            else:
                message = vehicle.exchange(tls, "V2G_Message", {
                    "Header": {"SessionID": session_id}, "Body": {name + "Req": body},
                }, Namespace.ISO_V2_MSG_DEF, 0x8001)["V2G_Message"]
                response = message["Body"][name + "Res"]
                response_session = message["Header"]["SessionID"]
            messages.append({"request": name, "session_id": response_session, "response": response})
            env.write_json(label + ".json", messages)
            if name != "SessionSetup":
                tc.assert_eq(session_id, response_session)
            return response, response_session

        response, session_id = exchange("SessionSetup", {"EVCCID": "020000000001"})
        tc.assert_eq("OK_NewSessionEstablished", response["ResponseCode"])
        tc.assert_(int(session_id, 16) != 0)
        tc.assert_eq(expected, response["EVSEID"])
        if iso20:
            tc.assert_eq("OK", exchange("AuthorizationSetup", {})[0]["ResponseCode"])
            authorization = {"SelectedAuthorizationService": "EIM", "EIM_AReqAuthorizationMode": {}}
        else:
            discovery = exchange("ServiceDiscovery", {})[0]
            tc.assert_eq("OK", discovery["ResponseCode"])
            tc.assert_("ExternalPayment" in discovery["PaymentOptionList"]["PaymentOption"])
            tc.assert_eq("OK", exchange("PaymentServiceSelection", {
                "SelectedPaymentOption": "ExternalPayment",
                "SelectedServiceList": {"SelectedService": [{"ServiceID": discovery["ChargeService"]["ServiceID"]}]},
            })[0]["ResponseCode"])
            authorization = {}
        deadline = time.monotonic() + 10
        while True:
            auth = exchange("Authorization", authorization)[0]
            if auth["EVSEProcessing"] != "Ongoing" or time.monotonic() >= deadline:
                break
            time.sleep(.5)
        tc.assert_eq("OK", auth["ResponseCode"])
        tc.assert_eq("Finished", auth["EVSEProcessing"])
        tc.assert_eq("OK", exchange("SessionStop", {"ChargingSession": "Terminate"})[0]["ResponseCode"])
    env.record(label, {"EVSEID": expected, "SessionID": session_id})
    time.sleep(2)
    return session_id


def test_reported_evseid_drives_both_protocols_and_subsequent_sessions(tc: TestContext):
    tc.set_test_timeout(240)
    env = validation.environment
    data = inventory(tc)
    entries = [entry for entry in data if entry["component"]["name"] == "ISO15118Ctrlr"
               and entry["variable"]["name"] == "ISO15118EvseId"]
    tc.assert_eq(1, len(entries))
    entry = entries[0]
    component = entry["component"]
    tc.assert_eq({"name": "ISO15118Ctrlr", "evse": {"id": 1}}, component)
    evses = sorted({item["component"]["evse"]["id"] for item in data
                    if "evse" in item["component"]})
    tc.assert_eq([1], evses)
    env.write_json("evseid-addressing.json", {"reported_component": component, "evses": evses})
    characteristics = entry["variableCharacteristics"]
    session_ids = set()
    try:
        for index, expected in enumerate(("DE*TFO*E1234567", "DE*TFO*E7654321")):
            tc.assert_(characteristics["minLimit"] <= len(expected) <= characteristics["maxLimit"])
            variable(tc, component, expected)
            tc.assert_eq(expected, variable(tc, component))
            # No debug reset between these sessions: exercise real SessionStop
            # cleanup and live EVSEID refresh at the next SAP exchange.
            for iso20 in (False, True, False, True):
                label = f"evseid-{index}-{'iso20' if iso20 else 'iso2'}-{len(session_ids)}"
                session_id = session(tc, label, expected, iso20)
                tc.assert_(session_id not in session_ids)
                session_ids.add(session_id)
    finally:
        variable(tc, component, env.variables["ISO15118EvseId"])
        tc.assert_eq(env.variables["ISO15118EvseId"], variable(tc, component))


def test_evse_addressed_evseid(tc: TestContext):
    # Workbook A1/A2 addresses each reported EVSE. Record failures separately
    # from runtime checks using the FullInventory-advertised address.
    tc.set_test_timeout(30)
    expected = validation.environment.variable("ISO15118EvseId")
    component = {"name": "ISO15118Ctrlr", "evse": {"id": 1}}
    tc.assert_eq(expected, variable(tc, component))
    variable(tc, component, expected)


def test_unknown_evse_does_not_alias_evseid(tc: TestContext):
    env = validation.environment
    expected = env.variable("ISO15118EvseId")
    item = {"component": {"name": "ISO15118Ctrlr", "evse": {"id": 2}},
            "variable": {"name": "ISO15118EvseId"}, "attributeType": "Actual"}
    result = env.csms.call("GetVariables", {"getVariableData": [item]})["getVariableResult"][0]
    tc.assert_eq("UnknownComponent", result["attributeStatus"])
    result = env.csms.call("SetVariables", {"setVariableData": [dict(item, attributeValue="DE*TFO*E9999999")]})["setVariableResult"][0]
    tc.assert_eq("UnknownComponent", result["attributeStatus"])
    tc.assert_eq(expected, env.variable("ISO15118EvseId"))


if __name__ == "__main__":
    run_testsuite(locals())
