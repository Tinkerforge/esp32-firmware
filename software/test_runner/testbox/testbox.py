from typing import Protocol, Literal, TYPE_CHECKING, Any
import inspect

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

    def is_contactor_closed(self, contactor: int) -> bool: self.__not_implemented()

    def set_input_3(self, i: Input) -> None: self.__not_implemented()
    def get_input_3(self) -> Input: self.__not_implemented()

    def set_input_4(self, i: Input) -> None: self.__not_implemented()
    def get_input_4(self) -> Input: self.__not_implemented()

    def is_relay_closed(self) -> bool: self.__not_implemented()

    def reset(self): ...
    def start(self, host, port): ...
    def stop(self): ...
