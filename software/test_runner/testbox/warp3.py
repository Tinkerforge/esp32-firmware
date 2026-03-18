from time import sleep
from dataclasses import dataclass, field

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_industrial_digital_in_4_v2 import BrickletIndustrialDigitalIn4V2
from tinkerforge.bricklet_industrial_quad_relay_v2 import BrickletIndustrialQuadRelayV2
from tinkerforge.bricklet_industrial_dual_relay import BrickletIndustrialDualRelay
from tinkerforge.bricklet_rs485 import BrickletRS485
from tinkerforge.bricklet_color_v2 import BrickletColorV2

from .testbox import TestBox, CP, Meter, Aux

"""
RPi Port    Bricklet            Pin     Function
A           Ind Quad Relay V2   0       Diode Fault
                                1       CP B (2700 Ω)
                                2       CP C (880 Ω) (always switch with B)
                                3       CP D (240 Ω) (always switch with B)

B           Ind Quad Relay V2   0       Contactor 0 Aux Contact (if F -> 0 is NO)
                                1       Contactor 1 Aux Contact (if F -> 1 is NO)
                                2       Front button (relay closed = button not pressed, relay open = button pressed)
                                3       DC Fault

C           Ind Quad Relay V2   0       GPIO Shutdown Input

D           Ind Dual Relay      0       RS485 to energy meter (NC), to Bricklet (NO)
                                1       RS485 to energy meter (NC), to Bricklet (NO)

E           Ind Digital In 4 V2 0       Contactor 0 (NC -> open?)
                                1       Contactor 1 (NC -> open?)

F           Ind Dual Relay      0       Contactor 0 Aux Contact connected (NC), to B -> 0 (N0)
                                1       Contactor 1 Aux Contact connected (NC), to B -> 0 (N0)

G           RS485 Bricklet              Use to fake energy meters

H           Color Bricklet              Front LED color
"""

@dataclass
class WARP3TestBox(TestBox):
    ipcon: IPConnection = None

    A: BrickletIndustrialQuadRelayV2 = None
    B: BrickletIndustrialQuadRelayV2 = None
    C: BrickletIndustrialQuadRelayV2 = None
    D: BrickletIndustrialDualRelay = None
    E: BrickletIndustrialDigitalIn4V2 = None
    F: BrickletIndustrialDualRelay = None
    G: BrickletRS485 = None
    H: BrickletColorV2 = None

    cp_relays: dict[CP, tuple[bool, bool, bool]] = field(default_factory=lambda: {
        'A': (False, False, False),
        'B': (True, False, False),
        'C': (True, True, False),
        'D': (True, False, True)
    })

    meter_relays: dict[Meter, tuple[bool, bool]] = field(default_factory=lambda: {
        'real': (False, False),
        'fake': (True, True),
    })

    duty_cycle_ringbuf = []
    value_ringbuf = []

    ## API
    def set_diode_faulted(self, fault: bool, clear_after_s: int = 0):
        if clear_after_s != 0:
            self.A.set_monoflop(0, fault, int(clear_after_s * 1000))
        else:
            self.A.set_selected_value(0, fault)

    def is_diode_fauled(self):
        return self.A.get_value()[0]


    def set_cp(self, cp: CP):
        self.A.set_value([self.A.get_value()[0], *self.cp_relays[cp]])

    def get_cp(self) -> CP:
        r = self.A.get_value()[1:]
        for k, v in self.cp_relays.items():
            if r == v:
                return k
        raise Exception("unexpected state of CP relays. should not be reachable!")

    def connect_contactor_aux(self, contactor: int, aux: Aux):
        if contactor < 0 or contactor > 1:
            raise Exception(f"Unknown contactor {contactor}")

        if aux == 'real':
            self.F.set_selected_value(contactor, False)
        elif aux == 'open':
            self.B.set_selected_value(contactor, False)
            self.F.set_selected_value(contactor, True)
        elif aux == 'closed':
            self.B.set_selected_value(contactor, True)
            self.F.set_selected_value(contactor, True)

    def get_contactor_aux_state(self, contactor: int) -> Aux:
        if contactor < 0 or contactor > 1:
            raise Exception(f"Unknown contactor {contactor}")

        if not self.F.get_value()[contactor]:
            return 'real'
        elif self.B.get_value()[contactor]:
            return 'closed'
        return 'open'

    def set_button_pressed(self, pressed: bool):
        self.B.set_selected_value(2, not pressed)

    def is_button_pressed(self):
        return not self.B.get_value()[2]

    def set_dc_faulted(self, fault: bool, clear_after_s: int = 0):
        if clear_after_s != 0:
            self.B.set_monoflop(3, fault, int(clear_after_s * 1000))
        else:
            self.B.set_selected_value(3, fault)

    def is_dc_fauled(self):
        return self.B.get_value()[3]

    def set_gpio_shutdown_closed(self, val: bool):
        self.C.set_selected_value(0, val)

    def is_gpio_shutdown_closed(self):
        return self.C.get_value()[0]

    def connect_meter(self, m: Meter):
        self.D.set_value(*self.meter_relays[m])

    def get_meter_connected(self):
        r = self.D.get_value()
        for k, v in self.meter_relays.items():
            if r == v:
                return k

    def is_contactor_closed(self, contactor: int):
        if contactor < 0 or contactor > 1:
            raise Exception(f"Unknown contactor {contactor}")

        return not self.E.get_value()[contactor]

    def reset(self):
        # TODO why is this not called
        self.set_cp('A')
        self.connect_meter('real')
        self.set_gpio_shutdown_closed(False)
        self.set_button_pressed(False)
        self.set_dc_faulted(False)
        self.set_diode_faulted(False)
        self.connect_contactor_aux(0, 'real')
        self.connect_contactor_aux(1, 'real')

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
                print("Missing device at port {}!".format(chr(ord('A') + i)))

        for x in self._bricklets():
            x.set_response_expected_all(True)

        self.reset()

    def stop(self):
        self.ipcon.disconnect()

    ## Internals

    def _bricklets(self):
        return [self.A, self.B, self.C, self.D, self.E, self.F, self.G, self.H]

    def _bricklet_names(self):
        return [BrickletIndustrialQuadRelayV2.DEVICE_DISPLAY_NAME,
                BrickletIndustrialQuadRelayV2.DEVICE_DISPLAY_NAME,
                BrickletIndustrialQuadRelayV2.DEVICE_DISPLAY_NAME,
                BrickletIndustrialDualRelay.DEVICE_DISPLAY_NAME,
                BrickletIndustrialDigitalIn4V2.DEVICE_DISPLAY_NAME,
                BrickletIndustrialDualRelay.DEVICE_DISPLAY_NAME,
                BrickletRS485.DEVICE_DISPLAY_NAME,
                BrickletColorV2.DEVICE_DISPLAY_NAME]

    def _cb_enumerate(self, uid, connected_uid, position, hardware_version, firmware_version,
                    device_identifier, enumeration_type):
        if enumeration_type == IPConnection.ENUMERATION_TYPE_DISCONNECTED:
            return

        if   position == 'a' and device_identifier == BrickletIndustrialQuadRelayV2.DEVICE_IDENTIFIER:
            self.A = BrickletIndustrialQuadRelayV2(uid, self.ipcon)
        elif position == 'b' and device_identifier == BrickletIndustrialQuadRelayV2.DEVICE_IDENTIFIER:
            self.B = BrickletIndustrialQuadRelayV2(uid, self.ipcon)
        elif position == 'c' and device_identifier == BrickletIndustrialQuadRelayV2.DEVICE_IDENTIFIER:
            self.C = BrickletIndustrialQuadRelayV2(uid, self.ipcon)
        elif position == 'd' and device_identifier == BrickletIndustrialDualRelay.DEVICE_IDENTIFIER:
            self.D = BrickletIndustrialDualRelay(uid, self.ipcon)
        elif position == 'e' and device_identifier == BrickletIndustrialDigitalIn4V2.DEVICE_IDENTIFIER:
            self.E = BrickletIndustrialDigitalIn4V2(uid, self.ipcon)
        elif position == 'f' and device_identifier == BrickletIndustrialDualRelay.DEVICE_IDENTIFIER:
            self.F = BrickletIndustrialDualRelay(uid, self.ipcon)
        elif position == 'g' and device_identifier == BrickletRS485.DEVICE_IDENTIFIER:
            self.G = BrickletRS485(uid, self.ipcon)
        elif position == 'h' and device_identifier == BrickletColorV2.DEVICE_IDENTIFIER:
            self.H = BrickletColorV2(uid, self.ipcon)

    def _cb_connected(self, reason):
        self.ipcon.enumerate()

    def _get_led_state(self):
        return 'unknown'
