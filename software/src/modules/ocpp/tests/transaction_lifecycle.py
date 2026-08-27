#!/usr/bin/env -S uv run --locked --group iso15118-tests --script
# ruff: noqa: I001

import math
import time

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.src.modules.iso15118.tests._common import CSMSSim
from software.test_runner.test_context import TestContext, run_testsuite


TAG = "04:A1:B2:C3:D4:E5:F6"
ID_TOKEN = TAG.replace(":", "")

csms = None
saved_ocpp = None
saved_ocpp_enabled = None
test_ocpp = None
testbox = None


def assert_status(tc: TestContext, expected: str, timeout: float = 15):
    assert csms is not None
    payload, message_id = csms.expect("StatusNotification", timeout=timeout)
    tc.assert_eq(expected, payload.get("connectorStatus"))
    tc.assert_eq(1, payload.get("evseId"))
    tc.assert_eq(1, payload.get("connectorId"))
    tc.assert_(isinstance(payload.get("timestamp"), str))
    csms.respond(message_id, {})


def meter_value(tc: TestContext, payload, expected_context: str) -> float:
    values = payload.get("meterValue")
    tc.assert_(isinstance(values, list) and len(values) == 1)
    samples = values[0].get("sampledValue")
    tc.assert_(isinstance(samples, list) and len(samples) == 1)
    sample = samples[0]
    tc.assert_eq("Energy.Active.Import.Register", sample.get("measurand"))
    tc.assert_eq(expected_context, sample.get("context"))
    value = sample.get("value")
    tc.assert_(isinstance(value, (int, float)) and not isinstance(value, bool))
    tc.assert_(math.isfinite(value))
    return float(value)


def assert_transaction_common(tc: TestContext, payload, event_type: str, transaction_id: str, seq_no: int):
    tc.assert_eq(event_type, payload.get("eventType"))
    tc.assert_eq(seq_no, payload.get("seqNo"))
    tc.assert_(isinstance(payload.get("timestamp"), str))
    tc.assert_eq(transaction_id, payload.get("transactionInfo", {}).get("transactionId"))
    tc.assert_eq(1, payload.get("evse", {}).get("id"))
    tc.assert_eq(1, payload.get("evse", {}).get("connectorId"))


def suite_setup(tc: TestContext):
    global csms, saved_ocpp, saved_ocpp_enabled, test_ocpp, testbox

    if not tc.device_type().is_warp(4):
        tc.skip("OCPP 2.1 transaction lifecycle requires a WARP4")

    testbox = tc.get_testbox()
    testbox.set_cp("A")
    tc.wait_for(lambda: tc.assert_eq(0, tc.api("evse/state")["charger_state"]), timeout=10)

    saved_ocpp = tc.api("ocpp/config")
    saved_ocpp_enabled = tc.api("evse/ocpp_enabled")

    # Stop a configured backend before replacing it, then use a unique
    # identity so a persisted OCPP 2.1 network profile cannot override the URL.
    disabled = dict(saved_ocpp)
    disabled["enable"] = False
    tc.api("ocpp/config_update", disabled, timeout=5)
    time.sleep(1)

    csms = CSMSSim(interactive=(
        "BootNotification",
        "StatusNotification",
        "Authorize",
        "TransactionEvent",
    ))
    test_ocpp = dict(saved_ocpp)
    test_ocpp.update({
        "enable": True,
        "protocol": 1,
        "url": f"ws://{tc.get_local_ip()}:{csms.port}",
        "identity": "warp4-ocpp21-lifecycle-test",
        "enable_auth": False,
        # The GET API censors a configured password as null. A null PUT keeps
        # that value unchanged, which is required for lossless restoration.
        "pass": None,
        "cert_id": -1,
    })
    tc.api("evse/ocpp_enabled_update", {"enabled": True}, timeout=5)
    tc.api("ocpp/config_update", test_ocpp, timeout=5)
    if not csms.connected.wait(timeout=60):
        raise TimeoutError("WARP4 did not connect to the OCPP 2.1 CSMS simulator")


def suite_teardown(tc: TestContext):
    errors = []

    try:
        if testbox is not None:
            testbox.set_cp("A")
    except Exception as e:  # noqa: BLE001 - continue restoring independent resources
        errors.append(e)

    if test_ocpp is not None:
        try:
            disabled = dict(test_ocpp)
            disabled["enable"] = False
            tc.api("ocpp/config_update", disabled, timeout=5)
            time.sleep(1)
        except Exception as e:  # noqa: BLE001 - continue restoring independent resources
            errors.append(e)

    if csms is not None:
        try:
            csms.stop()
        except Exception as e:  # noqa: BLE001 - continue restoring independent resources
            errors.append(e)

    if saved_ocpp is not None:
        try:
            tc.api("ocpp/config_update", saved_ocpp, timeout=5)
        except Exception as e:  # noqa: BLE001 - continue restoring independent resources
            errors.append(e)

    if saved_ocpp_enabled is not None:
        try:
            tc.api("evse/ocpp_enabled_update", saved_ocpp_enabled, timeout=5)
        except Exception as e:  # noqa: BLE001 - continue restoring independent resources
            errors.append(e)

    if errors:
        raise errors[0]


def test_ocpp21_transaction_lifecycle(tc: TestContext):
    tc.set_test_timeout(150)
    assert csms is not None

    boot, message_id = csms.expect("BootNotification", timeout=30)
    tc.assert_eq("PowerUp", boot.get("reason"))
    tc.assert_(isinstance(boot.get("chargingStation", {}).get("model"), str))
    tc.assert_(isinstance(boot.get("chargingStation", {}).get("vendorName"), str))
    csms.respond(message_id, {
        "currentTime": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "interval": 300,
        "status": "Accepted",
    })
    assert_status(tc, "Available")

    result = csms.call("SetVariables", {"setVariableData": [{
        "component": {"name": "SampledDataCtrlr"},
        "variable": {"name": "TxUpdatedInterval"},
        "attributeValue": "2",
    }]})
    tc.assert_eq("Accepted", result["setVariableResult"][0]["attributeStatus"])

    tc.api("nfc/inject_tag", {"tag_type": 2, "tag_id": TAG}, timeout=5)
    authorize, message_id = csms.expect("Authorize", timeout=15)
    tc.assert_eq(ID_TOKEN, authorize.get("idToken", {}).get("idToken"))
    tc.assert_eq("ISO14443", authorize.get("idToken", {}).get("type"))
    csms.respond(message_id, {"idTokenInfo": {"status": "Accepted"}})
    assert_status(tc, "Occupied")

    testbox = tc.get_testbox()
    testbox.set_cp("C")

    started, message_id = csms.expect("TransactionEvent", timeout=20)
    tc.assert_eq("Started", started.get("eventType"))
    transaction_id = started.get("transactionInfo", {}).get("transactionId")
    tc.assert_(isinstance(transaction_id, str) and len(transaction_id) > 0)
    assert_transaction_common(tc, started, "Started", transaction_id, 0)
    tc.assert_eq("CablePluggedIn", started.get("triggerReason"))
    tc.assert_eq(ID_TOKEN, started.get("idToken", {}).get("idToken"))
    start_energy = meter_value(tc, started, "Transaction.Begin")
    csms.respond(message_id, {})
    tc.wait_for(lambda: tc.assert_(testbox.is_contactor_closed()), timeout=10)

    seq_no = 1
    deadline = time.monotonic() + 20
    while True:
        updated, message_id = csms.expect("TransactionEvent", timeout=max(0.1, deadline - time.monotonic()))
        assert_transaction_common(tc, updated, "Updated", transaction_id, seq_no)
        seq_no += 1
        csms.respond(message_id, {})
        if updated.get("triggerReason") == "MeterValuePeriodic":
            periodic_energy = meter_value(tc, updated, "Sample.Periodic")
            break
        tc.assert_eq("ChargingStateChanged", updated.get("triggerReason"))

    testbox.set_cp("A")
    tc.wait_for(lambda: tc.assert_false(testbox.is_contactor_closed()), timeout=10)

    while True:
        ended, message_id = csms.expect("TransactionEvent", timeout=20)
        event_type = ended.get("eventType")
        assert_transaction_common(tc, ended, event_type, transaction_id, seq_no)
        seq_no += 1
        csms.respond(message_id, {})
        if event_type == "Ended":
            break
        tc.assert_eq("Updated", event_type)

    tc.assert_eq("EVCommunicationLost", ended.get("triggerReason"))
    tc.assert_eq("EVDisconnected", ended.get("transactionInfo", {}).get("stoppedReason"))
    end_energy = meter_value(tc, ended, "Transaction.End")
    tc.assert_(start_energy <= periodic_energy <= end_energy)
    assert_status(tc, "Available")


if __name__ == "__main__":
    run_testsuite(locals())
