"""Home Assistant test instance running in Docker.

Starts a minimal Home Assistant container in the host network,
onboards it headlessly and provides helpers for the REST and WebSocket APIs.
"""

import itertools
import json
import os
import time

import requests
from websockets.sync.client import connect as ws_connect

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.test_runner.docker_container import Container

HA_IMAGE = os.environ.get("HA_TEST_IMAGE", "ghcr.io/home-assistant/home-assistant:stable")
HA_CONTAINER_NAME = "ha-mqtt-discovery-test"

MOSQUITTO_IMAGE = os.environ.get("MOSQUITTO_TEST_IMAGE", "eclipse-mosquitto:2")
MOSQUITTO_CONTAINER_NAME = "ha-mqtt-discovery-test-broker"

MOSQUITTO_CONF = """\
listener {port}
allow_anonymous true
persistence false
"""

HA_CONFIGURATION_YAML = """\
homeassistant:
  name: Test
  latitude: 52.0
  longitude: 8.5
  elevation: 0
  unit_system: metric
  time_zone: Europe/Berlin
  country: DE
  language: en
  currency: EUR

http:
  server_port: {port}

api:
config:
onboarding:
websocket_api:

logger:
  default: warning
"""

HA_USER = "test"
HA_PASSWORD = "test-password-1234"


class HomeAssistantError(Exception):
    pass


class MosquittoBroker:
    """Mosquitto without persistence: A restart loses all retained messages."""

    def __init__(self, port: int, image: str = MOSQUITTO_IMAGE, container_name: str = MOSQUITTO_CONTAINER_NAME):
        self.port = port
        self.container = Container(container_name, image, {"/mosquitto/config/mosquitto.conf": MOSQUITTO_CONF.format(port=port)})

    def start(self, timeout: float = 30) -> None:
        self.container.start()
        self.container.wait_for_port(self.port, timeout)

    def restart(self, timeout: float = 30) -> None:
        self.container.restart()
        self.container.wait_for_port(self.port, timeout)

    def stop(self) -> None:
        self.container.remove()


def _schema_defaults(schema: list[dict]) -> dict:
    """Builds a config flow payload from the defaults of a serialized data_schema, including nested sections."""
    data: dict = {}
    for field in schema:
        if field.get("type") == "expandable":
            data[field["name"]] = _schema_defaults(field["schema"])
        elif "default" in field:
            data[field["name"]] = field["default"]
    return data


class HomeAssistant:
    def __init__(self, port: int, image: str = HA_IMAGE, container_name: str = HA_CONTAINER_NAME):
        self.port = port
        self.container = Container(container_name, image, {"/config/configuration.yaml": HA_CONFIGURATION_YAML.format(port=port)}, ["-e", "TZ=Europe/Berlin"])
        self.base_url = f"http://127.0.0.1:{port}"
        self.client_id = f"{self.base_url}/"
        self.token: str | None = None
        self._ws = None
        self._ws_ids = itertools.count(1)

    # ---- Container lifecycle -------------------------------------------------

    def start(self, timeout: float = 180) -> None:
        self.container.start()
        self._wait_for_http(timeout)

    def stop(self) -> None:
        self.ws_close()
        self.container.remove()

    def logs(self) -> str:
        return self.container.logs()

    def _wait_for_http(self, timeout: float) -> None:
        deadline = time.monotonic() + timeout
        last_error: Exception | None = None
        while time.monotonic() < deadline:
            self.container.assert_running()
            try:
                r = requests.get(f"{self.base_url}/api/onboarding", timeout=2)
                if r.status_code == 200:
                    return
            except requests.RequestException as e:
                last_error = e
            time.sleep(1)
        raise HomeAssistantError(f"Home Assistant did not start within {timeout}s: {last_error}")

    # ---- Onboarding and authentication ---------------------------------------

    def onboard(self) -> None:
        r = requests.post(f"{self.base_url}/api/onboarding/users", json={
            "client_id": self.client_id,
            "name": "Test",
            "username": HA_USER,
            "password": HA_PASSWORD,
            "language": "en",
        }, timeout=30)
        if r.status_code != 200:
            raise HomeAssistantError(f"Onboarding user creation failed: {r.status_code} {r.text}")
        auth_code = r.json()["auth_code"]

        r = requests.post(f"{self.base_url}/auth/token", data={
            "grant_type": "authorization_code",
            "code": auth_code,
            "client_id": self.client_id,
        }, timeout=30)
        if r.status_code != 200:
            raise HomeAssistantError(f"Token request failed: {r.status_code} {r.text}")
        self.token = r.json()["access_token"]

        # The remaining onboarding steps are skipped on purpose:
        # core_config would set up unrelated integrations (weather, TTS, ...) that need internet access.

    # ---- REST API ------------------------------------------------------------

    def _headers(self) -> dict[str, str]:
        if self.token is None:
            raise HomeAssistantError("Not authenticated")
        return {"Authorization": f"Bearer {self.token}"}

    def get(self, path: str):
        r = requests.get(f"{self.base_url}{path}", headers=self._headers(), timeout=30)
        if r.status_code != 200:
            raise HomeAssistantError(f"GET {path} failed: {r.status_code} {r.text}")
        return r.json()

    def post(self, path: str, payload: dict):
        r = requests.post(f"{self.base_url}{path}", headers=self._headers(), json=payload, timeout=30)
        if r.status_code not in (200, 201):
            raise HomeAssistantError(f"POST {path} failed: {r.status_code} {r.text}")
        return r.json() if r.text else None

    def states(self) -> dict[str, dict]:
        return {s["entity_id"]: s for s in self.get("/api/states")}

    def state(self, entity_id: str) -> dict | None:
        r = requests.get(f"{self.base_url}/api/states/{entity_id}", headers=self._headers(), timeout=30)
        if r.status_code == 404:
            return None
        if r.status_code != 200:
            raise HomeAssistantError(f"GET state {entity_id} failed: {r.status_code} {r.text}")
        return r.json()

    def call_service(self, domain: str, service: str, data: dict) -> None:
        self.post(f"/api/services/{domain}/{service}", data)

    # ---- MQTT integration ----------------------------------------------------

    def add_mqtt_integration(self, broker: str, port: int) -> None:
        flow = self.post("/api/config/config_entries/flow", {"handler": "mqtt", "show_advanced_options": False})

        # Depending on the HA version, the flow might start with a menu or directly with the broker form.
        for _ in range(5):
            if flow["type"] == "create_entry":
                return
            if flow["type"] == "abort":
                raise HomeAssistantError(f"MQTT config flow aborted: {flow}")
            if flow["type"] == "menu":
                flow = self.post(f"/api/config/config_entries/flow/{flow['flow_id']}", {"next_step_id": "broker"})
                continue
            if flow["type"] == "form" and flow["step_id"] == "broker":
                if flow.get("errors"):
                    raise HomeAssistantError(f"MQTT config flow failed: {flow['errors']}")
                data = _schema_defaults(flow["data_schema"])
                data.update({"broker": broker, "port": port})
                # Required TLS fields without defaults: Plain TCP without certificates.
                data.setdefault("other_settings", {}).update({"set_client_cert": False, "set_ca_cert": "off"})
                flow = self.post(f"/api/config/config_entries/flow/{flow['flow_id']}", data)
                continue
            raise HomeAssistantError(f"Unexpected MQTT config flow step: {flow}")

        raise HomeAssistantError(f"MQTT config flow did not finish: {flow}")

    # ---- WebSocket API -------------------------------------------------------

    def ws_command(self, command: dict, timeout: float = 30):
        if self._ws is None:
            self._ws_open()
        assert self._ws is not None

        msg_id = next(self._ws_ids)
        self._ws.send(json.dumps({"id": msg_id, **command}))

        deadline = time.monotonic() + timeout
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise HomeAssistantError(f"WebSocket command {command} timed out")
            msg = json.loads(self._ws.recv(timeout=remaining))
            if msg.get("id") != msg_id:
                continue
            if not msg.get("success", False):
                raise HomeAssistantError(f"WebSocket command {command} failed: {msg.get('error')}")
            return msg["result"]

    def _ws_open(self) -> None:
        ws = ws_connect(f"ws://127.0.0.1:{self.port}/api/websocket", max_size=None)
        msg = json.loads(ws.recv(timeout=10))
        if msg["type"] != "auth_required":
            raise HomeAssistantError(f"Unexpected WebSocket greeting: {msg}")
        ws.send(json.dumps({"type": "auth", "access_token": self.token}))
        msg = json.loads(ws.recv(timeout=10))
        if msg["type"] != "auth_ok":
            raise HomeAssistantError(f"WebSocket authentication failed: {msg}")
        self._ws = ws

    def ws_close(self) -> None:
        if self._ws is not None:
            try:
                self._ws.close()
            except Exception:
                pass
            self._ws = None

    def entity_registry(self) -> list[dict]:
        return self.ws_command({"type": "config/entity_registry/list"})

    def device_registry(self) -> list[dict]:
        return self.ws_command({"type": "config/device_registry/list"})

    def mqtt_entities(self) -> dict[str, dict]:
        """Returns all entities created by the MQTT integration, keyed by unique_id."""
        return {e["unique_id"]: e for e in self.entity_registry() if e["platform"] == "mqtt"}
