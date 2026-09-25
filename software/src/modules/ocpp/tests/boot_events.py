#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import time
from datetime import datetime, timezone

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import TestContext, run_testsuite
from software.src.modules.iso15118.tests._common import CSMSSim, LocalCSMSTls


def test_boot_and_remote_reset_availability(tc: TestContext):
    tc.set_test_timeout(180)
    if not tc.device_type().is_warp(4):
        tc.skip("Requires WARP4")
    saved = tc.api("ocpp/config")
    saved_certs = tc.api("certs/state")
    tls = None
    csms = None
    try:
        tc.api("ocpp/config_update", dict(saved, enable=False))
        time.sleep(1)
        local_ip = tc.get_local_ip()
        tls = LocalCSMSTls(tc._esp_host, local_ip)
        csms = CSMSSim(certfile=str(tls.certfile), keyfile=str(tls.keyfile),
                       record_calls=True,
                       interactive=("BootNotification",),
                       expected_basic_auth=("warp4-boot-events", "boot-events-password"))
        config = dict(saved, enable=True, protocol=1, url=f"wss://{local_ip}:{csms.port}",
                      identity="warp4-boot-events", enable_auth=True,
                      cert_id=tls.cert_id, **{"pass": "boot-events-password"})
        tc.api("ocpp/config_update", config)
        expected_readback = tc.api("ocpp/config")
        boot, mid = csms.expect("BootNotification")
        # Finish setup registration before the actual serial-reset test.
        def answer(mid, status="Accepted", interval=2):
            csms.respond(mid, {"status": status, "interval": interval,
                               "currentTime": datetime.now(timezone.utc).isoformat()})

        def calls(connection):
            return [(a, p) for n, a, p in csms.received_calls if n == connection]

        def registered_snapshot(connection):
            def events():
                return [p for a, p in calls(connection) if a == "NotifyEvent"]
            tc.wait_for(lambda: tc.assert_eq(1, len(events())), timeout=10)
            event = events()[0]
            tc.assert_eq(0, event["seqNo"])
            tc.assert_eq(False, event.get("tbc", False))
            tc.assert_eq(1, len(event["eventData"]))
            data = event["eventData"][0]
            tc.assert_eq({"name": "Connector", "evse": {"id": 1, "connectorId": 1}}, data["component"])
            tc.assert_eq({"name": "AvailabilityState"}, data["variable"])
            tc.assert_(data["actualValue"] in ("Available", "Occupied", "Reserved", "Unavailable", "Faulted"))
            tc.assert_eq("HardWiredNotification", data["eventNotificationType"])
            tc.assert_eq("Alerting", data["trigger"])
            tc.assert_eq(8, data["severity"])
            tc.assert_("variableMonitoringId" not in data)
            for stamp in (event["generatedAt"], data["timestamp"]):
                tc.assert_(abs((datetime.now(timezone.utc) - datetime.fromisoformat(stamp.replace("Z", "+00:00"))).total_seconds()) < 30)
            tc.wait_for(lambda: tc.assert_(any(a == "Heartbeat" for a, _ in calls(connection))), timeout=10)
            tc.assert_eq(1, len(events()))
            actual = csms.call("GetVariables", {"getVariableData": [{
                "component": data["component"], "variable": data["variable"],
            }]})["getVariableResult"][0]
            tc.assert_eq("Accepted", actual["attributeStatus"])
            tc.assert_eq(data["actualValue"], actual["attributeValue"])
            print(f"Connector availability snapshot matches device readback: {data['actualValue']}")

        answer(mid)
        registered_snapshot(csms.connection_count)
        previous = csms.connection_count
        tc.reboot()
        csms.wait_for_connection(after=previous, timeout=60)
        boot, mid = csms.expect("BootNotification")
        connection = csms.connection_count
        tc.assert_eq("PowerUp", boot["reason"])
        identity = boot["chargingStation"]
        for key in ("serialNumber", "model", "vendorName", "firmwareVersion"):
            tc.assert_(isinstance(identity[key], str) and bool(identity[key]))
        answer(mid, "Pending", 2)
        boot_retry, mid = csms.expect("BootNotification", timeout=10)
        tc.assert_eq("PowerUp", boot_retry["reason"])
        tc.assert_eq(["BootNotification", "BootNotification"], [a for a, _ in calls(connection)])
        answer(mid)
        registered_snapshot(connection)
        print("Serial-reset boot: Pending gates requests; acceptance releases one Connector snapshot and heartbeat")

        previous = csms.connection_count
        tc.assert_eq("Accepted", csms.call("Reset", {"type": "Immediate"})["status"])
        csms.wait_for_connection(after=previous, timeout=60)
        boot, mid = csms.expect("BootNotification")
        connection = csms.connection_count
        tc.assert_eq("RemoteReset", boot["reason"])
        tc.assert_eq(identity, boot["chargingStation"])
        time.sleep(0.5)
        tc.assert_eq(["BootNotification"], [a for a, _ in calls(connection)])
        answer(mid)
        registered_snapshot(connection)
        tc.assert_eq([], csms.security_events)
        tc.assert_eq(expected_readback, tc.api("ocpp/config"))
        print("Remote Reset: Accepted response, reconnect, RemoteReset first, unchanged identity/config and post-acceptance snapshot passed")
    finally:
        errors = []
        try:
            tc.api("ocpp/config_update", dict(saved, enable=False), timeout=15)
            time.sleep(1)
            tc.api("ocpp/config_update", saved, timeout=15)
            tc.assert_eq(saved, tc.api("ocpp/config"))
        except Exception as e:
            errors.append(e)
        if csms is not None:
            csms.stop()
        if tls is not None:
            try:
                tls.close()
                tc.assert_eq(saved_certs, tc.api("certs/state"))
            except Exception as e:
                errors.append(e)
        if errors:
            raise errors[0]


if __name__ == "__main__":
    run_testsuite(locals())
