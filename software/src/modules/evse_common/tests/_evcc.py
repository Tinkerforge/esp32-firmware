"""evcc test instance running in Docker, configured with the WARP charger as the only loadpoint."""

import json
import os
import re
import time

import requests

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")
from software.test_runner.docker_container import Container

EVCC_IMAGE = os.environ.get("EVCC_TEST_IMAGE", "evcc/evcc:latest")
EVCC_CONTAINER_NAME = "evcc-warp-test"

# The grid meter is required by evcc but irrelevant for the tests.
EVCC_YAML = """\
network:
  port: {port}

log: info
levels:
  warp-ws: debug

interval: {interval}s

site:
  title: Test
  meters:
    grid: grid

meters:
  - name: grid
    type: custom
    power:
      source: const
      value: 0

chargers:
  - name: warp
    type: template
    template: tinkerforge-warp-ws
    uri: {uri}
    energyMeterIndex: {meter_index}

loadpoints:
  - title: Test
    charger: warp
    mode: off
"""

ANSI_ESCAPE_RE = re.compile(r"\x1b\[[0-9;]*m")
EVCC_LOG_PROBLEM_RE = re.compile(r"\b(ERROR|FATAL)\b")


class EvccError(Exception):
    pass


class Evcc:
    def __init__(self, port: int, charger_uri: str, meter_index: int, interval: int = 5, image: str = EVCC_IMAGE, container_name: str = EVCC_CONTAINER_NAME):
        self.port = port
        self.interval = interval
        self.base_url = f"http://127.0.0.1:{port}"
        config = EVCC_YAML.format(port=port, interval=interval, uri=charger_uri, meter_index=meter_index)
        self.container = Container(container_name, image, {"/etc/evcc.yaml": config}, command=["evcc", "--config", "/etc/evcc.yaml"])
        self._log_offset = 0

    def start(self, timeout: float = 60) -> None:
        self.container.start()
        self.wait_ready(timeout)

    def restart(self, timeout: float = 60) -> None:
        self.container.restart()
        self.wait_ready(timeout)

    def stop(self) -> None:
        self.container.remove()

    def wait_ready(self, timeout: float) -> None:
        """Waits until evcc serves its API and the loadpoint was set up (i.e. the charger was initialized)."""
        deadline = time.monotonic() + timeout
        last_error: Exception | str | None = None
        while time.monotonic() < deadline:
            self.container.assert_running()
            try:
                if self.loadpoint() is not None:
                    return
            except (requests.RequestException, ValueError, KeyError, IndexError) as e:
                last_error = e
            time.sleep(1)
        raise EvccError(f"evcc did not get ready within {timeout}s: {last_error}\n{self.logs()[-3000:]}")

    # ---- API -----------------------------------------------------------------

    def state(self) -> dict:
        r = requests.get(f"{self.base_url}/api/state", timeout=10)
        r.raise_for_status()
        data = r.json()
        # Older evcc versions wrap the state.
        return data.get("result", data)

    def loadpoint(self) -> dict:
        return self.state()["loadpoints"][0]

    def _post(self, path: str) -> None:
        r = requests.post(f"{self.base_url}/api/{path}", timeout=10)
        if r.status_code != 200:
            raise EvccError(f"POST {path} failed: {r.status_code} {r.text}")

    def set_mode(self, mode: str) -> None:
        self._post(f"loadpoints/1/mode/{mode}")

    def set_max_current(self, current: float) -> None:
        self._post(f"loadpoints/1/maxcurrent/{current:g}")

    # ---- Logs ----------------------------------------------------------------

    def logs(self) -> str:
        return ANSI_ESCAPE_RE.sub("", self.container.logs())

    def mark_log(self) -> None:
        self._log_offset = len(self.logs())

    def log_problems(self) -> list[str]:
        """ERROR and FATAL lines logged since the last mark_log()."""
        return [line for line in self.logs()[self._log_offset:].splitlines() if EVCC_LOG_PROBLEM_RE.search(line)]


def format_state(lp: dict) -> str:
    keys = ["mode", "connected", "charging", "enabled", "offeredCurrent", "maxCurrent", "chargePower", "chargeTotalImport", "chargeCurrents", "chargeVoltages"]
    return json.dumps({k: lp.get(k) for k in keys})
