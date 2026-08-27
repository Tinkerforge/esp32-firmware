#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import contextlib
import time

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from _common import (
    EVTestClient,
    ISO20_AC,
    IsoTestEnvironment,
    V2GTP_ISO20_AC,
    V2GTP_ISO20_COMMON,
    managed_socket,
)


environment = None
client = None
saved_ocpp = None

ZERO_SESSION = "0000000000000000"


def suite_setup(tc: TestContext):
    global environment, client, saved_ocpp
    saved_ocpp = tc.api("ocpp/config")
    if saved_ocpp["enable"]:
        disabled = dict(saved_ocpp)
        disabled["enable"] = False
        tc.api("ocpp/config_update", disabled, timeout=5)
        time.sleep(1)

    environment = IsoTestEnvironment(tc)
    environment.start()
    client = EVTestClient(environment.host, environment.iface, environment.secc_ll)


def setup(tc: TestContext):
    assert environment is not None
    environment.reset_session()


def suite_teardown(tc: TestContext):
    errors = []
    if environment is not None:
        try:
            environment.stop()
        except Exception as e:
            errors.append(e)
    if saved_ocpp is not None:
        try:
            tc.api("ocpp/config_update", saved_ocpp, timeout=5)
        except Exception as e:
            errors.append(e)
    if errors:
        raise errors[0]


def message_header(session_id):
    return {"SessionID": session_id, "TimeStamp": int(time.time())}


def common_exchange(tls, request_name, session_id, fields=None):
    assert client is not None
    request = {"Header": message_header(session_id), **(fields or {})}
    response = client.exchange(
        tls,
        {request_name: request},
        "urn:iso:std:iso:15118:-20:CommonMessages",
        V2GTP_ISO20_COMMON,
    )
    return response[request_name.removesuffix("Req") + "Res"]


def ac_exchange(tls, request_name, session_id, fields):
    assert client is not None
    response = client.exchange(
        tls,
        {request_name: {"Header": message_header(session_id), **fields}},
        "urn:iso:std:iso:15118:-20:AC",
        V2GTP_ISO20_AC,
    )
    return response[request_name.removesuffix("Req") + "Res"]


def assert_response(tc: TestContext, response, session_id, expected):
    actual = dict(response)
    header = actual.pop("Header")
    tc.assert_eq({"SessionID", "TimeStamp"}, set(header))
    tc.assert_eq(session_id, header["SessionID"])
    tc.assert_(isinstance(header["TimeStamp"], int))
    tc.assert_eq(expected, actual)


@contextlib.contextmanager
def iso20_session(tc: TestContext):
    assert client is not None
    context = client.tls13_context()
    with managed_socket(client.connect_tls(context)) as tls:
        tc.assert_false(tls.session_reused)
        tc.assert_eq("TLSv1.3", tls.version())
        tc.assert_eq(
            {"ResponseCode": "OK_SuccessfulNegotiation", "SchemaID": 2},
            client.sap(tls, [ISO20_AC]),
        )
        setup_res = common_exchange(
            tls,
            "SessionSetupReq",
            ZERO_SESSION,
            {"EVCCID": "020000000001"},
        )
        tc.assert_eq("OK_NewSessionEstablished", setup_res["ResponseCode"])
        session_id = setup_res["Header"]["SessionID"]
        tc.assert_search(r"^[0-9A-F]{16}$", session_id)
        yield tls, session_id


def set_debug_power(tc: TestContext, current, phases):
    tc.api("iso15118/debug_update", {
        "enable": True,
        "current": current,
        "phases": phases,
    }, timeout=5)


def discover_and_select(tc: TestContext, tls, session_id, parameter_set_id):
    discovery = common_exchange(tls, "ServiceDiscoveryReq", session_id)
    tc.assert_eq("OK", discovery["ResponseCode"])
    detail = common_exchange(
        tls, "ServiceDetailReq", session_id, {"ServiceID": 1}
    )
    tc.assert_eq("OK", detail["ResponseCode"])
    selection = common_exchange(tls, "ServiceSelectionReq", session_id, {
        "SelectedEnergyTransferService": {
            "ServiceID": 1,
            "ParameterSetID": parameter_set_id,
        },
    })
    tc.assert_eq("OK", selection["ResponseCode"])


def reset_transport():
    assert environment is not None
    environment.reset_session()


def rational(value, exponent=0):
    return {"Exponent": exponent, "Value": value}


def cpd_request(asymmetric=False):
    params = {
        "EVMaximumChargePower": rational(11000),
        "EVMinimumChargePower": rational(0),
    }
    if asymmetric:
        params.update({
            "EVMaximumChargePower_L2": rational(11000),
            "EVMaximumChargePower_L3": rational(11000),
            "EVMinimumChargePower_L2": rational(0),
            "EVMinimumChargePower_L3": rational(0),
        })
    return {"AC_CPDReqEnergyTransferMode": params}


def charge_loop_request(asymmetric=False):
    params = {
        "EVTargetEnergyRequest": rational(0),
        "EVMaximumEnergyRequest": rational(0),
        "EVMinimumEnergyRequest": rational(0),
        "EVMaximumChargePower": rational(0),
        "EVMinimumChargePower": rational(0),
        "EVPresentActivePower": rational(0),
        "EVPresentReactivePower": rational(0),
    }
    if asymmetric:
        for name in (
            "EVMaximumChargePower",
            "EVMinimumChargePower",
            "EVPresentActivePower",
            "EVPresentReactivePower",
        ):
            params[f"{name}_L2"] = rational(0)
            params[f"{name}_L3"] = rational(0)
    return {
        "MeterInfoRequested": False,
        "Dynamic_AC_CLReqControlMode": params,
    }


def test_fresh_tls13_iso20_sap(tc: TestContext):
    assert client is not None
    with managed_socket(client.connect_tls(client.tls13_context())) as tls:
        tc.assert_false(tls.session_reused)
        tc.assert_eq("TLSv1.3", tls.version())
        tc.assert_eq(
            {"ResponseCode": "OK_SuccessfulNegotiation", "SchemaID": 2},
            client.sap(tls, [ISO20_AC]),
        )


def test_service_discovery_and_detail_shape(tc: TestContext):
    expected_parameters = [
        (1, 2),
        (2, 1),
    ]
    with iso20_session(tc) as (tls, session_id):
        discovery = common_exchange(tls, "ServiceDiscoveryReq", session_id)
        assert_response(tc, discovery, session_id, {
            "ResponseCode": "OK",
            "ServiceRenegotiationSupported": True,
            "EnergyTransferServiceList": {
                "Service": [{"ServiceID": 1, "FreeService": True}],
            },
        })

        detail = common_exchange(
            tls, "ServiceDetailReq", session_id, {"ServiceID": 1}
        )
        parameter_sets = []
        for set_id, connector in expected_parameters:
            parameter_sets.append({
                "ParameterSetID": set_id,
                "Parameter": [
                    {"Name": "Connector", "intValue": connector},
                    {"Name": "ControlMode", "intValue": 2},
                    {"Name": "EVSENominalVoltage", "intValue": 230},
                    {"Name": "MobilityNeedsMode", "intValue": 1},
                    {"Name": "Pricing", "intValue": 0},
                ],
            })
        assert_response(tc, detail, session_id, {
            "ResponseCode": "OK",
            "ServiceID": 1,
            "ServiceParameterList": {"ParameterSet": parameter_sets},
        })

    reset_transport()
    with iso20_session(tc) as (tls, session_id):
        common_exchange(tls, "ServiceDiscoveryReq", session_id)
        invalid = common_exchange(tls, "ServiceDetailReq", session_id, {"ServiceID": 99})
        assert_response(tc, invalid, session_id, {
            "ResponseCode": "FAILED_ServiceIDInvalid",
            "ServiceID": 99,
            "ServiceParameterList": {
                "ParameterSet": [{
                    "ParameterSetID": 0,
                    "Parameter": [{"Name": "Invalid", "boolValue": False}],
                }],
            },
        })


def test_service_selection_ids(tc: TestContext):
    cases = [
        (1, 1, "OK"),
        (1, 2, "OK"),
        (99, 1, "FAILED_ServiceSelectionInvalid"),
        (1, 99, "FAILED_ServiceSelectionInvalid"),
    ]
    for index, (service_id, parameter_id, response_code) in enumerate(cases):
        if index:
            reset_transport()
        with iso20_session(tc) as (tls, session_id):
            common_exchange(tls, "ServiceDiscoveryReq", session_id)
            common_exchange(
                tls, "ServiceDetailReq", session_id, {"ServiceID": 1}
            )
            response = common_exchange(tls, "ServiceSelectionReq", session_id, {
                "SelectedEnergyTransferService": {
                    "ServiceID": service_id,
                    "ParameterSetID": parameter_id,
                },
            })
            assert_response(
                tc, response, session_id, {"ResponseCode": response_code}
            )


def test_ac_symmetric_three_phase_power(tc: TestContext):
    set_debug_power(tc, current=6000, phases=3)
    with iso20_session(tc) as (tls, session_id):
        discover_and_select(tc, tls, session_id, parameter_set_id=1)
        cpd = ac_exchange(
            tls, "AC_ChargeParameterDiscoveryReq", session_id, cpd_request()
        )
        assert_response(tc, cpd, session_id, {
            "ResponseCode": "OK",
            "AC_CPDResEnergyTransferMode": {
                "EVSEMaximumChargePower": rational(4140),
                "EVSEMinimumChargePower": rational(0),
                "EVSENominalFrequency": rational(50),
            },
        })

        loop = ac_exchange(
            tls, "AC_ChargeLoopReq", session_id, charge_loop_request()
        )
        assert_response(tc, loop, session_id, {
            "ResponseCode": "OK",
            "Dynamic_AC_CLResControlMode": {
                "EVSETargetActivePower": rational(4140),
            },
        })


def test_ac_symmetric_single_phase_power(tc: TestContext):
    set_debug_power(tc, current=6000, phases=1)
    with iso20_session(tc) as (tls, session_id):
        discover_and_select(tc, tls, session_id, parameter_set_id=2)
        cpd = ac_exchange(
            tls, "AC_ChargeParameterDiscoveryReq", session_id, cpd_request()
        )
        assert_response(tc, cpd, session_id, {
            "ResponseCode": "OK",
            "AC_CPDResEnergyTransferMode": {
                "EVSEMaximumChargePower": rational(13800, -1),
                "EVSEMinimumChargePower": rational(0),
                "EVSENominalFrequency": rational(50),
            },
        })

        loop = ac_exchange(
            tls, "AC_ChargeLoopReq", session_id, charge_loop_request()
        )
        assert_response(tc, loop, session_id, {
            "ResponseCode": "OK",
            "Dynamic_AC_CLResControlMode": {
                "EVSETargetActivePower": rational(13800, -1),
            },
        })


def test_ac_asymmetric_phase_power(tc: TestContext):
    set_debug_power(tc, current=6000, phases=3)
    with iso20_session(tc) as (tls, session_id):
        discover_and_select(tc, tls, session_id, parameter_set_id=1)
        cpd = ac_exchange(
            tls, "AC_ChargeParameterDiscoveryReq", session_id, cpd_request(True)
        )
        assert_response(tc, cpd, session_id, {
            "ResponseCode": "OK",
            "AC_CPDResEnergyTransferMode": {
                "EVSEMaximumChargePower": rational(13800, -1),
                "EVSEMaximumChargePower_L2": rational(13800, -1),
                "EVSEMaximumChargePower_L3": rational(13800, -1),
                "EVSEMinimumChargePower": rational(0),
                "EVSEMinimumChargePower_L2": rational(0),
                "EVSEMinimumChargePower_L3": rational(0),
                "EVSENominalFrequency": rational(50),
            },
        })

        loop = ac_exchange(
            tls, "AC_ChargeLoopReq", session_id, charge_loop_request(True)
        )
        expected_three_phase = {
            "ResponseCode": "OK",
            "Dynamic_AC_CLResControlMode": {
                "EVSETargetActivePower": rational(13800, -1),
                "EVSETargetActivePower_L2": rational(13800, -1),
                "EVSETargetActivePower_L3": rational(13800, -1),
            },
        }
        assert_response(tc, loop, session_id, expected_three_phase)

        set_debug_power(tc, current=6000, phases=1)
        loop = ac_exchange(
            tls, "AC_ChargeLoopReq", session_id, charge_loop_request(True)
        )
        assert_response(tc, loop, session_id, {
            "ResponseCode": "OK",
            "Dynamic_AC_CLResControlMode": {
                "EVSETargetActivePower": rational(13800, -1),
                "EVSETargetActivePower_L2": rational(0, -3),
                "EVSETargetActivePower_L3": rational(0, -3),
            },
        })


def test_unknown_session_common_and_ac(tc: TestContext):
    with iso20_session(tc) as (tls, session_id):
        unknown_session = "".join(
            f"{int(char, 16) ^ 0xF:X}" for char in session_id
        )
        common = common_exchange(tls, "ServiceDiscoveryReq", unknown_session)
        assert_response(tc, common, session_id, {
            "ResponseCode": "FAILED_UnknownSession",
            "ServiceRenegotiationSupported": False,
            "EnergyTransferServiceList": {
                "Service": [{"ServiceID": 1, "FreeService": True}],
            },
        })

    reset_transport()
    with iso20_session(tc) as (tls, session_id):
        unknown_session = "".join(
            f"{int(char, 16) ^ 0xF:X}" for char in session_id
        )
        ac = ac_exchange(
            tls,
            "AC_ChargeParameterDiscoveryReq",
            unknown_session,
            cpd_request(),
        )
        assert_response(tc, ac, session_id, {
            "ResponseCode": "FAILED_UnknownSession",
            "AC_CPDResEnergyTransferMode": {
                "EVSEMaximumChargePower": rational(0),
                "EVSEMinimumChargePower": rational(0),
                "EVSENominalFrequency": rational(50),
            },
        })


if __name__ == "__main__":
    run_testsuite(locals())
