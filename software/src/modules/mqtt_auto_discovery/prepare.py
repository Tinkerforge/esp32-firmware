from enum import Enum
from dataclasses import dataclass
import json
import os
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")

from software import util

meters_max_slots = util.get_env_metadata()['options']['meters_max_slots']
charge_mode_names = [x.name.space for x in util.parse_enum_spec(os.path.join(os.path.dirname(os.path.realpath(__file__)),
                                                                             '..', 'cm_networking', 'Config Charge Mode.uint8.enum'))[2]]


def enum_value_template(json_field, names):
    """Generate a Jinja value_template that maps an integer enum field to its name.
    Uses a Jinja `{% set map = {0: 'A', 1: 'B', ...} %}` lookup with a fallback to 'Unknown'."""
    mapping = ", ".join(f"{i}: '{name}'" for i, name in enumerate(names))
    return (
        "{{% set m = {{ {mapping} }} %}}"
        "{{{{ m.get(value_json.{field}, 'Unknown') }}}}"
    ).format(mapping=mapping, field=json_field)


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

    def get_discovery_type(self) -> DiscoveryType:
        return {
            Component.BINARY_SENSOR: DiscoveryType.STATE_ONLY,
            Component.SENSOR: DiscoveryType.STATE_ONLY,
            Component.SWITCH: DiscoveryType.STATE_AND_UPDATE,
            Component.NUMBER: DiscoveryType.STATE_AND_UPDATE,
            Component.BUTTON: DiscoveryType.COMMAND_ONLY,
        }[self]


class Feature(Enum):
    EVSE = "evse"
    METER = "meter"
    METER_PHASES = "meter_phases"
    P14A_ENWG = "p14a_enwg"
    METERS = "meters"
    POWER_MANAGER = "power_manager"
    SOLAR_FORECAST = "solar_forecast"


@dataclass
class Entity:
    include_generic: bool
    component: Component
    feature: Feature
    object_id: str
    path: str
    name_de: str
    name_en: str
    availability_topic: str
    availability_info: dict
    static_info_generic: dict
    static_info_homeassistant: dict
    json_attributes_topic: str = ""
    json_attributes_info: dict = None

    def __post_init__(self):
        if self.json_attributes_info is None:
            self.json_attributes_info = {}

    def get_json_len(self):
        return (
            max(len(self.name_de), len(self.name_en))
            + len(self.object_id)
            + len(self.path)
            + max(
                len(self.get_static_info_generic_str()),
                len(self.get_static_info_homeassistant_str()),
            )
            + len(self.get_availability_info_str())
            + len(self.get_json_attributes_info_str())
        )

    def get_availability_info_str(self):
        return (
            '"'
            + json.dumps(self.availability_info)
            .strip()
            .lstrip("{")
            .rstrip("}")
            .replace('"', '\\"')
            + '"'
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




topic_template = """    {{
        .feature = "{feature}",
        .path = "{path}",
        .component = "{component}",
        .object_id = "{object_id}",
        .name_de = "{name_de}",
        .name_en = "{name_en}",
        .availability_topic = "{availability_topic}",
        .availability_info = {availability_info},
        .json_attributes_topic = "{json_attributes_topic}",
        .json_attributes_info = {json_attributes_info},
        .static_infos = {{
            {static_info_generic},
            {static_info_homeassistant}
        }},
        .type = MqttDiscoveryType::{discovery_type},
    }}"""

entities = [
    Entity(
        include_generic=True,
        component=Component.SENSOR,
        feature=Feature.METER,
        object_id="powernow",
        path="meter/values",
        name_de="Leistungsaufnahme",
        name_en="Power draw",
        availability_topic="",
        availability_info={},
        static_info_generic={
            "value_template": "{{value_json.power | round(0)}}",
            "unit_of_measurement": "W",
            "device_class": "power",
            "state_class": "measurement",
        },
        static_info_homeassistant={},
    ),
    Entity(
        include_generic=True,
        component=Component.SENSOR,
        feature=Feature.METER,
        object_id="energyabs",
        path="meter/values",
        name_de="Stromverbrauch absolut",
        name_en="Energy consumption (absolute)",
        availability_topic="",
        availability_info={},
        static_info_generic={
            "value_template": "{{value_json.energy_abs | round(3)}}",
            "unit_of_measurement": "kWh",
            "device_class": "energy",
            "state_class": "total",
        },
        static_info_homeassistant={},
    ),
    Entity(
        include_generic=True,
        component=Component.SENSOR,
        feature=Feature.METER,
        object_id="energyrel",
        path="meter/values",
        name_de="Stromverbrauch relativ",
        name_en="Energy consumption (relative)",
        availability_topic="",
        availability_info={},
        static_info_generic={
            "value_template": "{{value_json.energy_rel | round(3)}}",
            "unit_of_measurement": "kWh",
            "device_class": "energy",
            "state_class": "total",
        },
        static_info_homeassistant={},
    ),
    Entity(
        include_generic=True,
        component=Component.SENSOR,
        feature=Feature.METER_PHASES,
        object_id="current_l1",
        path="meter/all_values",
        name_de="Strom L1",
        name_en="Current L1",
        availability_topic="",
        availability_info={},
        static_info_generic={
            "value_template": "{{value_json[3] | round(3)}}",
            "unit_of_measurement": "A",
            "device_class": "current",
            "state_class": "measurement",
        },
        static_info_homeassistant={},
    ),
    Entity(
        include_generic=True,
        component=Component.SENSOR,
        feature=Feature.METER_PHASES,
        object_id="current_l2",
        path="meter/all_values",
        name_de="Strom L2",
        name_en="Current L2",
        availability_topic="",
        availability_info={},
        static_info_generic={
            "value_template": "{{value_json[4] | round(3)}}",
            "unit_of_measurement": "A",
            "device_class": "current",
            "state_class": "measurement",
        },
        static_info_homeassistant={},
    ),
    Entity(
        include_generic=True,
        component=Component.SENSOR,
        feature=Feature.METER_PHASES,
        object_id="current_l3",
        path="meter/all_values",
        name_de="Strom L3",
        name_en="Current L3",
        availability_topic="",
        availability_info={},
        static_info_generic={
            "value_template": "{{value_json[5] | round(3)}}",
            "unit_of_measurement": "A",
            "device_class": "current",
            "state_class": "measurement",
        },
        static_info_homeassistant={},
    ),
    Entity(
        include_generic=True,
        component=Component.SENSOR,
        feature=Feature.EVSE,
        object_id="chargerstate",
        path="evse/state",
        name_de="Ladestatus",
        name_en="Charge state",
        availability_topic="",
        availability_info={},
        static_info_generic={
            "value_template": "{{value_json.charger_state}}",
            "icon": "mdi:ev-plug-type2",
        },
        static_info_homeassistant={},
    ),
    Entity(
        include_generic=True,
        component=Component.NUMBER,
        feature=Feature.EVSE,
        object_id="globalcurrent",
        path="evse/external_current",
        name_de="Ladestromlimit",
        name_en="charging current limit",
        availability_topic="",
        availability_info={},
        static_info_generic={
            "value_template": "{{(value_json.current | float / 1000) | round(3)}}",
            "command_template": "{{ value * 1000 }}",
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
        availability_topic="",
        availability_info={},
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
        availability_topic="",
        availability_info={},
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
        availability_topic="",
        availability_info={},
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
        availability_topic="",
        availability_info={},
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
        availability_topic="",
        availability_info={},
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
        availability_topic="",
        availability_info={},
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
        availability_topic="",
        availability_info={},
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
        availability_topic="",
        availability_info={},
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
        feature=Feature.P14A_ENWG,
        object_id="limited",
        path="p14a_enwg/state",
        name_de="Limitiert nach §14a ENWG",
        name_en="Limited according to §14a EnWG",
        availability_topic="",
        availability_info={},
        static_info_generic={
            "device_class": "enum",
        },
        static_info_homeassistant={
            "value_template": "{{ 'Limitiert' if value_json.active else 'Kein Limit' }}",
            "options": ["Limitiert", "Kein Limit"],
            "icon": "mdi:transmission-tower-export",
        },
    ),
    Entity(
        include_generic=False,
        component=Component.SENSOR,
        feature=Feature.POWER_MANAGER,
        object_id="active_charge_mode",
        path="power_manager/charge_mode",
        name_de="Aktiver Lademodus",
        name_en="Active charge mode",
        availability_topic="",
        availability_info={},
        static_info_generic={
            "device_class": "enum",
        },
        static_info_homeassistant={
            "value_template": enum_value_template("mode", charge_mode_names),
            "options": charge_mode_names + ["Unknown"],
            "icon": "mdi:ev-station",
        },
    ),
    Entity(
        include_generic=False,
        component=Component.SENSOR,
        feature=Feature.SOLAR_FORECAST,
        object_id="solar_forecast_tomorrow",
        path="solar_forecast/state",
        name_de="PV Ertragsprognose morgen",
        name_en="Solar Forecast tomorrow",
        availability_topic="solar_forecast/config",
        availability_info={"availability_template": "{{ 'online' if value_json.enable else 'offline' }}"},
        static_info_generic={
            "device_class": "energy",
        },
        static_info_homeassistant={
            "value_template":"{{(value_json.wh_tomorrow | float / 1000) | round(2)}}",
            "icon": "mdi:solar-power-variant-outline",
            "unit_of_measurement": "kWh",
        },
    ),
    Entity(
        include_generic=False,
        component=Component.SENSOR,
        feature=Feature.SOLAR_FORECAST,
        object_id="solar_forecast_today",
        path="solar_forecast/state",
        name_de="PV Ertragsprognose heute",
        name_en="Solar Forecast today",
        availability_topic="solar_forecast/config",
        availability_info={"availability_template": "{{ 'online' if value_json.enable else 'offline' }}"},
        static_info_generic={
            "device_class": "energy",
        },
        static_info_homeassistant={
            "value_template": "{{(value_json.wh_today | float / 1000) | round(2)}}",
            "icon": "mdi:solar-power-variant",
            "unit_of_measurement": "kWh",
        },
    ),
Entity(
        include_generic=False,
        component=Component.SENSOR,
        feature=Feature.SOLAR_FORECAST,
        object_id="solar_forecast_outstanding",
        path="solar_forecast/state",
        name_de="PV Ertragsprognose ab jetzt",
        name_en="Solar Forecast from now",
        availability_topic="solar_forecast/config",
        availability_info={"availability_template": "{{ 'online' if value_json.enable else 'offline' }}"},
        static_info_generic={
            "device_class": "energy",
        },
        static_info_homeassistant={
            "value_template": "{{(value_json.wh_today_remaining | float / 1000) | round(2)}}",
            "icon": "mdi:solar-power",
            "unit_of_measurement": "kWh",
        },
    ),
]
# MeterValueID constants (from generated/meter_value_id.h)
METER_VALUE_ID_POWER_ACTIVE_L_SUM_IM_EX_DIFF = 74
METER_VALUE_ID_ENERGY_ACTIVE_L_SUM_IM_EX_SUM = 213
METER_VALUE_ID_ENERGY_ACTIVE_L_SUM_IM_EX_SUM_RESETTABLE = 214


def meter_value_template(value_id, digits):
    """Generate a Jinja value_template that looks up a MeterValueID in the value_ids attribute (populated via json_attributes_topic from meters/{id}/value_ids) and uses the found index to extract the value from the values array."""
    return (
        "{{% set ids = this.attributes.get('ids', none) %}}"
        "{{% if ids != none and {vid} in ids %}}"
        "{{{{ value_json[ids.index({vid})] | round({digits}) }}}}"
        "{{% else %}}None{{% endif %}}"
    ).format(vid=value_id, digits=digits)


for meter_id in range(0, meters_max_slots):
    # Power
    entities.append(
        Entity(
            include_generic=True,
            component=Component.SENSOR,
            feature=Feature.METERS,
            object_id=f"meter_{meter_id}_powernow",
            path=f"meters/{meter_id}/values",
            name_de=f"Leistungsaufnahme Stromzähler {meter_id}",
            name_en=f"Power draw meter {meter_id}",
            availability_topic=f"meters/{meter_id}/config",
            availability_info={"availability_template": "{{ 'offline' if value_json[0] == 0 else 'online' }}"},
            static_info_generic={
                "value_template": meter_value_template(
                    METER_VALUE_ID_POWER_ACTIVE_L_SUM_IM_EX_DIFF, 0
                ),
                "unit_of_measurement": "W",
                "device_class": "power",
                "state_class": "measurement",
            },
            static_info_homeassistant={},
            json_attributes_topic=f"meters/{meter_id}/value_ids",
            json_attributes_info={
                "json_attributes_template": "{{ {'ids': value_json} | tojson }}"
            },
        )
    )
    # Energy absolute
    entities.append(
        Entity(
            include_generic=True,
            component=Component.SENSOR,
            feature=Feature.METERS,
            object_id=f"meter_{meter_id}_energyabs",
            path=f"meters/{meter_id}/values",
            name_de=f"Stromverbrauch Stromzähler {meter_id} absolut",
            name_en=f"Energy consumption meter {meter_id} (absolute)",
            availability_topic=f"meters/{meter_id}/config",
            availability_info={"availability_template": "{{ 'offline' if value_json[0] == 0 else 'online' }}"},
            static_info_generic={
                "value_template": meter_value_template(
                    METER_VALUE_ID_ENERGY_ACTIVE_L_SUM_IM_EX_SUM, 3
                ),
                "unit_of_measurement": "kWh",
                "device_class": "energy",
                "state_class": "total",
            },
            static_info_homeassistant={},
            json_attributes_topic=f"meters/{meter_id}/value_ids",
            json_attributes_info={
                "json_attributes_template": "{{ {'ids': value_json} | tojson }}"
            },
        )
    )
    # Energy relative
    entities.append(
        Entity(
            include_generic=True,
            component=Component.SENSOR,
            feature=Feature.METERS,
            object_id=f"meter_{meter_id}_energyrel",
            path=f"meters/{meter_id}/values",
            name_de=f"Stromverbrauch Stromzähler {meter_id} relativ",
            name_en=f"Energy consumption meter {meter_id} (relative)",
            availability_topic=f"meters/{meter_id}/config",
            availability_info={"availability_template": "{{ 'offline' if value_json[0] == 0 else 'online' }}"},
            static_info_generic={
                "value_template": meter_value_template(
                    METER_VALUE_ID_ENERGY_ACTIVE_L_SUM_IM_EX_SUM_RESETTABLE, 3
                ),
                "unit_of_measurement": "kWh",
                "device_class": "energy",
                "state_class": "total",
            },
            static_info_homeassistant={},
            json_attributes_topic=f"meters/{meter_id}/value_ids",
            json_attributes_info={
                "json_attributes_template": "{{ {'ids': value_json} | tojson }}"
            },
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
        availability_topic=x.availability_topic,
        availability_info=x.get_availability_info_str(),
        json_attributes_topic=x.json_attributes_topic,
        json_attributes_info=x.get_json_attributes_info_str(),
        static_info_generic=x.get_static_info_generic_str(),
        static_info_homeassistant=x.get_static_info_homeassistant_str(),
        discovery_type=x.component.get_discovery_type().value,
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
    },
)

util.write_generated_file("generated/mqtt_discovery_topics.h", h)
