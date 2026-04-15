#!/usr/bin/env -S uv run --group tests --script

from ocpp.routing import on
from ocpp.exceptions import NotImplementedError
from ocpp.v16 import ChargePoint
from ocpp.v16 import call_result
from ocpp.v16.enums import Action, RegistrationStatus

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from datetime import datetime, timezone

from software.src.modules.ocpp.tests._common import *

def suite_setup(tc: TestContext):
    tc.restore_before_suite_teardown('ocpp/config')

def test_connect(tc: TestContext):
    connected_identity = ""

    class MyChargePoint(ChargePoint):
        async def _handle_call(self, msg):
            try:
                return await super()._handle_call(msg)
            except NotImplementedError:
                return None

        @on(Action.boot_notification)
        async def on_boot_notification(
            self, charge_point_vendor, charge_point_model, **kwargs
        ):
            nonlocal connected_identity
            connected_identity = self.id

            return call_result.BootNotification(
                current_time=datetime.now(tz=timezone.utc).isoformat(),
                interval=10,
                status=RegistrationStatus.accepted,
            )

    central = TestOcppCentral(lambda *args: MyChargePoint(*args), port=tc.find_free_port(9000))
    central.start()

    tc.api('ocpp/config', {
        'enable': True,
        'url': f"ws://{tc.get_local_ip()}:{central.port}",
        'identity': 'warp-ocpp-test',
        'enable_auth': False,
        'pass': "",
        'cert_id': -1
    })

    tc.wait_for(lambda: tc.assert_eq('warp-ocpp-test', connected_identity), timeout=20)

if __name__ == "__main__":
    run_testsuite(locals())
