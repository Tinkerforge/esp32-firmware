#!/usr/bin/env -S uv run --group tests --script

from contextlib import ExitStack, contextmanager
from functools import partial
import math
import os
import struct
import time
from uuid import uuid4

from pymodbus.client import ModbusTcpClient
from pymodbus.constants import ExcCodes
from pymodbus.exceptions import ModbusException
from pymodbus.pdu import ExceptionResponse
from tinkerforge.ip_connection import base58decode

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import TestContext, run_testsuite
from software.src.modules.modbus_tcp.tests.modbus_reader import (
    BENDER_REGISTER_MAP,
    KEBA_REGISTER_MAP,
    WARP_REGISTER_MAP,
    read_all_regs,
    unpack_registers,
)


@contextmanager
def modbus_session(tc: TestContext, table, *, required_features, required_modules=(), testbox=None, **options):
    features = tc.api('info/features')
    missing = sorted(set(required_features) - set(features))
    if missing:
        tc.skip(f'Missing features: {", ".join(missing)}')
    missing = sorted({'modbus_tcp', *required_modules} - set(tc.api('info/modules')))
    if missing:
        tc.skip(f'Missing modules: {", ".join(missing)}')

    rw_api = 'evse/modbus_tcp_enabled'
    config_api = 'modbus_tcp/config'
    saved_rw = tc.api(rw_api)
    saved_config = tc.api(config_api)
    config = {**saved_config, 'enable': True, 'table': table, **options}
    client = ModbusTcpClient(tc._esp_host, port=config['port'], timeout=1, retries=0)

    def assert_stopped():
        client.close()
        try:
            tc.assert_false(client.connect())
        finally:
            client.close()

    def stop_listener():
        tc.api(config_api + '_update', {**config, 'enable': False})
        tc.wait_for(assert_stopped)

    def assert_idle():
        state = tc.api('evse/state')
        tc.assert_eq(0, state['iec61851_state'])
        tc.assert_eq(0, state['charger_state'])

    with ExitStack() as cleanup:
        cleanup.callback(tc.api, config_api + '_update', saved_config)
        cleanup.callback(tc.api, rw_api + '_update', saved_rw)
        if testbox is not None:
            autostart_api = 'evse/auto_start_charging'
            cleanup.callback(tc.api, autostart_api + '_update', tc.api(autostart_api))
            cleanup.callback(testbox.set_cp, 'A')
            testbox.set_cp('A')
            tc.api(autostart_api + '_update', {'auto_start_charging': True})

        tc.wait_for(assert_idle)
        tc.api(rw_api + '_update', {'enabled': False})
        tc.wait_for(lambda: tc.assert_false(tc.api('evse/slots/9/active')))

        # The persisted enable flag can change before the firmware closes the listener.
        stop_listener()
        cleanup.callback(stop_listener)
        tc.api(config_api + '_update', config)

        cleanup.callback(client.close)
        yield client


def build_table(tc, client, table):
    # A connection can succeed before the configuration event has restarted the server.
    reads = wait_for_modbus(tc, client, lambda: read_all_regs(client, table))
    regs = {}
    for block in table:
        bank = regs.setdefault(block.reg_type, {})
        for _, _, name, _ in reads[block.name]:
            if name in bank:
                tc.fail(f'Duplicate {block.reg_type} field {name}')
            bank[name] = partial(read_register, block, name, client)
    return regs


def read_register(block, name, client):
    return block.read_single(name, client)[-1]


def check_write(tc, response):
    if response.isError():
        tc.fail(f'Modbus write failed: {response}')


def check_registers(tc, response, expected):
    if response.isError():
        tc.fail(f'Modbus read failed: {response}')
    tc.assert_eq(expected, response.registers)


def check_bits(tc, response, expected):
    if response.isError():
        tc.fail(f'Modbus read failed: {response}')
    tc.assert_eq(expected, response.bits[:len(expected)])


def check_exception(tc, response, code):
    if not isinstance(response, ExceptionResponse):
        tc.fail(f'Expected Modbus exception {code.name}, got {response}')
    tc.assert_eq(code, response.exception_code)


def u32_words(value):
    return [value >> 16, value & 0xFFFF]


def read_u32(read, address):
    response = read(address, count=2)
    if response.isError():
        raise ModbusException(f'Modbus read at {address} failed: {response}')
    high, low = response.registers
    return high << 16 | low


def write_u32(tc, client, address, value):
    check_write(tc, client.write_registers(address, u32_words(value)))


def wait_for_modbus(tc, client, read):
    def assert_readable():
        try:
            return read()
        except (ModbusException, OSError) as exc:
            client.close()
            tc.fail(str(exc))

    return tc.wait_for(assert_readable)


def enable_writes(tc, client, read):
    tc.api('evse/modbus_tcp_enabled_update', {'enabled': True})

    def assert_old_connection_closed():
        try:
            read()
        except (ModbusException, OSError):
            return
        tc.fail('The Modbus server still accepts requests on the old connection')

    # Wait for the restart to close this connection before opening another one.
    tc.wait_for(assert_old_connection_closed)
    client.close()
    tc.wait_for(lambda: tc.assert_true(tc.api('evse/slots/9/active')))
    wait_for_modbus(tc, client, read)


def enable_warp_writes(tc, client):
    uptime = partial(read_u32, client.read_input_registers, 12)
    wait_for_modbus(tc, client, uptime)
    enable_writes(tc, client, uptime)
    return uptime


def verify_slots(tc, read_slots):
    slots = tc.api('evse/slots')
    expected = [s['max_current'] if s['active'] else 0xFFFFFFFF for s in slots]
    actual = read_slots()
    tc.assert_ge(len(expected), len(actual))
    tc.assert_eq(expected + [0xFFFFFFFF] * (len(actual) - len(expected)), actual)


def text_register(read):
    return read().rstrip(b'\x00').decode('ascii')


def assert_phase_meter_values(tc, expected, actual, *, scale=1, tolerance=1):
    for api_value, register_value in zip(expected, actual, strict=True):
        if api_value is None:
            continue
        tc.assert_epsilon(api_value * scale, tolerance, register_value)


def test_warp(tc: TestContext):
    testbox = tc.get_testbox()
    with modbus_session(
        tc, 0,
        required_features=('evse', 'meter', 'meter_phases', 'meter_all_values', 'nfc'),
        required_modules=('charge_tracker',),
        testbox=testbox,
        send_illegal_data_address=True,
        ignore_writes=False,
    ) as client:
        regs = build_table(tc, client, WARP_REGISTER_MAP)
        verify_warp_registers(tc, regs)
        verify_warp_read_only(tc, client, regs)
        verify_warp_charge_control(tc, client, testbox, regs)
        verify_warp_nfc(tc, client, regs['input'])


def verify_warp_registers(tc, regs):
    inputs, holding = regs['input'], regs['holding']

    tc.assert_eq(5, inputs['table_version']())
    version, timestamp = tc.api('info/version/firmware').split('+')
    release, _, beta = version.partition('-beta.')
    major, minor, patch = map(int, release.split('.'))
    tc.assert_eq(major, inputs['firmware_major']())
    tc.assert_eq(minor, inputs['firmware_minor']())
    tc.assert_eq(patch, inputs['firmware_patch']())
    tc.assert_eq(int(beta or 0), inputs['firmware_beta']())
    tc.assert_eq(int(timestamp, 16), inputs['firmware_build_ts']())
    tc.assert_eq(base58decode(tc.api('info/name/uid')), inputs['box_id']())

    uptime = inputs['uptime']()
    tc.wait_for(lambda: tc.assert_gt(uptime, inputs['uptime']()))

    tc.assert_eq(0, inputs['iec_state']())
    tc.assert_eq(0, inputs['charger_state']())
    tc.assert_eq(0xFFFFFFFF, inputs['current_user']())
    tc.assert_eq(0, inputs['start_time_min']())
    tc.assert_eq(0, inputs['charging_time_sec']())
    tc.assert_eq(tc.api('evse/state/allowed_charging_current'), inputs['max_current']())
    verify_slots(tc, inputs['slots'])

    tc.assert_eq(tc.api('meter/state/type'), inputs['meter_type']())
    tc.assert_epsilon(tc.api('meter/values/power'), 1, inputs['power']())
    tc.assert_epsilon(tc.api('meter/values/energy_rel'), 1, inputs['energy_relative']())
    tc.assert_epsilon(tc.api('meter/values/energy_abs'), 1, inputs['energy_absolute']())
    tc.assert_eq(0, inputs['energy_this_charge']())

    for expected, actual in zip(tc.api('meter/all_values'), inputs['meter_values'](), strict=True):
        if expected is None:
            tc.assert_true(math.isnan(actual))
        else:
            tc.assert_epsilon(expected, 1, actual)

    tc.assert_eq(0, holding['reboot']())
    tc.assert_eq(0, holding['trigger_reset']())
    tc.assert_eq(0xFFFFFFFF, holding['enable_charging_deprecated']())
    tc.assert_eq(0xFFFFFFFF, holding['allowed_current']())
    led = tc.api('evse/indicator_led')
    tc.assert_eq(led['indication'] & 0xFFFFFFFF, holding['led_blink_state']())
    tc.assert_eq(led['duration'], holding['led_blink_duration']())

    features = tc.api('info/features')
    discrete = regs['discrete']
    for feature in ('evse', 'meter', 'meter_phases', 'meter_all_values', 'phase_switch', 'nfc'):
        tc.assert_eq(feature in features, discrete[feature]())
    evse_version = tc.api('evse/hardware_configuration/evse_version')
    tc.assert_eq(evse_version >= 20, discrete['gp_output_supported']())
    tc.assert_eq(20 <= evse_version < 30, discrete['input_3_supported']())
    tc.assert_eq(20 <= evse_version < 30, discrete['input_4_supported']())
    phases = tc.api('meter/phases')
    for i, phase in enumerate(('one', 'two', 'three')):
        tc.assert_eq(phases['phases_connected'][i], discrete[f'phase_{phase}_connected']())
        tc.assert_eq(phases['phases_active'][i], discrete[f'phase_{phase}_active']())

    tc.assert_true(regs['coil']['enable_charging']())


def verify_warp_read_only(tc, client, regs):
    counter_api = 'modbus_tcp/error_counters/illegal_function'
    count = tc.api(counter_api)
    slots = tc.api('evse/slots')[9:11]
    check_exception(tc, client.write_coil(1000, False), ExcCodes.ILLEGAL_FUNCTION)
    check_exception(tc, client.write_registers(1002, [0, 6000]), ExcCodes.ILLEGAL_FUNCTION)
    tc.wait_for(lambda: tc.assert_eq(count + 2, tc.api(counter_api)))
    tc.assert_eq(slots, tc.api('evse/slots')[9:11])
    tc.assert_true(regs['coil']['enable_charging']())
    tc.assert_eq(0xFFFFFFFF, regs['holding']['allowed_current']())


def test_warp_address_handling(tc: TestContext):
    for strict in (True, False):
        with modbus_session(
            tc, 0,
            required_features=('evse',),
            send_illegal_data_address=strict,
            ignore_writes=False,
        ) as client:
            timestamp = int(tc.api('info/version/firmware').rsplit('+', 1)[1], 16)

            def verify_timestamp_high_word():
                check_registers(tc, client.read_input_registers(8, count=1), [timestamp >> 16])

            wait_for_modbus(tc, client, verify_timestamp_high_word)
            check_registers(tc, client.read_input_registers(9, count=1), [timestamp & 0xFFFF])

            counter_api = 'modbus_tcp/error_counters/illegal_data_address'
            count = tc.api(counter_api)
            gaps = (
                (client.read_input_registers, 16, 'registers', 0),
                (client.read_holding_registers, 2, 'registers', 0),
                (client.read_discrete_inputs, 9, 'bits', False),
                (client.read_coils, 1002, 'bits', False),
            )
            for read, address, field, zero in gaps:
                response = read(address, count=1)
                if strict:
                    check_exception(tc, response, ExcCodes.ILLEGAL_ADDRESS)
                else:
                    tc.assert_false(response.isError())
                    tc.assert_eq([zero], getattr(response, field)[:1])
            expected_count = count + len(gaps) if strict else count
            tc.wait_for(lambda: tc.assert_eq(expected_count, tc.api(counter_api)))

            enable_writes(tc, client, verify_timestamp_high_word)
            tc.assert_eq(0, tc.api(counter_api))
            writes = (
                client.write_coils(1002, [True, False]),
                client.write_registers(2, [0, 0]),
            )
            for response in writes:
                if strict:
                    check_exception(tc, response, ExcCodes.ILLEGAL_ADDRESS)
                else:
                    check_write(tc, response)
            tc.assert_eq(len(writes) if strict else 0, tc.api(counter_api))


def test_warp_partial_holding_writes(tc: TestContext):
    with modbus_session(
        tc, 0,
        required_features=('evse', 'nfc'),
        send_illegal_data_address=True,
        ignore_writes=False,
    ) as client:
        def verify_empty_buffer():
            check_registers(tc, client.read_holding_registers(4000, count=4), [0] * 4)

        wait_for_modbus(tc, client, verify_empty_buffer)
        enable_writes(tc, client, verify_empty_buffer)

        # Stage an NFC ID without writing the register that triggers injection.
        check_write(tc, client.write_registers(4000, [0x3031, 0x3233, 0x3435, 0x3637]))
        # Change the low word of one 32-bit value and the high word of the next.
        check_write(tc, client.write_registers(4001, [0x3839, 0x4142]))
        check_registers(tc, client.read_holding_registers(4001, count=2), [0x3839, 0x4142])
        check_registers(tc, client.read_holding_registers(4000, count=4), [0x3031, 0x3839, 0x4142, 0x3637])


def verify_warp_charge_control(tc, client, testbox, regs):
    inputs, holding, coils = regs['input'], regs['holding'], regs['coil']
    autostart_current = tc.api('evse/slots/4/max_current')
    tc.assert_eq(autostart_current > 0, coils['autostart_button']())

    # Enabling the server alone leaves the EVSE slots inactive.
    testbox.set_cp('C')
    tc.wait_for(lambda: tc.assert_eq(2, inputs['iec_state']()))
    tc.assert_eq(3, inputs['charger_state']())
    tc.wait_for(lambda: tc.assert_eq(0, inputs['current_user']()))
    tc.wait_for(lambda: tc.assert_epsilon(time.time() // 60, 1, inputs['start_time_min']()))
    charging_time = tc.wait_for(lambda: tc.assert_gt(0, inputs['charging_time_sec']()))
    tc.wait_for(lambda: tc.assert_gt(charging_time, inputs['charging_time_sec']()))
    meter_start = tc.api('charge_tracker/current_charge/meter_start')
    tc.wait_for(lambda: tc.assert_epsilon(tc.api('meter/values/energy_abs') - meter_start, 0.001, inputs['energy_this_charge']()))

    enable_writes(tc, client, inputs['iec_state'])
    tc.assert_fail(lambda: tc.wait_for(lambda: tc.assert_eq(1, inputs['iec_state']())))

    testbox.set_cp('A')
    tc.wait_for(lambda: tc.assert_eq(0, inputs['iec_state']()))
    tc.assert_eq(0, inputs['charger_state']())
    tc.wait_for(lambda: tc.assert_eq(0xFFFFFFFF, inputs['current_user']()))
    tc.assert_eq(0, inputs['start_time_min']())
    tc.assert_eq(0, inputs['charging_time_sec']())

    def wait_for_slot(slot, current):
        tc.wait_for(lambda: tc.assert_eq(current, tc.api(f'evse/slots/{slot}/max_current')))

    check_write(tc, client.write_coil(1000, False))
    wait_for_slot(10, 0)
    tc.assert_false(coils['enable_charging']())
    testbox.set_cp('C')
    tc.wait_for(lambda: tc.assert_eq(1, inputs['iec_state']()))
    tc.assert_fail(lambda: tc.wait_for(lambda: tc.assert_eq(2, inputs['iec_state']())))
    check_write(tc, client.write_coil(1000, True))
    wait_for_slot(10, 32000)
    tc.assert_true(coils['enable_charging']())
    tc.wait_for(lambda: tc.assert_eq(2, inputs['iec_state']()))

    for current, iec_state in ((6000, 2), (0, 1), (32000, 2)):
        write_u32(tc, client, 1002, current)
        wait_for_slot(9, current)
        wait_for_slot(10, 32000)
        tc.assert_eq(current, holding['allowed_current']())
        verify_slots(tc, inputs['slots'])
        tc.wait_for(lambda: tc.assert_eq(iec_state, inputs['iec_state']()))

    for value, enabled_current, iec_state in ((6000, 32000, 2), (0, 0, 1), (1, 32000, 2)):
        write_u32(tc, client, 1000, value)
        wait_for_slot(9, 32000)
        wait_for_slot(10, enabled_current)
        tc.assert_eq(enabled_current, holding['enable_charging_deprecated']())
        verify_slots(tc, inputs['slots'])
        tc.wait_for(lambda: tc.assert_eq(iec_state, inputs['iec_state']()))

    try:
        check_write(tc, client.write_coil(1001, False))
        wait_for_slot(4, 0)
        tc.assert_false(coils['autostart_button']())
        check_write(tc, client.write_coil(1001, True))
        wait_for_slot(4, 32000)
        tc.assert_true(coils['autostart_button']())
    finally:
        check_write(tc, client.write_coil(1001, autostart_current > 0))


def verify_warp_nfc(tc, client, inputs):
    tc.api('nfc/inject_tag', {'tag_type': 2, 'tag_id': '01:23:45:67:89:AB:CD:EF:AB:CD'})
    tc.wait_for(lambda: tc.assert_eq('0123456789ABCDEFABCD', text_register(inputs['tag_id'])))
    check_registers(tc, client.read_input_registers(4020, count=6), [0x0123, 0x4567, 0x89AB, 0xCDEF, 0xABCD, 0])
    tc.assert_eq('0002', text_register(inputs['tag_type']))
    tc.assert_gt(0, inputs['tag_id_age']())
    age = tc.wait_for(lambda: tc.assert_gt(2000, inputs['tag_id_age']()))

    tc.api('nfc/inject_tag', {'tag_type': 2, 'tag_id': '01:23:45:68'})
    tc.wait_for(lambda: tc.assert_lt(age, inputs['tag_id_age']()))
    tc.assert_eq('01234568', text_register(inputs['tag_id']))

    tag_id = '0102030405060708090A'
    tag_words = list(struct.unpack('>10H', tag_id.encode('ascii')))
    check_write(tc, client.write_registers(4000, tag_words))
    write_u32(tc, client, 4010, 0x3030)
    check_registers(tc, client.read_holding_registers(4000, count=14), [*tag_words, 0, 0x3030, 0, 0])
    write_u32(tc, client, 4012, 0x3032)
    tc.wait_for(lambda: tc.assert_eq(tag_id, text_register(inputs['tag_id'])))
    check_registers(tc, client.read_input_registers(4020, count=6), [0x0102, 0x0304, 0x0506, 0x0708, 0x090A, 0])
    check_registers(tc, client.read_holding_registers(4000, count=14), [0] * 14)

    write_u32(tc, client, 4000, 0x30313032)
    write_u32(tc, client, 4010, 0x3030)
    write_u32(tc, client, 4012, 0x3034)
    tc.wait_for(lambda: tc.assert_eq('0102', text_register(inputs['tag_id'])))
    tc.assert_eq('0004', text_register(inputs['tag_type']))
    check_registers(tc, client.read_input_registers(4020, count=6), [0x0102, 0, 0, 0, 0, 0])


def test_warp_button(tc: TestContext):
    testbox = tc.get_testbox()
    with modbus_session(tc, 0, required_features=('evse',), testbox=testbox) as client, ExitStack() as restore:
        restore.callback(testbox.set_button_pressed, testbox.is_button_pressed())

        def verify_button(pressed, *, changed=None, old_time=None):
            state = tc.api('evse/button_state', timeout=1)
            tc.assert_eq(pressed, state['button_pressed'])
            if changed is not None:
                tc.assert_ne(old_time, state[changed])
            check_bits(tc, client.read_discrete_inputs(1102, count=1), [pressed])
            check_registers(
                tc, client.read_input_registers(1100, count=4),
                u32_words(state['button_press_time']) + u32_words(state['button_release_time']),
            )
            return state

        testbox.set_button_pressed(False)
        before = wait_for_modbus(tc, client, lambda: verify_button(False))
        testbox.set_button_pressed(True)
        pressed = tc.wait_for(lambda: verify_button(True, changed='button_press_time', old_time=before['button_press_time']))
        testbox.set_button_pressed(False)
        tc.wait_for(lambda: verify_button(False, changed='button_release_time', old_time=pressed['button_release_time']))


def test_warp_shutdown_input(tc: TestContext):
    version = tc.api('evse/hardware_configuration/evse_version')
    if not 20 <= version < 40:
        tc.skip('Requires a WARP2 or WARP3 shutdown input')
    testbox = tc.get_testbox()
    with modbus_session(tc, 0, required_features=('evse',), testbox=testbox) as client, ExitStack() as restore:
        closed = testbox.is_gpio_shutdown_closed()
        restore.callback(testbox.set_gpio_shutdown_closed, closed)

        def verify_input(closed):
            check_bits(tc, client.read_discrete_inputs(1100, count=1), [not closed])

        wait_for_modbus(tc, client, lambda: verify_input(closed))
        for state in (not closed, closed):
            testbox.set_gpio_shutdown_closed(state)
            tc.wait_for(lambda: verify_input(state))


def test_warp_general_purpose_io(tc: TestContext):
    testbox = tc.get_testbox()
    with modbus_session(tc, 0, required_features=('evse', 'evse_gp_output'), testbox=testbox) as client, ExitStack() as restore:
        enable_warp_writes(tc, client)
        input_closed = testbox.is_gpio_input_closed()
        output_api = 'evse/gp_output'
        output = tc.api(output_api)
        restore.callback(testbox.set_gpio_input_closed, input_closed)
        restore.callback(tc.api, output_api + '_update', output)

        for closed in (not input_closed, input_closed):
            testbox.set_gpio_input_closed(closed)
            tc.wait_for(lambda: check_bits(tc, client.read_discrete_inputs(1101, count=1), [not closed]))

        def verify_output(high_impedance):
            tc.assert_eq(not high_impedance, testbox.is_gpio_output_closed())
            tc.assert_eq(int(high_impedance), tc.api(output_api + '/gp_output'))
            check_bits(tc, client.read_coils(1100, count=1), [high_impedance])

        high_impedance = bool(output['gp_output'])
        for state in (not high_impedance, high_impedance):
            check_write(tc, client.write_coil(1100, state))
            tc.wait_for(lambda: verify_output(state))


def test_warp_indicator_led(tc: TestContext):
    with modbus_session(tc, 0, required_features=('evse',), required_modules=('evse_led',)) as client, ExitStack() as restore:
        config_api = 'evse/led_configuration'
        state_api = 'evse/indicator_led'
        config = tc.api(config_api)
        restore.callback(tc.api, config_api + '_update', config)
        restore.callback(tc.api, state_api + '_update', tc.api(state_api))
        tc.api(config_api + '_update', {**config, 'enable_api': True})
        enable_warp_writes(tc, client)

        def write_and_verify_led(indication, duration, hsv=()):
            words = [word for value in (indication, duration, *hsv) for word in u32_words(value)]
            sent_at = time.monotonic()
            check_write(tc, client.write_registers(1004, words))

            def verify_led():
                state = tc.api(state_api)
                tc.assert_eq(indication, state['indication'])
                if hsv:
                    tc.assert_eq(hsv, (state['color_h'], state['color_s'], state['color_v']))

                tc.assert_eq(indication, read_u32(client.read_holding_registers, 1004))
                modbus_duration = read_u32(client.read_holding_registers, 1006)
                # The EVSE reports time remaining, not the requested duration.
                elapsed_ms = math.ceil((time.monotonic() - sent_at) * 1000)
                for remaining in (state['duration'], modbus_duration):
                    tc.assert_ge(max(1, duration - elapsed_ms - 100), remaining)
                    tc.assert_le(duration, remaining)

            tc.wait_for(verify_led)

        write_and_verify_led(64, 10000)
        if 'rgb_led' in tc.api('info/features'):
            write_and_verify_led(128, 12000, (251, 170, 125))


def test_warp_phase_switch(tc: TestContext):
    with modbus_session(tc, 0, required_features=('evse', 'phase_switch'), required_modules=('power_manager',)) as client:
        control_api = 'power_manager/state/external_control'
        if tc.api(control_api) != 0:
            tc.skip('External phase control is not available')
        original_phases = 3 if tc.api('power_manager/low_level_state/is_3phase') else 1
        enable_warp_writes(tc, client)

        def verify_phases(phases):
            tc.assert_eq(0, tc.api(control_api))
            tc.assert_eq(phases == 3, tc.api('power_manager/low_level_state/is_3phase'))
            check_registers(tc, client.read_input_registers(3100, count=4), [0, phases, 0, 0])

        def switch_phases(phases, *, verify_switching=False):
            write_u32(tc, client, 3100, phases)
            if verify_switching:
                # The state changes to Switching immediately after the request is accepted.
                check_registers(tc, client.read_input_registers(3102, count=2), [0, 3])
            check_registers(tc, client.read_holding_registers(3100, count=2), [0, phases])
            tc.wait_for(lambda: verify_phases(phases), timeout=45)

        verify_phases(original_phases)
        try:
            switch_phases(1 if original_phases == 3 else 3, verify_switching=True)
        finally:
            tc.wait_for(lambda: tc.assert_eq(0, tc.api(control_api)), timeout=45)
            switch_phases(original_phases)


def test_warp_nfc_charge_actions(tc: TestContext):
    testbox = tc.get_testbox()
    with modbus_session(
        tc, 0,
        required_features=('evse', 'nfc'),
        required_modules=('charge_authorization', 'charge_tracker', 'users'),
        testbox=testbox,
    ) as client, ExitStack() as restore:
        enable_warp_writes(tc, client)
        user_id = tc.api('users/config/next_user_id')
        nonce = uuid4().hex[:6]
        tc.api('users/add', {
            'id': user_id, 'roles': 0, 'current': 6000, 'display_name': 'Modbus NFC test',
            'username': f'modbus-{nonce}', 'digest_hash': '',
        })
        restore.callback(tc.api, 'users/remove', {'id': user_id})

        tag_ids = [f'{nonce.upper()}{i:02X}' for i in range(4)]
        tags = [
            {'tag_type': 2, 'tag_id': bytes.fromhex(tag_id).hex(':').upper(), 'user_id': user_id}
            for tag_id in tag_ids
        ]
        nfc_config = tc.api('nfc/config')
        restore.callback(tc.api, 'nfc/config_update', nfc_config)
        tc.api('nfc/config_update', {**nfc_config, 'authorized_tags': [*nfc_config['authorized_tags'], *tags]})
        user_api = 'evse/user_enabled'
        restore.callback(tc.api, user_api + '_update', tc.api(user_api))
        tc.api(user_api + '_update', {'enabled': True})
        tc.wait_for(lambda: tc.assert_true(tc.api('evse/slots/6/active')))

        def iec_state():
            return read_u32(client.read_input_registers, 1000)

        def inject(tag_index, action):
            check_write(tc, client.write_registers(4000, list(struct.unpack('>4H', tag_ids[tag_index].encode('ascii')))))
            write_u32(tc, client, 4010, action)
            write_u32(tc, client, 4012, 0x3032)
            tc.wait_for(lambda: tc.assert_eq(tags[tag_index]['tag_id'], tc.api('nfc/seen_tags/0/tag_id')))

        def assert_stays_in_state(state):
            tc.assert_fail(lambda: tc.wait_for(lambda: tc.assert_ne(state, iec_state()), timeout=1))

        restore.callback(tc.wait_for, lambda: tc.assert_eq(-1, tc.api('charge_tracker/current_charge/user_id')))
        restore.callback(testbox.set_cp, 'A')
        testbox.set_cp('C')
        tc.wait_for(lambda: tc.assert_eq(1, iec_state()))

        inject(0, 2)  # Stop-only must not start a charge.
        assert_stays_in_state(1)
        inject(1, 0x3031)  # Start-only.
        tc.wait_for(lambda: tc.assert_eq(2, iec_state()))
        tc.wait_for(lambda: tc.assert_eq(user_id, tc.api('charge_tracker/current_charge/user_id')))
        tc.wait_for(lambda: tc.assert_gt(3000, tc.api('evse/low_level_state/time_since_state_change')))

        inject(2, 0x3031)  # Start-only must not stop a charge.
        assert_stays_in_state(2)
        inject(3, 2)  # Stop-only.
        tc.wait_for(lambda: tc.assert_eq(1, iec_state()))


def test_warp_reboot(tc: TestContext):
    with modbus_session(tc, 0, required_features=('evse',)) as client:
        uptime = enable_warp_writes(tc, client)
        before = tc.wait_for(lambda: tc.assert_ge(15, uptime()), timeout=20)
        write_u32(tc, client, 0, 0)
        tc.assert_ge(before, uptime())

        write_u32(tc, client, 0, 0x012EB007)
        client.close()
        tc.wait_for_reboot(pre_reboot_delay=2)
        wait_for_modbus(tc, client, lambda: tc.assert_lt(before, uptime()))


def test_warp_meter_reset(tc: TestContext):
    if os.environ.get('MODBUS_TCP_TEST_ALLOW_METER_RESET') != '1':
        tc.skip('Set MODBUS_TCP_TEST_ALLOW_METER_RESET=1 to clear the test meter relative energy')
    with modbus_session(tc, 0, required_features=('evse', 'meter')) as client:
        enable_warp_writes(tc, client)
        last_reset_api = 'meter/last_reset/last_reset'
        before = tc.api(last_reset_api)
        write_u32(tc, client, 2000, 0)
        tc.assert_eq(before, tc.api(last_reset_api))

        write_u32(tc, client, 2000, 0x3E12E5E7)
        tc.wait_for(lambda: tc.assert_gt(before, tc.api(last_reset_api)), timeout=20)

        def verify_relative_energy():
            response = client.read_input_registers(2006, count=2)
            if response.isError():
                tc.fail(f'Modbus read failed: {response}')
            energy, = unpack_registers('f', response.registers)
            tc.assert_epsilon(0, 0.001, energy)

        tc.wait_for(verify_relative_energy, timeout=20)


def test_keba(tc: TestContext):
    testbox = tc.get_testbox()
    with modbus_session(
        tc, 2,
        required_features=('evse', 'meter', 'meter_all_values'),
        required_modules=('charge_limits',),
        testbox=testbox,
        send_illegal_data_address=False,
        ignore_writes=True,
    ) as client:
        holding = build_table(tc, client, KEBA_REGISTER_MAP)['holding']

        tc.assert_eq(1, holding['charging_state']())
        tc.assert_eq(3, holding['cable_state']())
        tc.assert_eq(0, holding['error_code']())
        tc.assert_eq(base58decode(tc.api('info/name/uid')), holding['serial_number']())

        features = tc.api('info/features')
        slots = tc.api('evse/slots')
        jumper = tc.api('evse/hardware_configuration/jumper_configuration')
        supported_current = {2: 1000, 3: 2000, 4: 3000, 6: 4000}.get(jumper, 0)
        tc.assert_eq(310100 + supported_current + 20 + int('nfc' in features), holding['features']())
        tc.assert_eq(0x030A1B00, holding['firmware_version']())
        tc.assert_eq(tc.api('evse/state/allowed_charging_current'), holding['max_current']())
        tc.assert_eq(min(slots[0]['max_current'], slots[1]['max_current']), holding['max_hardware_current']())

        tc.assert_epsilon(tc.api('meter/values/power') * 1000, 1000, holding['power']())
        tc.assert_epsilon(tc.api('meter/values/energy_abs') * 10000, 100, holding['total_energy']())

        # KEBA reports per-phase current in mA and voltage in whole volts.
        meter_values = tc.api('meter/all_values')
        assert_phase_meter_values(tc, meter_values[:3], holding['voltages']())
        assert_phase_meter_values(tc, meter_values[3:6], holding['currents'](), scale=1000, tolerance=10)
        if meter_values[27] is not None:
            tc.assert_epsilon(meter_values[27] * 10, 1, holding['power_factor']())

        tc.assert_eq(0, holding['rfid_tag']())
        tc.assert_eq(0, holding['charged_energy']())
        can_switch = 'phase_switch' in features
        phases = holding['phase_switching_state_read']()
        if can_switch:
            tc.assert_in((1, 3), phases)
            tc.wait_for(lambda: tc.assert_eq(phases == 3, tc.api('power_manager/low_level_state/is_3phase')))

        testbox.set_cp('C')
        tc.wait_for(lambda: tc.assert_eq(3, holding['charging_state']()))
        tc.assert_eq(7, holding['cable_state']())
        tc.assert_eq(0, holding['error_code']())
        testbox.set_cp('A')
        tc.wait_for(lambda: tc.assert_eq(1, holding['charging_state']()))

        verify_keba_writes(tc, client, holding, can_switch, phases)


def verify_keba_writes(tc, client, holding, can_switch, phases):
    energy_api = 'charge_limits/active_limits/energy_wh'
    energy = tc.api(energy_api)

    def write(address, value):
        check_write(tc, client.write_registers(address, [value]))

    def check_slots(current, enabled_current):
        slots = tc.api('evse/slots')
        tc.assert_eq(current, slots[9]['max_current'])
        tc.assert_eq(enabled_current, slots[10]['max_current'])

    check_slots(32000, 32000)
    ignored_api = 'modbus_tcp/error_counters/ignored_write'
    ignored = tc.api(ignored_api)
    write(5004, 12345)
    write(5010, 12345)
    write(5014, 0)
    if can_switch:
        write(5052, 0 if phases == 3 else 1)
    # The KEBA write handler initializes the firmware's phase-switch feature cache.
    tc.assert_eq(3 if can_switch else 0, holding['phase_switching_source_read']())
    tc.assert_eq(ignored + 3 + int(can_switch), tc.api(ignored_api))
    check_slots(32000, 32000)
    tc.assert_eq(energy, tc.api(energy_api))
    tc.assert_eq(phases, holding['phase_switching_state_read']())

    enable_writes(tc, client, holding['charging_state'])
    check_slots(32000, 32000)
    tc.assert_eq(energy, tc.api(energy_api))

    with ExitStack() as restore:
        restore.callback(write, 5014, 1)
        restore.callback(write, 5004, 32000)
        restore.callback(tc.api, 'charge_limits/override_energy', {'energy_wh': energy})
        write(5004, 12345)
        write(5010, 12345)
        write(5014, 0)
        tc.wait_for(lambda: check_slots(12345, 0))
        tc.assert_eq(123450, tc.api(energy_api))

    # External phase control is available only on some testbox configurations.
    external_control_api = 'power_manager/state/external_control'
    if not can_switch or tc.api(external_control_api) != 0:
        return

    other_phases = 1 if phases == 3 else 3

    def wait_for_phases(expected):
        tc.wait_for(lambda: tc.assert_eq(expected, holding['phase_switching_state_read']()), timeout=45)
        tc.wait_for(lambda: tc.assert_eq(0, tc.api(external_control_api)), timeout=10)

    try:
        write(5052, 1 if other_phases == 3 else 0)
        wait_for_phases(other_phases)
    finally:
        tc.wait_for(lambda: tc.assert_eq(0, tc.api(external_control_api)), timeout=45)
        write(5052, 1 if phases == 3 else 0)
        wait_for_phases(phases)


def test_bender(tc: TestContext):
    with modbus_session(tc, 1, required_features=('evse', 'meter_all_values')) as client:
        holding = build_table(tc, client, BENDER_REGISTER_MAP)['holding']
        tc.assert_eq('4.40', text_register(holding['firmware_version']))
        tc.assert_eq(0, holding['ocpp_cp_state']())
        tc.assert_eq([0, 0, 0, 0], holding['errorcodes']())
        tc.assert_eq('0.14', text_register(holding['protocol_version']))
        tc.assert_eq(1, holding['vehicle_state']())
        tc.assert_eq(0, holding['chargepoint_available']())
        tc.assert_eq(0, holding['safe_current']())

        # The emulation uses the same legacy meter API as the WARP table.
        meter_values = tc.api('meter/all_values')
        for name, first, scale, tolerance in (
            ('energy', 67, 1000, 10),  # Per-phase import energy
            ('power', 6, 1, 1),
            ('current', 3, 1000, 10),
            ('voltage', 0, 1, 1),
        ):
            expected = meter_values[first:first + 3]
            assert_phase_meter_values(tc, expected, holding[name](), scale=scale, tolerance=tolerance)

        tc.assert_epsilon(meter_values[30] * 1000, 10, holding['total_energy']())
        tc.assert_epsilon(meter_values[24], 1, holding['total_power']())

        verify_bender_writes(tc, client, holding)


def verify_bender_writes(tc, client, holding):
    tc.assert_eq(0, holding['hems_limit']())
    enable_writes(tc, client, holding['hems_limit'])

    for limit in (13, 32):
        check_write(tc, client.write_registers(1000, [limit]))
        tc.wait_for(lambda: tc.assert_eq(limit * 1000, tc.api('evse/slots/9/max_current')))
        tc.assert_eq(limit, holding['hems_limit']())

    for availability, current in ((1, 0), (0, 32000)):
        check_write(tc, client.write_register(124, availability))
        tc.wait_for(lambda: tc.assert_eq(current, tc.api('evse/slots/10/max_current')))
        tc.assert_eq(availability, holding['chargepoint_available']())


def test_compatibility_table_exceptions(tc: TestContext):
    tables = (
        (1, 100, [0x342E, 0x3430], 102, False),  # Bender zero-fills read gaps even in strict mode.
        (2, 1018, [0x030A, 0x1B00], 1022, True),  # KEBA rejects read gaps in strict mode.
    )
    for table, version_address, version, gap, reject_gap_read in tables:
        with modbus_session(
            tc, table,
            required_features=('evse',),
            send_illegal_data_address=True,
            ignore_writes=False,
        ) as client:
            def verify_version():
                check_registers(tc, client.read_holding_registers(version_address, count=2), version)

            wait_for_modbus(tc, client, verify_version)
            enable_writes(tc, client, verify_version)

            counters_api = 'modbus_tcp/error_counters'
            counters = tc.api(counters_api)
            unsupported = (
                client.read_coils(0, count=1),
                client.read_discrete_inputs(0, count=1),
                client.read_input_registers(0, count=1),
                client.write_coils(0, [False, False]),
                client.mask_write_register(address=version_address),
            )
            for response in unsupported:
                check_exception(tc, response, ExcCodes.ILLEGAL_FUNCTION)

            gap_read = client.read_holding_registers(gap, count=2)
            if reject_gap_read:
                check_exception(tc, gap_read, ExcCodes.ILLEGAL_ADDRESS)
            else:
                check_registers(tc, gap_read, [0, 0])
            check_exception(tc, client.write_register(gap, 0), ExcCodes.ILLEGAL_ADDRESS)

            counters['illegal_function'] += len(unsupported)
            counters['illegal_data_address'] += 1 + int(reject_gap_read)
            tc.assert_eq(counters, tc.api(counters_api))
            verify_version()


if __name__ == '__main__':
    run_testsuite(locals())
