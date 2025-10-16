#!/usr/bin/python3 -u

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')

import sys
import queue
import threading
import time
import traceback
import tkinter as tk
import typing
import subprocess

import cv2 # sudo pip3 install openvc-python

from provisioning.tinkerforge.ip_connection import IPConnection
from provisioning.tinkerforge.device_factory import create_device
from provisioning.tinkerforge.brick_master import BrickMaster
from provisioning.tinkerforge.bricklet_industrial_dual_relay import BrickletIndustrialDualRelay
from provisioning.tinkerforge.bricklet_industrial_quad_relay_v2 import BrickletIndustrialQuadRelayV2
from provisioning.tinkerforge.bricklet_industrial_dual_ac_relay import BrickletIndustrialDualACRelay
from provisioning.tinkerforge.bricklet_energy_monitor import BrickletEnergyMonitor
from provisioning.tinkerforge.bricklet_servo_v2 import BrickletServoV2
from provisioning.tinkerforge.bricklet_led_strip_v2 import BrickletLEDStripV2
from provisioning.tinkerforge.bricklet_color_v2 import BrickletColorV2
from provisioning.tinkerforge.bricklet_piezo_speaker_v2 import BrickletPiezoSpeakerV2
from provisioning.tinkerforge.bricklet_nfc import BrickletNFC

from provisioning.provision_common.inventory import Inventory
from provisioning.provision_common.provision_common import FatalError, fatal_error, green, my_input

IPCON_HOST = 'localhost'
IPCON_TIMEOUT = 1.0 # seconds

ACTION_INTERVAL = 0.5 # seconds
ACTION_COMPLETION_TIMEOUT = 30.0 # seconds
ACTION_TRY_COUNT = 3

MONOFLOP_DURATION = (IPCON_TIMEOUT * ACTION_TRY_COUNT + ACTION_INTERVAL) * 1000 # milliseconds

RELAY_SETTLE_DURATION = 0.25 # seconds

EVSE_SETTLE_DURATION = 2.0 # seconds

DC_PROTECT_SETTLE_DURATION = 5.0 # seconds

METER_SETTLE_DURATION = 5.0 # seconds

VOLTAGE_SETTLE_DURATION = 1.0 # seconds
VOLTAGE_OFF_THRESHOLD = 5.0 # Volt
VOLTAGE_ON_THRESHOLD = 200.0 # Volt

IEC_STATE_CHECK_DURATION = 5.0 # seconds
IEC_STATE_CHECK_INTERVAL = 0.01 # seconds

PHASE_SWITCH_SETTLE_DURATION = 10.0 # seconds

STACK_MASTER_UIDS = {
    '0': '61SMKP',
    '1': '6jEhrp',
    '2': '6DY5kB',
}

EXPECTED_DEVICE_IDENTIFIERS = {
    '00': BrickMaster.DEVICE_IDENTIFIER,
    '00A': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '00B': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '00C': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '00D': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '01': BrickMaster.DEVICE_IDENTIFIER,
    '01A': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '01B': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '01C': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '01D': BrickletIndustrialQuadRelayV2.DEVICE_IDENTIFIER,
    '02': BrickMaster.DEVICE_IDENTIFIER,
    '02A': BrickletIndustrialDualACRelay.DEVICE_IDENTIFIER,
    '02B': BrickletEnergyMonitor.DEVICE_IDENTIFIER,
    '02C': BrickletEnergyMonitor.DEVICE_IDENTIFIER,
    '02D': BrickletEnergyMonitor.DEVICE_IDENTIFIER,
    '03': BrickMaster.DEVICE_IDENTIFIER,
    '03A': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '03B': BrickletPiezoSpeakerV2.DEVICE_IDENTIFIER,
    '10': BrickMaster.DEVICE_IDENTIFIER,
    '10A': BrickletServoV2.DEVICE_IDENTIFIER,
    '20': BrickMaster.DEVICE_IDENTIFIER,
    '20A': BrickletLEDStripV2.DEVICE_IDENTIFIER,
    '20B': BrickletNFC.DEVICE_IDENTIFIER,
    '20C': BrickletServoV2.DEVICE_IDENTIFIER,
    '20D': BrickletColorV2.DEVICE_IDENTIFIER,
}

class Stage3:
    def __init__(self,
                 is_front_panel_button_pressed_function,
                 has_evse_error_function,
                 get_iec_state_function,
                 reset_dc_fault_function,
                 switch_phases_function,
                 get_evse_uptime_function,
                 reset_evse_function,
                 get_cp_pwm_function,
                 get_meter_voltages_function):
        self.is_front_panel_button_pressed_function = is_front_panel_button_pressed_function
        self.has_evse_error_function = has_evse_error_function
        self.get_iec_state_function = get_iec_state_function
        self.reset_dc_fault_function = reset_dc_fault_function
        self.switch_phases_function = switch_phases_function
        self.get_evse_uptime_function = lambda: get_evse_uptime_function() / 1000.0
        self.reset_evse_function = reset_evse_function
        self.get_cp_pwm_function = get_cp_pwm_function
        self.get_meter_voltages_function = get_meter_voltages_function
        self.ipcon = IPConnection()
        self.inventory = Inventory(self.ipcon)
        self.devices = {} # by position path
        self.prepared = False
        self.action_stop_queue = None
        self.action_enabled_ref = None
        self.action_thread = None
        self.wall_clock_start = None

        self.ipcon.set_timeout(IPCON_TIMEOUT)

    # internal
    def try_action(self, position, function):
        tries = ACTION_TRY_COUNT

        while tries > 0:
            try:
                return function(self.devices[position])
            except Exception as e:
                tries -= 1

                if tries == 1:
                    print('WARNING: Could not complete action for device at position {0}, 1 try left: {1}'.format(position, e))
                elif tries > 1:
                    print('WARNING: Could not complete action for device at position {0}, {1} tries left: {2}'.format(position, tries, e))
                else:
                    fatal_error('Could not complete action for device at position {0}: {1}'.format(position, e), force_os_exit=1)

    # internal
    def action_loop(self, stop_queue, enabled_ref):
        timestamp = -ACTION_INTERVAL
        actions = {} # by position path
        events = {} # by position path
        tries = {} # by position path

        while enabled_ref[0]:
            elapsed = time.monotonic() - timestamp
            remaining = max(ACTION_INTERVAL - elapsed, 0)

            try:
                action = stop_queue.get(timeout=remaining)
            except queue.Empty:
                action = False

            if not enabled_ref[0] or action == None:
                break

            timestamp = time.monotonic()

            if action != False:
                position, function, event = action
                actions[position] = function
                events[position] = event
                tries[position] = ACTION_TRY_COUNT

            for position, function in actions.items():
                try:
                    function(self.devices[position[0]])
                except Exception as e:
                    tries[position] -= 1

                    if tries[position] == 1:
                        print('WARNING: Could not complete action {0} for device at position {1}, 1 try left: {2}'.format(position[1], position[0], e))
                    elif tries[position] > 1:
                        print('WARNING: Could not complete action {0} for device at position {1}, {2} tries left: {3}'.format(position[1], position[0], tries[position], e))
                    else:
                        fatal_error('Could not complete action {0} for device at position {1}: {2}'.format(position[1], position[0], e), force_os_exit=1)
                else:
                    event = events.pop(position, None)

                    if event != None:
                        event.set()

                    tries[position] = ACTION_TRY_COUNT

    # internal
    def connect_warp_power(self, phases):
        assert set(phases).issubset({'L1', 'L2', 'L3'}), phases

        events = [
            threading.Event(),
            threading.Event(),
        ]

        if len(phases) == 0:
            self.action_stop_queue.put((('00D', 0), lambda device: device.set_value(False, False), events[0]))
            self.action_stop_queue.put((('01A', 0), lambda device: device.set_value(False, False), events[1]))
        else:
            assert 'L1' in phases, phases

            self.action_stop_queue.put((('00D', 0), lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_monoflop(1, True, MONOFLOP_DURATION)], events[0]))

            if 'L2' in phases and 'L3' in phases:
                self.action_stop_queue.put((('01A', 0), lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_monoflop(1, True, MONOFLOP_DURATION)], events[1]))
            elif 'L2' in phases:
                self.action_stop_queue.put((('01A', 0), lambda device: [device.set_selected_value(0, False), device.set_monoflop(1, True, MONOFLOP_DURATION)], events[1]))
            elif 'L3' in phases:
                self.action_stop_queue.put((('01A', 0), lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_selected_value(1, False)], events[1]))
            else:
                self.action_stop_queue.put((('01A', 0), lambda device: [device.set_value(False, False)], events[1]))

        timeout_remaining = ACTION_COMPLETION_TIMEOUT

        for event in events:
            start = time.monotonic()

            if not event.wait(timeout=timeout_remaining):
                fatal_error('Action did not complete in time')

            timeout_remaining -= time.monotonic() - start

        if len(phases) > 0 and self.wall_clock_start != None:
            self.verify_evse_not_crashed()
            self.reset_evse_function()
            time.sleep(5)

            self.evse_uptime_start = self.get_evse_uptime_function()
            self.wall_clock_start = time.time()

    # internal
    def connect_outlet(self, outlet):
        assert outlet in ['Basic', 'Smart', 'Pro', 'CEE']

        event = threading.Event()

        if outlet in ['Basic', 'Smart']:
            self.action_stop_queue.put((('02A', 0), lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_selected_value(1, False)], event))
        elif outlet == 'Pro':
            self.action_stop_queue.put((('02A', 0), lambda device: [device.set_selected_value(0, False), device.set_monoflop(1, True, MONOFLOP_DURATION)], event))
        elif outlet == 'CEE':
            self.action_stop_queue.put((('02A', 0), lambda device: device.set_value(False, False), event))
        else:
            assert False, outlet

        if not event.wait(timeout=ACTION_COMPLETION_TIMEOUT):
            fatal_error('Action did not complete in time')

    # internal
    def connect_voltage_monitors(self, connect):
        events = [
            threading.Event(),
            threading.Event(),
        ]

        if connect:
            self.action_stop_queue.put((('01B', 0), lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_monoflop(1, True, MONOFLOP_DURATION)], events[0]))
            self.action_stop_queue.put((('01C', 0), lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_monoflop(1, True, MONOFLOP_DURATION)], events[1]))
        else:
            self.action_stop_queue.put((('01B', 0), lambda device: device.set_value(False, False), events[0]))
            self.action_stop_queue.put((('01C', 0), lambda device: device.set_value(False, False), events[1]))

        timeout_remaining = ACTION_COMPLETION_TIMEOUT

        for event in events:
            start = time.monotonic()

            if not event.wait(timeout=timeout_remaining):
                fatal_error('Action did not complete in time')

            timeout_remaining -= time.monotonic() - start

    # internal
    def connect_front_panel(self, connect):
        event = threading.Event()

        if connect:
            self.action_stop_queue.put((('03A', 0), lambda device: device.set_monoflop(0, True, MONOFLOP_DURATION), event))
        else:
            self.action_stop_queue.put((('03A', 0), lambda device: device.set_selected_value(0, False), event))

        if not event.wait(timeout=ACTION_COMPLETION_TIMEOUT):
            fatal_error('Action did not complete in time')

    # internal
    def connect_type2_pe(self, connect):
        event = threading.Event()

        if connect:
            self.action_stop_queue.put((('03A', 1), lambda device: device.set_selected_value(1, False), event))
        else:
            self.action_stop_queue.put((('03A', 1), lambda device: device.set_monoflop(1, True, MONOFLOP_DURATION), event))

        if not event.wait(timeout=ACTION_COMPLETION_TIMEOUT):
            fatal_error('Action did not complete in time')

    # internal
    def change_cp_pe_state(self, state, quiet=False):
        if not quiet:
            print('Changing CP-PE state to {0}'.format(state))

        if state == 'A':
            value = [False, False, False, False]
        elif state == 'B':
            value = [False, True,  False, False]
        elif state == 'C':
            value = [False, True,  True,  False]
        elif state == 'D':
            value = [False, True,  False, True]
        else:
            assert False, state

        event = threading.Event()

        self.action_stop_queue.put((('01D', 0), lambda device: device.set_value(value), event))

        if not event.wait(timeout=ACTION_COMPLETION_TIMEOUT):
            fatal_error('Action did not complete in time')

    # internal
    def change_meter_state(self, state, quiet=False):
        if not quiet:
            print('Changing meter state to {0}'.format(state))

        if state == 'Type2-L1':
            value = [(True,  True),  (False, False), (False, False)]
        elif state == 'Type2-L2':
            value = [(True,  True),  (True,  False), (False, False)]
        elif state == 'Type2-L3':
            value = [(True,  True),  (True,  False), (True,  False)]
        elif state == 'WARP-L1':
            value = [(False, False), (False, False), (False, False)]
        elif state == 'WARP-L2':
            value = [(False, False), (False, True),  (False, False)]
        elif state == 'WARP-L3':
            value = [(False, False), (False, True),  (False, True)]
        else:
            assert False, state

        events = [
            threading.Event(),
            threading.Event(),
            threading.Event(),
        ]

        self.action_stop_queue.put((('00A', 0), lambda device: device.set_value(*value[0]), events[0]))
        self.action_stop_queue.put((('00B', 0), lambda device: device.set_value(*value[1]), events[1]))
        self.action_stop_queue.put((('00C', 0), lambda device: device.set_value(*value[2]), events[2]))

        timeout_remaining = ACTION_COMPLETION_TIMEOUT

        for event in events:
            start = time.monotonic()

            if not event.wait(timeout=timeout_remaining):
                fatal_error('Action did not complete in time')

            timeout_remaining -= time.monotonic() - start

    # internal
    def read_voltage_monitors(self):
        return [
            self.try_action('02B', lambda device: device.get_energy_data()[0] / 100),
            self.try_action('02C', lambda device: device.get_energy_data()[0] / 100),
            self.try_action('02D', lambda device: device.get_energy_data()[0] / 100),
        ]

    # internal
    def set_servo_position(self, servo, channel, position):
        self.try_action(servo, lambda device: device.set_position(channel, position))

        current_position = self.try_action(servo, lambda device: device.get_current_position(channel))

        # FIXME: add timeout?
        while current_position != position:
            time.sleep(0.1)

            current_position = self.try_action(servo, lambda device: device.get_current_position(channel))

    # internal
    def click_meter_run_button(self):
        servo = '10A'
        channel = 0

        try:
            self.set_servo_position(servo, channel, 4000)

            if not self.try_action(servo, lambda device: device.get_enabled(channel)):
                self.try_action(servo, lambda device: device.set_enable(channel, True))

            self.set_servo_position(servo, channel, 7300)
            time.sleep(0.1)
            self.set_servo_position(servo, channel, 4000)
            time.sleep(0.1)
        except Exception as e:
            fatal_error('Could not click meter run button: {0}'.format(e))

    # internal
    def click_meter_back_button(self):
        servo = '10A'
        channel = 1

        try:
            self.set_servo_position(servo, channel, 3000)

            if not self.try_action(servo, lambda device: device.get_enabled(channel)):
                self.try_action(servo, lambda device: device.set_enable(channel, True))

            self.set_servo_position(servo, channel, 6200)
            time.sleep(0.1)
            self.set_servo_position(servo, channel, 3000)
            time.sleep(0.1)
        except Exception as e:
            fatal_error('Could not click meter back button: {0}'.format(e))

    # internal
    def read_meter_qr_code(self, timeout=5, allow_no_detection=False):
        text = ''
        timestamp = time.monotonic()
        error = None

        for _ in range(5):
            error = None

            try:
                capture = cv2.VideoCapture(0)
            except Exception as e:
                error = str(e)
                continue

            try:
                decoder = cv2.QRCodeDetector()

                while len(text) == 0:
                    _, frame = capture.read()
                    text, _, _ = decoder.detectAndDecode(frame)

                    if timeout == None or timestamp + timeout < time.monotonic():
                        break

                    time.sleep(0.1)
            except Exception as e:
                error = str(e)
                continue
            finally:
                capture.release()

            if len(text) > 0:
                break

            time.sleep(0.1)

        if not allow_no_detection and len(text) == 0:
            fatal_error('Could not read QR code: {0}'.format(error))

        return text

    # internal
    def is_front_panel_led_blue(self):
        color = self.try_action('20D', lambda device: device.get_color())
        return color[2] / color[3] > 0.65 and color[3] > 10000

    def is_front_panel_led_red(self):
        color = self.try_action('20D', lambda device: device.get_color())

        return color[0] / color[3] > 0.85 and color[3] > 10000

    def is_front_panel_led_green(self):
        color = self.try_action('20D', lambda device: device.get_color())

        return color[1] / color[3] > 0.60 and color[3] > 10000

    def is_front_panel_led_white(self):
        color = self.try_action('20D', lambda device: device.get_color())

        # White can saturate the sensor
        if color[0] >= 60000 and color[1] >= 60000 and color[2] >= 60000 and color[3] >= 60000:
            return True

        return color[0] / color[3] < 0.5 and color[1] / color[3] < 0.5 and color[2] / color[3] < 0.5 and color[3] > 10000

    # internal
    def check_iec_state(self, expected_state):
        assert self.get_iec_state_function != None

        actual_states = set()
        start = time.monotonic()

        error_counter = 0
        while time.monotonic() < start + IEC_STATE_CHECK_DURATION:
            try:
                actual_states.add(self.get_iec_state_function()) # FIXME: missing error handling
            except:
                error_counter += 1
                if error_counter == 3:
                    raise
            time.sleep(IEC_STATE_CHECK_INTERVAL)

        return actual_states == set(expected_state)

    def setup(self):
        assert not self.prepared

        print('Connecting to brickd')

        try:
            self.ipcon.connect(IPCON_HOST, 4223)
        except Exception as e:
            fatal_error('Could not connect to brickd: {0}'.format(e))

        print('Enumerating Bricklets')

        self.inventory.clear()
        self.inventory.update(timeout=0.5)

        self.devices = {}

        for stack_position, positions in [('0', {'0': ['A', 'B', 'C', 'D'], '1': ['A', 'B', 'C', 'D'], '2': ['A', 'B', 'C', 'D'], '3': ['A', 'B']}),
                                          ('1', {'0': ['A']}),
                                          ('2', {'0': ['A', 'B', 'C', 'D']})]:
            for brick_position, bricklet_positions in positions.items():
                full_position = stack_position + brick_position

                if brick_position == '0':
                    brick_entry = self.inventory.get_one(uid=STACK_MASTER_UIDS[stack_position], connected_uid='0', position=brick_position)
                else:
                    brick_entry = self.inventory.get_one(connected_uid=STACK_MASTER_UIDS[stack_position], position=brick_position)

                if brick_entry == None:
                    fatal_error('Missing Brick at postion {0}'.format(full_position))

                if EXPECTED_DEVICE_IDENTIFIERS[full_position] != brick_entry.device_identifier:
                    fatal_error('Wrong Brick at postion {0}'.format(full_position))

                device = create_device(brick_entry.device_identifier, brick_entry.uid, self.ipcon)
                device.set_response_expected_all(True)

                self.devices[full_position] = device

                for bricklet_position in bricklet_positions:
                    full_position = stack_position + brick_position + bricklet_position
                    bricklet_entry = self.inventory.get_one(connected_uid=brick_entry.uid, position=bricklet_position)

                    if bricklet_entry == None:
                        fatal_error('Missing Bricklet at postion {0}'.format(full_position))

                    if EXPECTED_DEVICE_IDENTIFIERS[full_position] != bricklet_entry.device_identifier:
                        fatal_error('Wrong Bricklet at postion {0}'.format(full_position))

                    device = create_device(bricklet_entry.device_identifier, bricklet_entry.uid, self.ipcon)

                    # Don't reset the servo bricklets. Resetting can move the motor a bit.
                    if bricklet_entry.device_identifier != BrickletServoV2.DEVICE_IDENTIFIER:
                        device.reset()
                        device = create_device(bricklet_entry.device_identifier, bricklet_entry.uid, self.ipcon)

                    device.set_response_expected_all(True)

                    if bricklet_entry.device_identifier == BrickletColorV2.DEVICE_IDENTIFIER:
                        try:
                            device.set_status_led_config(BrickletColorV2.STATUS_LED_CONFIG_OFF)
                        except Exception as e:
                            fatal_error('Could not disable status LED for Color Bricklet 2.0 at postion {0}: {1}'.format(full_position, e))
                    elif bricklet_entry.device_identifier == BrickletNFC.DEVICE_IDENTIFIER:
                        try:
                            device.set_mode(BrickletNFC.MODE_SIMPLE)
                        except Exception as e:
                            fatal_error('Could not set simple mode for NFC Bricklet at postion {0}: {1}'.format(full_position, e))
                    elif bricklet_entry.device_identifier == BrickletLEDStripV2.DEVICE_IDENTIFIER:
                        try:
                            device.set_chip_type(BrickletLEDStripV2.CHIP_TYPE_WS2812)
                            device.set_channel_mapping(BrickletLEDStripV2.CHANNEL_MAPPING_GRB)
                        except Exception as e:
                            fatal_error('Could not set chip type and channel mapping for LED Strip Bricklet 2.0 at postion {0}: {1}'.format(full_position, e))

                    self.devices[full_position] = device

        self.try_action('20B', lambda device: device.simple_get_tag_id(255)) # clear tags from last test run

        self.action_stop_queue = queue.Queue()
        self.action_enabled_ref = [True]
        self.action_thread = threading.Thread(target=self.action_loop, args=(self.action_stop_queue, self.action_enabled_ref), daemon=True)
        self.action_thread.start()

        self.prepared = True

    def teardown(self):
        assert self.prepared

        self.prepared = False

        self.action_enabled_ref[0] = False
        self.action_stop_queue.put(None)
        self.action_thread.join()

        self.action_stop_queue = None
        self.action_enabled_ref = None
        self.action_thread = None

    def power_off(self):
        assert self.prepared

        print('Switching power off')

        self.connect_warp_power([])

        time.sleep(RELAY_SETTLE_DURATION)

        self.connect_outlet('CEE')
        self.connect_voltage_monitors(False)
        self.connect_front_panel(False)
        self.connect_type2_pe(True)
        self.change_cp_pe_state('A', quiet=True)
        self.change_meter_state('Type2-L1', quiet=True)

        time.sleep(RELAY_SETTLE_DURATION)

    def power_on(self, outlet, phases=['L1']):
        assert self.prepared
        assert outlet in ['Basic', 'Smart', 'Pro', 'CEE']

        print('Switching power on for {} outlet'.format(outlet))

        self.connect_warp_power([])

        time.sleep(RELAY_SETTLE_DURATION)

        self.connect_outlet(outlet)
        self.connect_voltage_monitors(False)
        self.connect_front_panel(False)
        self.connect_type2_pe(True)
        self.change_cp_pe_state('A', quiet=True)
        self.change_meter_state('Type2-L1', quiet=True)

        time.sleep(RELAY_SETTLE_DURATION)

        self.connect_warp_power(phases)

        time.sleep(RELAY_SETTLE_DURATION)

    # requires power_on
    def test_front_panel_button(self, automatic):
        assert self.is_front_panel_button_pressed_function != None

        servo = '20C'
        channel = 0

        try:
            button_before = self.is_front_panel_button_pressed_function()
            led_before = self.is_front_panel_led_blue() if automatic else True

            if button_before:
                fatal_error('Front panel button is already pressed before test')

            if not led_before:
                fatal_error('Front panel LED is not blue before test')

            if automatic:
                self.set_servo_position(servo, channel, -3000)

                if not self.try_action(servo, lambda device: device.get_enabled(channel)):
                    self.try_action(servo, lambda device: device.set_enable(channel, True))

                self.set_servo_position(servo, channel, 2500)
                time.sleep(0.5) # wait for Color Bricklet 2.0 integration time
            else:
                print(green('Waiting for front panel button press'))
                self.beep_notify()

                while not self.is_front_panel_button_pressed_function():
                    time.sleep(0.25)

                print('Front panel button is pressed')

            button_pressed = self.is_front_panel_button_pressed_function()
            led_pressed = self.is_front_panel_led_blue() if automatic else False

            if automatic:
                self.set_servo_position(servo, channel, -3000)
                time.sleep(0.5) # wait for Color Bricklet 2.0 integration time

            if not button_pressed:
                fatal_error('Front panel button is not pressed during test')

            if led_pressed:
                fatal_error('Front panel LED is still blue during test')

            if not automatic:
                print(green('Waiting for front panel button release'))
                self.beep_notify()

                while self.is_front_panel_button_pressed_function():
                    time.sleep(0.25)

                print('Front panel button is released')
                self.beep_notify()

            button_after = self.is_front_panel_button_pressed_function()
            led_after = self.is_front_panel_led_blue() if automatic else True

            if button_after:
                fatal_error('Front panel button is still pressed after test')

            if not led_after:
                fatal_error('Front panel LED is not on after test')
        except Exception as e:
            fatal_error('Could not test front panel button: {0}'.format(e))

    def get_nfc_tag_ids(self):
        return [self.try_action('20B', lambda device: device.simple_get_tag_id(i)) for i in range(8)]

    def set_led_strip_color(self, left, middle=None, right=None):
        if middle == None:
            middle = left

        if right == None:
            right = middle

        self.try_action('20A', lambda device: device.set_led_values(0, right * 3 + middle * 5 + left * 3))

    def beep_success(self):
        volume = 5
        beep_duration = 400

        self.try_action('03B', lambda device: device.set_beep(1200, volume, beep_duration))
        time.sleep(beep_duration / 1000)

    def beep_failure(self):
        volume = 8
        beep_duration = 175
        pause_duration = 50

        self.try_action('03B', lambda device: device.set_beep(400, volume, beep_duration))
        time.sleep((beep_duration + pause_duration) / 1000)
        self.try_action('03B', lambda device: device.set_beep(400, volume, beep_duration))
        time.sleep((beep_duration + pause_duration) / 1000)
        self.try_action('03B', lambda device: device.set_beep(400, volume, beep_duration))
        time.sleep((beep_duration) / 1000)

    def beep_notify(self):
        volume = 8
        beep_duration = 150

        self.try_action('03B', lambda device: device.set_beep(2400, volume, beep_duration))
        time.sleep(beep_duration / 1000)

    def reset_dc_fault(self, cp_pe_state):
        print('Resetting DC fault')

        self.reset_dc_fault_function()

        time.sleep(EVSE_SETTLE_DURATION)

        self.change_cp_pe_state('A')

        time.sleep(RELAY_SETTLE_DURATION + EVSE_SETTLE_DURATION)

        if not self.check_iec_state('A'):
            fatal_error('Wallbox not in IEC state A')

        print('Waiting DC protect calibration')

        time.sleep(DC_PROTECT_SETTLE_DURATION)

        print('Waiting for 30 second state E/F deadtime')

        time.sleep(30)

        if cp_pe_state != 'A':
            self.change_cp_pe_state(cp_pe_state)

            time.sleep(RELAY_SETTLE_DURATION + EVSE_SETTLE_DURATION)

            if not self.check_iec_state(cp_pe_state):
                fatal_error('Wallbox not in IEC state ' + cp_pe_state)

    def verify_evse_not_crashed(self):
        wall_clock_now = time.time()
        wall_clock_elapsed = wall_clock_now - self.wall_clock_start

        evse_uptime_now = self.get_evse_uptime_function()
        evse_uptime_elapsed = evse_uptime_now - self.evse_uptime_start
        if not wall_clock_elapsed * 0.9 < evse_uptime_elapsed < wall_clock_elapsed * 1.1:
            fatal_error(f'EVSE uptime not elapsing as expected:\n\t',
                        f'EVSE start {self.evse_uptime_start} now {evse_uptime_now} elapsed {evse_uptime_elapsed}\n\t'
                        f'WALL start {self.wall_clock_start} now {wall_clock_now} elapsed {wall_clock_elapsed}')

    # EVSE firmwares >= 2.2.12 require that the vehicle switches to B in 3 seconds after stopping the PWM.
    # Otherwise the EVSE will wait for 60 seconds before going back to C when phase switching.
    def switch_phases(self, phases):
        self.switch_phases_function(phases)
        while self.get_cp_pwm_function() != 1000:
            time.sleep(0.1)
        self.change_cp_pe_state('B')
        while self.get_cp_pwm_function() == 1000:
            time.sleep(0.1)
        self.change_cp_pe_state('C')
        time.sleep(PHASE_SWITCH_SETTLE_DURATION)

    def verify_voltages(self, p_type2: list[typing.Literal['L1', 'L2', 'L3']], p_meter: list[typing.Literal['L1', 'L2', 'L3']] | None = None):
        voltages = self.read_voltage_monitors()
        print('Reading voltages as {0}'.format(voltages), ' expecting ', p_type2)

        meter_voltages = self.get_meter_voltages_function()

        if p_meter is None:
            p_meter = p_type2

        if meter_voltages is None:
            # Set meter voltages to voltages if there's no energy meter connected
            # (i.e. this is not a Pro): voltages are checked first.
            meter_voltages = voltages
        else:
            print('Energy meter measured voltages as {0}'.format(meter_voltages), ' expecting ', p_meter)

        for i in range(3):
            name = ['L1', 'L2', 'L3'][i]
            expect_on = name in p_type2
            expect_on_meter = name in p_meter
            if expect_on and voltages[i] < VOLTAGE_ON_THRESHOLD:
                fatal_error(f'Missing voltage on {name}')
            elif not expect_on and voltages[i] > VOLTAGE_OFF_THRESHOLD:
                fatal_error(f'Unexpected voltage on {name}')

            if expect_on_meter and meter_voltages[i] < VOLTAGE_ON_THRESHOLD:
                fatal_error(f'Energy meter measured missing voltage on {name}')
            elif not expect_on_meter and meter_voltages[i] > VOLTAGE_OFF_THRESHOLD:
                fatal_error(f'Energy meter measured unexpected voltage on {name}')

    def is_meter_connected_to_usb(self):
        return subprocess.call(['lsusb', '-d', 'c251:7350'], stdout=subprocess.DEVNULL) == 0

    # requires power_on
    def test_wallbox(self, has_phase_switch):
        assert self.has_evse_error_function != None
        assert self.get_iec_state_function != None
        assert self.reset_dc_fault_function != None
        assert self.get_evse_uptime_function != None
        assert self.reset_evse_function != None

        if has_phase_switch:
            assert self.switch_phases_function != None

        while self.is_meter_connected_to_usb():
            self.beep_notify()

            while my_input('Disconnect electrical tester from USB and press y + return to continue') != 'y':
                pass

        if self.read_meter_qr_code(allow_no_detection=True) != '01':
            self.beep_notify()

            while my_input('Bring electrical tester to step 01/15 and press y + return to continue') != 'y':
                pass

        self.evse_uptime_start = self.get_evse_uptime_function()
        self.wall_clock_start = time.time()

        # step 01
        print('Testing wallbox, step 01/15, test IEC states')

        self.click_meter_run_button() # skip QR code

        print('Connecting power to L1, L2 and L3')

        self.connect_warp_power(['L1', 'L2', 'L3'])
        self.connect_voltage_monitors(True)

        time.sleep(RELAY_SETTLE_DURATION)

        # step 01: test IEC states
        for state in ['A', 'B', 'C', 'B', 'C', 'B', 'C', 'D']:
            self.change_cp_pe_state(state)

            time.sleep(RELAY_SETTLE_DURATION + EVSE_SETTLE_DURATION)

            if not self.check_iec_state(state):
                if state == 'D' and self.get_iec_state_function() == 'E':
                    fatal_error('Wallbox not in IEC state {0}. If the DC fault protector has triggered, please check whether the CP wire is (wronly) placed in the DC fault protector ring.'.format(state))

                fatal_error('Wallbox not in IEC state {0}'.format(state))

            if self.has_evse_error_function():
                if state != 'D':
                    fatal_error('Unexpected EVSE error')
            else:
                if state == 'D':
                    fatal_error('Missing EVSE error for IEC state D')

            time.sleep(VOLTAGE_SETTLE_DURATION)

            if state == 'C':
                self.verify_voltages(['L1', 'L2', 'L3'])
            else:
                self.verify_voltages(p_type2=[], p_meter=['L1', 'L2', 'L3'])

            self.verify_evse_not_crashed()

        # since EVSE 2.0 firmware 2.1.14 the contactor stays off for 30 seconds
        # after state D. the previous test ends with state D, so we need to leave
        # state D and then wait here for at least 30 seconds
        self.change_cp_pe_state('A')

        time.sleep(RELAY_SETTLE_DURATION + EVSE_SETTLE_DURATION)

        if not self.check_iec_state('A'):
            fatal_error('Wallbox not in IEC state A')

        print('Waiting for 30 second state D deadtime')

        time.sleep(30)

        self.verify_evse_not_crashed()

        # step 01: test phase separation
        self.change_cp_pe_state('C')
        time.sleep(EVSE_SETTLE_DURATION)
        if not self.check_iec_state('C'):
            fatal_error('Wallbox not in IEC state C')

        for phases in [['L1', 'L2'],
                       ['L1'],
                       ['L1', 'L3'],
                       ['L1', 'L2', 'L3']]:
            print('Connecting power to', ', '.join(phases))
            self.connect_warp_power(phases)
            time.sleep(RELAY_SETTLE_DURATION + VOLTAGE_SETTLE_DURATION)
            self.verify_voltages(phases)

        if has_phase_switch:
            print('Testing phase switch')

            self.switch_phases(1)
            self.verify_voltages(p_type2=['L1'], p_meter=['L1', 'L2', 'L3'])

            self.switch_phases(3)
            self.verify_voltages(['L1', 'L2', 'L3'])

        self.connect_voltage_monitors(False)
        time.sleep(RELAY_SETTLE_DURATION)

        self.verify_evse_not_crashed()

        # step 01: test PE disconnect
        print('Disconnecting PE')

        self.connect_type2_pe(False)

        time.sleep(RELAY_SETTLE_DURATION + EVSE_SETTLE_DURATION)

        if not self.check_iec_state('A'):
            fatal_error('Wallbox not in IEC state A')

        print('Reconnecting PE')

        self.connect_type2_pe(True)

        time.sleep(RELAY_SETTLE_DURATION + EVSE_SETTLE_DURATION)

        self.verify_evse_not_crashed()

        # step 01: mark test as passed
        self.click_meter_run_button()
        self.click_meter_run_button()
        self.click_meter_back_button()

        if self.read_meter_qr_code() != '02':
            fatal_error('Meter in wrong step')

        self.verify_evse_not_crashed()

        # step 02: test voltage L1
        print('Testing wallbox, step 02/15, test voltage L1')

        self.change_cp_pe_state('C')

        time.sleep(RELAY_SETTLE_DURATION + EVSE_SETTLE_DURATION)

        if not self.check_iec_state('C'):
            fatal_error('Wallbox not in IEC state C. Check contactor!')

        self.change_meter_state('Type2-L1')
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Waiting for test to become ready')

        time.sleep(METER_SETTLE_DURATION)

        print('Starting test')

        self.click_meter_run_button()

        if self.read_meter_qr_code(timeout=15) != '03':
            fatal_error('Step 02 timed out')

        self.verify_evse_not_crashed()

        # step 03: test Z auto L1
        print('Testing wallbox, step 03/15, test Z auto L1')

        self.click_meter_run_button() # skip QR code

        print('Waiting for test to become ready')

        time.sleep(METER_SETTLE_DURATION)

        print('Starting test')

        self.click_meter_run_button()

        if self.read_meter_qr_code(timeout=30) != '04':
            fatal_error('Step 03 timed out')

        self.verify_evse_not_crashed()

        # step 04: test voltage L2
        print('Testing wallbox, step 04/15, test voltage L2')

        self.change_meter_state('Type2-L2')
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '05':
            fatal_error('Step 04 timed out')

        self.verify_evse_not_crashed()

        # step 05: test Z auto L2
        print('Testing wallbox, step 05/15, test Z auto L2')

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=30) != '06':
            fatal_error('Step 05 timed out')

        self.verify_evse_not_crashed()

        # step 06: test voltage L3
        print('Testing wallbox, step 06/15, test voltage L3')

        self.change_meter_state('Type2-L3')
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '07':
            fatal_error('Step 06 timed out')

        self.verify_evse_not_crashed()

        # step 07: test Z auto L3
        print('Testing wallbox, step 07/15, test Z auto L3')

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=30) != '08':
            fatal_error('Step 07 timed out')

        self.verify_evse_not_crashed()

        # step 08: test RCD positive
        print('Testing wallbox, step 08/15, test RCD positive')

        self.change_meter_state('Type2-L1')

        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Waiting for test to become ready')

        time.sleep(METER_SETTLE_DURATION)

        self.click_meter_run_button()

        print('Starting test')

        if self.read_meter_qr_code(timeout=30) != '09':
            fatal_error('Step 08 timed out')

        self.reset_dc_fault('C')

        self.verify_evse_not_crashed()

        # step 09: test RCD negative
        print('Testing wallbox, step 09/15, test RCD negative')

        self.click_meter_run_button() # skip QR code

        print('Waiting for test to become ready')

        time.sleep(METER_SETTLE_DURATION)

        print('Starting test')

        self.click_meter_run_button()

        if self.read_meter_qr_code(timeout=30) != '10':
            fatal_error('Step 09 timed out')

        self.reset_dc_fault('A')

        self.verify_evse_not_crashed()

        # step 10: test R iso L1
        print('Testing wallbox, step 10/15, test R iso L1')

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '11':
            fatal_error('Step 10 timed out')

        self.verify_evse_not_crashed()

        # step 11: test R iso L2
        print('Testing wallbox, step 11/15, test R iso L2')

        self.change_meter_state('Type2-L2')
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '12':
            fatal_error('Step 11 timed out')

        self.verify_evse_not_crashed()

        # step 12: test R iso L3
        print('Testing wallbox, step 12/15, test R iso L3')

        self.change_meter_state('Type2-L3')
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '13':
            fatal_error('Step 12 timed out')

        self.verify_evse_not_crashed()

        # step 13: test R iso N
        print('Testing wallbox, step 13/15, test R iso N')

        self.change_meter_state('Type2-L1')
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '14':
            fatal_error('Step 13 timed out')

        self.verify_evse_not_crashed()

        # step 14: test R low front panel
        print('Testing wallbox, step 14/15, test R low front panel')
        print('Disconnecting front panel')

        self.connect_front_panel(True)
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '15':
            fatal_error('Step 14 timed out')

        print('Connecting front panel')

        self.connect_front_panel(False)
        time.sleep(RELAY_SETTLE_DURATION)

        self.verify_evse_not_crashed()

        # step 15: result
        print('Testing wallbox, step 15/15')

        self.connect_warp_power(['L1'])
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        self.verify_evse_not_crashed()

        print('Testing wallbox, done')

def main():
    stage3 = Stage3(is_front_panel_button_pressed_function=lambda: False,
                    has_evse_error_function=lambda: False,
                    get_iec_state_function=lambda: 'A',
                    reset_dc_fault_function=lambda: None,
                    switch_phases_function=lambda x: None,
                    get_evse_uptime_function=lambda: None,
                    reset_evse_function=lambda: None,
                    get_cp_pwm_function=lambda: 1000,
                    get_meter_voltages_function=lambda: None)

    stage3.setup()

    root = tk.Tk()
    root.title('WARP2 Manual Control')

    buttons = []

    def add_button(text, command):
        button = tk.Button(root, text=text, width=50, command=command)
        button.grid(row=len(buttons), column=0, padx=10, pady=10)
        buttons.append(button)

    add_button('Power On - Smart L1', lambda: stage3.power_on('Smart'))
    add_button('Power On - Smart L1/L2/L3', lambda: stage3.power_on('Smart', phases=['L1', 'L2', 'L3']))
    add_button('Power On - Pro L1', lambda: stage3.power_on('Pro'))
    add_button('Power On - Pro L1/L2/L3', lambda: stage3.power_on('Pro', phases=['L1', 'L2', 'L3']))
    add_button('Power On - CEE L1', lambda: stage3.power_on('CEE'))
    add_button('Power On - CEE L1/L2/L3', lambda: stage3.power_on('CEE', phases=['L1', 'L2', 'L3']))
    add_button('Power Off', lambda: stage3.power_off())
    add_button('CP/PE State A', lambda: stage3.change_cp_pe_state('A'))
    add_button('CP/PE State B', lambda: stage3.change_cp_pe_state('B'))
    add_button('CP/PE State C', lambda: stage3.change_cp_pe_state('C'))
    add_button('CP/PE State D', lambda: stage3.change_cp_pe_state('D'))
    add_button('Meter State Type2 L1', lambda: stage3.change_meter_state('Type2-L1'))
    add_button('Meter State Type2 L2', lambda: stage3.change_meter_state('Type2-L2'))
    add_button('Meter State Type2 L3', lambda: stage3.change_meter_state('Type2-L3'))
    add_button('Meter Run Button', lambda: stage3.click_meter_run_button())
    add_button('Meter Back Button', lambda: stage3.click_meter_back_button())

    root.mainloop()

    stage3.power_off()
    stage3.teardown()

    print('Done')

if __name__ == '__main__':
    try:
        main()
        input('Press return to exit ')
    except FatalError:
        input('Press return to exit ')
        sys.exit(1)
    except Exception as e:
        traceback.print_exc()
        input('Press return to exit ')
        sys.exit(1)
