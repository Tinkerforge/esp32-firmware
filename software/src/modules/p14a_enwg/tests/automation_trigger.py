#!/usr/bin/env -S uv run --group tests --script

# Tests for the §14a EnWG automation trigger.

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext


_TRIGGER_P14A_ENWG = 26
_ACTION_PRINT = 1

_RULE_TRIGGERED = 0
_RULE_NOT_TRIGGERED = 1

_original_p14a_config: dict | None = None
_original_automation_config: dict | None = None


def _make_api_config(enable: bool = True) -> dict:
    return {
        "enable": enable,
        "source": [2, None],
        "limit_charger": False,
        "limit_charge_manager": False,
        "limit_heating": False,
        "heating_max_power": 0,
    }


def _set_control(tc: TestContext, active: bool, limit_w: int = 4200) -> None:
    tc.api("p14a_enwg/control_update", {"active": active, "limit_w": limit_w})


def _wait_p14a_active(tc: TestContext, active: bool) -> None:
    tc.wait_for(lambda: tc.assert_eq(active, tc.api("p14a_enwg/state")["active"]))


def _set_rules(tc: TestContext) -> None:
    tc.api("automation/config_update", {"tasks": [
        {
            "trigger": [_TRIGGER_P14A_ENWG, {"active": True}],
            "action": [_ACTION_PRINT, {"message": "p14a test: triggered"}],
            "delay": 0,
        },
        {
            "trigger": [_TRIGGER_P14A_ENWG, {"active": False}],
            "action": [_ACTION_PRINT, {"message": "p14a test: not triggered"}],
            "delay": 0,
        },
    ]})

    # apply_config resets all last_run timestamps to 0.
    tc.wait_for(lambda: tc.assert_eq([0, 0], _last_run(tc)))


def _last_run(tc: TestContext) -> list[int]:
    return tc.api("automation/state")["last_run"]


def _wait_fired(tc: TestContext, rule: int, since: int) -> int:
    def _check() -> int:
        last_run = _last_run(tc)[rule]
        tc.assert_gt(since, last_run)
        return last_run

    return tc.wait_for(_check)


def suite_setup(tc: TestContext) -> None:
    global _original_p14a_config, _original_automation_config
    _original_p14a_config = tc.api("p14a_enwg/config")
    _original_automation_config = tc.api("automation/config")


def test_trigger_registered(tc: TestContext) -> None:
    tc.assert_true(_TRIGGER_P14A_ENWG in tc.api("automation/state")["registered_triggers"])


def test_trigger_enabled_follows_module(tc: TestContext) -> None:
    tc.api("p14a_enwg/config_update", _make_api_config(enable=False))
    tc.wait_for(lambda: tc.assert_false(_TRIGGER_P14A_ENWG in tc.api("automation/state")["enabled_triggers"]))

    tc.api("p14a_enwg/config_update", _make_api_config(enable=True))
    tc.wait_for(lambda: tc.assert_true(_TRIGGER_P14A_ENWG in tc.api("automation/state")["enabled_triggers"]))


def test_initial_state_fires_on_enable(tc: TestContext) -> None:
    tc.api("p14a_enwg/config_update", _make_api_config(enable=False))
    _set_control(tc, False)
    _wait_p14a_active(tc, False)

    _set_rules(tc)

    tc.api("p14a_enwg/config_update", _make_api_config(enable=True))

    _wait_fired(tc, _RULE_NOT_TRIGGERED, 0)
    tc.assert_eq(0, _last_run(tc)[_RULE_TRIGGERED])


def test_fires_on_activate_and_deactivate(tc: TestContext) -> None:
    tc.api("p14a_enwg/config_update", _make_api_config(enable=True))
    _set_control(tc, False)
    _wait_p14a_active(tc, False)

    _set_rules(tc)

    _set_control(tc, True)
    _wait_p14a_active(tc, True)
    triggered_ts = _wait_fired(tc, _RULE_TRIGGERED, 0)
    tc.assert_eq(0, _last_run(tc)[_RULE_NOT_TRIGGERED])

    _set_control(tc, False)
    _wait_p14a_active(tc, False)
    _wait_fired(tc, _RULE_NOT_TRIGGERED, 0)
    tc.assert_eq(triggered_ts, _last_run(tc)[_RULE_TRIGGERED])


def test_no_refire_on_limit_change(tc: TestContext) -> None:
    tc.api("p14a_enwg/config_update", _make_api_config(enable=True))
    _set_control(tc, False)
    _wait_p14a_active(tc, False)

    _set_rules(tc)

    _set_control(tc, True, 4200)
    triggered_ts = _wait_fired(tc, _RULE_TRIGGERED, 0)

    # Changing only the limit while active must not fire again.
    _set_control(tc, True, 6000)
    tc.wait_for(lambda: tc.assert_eq(6000, tc.api("p14a_enwg/state")["limit_w"]))
    tc.assert_eq([triggered_ts, 0], _last_run(tc))


def test_disable_while_active_fires_not_triggered(tc: TestContext) -> None:
    tc.api("p14a_enwg/config_update", _make_api_config(enable=True))
    _set_control(tc, True)
    _wait_p14a_active(tc, True)

    _set_rules(tc)

    tc.api("p14a_enwg/config_update", _make_api_config(enable=False))
    _wait_p14a_active(tc, False)
    _wait_fired(tc, _RULE_NOT_TRIGGERED, 0)
    tc.assert_eq(0, _last_run(tc)[_RULE_TRIGGERED])


def test_disabled_saving_config_does_not_fire(tc: TestContext) -> None:
    tc.api("p14a_enwg/config_update", _make_api_config(enable=False))
    _set_control(tc, False)
    _wait_p14a_active(tc, False)

    _set_rules(tc)

    # Saving the config while disabled must not fire anything.
    tc.api("p14a_enwg/config_update", _make_api_config(enable=False))
    _set_control(tc, True)
    tc.wait_for(lambda: tc.assert_eq(True, tc.api("p14a_enwg/control")["active"]))
    tc.assert_eq([0, 0], _last_run(tc))


def suite_teardown(tc: TestContext) -> None:
    _set_control(tc, False, 0)
    if _original_automation_config is not None:
        tc.api("automation/config_update", _original_automation_config)
    if _original_p14a_config is not None:
        tc.api("p14a_enwg/config_update", _original_p14a_config)


if __name__ == "__main__":
    run_testsuite(locals())
