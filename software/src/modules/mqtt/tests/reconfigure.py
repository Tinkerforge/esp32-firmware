#!/usr/bin/env -S uv run --group tests --script

# Tests for MQTT re-configuration without reboot
#
# For the tests an amqtt broker is started locally on the test machine.

import logging
import time

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext
from software.src.modules.mqtt.tests._common import *

_PREFIX = "test"

_original_mqtt_config: dict | None = None
_original_automation_config: dict | None = None
_broker: "TestMQTTBroker | None" = None
_mqtt_helper: "MqttHelper | None" = None
_broker_host: str = ""
_broker_port: int = 0


def _make_test_config(**overrides: object) -> dict:
    cfg: dict = {
        "enable_mqtt": True,
        "broker_host": _broker_host,
        "broker_port": _broker_port,
        "broker_username": "",
        "broker_password": "",
        "global_topic_prefix": _PREFIX,
        "client_name": "test",
        "interval": 1,
        "protocol": 0,
        "cert_id": -1,
        "client_cert_id": -1,
        "client_key_id": -1,
        "path": "",
        "read_only": False,
    }
    cfg.update(overrides)
    return cfg


def _get_helper() -> MqttHelper:
    assert _mqtt_helper is not None
    return _mqtt_helper


def suite_setup(tc: TestContext) -> None:
    global \
        _original_mqtt_config, \
        _original_automation_config, \
        _broker, \
        _mqtt_helper, \
        _broker_host, \
        _broker_port

    # Suppress noisy amqtt logging
    logging.getLogger("amqtt").setLevel(logging.WARNING)

    _original_mqtt_config = tc.api("mqtt/config")
    _original_automation_config = tc.api("automation/config")

    _broker_host = tc.get_local_ip()
    _broker_port = find_free_port()

    _broker = TestMQTTBroker(_broker_port)
    _broker.start()

    _mqtt_helper = MqttHelper("127.0.0.1", _broker_port)
    _mqtt_helper.connect()
    time.sleep(0.5)


def test_disable_mqtt(tc: TestContext) -> None:
    tc.api("mqtt/config_update", _make_test_config(enable_mqtt=False), timeout=3)
    wait_mqtt_state(tc, CONN_NOT_CONFIGURED)


def test_enable_mqtt(tc: TestContext) -> None:
    tc.api("mqtt/config_update", _make_test_config(), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)


def test_disable_and_reenable(tc: TestContext) -> None:
    tc.api("mqtt/config_update", _make_test_config(enable_mqtt=False), timeout=3)
    wait_mqtt_state(tc, CONN_NOT_CONFIGURED)

    tc.api("mqtt/config_update", _make_test_config(), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)


def test_state_publishes_after_enable(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_test_config())

    helper = _get_helper()
    helper.clear_messages()
    helper.subscribe(f"{_PREFIX}/#")
    time.sleep(1)

    msg = helper.wait_for_any_message(f"{_PREFIX}/", timeout=15)
    tc.assert_(msg is not None)


def test_change_topic_prefix(tc: TestContext) -> None:
    new_prefix = "test_new"
    ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    helper = _get_helper()
    helper.clear_messages()
    helper.subscribe(f"{_PREFIX}/#")
    helper.subscribe(f"{new_prefix}/#")

    tc.api("mqtt/config_update", _make_test_config(global_topic_prefix=new_prefix), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)

    msg = helper.wait_for_any_message(f"{new_prefix}/", timeout=15)
    tc.assert_(msg is not None)

    # Verify no new messages on old prefix after the switch.
    helper.clear_messages()
    time.sleep(3)
    old_msgs = [m for m in helper.get_messages() if m[0].startswith(f"{_PREFIX}/")]
    tc.assert_eq(0, len(old_msgs))

    # Restore
    tc.api("mqtt/config_update", _make_test_config(), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)


def test_wrong_broker_port(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_test_config())

    tc.api("mqtt/config_update", _make_test_config(broker_port=_broker_port + 100), timeout=3)

    def _check_disconnected() -> None:
        try:
            state = tc.api("mqtt/state")
        except (TimeoutError, OSError):
            raise AssertionError("Device not reachable yet")
        conn = state["connection_state"]
        if conn not in (CONN_NOT_CONNECTED, CONN_ERROR):
            raise AssertionError(f"Expected NotConnected (1) or Error (3), got {conn}")

    tc.wait_for(_check_disconnected, timeout=60)

    # Restore correct port
    tc.api("mqtt/config_update", _make_test_config(), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)


def test_change_client_name(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_test_config())

    tc.api("mqtt/config_update", _make_test_config(client_name="test-renamed"), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)

    # Restore
    tc.api("mqtt/config_update", _make_test_config(), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)


def test_change_interval(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_test_config(interval=1))

    helper = _get_helper()
    helper.clear_messages()
    helper.subscribe(f"{_PREFIX}/#")
    time.sleep(5)
    msgs_fast = len(helper.get_messages())

    # Change to interval=10
    tc.api("mqtt/config_update", _make_test_config(interval=10), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)

    # Wait for initial burst to complete, then measure
    time.sleep(5)
    helper.clear_messages()
    time.sleep(5)
    msgs_slow = len(helper.get_messages())

    tc.assert_gt(msgs_slow, msgs_fast)

    # Restore
    tc.api("mqtt/config_update", _make_test_config(), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)


def test_mqtt_trigger_print(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker = f"MQT09_{int(time.time())}"
    task = make_task(
        trigger=make_mqtt_trigger(
            "trig/close", payload="on", retain=True, use_prefix=False
        ),
        action=make_print_action(marker),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(2)

    _get_helper().publish("trig/close", "on")
    assert_in_event_log(tc, marker)


def test_mqtt_trigger_with_prefix(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker = f"MQT10_{int(time.time())}"
    task = make_task(
        trigger=make_mqtt_trigger(
            "trig/pfx", payload="go", retain=True, use_prefix=True
        ),
        action=make_print_action(marker),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(2)

    _get_helper().publish(f"{_PREFIX}/automation_trigger/trig/pfx", "go")
    assert_in_event_log(tc, marker)


def test_add_trigger_live(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    tc.api("automation/config_update", {"tasks": []}, timeout=3)
    time.sleep(1)

    marker = f"MQT11_{int(time.time())}"
    task = make_task(
        trigger=make_mqtt_trigger(
            "trig/live", payload="fire", retain=True, use_prefix=False
        ),
        action=make_print_action(marker),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(2)

    _get_helper().publish("trig/live", "fire")
    assert_in_event_log(tc, marker)


def test_remove_trigger_live(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker_a = f"MQT12a_{int(time.time())}"
    task = make_task(
        trigger=make_mqtt_trigger(
            "trig/remove", payload="bang", retain=True, use_prefix=False
        ),
        action=make_print_action(marker_a),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(2)

    # Verify trigger works
    _get_helper().publish("trig/remove", "bang")
    assert_in_event_log(tc, marker_a)

    # Remove all triggers
    tc.api("automation/config_update", {"tasks": []}, timeout=3)
    time.sleep(2)

    # Use a new marker so we can distinguish old from new
    marker_b = f"MQT12b_{int(time.time())}"

    _get_helper().publish("trig/remove", "bang")
    time.sleep(3)

    assert_not_in_event_log(tc, marker_b)


def test_change_trigger_topic(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker_a = f"MQT13a_{int(time.time())}"
    task = make_task(
        trigger=make_mqtt_trigger("trig/a", payload="x", retain=True, use_prefix=False),
        action=make_print_action(marker_a),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(2)

    helper = _get_helper()

    # Verify test/a works
    helper.publish("trig/a", "x")
    assert_in_event_log(tc, marker_a)

    # Change to test/b
    marker_b = f"MQT13b_{int(time.time())}"
    task = make_task(
        trigger=make_mqtt_trigger("trig/b", payload="x", retain=True, use_prefix=False),
        action=make_print_action(marker_b),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(2)

    # test/a should no longer fire (marker_b must not appear after publishing on test/a)
    helper.publish("trig/a", "x")
    time.sleep(3)
    assert_not_in_event_log(tc, marker_b)

    # test/b should fire
    helper.publish("trig/b", "x")
    assert_in_event_log(tc, marker_b)


def test_mqtt_action_publishes(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker = f"test_action_{int(time.time())}"
    action_topic = "act/out"

    task = make_task(
        trigger=make_http_trigger("test_mqtt_action"),
        action=make_mqtt_action(action_topic, marker),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(2)

    helper = _get_helper()
    helper.clear_messages()
    helper.subscribe(action_topic)
    time.sleep(1)

    send_trigger(tc, "test_mqtt_action")

    msg = helper.wait_for_message(action_topic, timeout=10)
    tc.assert_(msg is not None)
    assert msg is not None
    tc.assert_eq(marker, msg[1])


def test_mqtt_action_with_prefix(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker = f"pfx_action_{int(time.time())}"
    action_suffix = "act/pfx_out"
    full_topic = f"{_PREFIX}/automation_action/{action_suffix}"

    task = make_task(
        trigger=make_http_trigger("test_pfx_action"),
        action=make_mqtt_action(action_suffix, marker, use_prefix=True),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(2)

    helper = _get_helper()
    helper.clear_messages()
    helper.subscribe(full_topic)
    time.sleep(1)

    send_trigger(tc, "test_pfx_action")

    msg = helper.wait_for_message(full_topic, timeout=10)
    tc.assert_(msg is not None)
    assert msg is not None
    tc.assert_eq(marker, msg[1])


def test_prefix_change_rebuilds_triggers(tc: TestContext) -> None:
    old_prefix = _PREFIX
    new_prefix = "test_pfx2"

    ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=old_prefix))

    marker_old = f"MQT16o_{int(time.time())}"
    task = make_task(
        trigger=make_mqtt_trigger(
            "pfxtest", payload="go", retain=True, use_prefix=True
        ),
        action=make_print_action(marker_old),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(2)

    # Verify on old prefix
    helper = _get_helper()
    helper.publish(f"{old_prefix}/automation_trigger/pfxtest", "go")
    assert_in_event_log(tc, marker_old)

    # Change prefix and update the action marker
    marker_new = f"MQT16n_{int(time.time())}"
    task = make_task(
        trigger=make_mqtt_trigger(
            "pfxtest", payload="go", retain=True, use_prefix=True
        ),
        action=make_print_action(marker_new),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    tc.api("mqtt/config_update", _make_test_config(global_topic_prefix=new_prefix), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)
    time.sleep(2)

    # Old prefix should no longer fire
    helper.publish(f"{old_prefix}/automation_trigger/pfxtest", "go")
    time.sleep(3)
    assert_not_in_event_log(tc, marker_new)

    # New prefix should fire
    helper.publish(f"{new_prefix}/automation_trigger/pfxtest", "go")
    assert_in_event_log(tc, marker_new)

    # Restore
    tc.api("mqtt/config_update", _make_test_config(), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)


def test_disable_stops_triggers(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker_a = f"MQT17a_{int(time.time())}"
    task = make_task(
        trigger=make_mqtt_trigger(
            "trig/disable", payload="x", retain=True, use_prefix=False
        ),
        action=make_print_action(marker_a),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(2)

    # Verify trigger works
    helper = _get_helper()
    helper.publish("trig/disable", "x")
    assert_in_event_log(tc, marker_a)

    # Disable MQTT
    tc.api("mqtt/config_update", _make_test_config(enable_mqtt=False), timeout=3)
    wait_mqtt_state(tc, CONN_NOT_CONFIGURED)
    time.sleep(1)

    # Use a new marker to detect any new action firing
    marker_b = f"MQT17b_{int(time.time())}"
    task = make_task(
        trigger=make_mqtt_trigger(
            "trig/disable", payload="x", retain=True, use_prefix=False
        ),
        action=make_print_action(marker_b),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(1)

    helper.publish("trig/disable", "x")
    time.sleep(3)

    assert_not_in_event_log(tc, marker_b)

    # Re-enable for subsequent tests
    tc.api("mqtt/config_update", _make_test_config(), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)


def test_state_on_new_prefix(tc: TestContext) -> None:
    new_prefix = "test_state"

    ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    helper = _get_helper()
    helper.clear_messages()
    helper.subscribe(f"{_PREFIX}/#")
    helper.subscribe(f"{new_prefix}/#")
    time.sleep(1)

    msg = helper.wait_for_any_message(f"{_PREFIX}/", timeout=15)
    tc.assert_(msg is not None)

    # Change prefix
    tc.api("mqtt/config_update", _make_test_config(global_topic_prefix=new_prefix), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)

    helper.clear_messages()
    msg = helper.wait_for_any_message(f"{new_prefix}/", timeout=15)
    tc.assert_(msg is not None)

    # Verify no new messages on old prefix
    time.sleep(3)
    old_msgs = [m for m in helper.get_messages() if m[0].startswith(f"{_PREFIX}/")]
    tc.assert_eq(0, len(old_msgs))

    # Restore
    tc.api("mqtt/config_update", _make_test_config(), timeout=3)
    wait_mqtt_state(tc, CONN_CONNECTED)


def suite_teardown(tc: TestContext) -> None:
    global _mqtt_helper, _broker

    if _mqtt_helper:
        _mqtt_helper.disconnect()
        _mqtt_helper = None

    # The device may be temporarily unresponsive (e.g. after a wrong-broker
    # test), so retry the restore calls a few times.
    for _ in range(5):
        try:
            if _original_automation_config is not None:
                tc.api("automation/config_update", {"tasks": _original_automation_config.get("tasks", [])}, timeout=3)
            break
        except (TimeoutError, OSError):
            time.sleep(2)

    for _ in range(5):
        try:
            # Does not restore the original password...
            if _original_mqtt_config is not None:
                restore = dict(_original_mqtt_config)
                tc.api("mqtt/config_update", restore, timeout=3)
            break
        except (TimeoutError, OSError):
            time.sleep(2)

    if _broker:
        _broker.stop()
        _broker = None


if __name__ == "__main__":
    run_testsuite(locals())
