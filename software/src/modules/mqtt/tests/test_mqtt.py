#!/usr/bin/env python3
"""Integration test for MQTT live-reconfigure.

Tests that the MQTT module can be enabled, disabled, and reconfigured at
runtime without requiring a device reboot. Also tests that automation MQTT
triggers and actions work correctly and are rebuilt on config changes.

Usage:
    python3 test_mqtt.py <device-ip>
    python3 test_mqtt.py <device-ip> --verbose
    python3 test_mqtt.py <device-ip> --test 09

Prerequisites:
    - Energy Manager 2.0
    - MQTT broker reachable from both the device and the test machine
    - python3 with paho-mqtt
"""

import argparse
import json
import sys
import threading
import time
import urllib.request
import urllib.error

import paho.mqtt.client as paho_mqtt


# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

# MQTT connection states (from Mqtt Connection State.uint8.enum)
CONN_NOT_CONFIGURED = 0
CONN_NOT_CONNECTED = 1
CONN_CONNECTED = 2
CONN_ERROR = 3

# Automation trigger/action IDs
TRIGGER_MQTT = 3
TRIGGER_HTTP = 18
ACTION_PRINT = 1
ACTION_MQTT = 2
ACTION_EM_RELAY_SWITCH = 13

# HTTP trigger methods
HTTP_GET_POST_PUT = 4


# ---------------------------------------------------------------------------
# Device communication helpers
# ---------------------------------------------------------------------------

def get_json(host, path):
    url = f"http://{host}/{path}"
    req = urllib.request.Request(url)
    with urllib.request.urlopen(req, timeout=5) as resp:
        return json.loads(resp.read())


def put_json(host, path, payload):
    url = f"http://{host}/{path}"
    data = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(url, data=data, method="PUT")
    req.add_header("Content-Type", "application/json")
    try:
        with urllib.request.urlopen(req, timeout=10) as resp:
            body = resp.read().decode("utf-8", errors="replace")
            return resp.status, body
    except urllib.error.HTTPError as e:
        body = e.read().decode("utf-8", errors="replace")
        return e.code, body


def get_text(host, path):
    url = f"http://{host}/{path}"
    req = urllib.request.Request(url)
    with urllib.request.urlopen(req, timeout=5) as resp:
        return resp.read().decode("utf-8", errors="replace")


def send_trigger(host, suffix, method="GET", payload=None):
    url = f"http://{host}/automation_trigger/{suffix}"
    data = payload.encode("utf-8") if payload else None
    req = urllib.request.Request(url, data=data, method=method)
    if data:
        req.add_header("Content-Type", "text/plain")
    try:
        with urllib.request.urlopen(req, timeout=10) as resp:
            body = resp.read().decode("utf-8", errors="replace")
            return resp.status, body
    except urllib.error.HTTPError as e:
        body = e.read().decode("utf-8", errors="replace")
        return e.code, body


# ---------------------------------------------------------------------------
# Device state helpers
# ---------------------------------------------------------------------------

def get_mqtt_config(host):
    return get_json(host, "mqtt/config")


def set_mqtt_config(host, config):
    status, body = put_json(host, "mqtt/config_update", config)
    if status != 200:
        raise RuntimeError(f"mqtt/config_update failed: HTTP {status}: {body}")


def get_mqtt_state(host):
    return get_json(host, "mqtt/state")


def get_relay_state(host, index=0):
    state = get_json(host, "energy_manager/state")
    return state["relays"][index]


def set_relay_direct(host, index, closed):
    payload = [255, 255, 255, 255]
    payload[2 + index] = 1 if closed else 0
    status, body = put_json(host, "energy_manager/outputs_update", payload)
    if status != 200:
        raise RuntimeError(f"outputs_update failed: HTTP {status}: {body}")


def set_automation_config(host, tasks):
    payload = {"tasks": tasks}
    status, body = put_json(host, "automation/config_update", payload)
    if status != 200:
        raise RuntimeError(
            f"automation/config_update failed: HTTP {status}: {body}")


def get_automation_config(host):
    return get_json(host, "automation/config")


def wait_for_mqtt_state(host, expected_state, timeout=20, poll=0.5):
    """Wait for mqtt/state connection_state to reach expected_state."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        state = get_mqtt_state(host)
        last = state["connection_state"]
        if last == expected_state:
            return True
        time.sleep(poll)
    return False


def wait_for_relay(host, index, expected, timeout=15, poll=0.5):
    deadline = time.time() + timeout
    while time.time() < deadline:
        actual = get_relay_state(host, index)
        if actual == expected:
            return True
        time.sleep(poll)
    return False


# ---------------------------------------------------------------------------
# MQTT client helper
# ---------------------------------------------------------------------------

class MqttHelper:
    """Paho MQTT client wrapper for test subscriptions and publishing."""

    def __init__(self, broker_host, broker_port, broker_user, broker_pass):
        self.broker_host = broker_host
        self.broker_port = broker_port
        self.broker_user = broker_user
        self.broker_pass = broker_pass
        self.client = None
        self._messages = []
        self._lock = threading.Lock()
        self._events = {}  # topic -> threading.Event

    def connect(self, client_id="mqtt_test_helper"):
        if hasattr(paho_mqtt, 'CallbackAPIVersion'):
            self.client = paho_mqtt.Client(
                paho_mqtt.CallbackAPIVersion.VERSION2, client_id)
        else:
            self.client = paho_mqtt.Client(client_id=client_id)
        self.client.username_pw_set(self.broker_user, self.broker_pass)
        self.client.on_message = self._on_message
        self.client.connect(self.broker_host, self.broker_port, keepalive=60)
        self.client.loop_start()

    def disconnect(self):
        if self.client:
            self.client.loop_stop()
            self.client.disconnect()
            self.client = None

    def subscribe(self, topic):
        if self.client:
            self.client.subscribe(topic, qos=0)

    def publish(self, topic, payload, retain=False):
        if self.client:
            self.client.publish(topic, payload, qos=0, retain=retain)

    def clear_messages(self):
        with self._lock:
            self._messages.clear()
            self._events.clear()

    def wait_for_message(self, topic_filter, timeout=10):
        """Wait for a message on a topic (exact match).

        Returns (topic, payload) or None.
        """
        evt = threading.Event()
        with self._lock:
            # Check already received
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

    def wait_for_any_message(self, topic_prefix, timeout=10):
        """Wait for any message whose topic starts with topic_prefix."""
        deadline = time.time() + timeout
        while time.time() < deadline:
            with self._lock:
                for msg in self._messages:
                    if msg[0].startswith(topic_prefix):
                        return msg
            time.sleep(0.2)
        return None

    def get_messages(self):
        with self._lock:
            return list(self._messages)

    def _on_message(self, client, userdata, msg):
        entry = (msg.topic, msg.payload.decode("utf-8", errors="replace"))
        with self._lock:
            self._messages.append(entry)
            # Wake any waiters for this exact topic
            if msg.topic in self._events:
                self._events[msg.topic].set()


# ---------------------------------------------------------------------------
# Automation rule builders
# ---------------------------------------------------------------------------

def make_mqtt_trigger(topic_filter, payload="", retain=False,
                      use_prefix=False):
    return [TRIGGER_MQTT, {
        "topic_filter": topic_filter,
        "payload": payload,
        "retain": retain,
        "use_prefix": use_prefix,
    }]


def make_http_trigger(url_suffix, method=HTTP_GET_POST_PUT, payload=""):
    return [TRIGGER_HTTP, {
        "method": method,
        "url_suffix": url_suffix,
        "payload": payload,
    }]


def make_relay_action(index, closed):
    return [ACTION_EM_RELAY_SWITCH, {
        "index": index,
        "closed": closed,
    }]


def make_mqtt_action(topic, payload, retain=False, use_prefix=False):
    return [ACTION_MQTT, {
        "topic": topic,
        "payload": payload,
        "retain": retain,
        "use_prefix": use_prefix,
    }]


def make_print_action(message):
    return [ACTION_PRINT, {
        "message": message,
    }]


def make_task(trigger, action, delay=0):
    return {
        "trigger": trigger,
        "action": action,
        "delay": delay,
    }


# ---------------------------------------------------------------------------
# Test helpers
# ---------------------------------------------------------------------------

class TestFailure(Exception):
    pass


class TestContext:
    """Holds shared state across tests."""

    def __init__(self, host, broker_host, broker_port, broker_user,
                 broker_pass, verbose):
        self.host = host
        self.broker_host = broker_host
        self.broker_port = broker_port
        self.broker_user = broker_user
        self.broker_pass = broker_pass
        self.verbose = verbose
        self.mqtt_helper = None

    def log(self, msg):
        if self.verbose:
            print(f"    [debug] {msg}")

    def get_helper(self):
        """Get or create the MqttHelper."""
        if self.mqtt_helper is None:
            self.mqtt_helper = MqttHelper(
                self.broker_host, self.broker_port,
                self.broker_user, self.broker_pass)
            self.mqtt_helper.connect()
            time.sleep(0.5)  # let connection establish
        return self.mqtt_helper

    def close_helper(self):
        if self.mqtt_helper:
            self.mqtt_helper.disconnect()
            self.mqtt_helper = None


def ensure_mqtt_connected(ctx, config=None):
    """Ensure MQTT is enabled and connected. Optionally update config."""
    if config:
        set_mqtt_config(ctx.host, config)
    if not wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20):
        state = get_mqtt_state(ctx.host)
        raise RuntimeError(
            f"MQTT did not connect (state={state['connection_state']})")


def make_test_config(ctx, **overrides):
    """Build an MQTT config dict with test defaults, applying overrides."""
    cfg = {
        "enable_mqtt": True,
        "broker_host": ctx.broker_host,
        "broker_port": ctx.broker_port,
        "broker_username": ctx.broker_user,
        "broker_password": ctx.broker_pass,
        "global_topic_prefix": "wem2/test",
        "client_name": "wem2-test",
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


# ---------------------------------------------------------------------------
# Test cases: A. Enable/Disable
# ---------------------------------------------------------------------------

def test_01_disable_mqtt(ctx):
    """Disable MQTT, verify state goes to NotConfigured."""
    print("  [01] Disable MQTT ...")

    cfg = make_test_config(ctx, enable_mqtt=False)
    set_mqtt_config(ctx.host, cfg)

    if not wait_for_mqtt_state(ctx.host, CONN_NOT_CONFIGURED, timeout=20):
        state = get_mqtt_state(ctx.host)
        raise TestFailure(
            f"Expected NotConfigured (0), got {state['connection_state']}")

    print("    PASS")


def test_02_enable_mqtt(ctx):
    """Enable MQTT, verify connection to broker."""
    print("  [02] Enable MQTT ...")

    cfg = make_test_config(ctx)
    set_mqtt_config(ctx.host, cfg)

    if not wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20):
        state = get_mqtt_state(ctx.host)
        raise TestFailure(
            f"Expected Connected (2), got {state['connection_state']}")

    print("    PASS")


def test_03_disable_and_reenable(ctx):
    """Disable then re-enable MQTT in quick succession."""
    print("  [03] Disable then re-enable MQTT ...")

    # Disable
    cfg = make_test_config(ctx, enable_mqtt=False)
    set_mqtt_config(ctx.host, cfg)

    if not wait_for_mqtt_state(ctx.host, CONN_NOT_CONFIGURED, timeout=20):
        state = get_mqtt_state(ctx.host)
        raise TestFailure(
            f"Disable: expected NotConfigured (0), "
            f"got {state['connection_state']}")

    ctx.log("MQTT disabled successfully")

    # Re-enable
    cfg = make_test_config(ctx)
    set_mqtt_config(ctx.host, cfg)

    if not wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20):
        state = get_mqtt_state(ctx.host)
        raise TestFailure(
            f"Re-enable: expected Connected (2), "
            f"got {state['connection_state']}")

    print("    PASS")


def test_04_state_publishes_after_enable(ctx):
    """After enabling MQTT, device publishes state updates."""
    print("  [04] State publishes after enable ...")

    # Ensure connected
    ensure_mqtt_connected(ctx, make_test_config(ctx))
    prefix = "wem2/test"

    helper = ctx.get_helper()
    helper.clear_messages()
    helper.subscribe(f"{prefix}/#")
    time.sleep(1)

    # Wait for the periodic state push (interval=1s).
    msg = helper.wait_for_any_message(f"{prefix}/", timeout=15)

    if msg is None:
        raise TestFailure(
            f"No state messages received on {prefix}/# within 15s")

    ctx.log(f"Received: {msg[0]} = {msg[1][:80]}...")

    print("    PASS")


# ---------------------------------------------------------------------------
# Test cases: B. Config Changes
# ---------------------------------------------------------------------------

def test_05_change_topic_prefix(ctx):
    """Change global_topic_prefix, verify state arrives on new prefix."""
    print("  [05] Change topic prefix ...")

    old_prefix = "wem2/test"
    new_prefix = "wem2/test_new"

    # Start connected with old prefix
    ensure_mqtt_connected(ctx, make_test_config(ctx,
        global_topic_prefix=old_prefix))

    helper = ctx.get_helper()
    helper.clear_messages()
    helper.subscribe(f"{old_prefix}/#")
    helper.subscribe(f"{new_prefix}/#")
    time.sleep(1)

    # Change prefix
    cfg = make_test_config(ctx, global_topic_prefix=new_prefix)
    set_mqtt_config(ctx.host, cfg)

    if not wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20):
        state = get_mqtt_state(ctx.host)
        raise TestFailure(
            f"Not connected after prefix change "
            f"(state={state['connection_state']})")

    # Wait for a message on new prefix
    msg = helper.wait_for_any_message(f"{new_prefix}/", timeout=15)
    if msg is None:
        raise TestFailure(
            f"No messages on new prefix {new_prefix}/# within 15s")

    ctx.log(f"Received on new prefix: {msg[0]}")

    # Verify no *new* messages on old prefix after the switch.
    helper.clear_messages()
    time.sleep(3)
    old_msgs = [m for m in helper.get_messages()
                if m[0].startswith(f"{old_prefix}/")]
    if old_msgs:
        raise TestFailure(
            f"Still receiving on old prefix: {old_msgs[0][0]}")

    # Restore
    cfg = make_test_config(ctx)
    set_mqtt_config(ctx.host, cfg)
    wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20)

    print("    PASS")


def test_06_wrong_broker_port(ctx):
    """Change to wrong port, verify NotConnected. Restore, verify recovery."""
    print("  [06] Wrong broker port -> NotConnected ...")

    # Start connected
    ensure_mqtt_connected(ctx, make_test_config(ctx))

    # Change to wrong port
    cfg = make_test_config(ctx, broker_port=1884)
    set_mqtt_config(ctx.host, cfg)

    # Should become NotConnected (or stay that way)
    if not wait_for_mqtt_state(ctx.host, CONN_NOT_CONNECTED, timeout=20):
        state = get_mqtt_state(ctx.host)
        # Also accept Error state
        if state["connection_state"] != CONN_ERROR:
            raise TestFailure(
                f"Expected NotConnected (1) or Error (3), "
                f"got {state['connection_state']}")

    ctx.log("Correctly not connected on wrong port")

    # Restore correct port
    cfg = make_test_config(ctx)
    set_mqtt_config(ctx.host, cfg)

    if not wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20):
        state = get_mqtt_state(ctx.host)
        raise TestFailure(
            f"Did not recover after restoring port "
            f"(state={state['connection_state']})")

    print("    PASS")


def test_07_change_client_name(ctx):
    """Change client_name, verify reconnection."""
    print("  [07] Change client name ...")

    ensure_mqtt_connected(ctx, make_test_config(ctx))

    # Change client name
    cfg = make_test_config(ctx, client_name="wem2-test-renamed")
    set_mqtt_config(ctx.host, cfg)

    if not wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20):
        state = get_mqtt_state(ctx.host)
        raise TestFailure(
            f"Not connected after client name change "
            f"(state={state['connection_state']})")

    # Restore
    cfg = make_test_config(ctx)
    set_mqtt_config(ctx.host, cfg)
    wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20)

    print("    PASS")


def test_08_change_interval(ctx):
    """Change send interval, verify update timing changes."""
    print("  [08] Change interval ...")

    # Start with interval=1
    ensure_mqtt_connected(ctx, make_test_config(ctx, interval=1))
    prefix = "wem2/test"

    helper = ctx.get_helper()

    # Measure message rate with interval=1
    helper.clear_messages()
    helper.subscribe(f"{prefix}/#")
    time.sleep(5)
    msgs_fast = len(helper.get_messages())
    ctx.log(f"Messages in 5s at interval=1: {msgs_fast}")

    # Change to interval=10
    cfg = make_test_config(ctx, interval=10)
    set_mqtt_config(ctx.host, cfg)
    wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20)

    # Wait for initial burst to complete, then measure
    time.sleep(5)
    helper.clear_messages()
    time.sleep(5)
    msgs_slow = len(helper.get_messages())
    ctx.log(f"Messages in 5s at interval=10 (after burst): {msgs_slow}")

    # With interval=10, after the initial burst we should see very few
    # messages in a 5s window. The fast count should be notably higher.
    if msgs_fast <= msgs_slow:
        raise TestFailure(
            f"Expected fewer messages at interval=10. "
            f"Got {msgs_fast} at interval=1 vs {msgs_slow} at interval=10")

    # Restore
    cfg = make_test_config(ctx)
    set_mqtt_config(ctx.host, cfg)
    wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20)

    print("    PASS")


# ---------------------------------------------------------------------------
# Test cases: C. Automation MQTT Triggers
# ---------------------------------------------------------------------------

def test_09_mqtt_trigger_relay(ctx):
    """MQTT trigger (no prefix) -> close relay 0."""
    print("  [09] MQTT trigger closes relay ...")

    prefix = "wem2/test"
    ensure_mqtt_connected(ctx, make_test_config(ctx,
        global_topic_prefix=prefix))

    set_relay_direct(ctx.host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_mqtt_trigger("test/close", payload="on",
                                  retain=True, use_prefix=False),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(ctx.host, [task])
    time.sleep(2)

    # Publish via external client
    helper = ctx.get_helper()
    helper.publish("test/close", "on")

    if not wait_for_relay(ctx.host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close. State: {get_relay_state(ctx.host, 0)}")

    print("    PASS")


def test_10_mqtt_trigger_with_prefix(ctx):
    """MQTT trigger with use_prefix -> close relay 0."""
    print("  [10] MQTT trigger with prefix closes relay ...")

    prefix = "wem2/test"
    ensure_mqtt_connected(ctx, make_test_config(ctx,
        global_topic_prefix=prefix))

    set_relay_direct(ctx.host, 0, False)
    time.sleep(0.5)

    task = make_task(
        trigger=make_mqtt_trigger("test/pfx", payload="go",
                                  retain=True, use_prefix=True),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(ctx.host, [task])
    time.sleep(2)

    # With use_prefix, the full topic is <prefix>/automation_trigger/<filter>
    helper = ctx.get_helper()
    helper.publish(f"{prefix}/automation_trigger/test/pfx", "go")

    if not wait_for_relay(ctx.host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close. State: {get_relay_state(ctx.host, 0)}")

    print("    PASS")


def test_11_add_trigger_live(ctx):
    """Add MQTT trigger at runtime (no reboot), verify it fires."""
    print("  [11] Add MQTT trigger live ...")

    prefix = "wem2/test"
    ensure_mqtt_connected(ctx, make_test_config(ctx,
        global_topic_prefix=prefix))

    # Start with empty automation
    set_automation_config(ctx.host, [])
    time.sleep(1)

    set_relay_direct(ctx.host, 0, False)
    time.sleep(0.5)

    # Now add a trigger
    task = make_task(
        trigger=make_mqtt_trigger("test/live", payload="fire",
                                  retain=True, use_prefix=False),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(ctx.host, [task])
    time.sleep(2)

    helper = ctx.get_helper()
    helper.publish("test/live", "fire")

    if not wait_for_relay(ctx.host, 0, True, timeout=15):
        raise TestFailure(
            f"Relay 0 did not close after live add. "
            f"State: {get_relay_state(ctx.host, 0)}")

    print("    PASS")


def test_12_remove_trigger_live(ctx):
    """Remove MQTT trigger at runtime, verify it no longer fires."""
    print("  [12] Remove MQTT trigger live ...")

    prefix = "wem2/test"
    ensure_mqtt_connected(ctx, make_test_config(ctx,
        global_topic_prefix=prefix))

    # Set up a trigger
    task = make_task(
        trigger=make_mqtt_trigger("test/remove", payload="bang",
                                  retain=True, use_prefix=False),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(ctx.host, [task])
    time.sleep(2)

    # Verify it works first
    set_relay_direct(ctx.host, 0, False)
    time.sleep(0.5)

    helper = ctx.get_helper()
    helper.publish("test/remove", "bang")

    if not wait_for_relay(ctx.host, 0, True, timeout=15):
        raise TestFailure("Trigger did not work before removal")

    # Now remove all triggers
    set_automation_config(ctx.host, [])
    time.sleep(2)

    # Reset relay and try again
    set_relay_direct(ctx.host, 0, False)
    time.sleep(0.5)

    helper.publish("test/remove", "bang")
    time.sleep(3)

    if get_relay_state(ctx.host, 0):
        raise TestFailure(
            "Relay 0 closed after trigger was removed")

    print("    PASS")


def test_13_change_trigger_topic(ctx):
    """Change trigger topic from test/a to test/b."""
    print("  [13] Change trigger topic ...")

    prefix = "wem2/test"
    ensure_mqtt_connected(ctx, make_test_config(ctx,
        global_topic_prefix=prefix))

    # Start with topic test/a
    task = make_task(
        trigger=make_mqtt_trigger("test/a", payload="x",
                                  retain=True, use_prefix=False),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(ctx.host, [task])
    time.sleep(2)

    set_relay_direct(ctx.host, 0, False)
    time.sleep(0.5)

    helper = ctx.get_helper()

    # Verify test/a works
    helper.publish("test/a", "x")
    if not wait_for_relay(ctx.host, 0, True, timeout=15):
        raise TestFailure("Trigger test/a did not fire")

    # Change to test/b
    task = make_task(
        trigger=make_mqtt_trigger("test/b", payload="x",
                                  retain=True, use_prefix=False),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(ctx.host, [task])
    time.sleep(2)

    # test/a should no longer fire
    set_relay_direct(ctx.host, 0, False)
    time.sleep(0.5)

    helper.publish("test/a", "x")
    time.sleep(3)
    if get_relay_state(ctx.host, 0):
        raise TestFailure(
            "Old topic test/a still fires after change to test/b")

    # test/b should fire
    helper.publish("test/b", "x")
    if not wait_for_relay(ctx.host, 0, True, timeout=15):
        raise TestFailure("New topic test/b did not fire")

    print("    PASS")


# ---------------------------------------------------------------------------
# Test cases: D. Automation MQTT Actions
# ---------------------------------------------------------------------------

def test_14_mqtt_action_publishes(ctx):
    """HTTP trigger -> MQTT action (no prefix), verify message on broker."""
    print("  [14] MQTT action publishes ...")

    prefix = "wem2/test"
    ensure_mqtt_connected(ctx, make_test_config(ctx,
        global_topic_prefix=prefix))

    marker = f"test_action_{int(time.time())}"
    action_topic = "test/action_out"

    task = make_task(
        trigger=make_http_trigger("test_mqtt_action"),
        action=make_mqtt_action(action_topic, marker),
    )
    set_automation_config(ctx.host, [task])
    time.sleep(2)

    helper = ctx.get_helper()
    helper.clear_messages()
    helper.subscribe(action_topic)
    time.sleep(1)

    # Fire HTTP trigger
    status, body = send_trigger(ctx.host, "test_mqtt_action", method="GET")
    if status != 200:
        raise TestFailure(f"HTTP trigger returned {status}: {body}")

    msg = helper.wait_for_message(action_topic, timeout=10)
    if msg is None:
        raise TestFailure(
            f"No MQTT message on '{action_topic}' within 10s")

    if msg[1] != marker:
        raise TestFailure(
            f"Payload mismatch: expected '{marker}', got '{msg[1]}'")

    ctx.log(f"Received: {msg[0]} = {msg[1]}")

    print("    PASS")


def test_15_mqtt_action_with_prefix(ctx):
    """HTTP trigger -> MQTT action with use_prefix."""
    print("  [15] MQTT action with prefix ...")

    prefix = "wem2/test"
    ensure_mqtt_connected(ctx, make_test_config(ctx,
        global_topic_prefix=prefix))

    marker = f"pfx_action_{int(time.time())}"
    action_suffix = "test/pfx_out"
    full_topic = f"{prefix}/automation_action/{action_suffix}"

    task = make_task(
        trigger=make_http_trigger("test_pfx_action"),
        action=make_mqtt_action(action_suffix, marker, use_prefix=True),
    )
    set_automation_config(ctx.host, [task])
    time.sleep(2)

    helper = ctx.get_helper()
    helper.clear_messages()
    helper.subscribe(full_topic)
    time.sleep(1)

    status, body = send_trigger(ctx.host, "test_pfx_action", method="GET")
    if status != 200:
        raise TestFailure(f"HTTP trigger returned {status}: {body}")

    msg = helper.wait_for_message(full_topic, timeout=10)
    if msg is None:
        raise TestFailure(
            f"No MQTT message on '{full_topic}' within 10s")

    if msg[1] != marker:
        raise TestFailure(
            f"Payload mismatch: expected '{marker}', got '{msg[1]}'")

    ctx.log(f"Received: {msg[0]} = {msg[1]}")

    print("    PASS")


# ---------------------------------------------------------------------------
# Test cases: E. Edge Cases
# ---------------------------------------------------------------------------

def test_16_prefix_change_rebuilds_triggers(ctx):
    """Change MQTT prefix while a use_prefix trigger exists. Verify it
    fires on the new prefix."""
    print("  [16] Prefix change rebuilds triggers ...")

    old_prefix = "wem2/test"
    new_prefix = "wem2/test_pfx2"

    ensure_mqtt_connected(ctx, make_test_config(ctx,
        global_topic_prefix=old_prefix))

    # Set up trigger with use_prefix
    task = make_task(
        trigger=make_mqtt_trigger("pfxtest", payload="go",
                                  retain=True, use_prefix=True),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(ctx.host, [task])
    time.sleep(2)

    # Verify it works on old prefix
    set_relay_direct(ctx.host, 0, False)
    time.sleep(0.5)

    helper = ctx.get_helper()
    helper.publish(f"{old_prefix}/automation_trigger/pfxtest", "go")

    if not wait_for_relay(ctx.host, 0, True, timeout=15):
        raise TestFailure("Trigger did not fire on old prefix")

    ctx.log("Trigger works on old prefix")

    # Now change the MQTT prefix
    cfg = make_test_config(ctx, global_topic_prefix=new_prefix)
    set_mqtt_config(ctx.host, cfg)

    if not wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20):
        raise TestFailure("Not connected after prefix change")

    time.sleep(2)

    # Old prefix should no longer fire
    set_relay_direct(ctx.host, 0, False)
    time.sleep(0.5)

    helper.publish(f"{old_prefix}/automation_trigger/pfxtest", "go")
    time.sleep(3)
    if get_relay_state(ctx.host, 0):
        raise TestFailure(
            "Trigger still fires on OLD prefix after change")

    # New prefix should fire
    helper.publish(f"{new_prefix}/automation_trigger/pfxtest", "go")
    if not wait_for_relay(ctx.host, 0, True, timeout=15):
        raise TestFailure("Trigger did not fire on NEW prefix")

    ctx.log("Trigger correctly fires on new prefix")

    # Restore
    cfg = make_test_config(ctx)
    set_mqtt_config(ctx.host, cfg)
    wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20)

    print("    PASS")


def test_17_disable_stops_triggers(ctx):
    """Disable MQTT, verify automation triggers no longer fire."""
    print("  [17] Disable MQTT stops triggers ...")

    prefix = "wem2/test"
    ensure_mqtt_connected(ctx, make_test_config(ctx,
        global_topic_prefix=prefix))

    # Set up trigger
    task = make_task(
        trigger=make_mqtt_trigger("test/disable", payload="x",
                                  retain=True, use_prefix=False),
        action=make_relay_action(index=0, closed=True),
    )
    set_automation_config(ctx.host, [task])
    time.sleep(2)

    # Verify it works
    set_relay_direct(ctx.host, 0, False)
    time.sleep(0.5)

    helper = ctx.get_helper()
    helper.publish("test/disable", "x")

    if not wait_for_relay(ctx.host, 0, True, timeout=15):
        raise TestFailure("Trigger did not fire before disable")

    ctx.log("Trigger works before disable")

    # Disable MQTT
    cfg = make_test_config(ctx, enable_mqtt=False)
    set_mqtt_config(ctx.host, cfg)

    if not wait_for_mqtt_state(ctx.host, CONN_NOT_CONFIGURED, timeout=20):
        raise TestFailure(
            "MQTT did not become NotConfigured after disable")

    time.sleep(1)

    # Try to fire (device is disconnected, so it can't receive)
    set_relay_direct(ctx.host, 0, False)
    time.sleep(0.5)

    helper.publish("test/disable", "x")
    time.sleep(3)

    if get_relay_state(ctx.host, 0):
        raise TestFailure("Relay closed after MQTT was disabled")

    # Re-enable for subsequent tests
    cfg = make_test_config(ctx)
    set_mqtt_config(ctx.host, cfg)
    wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20)

    print("    PASS")


def test_18_state_on_new_prefix(ctx):
    """After prefix change, API states are published under new prefix."""
    print("  [18] State updates on new prefix ...")

    old_prefix = "wem2/test"
    new_prefix = "wem2/test_state"

    ensure_mqtt_connected(ctx, make_test_config(ctx,
        global_topic_prefix=old_prefix))

    helper = ctx.get_helper()
    helper.clear_messages()
    helper.subscribe(f"{old_prefix}/#")
    helper.subscribe(f"{new_prefix}/#")
    time.sleep(1)

    # Verify we get messages on old prefix
    msg = helper.wait_for_any_message(f"{old_prefix}/", timeout=15)
    if msg is None:
        raise TestFailure(f"No state on old prefix {old_prefix}")

    ctx.log(f"Got state on old prefix: {msg[0]}")

    # Change prefix
    cfg = make_test_config(ctx, global_topic_prefix=new_prefix)
    set_mqtt_config(ctx.host, cfg)
    wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20)

    # Wait for messages on new prefix
    helper.clear_messages()
    msg = helper.wait_for_any_message(f"{new_prefix}/", timeout=15)
    if msg is None:
        raise TestFailure(f"No state on new prefix {new_prefix}")

    ctx.log(f"Got state on new prefix: {msg[0]}")

    # Verify no new messages on old prefix
    time.sleep(3)
    old_msgs = [m for m in helper.get_messages()
                if m[0].startswith(f"{old_prefix}/")]
    if old_msgs:
        raise TestFailure(
            f"Still getting state on old prefix: {old_msgs[0][0]}")

    # Restore
    cfg = make_test_config(ctx)
    set_mqtt_config(ctx.host, cfg)
    wait_for_mqtt_state(ctx.host, CONN_CONNECTED, timeout=20)

    print("    PASS")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

ALL_TESTS = [
    # A. Enable/Disable
    test_01_disable_mqtt,
    test_02_enable_mqtt,
    test_03_disable_and_reenable,
    test_04_state_publishes_after_enable,
    # B. Config Changes
    test_05_change_topic_prefix,
    test_06_wrong_broker_port,
    test_07_change_client_name,
    test_08_change_interval,
    # C. Automation MQTT Triggers
    test_09_mqtt_trigger_relay,
    test_10_mqtt_trigger_with_prefix,
    test_11_add_trigger_live,
    test_12_remove_trigger_live,
    test_13_change_trigger_topic,
    # D. Automation MQTT Actions
    test_14_mqtt_action_publishes,
    test_15_mqtt_action_with_prefix,
    # E. Edge Cases
    test_16_prefix_change_rebuilds_triggers,
    test_17_disable_stops_triggers,
    test_18_state_on_new_prefix,
]


def main():
    parser = argparse.ArgumentParser(
        description="Integration test for MQTT live-reconfigure.",
        epilog=(
            "Example:\n"
            "  python3 test_mqtt.py 192.168.1.246\n"
            "  python3 test_mqtt.py 192.168.1.246 --verbose\n"
            "  python3 test_mqtt.py 192.168.1.246 --test 09\n"
            "  python3 test_mqtt.py 192.168.1.246 --broker mqtt.local"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("host", nargs="?", default=None,
                        help="Device IP or hostname")
    parser.add_argument("--broker", default="mqtt.localdomain",
                        help="MQTT broker hostname (default: mqtt.localdomain)")
    parser.add_argument("--broker-port", type=int, default=1883,
                        help="MQTT broker port (default: 1883)")
    parser.add_argument("--broker-user", default="olaf",
                        help="MQTT broker username (default: olaf)")
    parser.add_argument("--broker-pass", default="",
                        help="MQTT broker password")
    parser.add_argument("--verbose", "-v", action="store_true",
                        help="Verbose output")
    parser.add_argument("--test", "-t", type=str, default=None,
                        help="Run only tests matching this substring")
    args = parser.parse_args()

    if args.host is None:
        parser.print_help()
        sys.exit(1)

    host = args.host
    ctx = TestContext(
        host=host,
        broker_host=args.broker,
        broker_port=args.broker_port,
        broker_user=args.broker_user,
        broker_pass=args.broker_pass,
        verbose=args.verbose,
    )

    print("=== MQTT Live-Reconfigure Test ===")
    print(f"Device: {host}")
    print(f"Broker: {args.broker}:{args.broker_port}")
    print()

    # Verify connectivity
    try:
        state = get_mqtt_state(host)
        print(f"MQTT state: connection_state={state['connection_state']}")
    except Exception as e:
        print(f"ERROR: Cannot connect to device: {e}")
        sys.exit(1)

    try:
        em_state = get_json(host, "energy_manager/state")
        print(f"EM state: relays={em_state['relays']}")
    except Exception as e:
        print(f"ERROR: Cannot read EM state: {e}")
        sys.exit(1)

    # Save original configs for cleanup
    original_mqtt_config = get_mqtt_config(host)
    original_automation_config = get_automation_config(host)

    if ctx.verbose:
        print(f"Original MQTT config: "
              f"{json.dumps(original_mqtt_config, indent=2)}")
        print(f"Original automation config: "
              f"{json.dumps(original_automation_config, indent=2)}")

    print()

    # Select tests
    tests = ALL_TESTS
    if args.test:
        tests = [t for t in tests if args.test in t.__name__]
        if not tests:
            print(f"No tests matching '{args.test}'")
            sys.exit(1)

    passed = 0
    failed = 0
    errors = []

    for test_fn in tests:
        try:
            test_fn(ctx)
            passed += 1
        except TestFailure as e:
            failed += 1
            errors.append((test_fn.__name__, str(e)))
            print(f"    FAIL: {e}")
        except Exception as e:
            failed += 1
            errors.append((test_fn.__name__, f"Unexpected error: {e}"))
            print(f"    ERROR: {e}")
            if ctx.verbose:
                import traceback
                traceback.print_exc()

    # Cleanup
    print("\nCleaning up ...")
    ctx.close_helper()

    try:
        set_automation_config(host,
            original_automation_config.get("tasks", []))
    except Exception as e:
        print(f"  WARNING: Failed to restore automation config: {e}")

    try:
        # Restore MQTT config. Password is returned as null from the API,
        # so we need the original credentials.
        restore_cfg = dict(original_mqtt_config)
        if restore_cfg.get("broker_password") is None:
            restore_cfg["broker_password"] = args.broker_pass
        set_mqtt_config(host, restore_cfg)
    except Exception as e:
        print(f"  WARNING: Failed to restore MQTT config: {e}")

    for label, fn in [
        ("relay 0", lambda: set_relay_direct(host, 0, False)),
        ("relay 1", lambda: set_relay_direct(host, 1, False)),
    ]:
        try:
            fn()
        except Exception as e:
            if ctx.verbose:
                print(f"  WARNING: Failed to reset {label}: {e}")

    print(f"\n{'='*50}")
    print(f"Results: {passed} passed, {failed} failed, {len(tests)} total")

    if errors:
        print("\nFailed tests:")
        for name, err in errors:
            print(f"  - {name}: {err}")
        sys.exit(1)
    else:
        print("\nAll tests passed!")


if __name__ == "__main__":
    main()
