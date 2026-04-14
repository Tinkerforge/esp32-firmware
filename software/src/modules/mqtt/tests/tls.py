#!/usr/bin/env -S uv run --group tests --script

# Tests for MQTT over TLS
#
# Verifies that the device can connect to a MQTT broker over TLS.

import logging
import os
import shutil
import tempfile
import time

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext
from software.src.modules.mqtt.tests._common import *

_PREFIX = "tls_test"
_CERT_ID = 4

_original_mqtt_config: dict | None = None
_original_automation_config: dict | None = None
_tls_broker: "TestMQTTBroker | None" = None
_plain_broker: "TestMQTTBroker | None" = None
_tls_helper: "MqttHelper | None" = None
_broker_host: str = ""
_tls_port: int = 0
_plain_port: int = 0
_cert_pem: str = ""
_key_pem: str = ""
_tmpdir: str = ""


def _make_tls_config(**overrides: object) -> dict:
    cfg: dict = {
        "enable_mqtt": True,
        "broker_host": _broker_host,
        "broker_port": _tls_port,
        "broker_username": "",
        "broker_password": "",
        "global_topic_prefix": _PREFIX,
        "client_name": "tls_test",
        "interval": 1,
        "protocol": 1,  # MQTT over SSL
        "cert_id": _CERT_ID,  # our self-signed broker cert
        "client_cert_id": -1,
        "client_key_id": -1,
        "path": "",
        "read_only": False,
    }
    cfg.update(overrides)
    return cfg


def _make_plain_config(**overrides: object) -> dict:
    cfg: dict = {
        "enable_mqtt": True,
        "broker_host": _broker_host,
        "broker_port": _plain_port,
        "broker_username": "",
        "broker_password": "",
        "global_topic_prefix": _PREFIX,
        "client_name": "tls_test",
        "interval": 1,
        "protocol": 0,  # plain TCP
        "cert_id": -1,
        "client_cert_id": -1,
        "client_key_id": -1,
        "path": "",
        "read_only": False,
    }
    cfg.update(overrides)
    return cfg


def _get_tls_helper() -> MqttHelper:
    assert _tls_helper is not None
    return _tls_helper


def suite_setup(tc: TestContext) -> None:
    global \
        _original_mqtt_config, \
        _original_automation_config, \
        _tls_broker, \
        _plain_broker, \
        _tls_helper, \
        _broker_host, \
        _tls_port, \
        _plain_port, \
        _cert_pem, \
        _key_pem, \
        _tmpdir

    logging.getLogger("amqtt").setLevel(logging.WARNING)

    _original_mqtt_config = tc.api("mqtt/config")
    _original_automation_config = tc.api("automation/config")

    _broker_host = tc.get_local_ip()

    # Generate a self-signed cert for the broker
    _cert_pem, _key_pem = tc.generate_self_signed_cert(_broker_host)

    # Write cert/key to temp files (amqtt needs file paths)
    _tmpdir = tempfile.mkdtemp(prefix="mqtt_tls_test_")
    cert_path = os.path.join(_tmpdir, "cert.pem")
    key_path = os.path.join(_tmpdir, "key.pem")
    with open(cert_path, "w") as f:
        f.write(_cert_pem)
    with open(key_path, "w") as f:
        f.write(_key_pem)

    # Start TLS broker
    _tls_port = tc.find_free_port(8883)
    _tls_broker = TestMQTTBroker(_tls_port, certfile=cert_path, keyfile=key_path)
    _tls_broker.start()

    # Start plain TCP broker (for switch tests)
    _plain_port = tc.find_free_port(_tls_port + 1)
    _plain_broker = TestMQTTBroker(_plain_port)
    _plain_broker.start()

    # Upload broker CA cert to the device
    tc.api("certs/add", {"id": _CERT_ID, "name": "mqtt-tls-test", "cert": _cert_pem})

    # Start test helper client (connects to TLS broker)
    _tls_helper = MqttHelper(_broker_host, _tls_port)
    _tls_helper.connect(cafile=cert_path)
    time.sleep(0.5)


# Bbasic TLS connectivity
def test_tls_connect(tc: TestContext) -> None:
    tc.api("mqtt/config_update", _make_tls_config(), timeout=5)
    wait_mqtt_state(tc, CONN_CONNECTED)


def test_tls_state_publishes(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_tls_config())

    helper = _get_tls_helper()
    helper.clear_messages()
    helper.subscribe(f"{_PREFIX}/#")
    time.sleep(1)

    msg = helper.wait_for_any_message(f"{_PREFIX}/", timeout=15)
    tc.assert_(msg is not None)


def test_tls_wrong_cert(tc: TestContext) -> None:
    # Generate a *different* self-signed cert and upload it
    wrong_cert_id = _CERT_ID + 1
    wrong_cert_pem, _ = tc.generate_self_signed_cert(_broker_host)
    tc.api(
        "certs/add",
        {"id": wrong_cert_id, "name": "mqtt-tls-wrong", "cert": wrong_cert_pem},
    )

    try:
        tc.api("mqtt/config_update", _make_tls_config(cert_id=wrong_cert_id), timeout=5)
        wait_mqtt_disconnected(tc)
    finally:
        # Restore working config and clean up
        tc.api("mqtt/config_update", _make_tls_config(), timeout=5)
        wait_mqtt_state(tc, CONN_CONNECTED)
        tc.api("certs/remove", {"id": wrong_cert_id})


def test_tls_no_cert(tc: TestContext) -> None:
    """Connection needs to fail when using the built-in CA bundle (cert_id=-1)."""
    tc.api("mqtt/config_update", _make_tls_config(cert_id=-1), timeout=5)
    wait_mqtt_disconnected(tc)

    # Restore
    tc.api("mqtt/config_update", _make_tls_config(), timeout=5)
    wait_mqtt_state(tc, CONN_CONNECTED)


# Switching between plain and TLS
def test_switch_plain_to_tls(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_plain_config())

    # Now switch to TLS
    tc.api("mqtt/config_update", _make_tls_config(), timeout=5)
    wait_mqtt_state(tc, CONN_CONNECTED)

    # Verify messages arrive through the TLS broker
    helper = _get_tls_helper()
    helper.clear_messages()
    helper.subscribe(f"{_PREFIX}/#")
    time.sleep(1)

    msg = helper.wait_for_any_message(f"{_PREFIX}/", timeout=15)
    tc.assert_(msg is not None)


def test_switch_tls_to_plain(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_tls_config())

    # Switch to plain
    tc.api("mqtt/config_update", _make_plain_config(), timeout=5)
    wait_mqtt_state(tc, CONN_CONNECTED)

    # Switch back to TLS for subsequent tests
    tc.api("mqtt/config_update", _make_tls_config(), timeout=5)
    wait_mqtt_state(tc, CONN_CONNECTED)


# Automation triggers and actions over TLS
def test_tls_trigger(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_tls_config())

    marker = f"TLS_TRIG_{int(time.time())}"
    task = make_task(
        trigger=make_mqtt_trigger(
            "tls/trig", payload="go", retain=True, use_prefix=False
        ),
        action=make_print_action(marker),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(2)

    _get_tls_helper().publish("tls/trig", "go")
    assert_in_event_log(tc, marker)


def test_tls_action(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, _make_tls_config())

    marker = f"tls_action_{int(time.time())}"
    action_topic = "tls/act_out"

    task = make_task(
        trigger=make_http_trigger("test_tls_action"),
        action=make_mqtt_action(action_topic, marker),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=3)
    time.sleep(2)

    helper = _get_tls_helper()
    helper.clear_messages()
    helper.subscribe(action_topic)
    time.sleep(1)

    send_trigger(tc, "test_tls_action")

    msg = helper.wait_for_message(action_topic, timeout=10)
    tc.assert_(msg is not None)
    assert msg is not None
    tc.assert_eq(marker, msg[1])


def suite_teardown(tc: TestContext) -> None:
    global _tls_helper, _tls_broker, _plain_broker, _tmpdir

    if _tls_helper:
        _tls_helper.disconnect()
        _tls_helper = None

    # Restore original configs
    for _ in range(5):
        try:
            if _original_automation_config is not None:
                tc.api(
                    "automation/config_update",
                    {"tasks": _original_automation_config.get("tasks", [])},
                    timeout=3,
                )
            break
        except (TimeoutError, OSError):
            time.sleep(2)

    for _ in range(5):
        try:
            if _original_mqtt_config is not None:
                restore = dict(_original_mqtt_config)
                tc.api("mqtt/config_update", restore, timeout=3)
            break
        except (TimeoutError, OSError):
            time.sleep(2)

    # Remove the uploaded cert from the device
    for _ in range(5):
        try:
            tc.api("certs/remove", {"id": _CERT_ID})
            break
        except (TimeoutError, OSError):
            time.sleep(2)

    if _tls_broker:
        _tls_broker.stop()
        _tls_broker = None

    if _plain_broker:
        _plain_broker.stop()
        _plain_broker = None

    if _tmpdir:
        shutil.rmtree(_tmpdir, ignore_errors=True)
        _tmpdir = ""

if __name__ == "__main__":
    run_testsuite(locals())
