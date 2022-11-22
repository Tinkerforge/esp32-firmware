import os
import sys
import importlib.util
import importlib.machinery

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

from software import util

from enum import Enum
from dataclasses import dataclass
import json

class DiscoveryType(Enum):
    STATE_ONLY = "STATE_ONLY"
    STATE_AND_UPDATE = "STATE_AND_UPDATE"
    COMMAND_ONLY = "COMMAND_ONLY"

class Component(Enum):
    BINARY_SENSOR = "binary_sensor"
    SENSOR = "sensor"
    SWITCH = "switch"
    BUTTON = "button"

    def get_discovery_type(self) -> DiscoveryType:
        return {
            Component.BINARY_SENSOR: DiscoveryType.STATE_ONLY,
            Component.SENSOR: DiscoveryType.STATE_ONLY,
            Component.SWITCH: DiscoveryType.STATE_AND_UPDATE,
            Component.BUTTON: DiscoveryType.COMMAND_ONLY
        }[self]

class Feature(Enum):
    EVSE = "evse"
    METER = "meter"

@dataclass
class Entity:
    component: Component
    feature: Feature
    object_id: str
    path: str
    name_de: str
    name_en: str
    static_info: str

    def get_json_len(self):
        return max(len(self.name_de), len(self.name_en)) + len(self.object_id) + len(self.path) + len(self.get_static_info_str())

    def get_static_info_str(self):
        return ",".join(['\\"{}\\":\\"{}\\"'.format(k, v.replace('"','\\"')) for k, v in self.static_info.items()])

topic_template = """    {{
        .feature = "{feature}",
        .path = "{path}",
        .component = "{component}",
        .object_id = "{object_id}",
        .name_de = "{name_de}",
        .name_en = "{name_en}",
        .static_info = "{static_info}",
        .type = MqttDiscoveryType::{discovery_type},
    }}"""






entities = [

Entity(Component.BINARY_SENSOR, Feature.EVSE, "cable", "evse/state", "Wallbox Ladekabel verbunden", "Wallbox cable connected",
    {"value_template":"{{value_json.charger_state in [1, 2, 3]}}",
     "payload_on":"True",
     "payload_off":"False",
     "device_class":"plug"}),

Entity(Component.BINARY_SENSOR, Feature.EVSE, "ready", "evse/state", "Wallbox ladebereit", "Wallbox ready to charge",
    {"value_template":"{{value_json.charger_state in [2, 3]}}",
     "payload_on":"True",
     "payload_off":"False",
     "device_class":"power"}),

Entity(Component.BINARY_SENSOR, Feature.EVSE, "charging", "evse/state", "Wallbox Ladevorgang", "Wallbox charging",
    {"value_template":"{{value_json.charger_state in [3]}}",
     "payload_on":"True",
     "payload_off":"False",
     "device_class":"battery_charging"}),

Entity(Component.BINARY_SENSOR, Feature.EVSE, "error", "evse/state", "Wallbox Fehler", "Wallbox error",
    {"value_template":"{{value_json.charger_state in [4]}}",
     "payload_on":"True",
     "payload_off":"False",
     "device_class":"problem"}),

Entity(Component.BINARY_SENSOR, Feature.EVSE, "online", "evse/low_level_state", "Wallbox verfügbar", "Wallbox online",
    {"value_template":"{{value_json.uptime>0}}",
     "payload_on":"True",
     "payload_off":"False",
     "device_class":"connectivity",
     "expire_after": "30"}),

Entity(Component.SENSOR, Feature.METER, "powernow", "meter/values", "Wallbox aktuelle Ladeleistung", "Wallbox current charge power",
    {"value_template":"{{value_json.power}}",
     "unit_of_measurement":"W",
     "device_class":"power",
     "state_class": "measurement"}),

Entity(Component.SENSOR, Feature.METER, "energyabs", "meter/values", "Wallbox Zählerstand absolut", "Wallbox meter value (absolute)",
    {"value_template":"{{value_json.energy_abs}}",
     "unit_of_measurement":"kWh",
     "device_class":"energy",
     "state_class": "total"}),

Entity(Component.SENSOR, Feature.METER, "energyrel", "meter/values", "Wallbox Zählerstand relativ", "Wallbox meter value (relative)",
    {"value_template":"{{value_json.energy_rel}}",
     "unit_of_measurement":"kWh",
     "device_class":"energy",
     "state_class": "total"}),

Entity(Component.SWITCH, Feature.EVSE, "autocharge", "evse/auto_start_charging", "Wallbox automatisches Laden", "Wallbox automatic charging",
    {"value_template":"{{value_json.auto_start_charging}}",
     "state_on":"True",
     "state_off":"False",
     "payload_on":"True",
     "payload_off":"False",
     "icon": "mdi:ev-plug-type2"}),

Entity(Component.BUTTON, Feature.EVSE, "startcharge", "evse/start_charging", "Wallbox Ladevorgang freigeben", "Wallbox start charging",
    {"payload_press": "null",
     "icon": "mdi:flash"}),

Entity(Component.BUTTON, Feature.EVSE, "stopcharge", "evse/stop_charging", "Wallbox Ladevorgang beenden", "Wallbox stop charging",
    {"payload_press": "null",
     "icon": "mdi:flash-off"}),

]

topics = [topic_template.format(
            feature=x.feature.value,
            path=x.path,
            component=x.component.value,
            object_id=x.object_id,
            name_de=x.name_de,
            name_en=x.name_en,
            static_info=",".join(['\\"{}\\":\\"{}\\"'.format(k, v.replace('"','\\"')) for k, v in x.static_info.items()]),
            discovery_type=x.component.get_discovery_type().value,
            )
          for x in entities]

util.specialize_template("mqtt_discovery_topics.cpp.template", "mqtt_discovery_topics.cpp", {
    "{{{topics}}}": ",\n".join(topics),
    })

util.specialize_template("mqtt_discovery_topics.h.template", "mqtt_discovery_topics.h", {
    "{{{topic_count}}}": str(len(topics)),
    "{{{max_json_len}}}": str(max([x.get_json_len() for x in entities]))
    })
