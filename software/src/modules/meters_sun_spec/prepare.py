from collections import namedtuple
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

ModelSpec = namedtuple('ModelSpec', 'model_name display_name_en display_name_de model_id is_meter_like is_supported')

model_specs = [
    ModelSpec('Basic Aggregator',                           'FIXME',                            'FIXME',                               2,     False, False),
    ModelSpec('Secure Dataset Read Request',                'FIXME',                            'FIXME',                               3,     False, False),
    ModelSpec('Secure Dataset Read Response',               'FIXME',                            'FIXME',                               4,     False, False),
    ModelSpec('Secure Write Request',                       'FIXME',                            'FIXME',                               5,     False, False),
    ModelSpec('Secure Write Sequential Request',            'FIXME',                            'FIXME',                               6,     False, False),
    ModelSpec('Secure Write Response Model',                'FIXME',                            'FIXME',                               7,     False, False),
    ModelSpec('Get Device Security Certificate',            'FIXME',                            'FIXME',                               8,     False, False),
    ModelSpec('Set Operator Security Certificate',          'FIXME',                            'FIXME',                               9,     False, False),
    ModelSpec('Communication Interface Header',             'FIXME',                            'FIXME',                               10,    False, False),
    ModelSpec('Ethernet Link Layer',                        'FIXME',                            'FIXME',                               11,    False, False),
    ModelSpec('IPv4',                                       'FIXME',                            'FIXME',                               12,    False, False),
    ModelSpec('IPv6',                                       'FIXME',                            'FIXME',                               13,    False, False),
    ModelSpec('Proxy Server',                               'FIXME',                            'FIXME',                               14,    False, False),
    ModelSpec('Interface Counters',                         'FIXME',                            'FIXME',                               15,    False, False),
    ModelSpec('Simple IP Network',                          'FIXME',                            'FIXME',                               16,    False, False),
    ModelSpec('Serial Interface',                           'FIXME',                            'FIXME',                               17,    False, False),
    ModelSpec('Cellular Link',                              'FIXME',                            'FIXME',                               18,    False, False),
    ModelSpec('PPP Link',                                   'FIXME',                            'FIXME',                               19,    False, False),
    ModelSpec('Inverter Single Phase Integer',              'Inverter',                         'Wechselrichter',                      101,   True,  True),
    ModelSpec('Inverter Split Phase Integer',               'Inverter',                         'Wechselrichter',                      102,   True,  True),
    ModelSpec('Inverter Three Phase Integer',               'Inverter',                         'Wechselrichter',                      103,   True,  True),
    ModelSpec('Inverter Single Phase Float',                'Inverter',                         'Wechselrichter',                      111,   True,  True),
    ModelSpec('Inverter Split Phase Float',                 'Inverter',                         'Wechselrichter',                      112,   True,  True),
    ModelSpec('Inverter Three Phase Float',                 'Inverter',                         'Wechselrichter',                      113,   True,  True),
    ModelSpec('Inverter Nameplate',                         'FIXME',                            'FIXME',                               120,   False, False),
    ModelSpec('Inverter Basic Settings',                    'FIXME',                            'FIXME',                               121,   False, False),
    ModelSpec('Inverter Measurements And Status',           'Inverter Measurements and Status', 'Wechselrichter Messwerte und Status', 122,   True,  False),
    ModelSpec('Inverter Immediate Controls',                'FIXME',                            'FIXME',                               123,   False, False),
    ModelSpec('Inverter Basic Storage Controls',            'FIXME',                            'FIXME',                               124,   False, False),
    ModelSpec('Inverter Pricing',                           'FIXME',                            'FIXME',                               125,   False, False),
    ModelSpec('Inverter Static Volt-VAR Arrays',            'FIXME',                            'FIXME',                               126,   False, False),
    ModelSpec('Inverter Parameterized Frequency-Watt',      'FIXME',                            'FIXME',                               127,   False, False),
    ModelSpec('Inverter Dynamic Reactive Current',          'FIXME',                            'FIXME',                               128,   False, False),
    ModelSpec('Inverter LVRT Must Disconnect',              'FIXME',                            'FIXME',                               129,   False, False),
    ModelSpec('Inverter HVRT Must Disconnect',              'FIXME',                            'FIXME',                               130,   False, False),
    ModelSpec('Inverter Watt-Power Factor',                 'FIXME',                            'FIXME',                               131,   False, False),
    ModelSpec('Inverter Volt-Watt',                         'FIXME',                            'FIXME',                               132,   False, False),
    ModelSpec('Inverter Basic Scheduling',                  'FIXME',                            'FIXME',                               133,   False, False),
    ModelSpec('Inverter Curve-Based Frequency-Watt',        'FIXME',                            'FIXME',                               134,   False, False),
    ModelSpec('Inverter Low Frequency Ride-through',        'FIXME',                            'FIXME',                               135,   False, False),
    ModelSpec('Inverter High Frequency Ride-through',       'FIXME',                            'FIXME',                               136,   False, False),
    ModelSpec('Inverter LVRT Must Remain Connected',        'FIXME',                            'FIXME',                               137,   False, False),
    ModelSpec('Inverter HVRT Must Remain Connected',        'FIXME',                            'FIXME',                               138,   False, False),
    ModelSpec('Inverter LVRT Extended Curve',               'FIXME',                            'FIXME',                               139,   False, False),
    ModelSpec('Inverter HVRT Extended Curve',               'FIXME',                            'FIXME',                               140,   False, False),
    ModelSpec('Inverter LFRT Must Remain Connected',        'FIXME',                            'FIXME',                               141,   False, False),
    ModelSpec('Inverter HFRT Must Remain Connected',        'FIXME',                            'FIXME',                               142,   False, False),
    ModelSpec('Inverter LFRT Extended Curve',               'FIXME',                            'FIXME',                               143,   False, False),
    ModelSpec('Inverter HFRT Extended Curve',               'FIXME',                            'FIXME',                               144,   False, False),
    ModelSpec('Inverter Extended Settings',                 'FIXME',                            'FIXME',                               145,   False, False),
    ModelSpec('Inverter Multiple MPPT Extension',           'FIXME',                            'FIXME',                               160,   False, False),
    ModelSpec('Meter Single Phase Integer',                 'Meter',                            'Zähler',                              201,   True,  True),
    ModelSpec('Meter Split Phase Integer',                  'Meter',                            'Zähler',                              202,   True,  True),
    ModelSpec('Meter Wye Three Phase Integer',              'Meter',                            'Zähler',                              203,   True,  True),
    ModelSpec('Meter Delta Three Phase Integer',            'Meter',                            'Zähler',                              204,   True,  True),
    ModelSpec('Meter Single Phase Float',                   'Meter',                            'Zähler',                              211,   True,  True),
    ModelSpec('Meter Split Phase Float',                    'Meter',                            'Zähler',                              212,   True,  True),
    ModelSpec('Meter Wye Three Phase Float',                'Meter',                            'Zähler',                              213,   True,  True),
    ModelSpec('Meter Delta Three Phase Float',              'Meter',                            'Zähler',                              214,   True,  True),
    ModelSpec('Meter Secure',                               'Meter (Secure)',                   'Zähler (Signiert)',                   220,   True,  False),
    ModelSpec('Irradiance',                                 'FIXME',                            'FIXME',                               302,   False, False),
    ModelSpec('Back of Module Temperature',                 'FIXME',                            'FIXME',                               303,   False, False),
    ModelSpec('Inclinometer',                               'FIXME',                            'FIXME',                               304,   False, False),
    ModelSpec('GPS',                                        'FIXME',                            'FIXME',                               305,   False, False),
    ModelSpec('Reference Point',                            'FIXME',                            'FIXME',                               306,   False, False),
    ModelSpec('Meteorological Base',                        'FIXME',                            'FIXME',                               307,   False, False),
    ModelSpec('Meteorological Minimal',                     'FIXME',                            'FIXME',                               308,   False, False),
    ModelSpec('String Combiner Basic',                      'FIXME',                            'FIXME',                               401,   False, False),
    ModelSpec('String Combiner Advanced',                   'FIXME',                            'FIXME',                               402,   False, False),
    ModelSpec('String Combiner Current',                    'FIXME',                            'FIXME',                               403,   False, False),
    ModelSpec('String Combiner Advanced Voltage Energy',    'FIXME',                            'FIXME',                               404,   False, False),
    ModelSpec('Solar Module A',                             'FIXME',                            'FIXME',                               501,   False, False),
    ModelSpec('Solar Module B',                             'FIXME',                            'FIXME',                               502,   False, False),
    ModelSpec('Tracker Controller',                         'FIXME',                            'FIXME',                               601,   False, False),
    ModelSpec('DER AC Measurements',                        'AC Measurements',                  'AC-Messwerte',                        701,   True,  True),
    ModelSpec('DER Capacity',                               'FIXME',                            'FIXME',                               702,   False, False),
    ModelSpec('DER Enter Service',                          'FIXME',                            'FIXME',                               703,   False, False),
    ModelSpec('DER AC Controls',                            'FIXME',                            'FIXME',                               704,   False, False),
    ModelSpec('DER Volt-VAR',                               'FIXME',                            'FIXME',                               705,   False, False),
    ModelSpec('DER Volt-Watt',                              'FIXME',                            'FIXME',                               706,   False, False),
    ModelSpec('DER Low Voltage Trip',                       'FIXME',                            'FIXME',                               707,   False, False),
    ModelSpec('DER High Voltage Trip',                      'FIXME',                            'FIXME',                               708,   False, False),
    ModelSpec('DER Low Frequency Trip',                     'FIXME',                            'FIXME',                               709,   False, False),
    ModelSpec('DER High Frequency Trip',                    'FIXME',                            'FIXME',                               710,   False, False),
    ModelSpec('DER Frequency Droop',                        'FIXME',                            'FIXME',                               711,   False, False),
    ModelSpec('DER Watt-VAR',                               'FIXME',                            'FIXME',                               712,   False, False),
    ModelSpec('DER Storage Capacity',                       'Storage Capacity',                 'Speicherkapazität',                   713,   True,  True),
    ModelSpec('DER DC Measurements',                        'DC Measurements',                  'DC-Messwerte',                        714,   True,  True),
    ModelSpec('DER Control',                                'FIXME',                            'FIXME',                               715,   False, False),
    ModelSpec('Battery Base Deprecated',                    'FIXME',                            'FIXME',                               801,   False, False),
    ModelSpec('Battery Base',                               'Battery',                          'Batterie',                            802,   True,  False),
    ModelSpec('Battery Lithium-Ion',                        'FIXME',                            'FIXME',                               803,   False, False),
    ModelSpec('Battery Lithium-Ion String',                 'FIXME',                            'FIXME',                               804,   False, False),
    ModelSpec('Battery Lithium-Ion Module',                 'FIXME',                            'FIXME',                               805,   False, False),
    ModelSpec('Battery Flow',                               'FIXME',                            'FIXME',                               806,   False, False),
    ModelSpec('Battery Flow String',                        'FIXME',                            'FIXME',                               807,   False, False),
    ModelSpec('Battery Flow Module',                        'FIXME',                            'FIXME',                               808,   False, False),
    ModelSpec('Battery Flow Stack',                         'FIXME',                            'FIXME',                               809,   False, False),
    ModelSpec('Test 1',                                     'FIXME',                            'FIXME',                               63001, False, False),
    ModelSpec('Test 2',                                     'FIXME',                            'FIXME',                               63002, False, False),
    ModelSpec('Veris Status and Configuration',             'FIXME',                            'FIXME',                               64001, False, False),
    ModelSpec('Mersen Green String',                        'FIXME',                            'FIXME',                               64020, False, False),
    ModelSpec('Eltek Inverter',                             'FIXME',                            'FIXME',                               64101, False, False),
    ModelSpec('OutBack AXS Device',                         'FIXME',                            'FIXME',                               64110, False, False),
    ModelSpec('Basic Charge Controller',                    'FIXME',                            'FIXME',                               64111, False, False),
    ModelSpec('OutBack FM Charge Controller',               'FIXME',                            'FIXME',                               64112, False, False),
]

enum_values = []
spec_values = []
translation_values = {'en': [], 'de': []}
model_infos = []
model_is_meter_like = []
model_is_supported = []

for model_spec in model_specs:
    enum_key = model_spec.model_name.replace(' ', '').replace('-', '')

    enum_values.append(f'    {enum_key} = {model_spec.model_id},\n')
    spec_values.append(f'    {{\n        SunSpecModelID::{enum_key},\n        "{model_spec.model_name}",\n    }},\n')

    if model_spec.is_meter_like:
        translation_values['en'].append(f'"model_{model_spec.model_id}": "{model_spec.display_name_en}"')
        translation_values['de'].append(f'"model_{model_spec.model_id}": "{model_spec.display_name_de}"')

    model_infos.append(f'    {{model_id: {model_spec.model_id}, is_meter_like: {str(model_spec.is_meter_like).lower()}, is_supported: {str(model_spec.is_supported).lower()}}},\n')
    model_is_meter_like.append(f'    {model_spec.model_id}: {str(model_spec.is_meter_like).lower()},\n')
    model_is_supported.append(f'    {model_spec.model_id}: {str(model_spec.is_supported).lower()},\n')

with open('sun_spec_model_id.h', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#include <stdint.h>\n')
    f.write('#include <stdlib.h>\n\n')
    f.write('#pragma once\n\n')
    f.write('enum class SunSpecModelID : uint16_t {\n')
    f.write(''.join(enum_values))
    f.write('};\n\n')
    f.write('struct SunSpecModelSpec {\n')
    f.write('    SunSpecModelID model_id;\n')
    f.write('    const char *model_name;\n')
    f.write('};\n\n')
    f.write('extern const SunSpecModelSpec sun_spec_model_specs[];\n\n')
    f.write('extern const size_t sun_spec_model_specs_length;\n')

with open('sun_spec_model_id.cpp', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#include "sun_spec_model_id.h"\n\n')
    f.write('const SunSpecModelSpec sun_spec_model_specs[] = {\n')
    f.write(''.join(spec_values))
    f.write('};\n\n')
    f.write(f'const size_t sun_spec_model_specs_length = {len(model_specs)};\n')

for lang in translation_values:
    tfutil.specialize_template(f'../../../web/src/modules/meters_sun_spec/translation_{lang}.tsx.template', f'../../../web/src/modules/meters_sun_spec/translation_{lang}.tsx', {
        '{{{models}}}': ',\n            '.join(translation_values[lang]),
    })

with open('../../../web/src/modules/meters_sun_spec/sun_spec_model_specs.ts', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('export const SUN_SPEC_MODEL_INFOS: {model_id: number, is_meter_like: boolean, is_supported: boolean}[] = [\n')
    f.write(''.join(model_infos))
    f.write('];\n\n')
    f.write('export const SUN_SPEC_MODEL_IS_METER_LIKE: {[model_id: number]: boolean} = {\n')
    f.write(''.join(model_is_meter_like))
    f.write('};\n\n')
    f.write('export const SUN_SPEC_MODEL_IS_SUPPORTED: {[model_id: number]: boolean} = {\n')
    f.write(''.join(model_is_supported))
    f.write('};\n')
