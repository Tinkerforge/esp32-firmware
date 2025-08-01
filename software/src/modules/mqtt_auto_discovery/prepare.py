from enum import Enum
from dataclasses import dataclass
import json
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

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
            Component.BUTTON: DiscoveryType.COMMAND_ONLY
        }[self]

class Feature(Enum):
    EVSE = "evse"
    METER = "meter"
    METER_PHASES = "meter_phases"

@dataclass
class Entity:
    include_generic: bool
    component: Component
    feature: Feature
    object_id: str
    path: str
    name_de: str
    name_en: str
    availability_path: str
    availability_yes: str
    availability_no: str
    static_info_generic: dict
    static_info_homeassistant: dict

    def get_json_len(self):
        return max(len(self.name_de), len(self.name_en)) + len(self.object_id) + len(self.path) + max(len(self.get_static_info_generic_str()), len(self.get_static_info_homeassistant_str()))

    def get_static_info_generic_str(self):
        if not self.include_generic:
            return "NULL"
        # static info is not a json object, but only more key value pairs, so remove the {}.
        # also this is a string literal, so escape inner ".
        return "\"" + json.dumps(self.static_info_generic).strip().lstrip("{").rstrip("}").replace('"', '\\"') + "\""

    def get_static_info_homeassistant_str(self):
        return "\"" + json.dumps({**self.static_info_generic, **self.static_info_homeassistant}).strip().lstrip("{").rstrip("}").replace('"', '\\"') + "\""

topic_template = """    {{
        .feature = "{feature}",
        .path = "{path}",
        .component = "{component}",
        .object_id = "{object_id}",
        .name_de = "{name_de}",
        .name_en = "{name_en}",
        .availability_path = "{availability_path}",
        .availability_yes = "{availability_yes}",
        .availability_no = "{availability_no}",
        .static_infos = {{
            {static_info_generic},
            {static_info_homeassistant}
        }},
        .type = MqttDiscoveryType::{discovery_type},
    }}"""






entities = [

Entity(True, Component.SENSOR, Feature.METER, "powernow", "meter/values", "Leistungsaufnahme", "Power draw", "", "", "",
    {"value_template":"{{value_json.power | round(0)}}",
     "unit_of_measurement":"W",
     "device_class":"power",
     "state_class": "measurement"},
    {}),

Entity(True, Component.SENSOR, Feature.METER, "energyabs", "meter/values", "Stromverbrauch absolut", "Energy consumption (absolute)", "", "", "",
    {"value_template":"{{value_json.energy_abs | round(3)}}",
     "unit_of_measurement":"kWh",
     "device_class":"energy",
     "state_class": "total"},
    {}),

Entity(True, Component.SENSOR, Feature.METER, "energyrel", "meter/values", "Stromverbrauch relativ", "Energy consumption (relative)", "", "", "",
    {"value_template":"{{value_json.energy_rel | round(3)}}",
     "unit_of_measurement":"kWh",
     "device_class":"energy",
     "state_class": "total"},
    {}),

Entity(True, Component.SENSOR, Feature.METER_PHASES, "current_l1", "meter/all_values", "Strom L1", "Current L1", "", "", "",
    {"value_template":"{{value_json[3] | round(3)}}",
     "unit_of_measurement":"A",
     "device_class":"current",
     "state_class": "measurement"},
    {}),

Entity(True, Component.SENSOR, Feature.METER_PHASES, "current_l2", "meter/all_values", "Strom L2", "Current L2", "", "", "",
    {"value_template":"{{value_json[4] | round(3)}}",
     "unit_of_measurement":"A",
     "device_class":"current",
     "state_class": "measurement"},
    {}),

Entity(True, Component.SENSOR, Feature.METER_PHASES, "current_l3", "meter/all_values", "Strom L3", "Current L3", "", "", "",
    {"value_template":"{{value_json[5] | round(3)}}",
     "unit_of_measurement":"A",
     "device_class":"current",
     "state_class": "measurement"},
    {}),

Entity(True, Component.SENSOR, Feature.EVSE, "chargerstate", "evse/state", "Ladestatus", "Charge state", "", "", "",
    {"value_template":"{{value_json.charger_state}}",
     "icon": "mdi:ev-plug-type2"},
    {}),

Entity(True, Component.NUMBER, Feature.EVSE, "globalcurrent", "evse/external_current", "Ladestromlimit", "charging current limit", "", "", "",
    {"value_template":"{{(value_json.current | float / 1000) | round(3)}}",
     "command_template": "{{ value * 1000 }}",
     "unit_of_measurement":"A",
     "min": 0,
     "max": 32,
     "mode": "auto",
     "icon": "mdi:gauge"},
    {}),

Entity(True, Component.SENSOR, Feature.EVSE, "allowedcurrent", "evse/state", "Erlaubter Ladestrom", "Allowed charging current", "", "", "",
    {"value_template":"{{(value_json.allowed_charging_current | float / 1000) | round(3)}}",
     "unit_of_measurement":"A",
     "device_class":"current",
     "state_class": "measurement",
     "min": 0,
     "max": 32},
    {}),

Entity(True, Component.BUTTON, Feature.EVSE, "startcharge", "evse/start_charging", "Ladevorgang starten", "Start charging", "", "", "",
    {"payload_press": "null",   # for Home Assistant
     "payload_on":"null",       # for Domoticz
     "icon": "mdi:flash"},
    {}),

Entity(True, Component.BUTTON, Feature.EVSE, "stopcharge", "evse/stop_charging", "Ladevorgang beenden", "Stop charging", "", "", "",
    {"payload_press": "null",   # for Home Assistant
     "payload_on":"null",       # for Domoticz
     "icon": "mdi:flash-off"},
    {}),

Entity(False, Component.BINARY_SENSOR, Feature.EVSE, "cable", "evse/state", "Wallbox-Ladekabel verbunden", "Charger cable connected", "", "", "",
    {"device_class":"plug"},
    {"value_template":"{{value_json.charger_state in [1, 2, 3]}}",
     "payload_on":"True",
     "payload_off":"False"}),

Entity(False, Component.BINARY_SENSOR, Feature.EVSE, "ready", "evse/state", "Wallbox ladebereit", "Charger ready to charge", "", "", "",
    {"device_class":"power"},
    {"value_template":"{{value_json.charger_state in [2, 3]}}",
     "payload_on":"True",
     "payload_off":"False"}),

Entity(False, Component.BINARY_SENSOR, Feature.EVSE, "charging", "evse/state", "Wallbox lädt", "Charger charging", "", "", "",
    {"device_class":"battery_charging"},
    {"value_template":"{{value_json.charger_state in [3]}}",
     "payload_on":"True",
     "payload_off":"False"}),

Entity(False, Component.BINARY_SENSOR, Feature.EVSE, "error", "evse/state", "Wallbox-Fehler", "Charger error", "", "", "",
    {"device_class":"problem"},
    {"value_template":"{{value_json.charger_state in [4]}}",
     "payload_on":"True",
     "payload_off":"False"}),

Entity(False, Component.BINARY_SENSOR, Feature.EVSE, "online", "evse/low_level_state", "Wallbox verfügbar", "Charger online", "", "", "",
    {"device_class":"connectivity",
     "expire_after": "30"},
    {"value_template":"{{value_json.uptime>0}}",
     "payload_on":"True",
     "payload_off":"False"}),
]

topics = [topic_template.format(
            feature=x.feature.value,
            path=x.path,
            component=x.component.value,
            object_id=x.object_id,
            name_de=x.name_de,
            name_en=x.name_en,
            availability_path=x.availability_path,
            availability_yes=x.availability_yes,
            availability_no=x.availability_no,
            static_info_generic=x.get_static_info_generic_str(),
            static_info_homeassistant=x.get_static_info_homeassistant_str(),
            discovery_type=x.component.get_discovery_type().value,
            )
          for x in entities]

tfutil.specialize_template("mqtt_discovery_topics.cpp.template", "mqtt_discovery_topics.cpp", {
    "{{{topics}}}": ",\n".join(topics),
    })

tfutil.specialize_template("mqtt_discovery_topics.h.template", "mqtt_discovery_topics.h", {
    "{{{topic_count}}}": str(len(topics)),
    "{{{max_json_len}}}": str(max([x.get_json_len() for x in entities]))
    })
