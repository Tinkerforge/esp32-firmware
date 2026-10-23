"""Shared helpers for MQTT test suites.

Provides the test broker, MQTT client wrapper, automation builders,
and assertion helpers used by both the reconfigure and TLS test suites.
"""

import asyncio
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


class TestMQTTBroker: # uses tls when certfile/keyfile are provided
    """Runs an amqtt broker in a background thread with its own event loop."""

    def __init__(self, port: int, certfile: str | None = None, keyfile: str | None = None):
        self.port = port
        self.certfile = certfile
        self.keyfile = keyfile
        self.loop: asyncio.AbstractEventLoop | None = None
        self.thread: threading.Thread | None = None
        self.broker: AmqttBroker | None = None

    def start(self) -> None:
        self.loop = asyncio.new_event_loop()
        self.thread = threading.Thread(target=self.run_loop, daemon=True)
        self.thread.start()

        fut = asyncio.run_coroutine_threadsafe(self.start_broker(), self.loop)
        fut.result(timeout=10)

    async def start_broker(self) -> None:
        listener: dict = {
            "type": "tcp",
            "bind": f"0.0.0.0:{self.port}",
        }
        if self.certfile is not None:
            listener["ssl"] = True
            listener["certfile"] = self.certfile
            listener["keyfile"] = self.keyfile
        config = {
            "listeners": {"default": listener},
            "timeout_disconnect_delay": 0,
            "plugins": {
                "amqtt.plugins.authentication.AnonymousAuthPlugin": {
                    "allow_anonymous": True
                },
            },
        }
        self.broker = AmqttBroker(config)
        await self.broker.start()

    def stop(self) -> None:
        if self.broker and self.loop:
            fut = asyncio.run_coroutine_threadsafe(self.broker.shutdown(), self.loop)
            fut.result(timeout=10)
        if self.loop:
            self.loop.call_soon_threadsafe(self.loop.stop)
        if self.thread:
            self.thread.join(timeout=5)

    def run_loop(self) -> None:
        assert self.loop is not None
        asyncio.set_event_loop(self.loop)
        self.loop.run_forever()


class MqttHelper:
    def __init__(self, broker_host: str, broker_port: int):
        self.broker_host = broker_host
        self.broker_port = broker_port
        self.client: MQTTClient | None = None
        self.messages: list[tuple[str, str]] = []
        self.lock = threading.Lock()
        self.events: dict[str, threading.Event] = {}
        self.loop: asyncio.AbstractEventLoop | None = None
        self.thread: threading.Thread | None = None
        self.receiver_task: asyncio.Task | None = None  # type: ignore[type-arg]

    def connect(self, client_id: str = "mqtt_test_helper", cafile: str | None = None) -> None:
        self.loop = asyncio.new_event_loop()
        self.thread = threading.Thread(target=self.run_loop, daemon=True)
        self.thread.start()

        self.client = MQTTClient(client_id=client_id)
        scheme = "mqtts" if cafile else "mqtt"
        uri = f"{scheme}://{self.broker_host}:{self.broker_port}/"
        fut = asyncio.run_coroutine_threadsafe(self.client.connect(uri, cafile=cafile), self.loop)
        fut.result(timeout=10)

        # Start background message receiver
        fut = asyncio.run_coroutine_threadsafe(self.start_receiver(), self.loop)
        fut.result(timeout=5)

    def disconnect(self) -> None:
        if self.receiver_task and self.loop:
            self.loop.call_soon_threadsafe(self.receiver_task.cancel)
            time.sleep(0.2)
        if self.client and self.loop:
            fut = asyncio.run_coroutine_threadsafe(self.client.disconnect(), self.loop)
            try:
                fut.result(timeout=5)
            except Exception:
                pass
            self.client = None
        if self.loop:
            self.loop.call_soon_threadsafe(self.loop.stop)
        if self.thread:
            self.thread.join(timeout=5)

    def subscribe(self, topic: str) -> None:
        assert self.client is not None and self.loop is not None
        fut = asyncio.run_coroutine_threadsafe(self.client.subscribe([(topic, QOS_0)]), self.loop)
        fut.result(timeout=10)

    def publish(self, topic: str, payload: str, *, retain: bool = False) -> None:
        assert self.client is not None and self.loop is not None
        fut = asyncio.run_coroutine_threadsafe(self.client.publish(topic, payload.encode("utf-8"), qos=QOS_0, retain=retain), self.loop)
        fut.result(timeout=10)

    def clear_messages(self) -> None:
        with self.lock:
            self.messages.clear()
            self.events.clear()

    def wait_for_message(self, topic_filter: str, timeout: float = 10) -> tuple[str, str] | None:
        evt = threading.Event()
        with self.lock:
            for msg in self.messages:
                if msg[0] == topic_filter:
                    return msg
            self.events[topic_filter] = evt

        if evt.wait(timeout=timeout):
            with self.lock:
                for msg in self.messages:
                    if msg[0] == topic_filter:
                        return msg
        return None

    def wait_for_any_message(self, topic_prefix: str, timeout: float = 10) -> tuple[str, str] | None:
        """Wait for any message whose topic starts with *topic_prefix*."""
        deadline = time.time() + timeout
        while time.time() < deadline:
            with self.lock:
                for msg in self.messages:
                    if msg[0].startswith(topic_prefix):
                        return msg
            time.sleep(0.2)
        return None

    def get_messages(self) -> list[tuple[str, str]]:
        with self.lock:
            return list(self.messages)

    async def start_receiver(self) -> None:
        self.receiver_task = asyncio.create_task(self.receive_loop())

    async def receive_loop(self) -> None:
        assert self.client is not None
        while True:
            try:
                msg = await self.client.deliver_message(timeout_duration=1.0)
                if msg is None:
                    continue
                pkt = msg.publish_packet
                assert pkt is not None and pkt.variable_header is not None and pkt.payload is not None
                topic = pkt.variable_header.topic_name
                payload = pkt.payload.data.decode("utf-8", errors="replace")  # type: ignore[union-attr]
                entry = (topic, payload)
                with self.lock:
                    self.messages.append(entry)
                    if topic in self.events:
                        self.events[topic].set()
            except asyncio.CancelledError:
                return
            except TimeoutError:
                continue
            except Exception:
                # Connection lost or other error; stop receiving
                return

    def run_loop(self) -> None:
        assert self.loop is not None
        asyncio.set_event_loop(self.loop)
        self.loop.run_forever()


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


def wait_mqtt_state(tc: TestContext, expected: int, *, timeout: float = 20, after_connection_start: int | None = None) -> None:
    def check() -> None:
        try:
            state = tc.api("mqtt/state", timeout=5)
        except (TimeoutError, OSError):
            raise AssertionError("Device not reachable yet")
        tc.assert_eq(expected, state["connection_state"])
        if after_connection_start is not None:
            tc.assert_(state["connection_start"] != after_connection_start)

    tc.wait_for(check, timeout=timeout)


def wait_mqtt_disconnected(tc: TestContext, *, timeout: float = 60) -> None:
    """Wait until the device reports NOT_CONNECTED or ERROR."""

    def check() -> None:
        try:
            state = tc.api("mqtt/state", timeout=5)
        except (TimeoutError, OSError):
            raise AssertionError("Device not reachable yet")
        conn = state["connection_state"]
        if conn not in (CONN_NOT_CONNECTED, CONN_ERROR):
            raise AssertionError(f"Expected NotConnected (1) or Error (3), got {conn}")

    tc.wait_for(check, timeout=timeout)


def ensure_mqtt_connected(tc: TestContext, config: dict | None = None) -> None:
    previous_start = None
    if config is not None:
        previous_start = tc.api("mqtt/state")["connection_start"]
        tc.api("mqtt/config_update", config)
    # Reconfiguration is deferred. "Connected" alone can still describe the old
    # client, just before it is stopped and replaced with the new one.
    wait_mqtt_state(tc, CONN_CONNECTED, after_connection_start=previous_start)


def event_log_count(tc: TestContext, marker: str) -> int:
    log = tc.http_request("GET", "/event_log")
    if isinstance(log, bytes):
        log = log.decode("utf-8", errors="replace")
    return log.count(marker)


def assert_in_event_log(tc: TestContext, marker: str, *, timeout: float = 10, count: int = 1) -> None:
    """Wait for *marker* to appear in the device event log at least *count* times."""

    def check() -> None:
        try:
            found = event_log_count(tc, marker)
        except (TimeoutError, OSError):
            raise AssertionError("Device not reachable yet")
        tc.assert_ge(count, found)

    tc.wait_for(check, timeout=timeout)


def publish_until_in_event_log(tc: TestContext, helper: MqttHelper, topic: str, payload: str, marker: str, *, timeout: float = 30, count: int = 1) -> None:
    """Publish until *marker* appears in the device event log at least *count* times.

    The device subscribes to automation trigger topics asynchronously after a config update.
    A single non-retained message published before the subscription reaches the broker is lost.
    Messages published by earlier iterations can still be in flight when this returns."""

    def check() -> None:
        helper.publish(topic, payload)
        assert_in_event_log(tc, marker, timeout=2, count=count)

    tc.wait_for(check, timeout=timeout)


def assert_not_in_event_log(tc: TestContext, marker: str) -> None:
    """Assert that *marker* does NOT appear in the device event log."""
    log = tc.http_request("GET", "/event_log")
    if isinstance(log, bytes):
        log = log.decode("utf-8", errors="replace")
    tc.assert_false(marker in log)


def send_trigger(tc: TestContext, suffix: str) -> None:
    tc.http_request("GET", f"/automation_trigger/{suffix}")
