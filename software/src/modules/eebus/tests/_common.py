import time
import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import MeterValueID

METER_CLASS_API = 4
METER_LOCATION_CHARGER = 2
METER_LOCATION_GRID = 4

EEBUS_NO_VALUE = -2147483648

# MeterValueID constants matching the mvids[] array in eebus.cpp.
METER_VALUE_IDS = [
    MeterValueID.CurrentL1ImExDiff,
    MeterValueID.CurrentL2ImExDiff,
    MeterValueID.CurrentL3ImExDiff,
    MeterValueID.PowerActiveL1ImExDiff,
    MeterValueID.PowerActiveL2ImExDiff,
    MeterValueID.PowerActiveL3ImExDiff,
    MeterValueID.PowerActiveLSumImExDiff,
    MeterValueID.EnergyActiveLSumImport,
    MeterValueID.EnergyActiveLSumExport,
    MeterValueID.VoltageL1N,
    MeterValueID.VoltageL2N,
    MeterValueID.VoltageL3N,
    MeterValueID.VoltageL1L2,
    MeterValueID.VoltageL2L3,
    MeterValueID.VoltageL3L1,
    MeterValueID.FrequencyLAvg,
]

# Test values matching METER_VALUE_IDS order.
METER_VALUES = [
    10.0,
    11.0,
    9.0,    # Current per phase (A)
    2300.0,
    2530.0,
    2070.0, # Power per phase (W)
    6900.0, # Total power (W)
    100.0,  # Energy import (kWh)
    50.0,   # Energy export (kWh)
    230.0,
    231.0,
    229.0,  # Phase-neutral voltage (V)
    400.0,
    399.0,
    401.0,  # Phase-phase voltage (V)
    50.0,   # Frequency (Hz)
]

VOLTAGE_ONLY_IDS = [MeterValueID.VoltageL1N, MeterValueID.VoltageL2N, MeterValueID.VoltageL3N]
VOLTAGE_ONLY_VALUES = [230.0, 231.0, 229.0]

def enable_eebus(tc):
    config = tc.api("eebus/config")
    config["enable"] = True
    tc.api("eebus/config_update", config)
    time.sleep(1)


def set_eebus_enabled(tc, enabled: bool):
    config = tc.api("eebus/config")
    config["enable"] = enabled
    tc.api("eebus/config_update", config)


def device_uptime_ms(tc) -> int:
    """Uptime of the ESP in ms, from the info/keep_alive message that is sent first on a new web socket connection."""
    import json
    from websockets.sync.client import connect

    with connect(f"ws://{tc._esp_host}/ws", open_timeout=5, close_timeout=1, max_size=None) as ws:
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            msg = ws.recv(timeout=5)
            for line in (msg if isinstance(msg, str) else msg.decode()).splitlines():
                if '"info/keep_alive"' in line:
                    return json.loads(line)["payload"]["uptime"]
    raise AssertionError("No info/keep_alive received")


# LPC/LPP usecase_state in eebus/usecases
LPC_STARTUP = 0
LPC_INIT = 1
LPC_UNLIMITED_CONTROLLED = 2
LPC_LIMITED = 3
LPC_FAILSAFE = 4
LPC_UNLIMITED_AUTONOMOUS = 5

P14A_SOURCE_EEBUS = 1


def is_energy_manager(tc) -> bool:
    """True if the EEBUS module runs in energy manager mode (CEM), false in charger mode (EVSE)."""
    return "mgcp" in tc.api("eebus/usecases")


def lpc_state(tc) -> dict:
    return tc.api("eebus/usecases")["lpc"]


def wait_for_lpc_state(tc, state: int, timeout: float = 5.0) -> dict:
    def check():
        lpc = lpc_state(tc)
        tc.assert_eq(state, lpc["usecase_state"])
        return lpc
    return tc.wait_for(check, timeout=timeout, poll_delay=0.5)


def isolate_eebus_peers(tc, keep_skis: tuple[str, ...] = ()) -> list[dict]:
    """Removes all trusted peers (e.g. a real energy manager), so that they do not interfere with the test peer.
    Returns the removed peers for restore_eebus_peers()."""
    removed = []
    for peer in tc.api("eebus/state")["peers"]:
        if peer["trusted"] and peer["ski"] not in keep_skis:
            removed.append(peer)
            tc.api("eebus/remove", {"ski": peer["ski"]})
    if removed:
        time.sleep(2)
    return removed


def restore_eebus_peers(tc, removed: list[dict]):
    for peer in removed:
        tc.api("eebus/add", {
            "ski": peer["ski"],
            "ip": peer["ip"].split(";")[0],
            "port": peer["port"],
            "trusted": peer["trusted"],
            "persistent": peer["persistent"],
            "dns_name": peer["dns_name"],
            "wss_path": peer["wss_path"],
        })


def start_test_peer(tc, layout=None, **kwargs):
    """Creates an EEBUS test peer, registers it as trusted peer of the device and connects to the device."""
    from software.src.modules.eebus.tests._eebus_peer import EebusPeer

    host = tc._esp_host
    if host is None:
        tc.skip("ESP host not passed")
    kwargs.setdefault("log", tc.dbg)
    peer = EebusPeer(host, layout, **kwargs)
    tc.api("eebus/add", peer.add_peer_payload())
    try:
        peer.connect()
    except Exception:
        tc.api("eebus/remove", {"ski": peer.identity.ski})
        raise
    tc.assert_eq(tc.api("eebus/state")["ski"], peer.ship.remote_ski)
    return peer


def stop_test_peer(tc, peer):
    if peer is None:
        return
    peer.close()
    tc.api("eebus/remove", {"ski": peer.identity.ski})
