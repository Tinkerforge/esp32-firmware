"""Shared helpers for MQTT test suites.

Provides the test broker, MQTT client wrapper, automation builders,
and assertion helpers used by both the reconfigure and TLS test suites.
"""

import asyncio
import socket
import threading
import time

from amqtt.broker import Broker as AmqttBroker
from amqtt.client import MQTTClient
from amqtt.mqtt.constants import QOS_0

from software.test_runner.test_context import TestContext


# Connection-state constants (mirror mqtt/state -> connection_state)
CONN_NOT_CONFIGURED = 0
CONN_NOT_CONNECTED = 1
CONN_CONNECTED = 2
CONN_ERROR = 3

# Automation type IDs
TRIGGER_MQTT = 3
TRIGGER_HTTP = 18
ACTION_MQTT = 2
ACTION_PRINT = 1
HTTP_GET_POST_PUT = 4


def find_free_port(start: int = 1883) -> int:
    """Return *start* if it is available, otherwise increment until a free port is found."""
    for port in range(start, start + 100):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.bind(("0.0.0.0", port))
                return port
        except OSError:
            continue
    raise RuntimeError(f"No free TCP port found in range {start}..{start + 99}")


class TestMQTTBroker: # uses tls when certfile/keyfile are provided
    """Runs an amqtt broker in a background thread with its own event loop."""

    def __init__(self, port: int, certfile: str | None = None, keyfile: str | None = None):
        self._port = port
        self._certfile = certfile
        self._keyfile = keyfile
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
        listener: dict = {
            "type": "tcp",
            "bind": f"0.0.0.0:{self._port}",
        }
        if self._certfile is not None:
            listener["ssl"] = True
            listener["certfile"] = self._certfile
            listener["keyfile"] = self._keyfile
        config = {
            "listeners": {"default": listener},
            "timeout_disconnect_delay": 0,
            "plugins": {
                "amqtt.plugins.authentication.AnonymousAuthPlugin": {
                    "allow_anonymous": True
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


class MqttHelper:
    def __init__(self, broker_host: str, broker_port: int):
        self._broker_host = broker_host
        self._broker_port = broker_port
        self._client: MQTTClient | None = None
        self._messages: list[tuple[str, str]] = []
        self._lock = threading.Lock()
        self._events: dict[str, threading.Event] = {}
        self._loop: asyncio.AbstractEventLoop | None = None
        self._thread: threading.Thread | None = None
        self._receiver_task: asyncio.Task | None = None  # type: ignore[type-arg]

    def connect(self, client_id: str = "mqtt_test_helper", cafile: str | None = None) -> None:
        self._loop = asyncio.new_event_loop()
        self._thread = threading.Thread(target=self._run_loop, daemon=True)
        self._thread.start()

        self._client = MQTTClient(client_id=client_id)
        scheme = "mqtts" if cafile else "mqtt"
        uri = f"{scheme}://{self._broker_host}:{self._broker_port}/"
        fut = asyncio.run_coroutine_threadsafe(self._client.connect(uri, cafile=cafile), self._loop)
        fut.result(timeout=10)

        # Start background message receiver
        fut = asyncio.run_coroutine_threadsafe(self._start_receiver(), self._loop)
        fut.result(timeout=5)

    def disconnect(self) -> None:
        if self._receiver_task and self._loop:
            self._loop.call_soon_threadsafe(self._receiver_task.cancel)
            time.sleep(0.2)
        if self._client and self._loop:
            fut = asyncio.run_coroutine_threadsafe(self._client.disconnect(), self._loop)
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
        fut = asyncio.run_coroutine_threadsafe(self._client.subscribe([(topic, QOS_0)]), self._loop)
        fut.result(timeout=10)

    def publish(self, topic: str, payload: str, *, retain: bool = False) -> None:
        assert self._client is not None and self._loop is not None
        fut = asyncio.run_coroutine_threadsafe(self._client.publish(topic, payload.encode("utf-8"), qos=QOS_0, retain=retain), self._loop)
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
        """Wait for any message whose topic starts with *topic_prefix*."""
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
                assert pkt is not None and pkt.variable_header is not None and pkt.payload is not None
                topic = pkt.variable_header.topic_name
                payload = pkt.payload.data.decode("utf-8", errors="replace")  # type: ignore[union-attr]
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


def make_mqtt_trigger(topic_filter: str, payload: str = "", retain: bool = False, use_prefix: bool = False) -> list:
    return [
        TRIGGER_MQTT,
        {
            "topic_filter": topic_filter,
            "payload": payload,
            "retain": retain,
            "use_prefix": use_prefix,
        },
    ]


def make_http_trigger(url_suffix: str, method: int = HTTP_GET_POST_PUT, payload: str = "") -> list:
    return [
        TRIGGER_HTTP,
        {
            "method": method,
            "url_suffix": url_suffix,
            "payload": payload,
        },
    ]


def make_print_action(message: str) -> list:
    return [
        ACTION_PRINT,
        {"message": message},
    ]


def make_mqtt_action(topic: str, payload: str, retain: bool = False, use_prefix: bool = False) -> list:
    return [
        ACTION_MQTT,
        {
            "topic": topic,
            "payload": payload,
            "retain": retain,
            "use_prefix": use_prefix,
        },
    ]


def make_task(trigger: list, action: list, delay: int = 0) -> dict:
    return {
        "trigger": trigger,
        "action": action,
        "delay": delay,
    }


def wait_mqtt_state(tc: TestContext, expected: int, *, timeout: float = 20) -> None:
    def _check() -> None:
        try:
            state = tc.api("mqtt/state", timeout=5)
        except (TimeoutError, OSError):
            raise AssertionError("Device not reachable yet")
        tc.assert_eq(expected, state["connection_state"])

    tc.wait_for(_check, timeout=timeout)


def wait_mqtt_disconnected(tc: TestContext, *, timeout: float = 60) -> None:
    """Wait until the device reports NOT_CONNECTED or ERROR."""

    def _check() -> None:
        try:
            state = tc.api("mqtt/state", timeout=5)
        except (TimeoutError, OSError):
            raise AssertionError("Device not reachable yet")
        conn = state["connection_state"]
        if conn not in (CONN_NOT_CONNECTED, CONN_ERROR):
            raise AssertionError(f"Expected NotConnected (1) or Error (3), got {conn}")

    tc.wait_for(_check, timeout=timeout)


def ensure_mqtt_connected(tc: TestContext, config: dict | None = None) -> None:
    if config:
        tc.api("mqtt/config_update", config, timeout=5)
    wait_mqtt_state(tc, CONN_CONNECTED)


def assert_in_event_log(tc: TestContext, marker: str, *, timeout: float = 10) -> None:
    """Wait for *marker* to appear in the device event log."""

    def _check() -> None:
        log = tc.http_request("GET", "/event_log", timeout=5)
        if isinstance(log, bytes):
            log = log.decode("utf-8", errors="replace")
        tc.assert_(marker in log)

    tc.wait_for(_check, timeout=timeout)


def assert_not_in_event_log(tc: TestContext, marker: str) -> None:
    """Assert that *marker* does NOT appear in the device event log."""
    log = tc.http_request("GET", "/event_log", timeout=5)
    if isinstance(log, bytes):
        log = log.decode("utf-8", errors="replace")
    tc.assert_false(marker in log)


def send_trigger(tc: TestContext, suffix: str) -> None:
    tc.http_request("GET", f"/automation_trigger/{suffix}")
