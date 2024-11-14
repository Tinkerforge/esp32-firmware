#!/usr/bin/python3 -u

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')

import sys
import queue
import threading
import time
import traceback
import tkinter as tk

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
from provisioning.provision_common.provision_common import FatalError, fatal_error, green

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

PHASE_SWITCH_SETTLE_DURATION = 7.0 # seconds

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
    def __init__(self, is_front_panel_button_pressed_function, has_evse_error_function, get_iec_state_function, reset_dc_fault_function, switch_phases_function):
        self.is_front_panel_button_pressed_function = is_front_panel_button_pressed_function
        self.has_evse_error_function = has_evse_error_function
        self.get_iec_state_function = get_iec_state_function
        self.reset_dc_fault_function = reset_dc_fault_function
        self.switch_phases_function = switch_phases_function
        self.ipcon = IPConnection()
        self.inventory = Inventory(self.ipcon)
        self.devices = {} # by position path
        self.prepared = False
        self.action_stop_queue = None
        self.action_enabled_ref = None
        self.action_thread = None

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

            self.set_servo_position(servo, channel, 6800)
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

            self.set_servo_position(servo, channel, 5500)
            time.sleep(0.1)
            self.set_servo_position(servo, channel, 3000)
            time.sleep(0.1)
        except Exception as e:
            fatal_error('Could not click meter back button: {0}'.format(e))

    # internal
    def read_meter_qr_code(self, timeout=5):
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

        if len(text) == 0:
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
        if color[0] == 65535 and color[1] == 65535 and color[2] == 65535 and color[3] == 65535:
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

                    if bricklet_entry.device_identifier == BrickletNFC.DEVICE_IDENTIFIER:
                        device = BrickletNFC(bricklet_entry.uid, self.ipcon)
                    else:
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

    def power_on(self, outlet):
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

        self.connect_warp_power(['L1'])

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
                fatal_error('Front panel LED is not on before test')

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
                fatal_error('Front panel LED is still on during test')

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

    # requires power_on
    def test_wallbox(self, has_phase_switch):
        assert self.has_evse_error_function != None
        assert self.get_iec_state_function != None
        assert self.reset_dc_fault_function != None
        if has_phase_switch:
            assert self.switch_phases_function != None

        if self.read_meter_qr_code() != '01':
            fatal_error('Meter in wrong step')

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

            voltages = self.read_voltage_monitors()

            print('Reading voltages as {0}'.format(voltages))

            for i, phase in enumerate(['L1', 'L2', 'L3']):
                if state == 'C':
                    if voltages[i] < VOLTAGE_ON_THRESHOLD:
                        fatal_error('Missing voltage on {0}'.format(phase))
                else:
                    if voltages[i] > VOLTAGE_OFF_THRESHOLD:
                        fatal_error('Unexpected voltage on {0}'.format(phase))

        # since EVSE 2.0 firmware 2.1.14 the contactor stays off for 30 seconds
        # after state D. the previous test ends with state D, so we need to leave
        # state D and then wait here for at least 30 seconds
        self.change_cp_pe_state('A')

        time.sleep(RELAY_SETTLE_DURATION + EVSE_SETTLE_DURATION)

        if not self.check_iec_state('A'):
            fatal_error('Wallbox not in IEC state A')

        print('Waiting for 30 second state D deadtime')

        time.sleep(30)

        # step 01: test phase separation
        print('Connecting power to L1 and L2')

        self.connect_warp_power(['L1', 'L2'])
        self.change_cp_pe_state('C')

        time.sleep(RELAY_SETTLE_DURATION + EVSE_SETTLE_DURATION)

        if not self.check_iec_state('C'):
            fatal_error('Wallbox not in IEC state C')

        time.sleep(VOLTAGE_SETTLE_DURATION)

        voltages = self.read_voltage_monitors()

        print('Reading voltages as {0}'.format(voltages))

        if voltages[0] < VOLTAGE_ON_THRESHOLD:
            fatal_error('Missing voltage on L1')

        if voltages[1] < VOLTAGE_ON_THRESHOLD:
            fatal_error('Missing voltage on L2')

        if voltages[2] > VOLTAGE_OFF_THRESHOLD:
            fatal_error('Unexpected voltage on L3')

        print('Connecting power to L1')

        self.connect_warp_power(['L1'])

        time.sleep(RELAY_SETTLE_DURATION + VOLTAGE_SETTLE_DURATION)

        voltages = self.read_voltage_monitors()

        print('Reading voltages as {0}'.format(voltages))

        if voltages[0] < VOLTAGE_ON_THRESHOLD:
            fatal_error('Missing voltage on L1')

        if voltages[1] > VOLTAGE_OFF_THRESHOLD:
            fatal_error('Unexpected voltage on L2')

        if voltages[2] > VOLTAGE_OFF_THRESHOLD:
            fatal_error('Unexpected voltage on L3')

        print('Connecting power to L1 and L3')

        self.connect_warp_power(['L1', 'L3'])

        time.sleep(RELAY_SETTLE_DURATION + VOLTAGE_SETTLE_DURATION)

        voltages = self.read_voltage_monitors()

        print('Reading voltages as {0}'.format(voltages))

        if voltages[0] < VOLTAGE_ON_THRESHOLD:
            fatal_error('Missing voltage on L1')

        if voltages[1] > VOLTAGE_OFF_THRESHOLD:
            fatal_error('Unexpected voltage on L2')

        if voltages[2] < VOLTAGE_ON_THRESHOLD:
            fatal_error('Missing voltage on L3')

        print('Connecting power to L1, L2 and L3')

        self.connect_warp_power(['L1', 'L2', 'L3'])

        if has_phase_switch:
            time.sleep(RELAY_SETTLE_DURATION + VOLTAGE_SETTLE_DURATION)

            voltages = self.read_voltage_monitors()

            print('Reading voltages as {0}'.format(voltages))

            if voltages[0] < VOLTAGE_ON_THRESHOLD:
                fatal_error('Missing voltage on L1')

            if voltages[1] < VOLTAGE_OFF_THRESHOLD:
                fatal_error('Missing voltage on L2')

            if voltages[2] < VOLTAGE_ON_THRESHOLD:
                fatal_error('Missing voltage on L3')

            print('Testing phase switch')

            self.switch_phases_function(1)
            time.sleep(PHASE_SWITCH_SETTLE_DURATION + VOLTAGE_SETTLE_DURATION)

            voltages = self.read_voltage_monitors()

            print('Reading voltages as {0}'.format(voltages))

            if voltages[0] < VOLTAGE_ON_THRESHOLD:
                fatal_error('Missing voltage on L1')

            if voltages[1] > VOLTAGE_OFF_THRESHOLD:
                fatal_error('Unexpected voltage on L2')

            if voltages[2] > VOLTAGE_ON_THRESHOLD:
                fatal_error('Unexpected voltage on L3')

            self.switch_phases_function(3)
            time.sleep(PHASE_SWITCH_SETTLE_DURATION + VOLTAGE_SETTLE_DURATION)

            voltages = self.read_voltage_monitors()

            print('Reading voltages as {0}'.format(voltages))

            if voltages[0] < VOLTAGE_ON_THRESHOLD:
                fatal_error('Missing voltage on L1')

            if voltages[1] < VOLTAGE_OFF_THRESHOLD:
                fatal_error('Missing voltage on L2')

            if voltages[2] < VOLTAGE_ON_THRESHOLD:
                fatal_error('Missing voltage on L3')

        self.connect_voltage_monitors(False)
        time.sleep(RELAY_SETTLE_DURATION)

        # step 01: test PE disconnect
        print('Disconnecting PE')

        self.connect_type2_pe(False)

        time.sleep(RELAY_SETTLE_DURATION + EVSE_SETTLE_DURATION)

        if not self.check_iec_state('A'):
            fatal_error('Wallbox not in IEC state A')

        print('Reconnecting PE')

        self.connect_type2_pe(True)

        time.sleep(RELAY_SETTLE_DURATION + EVSE_SETTLE_DURATION)

        # step 01: mark test as passed
        self.click_meter_run_button()
        self.click_meter_run_button()
        self.click_meter_back_button()

        if self.read_meter_qr_code() != '02':
            fatal_error('Meter in wrong step')

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
            fatal_error('Step 02 timeouted')

        # step 03: test Z auto L1
        print('Testing wallbox, step 03/15, test Z auto L1')

        self.click_meter_run_button() # skip QR code

        print('Waiting for test to become ready')

        time.sleep(METER_SETTLE_DURATION)

        print('Starting test')

        self.click_meter_run_button()

        if self.read_meter_qr_code(timeout=30) != '04':
            fatal_error('Step 03 timeouted')

        # step 04: test voltage L2
        print('Testing wallbox, step 04/15, test voltage L2')

        self.change_meter_state('Type2-L2')
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '05':
            fatal_error('Step 04 timeouted')

        # step 05: test Z auto L2
        print('Testing wallbox, step 05/15, test Z auto L2')

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=30) != '06':
            fatal_error('Step 05 timeouted')

        # step 06: test voltage L3
        print('Testing wallbox, step 06/15, test voltage L3')

        self.change_meter_state('Type2-L3')
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '07':
            fatal_error('Step 06 timeouted')

        # step 07: test Z auto L3
        print('Testing wallbox, step 07/15, test Z auto L3')

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=30) != '08':
            fatal_error('Step 07 timeouted')

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
            fatal_error('Step 08 timeouted')

        self.reset_dc_fault('C')

        # step 09: test RCD negative
        print('Testing wallbox, step 09/15, test RCD negative')

        self.click_meter_run_button() # skip QR code

        print('Waiting for test to become ready')

        time.sleep(METER_SETTLE_DURATION)

        print('Starting test')

        self.click_meter_run_button()

        if self.read_meter_qr_code(timeout=30) != '10':
            fatal_error('Step 09 timeouted')

        self.reset_dc_fault('A')

        # step 10: test R iso L1
        print('Testing wallbox, step 10/15, test R iso L1')

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '11':
            fatal_error('Step 10 timeouted')

        # step 11: test R iso L2
        print('Testing wallbox, step 11/15, test R iso L2')

        self.change_meter_state('Type2-L2')
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '12':
            fatal_error('Step 11 timeouted')

        # step 12: test R iso L3
        print('Testing wallbox, step 12/15, test R iso L3')

        self.change_meter_state('Type2-L3')
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '13':
            fatal_error('Step 12 timeouted')

        # step 13: test R iso N
        print('Testing wallbox, step 13/15, test R iso N')

        self.change_meter_state('Type2-L1')
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '14':
            fatal_error('Step 13 timeouted')

        # step 14: test R low front panel
        print('Testing wallbox, step 14/15, test R low front panel')
        print('Disconnecting front panel')

        self.connect_front_panel(True)
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Test autostarts')

        if self.read_meter_qr_code(timeout=15) != '15':
            fatal_error('Step 14 timeouted')

        print('Connecting front panel')

        self.connect_front_panel(False)
        time.sleep(RELAY_SETTLE_DURATION)

        # step 15: result
        print('Testing wallbox, step 15/15')

        self.connect_warp_power(['L1'])
        time.sleep(RELAY_SETTLE_DURATION)

        self.click_meter_run_button() # skip QR code

        print('Testing wallbox, done')

def main():
    stage3 = Stage3(is_front_panel_button_pressed_function=lambda: False,
                    has_evse_error_function=lambda: False,
                    get_iec_state_function=lambda: 'A',
                    reset_dc_fault_function=lambda: None,
                    switch_phases_function=lambda x: None)

    stage3.setup()

    root = tk.Tk()
    root.title('WARP2 Manual Power')

    button_power_on_smart = tk.Button(root, text='Power On - Smart', width=50, command=lambda: stage3.power_on('Smart'))
    button_power_on_smart.grid(row=0, column=0, padx=10, pady=10)

    button_power_on_pro = tk.Button(root, text='Power On - Pro', width=50, command=lambda: stage3.power_on('Pro'))
    button_power_on_pro.grid(row=1, column=0, padx=10, pady=0)

    button_power_on_cee = tk.Button(root, text='Power On - CEE', width=50, command=lambda: stage3.power_on('CEE'))
    button_power_on_cee.grid(row=2, column=0, padx=10, pady=10)

    button_power_off = tk.Button(root, text='Power Off', width=50, command=lambda: stage3.power_off())
    button_power_off.grid(row=3, column=0, padx=10, pady=0)

    button_cp_pe_state_a = tk.Button(root, text='CP/PE State A', width=50, command=lambda: stage3.change_cp_pe_state('A'))
    button_cp_pe_state_a.grid(row=4, column=0, padx=10, pady=10)

    button_cp_pe_state_b = tk.Button(root, text='CP/PE State B', width=50, command=lambda: stage3.change_cp_pe_state('B'))
    button_cp_pe_state_b.grid(row=5, column=0, padx=10, pady=0)

    button_cp_pe_state_c = tk.Button(root, text='CP/PE State C', width=50, command=lambda: stage3.change_cp_pe_state('C'))
    button_cp_pe_state_c.grid(row=6, column=0, padx=10, pady=10)

    button_cp_pe_state_d = tk.Button(root, text='CP/PE State D', width=50, command=lambda: stage3.change_cp_pe_state('D'))
    button_cp_pe_state_d.grid(row=7, column=0, padx=10, pady=0)

    button_meter_state_type2_l1 = tk.Button(root, text='Meter State Type2 L1', width=50, command=lambda: stage3.change_meter_state('Type2-L1'))
    button_meter_state_type2_l1.grid(row=8, column=0, padx=10, pady=10)

    button_meter_state_type2_l2 = tk.Button(root, text='Meter State Type2 L2', width=50, command=lambda: stage3.change_meter_state('Type2-L2'))
    button_meter_state_type2_l2.grid(row=9, column=0, padx=10, pady=0)

    button_meter_state_type2_l3 = tk.Button(root, text='Meter State Type2 L3', width=50, command=lambda: stage3.change_meter_state('Type2-L3'))
    button_meter_state_type2_l3.grid(row=10, column=0, padx=10, pady=10)

    root.mainloop()

    stage3.power_off()
    stage3.teardown()

    print('Done')

if __name__ == '__main__':
    try:
        main()
    except FatalError:
        input('Press return to exit ')
        sys.exit(1)
    except Exception as e:
        traceback.print_exc()
        input('Press return to exit ')
        sys.exit(1)
