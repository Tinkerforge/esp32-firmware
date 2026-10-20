from typing import Protocol, Literal, TYPE_CHECKING, Any
import inspect
from enum import IntEnum, auto

CP = Literal['A', 'B', 'C', 'D']
Meter = Literal['none', 'real', 'fake']
Aux = Literal['real', 'open', 'closed']
Input = Literal['open', 'closed', 'contactor']

# This is not the usual "software package is created at runtime"-hack
# but a work-around for a circular import.
if TYPE_CHECKING:
    from ..test_context import TestContext
    tc_type = TestContext
else:
    tc_type = Any


class TestBox(Protocol):
    tc: tc_type = None

    def __not_implemented(self):
        self.tc.skip(f"No suitable testbox connected to call {inspect.stack()[1][3]}")

    def set_cp(self, cp: CP) -> None: self.__not_implemented()
    def get_cp(self) -> CP: self.__not_implemented()

    def set_diode_faulted(self, fault: bool, clear_after_s: int = 0) -> None: self.__not_implemented()
    def is_diode_fauled(self) -> bool: self.__not_implemented()

    def connect_contactor_aux(self, contactor: int, aux: Aux) -> None: self.__not_implemented()
    def get_contactor_aux_state(self, contactor: int) -> Aux: self.__not_implemented()

    def set_button_pressed(self, pressed: bool) -> None: self.__not_implemented()
    def is_button_pressed(self) -> bool: self.__not_implemented()

    def set_dc_faulted(self, fault: bool, clear_after_s: int = 0) -> None: self.__not_implemented()
    def is_dc_fauled(self) -> bool: self.__not_implemented()

    def set_gpio_shutdown_closed(self, val: bool) -> None: self.__not_implemented()
    def is_gpio_shutdown_closed(self) -> bool: self.__not_implemented()

    def set_gpio_input_closed(self, val: bool) -> None: self.__not_implemented()
    def is_gpio_input_closed(self) -> bool: self.__not_implemented()

    def is_gpio_output_closed(self) -> bool: self.__not_implemented()

    def connect_meter(self, m: Meter) -> None: self.__not_implemented()
    def get_meter_connected(self) -> str: self.__not_implemented()

    def is_contactor_closed(self, contactor: int = 0) -> bool: self.__not_implemented()

    def set_input_3(self, i: Input) -> None: self.__not_implemented()
    def get_input_3(self) -> Input: self.__not_implemented()

    def set_input_4(self, i: Input) -> None: self.__not_implemented()
    def get_input_4(self) -> Input: self.__not_implemented()

    def is_relay_closed(self) -> bool: self.__not_implemented()

    class GPIO(IntEnum):
        # common
        CURRENT_CONFIGURATION_0 = auto()
        MOTOR_FAULT = auto()
        DC_FAULT = auto()
        CURRENT_CONFIGURATION_1 = auto()
        DC_PROTECTOR_TEST = auto()
        SHUTDOWN_INPUT = auto()
        BUTTON = auto() # warp1 'input'
        CP_PWM = auto()
        MOTOR_INPUT_SWITCH = auto()
        CONTACTOR = auto() # warp1 relay
        GP_OUTPUT = auto()
        CP_DISCONNECT = auto()
        MOTOR_ENABLE = auto()
        MOTOR_PHASE = auto()
        CONTACTOR_CHECK_PRE = auto()
        CONTACTOR_CHECK_POST = auto()
        GP_INPUT = auto()
        DC_X6 = auto()
        DC_X30 = auto()
        EVSE_STATUS_LED = auto()

        UNUSED_0 = auto()
        UNUSED_1 = auto()
        UNUSED_2 = auto()
        UNUSED_3 = auto()

        # WARP1 only
        WARP1_OUTPUT = auto() # unused

        # WARP3+ only
        LED_RED = auto()
        LED_BLUE = auto()
        LED_GREEN = auto()
        PHASE_SWITCH = auto()
        CONTACTOR_CHECK_L2_L3 = auto()
        CONTACTOR_CHECK_L1_N = auto()
        PE_CHECK = auto()
        VERSION_DETECT = auto()

    def get_gpio_index(self, gpio: GPIO) -> int: self.__not_implemented()

    def reset(self): ...
    def start(self, host, port): ...
    def stop(self): ...
