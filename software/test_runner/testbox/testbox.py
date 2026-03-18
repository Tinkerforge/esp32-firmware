from typing import Protocol, Literal, TYPE_CHECKING, Any

CP = Literal['A', 'B', 'C', 'D']
Meter = Literal['real', 'fake']
Aux = Literal['real', 'open', 'closed']

if TYPE_CHECKING:
    from ..test_context import TestContext
    tc_type = TestContext
else:
    tc_type = Any

class TestBox(Protocol):
    tc: tc_type = None

    def set_cp(self, cp: CP): self.tc.skip("No suitable testbox connected to call set_cp")
    def get_cp(self) -> CP: self.tc.skip("No suitable testbox connected to call get_cp")

    def set_diode_faulted(self, fault: bool, clear_after_s: int = 0) -> None: self.tc.skip("No suitable testbox connected to call set_diode_faulted")
    def is_diode_fauled(self) -> bool: self.tc.skip("No suitable testbox connected to call is_diode_fauled")

    def connect_contactor_aux(self, contactor: int, aux: Aux) -> None: self.tc.skip("No suitable testbox connected to call connect_contactor_aux")
    def get_contactor_aux_state(self, contactor: int) -> Aux: self.tc.skip("No suitable testbox connected to call get_contactor_aux_state")

    def set_button_pressed(self, pressed: bool) -> None: self.tc.skip("No suitable testbox connected to call set_button_pressed")
    def is_button_pressed(self) -> bool: self.tc.skip("No suitable testbox connected to call is_button_pressed")

    def set_dc_faulted(self, fault: bool, clear_after_s: int = 0) -> None: self.tc.skip("No suitable testbox connected to call set_dc_faulted")
    def is_dc_fauled(self) -> bool: self.tc.skip("No suitable testbox connected to call is_dc_fauled")

    def set_gpio_shutdown_closed(self, val: bool) -> None: self.tc.skip("No suitable testbox connected to call set_gpio_shutdown_closed")
    def is_gpio_shutdown_closed(self) -> None: self.tc.skip("No suitable testbox connected to call is_gpio_shutdown_closed")

    def set_gpio_input_closed(self, val: bool) -> None: self.tc.skip("No suitable testbox connected to call set_gpio_input_closed")
    def is_gpio_input_closed(self) -> None: self.tc.skip("No suitable testbox connected to call is_gpio_input_closed")

    def is_gpio_output_closed(self) -> bool: self.tc.skip("No suitable testbox connected to call is_gpio_output_closed")

    def connect_meter(self, m: Meter): self.tc.skip("No suitable testbox connected to call connect_meter")
    def get_meter_connected(self): self.tc.skip("No suitable testbox connected to call get_meter_connected")

    def is_contactor_closed(self, contactor: int): self.tc.skip("No suitable testbox connected to call is_contactor_closed")

    def reset(self): ...
    def start(self, host, port): ...
    def stop(self): ...
