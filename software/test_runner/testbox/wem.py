import sys
from time import sleep
from struct import pack, unpack
import os
import typing
from dataclasses import dataclass, field

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_industrial_dual_relay import BrickletIndustrialDualRelay
from tinkerforge.bricklet_industrial_digital_in_4_v2 import BrickletIndustrialDigitalIn4V2
from tinkerforge.bricklet_industrial_quad_relay_v2 import BrickletIndustrialQuadRelayV2
from tinkerforge.bricklet_rs485 import BrickletRS485

from .testbox import TestBox,  Meter, Input

"""
RPi Port    Bricklet            Pin     Function
A           Ind Dual Relay      0       Input 3 (open -> connected to contactor check, closed -> connected to quad relay (Port B)
                                1       Input 4 (open -> connected to contactor check, closed -> connected to quad relay (Port B)

B           Ind Quad Relay V2   0       Input 3 open/closed (iff port A pin 0 is closed)
                                1       Input 4 open/closed (iff port A pin 1 is closed)

C           Ind Quad Relay V2   0       RS485 to energy meter
                                1       RS485 to energy meter
                                2       RS485 to Bricklet
                                3       RS485 to Bricklet

D           Ind Digital In 4 V2 0       Relay (Output high -> closed, Output low -> open)
                                1       Contactor (high -> open, low -> closed)

E           RS485 Bricklet              Use to fake energy meters

F           Color Bricklet              WEM LED
"""


@dataclass
class WEMTestBox(TestBox):
    ipcon: IPConnection = None

    A: BrickletIndustrialDualRelay = None
    B: BrickletIndustrialQuadRelayV2 = None
    C: BrickletIndustrialQuadRelayV2 = None
    D: BrickletIndustrialDigitalIn4V2 = None
    E: BrickletRS485 = None

    meter_relays: dict[Meter, tuple[bool, bool, bool, bool]] = field(default_factory=lambda: {
        'none': (False, False, False, False),
        'real': (True, True, False, False),
        'fake': (False, False, True, True),
    })

    ## API
    def connect_meter(self, m: Meter):
        try:
            self.C.set_value(self.meter_relays[m])
        except Exception as e:
            print(e)

    def get_meter_connected(self):
        try:
            r = self.C.get_value()
            for k, v in self.meter_relays.items():
                if r == v:
                    return k
        except Exception as e:
            print(e)
            return 'none'

    def set_input_3(self, i: Input):
        self._set_input(0, i)

    def get_input_3(self):
        return self._get_input(0)

    def set_input_4(self, i: Input):
        self._set_input(1, i)

    def get_input_4(self):
        return self._get_input(1)

    def is_relay_closed(self):
        try:
            return self.D.get_value()[0]
        except Exception as e:
            print(e)
            return False

    def is_contactor_closed(self):
        try:
            return not self.D.get_value()[1]
        except Exception as e:
            print(e)
            return False

    def _get_led_state(self):
        return "unknown"

    def reset(self):
        self.set_input_3('open')
        self.set_input_4('contactor')
        self.connect_meter('real')

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

    def stop(self):
        self.ipcon.disconnect()

    ## Internals

    def _bricklets(self):
        return [self.A, self.B, self.C, self.D, self.E]

    def _bricklet_types(self):
        return [BrickletIndustrialDualRelay,
                BrickletIndustrialQuadRelayV2,
                BrickletIndustrialQuadRelayV2,
                BrickletIndustrialDigitalIn4V2,
                BrickletRS485]

    def _bricklet_names(self):
        return [x.DEVICE_DISPLAY_NAME for x in self._bricklet_types()]

    def _set_input(self, x, i: Input):
        if i == 'open':
            self.B.set_selected_value(x, False)
            self.A.set_selected_value(x, True)
        elif i == 'closed':
            self.B.set_selected_value(x, True)
            self.A.set_selected_value(x, True)
        elif i == 'contactor':
            self.A.set_selected_value(x, False)

    def _get_input(self, x):
        if not self.A.get_value()[x]:
            return 'contactor'
        return 'closed' if self.B.get_value()[x] else 'open'

    def _cb_enumerate(self, uid, connected_uid, position, hardware_version, firmware_version,
                    device_identifier, enumeration_type):
        if enumeration_type == IPConnection.ENUMERATION_TYPE_DISCONNECTED:
            return

        dev_ids = [x.DEVICE_IDENTIFIER for x in self._bricklet_types()]

        for i in range(len(dev_ids)):
            if ord(position) == ord('a') + i and device_identifier == dev_ids[i]:
                self.__dict__[chr(ord('A') + i)] = self._bricklet_types()[i](uid, self.ipcon)

    def _cb_connected(self, reason):
        self.ipcon.enumerate()
