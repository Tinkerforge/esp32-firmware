#!/usr/bin/env -S uv run --group tests --script

# Tests for MQTT re-configuration without reboot
#
# For the tests an amqtt broker is started locally on the test machine.

from __future__ import annotations

import asyncio
import logging
import socket
import threading
import time

from amqtt.broker import Broker as AmqttBroker
from amqtt.client import MQTTClient
from amqtt.mqtt.constants import QOS_0

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext


_CONN_NOT_CONFIGURED = 0
_CONN_NOT_CONNECTED = 1
_CONN_CONNECTED = 2
_CONN_ERROR = 3

_TRIGGER_MQTT = 3
_TRIGGER_HTTP = 18
_ACTION_MQTT = 2
_ACTION_PRINT = 1
_HTTP_GET_POST_PUT = 4

_PREFIX = "test"


def _find_free_port(start: int = 1883) -> int:
    """Return *start* if it is available, otherwise increment until a free port is found."""
    for port in range(start, start + 100):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.bind(("0.0.0.0", port))
                return port
        except OSError:
            continue
    raise RuntimeError(f"No free TCP port found in range {start}..{start + 99}")


_original_mqtt_config: dict | None = None
_original_automation_config: dict | None = None
_broker: "_TestMQTTBroker | None" = None
_mqtt_helper: "_MqttHelper | None" = None
_broker_host: str = ""
_broker_port: int = 0


class _TestMQTTBroker:
    """Runs an amqtt broker in a background thread with its own event loop."""

    def __init__(self, port: int):
        self._port = port
        self._loop: asyncio.AbstractEventLoop | None = None
        self._thread: threading.Thread | None = None
        self._broker: AmqttBroker | None = None

    def start(self) -> None:
        self._loop = asyncio.new_event_loop()
        self._thread = threading.Thread(target=self._run_loop, daemon=True)
        self._thread.start()

        fut = asyncio.run_coroutine_threadsafe(self._start_broker(), self._loop)
        fut.result(timeout=10)

    async def _start_broker(self) -> None:
        config = {
            "listeners": {
                "default": {
                    "type": "tcp",
                    "bind": f"0.0.0.0:{self._port}",
                }
            },
            "timeout_disconnect_delay": 0,
            "plugins": {
                "amqtt.plugins.authentication.AnonymousAuthPlugin": {
                    "allow_anonymous": True,
                },
            },
        }
        self._broker = AmqttBroker(config)
        await self._broker.start()

    def stop(self) -> None:
        if self._broker and self._loop:
            fut = asyncio.run_coroutine_threadsafe(self._broker.shutdown(), self._loop)
            fut.result(timeout=10)
        if self._loop:
            self._loop.call_soon_threadsafe(self._loop.stop)
        if self._thread:
            self._thread.join(timeout=5)

    def _run_loop(self) -> None:
        assert self._loop is not None
        asyncio.set_event_loop(self._loop)
        self._loop.run_forever()


class _MqttHelper:
    def __init__(self, broker_port: int):
        self._broker_port = broker_port
        self._client: MQTTClient | None = None
        self._messages: list[tuple[str, str]] = []
        self._lock = threading.Lock()
        self._events: dict[str, threading.Event] = {}
        self._loop: asyncio.AbstractEventLoop | None = None
        self._thread: threading.Thread | None = None
        self._receiver_task: asyncio.Task | None = None  # type: ignore[type-arg]

    def connect(self, client_id: str = "mqtt_test_helper") -> None:
        self._loop = asyncio.new_event_loop()
        self._thread = threading.Thread(target=self._run_loop, daemon=True)
        self._thread.start()

        self._client = MQTTClient(client_id=client_id)
        uri = f"mqtt://127.0.0.1:{self._broker_port}/"
        fut = asyncio.run_coroutine_threadsafe(self._client.connect(uri), self._loop)
        fut.result(timeout=10)

        # Start background message receiver as a task on the event loop
        fut = asyncio.run_coroutine_threadsafe(self._start_receiver(), self._loop)
        fut.result(timeout=5)

    def disconnect(self) -> None:
        if self._receiver_task and self._loop:
            self._loop.call_soon_threadsafe(self._receiver_task.cancel)
            time.sleep(0.2)
        if self._client and self._loop:
            fut = asyncio.run_coroutine_threadsafe(
                self._client.disconnect(), self._loop
            )
            try:
                fut.result(timeout=5)
            except Exception:
                pass
            self._client = None
        if self._loop:
            self._loop.call_soon_threadsafe(self._loop.stop)
        if self._thread:
            self._thread.join(timeout=5)

    def subscribe(self, topic: str) -> None:
        assert self._client is not None and self._loop is not None
        fut = asyncio.run_coroutine_threadsafe(
            self._client.subscribe([(topic, QOS_0)]), self._loop
        )
        fut.result(timeout=10)

    def publish(self, topic: str, payload: str, *, retain: bool = False) -> None:
        assert self._client is not None and self._loop is not None
        fut = asyncio.run_coroutine_threadsafe(
            self._client.publish(
                topic, payload.encode("utf-8"), qos=QOS_0, retain=retain
            ),
            self._loop,
        )
        fut.result(timeout=10)

    def clear_messages(self) -> None:
        with self._lock:
            self._messages.clear()
            self._events.clear()

    def wait_for_message(self, topic_filter: str, timeout: float = 10) -> tuple[str, str] | None:
        evt = threading.Event()
        with self._lock:
            for msg in self._messages:
                if msg[0] == topic_filter:
                    return msg
            self._events[topic_filter] = evt

        if evt.wait(timeout=timeout):
            with self._lock:
                for msg in self._messages:
                    if msg[0] == topic_filter:
                        return msg
        return None

    def wait_for_any_message(self, topic_prefix: str, timeout: float = 10) -> tuple[str, str] | None:
        """Wait for any message whose topic starts with topic_prefix."""
        deadline = time.time() + timeout
        while time.time() < deadline:
            with self._lock:
                for msg in self._messages:
                    if msg[0].startswith(topic_prefix):
                        return msg
            time.sleep(0.2)
        return None

    def get_messages(self) -> list[tuple[str, str]]:
        with self._lock:
            return list(self._messages)

    async def _start_receiver(self) -> None:
        self._receiver_task = asyncio.create_task(self._receive_loop())

    async def _receive_loop(self) -> None:
        assert self._client is not None
        while True:
            try:
                msg = await self._client.deliver_message(timeout_duration=1.0)
                if msg is None:
                    continue
                pkt = msg.publish_packet
                assert (
                    pkt is not None
                    and pkt.variable_header is not None
                    and pkt.payload is not None
                )
                topic = pkt.variable_header.topic_name
                payload = pkt.payload.data.decode(  # type: ignore[union-attr]
                    "utf-8", errors="replace"
                )
                entry = (topic, payload)
                with self._lock:
                    self._messages.append(entry)
                    if topic in self._events:
                        self._events[topic].set()
            except asyncio.CancelledError:
                return
            except TimeoutError:
                continue
            except Exception:
                # Connection lost or other error; stop receiving
                return

    def _run_loop(self) -> None:
        assert self._loop is not None
        asyncio.set_event_loop(self._loop)
        self._loop.run_forever()


def _make_mqtt_trigger(topic_filter: str, payload: str = "", retain: bool = False, use_prefix: bool = False) -> list:
    return [
        _TRIGGER_MQTT,
        {
            "topic_filter": topic_filter,
            "payload": payload,
            "retain": retain,
            "use_prefix": use_prefix,
        },
    ]


def _make_http_trigger(url_suffix: str, method: int = _HTTP_GET_POST_PUT, payload: str = "") -> list:
    return [
        _TRIGGER_HTTP,
        {
            "method": method,
            "url_suffix": url_suffix,
            "payload": payload,
        },
    ]


def _make_print_action(message: str) -> list:
    return [
        _ACTION_PRINT,
        {
            "message": message,
        },
    ]


def _make_mqtt_action(topic: str, payload: str, retain: bool = False, use_prefix: bool = False) -> list:
    return [
        _ACTION_MQTT,
        {
            "topic": topic,
            "payload": payload,
            "retain": retain,
            "use_prefix": use_prefix,
        },
    ]


def _make_task(trigger: list, action: list, delay: int = 0) -> dict:
    return {
        "trigger": trigger,
        "action": action,
        "delay": delay,
    }


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


def _wait_mqtt_state(tc: TestContext, expected: int, *, timeout: float = 20) -> None:
    def _check() -> None:
        try:
            state = tc.api("mqtt/state", timeout=5)
        except (TimeoutError, OSError):
            raise AssertionError("Device not reachable yet")
        tc.assert_eq(expected, state["connection_state"])

    tc.wait_for(_check, timeout=timeout)


def _ensure_mqtt_connected(tc: TestContext, config: dict | None = None) -> None:
    if config:
        tc.api("mqtt/config_update", config, timeout=5)
    _wait_mqtt_state(tc, _CONN_CONNECTED)


def _assert_in_event_log(tc: TestContext, marker: str, *, timeout: float = 10) -> None:
    """Wait for marker to appear in the device event log."""

    def _check() -> None:
        log = tc.http_request("GET", "/event_log", timeout=5)
        if isinstance(log, bytes):
            log = log.decode("utf-8", errors="replace")
        tc.assert_(marker in log)

    tc.wait_for(_check, timeout=timeout)


def _assert_not_in_event_log(tc: TestContext, marker: str) -> None:
    """Assert that marker does NOT appear in the device event log."""

    log = tc.http_request("GET", "/event_log", timeout=5)
    if isinstance(log, bytes):
        log = log.decode("utf-8", errors="replace")
    tc.assert_false(marker in log)


def _get_helper() -> _MqttHelper:
    assert _mqtt_helper is not None
    return _mqtt_helper


def _send_trigger(tc: TestContext, suffix: str) -> None:
    tc.http_request("GET", f"/automation_trigger/{suffix}")


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
    _broker_port = _find_free_port()

    _broker = _TestMQTTBroker(_broker_port)
    _broker.start()

    _mqtt_helper = _MqttHelper(_broker_port)
    _mqtt_helper.connect()
    time.sleep(0.5)


def test_disable_mqtt(tc: TestContext) -> None:
    tc.api("mqtt/config_update", _make_test_config(enable_mqtt=False), timeout=2)
    _wait_mqtt_state(tc, _CONN_NOT_CONFIGURED)


def test_enable_mqtt(tc: TestContext) -> None:
    tc.api("mqtt/config_update", _make_test_config(), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)


def test_disable_and_reenable(tc: TestContext) -> None:
    tc.api("mqtt/config_update", _make_test_config(enable_mqtt=False), timeout=2)
    _wait_mqtt_state(tc, _CONN_NOT_CONFIGURED)

    tc.api("mqtt/config_update", _make_test_config(), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)


def test_state_publishes_after_enable(tc: TestContext) -> None:
    _ensure_mqtt_connected(tc, _make_test_config())

    helper = _get_helper()
    helper.clear_messages()
    helper.subscribe(f"{_PREFIX}/#")
    time.sleep(1)

    msg = helper.wait_for_any_message(f"{_PREFIX}/", timeout=15)
    tc.assert_(msg is not None)


def test_change_topic_prefix(tc: TestContext) -> None:
    new_prefix = "test_new"
    _ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    helper = _get_helper()
    helper.clear_messages()
    helper.subscribe(f"{_PREFIX}/#")
    helper.subscribe(f"{new_prefix}/#")

    tc.api("mqtt/config_update", _make_test_config(global_topic_prefix=new_prefix), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)

    msg = helper.wait_for_any_message(f"{new_prefix}/", timeout=15)
    tc.assert_(msg is not None)

    # Verify no new messages on old prefix after the switch.
    helper.clear_messages()
    time.sleep(3)
    old_msgs = [m for m in helper.get_messages() if m[0].startswith(f"{_PREFIX}/")]
    tc.assert_eq(0, len(old_msgs))

    # Restore
    tc.api("mqtt/config_update", _make_test_config(), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)


def test_wrong_broker_port(tc: TestContext) -> None:
    _ensure_mqtt_connected(tc, _make_test_config())

    tc.api("mqtt/config_update", _make_test_config(broker_port=_broker_port + 100), timeout=2)

    def _check_disconnected() -> None:
        try:
            state = tc.api("mqtt/state")
        except (TimeoutError, OSError):
            raise AssertionError("Device not reachable yet")
        conn = state["connection_state"]
        if conn not in (_CONN_NOT_CONNECTED, _CONN_ERROR):
            raise AssertionError(f"Expected NotConnected (1) or Error (3), got {conn}")

    tc.wait_for(_check_disconnected, timeout=60)

    # Restore correct port
    tc.api("mqtt/config_update", _make_test_config(), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)


def test_change_client_name(tc: TestContext) -> None:
    _ensure_mqtt_connected(tc, _make_test_config())

    tc.api("mqtt/config_update", _make_test_config(client_name="test-renamed"), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)

    # Restore
    tc.api("mqtt/config_update", _make_test_config(), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)


def test_change_interval(tc: TestContext) -> None:
    _ensure_mqtt_connected(tc, _make_test_config(interval=1))

    helper = _get_helper()
    helper.clear_messages()
    helper.subscribe(f"{_PREFIX}/#")
    time.sleep(5)
    msgs_fast = len(helper.get_messages())

    # Change to interval=10
    tc.api("mqtt/config_update", _make_test_config(interval=10), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)

    # Wait for initial burst to complete, then measure
    time.sleep(5)
    helper.clear_messages()
    time.sleep(5)
    msgs_slow = len(helper.get_messages())

    tc.assert_gt(msgs_slow, msgs_fast)

    # Restore
    tc.api("mqtt/config_update", _make_test_config(), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)


def test_mqtt_trigger_print(tc: TestContext) -> None:
    _ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker = f"MQT09_{int(time.time())}"
    task = _make_task(
        trigger=_make_mqtt_trigger(
            "trig/close", payload="on", retain=True, use_prefix=False
        ),
        action=_make_print_action(marker),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=2)
    time.sleep(2)

    _get_helper().publish("trig/close", "on")
    _assert_in_event_log(tc, marker)


def test_mqtt_trigger_with_prefix(tc: TestContext) -> None:
    _ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker = f"MQT10_{int(time.time())}"
    task = _make_task(
        trigger=_make_mqtt_trigger(
            "trig/pfx", payload="go", retain=True, use_prefix=True
        ),
        action=_make_print_action(marker),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=2)
    time.sleep(2)

    _get_helper().publish(f"{_PREFIX}/automation_trigger/trig/pfx", "go")
    _assert_in_event_log(tc, marker)


def test_add_trigger_live(tc: TestContext) -> None:
    _ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    tc.api("automation/config_update", {"tasks": []}, timeout=2)
    time.sleep(1)

    marker = f"MQT11_{int(time.time())}"
    task = _make_task(
        trigger=_make_mqtt_trigger(
            "trig/live", payload="fire", retain=True, use_prefix=False
        ),
        action=_make_print_action(marker),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=2)
    time.sleep(2)

    _get_helper().publish("trig/live", "fire")
    _assert_in_event_log(tc, marker)


def test_remove_trigger_live(tc: TestContext) -> None:
    _ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker_a = f"MQT12a_{int(time.time())}"
    task = _make_task(
        trigger=_make_mqtt_trigger(
            "trig/remove", payload="bang", retain=True, use_prefix=False
        ),
        action=_make_print_action(marker_a),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=2)
    time.sleep(2)

    # Verify trigger works
    _get_helper().publish("trig/remove", "bang")
    _assert_in_event_log(tc, marker_a)

    # Remove all triggers
    tc.api("automation/config_update", {"tasks": []}, timeout=2)
    time.sleep(2)

    # Use a new marker so we can distinguish old from new
    marker_b = f"MQT12b_{int(time.time())}"

    _get_helper().publish("trig/remove", "bang")
    time.sleep(3)

    _assert_not_in_event_log(tc, marker_b)


def test_change_trigger_topic(tc: TestContext) -> None:
    _ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker_a = f"MQT13a_{int(time.time())}"
    task = _make_task(
        trigger=_make_mqtt_trigger(
            "trig/a", payload="x", retain=True, use_prefix=False
        ),
        action=_make_print_action(marker_a),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=2)
    time.sleep(2)

    helper = _get_helper()

    # Verify test/a works
    helper.publish("trig/a", "x")
    _assert_in_event_log(tc, marker_a)

    # Change to test/b
    marker_b = f"MQT13b_{int(time.time())}"
    task = _make_task(
        trigger=_make_mqtt_trigger(
            "trig/b", payload="x", retain=True, use_prefix=False
        ),
        action=_make_print_action(marker_b),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=2)
    time.sleep(2)

    # test/a should no longer fire (marker_b must not appear after publishing on test/a)
    helper.publish("trig/a", "x")
    time.sleep(3)
    _assert_not_in_event_log(tc, marker_b)

    # test/b should fire
    helper.publish("trig/b", "x")
    _assert_in_event_log(tc, marker_b)


def test_mqtt_action_publishes(tc: TestContext) -> None:
    _ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker = f"test_action_{int(time.time())}"
    action_topic = "act/out"

    task = _make_task(
        trigger=_make_http_trigger("test_mqtt_action"),
        action=_make_mqtt_action(action_topic, marker),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=2)
    time.sleep(2)

    helper = _get_helper()
    helper.clear_messages()
    helper.subscribe(action_topic)
    time.sleep(1)

    _send_trigger(tc, "test_mqtt_action")

    msg = helper.wait_for_message(action_topic, timeout=10)
    tc.assert_(msg is not None)
    assert msg is not None
    tc.assert_eq(marker, msg[1])


def test_mqtt_action_with_prefix(tc: TestContext) -> None:
    _ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker = f"pfx_action_{int(time.time())}"
    action_suffix = "act/pfx_out"
    full_topic = f"{_PREFIX}/automation_action/{action_suffix}"

    task = _make_task(
        trigger=_make_http_trigger("test_pfx_action"),
        action=_make_mqtt_action(action_suffix, marker, use_prefix=True),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=2)
    time.sleep(2)

    helper = _get_helper()
    helper.clear_messages()
    helper.subscribe(full_topic)
    time.sleep(1)

    _send_trigger(tc, "test_pfx_action")

    msg = helper.wait_for_message(full_topic, timeout=10)
    tc.assert_(msg is not None)
    assert msg is not None
    tc.assert_eq(marker, msg[1])


def test_prefix_change_rebuilds_triggers(tc: TestContext) -> None:
    old_prefix = _PREFIX
    new_prefix = "test_pfx2"

    _ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=old_prefix))

    marker_old = f"MQT16o_{int(time.time())}"
    task = _make_task(
        trigger=_make_mqtt_trigger(
            "pfxtest", payload="go", retain=True, use_prefix=True
        ),
        action=_make_print_action(marker_old),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=2)
    time.sleep(2)

    # Verify on old prefix
    helper = _get_helper()
    helper.publish(f"{old_prefix}/automation_trigger/pfxtest", "go")
    _assert_in_event_log(tc, marker_old)

    # Change prefix and update the action marker
    marker_new = f"MQT16n_{int(time.time())}"
    task = _make_task(
        trigger=_make_mqtt_trigger(
            "pfxtest", payload="go", retain=True, use_prefix=True
        ),
        action=_make_print_action(marker_new),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=2)
    tc.api("mqtt/config_update", _make_test_config(global_topic_prefix=new_prefix), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)
    time.sleep(2)

    # Old prefix should no longer fire
    helper.publish(f"{old_prefix}/automation_trigger/pfxtest", "go")
    time.sleep(3)
    _assert_not_in_event_log(tc, marker_new)

    # New prefix should fire
    helper.publish(f"{new_prefix}/automation_trigger/pfxtest", "go")
    _assert_in_event_log(tc, marker_new)

    # Restore
    tc.api("mqtt/config_update", _make_test_config(), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)


def test_disable_stops_triggers(tc: TestContext) -> None:
    _ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    marker_a = f"MQT17a_{int(time.time())}"
    task = _make_task(
        trigger=_make_mqtt_trigger(
            "trig/disable", payload="x", retain=True, use_prefix=False
        ),
        action=_make_print_action(marker_a),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=2)
    time.sleep(2)

    # Verify trigger works
    helper = _get_helper()
    helper.publish("trig/disable", "x")
    _assert_in_event_log(tc, marker_a)

    # Disable MQTT
    tc.api("mqtt/config_update", _make_test_config(enable_mqtt=False), timeout=2)
    _wait_mqtt_state(tc, _CONN_NOT_CONFIGURED)
    time.sleep(1)

    # Use a new marker to detect any new action firing
    marker_b = f"MQT17b_{int(time.time())}"
    task = _make_task(
        trigger=_make_mqtt_trigger(
            "trig/disable", payload="x", retain=True, use_prefix=False
        ),
        action=_make_print_action(marker_b),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=2)
    time.sleep(1)

    helper.publish("trig/disable", "x")
    time.sleep(3)

    _assert_not_in_event_log(tc, marker_b)

    # Re-enable for subsequent tests
    tc.api("mqtt/config_update", _make_test_config(), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)


def test_state_on_new_prefix(tc: TestContext) -> None:
    new_prefix = "test_state"

    _ensure_mqtt_connected(tc, _make_test_config(global_topic_prefix=_PREFIX))

    helper = _get_helper()
    helper.clear_messages()
    helper.subscribe(f"{_PREFIX}/#")
    helper.subscribe(f"{new_prefix}/#")
    time.sleep(1)

    msg = helper.wait_for_any_message(f"{_PREFIX}/", timeout=15)
    tc.assert_(msg is not None)

    # Change prefix
    tc.api("mqtt/config_update", _make_test_config(global_topic_prefix=new_prefix), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)

    helper.clear_messages()
    msg = helper.wait_for_any_message(f"{new_prefix}/", timeout=15)
    tc.assert_(msg is not None)

    # Verify no new messages on old prefix
    time.sleep(3)
    old_msgs = [m for m in helper.get_messages() if m[0].startswith(f"{_PREFIX}/")]
    tc.assert_eq(0, len(old_msgs))

    # Restore
    tc.api("mqtt/config_update", _make_test_config(), timeout=2)
    _wait_mqtt_state(tc, _CONN_CONNECTED)


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
                tc.api("automation/config_update", {"tasks": _original_automation_config.get("tasks", [])}, timeout=2)
            break
        except (TimeoutError, OSError):
            time.sleep(2)

    for _ in range(5):
        try:
            # Does not restore the original password...
            if _original_mqtt_config is not None:
                restore = dict(_original_mqtt_config)
                tc.api("mqtt/config_update", restore, timeout=2)
            break
        except (TimeoutError, OSError):
            time.sleep(2)

    if _broker:
        _broker.stop()
        _broker = None


if __name__ == "__main__":
    run_testsuite(locals())
