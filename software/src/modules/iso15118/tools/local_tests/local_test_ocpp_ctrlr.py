#!/usr/bin/env python3
"""ISO15118Ctrlr wiring test: OCPP variables consumed by the ISO stack.

Tests:
1. ProtocolSupported instances 1..3 filled from the SAP protocol list, instance 4 answers UnknownVariable
2. PWMChargingFallbackTimeout, ISO15118EvseId, EnforceTlsEnabled defaults
3. ISO15118EvseId is consumed in the -2 SessionSetupRes
4. EnforceTlsEnabled refuses no-TLS SDP requests (HUB20-52-001) and the refusal ends when set back to false

Run this in evsim venv:
../../../../../.venv-evsim/bin/python local_test_ocpp_ctrlr.py --charger <ip>
"""

import argparse
import json
import queue
import sys
import threading
import time
import uuid

import common

try:
    from websockets.sync.server import serve
except ImportError:
    raise SystemExit("websockets missing, install it into the evsim venv: pip install websockets")


class Csms:
    def __init__(self, port):
        self.responses = queue.Queue()
        self.ws = None
        self.connected = threading.Event()
        self.server = serve(self._handler, "0.0.0.0", port,
                            select_subprotocol=lambda conn, protos: "ocpp2.1")
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()

    def _handler(self, ws):
        self.ws = ws
        self.connected.set()
        try:
            for raw in ws:
                msg = json.loads(raw)
                if msg[0] == 2:
                    _, msg_id, action, payload = msg
                    if action == "BootNotification":
                        ws.send(json.dumps([3, msg_id, {
                            "currentTime": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
                            "interval": 300, "status": "Accepted"}]))
                    elif action == "Heartbeat":
                        ws.send(json.dumps([3, msg_id, {
                            "currentTime": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())}]))
                    else:
                        ws.send(json.dumps([3, msg_id, {}]))
                elif msg[0] == 3:
                    self.responses.put((msg[1], msg[2]))
        except Exception:
            pass
        finally:
            self.connected.clear()

    def call(self, action, payload, timeout=30):
        msg_id = str(uuid.uuid4())
        self.ws.send(json.dumps([2, msg_id, action, payload]))
        deadline = time.time() + timeout
        while time.time() < deadline:
            rid, resp = self.responses.get(timeout=deadline - time.time())
            if rid == msg_id:
                return resp
        raise TimeoutError(action)

    def get_variables(self, requests):
        payload = {"getVariableData": [
            {"component": {"name": comp}, "variable": ({"name": var, "instance": inst} if inst else {"name": var})}
            for comp, var, inst in requests]}
        return self.call("GetVariables", payload)["getVariableResult"]

    def set_variable(self, comp, var, value):
        payload = {"setVariableData": [
            {"component": {"name": comp}, "variable": {"name": var}, "attributeValue": value}]}
        return self.call("SetVariables", payload)["setVariableResult"][0]["attributeStatus"]

    def stop(self):
        self.server.shutdown()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--charger", required=True)
    p.add_argument("--iface")
    p.add_argument("--port", type=int, default=9500)
    p.add_argument("--keep-store", action="store_true",
                   help="skip ocpp/reset during cleanup, the test EVSEID stays persisted")
    args = p.parse_args()

    common.require_iso_tls_config(args.charger)
    iface = args.iface or common.default_iface(args.charger)
    local_ip = common.local_ip_towards(args.charger)

    try:
        from local_test_sap_tls12 import session_setup_evseid
        have_codec = True
    except ImportError:
        have_codec = False

    failures = 0

    def check(name, ok, detail=""):
        nonlocal failures
        print(f'{"ok  " if ok else "FAIL"} {name}{": " + str(detail) if detail else ""}')
        failures += 0 if ok else 1

    saved_config = common.api_get(args.charger, "ocpp/config")
    csms = Csms(args.port)
    try:
        test_config = dict(saved_config)
        test_config.update({"enable": True, "protocol": 1, "url": f"ws://{local_ip}:{args.port}",
                            "enable_auth": False, "pass": ""})
        common.api_put(args.charger, "ocpp/config_update", test_config)

        if not csms.connected.wait(timeout=60):
            raise SystemExit("charger did not connect to the embedded CSMS")
        time.sleep(2)

        results = csms.get_variables([("ISO15118Ctrlr", "ProtocolSupported", str(i)) for i in range(1, 5)])
        values = [r.get("attributeValue") for r in results]
        check("ProtocolSupported 1..3 filled", [r["attributeStatus"] for r in results[:3]] == ["Accepted"] * 3
              and values[:3] == ["urn:din:70121:2012:MsgDef,2,0",
                                 "urn:iso:15118:2:2013:MsgDef,2,0",
                                 "urn:iso:std:iso:15118:-20:AC,1,0"], values[:3])
        check("ProtocolSupported 4 UnknownVariable", results[3]["attributeStatus"] == "UnknownVariable")

        results = csms.get_variables([("ISO15118Ctrlr", "PWMChargingFallbackTimeout", None),
                                      ("ISO15118Ctrlr", "ISO15118EvseId", None),
                                      ("ISO15118Ctrlr", "EnforceTlsEnabled", None)])
        check("PWMChargingFallbackTimeout default 7", results[0]["attributeValue"] == "7")
        check("ISO15118EvseId default ZZ00000", results[1]["attributeValue"] == "ZZ00000")
        check("EnforceTlsEnabled default false", results[2]["attributeValue"] == "false")

        check("SetVariables ISO15118EvseId",
              csms.set_variable("ISO15118Ctrlr", "ISO15118EvseId", "DE*TNK*E123456") == "Accepted")

        common.enable_debug_mode(args.charger)
        time.sleep(2)

        if have_codec:
            evseid = session_setup_evseid(args.charger, iface)
            check("SessionSetupRes carries the OCPP EVSEID", evseid == "DE*TNK*E123456", evseid)
        else:
            print("skip SessionSetupRes EVSEID check, EXI codec not importable (run with the evsim venv)")

        check("SetVariables EnforceTlsEnabled true",
              csms.set_variable("ISO15118Ctrlr", "EnforceTlsEnabled", "true") == "Accepted")
        time.sleep(1)
        check("no-TLS SDP refused while enforced",
              common.sdp_request(iface, common.SDP_SECURITY_NO_TLS) is None)
        res = common.sdp_request(iface)
        check("TLS SDP still answered with TLS", res is not None and res["security"] == common.SDP_SECURITY_TLS)

        check("SetVariables EnforceTlsEnabled false",
              csms.set_variable("ISO15118Ctrlr", "EnforceTlsEnabled", "false") == "Accepted")
        time.sleep(1)
        res = common.sdp_request(iface, common.SDP_SECURITY_NO_TLS)
        check("no-TLS SDP answered again", res is not None and res["security"] == common.SDP_SECURITY_NO_TLS)
    finally:
        try:
            disabled = dict(saved_config)
            disabled["enable"] = False
            common.api_put(args.charger, "ocpp/config_update", disabled)
            time.sleep(2)
            if not args.keep_store:
                common.api_put(args.charger, "ocpp/reset", None)
            common.api_put(args.charger, "ocpp/config_update", saved_config)
        except Exception as e:
            print(f"cleanup failed, restore the ocpp config manually: {e}")
        csms.stop()

    print("PASS" if failures == 0 else f"FAIL ({failures} failures)")
    sys.exit(0 if failures == 0 else 1)


if __name__ == "__main__":
    main()
