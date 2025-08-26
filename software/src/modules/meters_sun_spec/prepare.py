from collections import namedtuple
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

ModelSpec = namedtuple('ModelSpec', 'model_name display_name_en display_name_de model_id is_meter_like meter_location is_supported')

model_specs = [
    ModelSpec('Basic Aggregator',                           'FIXME',                            'FIXME',                               2,     False, 'Unknown',  False),
    ModelSpec('Secure Dataset Read Request',                'FIXME',                            'FIXME',                               3,     False, 'Unknown',  False),
    ModelSpec('Secure Dataset Read Response',               'FIXME',                            'FIXME',                               4,     False, 'Unknown',  False),
    ModelSpec('Secure Write Request',                       'FIXME',                            'FIXME',                               5,     False, 'Unknown',  False),
    ModelSpec('Secure Write Sequential Request',            'FIXME',                            'FIXME',                               6,     False, 'Unknown',  False),
    ModelSpec('Secure Write Response Model',                'FIXME',                            'FIXME',                               7,     False, 'Unknown',  False),
    ModelSpec('Get Device Security Certificate',            'FIXME',                            'FIXME',                               8,     False, 'Unknown',  False),
    ModelSpec('Set Operator Security Certificate',          'FIXME',                            'FIXME',                               9,     False, 'Unknown',  False),
    ModelSpec('Communication Interface Header',             'FIXME',                            'FIXME',                               10,    False, 'Unknown',  False),
    ModelSpec('Ethernet Link Layer',                        'FIXME',                            'FIXME',                               11,    False, 'Unknown',  False),
    ModelSpec('IPv4',                                       'FIXME',                            'FIXME',                               12,    False, 'Unknown',  False),
    ModelSpec('IPv6',                                       'FIXME',                            'FIXME',                               13,    False, 'Unknown',  False),
    ModelSpec('Proxy Server',                               'FIXME',                            'FIXME',                               14,    False, 'Unknown',  False),
    ModelSpec('Interface Counters',                         'FIXME',                            'FIXME',                               15,    False, 'Unknown',  False),
    ModelSpec('Simple IP Network',                          'FIXME',                            'FIXME',                               16,    False, 'Unknown',  False),
    ModelSpec('Serial Interface',                           'FIXME',                            'FIXME',                               17,    False, 'Unknown',  False),
    ModelSpec('Cellular Link',                              'FIXME',                            'FIXME',                               18,    False, 'Unknown',  False),
    ModelSpec('PPP Link',                                   'FIXME',                            'FIXME',                               19,    False, 'Unknown',  False),
    ModelSpec('Inverter Single Phase Integer',              'Inverter',                         'Wechselrichter',                      101,   True,  'Inverter', True),
    ModelSpec('Inverter Split Phase Integer',               'Inverter',                         'Wechselrichter',                      102,   True,  'Inverter', True),
    ModelSpec('Inverter Three Phase Integer',               'Inverter',                         'Wechselrichter',                      103,   True,  'Inverter', True),
    ModelSpec('Inverter Single Phase Float',                'Inverter',                         'Wechselrichter',                      111,   True,  'Inverter', True),
    ModelSpec('Inverter Split Phase Float',                 'Inverter',                         'Wechselrichter',                      112,   True,  'Inverter', True),
    ModelSpec('Inverter Three Phase Float',                 'Inverter',                         'Wechselrichter',                      113,   True,  'Inverter', True),
    ModelSpec('Inverter Nameplate',                         'FIXME',                            'FIXME',                               120,   False, 'Inverter', False),
    ModelSpec('Inverter Basic Settings',                    'FIXME',                            'FIXME',                               121,   False, 'Inverter', False),
    ModelSpec('Inverter Measurements And Status',           'Inverter Measurements and Status', 'Wechselrichter Messwerte und Status', 122,   True,  'Inverter', False),
    ModelSpec('Inverter Immediate Controls',                'FIXME',                            'FIXME',                               123,   False, 'Inverter', False),
    ModelSpec('Inverter Basic Storage Controls',            'FIXME',                            'FIXME',                               124,   False, 'Inverter', False),
    ModelSpec('Inverter Pricing',                           'FIXME',                            'FIXME',                               125,   False, 'Inverter', False),
    ModelSpec('Inverter Static Volt-VAR Arrays',            'FIXME',                            'FIXME',                               126,   False, 'Inverter', False),
    ModelSpec('Inverter Parameterized Frequency-Watt',      'FIXME',                            'FIXME',                               127,   False, 'Inverter', False),
    ModelSpec('Inverter Dynamic Reactive Current',          'FIXME',                            'FIXME',                               128,   False, 'Inverter', False),
    ModelSpec('Inverter LVRT Must Disconnect',              'FIXME',                            'FIXME',                               129,   False, 'Inverter', False),
    ModelSpec('Inverter HVRT Must Disconnect',              'FIXME',                            'FIXME',                               130,   False, 'Inverter', False),
    ModelSpec('Inverter Watt-Power Factor',                 'FIXME',                            'FIXME',                               131,   False, 'Inverter', False),
    ModelSpec('Inverter Volt-Watt',                         'FIXME',                            'FIXME',                               132,   False, 'Inverter', False),
    ModelSpec('Inverter Basic Scheduling',                  'FIXME',                            'FIXME',                               133,   False, 'Inverter', False),
    ModelSpec('Inverter Curve-Based Frequency-Watt',        'FIXME',                            'FIXME',                               134,   False, 'Inverter', False),
    ModelSpec('Inverter Low Frequency Ride-through',        'FIXME',                            'FIXME',                               135,   False, 'Inverter', False),
    ModelSpec('Inverter High Frequency Ride-through',       'FIXME',                            'FIXME',                               136,   False, 'Inverter', False),
    ModelSpec('Inverter LVRT Must Remain Connected',        'FIXME',                            'FIXME',                               137,   False, 'Inverter', False),
    ModelSpec('Inverter HVRT Must Remain Connected',        'FIXME',                            'FIXME',                               138,   False, 'Inverter', False),
    ModelSpec('Inverter LVRT Extended Curve',               'FIXME',                            'FIXME',                               139,   False, 'Inverter', False),
    ModelSpec('Inverter HVRT Extended Curve',               'FIXME',                            'FIXME',                               140,   False, 'Inverter', False),
    ModelSpec('Inverter LFRT Must Remain Connected',        'FIXME',                            'FIXME',                               141,   False, 'Inverter', False),
    ModelSpec('Inverter HFRT Must Remain Connected',        'FIXME',                            'FIXME',                               142,   False, 'Inverter', False),
    ModelSpec('Inverter LFRT Extended Curve',               'FIXME',                            'FIXME',                               143,   False, 'Inverter', False),
    ModelSpec('Inverter HFRT Extended Curve',               'FIXME',                            'FIXME',                               144,   False, 'Inverter', False),
    ModelSpec('Inverter Extended Settings',                 'FIXME',                            'FIXME',                               145,   False, 'Inverter', False),
    ModelSpec('Inverter Multiple MPPT Extension',           'MPPT Extension',                   'MPPT-Erweiterung',                    160,   True,  'PV',       True),
    ModelSpec('Meter Single Phase Integer',                 'Meter',                            'Zähler',                              201,   True,  'Unknown',  True),
    ModelSpec('Meter Split Phase Integer',                  'Meter',                            'Zähler',                              202,   True,  'Unknown',  True),
    ModelSpec('Meter Wye Three Phase Integer',              'Meter',                            'Zähler',                              203,   True,  'Unknown',  True),
    ModelSpec('Meter Delta Three Phase Integer',            'Meter',                            'Zähler',                              204,   True,  'Unknown',  True),
    ModelSpec('Meter Single Phase Float',                   'Meter',                            'Zähler',                              211,   True,  'Unknown',  True),
    ModelSpec('Meter Split Phase Float',                    'Meter',                            'Zähler',                              212,   True,  'Unknown',  True),
    ModelSpec('Meter Wye Three Phase Float',                'Meter',                            'Zähler',                              213,   True,  'Unknown',  True),
    ModelSpec('Meter Delta Three Phase Float',              'Meter',                            'Zähler',                              214,   True,  'Unknown',  True),
    ModelSpec('Meter Secure',                               'Meter (Secure)',                   'Zähler (Signiert)',                   220,   True,  'Unknown',  False),
    ModelSpec('Irradiance',                                 'FIXME',                            'FIXME',                               302,   False, 'Unknown',  False),
    ModelSpec('Back of Module Temperature',                 'FIXME',                            'FIXME',                               303,   False, 'Unknown',  False),
    ModelSpec('Inclinometer',                               'FIXME',                            'FIXME',                               304,   False, 'Unknown',  False),
    ModelSpec('GPS',                                        'FIXME',                            'FIXME',                               305,   False, 'Unknown',  False),
    ModelSpec('Reference Point',                            'FIXME',                            'FIXME',                               306,   False, 'Unknown',  False),
    ModelSpec('Meteorological Base',                        'FIXME',                            'FIXME',                               307,   False, 'Unknown',  False),
    ModelSpec('Meteorological Minimal',                     'FIXME',                            'FIXME',                               308,   False, 'Unknown',  False),
    ModelSpec('String Combiner Basic',                      'FIXME',                            'FIXME',                               401,   False, 'Unknown',  False),
    ModelSpec('String Combiner Advanced',                   'FIXME',                            'FIXME',                               402,   False, 'Unknown',  False),
    ModelSpec('String Combiner Current',                    'FIXME',                            'FIXME',                               403,   False, 'Unknown',  False),
    ModelSpec('String Combiner Advanced Voltage Energy',    'FIXME',                            'FIXME',                               404,   False, 'Unknown',  False),
    ModelSpec('Solar Module A',                             'FIXME',                            'FIXME',                               501,   False, 'Unknown',  False),
    ModelSpec('Solar Module B',                             'FIXME',                            'FIXME',                               502,   False, 'Unknown',  False),
    ModelSpec('Tracker Controller',                         'FIXME',                            'FIXME',                               601,   False, 'Unknown',  False),
    ModelSpec('DER AC Measurements',                        'AC Measurements',                  'AC-Messwerte',                        701,   True,  'Inverter', True),
    ModelSpec('DER Capacity',                               'FIXME',                            'FIXME',                               702,   False, 'Unknown',  False),
    ModelSpec('DER Enter Service',                          'FIXME',                            'FIXME',                               703,   False, 'Unknown',  False),
    ModelSpec('DER AC Controls',                            'FIXME',                            'FIXME',                               704,   False, 'Unknown',  False),
    ModelSpec('DER Volt-VAR',                               'FIXME',                            'FIXME',                               705,   False, 'Unknown',  False),
    ModelSpec('DER Volt-Watt',                              'FIXME',                            'FIXME',                               706,   False, 'Unknown',  False),
    ModelSpec('DER Low Voltage Trip',                       'FIXME',                            'FIXME',                               707,   False, 'Unknown',  False),
    ModelSpec('DER High Voltage Trip',                      'FIXME',                            'FIXME',                               708,   False, 'Unknown',  False),
    ModelSpec('DER Low Frequency Trip',                     'FIXME',                            'FIXME',                               709,   False, 'Unknown',  False),
    ModelSpec('DER High Frequency Trip',                    'FIXME',                            'FIXME',                               710,   False, 'Unknown',  False),
    ModelSpec('DER Frequency Droop',                        'FIXME',                            'FIXME',                               711,   False, 'Unknown',  False),
    ModelSpec('DER Watt-VAR',                               'FIXME',                            'FIXME',                               712,   False, 'Unknown',  False),
    ModelSpec('DER Storage Capacity',                       'Storage Capacity',                 'Speicherkapazität',                   713,   True,  'Battery',  True),
    ModelSpec('DER DC Measurements',                        'DC Measurements',                  'DC-Messwerte',                        714,   True,  'Unknown',  True),
    ModelSpec('DER Control',                                'FIXME',                            'FIXME',                               715,   False, 'Unknown',  False),
    ModelSpec('Battery Base Deprecated',                    'FIXME',                            'FIXME',                               801,   False, 'Battery',  False),
    ModelSpec('Battery Base',                               'Battery',                          'Batterie',                            802,   True,  'Battery',  True),
    ModelSpec('Battery Lithium-Ion',                        'FIXME',                            'FIXME',                               803,   False, 'Battery',  False),
    ModelSpec('Battery Lithium-Ion String',                 'FIXME',                            'FIXME',                               804,   False, 'Battery',  False),
    ModelSpec('Battery Lithium-Ion Module',                 'FIXME',                            'FIXME',                               805,   False, 'Battery',  False),
    ModelSpec('Battery Flow',                               'FIXME',                            'FIXME',                               806,   False, 'Battery',  False),
    ModelSpec('Battery Flow String',                        'FIXME',                            'FIXME',                               807,   False, 'Battery',  False),
    ModelSpec('Battery Flow Module',                        'FIXME',                            'FIXME',                               808,   False, 'Battery',  False),
    ModelSpec('Battery Flow Stack',                         'FIXME',                            'FIXME',                               809,   False, 'Battery',  False),
    ModelSpec('Test 1',                                     'FIXME',                            'FIXME',                               63001, False, 'Unknown',  False),
    ModelSpec('Test 2',                                     'FIXME',                            'FIXME',                               63002, False, 'Unknown',  False),
    ModelSpec('Veris Status and Configuration',             'FIXME',                            'FIXME',                               64001, False, 'Unknown',  False),
    ModelSpec('Mersen Green String',                        'FIXME',                            'FIXME',                               64020, False, 'Unknown',  False),
    ModelSpec('Eltek Inverter',                             'FIXME',                            'FIXME',                               64101, False, 'Unknown',  False),
    ModelSpec('OutBack AXS Device',                         'FIXME',                            'FIXME',                               64110, False, 'Unknown',  False),
    ModelSpec('Basic Charge Controller',                    'FIXME',                            'FIXME',                               64111, False, 'Unknown',  False),
    ModelSpec('OutBack FM Charge Controller',               'FIXME',                            'FIXME',                               64112, False, 'Unknown',  False),
]

enum_values = []
spec_values = []
translation_values = {'en': [], 'de': []}
model_infos = []
model_is_meter_like = []
model_meter_location = []
model_is_supported = []

for model_spec in model_specs:
    enum_key = model_spec.model_name.replace(' ', '').replace('-', '')

    enum_values.append(f'    {enum_key} = {model_spec.model_id},\n')
    spec_values.append(f'    {{\n        SunSpecModelID::{enum_key},\n        "{model_spec.model_name}",\n        MeterLocation::{model_spec.meter_location},\n    }},\n')

    if model_spec.is_meter_like:
        translation_values['en'].append(f'"model_{model_spec.model_id}": "{model_spec.display_name_en}"')
        translation_values['de'].append(f'"model_{model_spec.model_id}": "{model_spec.display_name_de}"')

    model_infos.append(f'    {{model_id: {model_spec.model_id}, is_meter_like: {str(model_spec.is_meter_like).lower()}, is_supported: {str(model_spec.is_supported).lower()}}},\n')
    model_is_meter_like.append(f'    {model_spec.model_id}: {str(model_spec.is_meter_like).lower()},\n')
    model_meter_location.append(f'    {model_spec.model_id}: MeterLocation.{model_spec.meter_location},\n')
    model_is_supported.append(f'    {model_spec.model_id}: {str(model_spec.is_supported).lower()},\n')

h = '// WARNING: This file is generated.\n\n'
h += '#include <stdint.h>\n'
h += '#include <stdlib.h>\n\n'
h += '#include "modules/meters/meter_location.enum.h"\n\n'
h += '#pragma once\n\n'
h += 'enum class SunSpecModelID : uint16_t {\n'
h += ''.join(enum_values)
h += '};\n\n'
h += 'struct SunSpecModelSpec {\n'
h += '    SunSpecModelID model_id;\n'
h += '    const char *model_name;\n'
h += '    MeterLocation meter_location;\n'
h += '};\n\n'
h += 'extern const SunSpecModelSpec sun_spec_model_specs[];\n\n'
h += 'extern const size_t sun_spec_model_specs_length;\n'

tfutil.write_file_if_different('sun_spec_model_specs.h', h)

cpp = '// WARNING: This file is generated.\n\n'
cpp += '#include "sun_spec_model_specs.h"\n\n'
cpp += 'const SunSpecModelSpec sun_spec_model_specs[] = {\n'
cpp += ''.join(spec_values)
cpp += '};\n\n'
cpp += f'const size_t sun_spec_model_specs_length = {len(model_specs)};\n'

tfutil.write_file_if_different('sun_spec_model_specs.cpp', cpp)

for lang in translation_values:
    tfutil.specialize_template(f'../../../web/src/modules/meters_sun_spec/translation_{lang}.tsx.template', f'../../../web/src/modules/meters_sun_spec/translation_{lang}.tsx', {
        '{{{models}}}': ',\n            '.join(translation_values[lang]),
    })

ts = '// WARNING: This file is generated.\n\n'
ts += 'import { MeterLocation } from "../meters/meter_location.enum";\n\n'
ts += 'export const SUN_SPEC_MODEL_INFOS: {model_id: number, is_meter_like: boolean, is_supported: boolean}[] = [\n'
ts += ''.join(model_infos)
ts += '];\n\n'
ts += 'export const SUN_SPEC_MODEL_IS_METER_LIKE: {[model_id: number]: boolean} = {\n'
ts += ''.join(model_is_meter_like)
ts += '};\n\n'
ts += 'export const SUN_SPEC_MODEL_METER_LOCATION: {[model_id: number]: number} = {\n'
ts += ''.join(model_meter_location)
ts += '};\n\n'
ts += 'export const SUN_SPEC_MODEL_IS_SUPPORTED: {[model_id: number]: boolean} = {\n'
ts += ''.join(model_is_supported)
ts += '};\n'

tfutil.write_file_if_different('../../../web/src/modules/meters_sun_spec/sun_spec_model_specs.ts', ts)
