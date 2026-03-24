#!/usr/bin/env -S uv run --group tests --script

# Tests for p14a_enwg module with API source:

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext


_original_config: dict | None = None
_SLOT_P14A_ENWG = 16


def _make_api_config(
    enable: bool = True,
    limit_charger: bool = False,
    limit_charge_manager: bool = False,
    limit_heating: bool = False,
    heating_max_power: int = 0,
) -> dict:
    return {
        "enable": enable,
        "source": [2, None],
        "limit_charger": limit_charger,
        "limit_charge_manager": limit_charge_manager,
        "limit_heating": limit_heating,
        "heating_max_power": heating_max_power,
    }


def _set_control(tc: TestContext, active: bool, limit_w: int) -> None:
    tc.api("p14a_enwg/control_update", {"active": active, "limit_w": limit_w})


def suite_setup(tc: TestContext) -> None:
    global _original_config
    _original_config = tc.api("p14a_enwg/config")


def test_switch_to_api_source(tc: TestContext) -> None:
    cfg = _make_api_config(enable=True)
    tc.api("p14a_enwg/config_update", cfg)

    readback = tc.api("p14a_enwg/config")
    tc.assert_true(readback["enable"])
    tc.assert_eq([2, None], readback["source"])


def test_disable_module(tc: TestContext) -> None:
    cfg = _make_api_config(enable=False)
    tc.api("p14a_enwg/config_update", cfg)

    readback = tc.api("p14a_enwg/config")
    tc.assert_false(readback["enable"])

    # When disabled, state should be inactive.
    def _check_inactive() -> None:
        state = tc.api("p14a_enwg/state")
        tc.assert_false(state["active"])
        tc.assert_eq(0, state["limit_w"])

    tc.wait_for(_check_inactive)


def test_re_enable_module(tc: TestContext) -> None:
    # Disable first.
    tc.api("p14a_enwg/config_update", _make_api_config(enable=False))
    _set_control(tc, False, 0)

    # Re-enable.
    tc.api("p14a_enwg/config_update", _make_api_config(enable=True))
    readback = tc.api("p14a_enwg/config")
    tc.assert_true(readback["enable"])


def test_activate_via_control(tc: TestContext) -> None:
    tc.api("p14a_enwg/config_update", _make_api_config(enable=True))
    _set_control(tc, True, 4200)

    def _check() -> None:
        state = tc.api("p14a_enwg/state")
        tc.assert_true(state["active"])
        tc.assert_eq(4200, state["limit_w"])

    tc.wait_for(_check)


def test_deactivate_via_control(tc: TestContext) -> None:
    tc.api("p14a_enwg/config_update", _make_api_config(enable=True))
    _set_control(tc, True, 4200)

    def _active() -> None:
        tc.assert_true(tc.api("p14a_enwg/state")["active"])

    tc.wait_for(_active)

    _set_control(tc, False, 0)

    def _inactive() -> None:
        state = tc.api("p14a_enwg/state")
        tc.assert_false(state["active"])
        tc.assert_eq(0, state["limit_w"])

    tc.wait_for(_inactive)


def test_change_limit_while_active(tc: TestContext) -> None:
    tc.api("p14a_enwg/config_update", _make_api_config(enable=True))
    _set_control(tc, True, 3000)

    def _check_3000() -> None:
        tc.assert_eq(3000, tc.api("p14a_enwg/state")["limit_w"])

    tc.wait_for(_check_3000)

    _set_control(tc, True, 5000)

    def _check_5000() -> None:
        tc.assert_eq(5000, tc.api("p14a_enwg/state")["limit_w"])

    tc.wait_for(_check_5000)


def test_state_limit_zero_when_inactive(tc: TestContext) -> None:
    """Even if control.limit_w > 0, state.limit_w should be 0 when not active."""
    tc.api("p14a_enwg/config_update", _make_api_config(enable=True))
    _set_control(tc, False, 9999)

    def _check() -> None:
        state = tc.api("p14a_enwg/state")
        tc.assert_false(state["active"])
        tc.assert_eq(0, state["limit_w"])

    tc.wait_for(_check)


def test_control_ignored_when_source_is_eebus(tc: TestContext) -> None:
    """When source is EEBus, control_update should not affect state."""
    cfg = _make_api_config(enable=True)
    cfg["source"] = [1, None]  # EEBus
    tc.api("p14a_enwg/config_update", cfg)

    # Activate via control, should be ignored.
    _set_control(tc, True, 4200)

    # State should remain inactive (EEBus has no external trigger here).
    def _check() -> None:
        state = tc.api("p14a_enwg/state")
        tc.assert_false(state["active"])
        tc.assert_eq(0, state["limit_w"])

    tc.wait_for(_check)

    tc.api("p14a_enwg/config_update", _make_api_config(enable=True))
    _set_control(tc, False, 0)


def test_slot_default_when_not_active(tc: TestContext) -> None:
    """When p14a is enabled but not active, slot 16 should be 32A."""
    if not tc.device_type().is_warp([2, 3, 4]):
        tc.skip("Not available on this device")

    tc.api("p14a_enwg/config_update", _make_api_config(enable=True, limit_charger=True))
    _set_control(tc, False, 0)

    def _check() -> None:
        slots = tc.api("evse/slots")
        tc.assert_eq(32000, slots[_SLOT_P14A_ENWG]["max_current"])

    tc.wait_for(_check)


def test_slot_limited_when_active(tc: TestContext) -> None:
    """When active with limit_charger, slot 16 should reflect computed current    """
    if not tc.device_type().is_warp([2, 3, 4]):
        tc.skip("Not available on this device")

    tc.api("p14a_enwg/config_update", _make_api_config(enable=True, limit_charger=True))
    _set_control(tc, True, 4200)

    def _check() -> None:
        state = tc.api("p14a_enwg/state")
        tc.assert_true(state["active"])

        slots = tc.api("evse/slots")
        current = slots[_SLOT_P14A_ENWG]["max_current"]
        tc.assert_ge(6000, current)
        tc.assert_le(32000, current)

    tc.wait_for(_check)


def test_slot_unrestricted_without_limit_charger(tc: TestContext) -> None:
    """When active but limit_charger=false, slot 16 should stay at 32A."""
    if not tc.device_type().is_warp([2, 3, 4]):
        tc.skip("Not available on this device")

    tc.api(
        "p14a_enwg/config_update", _make_api_config(enable=True, limit_charger=False)
    )
    _set_control(tc, True, 4200)

    def _check() -> None:
        state = tc.api("p14a_enwg/state")
        tc.assert_true(state["active"])

        slots = tc.api("evse/slots")
        tc.assert_eq(32000, slots[_SLOT_P14A_ENWG]["max_current"])

    tc.wait_for(_check)


def test_slot_recovers_on_deactivate(tc: TestContext) -> None:
    """After deactivating, slot 16 should return to 32A."""
    if not tc.device_type().is_warp([2, 3, 4]):
        tc.skip("Not available on this device")

    tc.api("p14a_enwg/config_update", _make_api_config(enable=True, limit_charger=True))
    _set_control(tc, True, 4200)

    # Wait for limit to be applied.
    def _limited() -> None:
        slots = tc.api("evse/slots")
        tc.assert_ne(32000, slots[_SLOT_P14A_ENWG]["max_current"])

    tc.wait_for(_limited)

    # Deactivate.
    _set_control(tc, False, 0)

    def _recovered() -> None:
        slots = tc.api("evse/slots")
        tc.assert_eq(32000, slots[_SLOT_P14A_ENWG]["max_current"])

    tc.wait_for(_recovered)


def test_disabled_ignores_control(tc: TestContext) -> None:
    tc.api("p14a_enwg/config_update", _make_api_config(enable=False))
    _set_control(tc, True, 4200)

    def _check() -> None:
        state = tc.api("p14a_enwg/state")
        tc.assert_false(state["active"])
        tc.assert_eq(0, state["limit_w"])

    tc.wait_for(_check)

    _set_control(tc, False, 0)


def suite_teardown(tc: TestContext) -> None:
    _set_control(tc, False, 0)
    if _original_config is not None:
        tc.api("p14a_enwg/config_update", _original_config)


if __name__ == "__main__":
    run_testsuite(locals())
