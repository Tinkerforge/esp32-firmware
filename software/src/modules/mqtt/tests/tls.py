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

PREFIX = "tls_test"
CERT_ID = 4

original_mqtt_config: dict | None = None
original_automation_config: dict | None = None
tls_broker: "TestMQTTBroker | None" = None
plain_broker: "TestMQTTBroker | None" = None
tls_helper: "MqttHelper | None" = None
broker_host: str = ""
tls_port: int = 0
plain_port: int = 0
cert_pem: str = ""
key_pem: str = ""
tmpdir: str = ""


def make_tls_config(**overrides: object) -> dict:
    cfg: dict = {
        "enable_mqtt": True,
        "broker_host": broker_host,
        "broker_port": tls_port,
        "broker_username": "",
        "broker_password": "",
        "global_topic_prefix": PREFIX,
        "client_name": "tls_test",
        "interval": 1,
        "protocol": 1,  # MQTT over SSL
        "cert_id": CERT_ID,  # our self-signed broker cert
        "client_cert_id": -1,
        "client_key_id": -1,
        "path": "",
        "read_only": False,
    }
    cfg.update(overrides)
    return cfg


def make_plain_config(**overrides: object) -> dict:
    cfg: dict = {
        "enable_mqtt": True,
        "broker_host": broker_host,
        "broker_port": plain_port,
        "broker_username": "",
        "broker_password": "",
        "global_topic_prefix": PREFIX,
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


def get_tls_helper() -> MqttHelper:
    assert tls_helper is not None
    return tls_helper


def suite_setup(tc: TestContext) -> None:
    global \
        original_mqtt_config, \
        original_automation_config, \
        tls_broker, \
        plain_broker, \
        tls_helper, \
        broker_host, \
        tls_port, \
        plain_port, \
        cert_pem, \
        key_pem, \
        tmpdir

    logging.getLogger("amqtt").setLevel(logging.WARNING)

    original_mqtt_config = tc.api("mqtt/config", timeout=MQTT_API_TIMEOUT)
    original_automation_config = tc.api("automation/config", timeout=MQTT_API_TIMEOUT)

    broker_host = tc.get_local_ip()

    # Generate a self-signed cert for the broker
    cert_pem, key_pem = tc.generate_self_signed_cert(broker_host)

    # Write cert/key to temp files (amqtt needs file paths)
    tmpdir = tempfile.mkdtemp(prefix="mqtt_tls_test_")
    cert_path = os.path.join(tmpdir, "cert.pem")
    key_path = os.path.join(tmpdir, "key.pem")
    with open(cert_path, "w") as f:
        f.write(cert_pem)
    with open(key_path, "w") as f:
        f.write(key_pem)

    # Start TLS broker
    tls_port = tc.find_free_port(8883)
    tls_broker = TestMQTTBroker(tls_port, certfile=cert_path, keyfile=key_path)
    tls_broker.start()

    # Start plain TCP broker (for switch tests)
    plain_port = tc.find_free_port(tls_port + 1)
    plain_broker = TestMQTTBroker(plain_port)
    plain_broker.start()

    # Upload broker CA cert to the device
    tc.api("certs/add", {"id": CERT_ID, "name": "mqtt-tls-test", "cert": cert_pem}, timeout=MQTT_API_TIMEOUT)

    # Start test helper client (connects to TLS broker)
    tls_helper = MqttHelper(broker_host, tls_port)
    tls_helper.connect(cafile=cert_path)
    time.sleep(0.5)


# Bbasic TLS connectivity
def test_tls_connect(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, make_tls_config())


def test_tls_state_publishes(tc: TestContext) -> None:
    prefix = f"{PREFIX}_state"
    helper = get_tls_helper()
    helper.clear_messages()
    helper.subscribe(f"{prefix}/#")
    time.sleep(1)

    ensure_mqtt_connected(tc, make_tls_config(global_topic_prefix=prefix))

    msg = helper.wait_for_any_message(f"{prefix}/", timeout=15)
    tc.assert_(msg is not None)


def test_tls_wrong_cert(tc: TestContext) -> None:
    # Generate a *different* self-signed cert and upload it
    wrong_cert_id = CERT_ID + 1
    wrong_cert_pem, _ = tc.generate_self_signed_cert(broker_host)
    tc.api(
        "certs/add",
        {"id": wrong_cert_id, "name": "mqtt-tls-wrong", "cert": wrong_cert_pem},
        timeout=MQTT_API_TIMEOUT,
    )

    try:
        tc.api("mqtt/config_update", make_tls_config(cert_id=wrong_cert_id), timeout=MQTT_API_TIMEOUT)
        wait_mqtt_disconnected(tc)
    finally:
        # Restore working config and clean up
        ensure_mqtt_connected(tc, make_tls_config())
        tc.api("certs/remove", {"id": wrong_cert_id}, timeout=MQTT_API_TIMEOUT)


def test_tls_no_cert(tc: TestContext) -> None:
    """Connection needs to fail when using the built-in CA bundle (cert_id=-1)."""
    tc.api("mqtt/config_update", make_tls_config(cert_id=-1), timeout=MQTT_API_TIMEOUT)
    wait_mqtt_disconnected(tc)

    # Restore
    ensure_mqtt_connected(tc, make_tls_config())


# Switching between plain and TLS
def test_switch_plain_to_tls(tc: TestContext) -> None:
    prefix = f"{PREFIX}_switch"
    ensure_mqtt_connected(tc, make_plain_config(global_topic_prefix=prefix))

    helper = get_tls_helper()
    helper.clear_messages()
    helper.subscribe(f"{prefix}/#")
    time.sleep(1)

    # Now switch to TLS
    ensure_mqtt_connected(tc, make_tls_config(global_topic_prefix=prefix))

    # Verify messages arrive through the TLS broker
    msg = helper.wait_for_any_message(f"{prefix}/", timeout=15)
    tc.assert_(msg is not None)


def test_switch_tls_to_plain(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, make_tls_config())

    # Switch to plain
    ensure_mqtt_connected(tc, make_plain_config())

    # Switch back to TLS for subsequent tests
    ensure_mqtt_connected(tc, make_tls_config())


# Automation triggers and actions over TLS
def test_tls_trigger(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, make_tls_config())

    marker = f"TLS_TRIG_{int(time.time())}"
    task = make_task(
        trigger=make_mqtt_trigger(
            "tls/trig", payload="go", retain=True, use_prefix=False
        ),
        action=make_print_action(marker),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=MQTT_API_TIMEOUT)
    time.sleep(2)

    publish_until_in_event_log(tc, get_tls_helper(), "tls/trig", "go", marker)


def test_tls_action(tc: TestContext) -> None:
    ensure_mqtt_connected(tc, make_tls_config())

    marker = f"tls_action_{int(time.time())}"
    action_topic = "tls/act_out"

    task = make_task(
        trigger=make_http_trigger("test_tls_action"),
        action=make_mqtt_action(action_topic, marker),
    )
    tc.api("automation/config_update", {"tasks": [task]}, timeout=MQTT_API_TIMEOUT)
    time.sleep(2)

    helper = get_tls_helper()
    helper.clear_messages()
    helper.subscribe(action_topic)
    time.sleep(1)

    send_trigger(tc, "test_tls_action")

    msg = helper.wait_for_message(action_topic, timeout=10)
    tc.assert_(msg is not None)
    assert msg is not None
    tc.assert_eq(marker, msg[1])


def suite_teardown(tc: TestContext) -> None:
    global tls_helper, tls_broker, plain_broker, tmpdir

    if tls_helper:
        tls_helper.disconnect()
        tls_helper = None

    # Restore original configs
    for _ in range(5):
        try:
            if original_automation_config is not None:
                tc.api(
                    "automation/config_update",
                    {"tasks": original_automation_config.get("tasks", [])},
                    timeout=MQTT_API_TIMEOUT,
                )
            break
        except (TimeoutError, OSError):
            time.sleep(2)

    for _ in range(5):
        try:
            if original_mqtt_config is not None:
                restore = dict(original_mqtt_config)
                tc.api("mqtt/config_update", restore, timeout=MQTT_API_TIMEOUT)
            break
        except (TimeoutError, OSError):
            time.sleep(2)

    # Remove the uploaded cert from the device
    for _ in range(5):
        try:
            tc.api("certs/remove", {"id": CERT_ID}, timeout=MQTT_API_TIMEOUT)
            break
        except (TimeoutError, OSError):
            time.sleep(2)

    if tls_broker:
        tls_broker.stop()
        tls_broker = None

    if plain_broker:
        plain_broker.stop()
        plain_broker = None

    if tmpdir:
        shutil.rmtree(tmpdir, ignore_errors=True)
        tmpdir = ""

if __name__ == "__main__":
    run_testsuite(locals())
