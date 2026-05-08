#!/usr/bin/env -S uv run --group tests --script

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from pathlib import Path
import json
import time

from software.src.modules.ocpp.tests._common import *

def suite_setup(tc: TestContext):
    tc.restore_before_suite_teardown('evse/ocpp_enabled')
    tc.restore_before_suite_teardown('ocpp/config')

def teardown(tc: TestContext):
    tc.get_testbox().set_cp('A')
    time.sleep(1)
    tc.api('ocpp/config', {
        'enable': False,
        'url': None,
        'identity': None,
        'enable_auth': None,
        'pass': None,
        'cert_id': None
    })
    time.sleep(1)


def verify_transaction(tc: TestContext, tag_id: str):
    tc.wait_for(lambda: tc.assert_true(tc.api("ocpp/state/connected")))
    tc.wait_for(lambda: tc.assert_eq(2, tc.api("ocpp/state/charge_point_state")))

    tb = tc.get_testbox()
    tb.set_cp('C')

    tc.wait_for(lambda: tc.assert_eq(1, tc.api("ocpp/state/connector_status")))

    tc.api('nfc/inject_tag', {"tag_type": 2, "tag_id": ":".join(tag_id[i:i+2] for i in range(0, len(tag_id), 2))})

    tc.wait_for(lambda: tc.assert_eq(2, tc.api("ocpp/state/connector_status")))
    tc.wait_for(lambda: tc.assert_(tb.is_contactor_closed()))

    tc.api('nfc/inject_tag', {"tag_type": 2, "tag_id": "00"})

    tb.set_cp('B')
    tc.wait_for(lambda: tc.assert_eq(3, tc.api("ocpp/state/connector_status")))
    tc.wait_for(lambda: tc.assert_false(tb.is_contactor_closed()))

    tb.set_cp('C')
    tc.wait_for(lambda: tc.assert_eq(2, tc.api("ocpp/state/connector_status")))
    tc.wait_for(lambda: tc.assert_(tb.is_contactor_closed()))

    tb.set_cp('B')
    tc.wait_for(lambda: tc.assert_false(tb.is_contactor_closed()))
    tc.api('nfc/inject_tag', {"tag_type": 2, "tag_id": ":".join(tag_id[i:i+2] for i in range(0, len(tag_id), 2))})

    tc.wait_for(lambda: tc.assert_eq(5, tc.api("ocpp/state/connector_status")))

    tc.api('nfc/inject_tag', {"tag_type": 2, "tag_id": "00"})

    tb.set_cp('A')
    tc.wait_for(lambda: tc.assert_eq(0, tc.api("ocpp/state/connector_status")))

def test_ladefoxx(tc: TestContext):
    cred_path = Path(__file__).parent / "ladefoxx_credentials.json"

    if not cred_path.is_file():
        tc.skip("Ladefoxx credentials missing")

    creds = json.loads(cred_path.read_text())
    tag_id = creds["tag_id"]
    del creds["tag_id"]

    tc.api('evse/ocpp_enabled', True)

    tc.api('ocpp/config', creds | {
        'enable': True,
        'enable_auth': True,
        'cert_id': -1
        })

    verify_transaction(tc, tag_id)

def test_ecarup_wss(tc: TestContext):
    cred_path = Path(__file__).parent / "ecarup_credentials.json"

    if not cred_path.is_file():
        tc.skip("eCarUp credentials missing")

    creds = json.loads(cred_path.read_text())
    tag_id = creds["tag_id"]
    del creds["tag_id"]

    tc.api('evse/ocpp_enabled', True)

    tc.api('ocpp/config', creds | {
        'enable': True,
        'enable_auth': True,
        'cert_id': -1
        })

    verify_transaction(tc, tag_id)

def test_ecarup_ws(tc: TestContext):
    cred_path = Path(__file__).parent / "ecarup_credentials.json"

    if not cred_path.is_file():
        tc.skip("eCarUp credentials missing")

    creds = json.loads(cred_path.read_text())
    creds["url"] = creds["url"].replace("wss://", "ws://")
    tag_id = creds["tag_id"]
    del creds["tag_id"]

    tc.api('evse/ocpp_enabled', True)

    tc.api('ocpp/config', creds | {
        'enable': True,
        'enable_auth': True,
        'cert_id': -1
        })

    verify_transaction(tc, tag_id)
