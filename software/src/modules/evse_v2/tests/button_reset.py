#!/usr/bin/env -S uv run --group tests --script

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext

import json
import time

from urllib.error import URLError, HTTPError

from software.test_runner.testbox.bricklet_evse_v2 import BrickletEVSEV2

RECOVERY_STATE_PATH = "/.evse_v2_recovery"

# setup will change the ESP's hostname to a marker value.
# Connect to the ESP's IP directly and restore the hostname after all tests are done.
original_host = None


def suite_setup(tc: TestContext) -> None:
    global original_host
    original_host = tc._esp_host
    tc._esp_host = tc.api('ethernet/state/ip')


def suite_teardown(tc: TestContext) -> None:
    tc.wait_for_reboot(1)

    for f in tc.list_dir('/'):
        if 'button_reset_test' in f:
            tc.remove_file(f)

    for f in tc.list_dir('/config'):
        if 'button_reset_test' in f:
            tc.remove_file(f)

    tc._esp_host = original_host


last_setup_run = None


def teardown(tc: TestContext):
    tc.remove_file(RECOVERY_STATE_PATH, missing_ok=True)

    with tc.Bricklet(tc, BrickletEVSEV2) as evse_v2:
        evse_v2.get_button_press_boot_time(True)


def setup(tc: TestContext) -> None:
    global last_setup_run
    last_setup_run = f'{int(time.time()):x}'
    if len(last_setup_run) != 8:
        raise Exception(f'Unexpected length {len(last_setup_run)} of last_setup_run {last_setup_run}')

    tc.restore_before_suite_teardown('wifi/sta_config')
    tc.restore_before_suite_teardown('wifi/ap_config')
    tc.restore_before_suite_teardown('network/config')
    tc.restore_before_suite_teardown('ocpp/config')
    tc.restore_before_suite_teardown('ethernet/config')

    tc.api('wifi/sta_config/ip6', [{'addr': f'fef0::{last_setup_run[:4]}:{last_setup_run[4:]}', 'flags': 0}])
    tc.api('wifi/ap_config/ssid', f'button_reset_test_{last_setup_run}')
    tc.api('network/config/hostname', f'button_reset_test_{last_setup_run}')

    user_config = tc.api('users/config')
    if not user_config['http_auth_enabled']:
        user_config['http_auth_enabled'] = True
        tc.upload_file("/config/users_config", json.dumps(user_config).encode('utf-8'))
        tc.reboot()

    tc.api('ocpp/config/identity', f'button_reset_test_{last_setup_run}')
    tc.upload_file(f'/config/button_reset_test_{last_setup_run}', f'button_reset_test_{last_setup_run}'.encode('utf-8'))
    tc.upload_file(f'/button_reset_test_{last_setup_run}', f'button_reset_test_{last_setup_run}'.encode('utf-8'))

    tc.api('ethernet/config/ip6', [{'addr': f'fef0::{last_setup_run[:4]}:{last_setup_run[4:]}', 'flags': 0}])
    tc.wait_for_reboot(1)


def is_secure(tc: TestContext) -> bool:
    try:
        return tc.http_request('GET', '/esp32/security_info', parse=True)['app_encrypted']
    except HTTPError as e:
        if e.code == 404:
            return False
        raise


def verify_stage_0(tc: TestContext, /, last_stage_executed) -> None:
    global last_setup_run

    assert_fn = tc.assert_ne if last_stage_executed >= 0 else tc.assert_eq

    assert_fn([{'addr': 'fef0::{last_setup_run[:4]}:{last_setup_run[4:]}', 'flags': 0}], tc.api('ethernet/config/ip6'))
    assert_fn([{'addr': 'fef0::{last_setup_run[:4]}:{last_setup_run[4:]}', 'flags': 0}], tc.api('wifi/sta_config/ip6'))
    assert_fn(f'button_reset_test_{last_setup_run}', tc.api('wifi/ap_config/ssid'))
    assert_fn(f'button_reset_test_{last_setup_run}', tc.api('network/config/hostname'))

    if last_stage_executed >= 2 or (last_stage_executed >= 0 and not is_secure(tc)):
        tc.assert_false(tc.api('users/config/http_auth_enabled'))


def verify_stage_1(tc: TestContext, /, last_stage_executed) -> None:
    global last_setup_run

    if last_stage_executed >= 2 or (last_stage_executed >= 1 and not is_secure(tc)):
        tc.assert_ne(f'button_reset_test_{last_setup_run}', tc.api('ocpp/config/identity'))
        tc.assert_fail(lambda: tc.assert_contains(f'/config/button_reset_test_{last_setup_run}', tc.list_dir('/config')))
    else:
        tc.assert_eq(f'button_reset_test_{last_setup_run}', tc.api('ocpp/config/identity'))
        tc.assert_contains(f'/config/button_reset_test_{last_setup_run}', tc.list_dir('/config'))


def verify_stage_2(tc: TestContext, /, last_stage_executed) -> None:
    global last_setup_run

    if last_stage_executed >= 2:
        tc.assert_fail(lambda: tc.assert_contains(f'/button_reset_test_{last_setup_run}', tc.list_dir('/')))
    else:
        tc.assert_contains(f'/button_reset_test_{last_setup_run}', tc.list_dir('/'))

    if last_stage_executed >= 0 and not is_secure(tc) or last_stage_executed >= 2:
        tc.assert_false(tc.api('users/config/http_auth_enabled'))
    else:
        tc.assert_true(tc.api('users/config/http_auth_enabled'))


def emulate_power_cycle(tc: TestContext) -> None:
    tc.api('evse/reset', None)
    tc.reboot(wait_for_host_responsive=False)


def power_cycle_with_button(tc: TestContext) -> None:
    tc.get_testbox().set_button_pressed(True)
    time.sleep(1)  # educated guess
    emulate_power_cycle(tc)

    time.sleep(13)
    tc.get_testbox().set_button_pressed(False)
    tc.wait_for_reboot(0)
    time.sleep(1)


def test_crash_advances_stage(tc: TestContext) -> None:
    power_cycle_with_button(tc)

    verify_stage_0(tc, last_stage_executed=0)
    verify_stage_1(tc, last_stage_executed=0)
    verify_stage_2(tc, last_stage_executed=0)

    try:
        tc.http_request('GET', '/debug/crash', timeout=1)
    except TimeoutError:
        pass
    except URLError as e:
        if not isinstance(e.reason, TimeoutError):
            raise

    tc.wait_for_reboot()

    verify_stage_0(tc, last_stage_executed=1)
    verify_stage_1(tc, last_stage_executed=1)
    verify_stage_2(tc, last_stage_executed=1)


def test_power_cycle_w_o_button_is_nop(tc: TestContext) -> None:
    power_cycle_with_button(tc)

    verify_stage_0(tc, last_stage_executed=0)
    verify_stage_1(tc, last_stage_executed=0)
    verify_stage_2(tc, last_stage_executed=0)

    emulate_power_cycle(tc)

    tc.wait_for_reboot()

    verify_stage_0(tc, last_stage_executed=1)
    verify_stage_1(tc, last_stage_executed=1)
    verify_stage_2(tc, last_stage_executed=1)


def test_no_crash_clears_recovery_state(tc: TestContext) -> None:
    power_cycle_with_button(tc)

    verify_stage_0(tc, last_stage_executed=0)
    verify_stage_1(tc, last_stage_executed=0)
    verify_stage_2(tc, last_stage_executed=0)

    tc.wait_for(lambda:
        tc.assert_fail(lambda:
            tc.assert_contains(RECOVERY_STATE_PATH, tc.list_dir("/"))), poll_delay=1, timeout=60)

    with tc.Bricklet(tc, BrickletEVSEV2) as evse_v2:
        tc.wait_for(lambda: tc.assert_eq(0, evse_v2.get_button_press_boot_time(False)))


def test_stages(tc: TestContext) -> None:
    power_cycle_with_button(tc)

    verify_stage_0(tc, last_stage_executed=0)
    verify_stage_1(tc, last_stage_executed=0)
    verify_stage_2(tc, last_stage_executed=0)

    power_cycle_with_button(tc)

    verify_stage_0(tc, last_stage_executed=1)
    verify_stage_1(tc, last_stage_executed=1)
    verify_stage_2(tc, last_stage_executed=1)

    power_cycle_with_button(tc)

    verify_stage_0(tc, last_stage_executed=2)
    verify_stage_1(tc, last_stage_executed=2)
    verify_stage_2(tc, last_stage_executed=2)


if __name__ == '__main__':
    run_testsuite(locals())
