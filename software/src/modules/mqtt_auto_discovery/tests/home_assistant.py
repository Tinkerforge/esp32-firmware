#!/usr/bin/env -S uv run --group tests --script

# End-to-end tests for the MQTT auto discovery with a real Home Assistant instance.
#
# Mosquitto and Home Assistant run in Docker containers (host network) on the test machine.
# Home Assistant is onboarded headlessly. The DUT and Home Assistant are both connected to the broker.
#
# Requires Docker. The images can be overridden with the HA_TEST_IMAGE and MOSQUITTO_TEST_IMAGE environment variables.
# Set HA_TEST_KEEP=1 to keep the containers running after the suite for debugging.

import json
import logging
import os
import re
import time
from urllib.error import HTTPError

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext, MeterValueID
from software.src.modules.mqtt.tests._common import MqttHelper, ensure_mqtt_connected, wait_mqtt_state, CONN_NOT_CONFIGURED
from software.src.modules.mqtt_auto_discovery.tests._home_assistant import HomeAssistant, MosquittoBroker, docker_available

CLIENT_NAME = "tf-ha-test"
RENAMED_CLIENT_NAME = "tf-ha-renamed"
TOPIC_PREFIX = "tf_ha_test"
DISCOVERY_PREFIX = "homeassistant"

MODE_DISABLED = 0
MODE_HOME_ASSISTANT = 2

LANGUAGE_GERMAN = 0
LANGUAGE_ENGLISH = 1

METER_CLASS_NONE = 0
METER_CLASS_API = 4
METER_LOCATION_OTHER = 1
METER_DISPLAY_NAME = "HA Test"

CHARGE_MODE_FAST = 0
CHARGE_MODE_OFF = 1

CHARGE_MODE_SENSOR_OPTIONS_DE = ["Schnell", "Aus", "PV", "Min + PV", "Min", "Eco", "Eco + PV", "Eco + Min", "Eco + Min + PV", "Standardmodus"]
CHARGE_MODE_SENSOR_OPTIONS_EN = ["Fast", "Off", "PV", "Min + PV", "Min", "Eco", "Eco + PV", "Eco + Min", "Eco + Min + PV", "Default mode"]

# value_id -> (object_id suffix, fractional digits, unit, device_class)
DISCOVERABLE_METER_VALUES = {
    MeterValueID.VoltageL1N:              ("voltage_l1_n", 1, "V", "voltage"),
    MeterValueID.PowerActiveLSumImExDiff: ("power_active_l_sum_im_ex_diff", 0, "W", "power"),
    MeterValueID.EnergyActiveLSumImport:  ("energy_active_l_sum_import", 3, "kWh", "energy"),
    MeterValueID.FrequencyLAvg:           ("frequency_l_avg", 3, "Hz", "frequency"),
    MeterValueID.StateOfCharge:           ("state_of_charge", 1, "%", "battery"),
}

# VoltageL1L2 is not in the auto discovery's meter value list and must not create an entity.
METER_VALUE_IDS = [
    MeterValueID.VoltageL1N,
    MeterValueID.VoltageL1L2,
    MeterValueID.PowerActiveLSumImExDiff,
    MeterValueID.EnergyActiveLSumImport,
    MeterValueID.FrequencyLAvg,
    MeterValueID.StateOfCharge,
]

# Values are chosen to not be rounding ties.
METER_VALUES = [230.46, 401.2, 1234.56, 12345.67891, 50.01234, 55.27]

ANSI_ESCAPE_RE = re.compile(r"\x1b\[[0-9;]*m")
HA_LOG_LINE_RE = re.compile(r"^\d{4}-\d{2}-\d{2} \S+ (DEBUG|INFO|WARNING|ERROR|CRITICAL) \(")

DISCOVERY_TIMEOUT = 90

broker: MosquittoBroker | None = None
mqtt_helper: MqttHelper | None = None
ha: HomeAssistant | None = None
broker_host = ""
broker_port = 0
meter_slot = -1

original_mqtt_config: dict | None = None
original_auto_discovery_config: dict | None = None
original_i18n_config: dict | None = None
original_charge_manager_config: dict | None = None
original_meter_config: list | None = None
original_charge_mode: int | None = None
reboot_on_restore = False
ha_log_offset = 0


# ---- Helpers -----------------------------------------------------------------

def get_ha() -> HomeAssistant:
    assert ha is not None
    return ha


def get_helper() -> MqttHelper:
    assert mqtt_helper is not None
    return mqtt_helper


def make_mqtt_config(**overrides: object) -> dict:
    assert original_mqtt_config is not None
    cfg = dict(original_mqtt_config)
    cfg.update({
        "enable_mqtt": True,
        "broker_host": broker_host,
        "broker_port": broker_port,
        "broker_username": "",
        "broker_password": "",
        "global_topic_prefix": TOPIC_PREFIX,
        "client_name": CLIENT_NAME,
        "interval": 1,
        "protocol": 0,
        "cert_id": -1,
        "client_cert_id": -1,
        "client_key_id": -1,
        "path": "",
        "read_only": False,
    })
    cfg.update(overrides)
    return cfg


def set_auto_discovery(tc: TestContext, mode: int, prefix: str = DISCOVERY_PREFIX) -> None:
    tc.api("mqtt/auto_discovery_config_update", {"auto_discovery_mode": mode, "auto_discovery_prefix": prefix})


def set_language(tc: TestContext, language: int) -> None:
    cfg = tc.api("system/i18n_config")
    cfg["language"] = language
    tc.api("system/i18n_config_update", cfg)


def meter_unique_id(value_id: int, client_name: str = CLIENT_NAME) -> str:
    return f"{client_name}-meter_{meter_slot}_{DISCOVERABLE_METER_VALUES[value_id][0]}"


def ha_device_entities(client_name: str = CLIENT_NAME) -> dict[str, dict]:
    """HA entity registry entries of the DUT, keyed by unique_id."""
    return {uid: e for uid, e in get_ha().mqtt_entities().items() if uid.startswith(f"{client_name}-")}


def announced_entities(client_name: str = CLIENT_NAME, prefix: str = DISCOVERY_PREFIX) -> dict[str, dict]:
    """Latest non-empty discovery payloads the DUT published for client_name, keyed by unique_id."""
    latest: dict[str, str] = {}
    for topic, payload in get_helper().get_messages():
        parts = topic.split("/")
        if len(parts) == 5 and parts[0] == prefix and parts[2] == client_name and parts[4] == "config":
            latest[topic] = payload

    result: dict[str, dict] = {}
    for topic, payload in latest.items():
        if payload == "":
            continue
        cfg = json.loads(payload)
        cfg["__topic"] = topic
        result[cfg["unique_id"]] = cfg
    return result


def wait_for_ha_entities(tc: TestContext, unique_ids: set[str], *, timeout: float = DISCOVERY_TIMEOUT, client_name: str = CLIENT_NAME) -> dict[str, dict]:
    def check() -> dict[str, dict]:
        entities = ha_device_entities(client_name)
        missing = unique_ids - set(entities)
        if missing:
            raise AssertionError(f"Entities missing in Home Assistant: {sorted(missing)}")
        return entities

    return tc.wait_for(check, timeout=timeout, poll_delay=1)


def wait_for_no_ha_entities(tc: TestContext, *, timeout: float = DISCOVERY_TIMEOUT, client_name: str = CLIENT_NAME) -> None:
    def check() -> None:
        remaining = set(ha_device_entities(client_name))
        if remaining:
            raise AssertionError(f"Entities still present in Home Assistant: {sorted(remaining)}")

    tc.wait_for(check, timeout=timeout, poll_delay=1)


def wait_for_announcement_settled(tc: TestContext, *, client_name: str = CLIENT_NAME, prefix: str = DISCOVERY_PREFIX) -> dict[str, dict]:
    """Waits until the DUT announced at least one entity and no new discovery messages arrived for 3 seconds."""
    last_count = -1
    last_change = time.monotonic()
    deadline = time.monotonic() + DISCOVERY_TIMEOUT

    while time.monotonic() < deadline:
        count = sum(1 for topic, _ in get_helper().get_messages() if topic.startswith(f"{prefix}/") and f"/{client_name}/" in topic)
        if count != last_count:
            last_count = count
            last_change = time.monotonic()
        elif count > 0 and time.monotonic() - last_change >= 3:
            announced = announced_entities(client_name, prefix)
            tc.assert_gt(0, len(announced))
            return announced
        time.sleep(0.5)

    raise AssertionError(f"Discovery announcement for {client_name} did not settle (saw {last_count} messages)")


def ha_state(tc: TestContext, entity_id: str) -> dict:
    st = get_ha().state(entity_id)
    if st is None:
        raise AssertionError(f"Home Assistant has no state for {entity_id}")
    return st


def mark_ha_log() -> None:
    global ha_log_offset
    ha_log_offset = len(get_ha().logs())


def assert_no_ha_problems(tc: TestContext) -> None:
    """Fails on any MQTT or template related warning or error that Home Assistant logged since the last mark_ha_log()."""
    # Group continuation lines (e.g. tracebacks) with the log line they belong to.
    entries: list[tuple[str, str]] = []
    for line in ANSI_ESCAPE_RE.sub("", get_ha().logs()[ha_log_offset:]).splitlines():
        m = HA_LOG_LINE_RE.match(line)
        if m is not None:
            entries.append((m.group(1), line))
        elif entries:
            entries[-1] = (entries[-1][0], entries[-1][1] + "\n" + line)

    problems = []
    for level, text in entries:
        if level not in ("WARNING", "ERROR", "CRITICAL"):
            continue
        lower = text.lower()
        if "mqtt" in lower or "template" in lower or CLIENT_NAME in lower or RENAMED_CLIENT_NAME in lower or TOPIC_PREFIX in lower:
            problems.append(text)

    if problems:
        raise AssertionError("Home Assistant logged problems:\n" + "\n".join(problems))


def charge_mode_select_id() -> str:
    return f"{CLIENT_NAME}-active_charge_mode"


def entity_id_of(tc: TestContext, unique_id: str) -> str:
    entities = wait_for_ha_entities(tc, {unique_id})
    return entities[unique_id]["entity_id"]


def find_free_meter_slot(tc: TestContext) -> int:
    slot = 0
    while True:
        try:
            cfg = tc.api(f"meters/{slot}/config")
        except HTTPError as e:
            if e.code == 404:
                return -1
            raise
        if cfg[0] == METER_CLASS_NONE:
            return slot
        slot += 1


def ensure_discovery_enabled(tc: TestContext) -> None:
    """Brings the DUT back to the default state of the suite (in case a previous test failed half-way)."""
    mqtt_cfg = tc.api("mqtt/config")
    if mqtt_cfg["client_name"] != CLIENT_NAME or mqtt_cfg["read_only"] or mqtt_cfg["global_topic_prefix"] != TOPIC_PREFIX:
        set_auto_discovery(tc, MODE_DISABLED)
        ensure_mqtt_connected(tc, make_mqtt_config())

    disco = tc.api("mqtt/auto_discovery_config")
    if disco["auto_discovery_mode"] != MODE_HOME_ASSISTANT or disco["auto_discovery_prefix"] != DISCOVERY_PREFIX:
        set_auto_discovery(tc, MODE_HOME_ASSISTANT)

    if tc.api("system/i18n_config")["language"] != LANGUAGE_GERMAN:
        set_language(tc, LANGUAGE_GERMAN)


# ---- Suite -------------------------------------------------------------------

def suite_setup(tc: TestContext) -> None:
    global broker, mqtt_helper, ha, broker_host, broker_port, meter_slot, reboot_on_restore, \
        original_mqtt_config, original_auto_discovery_config, original_i18n_config, \
        original_charge_manager_config, original_meter_config, original_charge_mode

    if not docker_available():
        tc.skip("Docker is not available")

    # Starting Home Assistant (and pulling the image on the first run) takes a while.
    tc.set_test_timeout(15 * 60)

    logging.getLogger("amqtt").setLevel(logging.CRITICAL)

    if not tc.device_type().is_warp() and not tc.device_type().is_wem():
        tc.skip("Unsupported device type")

    meter_slot = find_free_meter_slot(tc)
    if meter_slot < 0:
        tc.skip("No free meter slot")

    original_mqtt_config = tc.api("mqtt/config")
    original_auto_discovery_config = tc.api("mqtt/auto_discovery_config")
    original_i18n_config = tc.api("system/i18n_config")
    original_meter_config = tc.api(f"meters/{meter_slot}/config")

    try:
        original_charge_manager_config = tc.api("charge_manager/config")
        original_charge_mode = tc.api("power_manager/charge_mode")["mode"]
    except HTTPError as e:
        if e.code != 404:
            raise

    # Configure everything that requires a reboot first.
    reboot_on_restore = True
    tc.api(f"meters/{meter_slot}/config_update", [
        METER_CLASS_API,
        {
            "display_name": METER_DISPLAY_NAME,
            "location": METER_LOCATION_OTHER,
            "excluded": False,
            "value_ids": METER_VALUE_IDS,
        },
    ])

    if original_charge_manager_config is not None and not original_charge_manager_config["enable_charge_manager"]:
        # The charge mode select is only announced if the charge manager is enabled.
        # The charge manager requires at least one charger. Use an unreachable dummy (TEST-NET-1).
        cm = dict(original_charge_manager_config)
        cm["enable_charge_manager"] = True
        cm["maximum_available_current"] = max(cm["maximum_available_current"], 32000)
        cm["chargers"] = [{"host": "192.0.2.1", "name": "Dummy", "rot": 0, "uid": 0}]
        tc.api("charge_manager/config_update", cm)

    # Disable discovery before switching the broker: Otherwise we would clear the discovery topics on the original broker.
    set_auto_discovery(tc, MODE_DISABLED)

    tc.reboot()

    broker_host = tc.get_local_ip()
    broker_port = tc.find_free_port(18830)
    broker = MosquittoBroker(broker_port)
    broker.start()

    mqtt_helper = MqttHelper("127.0.0.1", broker_port)
    mqtt_helper.connect(client_id="ha_discovery_test_helper")
    mqtt_helper.subscribe(f"{DISCOVERY_PREFIX}/#")

    ha = HomeAssistant(tc.find_free_port(18123))
    ha.start()
    ha.onboard()
    ha.add_mqtt_integration("127.0.0.1", broker_port)

    set_language(tc, LANGUAGE_GERMAN)
    ensure_mqtt_connected(tc, make_mqtt_config())
    set_auto_discovery(tc, MODE_HOME_ASSISTANT)

    tc.api(f"meters/{meter_slot}/update", METER_VALUES)

    wait_for_announcement_settled(tc)


def setup(tc: TestContext) -> None:
    ensure_discovery_enabled(tc)
    wait_for_ha_entities(tc, {meter_unique_id(MeterValueID.VoltageL1N)})
    mark_ha_log()


def test_all_announced_entities_created(tc: TestContext) -> None:
    announced = wait_for_announcement_settled(tc)
    entities = wait_for_ha_entities(tc, set(announced))

    # Home Assistant must not have any entity of the DUT that is not announced (anymore).
    tc.assert_eq(sorted(announced), sorted(entities))

    for uid, cfg in announced.items():
        component = cfg["__topic"].split("/")[1]
        tc.assert_eq(component, entities[uid]["entity_id"].split(".")[0])

    assert_no_ha_problems(tc)


def test_meter_entities(tc: TestContext) -> None:
    entities = wait_for_ha_entities(tc, {meter_unique_id(vid) for vid in DISCOVERABLE_METER_VALUES})

    # Only the values the auto discovery knows are announced.
    meter_prefix = f"{CLIENT_NAME}-meter_{meter_slot}_"
    tc.assert_eq(sorted(meter_unique_id(vid) for vid in DISCOVERABLE_METER_VALUES),
                 sorted(uid for uid in entities if uid.startswith(meter_prefix)))

    for vid, (_, digits, unit, device_class) in DISCOVERABLE_METER_VALUES.items():
        entity_id = entities[meter_unique_id(vid)]["entity_id"]
        attributes = ha_state(tc, entity_id)["attributes"]
        tc.assert_eq(unit, attributes.get("unit_of_measurement"))
        tc.assert_eq(device_class, attributes.get("device_class"))
        tc.assert_contains(METER_DISPLAY_NAME, attributes.get("friendly_name", ""))

    assert_no_ha_problems(tc)


def test_meter_values(tc: TestContext) -> None:
    entities = wait_for_ha_entities(tc, {meter_unique_id(vid) for vid in DISCOVERABLE_METER_VALUES})

    for values in (METER_VALUES, [v * 2 for v in METER_VALUES]):
        tc.api(f"meters/{meter_slot}/update", values)

        for vid, (_, digits, _, _) in DISCOVERABLE_METER_VALUES.items():
            entity_id = entities[meter_unique_id(vid)]["entity_id"]
            expected = round(values[METER_VALUE_IDS.index(vid)], digits)

            def check() -> None:
                state = ha_state(tc, entity_id)["state"]
                try:
                    actual = float(state)
                except ValueError:
                    raise AssertionError(f"{entity_id} has non-numeric state {state!r}, expected {expected}")
                # The DUT stores floats; allow for float32 precision.
                tc.assert_epsilon(expected, max(abs(expected) * 1e-6, 10 ** -(digits + 3)), actual)

            tc.wait_for(check, timeout=15, poll_delay=0.5)

    assert_no_ha_problems(tc)


def test_device(tc: TestContext) -> None:
    entities = wait_for_ha_entities(tc, set(announced_entities()))

    devices = [d for d in get_ha().device_registry() if ["mqtt", CLIENT_NAME] in d["identifiers"]]
    tc.assert_eq(1, len(devices))
    device = devices[0]

    tc.assert_eq(f"{device['model']} ({CLIENT_NAME})", device["name"])
    tc.assert_true(device["manufacturer"])
    tc.assert_true(tc.api("info/name")["display_type"].startswith(device["model"]))

    for uid, entity in entities.items():
        if entity["device_id"] != device["id"]:
            raise AssertionError(f"Entity {uid} is not assigned to the device {device['id']}")

    assert_no_ha_problems(tc)


def test_charge_mode_select(tc: TestContext) -> None:
    if original_charge_manager_config is None:
        tc.skip("No charge manager")

    entity_id = entity_id_of(tc, charge_mode_select_id())
    supported = tc.api("charge_manager/supported_charge_modes")
    tc.assert_in(supported, CHARGE_MODE_FAST)
    tc.assert_in(supported, CHARGE_MODE_OFF)

    def wait_state(option: str) -> None:
        tc.wait_for(lambda: tc.assert_eq(option, ha_state(tc, entity_id)["state"]), timeout=15, poll_delay=0.5)

    tc.api("power_manager/charge_mode_update", {"mode": CHARGE_MODE_FAST})
    wait_state("Schnell")
    tc.assert_eq(["Schnell", "Aus"], ha_state(tc, entity_id)["attributes"]["options"])

    # Home Assistant -> DUT
    get_ha().call_service("select", "select_option", {"entity_id": entity_id, "option": "Aus"})
    tc.wait_for(lambda: tc.assert_eq(CHARGE_MODE_OFF, tc.api("power_manager/charge_mode")["mode"]), timeout=15, poll_delay=0.5)
    wait_state("Aus")

    get_ha().call_service("select", "select_option", {"entity_id": entity_id, "option": "Schnell"})
    tc.wait_for(lambda: tc.assert_eq(CHARGE_MODE_FAST, tc.api("power_manager/charge_mode")["mode"]), timeout=15, poll_delay=0.5)
    wait_state("Schnell")

    # DUT -> Home Assistant
    tc.api("power_manager/charge_mode_update", {"mode": CHARGE_MODE_OFF})
    wait_state("Aus")
    tc.api("power_manager/charge_mode_update", {"mode": CHARGE_MODE_FAST})
    wait_state("Schnell")

    assert_no_ha_problems(tc)


def test_read_only_makes_commands_unavailable(tc: TestContext) -> None:
    if original_charge_manager_config is None:
        tc.skip("No charge manager")

    select_entity = entity_id_of(tc, charge_mode_select_id())
    sensor_entity = entity_id_of(tc, meter_unique_id(MeterValueID.VoltageL1N))

    tc.wait_for(lambda: tc.assert_ne("unavailable", ha_state(tc, select_entity)["state"]), timeout=15, poll_delay=0.5)

    try:
        ensure_mqtt_connected(tc, make_mqtt_config(read_only=True))
        tc.wait_for(lambda: tc.assert_eq("unavailable", ha_state(tc, select_entity)["state"]), timeout=30, poll_delay=0.5)
        tc.assert_ne("unavailable", ha_state(tc, sensor_entity)["state"])
    finally:
        ensure_mqtt_connected(tc, make_mqtt_config())

    tc.wait_for(lambda: tc.assert_ne("unavailable", ha_state(tc, select_entity)["state"]), timeout=30, poll_delay=0.5)

    assert_no_ha_problems(tc)


def test_language(tc: TestContext) -> None:
    uid = meter_unique_id(MeterValueID.VoltageL1N)
    entity_id = entity_id_of(tc, uid)

    def wait_name(name_part: str) -> None:
        tc.wait_for(lambda: tc.assert_contains(name_part, ha_state(tc, entity_id)["attributes"].get("friendly_name", "")), timeout=DISCOVERY_TIMEOUT, poll_delay=1)

    wait_name(f"Zähler {METER_DISPLAY_NAME} Spannung")

    if original_charge_manager_config is not None:
        tc.api("power_manager/charge_mode_update", {"mode": CHARGE_MODE_FAST})

    # Changing the language during a running announcement pass is not supported.
    wait_for_announcement_settled(tc)

    try:
        set_language(tc, LANGUAGE_ENGLISH)
        wait_name(f"Meter {METER_DISPLAY_NAME} Voltage")

        if original_charge_manager_config is not None:
            select_entity = entity_id_of(tc, charge_mode_select_id())
            tc.wait_for(lambda: tc.assert_eq(["Fast", "Off"], ha_state(tc, select_entity)["attributes"]["options"]), timeout=DISCOVERY_TIMEOUT, poll_delay=1)

            # Language-dependent sensor options are announced in both languages first, then in the new language only.
            sensor_entity = entity_id_of(tc, f"{CLIENT_NAME}-current_charge_mode")
            tc.wait_for(lambda: tc.assert_eq(CHARGE_MODE_SENSOR_OPTIONS_EN, ha_state(tc, sensor_entity)["attributes"]["options"]), timeout=DISCOVERY_TIMEOUT, poll_delay=1)
            tc.assert_eq("Fast", ha_state(tc, sensor_entity)["state"])

        wait_for_announcement_settled(tc)
    finally:
        set_language(tc, LANGUAGE_GERMAN)

    wait_name(f"Zähler {METER_DISPLAY_NAME} Spannung")

    if original_charge_manager_config is not None:
        sensor_entity = entity_id_of(tc, f"{CLIENT_NAME}-current_charge_mode")
        tc.wait_for(lambda: tc.assert_eq(CHARGE_MODE_SENSOR_OPTIONS_DE, ha_state(tc, sensor_entity)["attributes"]["options"]), timeout=DISCOVERY_TIMEOUT, poll_delay=1)
        tc.assert_eq("Schnell", ha_state(tc, sensor_entity)["state"])

    # The entity ID must stay stable when the name changes.
    tc.assert_eq(entity_id, ha_device_entities()[uid]["entity_id"])

    assert_no_ha_problems(tc)


def test_disable_removes_entities(tc: TestContext) -> None:
    expected = set(announced_entities())
    wait_for_ha_entities(tc, expected)

    set_auto_discovery(tc, MODE_DISABLED)
    wait_for_no_ha_entities(tc)

    set_auto_discovery(tc, MODE_HOME_ASSISTANT)
    wait_for_ha_entities(tc, expected)

    assert_no_ha_problems(tc)


def test_prefix_change(tc: TestContext) -> None:
    expected = set(announced_entities())
    wait_for_ha_entities(tc, expected)

    # Home Assistant only listens to the default prefix.
    set_auto_discovery(tc, MODE_HOME_ASSISTANT, "tf_ha_other_prefix")
    wait_for_no_ha_entities(tc)

    set_auto_discovery(tc, MODE_HOME_ASSISTANT)
    wait_for_ha_entities(tc, expected)

    assert_no_ha_problems(tc)


def test_client_name_change(tc: TestContext) -> None:
    expected_suffixes = {uid.removeprefix(f"{CLIENT_NAME}-") for uid in announced_entities()}
    wait_for_ha_entities(tc, {f"{CLIENT_NAME}-{s}" for s in expected_suffixes})

    try:
        ensure_mqtt_connected(tc, make_mqtt_config(client_name=RENAMED_CLIENT_NAME))
        wait_for_ha_entities(tc, {f"{RENAMED_CLIENT_NAME}-{s}" for s in expected_suffixes}, client_name=RENAMED_CLIENT_NAME)

        # The entities with the old client name are removed.
        wait_for_no_ha_entities(tc)

        renamed = announced_entities(RENAMED_CLIENT_NAME)
        tc.assert_eq(sorted(expected_suffixes), sorted(uid.removeprefix(f"{RENAMED_CLIENT_NAME}-") for uid in renamed))
        for cfg in renamed.values():
            tc.assert_eq(RENAMED_CLIENT_NAME, cfg["device"]["identifiers"])
    finally:
        ensure_mqtt_connected(tc, make_mqtt_config())

    wait_for_ha_entities(tc, {f"{CLIENT_NAME}-{s}" for s in expected_suffixes})
    wait_for_no_ha_entities(tc, client_name=RENAMED_CLIENT_NAME)

    assert_no_ha_problems(tc)


def test_reannounce_after_broker_restart(tc: TestContext) -> None:
    global mqtt_helper

    expected = set(announced_entities())
    tc.assert_gt(0, len(expected))

    # A restarted broker without persistence has lost all retained discovery topics.
    get_helper().disconnect()
    assert broker is not None
    broker.restart()

    mqtt_helper = MqttHelper("127.0.0.1", broker_port)
    mqtt_helper.connect(client_id="ha_discovery_test_helper")
    mqtt_helper.subscribe(f"{DISCOVERY_PREFIX}/#")

    # The device reconnects and must announce everything again without waiting for the next periodic pass.
    tc.wait_for(lambda: tc.assert_eq(sorted(expected), sorted(announced_entities())), timeout=60, poll_delay=1)

    # Home Assistant reconnects as well and keeps all entities.
    wait_for_ha_entities(tc, expected)
    tc.assert_eq(sorted(expected), sorted(ha_device_entities()))

    # Values reach Home Assistant again. (Losing the broker connection is logged as warning, don't check the log here.)
    entity_id = entity_id_of(tc, meter_unique_id(MeterValueID.VoltageL1N))
    tc.api(f"meters/{meter_slot}/update", [v + 1 for v in METER_VALUES])
    expected_voltage = round(METER_VALUES[METER_VALUE_IDS.index(MeterValueID.VoltageL1N)] + 1, 1)
    def check() -> None:
        state = ha_state(tc, entity_id)["state"]
        try:
            actual = float(state)
        except ValueError:
            raise AssertionError(f"{entity_id} has non-numeric state {state!r}")
        tc.assert_epsilon(expected_voltage, 0.01, actual)

    tc.wait_for(check, timeout=60, poll_delay=1)


def suite_teardown(tc: TestContext) -> None:
    global broker, mqtt_helper, ha

    if mqtt_helper is not None:
        mqtt_helper.disconnect()
        mqtt_helper = None

    if ha is not None:
        if os.environ.get("HA_TEST_KEEP") == "1":
            tc.dbg(f"Keeping Home Assistant container running at {ha.base_url} (user test, token {ha.token})")
        else:
            ha.stop()
        ha = None

    # The device may be temporarily unresponsive, so retry the restore calls a few times.
    def restore(api: str, payload: object) -> None:
        for attempt in range(5):
            try:
                tc.api(api, payload)
                return
            except (TimeoutError, OSError):
                if attempt == 4:
                    raise
                time.sleep(2)

    # Disable discovery first: Restoring the MQTT config (and with it the client name) must not
    # announce entities with the test's client name to the original broker.
    restore("mqtt/auto_discovery_config_update", {"auto_discovery_mode": MODE_DISABLED, "auto_discovery_prefix": DISCOVERY_PREFIX})

    if original_i18n_config is not None:
        restore("system/i18n_config_update", original_i18n_config)

    if original_charge_mode is not None:
        restore("power_manager/charge_mode_update", {"mode": original_charge_mode})

    if original_mqtt_config is not None:
        # Does not restore the original password...
        restore("mqtt/config_update", original_mqtt_config)
        if not original_mqtt_config["enable_mqtt"]:
            wait_mqtt_state(tc, CONN_NOT_CONFIGURED)

    if reboot_on_restore:
        if original_meter_config is not None:
            restore(f"meters/{meter_slot}/config_update", original_meter_config)
        if original_charge_manager_config is not None:
            restore("charge_manager/config_update", original_charge_manager_config)

    if original_auto_discovery_config is not None:
        restore("mqtt/auto_discovery_config_update", original_auto_discovery_config)

    if reboot_on_restore:
        tc.reboot()

    if broker is not None:
        if os.environ.get("HA_TEST_KEEP") != "1":
            broker.stop()
        broker = None


if __name__ == "__main__":
    run_testsuite(locals())
