from enum import Enum
from dataclasses import dataclass
import csv
import json
import os
import re
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")

from software import util


# Meter value IDs to expose via MQTT auto-discovery (per meter slot).
# Add IDs from meters/meter_value_id.csv here to make them discoverable.
# Do not overdo it as this can drastically increase the required memory and flash size (number of meter value ids * meters_max_slots)
METER_VALUE_IDS = [
    1,
    2,
    3,
    13,
    17,
    21,
    74,
    210,
    212,
    356,
    364
]

meters_max_slots = util.get_env_metadata()['options']['meters_max_slots']
charge_mode_names_de = [
    "Schnell",
    "Aus",
    "PV",
    "Min + PV",
]
charge_mode_names_en = [
    "Fast",
    "Off",
    "PV",
    "Min + PV",
]


def command_template_for_select(names):
    """Generate a Jinja command_template that maps a selected option name back to a JSON mode payload."""
    mapping = ", ".join(f"'{name}': {i}" for i, name in enumerate(names))
    return "{%% set m = {%s} %%}{{ { 'mode': m.get(value, 0)} | tojson }}" % mapping


def enum_value_template(json_field, names):
    """Generate a Jinja value_template that maps an integer enum field to its name.
    Uses a Jinja `{% set map = {0: 'A', 1: 'B', ...} %}` lookup with a fallback to 'Unknown'."""
    mapping = ", ".join(f"{i}: '{name}'" for i, name in enumerate(names))
    return (
        "{{% set m = {{ {mapping} }} %}}"
        "{{{{ m.get(value_json.{field}, 'Unknown') }}}}"
    ).format(mapping=mapping, field=json_field)


def _sanitize_slug(s: str) -> str:
    """Convert a string into a ascii alphanumeric string for e.g. IDs"""
    s = s.lower().strip()
    s = s.replace("ä", "ae").replace("ö", "oe").replace("ü", "ue").replace("ß", "ss")
    s = s.replace("σ", "sum").replace("⌀", "avg")
    s = re.sub(r'[^a-z0-9]+', '_', s)
    return s.strip('_')


def load_meter_value_entries(wanted_ids: list[int]) -> list:
    """Read meter_value_id.csv and return entries only for the given IDs.

    Returns a list of tuples:
        (object_id_suffix, name_de, name_en, meter_value_id, rounding, unit,
         device_class, state_class)

    The HA device_class is derived from the CSV unit and measurand so that it
    matches https://www.home-assistant.io/integrations/sensor#device-class.
    The object_id_suffix is built from the CSV fields and the meter ID to guarantee uniqueness.
    """
    wanted = set(wanted_ids)
    csv_path = os.path.join(
        os.path.dirname(__file__), '..', 'meters', 'meter_value_id.csv'
    )

    # --- HA device-class mapping ------------------------------------------
    # Map (unit, measurand-hint) -> HA device_class.
    # HA only accepts specific unit strings per device_class; we must match.
    UNIT_TO_DEVICE_CLASS = {
        'V':     'voltage',
        'A':     'current',
        'W':     'power',
        'var':   'reactive_power',
        'VA':    'apparent_power',
        'kWh':   'energy',
        'kvarh': 'reactive_energy',  # HA accepts kvarh for reactive_energy
        'Hz':    'frequency',
        '°C':    'temperature',
        's':     'duration',
        'Ah':    None,      # no HA device class
        'kVAh':  None,      # HA has no "apparent_energy" class
        'rpm':   None,      # no HA device class
    }

    def get_device_class(unit: str, measurand: str) -> str | None:
        """Return the HA device_class string or None."""
        if unit == '%':
            if measurand == 'State Of Charge':
                return 'battery'
            if measurand == 'Power Factor':
                return 'power_factor'
            # THD percentages have no HA device class
            return None
        if unit == '°':
            # Phase angle - no HA device class
            return None
        return UNIT_TO_DEVICE_CLASS.get(unit)

    # --- state_class mapping ----------------------------------------------
    def get_state_class(measurand: str, kind: str, unit: str) -> str:
        """Return the HA state_class string."""
        # Energy values are totals (monotonically increasing or resettable)
        if measurand == 'Energy' or unit in ('kWh', 'kvarh', 'kVAh'):
            return 'total'
        # Electric charge (Ah) is also a total
        if unit == 'Ah':
            return 'total'
        # Run time is a total
        if measurand == 'Run Time':
            return 'total_increasing'
        # Capacity is a measurement
        if measurand == 'Capacity':
            return 'measurement'
        # Everything else (voltage, current, power, frequency, temp, etc.)
        return 'measurement'

    # --- Build object_id_suffix -------------------------------------------
    def build_object_id(row: dict) -> str:
        """Build a unique, short object_id_suffix from CSV fields.

        Format: {measurand}[_{submeasurand}]_{phase}[_{direction}][_{kind}]
        with appropriate shortening.
        """
        parts = []

        measurand = row['measurand'].strip()
        submeasurand = row['submeasurand'].strip()
        phase = row['phase'].strip()
        direction = row['direction'].strip()
        kind = row['kind'].strip()

        # Measurand
        meas_slug = _sanitize_slug(measurand)
        parts.append(meas_slug)

        # Submeasurand (e.g. Active, Reactive, Apparent, PV1..PV9, PV Sum, etc.)
        if submeasurand and not submeasurand.startswith('*'):
            parts.append(_sanitize_slug(submeasurand))

        # Phase
        if phase:
            parts.append(_sanitize_slug(phase))

        # Direction
        if direction and not direction.startswith('*'):
            parts.append(_sanitize_slug(direction))

        # Kind (Resettable, Virtual, etc.) - skip *-prefixed defaults
        if kind and not kind.startswith('*'):
            parts.append(_sanitize_slug(kind))

        return '_'.join(parts)

    # --- Build display names ----------------------------------------------
    def build_name(row: dict, lang: str) -> str:
        """Build a display name from display_name + display_name_muted columns."""
        name_key = f'display_name_{lang}'
        muted_key = f'display_name_{lang}_muted'
        name = row[name_key].strip()
        muted = row[muted_key].strip()
        if muted:
            return f"{name} ({muted})"
        return name

    # --- Read and parse CSV -----------------------------------------------
    entries = []
    seen_ids = set()

    with open(csv_path, newline='', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            vid = int(row['id'].strip())
            if vid not in wanted:
                continue
            unit = row['unit'].strip()
            digits = int(row['digits'].strip())
            measurand = row['measurand'].strip()
            kind = row['kind'].strip()

            obj_id = build_object_id(row)
            # Guarantee uniqueness by appending the numeric id if collision
            if obj_id in seen_ids:
                obj_id = f"{obj_id}_{vid}"
            seen_ids.add(obj_id)

            name_en = build_name(row, 'en')
            name_de = build_name(row, 'de')

            device_class = get_device_class(unit, measurand)
            state_class = get_state_class(measurand, kind, unit)

            entries.append((
                obj_id,     # object_id_suffix
                name_de,    # name_de
                name_en,    # name_en
                vid,        # meter_value_id
                digits,     # rounding
                unit,       # unit_of_measurement
                device_class,  # HA device_class (None if not applicable)
                state_class,   # HA state_class
            ))

    found_ids = {e[3] for e in entries}
    missing = wanted - found_ids
    if missing:
        raise ValueError(f"Meter value IDs not found in CSV: {sorted(missing)}")

    return entries


class DiscoveryType(Enum):
    STATE_ONLY = "StateOnly"
    STATE_AND_UPDATE = "StateAndUpdate"
    COMMAND_ONLY = "CommandOnly"


class Component(Enum):
    BINARY_SENSOR = "binary_sensor"
    SENSOR = "sensor"
    SWITCH = "switch"
    BUTTON = "button"
    NUMBER = "number"
    SELECT = "select"

    def get_discovery_type(self) -> DiscoveryType:
        return {
            Component.BINARY_SENSOR: DiscoveryType.STATE_ONLY,
            Component.SENSOR: DiscoveryType.STATE_ONLY,
            Component.SWITCH: DiscoveryType.STATE_AND_UPDATE,
            Component.NUMBER: DiscoveryType.STATE_AND_UPDATE,
            Component.BUTTON: DiscoveryType.COMMAND_ONLY,
            Component.SELECT: DiscoveryType.STATE_AND_UPDATE,
        }[self]


class Feature(Enum):
    NULL = "none"
    EVSE = "evse"
    METER = "meter"
    METER_PHASES = "meter_phases"
    METERS = "meters"


class CheckType(Enum):
    FEATURE = "Feature"  # Check api.hasFeature(feature)
    API_BOOL = "ApiBool"  # Check API path exists and bool key is true
    METER_VALUE = "MeterValue"  # Check meter config enabled + value_id present in value_ids


@dataclass
class AvailabilityEntry:
    topic: str
    value_template: str


# Common availability entry to check if MQTT is not in read-only mode.
# Should be added to all entities that have a command_topic.
MQTT_NOT_READ_ONLY = AvailabilityEntry(
    topic="mqtt/config",
    value_template="{{ 'online' if not value_json.read_only else 'offline' }}",
)


@dataclass
class Entity:
    include_generic: bool
    component: Component
    object_id: str
    path: str
    name_de: str
    name_en: str
    availability: list  # list of AvailabilityEntry
    static_info_generic: dict
    static_info_homeassistant: dict
    feature: Feature = Feature.NULL
    json_attributes_topic: str = ""
    json_attributes_info: dict = None
    # Optional English overrides for language-dependent static_info content
    # (e.g. value_template strings containing localized labels).
    # When set, the C++ code picks between the default (German) and English
    # variants at runtime based on the configured language.
    # When left empty, the default static_info is used regardless of language.
    static_info_generic_en: dict = None
    static_info_homeassistant_en: dict = None
    # Check type: how to determine if this entity should be announced.
    check_type: CheckType = CheckType.FEATURE
    api_check_path: str = None  # API path to check (for ApiBool and MeterValue)
    api_check_key: str = None  # Key within the config to check (for ApiBool)
    meter_value_id: int = -1   # MeterValueID to look up in value_ids (for MeterValue)
    value_template_fmt: str = None  # printf format with %d for resolved index (for MeterValue)

    def __post_init__(self):
        if self.json_attributes_info is None:
            self.json_attributes_info = {}
        if self.static_info_generic_en is None:
            self.static_info_generic_en = {}
        if self.static_info_homeassistant_en is None:
            self.static_info_homeassistant_en = {}

    def get_json_len(self):
        # Estimate max length of the availability section
        avail_len = 0
        for entry in self.availability:
            # Each entry contributes: {"topic":"<prefix>/<topic>","value_template":"<template>"}
            # Prefix is at most ~20 chars, plus overhead
            avail_len += len(entry.topic) + len(entry.value_template) + 80
        return (
                max(len(self.name_de), len(self.name_en))
                + len(self.object_id)
                + len(self.path)
                + max(
            len(self.get_static_info_generic_str()),
            len(self.get_static_info_homeassistant_str()),
            len(self.get_static_info_generic_en_str()),
            len(self.get_static_info_homeassistant_en_str()),
        )
                + avail_len
                + len(self.get_json_attributes_info_str())
        )

    def get_json_attributes_info_str(self):
        return (
                '"'
                + json.dumps(self.json_attributes_info)
                .strip()
                .lstrip("{")
                .rstrip("}")
                .replace('"', '\\"')
                + '"'
        )

    def get_static_info_generic_str(self):
        if not self.include_generic:
            return "NULL"
        # static info is not a json object, but only more key value pairs, so remove the {}.
        # also this is a string literal, so escape inner ".
        return (
                '"'
                + json.dumps(self.static_info_generic)
                .strip()
                .lstrip("{")
                .rstrip("}")
                .replace('"', '\\"')
                + '"'
        )

    def get_static_info_homeassistant_str(self):
        return (
                '"'
                + json.dumps({**self.static_info_generic, **self.static_info_homeassistant})
                .strip()
                .lstrip("{")
                .rstrip("}")
                .replace('"', '\\"')
                + '"'
        )

    def get_static_info_generic_en_str(self):
        if not self.static_info_generic_en:
            return "NULL"
        if not self.include_generic:
            return "NULL"
        return (
                '"'
                + json.dumps({**self.static_info_generic, **self.static_info_generic_en})
                .strip()
                .lstrip("{")
                .rstrip("}")
                .replace('"', '\\"')
                + '"'
        )

    def get_static_info_homeassistant_en_str(self):
        if not self.static_info_homeassistant_en:
            return "NULL"
        # Merge: generic base + generic_en overrides + homeassistant base + homeassistant_en overrides
        merged = {
            **self.static_info_generic,
            **self.static_info_generic_en,
            **self.static_info_homeassistant,
            **self.static_info_homeassistant_en,
        }
        return (
                '"'
                + json.dumps(merged)
                .strip()
                .lstrip("{")
                .rstrip("}")
                .replace('"', '\\"')
                + '"'
        )

    def get_availability_entries_str(self):
        """Generate C initializer list entries for availability array."""
        if not self.availability:
            return ""
        entries = []
        for entry in self.availability:
            escaped_template = entry.value_template.replace('"', '\\"')
            entries.append(f'{{"{entry.topic}", "{escaped_template}"}}')
        return ", ".join(entries)


topic_template = """    {{
        .feature = "{feature}",
        .path = "{path}",
        .component = "{component}",
        .object_id = "{object_id}",
        .name_de = "{name_de}",
        .name_en = "{name_en}",
        .availability = {{{availability_entries}}},
        .availability_count = {availability_count},
        .json_attributes_topic = "{json_attributes_topic}",
        .json_attributes_info = {json_attributes_info},
        .static_infos = {{
            {static_info_generic},
            {static_info_homeassistant}
        }},
        .static_infos_en = {{
            {static_info_generic_en},
            {static_info_homeassistant_en}
        }},
        .type = MqttDiscoveryType::{discovery_type},
        .check_type = MqttDiscoveryCheckType::{check_type},
        .api_check_path = {api_check_path},
        .api_check_key = {api_check_key},
        .meter_value_id = {meter_value_id},
        .value_template_fmt = {value_template_fmt},
    }}"""

entities = [
    # Entity(
    #     include_generic=True,
    #     component=Component.SENSOR,
    #     feature=Feature.METER,
    #     object_id="powernow",
    #     path="meter/values",
    #     name_de="Leistungsaufnahme",
    #     name_en="Power draw",
    #     availability=[],
    #     static_info_generic={
    #         "value_template": "{{value_json.power | round(0)}}",
    #         "unit_of_measurement": "W",
    #         "device_class": "power",
    #         "state_class": "measurement",
    #     },
    #     static_info_homeassistant={},
    # ),
    # Entity(
    #     include_generic=True,
    #     component=Component.SENSOR,
    #     feature=Feature.METER,
    #     object_id="energyabs",
    #     path="meter/values",
    #     name_de="Stromverbrauch absolut",
    #     name_en="Energy consumption (absolute)",
    #     availability=[],
    #     static_info_generic={
    #         "value_template": "{{value_json.energy_abs | round(3)}}",
    #         "unit_of_measurement": "kWh",
    #         "device_class": "energy",
    #         "state_class": "total",
    #     },
    #     static_info_homeassistant={},
    # ),
    # Entity(
    #     include_generic=True,
    #     component=Component.SENSOR,
    #     feature=Feature.METER,
    #     object_id="energyrel",
    #     path="meter/values",
    #     name_de="Stromverbrauch relativ",
    #     name_en="Energy consumption (relative)",
    #     availability=[],
    #     static_info_generic={
    #         "value_template": "{{value_json.energy_rel | round(3)}}",
    #         "unit_of_measurement": "kWh",
    #         "device_class": "energy",
    #         "state_class": "total",
    #     },
    #     static_info_homeassistant={},
    # ),
    # Entity(
    #     include_generic=True,
    #     component=Component.SENSOR,
    #     feature=Feature.METER_PHASES,
    #     object_id="current_l1",
    #     path="meter/all_values",
    #     name_de="Strom L1",
    #     name_en="Current L1",
    #     availability=[],
    #     static_info_generic={
    #         "value_template": "{{value_json[3] | round(3)}}",
    #         "unit_of_measurement": "A",
    #         "device_class": "current",
    #         "state_class": "measurement",
    #     },
    #     static_info_homeassistant={},
    # ),
    # Entity(
    #     include_generic=True,
    #     component=Component.SENSOR,
    #     feature=Feature.METER_PHASES,
    #     object_id="current_l2",
    #     path="meter/all_values",
    #     name_de="Strom L2",
    #     name_en="Current L2",
    #     availability=[],
    #     static_info_generic={
    #         "value_template": "{{value_json[4] | round(3)}}",
    #         "unit_of_measurement": "A",
    #         "device_class": "current",
    #         "state_class": "measurement",
    #     },
    #     static_info_homeassistant={},
    # ),
    # Entity(
    #     include_generic=True,
    #     component=Component.SENSOR,
    #     feature=Feature.METER_PHASES,
    #     object_id="current_l3",
    #     path="meter/all_values",
    #     name_de="Strom L3",
    #     name_en="Current L3",
    #     availability=[],
    #     static_info_generic={
    #         "value_template": "{{value_json[5] | round(3)}}",
    #         "unit_of_measurement": "A",
    #         "device_class": "current",
    #         "state_class": "measurement",
    #     },
    #     static_info_homeassistant={},
    # ),
    Entity(
        include_generic=True,
        component=Component.SENSOR,
        feature=Feature.EVSE,
        object_id="chargerstate",
        path="evse/state",
        name_de="Ladestatus",
        name_en="Charge state",
        availability=[],
        static_info_generic={
            "value_template": "{{value_json.charger_state}}",
            "icon": "mdi:ev-plug-type2",
        },
        static_info_homeassistant={},
    ),
    Entity(
        include_generic=False,
        component=Component.SENSOR,
        feature=Feature.EVSE,
        object_id="current_charge_mode_evse",
        path="evse/charge_mode",
        name_de="Aktueller Lademodus EVSE",
        name_en="Current charge mode EVSE",
        availability=[],
        static_info_generic={
            "icon": "mdi:ev-station",
            "device_class": "enum",
        },
        static_info_homeassistant={
            "value_template": enum_value_template("mode", charge_mode_names_de),
            "options": charge_mode_names_de,
        },
        static_info_homeassistant_en={
            "value_template": enum_value_template("mode", charge_mode_names_en),
            "options": charge_mode_names_en,
        },
        check_type=CheckType.FEATURE,
    ),
    Entity(
        include_generic=True,
        component=Component.NUMBER,
        feature=Feature.EVSE,
        object_id="globalcurrent",
        path="evse/external_current",
        name_de="Ladestromlimit",
        name_en="charging current limit",
        availability=[MQTT_NOT_READ_ONLY],
        static_info_generic={
            "value_template": "{{(value_json.current | float / 1000) | round(3)}}",
            "command_template": "{{ (value * 1000) | int }}",
            "unit_of_measurement": "A",
            "min": 0,
            "max": 32,
            "mode": "auto",
            "icon": "mdi:gauge",
        },
        static_info_homeassistant={},
    ),
    Entity(
        include_generic=True,
        component=Component.SENSOR,
        feature=Feature.EVSE,
        object_id="allowedcurrent",
        path="evse/state",
        name_de="Erlaubter Ladestrom",
        name_en="Allowed charging current",
        availability=[],
        static_info_generic={
            "value_template": "{{(value_json.allowed_charging_current | float / 1000) | round(3)}}",
            "unit_of_measurement": "A",
            "device_class": "current",
            "state_class": "measurement",
            "min": 0,
            "max": 32,
        },
        static_info_homeassistant={},
    ),
    Entity(
        include_generic=True,
        component=Component.BUTTON,
        feature=Feature.EVSE,
        object_id="startcharge",
        path="evse/start_charging",
        name_de="Ladevorgang starten",
        name_en="Start charging",
        availability=[MQTT_NOT_READ_ONLY],
        static_info_generic={
            "payload_press": "null",  # for Home Assistant
            "payload_on": "null",  # for Domoticz
            "icon": "mdi:flash",
        },
        static_info_homeassistant={},
    ),
    Entity(
        include_generic=True,
        component=Component.BUTTON,
        feature=Feature.EVSE,
        object_id="stopcharge",
        path="evse/stop_charging",
        name_de="Ladevorgang beenden",
        name_en="Stop charging",
        availability=[MQTT_NOT_READ_ONLY],
        static_info_generic={
            "payload_press": "null",  # for Home Assistant
            "payload_on": "null",  # for Domoticz
            "icon": "mdi:flash-off",
        },
        static_info_homeassistant={},
    ),
    Entity(
        include_generic=False,
        component=Component.BINARY_SENSOR,
        feature=Feature.EVSE,
        object_id="cable",
        path="evse/state",
        name_de="Wallbox-Ladekabel verbunden",
        name_en="Charger cable connected",
        availability=[],
        static_info_generic={
            "device_class": "plug",
        },
        static_info_homeassistant={
            "value_template": "{{value_json.charger_state in [1, 2, 3]}}",
            "payload_on": "True",
            "payload_off": "False",
        },
    ),
    Entity(
        include_generic=False,
        component=Component.BINARY_SENSOR,
        feature=Feature.EVSE,
        object_id="ready",
        path="evse/state",
        name_de="Wallbox ladebereit",
        name_en="Charger ready to charge",
        availability=[],
        static_info_generic={
            "device_class": "power",
        },
        static_info_homeassistant={
            "value_template": "{{value_json.charger_state in [2, 3]}}",
            "payload_on": "True",
            "payload_off": "False",
        },
    ),
    Entity(
        include_generic=False,
        component=Component.BINARY_SENSOR,
        feature=Feature.EVSE,
        object_id="charging",
        path="evse/state",
        name_de="Wallbox lädt",
        name_en="Charger charging",
        availability=[],
        static_info_generic={
            "device_class": "battery_charging",
        },
        static_info_homeassistant={
            "value_template": "{{value_json.charger_state in [3]}}",
            "payload_on": "True",
            "payload_off": "False",
        },
    ),
    Entity(
        include_generic=False,
        component=Component.BINARY_SENSOR,
        feature=Feature.EVSE,
        object_id="error",
        path="evse/state",
        name_de="Wallbox-Fehler",
        name_en="Charger error",
        availability=[],
        static_info_generic={
            "device_class": "problem",
        },
        static_info_homeassistant={
            "value_template": "{{value_json.charger_state in [4]}}",
            "payload_on": "True",
            "payload_off": "False",
        },
    ),
    Entity(
        include_generic=False,
        component=Component.BINARY_SENSOR,
        feature=Feature.EVSE,
        object_id="online",
        path="evse/low_level_state",
        name_de="Wallbox verfügbar",
        name_en="Charger online",
        availability=[],
        static_info_generic={
            "device_class": "connectivity",
            "expire_after": "30",
        },
        static_info_homeassistant={
            "value_template": "{{value_json.uptime>0}}",
            "payload_on": "True",
            "payload_off": "False",
        },
    ),
    Entity(
        include_generic=False,
        component=Component.SENSOR,
        object_id="limited",
        path="p14a_enwg/state",
        name_de="Limitiert nach §14a ENWG",
        name_en="Limited according to §14a EnWG",
        availability=[],
        static_info_generic={
            "device_class": "enum",
        },
        static_info_homeassistant={
            "value_template": "{{ 'Limitiert' if value_json.active else 'Kein Limit' }}",
            "options": ["Limitiert", "Kein Limit"],
            "icon": "mdi:transmission-tower-export",
        },
        static_info_homeassistant_en={
            "value_template": "{{ 'Limited' if value_json.active else 'No Limit' }}",
            "options": ["Limited", "No Limit"],
        },
        check_type=CheckType.API_BOOL,
        api_check_path="p14a_enwg/config",
        api_check_key="enable",
    ),
    Entity(
        include_generic=False,
        component=Component.SELECT,
        object_id="active_charge_mode",
        path="power_manager/charge_mode",
        name_de="Aktiver Lademodus",
        name_en="Active charge mode",
        availability=[
            MQTT_NOT_READ_ONLY,
            AvailabilityEntry("power_manager/config", "{{ 'online' if value_json.enabled else 'offline' }}"),
        ],
        static_info_generic={
            "icon": "mdi:ev-station",
        },
        static_info_homeassistant={
            "value_template": enum_value_template("mode", charge_mode_names_de),
            "command_template": command_template_for_select(charge_mode_names_de),
            "options": charge_mode_names_de,
        },
        static_info_homeassistant_en={
            "value_template": enum_value_template("mode", charge_mode_names_en),
            "command_template": command_template_for_select(charge_mode_names_en),
            "options": charge_mode_names_en,
        },
        check_type=CheckType.API_BOOL,
        api_check_path="power_manager/config",
        api_check_key="enabled",
    ),
    Entity(
        include_generic=False,
        component=Component.SENSOR,
        object_id="current_charge_mode",
        path="power_manager/charge_mode",
        name_de="Aktueller Lademodus",
        name_en="Current charge mode",
        availability=[
            AvailabilityEntry("power_manager/config", "{{ 'online' if value_json.enabled else 'offline' }}"),
        ],
        static_info_generic={
            "icon": "mdi:ev-station",
            "device_class": "enum",
        },
        static_info_homeassistant={
            "value_template": enum_value_template("mode", charge_mode_names_de),
            "options": charge_mode_names_de,
        },
        static_info_homeassistant_en={
            "value_template": enum_value_template("mode", charge_mode_names_en),
            "options": charge_mode_names_en,
        },
        check_type=CheckType.API_BOOL,
        api_check_path="power_manager/config",
        api_check_key="enabled",
    ),
    Entity(
        include_generic=False,
        component=Component.SENSOR,
        object_id="solar_forecast_tomorrow",
        path="solar_forecast/state",
        name_de="PV Ertragsprognose morgen",
        name_en="Solar Forecast tomorrow",
        availability=[AvailabilityEntry("solar_forecast/config", "{{ 'online' if value_json.enable else 'offline' }}")],
        static_info_generic={
            "device_class": "energy",
        },
        static_info_homeassistant={
            "value_template": "{{(value_json.wh_tomorrow | float / 1000) | round(2)}}",
            "icon": "mdi:solar-power-variant-outline",
            "unit_of_measurement": "kWh",
        },
        check_type=CheckType.API_BOOL,
        api_check_path="solar_forecast/config",
        api_check_key="enable",
    ),
    Entity(
        include_generic=False,
        component=Component.SENSOR,
        object_id="solar_forecast_today",
        path="solar_forecast/state",
        name_de="PV Ertragsprognose heute",
        name_en="Solar Forecast today",
        availability=[AvailabilityEntry("solar_forecast/config", "{{ 'online' if value_json.enable else 'offline' }}")],
        static_info_generic={
            "device_class": "energy",
        },
        static_info_homeassistant={
            "value_template": "{{(value_json.wh_today | float / 1000) | round(2)}}",
            "icon": "mdi:solar-power-variant",
            "unit_of_measurement": "kWh",
        },
        check_type=CheckType.API_BOOL,
        api_check_path="solar_forecast/config",
        api_check_key="enable",
    ),
    Entity(
        include_generic=False,
        component=Component.SENSOR,
        object_id="solar_forecast_outstanding",
        path="solar_forecast/state",
        name_de="PV Ertragsprognose ab jetzt",
        name_en="Solar Forecast from now",
        availability=[AvailabilityEntry("solar_forecast/config", "{{ 'online' if value_json.enable else 'offline' }}")],
        static_info_generic={
            "device_class": "energy",
        },
        static_info_homeassistant={
            "value_template": "{{(value_json.wh_today_remaining | float / 1000) | round(2)}}",
            "icon": "mdi:solar-power",
            "unit_of_measurement": "kWh",
        },
        check_type=CheckType.API_BOOL,
        api_check_path="solar_forecast/config",
        api_check_key="enable",
    ),
    Entity(
        include_generic=False,
        component=Component.SENSOR,
        object_id="current_electricity_price",
        path="day_ahead_prices/state",
        name_de="Börsenstrompreis",
        name_en="Electricity market price",
        availability=[AvailabilityEntry("day_ahead_prices/config", "{{ 'online' if value_json.enable else 'offline' }}")],
        static_info_generic={
            "device_class": "monetary",
        },
        static_info_homeassistant={
            "value_template": "{{(value_json.current_price | float / 1000) | round(2)}}",
            "icon": "mdi:solar-power",
            "unit_of_measurement": "ct/kWh",
        },
        check_type=CheckType.API_BOOL,
        api_check_path="day_ahead_prices/config",
        api_check_key="enable",
    ),
    Entity(
        include_generic=False,
        component=Component.BINARY_SENSOR,
        feature=Feature.EVSE,
        object_id="evse_button_pressed",
        path="evse/button_state",
        name_de="Fronttaster gedrückt",
        name_en="Front button pressed",
        availability=[],
        static_info_generic={},
        static_info_homeassistant={
            "value_template": "{{value_json.button_pressed}}",
            "payload_on": "True",
            "payload_off": "False",
        },
    ),
]


# meter value definitions, derived from meters/meter_value_id.csv.
# Each tuple: (object_id_suffix, name_de, name_en, meter_value_id, rounding, unit, device_class, state_class)
meter_value_entries = load_meter_value_entries(METER_VALUE_IDS)

for meter_id in range(0, meters_max_slots):
    for suffix, name_de, name_en, value_id, rounding, unit, dev_class, state_class in meter_value_entries:
        # The value_template_fmt contains a %d placeholder that the C++ code replaces
        # with the resolved index from meters/N/value_ids at runtime.
        value_template_fmt = "{{value_json[%d] | round(" + str(rounding) + ")}}"
        static_info = {
            "unit_of_measurement": unit,
            "state_class": state_class,
        }
        if dev_class is not None:
            static_info["device_class"] = dev_class
        entities.append(
            Entity(
                include_generic=True,
                component=Component.SENSOR,
                feature=Feature.METERS,
                object_id=f"meter_{meter_id}_{suffix}",
                path=f"meters/{meter_id}/values",
                name_de=f"{name_de} Zähler {meter_id}",
                name_en=f"{name_en} meter {meter_id}",
                availability=[
                    AvailabilityEntry(f"meters/{meter_id}/config", "{{ 'offline' if value_json[0] == 0 else 'online' }}")],
                static_info_generic=static_info,
                static_info_homeassistant={},
                check_type=CheckType.METER_VALUE,
                api_check_path=f"meters/{meter_id}/config",
                meter_value_id=value_id,
                value_template_fmt=value_template_fmt,
            )
        )

topics = [
    topic_template.format(
        feature=x.feature.value,
        path=x.path,
        component=x.component.value,
        object_id=x.object_id,
        name_de=x.name_de,
        name_en=x.name_en,
        availability_entries=x.get_availability_entries_str(),
        availability_count=len(x.availability),
        json_attributes_topic=x.json_attributes_topic,
        json_attributes_info=x.get_json_attributes_info_str(),
        static_info_generic=x.get_static_info_generic_str(),
        static_info_homeassistant=x.get_static_info_homeassistant_str(),
        static_info_generic_en=x.get_static_info_generic_en_str(),
        static_info_homeassistant_en=x.get_static_info_homeassistant_en_str(),
        discovery_type=x.component.get_discovery_type().value,
        check_type=x.check_type.value,
        api_check_path='"%s"' % x.api_check_path if x.api_check_path else "NULL",
        api_check_key='"%s"' % x.api_check_key if x.api_check_key else "NULL",
        meter_value_id=x.meter_value_id,
        value_template_fmt='"%s"' % x.value_template_fmt.replace('"', '\\"') if x.value_template_fmt else "NULL",
    )
    for x in entities
]

cpp = tfutil.specialize_template(
    "mqtt_discovery_topics.cpp.template",
    None,
    {
        "{{{topics}}}": ",\n".join(topics),
    },
)

util.write_generated_file("generated/mqtt_discovery_topics.cpp", cpp)

h = tfutil.specialize_template(
    "mqtt_discovery_topics.h.template",
    None,
    {
        "{{{topic_count}}}": str(len(topics)),
        "{{{max_json_len}}}": str(max([x.get_json_len() for x in entities])),
        "{{{max_availability_count}}}": str(max([len(x.availability) for x in entities])),
    },
)

util.write_generated_file("generated/mqtt_discovery_topics.h", h)
