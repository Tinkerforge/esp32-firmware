#!/usr/bin/env -S uv run --group tests --script

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext

import time

from tinkerforge.ip_connection import IPConnection
from software.test_runner.testbox.bricklet_evse import BrickletEVSE
from software.test_runner.testbox.bricklet_evse_v2 import BrickletEVSEV2

from urllib.error import HTTPError


def set_test_mode_enabled(enabled: bool, tc):
    identity = tc.api('evse/identity')
    did, uid = identity['device_identifier'], identity['uid']

    ipcon = IPConnection()

    if did == 2159:
        evse = BrickletEVSE(uid, ipcon)
    elif did == 2167:
        evse = BrickletEVSEV2(uid, ipcon)

    tc.http_request('GET', '/hidden_proxy/enable')
    try:
        ipcon.connect(tc._esp_host, 4223)
        try:
            evse.set_test_mode(enabled, 0xdeadbeef)
        finally:
            ipcon.disconnect()
    finally:
        tc.http_request('GET', '/hidden_proxy/disable')


def suite_setup(tc: TestContext):
    set_test_mode_enabled(True, tc)


def suite_teardown(tc: TestContext):
    try:
        set_test_mode_enabled(False, tc)
    except Exception:
        # Reset test box first and wait for the EVSE to see state A and a DC fault protector calibration.
        tc.get_testbox().reset()
        time.sleep(6.5)
        tc.http_request('GET', '/evse/reset')
        # Wait for second DC fault protector calibration
        time.sleep(5)
        raise
    finally:
        tc.get_testbox().reset()


def teardown(tc: TestContext):
    state = tc.api('evse/state/charger_state')

    tc.get_testbox().reset()

    if state != 0:
        time.sleep(6.5)
        return

    ll_state = tc.api('evse/low_level_state')

    t = min(5000 - ll_state['time_since_state_change'],
            5000 - ll_state['time_since_dc_fault_check'])
    t = max(0, t)

    time.sleep(t / 1000)




def test_cp(tc: TestContext):
    b = tc.get_testbox()

    for iec, chrg in [
        ('A', 0),
        ('B', 2),
        ('C', 3),
        ('D', 4),
        ('A', 0),
    ]:
        b.set_cp(iec)
        tc.assert_eq(iec, b.get_cp())

        tc.wait_for(lambda: tc.assert_eq(chrg, tc.api('evse/state/charger_state')))


def test_btn(tc: TestContext):
    b = tc.get_testbox()

    press = tc.api('evse/button_state/button_press_time')
    release = tc.api('evse/button_state/button_release_time')

    def button_pressed():
        return tc.api('evse/button_state/button_pressed')

    for p in [True, False, True, False]:
        tc.assert_ne(p, button_pressed())

        b.set_button_pressed(p)
        tc.assert_eq(p, b.is_button_pressed())

        tc.wait_for(lambda: tc.assert_eq(p, button_pressed()))

    tc.wait_for(lambda: tc.assert_gt(press, tc.api('evse/button_state/button_press_time')))
    tc.wait_for(lambda: tc.assert_gt(release, tc.api('evse/button_state/button_release_time')))


def test_gpio_shutdown(tc: TestContext):
    b = tc.get_testbox()

    def shutdown_closed():
        # GPIOs are inverted
        return not tc.api('evse/low_level_state/gpio')[b.get_gpio_index(b.GPIO.SHUTDOWN_INPUT)]

    for p in [True, False, True, False]:
        tc.assert_ne(p, shutdown_closed())

        b.set_gpio_shutdown_closed(p)
        tc.assert_eq(p, b.is_gpio_shutdown_closed())

        tc.wait_for(lambda: tc.assert_eq(p, shutdown_closed()))


def test_gpio_input(tc: TestContext):
    b = tc.get_testbox()

    def input_closed():
        # GPIOs are inverted
        return not tc.api('evse/low_level_state/gpio')[b.get_gpio_index(b.GPIO.GP_INPUT)]

    for p in [True, False, True, False]:
        tc.assert_ne(p, input_closed())

        b.set_gpio_input_closed(p)
        tc.assert_eq(p, b.is_gpio_input_closed())

        tc.wait_for(lambda: tc.assert_eq(p, input_closed()))


def test_gpio_output(tc: TestContext):
    b = tc.get_testbox()

    for p in [False, True, False, True]:
        tc.assert_ne(p, b.is_gpio_output_closed())

        tc.api('evse/gp_output', int(p))

        tc.wait_for(lambda: tc.assert_eq(p, b.is_gpio_output_closed()))


def test_dc_fault_injection(tc: TestContext):
    b = tc.get_testbox()

    b.set_cp('C')
    tc.wait_for(lambda: tc.assert_eq(3, tc.api('evse/state/charger_state')))
    tc.assert_eq(0, tc.api('evse/state/error_state'))

    b.set_dc_faulted(True)
    tc.assert_(b.is_dc_fauled())

    # IEC 61851 allows 6 seconds to detect DC fault
    tc.wait_for(lambda: tc.assert_eq(3, tc.api('evse/state/error_state')), timeout=7)

    b.set_dc_faulted(False)
    tc.assert_false(b.is_dc_fauled())

    b.set_cp('A')
    tc.api('evse/reset_dc_fault_current_state', 0xDC42FA23)

    tc.wait_for(lambda: tc.assert_eq(0, tc.api('evse/state/error_state')))
    tc.wait_for(lambda: tc.assert_eq(0, tc.api('evse/state/charger_state')))


def test_diode_fault_injection(tc: TestContext):
    b = tc.get_testbox()

    b.set_cp('C')
    tc.wait_for(lambda: tc.assert_eq(3, tc.api('evse/state/charger_state')))
    tc.assert_eq(0, tc.api('evse/state/error_state'))

    b.set_diode_faulted(True)
    tc.assert_(b.is_diode_fauled())

    tc.wait_for(lambda: tc.assert_eq(5, tc.api('evse/state/error_state')))

    b.set_diode_faulted(False)
    tc.assert_false(b.is_diode_fauled())

    b.set_cp('A')

    tc.wait_for(lambda: tc.assert_eq(0, tc.api('evse/state/error_state')))
    tc.wait_for(lambda: tc.assert_eq(0, tc.api('evse/state/charger_state')))


def test_contactor_read_back(tc: TestContext):
    b = tc.get_testbox()

    tc.assert_false(b.is_contactor_closed(0))
    tc.assert_false(b.is_contactor_closed(1))

    b.set_cp('C')
    tc.wait_for(lambda: tc.assert_eq(3, tc.api('evse/state/charger_state')))
    tc.wait_for(lambda: tc.assert_(b.is_contactor_closed(0)))
    tc.wait_for(lambda: tc.assert_(b.is_contactor_closed(1)))

    b.set_cp('A')
    tc.wait_for(lambda: tc.assert_eq(0, tc.api('evse/state/charger_state')))
    tc.wait_for(lambda: tc.assert_false(b.is_contactor_closed(0)))
    tc.wait_for(lambda: tc.assert_false(b.is_contactor_closed(1)))

    if tc.api('evse/hardware_configuration/evse_version') >= 30:
        tc.api('power_manager/external_control_update', 1)
        try:
            b.set_cp('C')
            tc.wait_for(lambda: tc.assert_eq(3, tc.api('evse/state/charger_state')))
            tc.wait_for(lambda: tc.assert_(b.is_contactor_closed(0)))
            tc.wait_for(lambda: tc.assert_false(b.is_contactor_closed(1)))
            time.sleep(1)
            tc.assert_false(b.is_contactor_closed(1))
        finally:
            b.set_cp('A')
            tc.wait_for(lambda: tc.assert_eq(0, tc.api('evse/state/charger_state')))
            tc.api('power_manager/external_control_update', 3)


def test_contactor_aux(tc: TestContext):
    b = tc.get_testbox()

    def contactor_closed(contactor: int):
        # GPIOs are inverted
        gpio = {
            0: b.GPIO.CONTACTOR_CHECK_L1_N,
            1: b.GPIO.CONTACTOR_CHECK_L2_L3
        }[contactor]

        return not tc.api('evse/low_level_state/gpio')[b.get_gpio_index(gpio)]

    tc.assert_eq('real', b.get_contactor_aux_state(0))
    tc.assert_eq('real', b.get_contactor_aux_state(1))
    tc.wait_for(lambda: tc.assert_false(contactor_closed(0)))
    tc.wait_for(lambda: tc.assert_false(contactor_closed(1)))

    for c, other in zip([0, 1], [1, 0]):
        for p in [True, False, True, False]:
            tc.assert_ne(p, contactor_closed(c))
            tc.assert_false(contactor_closed(other))

            b.connect_contactor_aux(c, 'closed' if p else 'open')
            tc.assert_eq('closed' if p else 'open', b.get_contactor_aux_state(c))

            tc.wait_for(lambda: tc.assert_eq(p, contactor_closed(c)))
            tc.assert_false(contactor_closed(other))

    b.connect_contactor_aux(0, 'real')
    b.connect_contactor_aux(1, 'real')
    tc.assert_eq('real', b.get_contactor_aux_state(0))
    tc.assert_eq('real', b.get_contactor_aux_state(1))
    tc.assert_false(contactor_closed(0))
    tc.assert_false(contactor_closed(1))


def test_meter_disconnect(tc: TestContext):
    b = tc.get_testbox()

    tc.assert_eq('real', b.get_meter_connected())

    meter_type = tc.assert_ne(0, tc.api("evse/hardware_configuration/energy_meter_type"))

    b.connect_meter('none')
    # WARP3 testboxes can't separate those cases
    tc.assert_in(['none', 'fake'], b.get_meter_connected())

    tc.http_request('GET', '/evse/reset')
    tc.wait_for(lambda: tc.assert_eq(0, tc.api("evse/hardware_configuration/energy_meter_type")), timeout=20)
    set_test_mode_enabled(True, tc)

    #time.sleep(1)
    uptime = tc.assert_le(13, tc.api("evse/low_level_state/uptime") / 1000)
    time.sleep(max(0, 10 - uptime))
    tc.assert_eq(0, tc.api("evse/hardware_configuration/energy_meter_type"))

    b.connect_meter('real')
    tc.assert_eq('real', b.get_meter_connected())
    tc.wait_for(lambda: tc.assert_eq(meter_type, tc.api("evse/hardware_configuration/energy_meter_type")), timeout=10)


if __name__ == '__main__':
    run_testsuite(locals())
