from time import sleep
import typing
from dataclasses import dataclass

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_industrial_counter import BrickletIndustrialCounter
from tinkerforge.bricklet_industrial_digital_in_4_v2 import BrickletIndustrialDigitalIn4V2
from tinkerforge.bricklet_industrial_quad_relay_v2 import BrickletIndustrialQuadRelayV2
from tinkerforge.bricklet_rs485 import BrickletRS485
from tinkerforge.bricklet_energy_monitor import BrickletEnergyMonitor
from tinkerforge.bricklet_nfc import BrickletNFC
from tinkerforge.bricklet_industrial_dual_ac_in import BrickletIndustrialDualACIn

from .testbox import TestBox, CP, Meter

"""
RPi Port    Bricklet            Pin     Function
A           Ind Quad Relay V2   0       Front button (relay closed = button not pressed, relay open = button pressed)
                                1       CP B (2700 Ω)
                                2       CP C (880 Ω) (always switch with B)
                                3       CP D (240 Ω) (always switch with B)

B           Ind Quad Relay V2   0       RS485 to energy meter
                                1       RS485 to energy meter
                                2       RS485 to Bricklet
                                3       RS485 to Bricklet

C           RS485 Bricklet              Use to fake energy meters

D           Ind Quad Relay V2   0       GPIO Shutdown Input
                                1       GPIO Input
                                2       DC Fault
                                3       Diode Fault

E           Ind Digital In 4 V2 0       GPIO Output (Output Z -> low, Output low -> high)

F           Ind Counter         0       Front LED

G           Energy Monitor or           Check if contactor is switched
            Ind Dual AC In

H           NFC                         Fake NFC tags
"""

DUTY_CYCLE_MAX_LEN = 5000

@dataclass
class WARP2TestBox(TestBox):
    ipcon: IPConnection = None

    A: BrickletIndustrialQuadRelayV2 = None
    B: BrickletIndustrialQuadRelayV2 = None
    C: BrickletRS485 = None
    D: BrickletIndustrialQuadRelayV2 = None
    E: BrickletIndustrialDigitalIn4V2 = None
    F: BrickletIndustrialCounter = None
    G: BrickletEnergyMonitor = None
    H: BrickletNFC = None

    def _bricklets(self):
        return [self.A, self.B, self.C, self.D, self.E, self.F, self.G, self.H]

    def _bricklet_names(self):
        return [BrickletIndustrialQuadRelayV2.DEVICE_DISPLAY_NAME,
                BrickletIndustrialQuadRelayV2.DEVICE_DISPLAY_NAME,
                BrickletRS485.DEVICE_DISPLAY_NAME,
                BrickletIndustrialQuadRelayV2.DEVICE_DISPLAY_NAME,
                BrickletIndustrialDigitalIn4V2.DEVICE_DISPLAY_NAME,
                BrickletIndustrialCounter.DEVICE_DISPLAY_NAME,
                BrickletEnergyMonitor.DEVICE_DISPLAY_NAME + " or " + BrickletIndustrialDualACIn.DEVICE_DISPLAY_NAME,
                BrickletNFC.DEVICE_DISPLAY_NAME]

    cp_relays = {
        'A': (False, False, False),
        'B': (True, False, False),
        'C': (True, True, False),
        'D': (True, False, True)
    }

    meter_relays = {
        'none': (False, False, False, False),
        'real': (True, True, False, False),
        'fake': (False, False, True, True),
    }

    _duty_cycle_ringbuf = []
    _value_ringbuf = []

    ## API
    def set_button_pressed(self, pressed: bool):
        try:
            self.A.set_selected_value(0, not pressed)
        except Exception as e:
            print(e)

    def is_button_pressed(self):
        try:
            return not self.A.get_value()[0]
        except Exception as e:
            print(e)
            return False

    def set_cp(self, cp: CP):
        try:
            self.A.set_value([self.A.get_value()[0], *self.cp_relays[cp]])
        except Exception as e:
            print(e)

    def get_cp(self):
        try:
            r = self.A.get_value()[1:]
            for k, v in self.cp_relays.items():
                if r == v:
                    return k
        except Exception as e:
            print(e)

        return 'D'

    def connect_meter(self, m: Meter):
        try:
            self.B.set_value(self.meter_relays[m])
        except Exception as e:
            print(e)

    def get_meter_connected(self):
        try:
            r = self.B.get_value()
            for k, v in self.meter_relays.items():
                if r == v:
                    return k
        except Exception as e:
            print(e)

        return 'none'

    def set_gpio_shutdown_closed(self, val: bool):
        try:
            self.D.set_selected_value(0, val)
        except Exception as e:
            print(e)

    def is_gpio_shutdown_closed(self):
        try:
            return self.D.get_value()[0]
        except Exception as e:
            print(e)
            return False

    def set_gpio_input_closed(self, val: bool):
        try:
            self.D.set_selected_value(1, val)
        except Exception as e:
            print(e)

    def is_gpio_input_closed(self):
        try:
            return self.D.get_value()[1]
        except Exception as e:
            print(e)
            return False

    def is_gpio_output_closed(self):
        try:
            return not self.E.get_value()[0]
        except Exception as e:
            print(e)
            return False

    def is_contactor_closed(self, contactor: int = 0):
        try:
            if isinstance(self.G, BrickletIndustrialDualACIn):
                return self.G.get_value()[0]
            return self.G.get_energy_data()[0] / 100 > 10
        except Exception as e:
            print(e)
            return False

    def set_dc_faulted(self, fault: bool, clear_after_s: int = 0):
        try:
            if clear_after_s != 0:
                self.D.set_monoflop(2, fault, int(clear_after_s * 1000))
            else:
                self.D.set_selected_value(2, fault)
        except Exception as e:
            print(e)

    def is_dc_fauled(self):
        try:
            return self.D.get_value()[2]
        except Exception as e:
            print(e)
            return False

    def set_diode_faulted(self, fault: bool, clear_after_s: int = 0):
        try:
            if clear_after_s != 0:
                self.D.set_monoflop(3, fault, int(clear_after_s * 1000))
            else:
                self.D.set_selected_value(3, fault)
        except Exception as e:
            print(e)

    def is_diode_fauled(self):
        try:
            return self.D.get_value()[3]
        except Exception as e:
            print(e)
            return False

    def reset(self):
        self.set_cp('A')
        self.connect_meter('real')
        self.set_gpio_shutdown_closed(False)
        self.set_gpio_input_closed(False)
        self.set_button_pressed(False)
        self.set_dc_faulted(False)
        self.set_diode_faulted(False)

    def start(self, host, port):
        self.ipcon = IPConnection()
        self.ipcon.connect(host, port)

        self.ipcon.register_callback(IPConnection.CALLBACK_ENUMERATE, self._cb_enumerate)
        self.ipcon.register_callback(IPConnection.CALLBACK_CONNECTED, self._cb_connected)

        self.ipcon.enumerate()
        for _ in range(30):
            if all(x is not None for x in self._bricklets()):
                break
            sleep(0.1)

        for i, x in enumerate(self._bricklets()):
            if x is None:
                raise Exception("Missing device at port {}!".format(chr(ord('A') + i)))

        for x in self._bricklets():
            x.set_response_expected_all(True)

        self.reset()

        self.F.set_counter_configuration(self.F.CHANNEL_0,
                                    self.F.COUNT_EDGE_RISING,
                                    self.F.COUNT_DIRECTION_UP,
                                    self.F.DUTY_CYCLE_PRESCALER_1,
                                    self.F.FREQUENCY_INTEGRATION_TIME_128_MS)
        self.F.set_response_expected_all(False)
        self.F.set_all_signal_data_callback_configuration(2, False)
        self.F.register_callback(self.F.CALLBACK_ALL_SIGNAL_DATA, self._cb_all_signal_data)

    def stop(self):
        self.ipcon.disconnect()

    ## Internals

    def _cb_enumerate(self, uid, connected_uid, position, hardware_version, firmware_version,
                    device_identifier, enumeration_type):
        if enumeration_type == IPConnection.ENUMERATION_TYPE_DISCONNECTED:
            return

        if   position == 'a' and device_identifier == BrickletIndustrialQuadRelayV2.DEVICE_IDENTIFIER:
            self.A = BrickletIndustrialQuadRelayV2(uid, self.ipcon)
        elif position == 'b' and device_identifier == BrickletIndustrialQuadRelayV2.DEVICE_IDENTIFIER:
            self.B = BrickletIndustrialQuadRelayV2(uid, self.ipcon)
        elif position == 'c' and device_identifier == BrickletRS485.DEVICE_IDENTIFIER:
            self.C = BrickletRS485(uid, self.ipcon)
        elif position == 'd' and device_identifier == BrickletIndustrialQuadRelayV2.DEVICE_IDENTIFIER:
            self.D = BrickletIndustrialQuadRelayV2(uid, self.ipcon)
        elif position == 'e' and device_identifier == BrickletIndustrialDigitalIn4V2.DEVICE_IDENTIFIER:
            self.E = BrickletIndustrialDigitalIn4V2(uid, self.ipcon)
        elif position == 'f' and device_identifier == BrickletIndustrialCounter.DEVICE_IDENTIFIER:
            self.F = BrickletIndustrialCounter(uid, self.ipcon)
        elif position == 'g' and device_identifier == BrickletEnergyMonitor.DEVICE_IDENTIFIER:
            self.G = BrickletEnergyMonitor(uid, self.ipcon)
        elif position == 'g' and device_identifier == BrickletIndustrialDualACIn.DEVICE_IDENTIFIER:
            self.G = BrickletIndustrialDualACIn(uid, self.ipcon)
        elif position == 'h' and device_identifier == BrickletNFC.DEVICE_IDENTIFIER:
            self.H = BrickletNFC(uid, self.ipcon)

    def _cb_connected(self, reason):
        self.ipcon.enumerate()

    def _cb_all_signal_data(self, duty_cycle, period, frequency, value):
        if len(self._duty_cycle_ringbuf) == DUTY_CYCLE_MAX_LEN:
            self._duty_cycle_ringbuf.pop(0)
            self._value_ringbuf.pop(0)

        if duty_cycle[0] > 9000:
            self._duty_cycle_ringbuf.append(100)
        else:
            self._duty_cycle_ringbuf.append(round(duty_cycle[0] / 1000.0) * 10)

        self._value_ringbuf.append(value[0])

    def __get_led_state(self):
        if len(self._duty_cycle_ringbuf) < DUTY_CYCLE_MAX_LEN:
            return "init"

        duty_cycles = self._duty_cycle_ringbuf[:]
        values = self._value_ringbuf[:]

        rise_start = []
        rise_end = []
        fall_start = []
        fall_end = []

        for i, x in enumerate(duty_cycles[1:-1]):
            if duty_cycles[i] == 0 and duty_cycles[i + 1] > 0:
                rise_start.append(i)
            if duty_cycles[i-1] > 0 and duty_cycles[i] == 0:
                fall_end.append(i)
            if duty_cycles[i] == 100 and duty_cycles[i + 1] < 100:
                fall_start.append(i)
            if duty_cycles[i-1] < 100 and duty_cycles[i] == 100:
                rise_end.append(i)

        #return "\n".join([str(x) for x in [rise_start, rise_end, fall_start, fall_end]]) + [] if len(rise_start) > 0 else str(values)

        periods = []

        p = [-1,-1,-1,-1,-1]

        S_NONE = 0
        S_WAIT_FOR_RE = 1
        S_WAIT_FOR_FS = 2
        S_WAIT_FOR_FE = 3
        S_WAIT_FOR_RS = 4

        state = S_NONE

        # plt.plot(duty_cycles, label='duty cycle')
        # plt.plot(rise_start, [duty_cycles[i] for i in rise_start], 's', label='rise start')
        # plt.plot(rise_end, [duty_cycles[i] for i in rise_end], 's', label='rise end')
        # plt.plot(fall_start, [duty_cycles[i] for i in fall_start], 's', label='fall start')
        # plt.plot(fall_end, [duty_cycles[i] for i in fall_end], 's', label='fall end')
        # plt.legend()
        # plt.ion()
        # plt.show()

        known_patterns = {
            "nag": [80, 10, 80, 266],
            "nack": [1, 5, 90, 231],
            "ack": [180, 10, 1, 66],
            "breathing": [440, 50, 440, 330]
        }

        for i in range(len(duty_cycles)):
            # Allow multiple points on the same i
            while True:
                if i in rise_start:
                    rise_start.remove(i)
                    if state == S_WAIT_FOR_RS:
                        p[4] = i
                        if p[4] - p[0] > 200:
                            periods.append([x for x in p])
                        p = [-1,-1,-1,-1,-1]
                        p[0] = i
                        state = S_WAIT_FOR_RE
                    elif state == S_NONE:
                        p[0] = i
                        state = S_WAIT_FOR_RE
                    else:
                        p = [-1,-1,-1,-1,-1]
                        p[0] = i
                        state = S_WAIT_FOR_RE
                        break
                elif i in rise_end:
                    rise_end.remove(i)
                    if state == S_WAIT_FOR_RE:
                        p[1] = i
                        state = S_WAIT_FOR_FS
                    else:
                        p = [-1,-1,-1,-1,-1]
                        state = S_NONE
                        break
                elif i in fall_start:
                    fall_start.remove(i)
                    if state == S_WAIT_FOR_FS:
                        p[2] = i
                        state = S_WAIT_FOR_FE
                    else:
                        p = [-1,-1,-1,-1,-1]
                        state = S_NONE
                        break
                elif i in fall_end:
                    fall_end.remove(i)
                    if state == S_WAIT_FOR_FE:
                        p[3] = i
                        state = S_WAIT_FOR_RS
                    else:
                        p = [-1,-1,-1,-1,-1]
                        state = S_NONE
                        break
                else:
                    break

        if state == S_WAIT_FOR_RS:
            p[4] = len(duty_cycles) - 1
            periods.append([x for x in p])

        # Try to find NFC patterns in first period only. We are only interested in "one" answer.
        if len(periods) > 0:
            p = periods[max(0, len(periods) - 2)]

            rise_time = p[1] - p[0]
            high_time = p[2] - p[1]
            fall_time = p[3] - p[2]
            low_time  = p[4] - p[3]
            pattern = [rise_time, high_time, fall_time, low_time]

            min_pattern_name = ''
            min_diff = 0xFFFFFFFF
            for k, v in known_patterns.items():
                diff = sum(abs(l - r) for l, r in zip(pattern, v))
                if diff < min_diff:
                    min_diff = diff
                    min_pattern_name = k

            if p[4] == len(duty_cycles) - 1:
                p[4] = min(p[3] + known_patterns[min_pattern_name][3], len(duty_cycles) - 1)

            return min_pattern_name
            #p.append(min_pattern_name)

            #result += "{} from {} to {}\n".format(min_pattern_name, p[0], p[4])

        intervals = [
            [0, len(duty_cycles) - 1, "unknown"]
        ]

        for p in periods:
            for i in range(len(intervals)):
                interval = intervals[i]
                if interval[0] <= p[0] < interval[1]:
                    pre = [interval[0], p[0], interval[2]]
                    inner = [p[0], p[4], p[5]]
                    post = [p[4], interval[1], interval[2]]
                    del intervals[i]
                    intervals.insert(i, pre)
                    intervals.insert(i + 1, inner)
                    intervals.insert(i + 2, post)
                    break

        intervals = [i for i in intervals if i[0] != i[1]]

        # We still have to handle LED on, off and blinking.
        # Blink timings are X *(250ms on, 250ms off) + 2000 ms off
        # -> The LED may only be regarded as off if the off time is ~ 3 seconds (1500 entries)

        if len(intervals) == 1 and intervals[0][2] == 'unknown':
            # first: check if the _last_ three seconds are off or on; return immediately
            l = 1500
            last_three_s = values[:-l]
            on = sum(last_three_s)
            off = l - on

            if off / l > 0.9:
                return 'off'
            elif on / l > 0.9:
                return 'on'

            # inconclusive. search for error blinks.
            # First, search for off times -> blocks of ~ 2 seconds (we accept 1.9-2.1) where the led is off
            # To make this easier, convert the bool list to a string, so that we can use .find()
            value_str = "".join("T" if x else "F" for x in values)

            def flatten(list_of_lists):
                return sum(list_of_lists, [])

            blocks = flatten([x.split("FT") for x in value_str.split("TF")])

            blink_counter = 0
            blinks = []
            for b in blocks:
                if len(b) > 0 and b[0] == 'F' and (900 < len(b) < 1250):
                    if blink_counter > 0:
                        blinks.append(blink_counter)
                    blink_counter = 0
                if len(b) > 0 and b[0] == 'T' and (100 < len(b) < 150):
                    blink_counter += 1
            if len(blinks) > 0:
                return f'error{max(blinks)}'

        return 'unknown'

