from time import sleep
from dataclasses import dataclass, field

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_io4_v2 import BrickletIO4V2
from tinkerforge.bricklet_industrial_quad_relay_v2 import BrickletIndustrialQuadRelayV2
from tinkerforge.bricklet_industrial_dual_relay import BrickletIndustrialDualRelay
from tinkerforge.bricklet_rs485 import BrickletRS485
from tinkerforge.bricklet_color_v2 import BrickletColorV2
from tinkerforge.bricklet_nfc import BrickletNFC

from .testbox import TestBox, CP, Meter, Aux

"""
RPi Port    Bricklet            Pin     Function
A           RS485 Bricklet              Use to fake energy meters

B           Ind Dual Relay      0       RS485 to energy meter (NC), to Bricklet (NO)
                                1       RS485 to energy meter (NC), to Bricklet (NO)

C           IO 4                0       Expansion Port 0
                                1       Expansion Port 1
                                2       Expansion Port 2

D           Ind Dual Relay      0       Contactor 0 Aux Contact connected (NC), to B -> 0 (N0)
                                1       Contactor 1 Aux Contact connected (NC), to B -> 0 (N0)

E           Ind Quad Relay V2   0       Front button (relay closed = button not pressed, relay open = button pressed)
                                1       DC Fault
                                2       Contactor 0 Aux Contact (if F -> 0 is NO)
                                3       Contactor 1 Aux Contact (if F -> 1 is NO)

F           Ind Quad Relay V2   missing: 0       Diode Fault
                                1       CP B (2700 Ω)
                                2       CP C (880 Ω) (always switch with B)
                                3       CP D (240 Ω) (always switch with B)

G           Color Bricklet              Front LED color

H           NFC                         Fake NFC tags
"""

@dataclass
class WARP4TestBox(TestBox):
    ipcon: IPConnection = None

    A: BrickletRS485 = None
    B: BrickletIndustrialDualRelay = None
    C: BrickletIO4V2 = None
    D: BrickletIndustrialDualRelay = None
    E: BrickletIndustrialQuadRelayV2 = None
    F: BrickletIndustrialQuadRelayV2 = None
    G: BrickletColorV2 = None
    H: BrickletNFC = None


    cp_relays: dict[CP, tuple[bool, bool, bool]] = field(default_factory=lambda: {
        'A': (False, False, False),
        'B': (True, False, False),
        'C': (True, True, False),
        'D': (True, False, True)
    })

    meter_relays: dict[Meter, tuple[bool, bool]] = field(default_factory=lambda: {
        'real': (False, False),
        'fake': (True, True),
        'none': (True, True),
    })

    ## API

    def set_cp(self, cp: CP):
        self.F.set_value([self.F.get_value()[0], *self.cp_relays[cp]])

    def get_cp(self) -> CP:
        r = self.F.get_value()[1:]
        for k, v in self.cp_relays.items():
            if r == v:
                return k
        raise Exception("unexpected state of CP relays. should not be reachable!")

    def connect_contactor_aux(self, contactor: int, aux: Aux):
        if contactor < 0 or contactor > 1:
            raise Exception(f"Unknown contactor {contactor}")

        if aux == 'real':
            self.D.set_selected_value(contactor, False)
        elif aux == 'open':
            self.E.set_selected_value(contactor + 2, False)
            self.D.set_selected_value(contactor, True)
        elif aux == 'closed':
            self.E.set_selected_value(contactor + 2, True)
            self.D.set_selected_value(contactor, True)

    def get_contactor_aux_state(self, contactor: int) -> Aux:
        if contactor < 0 or contactor > 1:
            raise Exception(f"Unknown contactor {contactor}")

        if not self.D.get_value()[contactor]:
            return 'real'
        elif self.E.get_value()[contactor + 2]:
            return 'closed'
        return 'open'

    def set_button_pressed(self, pressed: bool):
        self.E.set_selected_value(0, not pressed)

    def is_button_pressed(self):
        return not self.E.get_value()[0]

    def set_dc_faulted(self, fault: bool, clear_after_s: int = 0):
        if clear_after_s != 0:
            self.E.set_monoflop(1, fault, int(clear_after_s * 1000))
        else:
            self.E.set_selected_value(1, fault)

    def is_dc_fauled(self):
        return self.E.get_value()[1]

    def connect_meter(self, m: Meter):
        self.B.set_value(*self.meter_relays[m])

    def get_meter_connected(self):
        r = self.B.get_value()
        for k, v in self.meter_relays.items():
            if r == v:
                return k

    def get_gpio_index(self, gpio: TestBox.GPIO):
        d = {
            TestBox.GPIO.DC_X30: 0,
            TestBox.GPIO.DC_X6: 1,
            TestBox.GPIO.DC_FAULT: 2,
            TestBox.GPIO.DC_PROTECTOR_TEST: 3,
            TestBox.GPIO.EVSE_STATUS_LED: 4,
            TestBox.GPIO.BUTTON: 5,
            TestBox.GPIO.LED_RED: 6,
            TestBox.GPIO.LED_BLUE: 7,
            TestBox.GPIO.LED_GREEN: 8,
            TestBox.GPIO.CP_PWM: 9,
            TestBox.GPIO.PHASE_SWITCH: 10,
            TestBox.GPIO.CONTACTOR: 11,
            TestBox.GPIO.CONTACTOR_CHECK_L2_L3: 12,
            TestBox.GPIO.CONTACTOR_CHECK_L1_N: 13,
            TestBox.GPIO.PE_CHECK: 14,
            TestBox.GPIO.CURRENT_CONFIGURATION_1: 15,
            TestBox.GPIO.CP_DISCONNECT: 16,
            TestBox.GPIO.CURRENT_CONFIGURATION_0: 17,
            TestBox.GPIO.SHUTDOWN_INPUT: 18,
            TestBox.GPIO.VERSION_DETECT: 19,
            TestBox.GPIO.UNUSED_0: 20,
            TestBox.GPIO.UNUSED_1: 21,
            TestBox.GPIO.UNUSED_2: 22,
            TestBox.GPIO.UNUSED_3: 23,
        }

        if gpio not in d:
            self.tc.skip(f"EVSE does not support {gpio=}")

        return d[gpio]

    def reset(self):
        self.set_cp('A')
        self.connect_contactor_aux(0, 'real')
        self.connect_contactor_aux(1, 'real')
        self.set_button_pressed(False)
        self.set_dc_faulted(False)
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
        return [self.A, self.B, self.C, self.D, self.E, self.F, self.G, self.H]

    def _bricklet_names(self):
        return [BrickletRS485.DEVICE_DISPLAY_NAME,
                BrickletIndustrialDualRelay.DEVICE_DISPLAY_NAME,
                BrickletIO4V2.DEVICE_DISPLAY_NAME,
                BrickletIndustrialDualRelay.DEVICE_DISPLAY_NAME,
                BrickletIndustrialQuadRelayV2.DEVICE_DISPLAY_NAME,
                BrickletIndustrialQuadRelayV2.DEVICE_DISPLAY_NAME,
                BrickletColorV2.DEVICE_DISPLAY_NAME,
                BrickletNFC.DEVICE_DISPLAY_NAME]

    def _cb_enumerate(self, uid, connected_uid, position, hardware_version, firmware_version,
                    device_identifier, enumeration_type):
        if enumeration_type == IPConnection.ENUMERATION_TYPE_DISCONNECTED:
            return

        if   position == 'a' and device_identifier == BrickletRS485.DEVICE_IDENTIFIER:
            self.A = BrickletRS485(uid, self.ipcon)
        elif position == 'b' and device_identifier == BrickletIndustrialDualRelay.DEVICE_IDENTIFIER:
            self.B = BrickletIndustrialDualRelay(uid, self.ipcon)
        elif position == 'c' and device_identifier == BrickletIO4V2.DEVICE_IDENTIFIER:
            self.C = BrickletIO4V2(uid, self.ipcon)
        elif position == 'd' and device_identifier == BrickletIndustrialDualRelay.DEVICE_IDENTIFIER:
            self.D = BrickletIndustrialDualRelay(uid, self.ipcon)
        elif position == 'e' and device_identifier == BrickletIndustrialQuadRelayV2.DEVICE_IDENTIFIER:
            self.E = BrickletIndustrialQuadRelayV2(uid, self.ipcon)
        elif position == 'f' and device_identifier == BrickletIndustrialQuadRelayV2.DEVICE_IDENTIFIER:
            self.F = BrickletIndustrialQuadRelayV2(uid, self.ipcon)
        elif position == 'g' and device_identifier == BrickletColorV2.DEVICE_IDENTIFIER:
            self.G = BrickletColorV2(uid, self.ipcon)
        elif position == 'h' and device_identifier == BrickletNFC.DEVICE_IDENTIFIER:
            self.H = BrickletNFC(uid, self.ipcon)

    def _cb_connected(self, reason):
        self.ipcon.enumerate()

    def _get_led_state(self):
        return 'unknown'
