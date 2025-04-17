import sys
import tinkerforge_util as tfutil
import sungrow
import solarmax
import victron_energy
import deye
import alpha_ess
import shelly
import goodwe
import solax
import fronius_gen24_plus
import hailei
import fox_ess
import siemens
import carlo_gavazzi
import solaredge
import eastron
import tinkerforge
import sax_power
import e3dc
import huawei

tfutil.create_parent_module(__file__, 'software')

from software import util

specs = sungrow.specs + solarmax.specs + victron_energy.specs + deye.specs + alpha_ess.specs + shelly.specs + goodwe.specs \
      + solax.specs + fronius_gen24_plus.specs + hailei.specs + fox_ess.specs + siemens.specs + carlo_gavazzi.specs + solaredge.specs \
      + eastron.specs + tinkerforge.specs + sax_power.specs + e3dc.specs + huawei.specs
spec_values = []

for spec in specs:
    for variant_spec in spec.get('variants', [None]):
        spec_name = util.FlavoredName(spec['name'].format(variant=variant_spec)).get()

        value_names = []
        value_addresses = []
        value_specs = []
        value_ids_raw = []
        value_ids = []
        value_index = []
        f32_negative_max_as_nan = 'true' if spec.get('f32_negative_max_as_nan', False) else 'false'
        current_index = 0

        for value in spec['values']:
            variants_value = value.get('variants')

            if variants_value != None and variant_spec not in variants_value:
                continue

            if value['name'] in value_names:
                print(f'Error: Value {spec_name.space} / {value["name"]} is duplicate')
                sys.exit(1)

            value_names.append(value['name'])

            start_address_offset = value.get('start_address_offset', spec.get('start_address_offset', 0))
            start_address = value['start_address'] - start_address_offset if value['start_address'] != 'START_ADDRESS_VIRTUAL' else 'START_ADDRESS_VIRTUAL'

            if start_address != "START_ADDRESS_VIRTUAL":
                address_name_words = value['name'].split('|', 1)[-1].split('[')[0].replace('-', ' ').replace('/', ' ').split(' ')
                address_name = ''.join([word[0].upper() + word[1:] for word in address_name_words if len(word) > 0])

                value_addresses.append(f'    {address_name} = {start_address},')

            value_specs.append(
                '    {\n'
                f'        "{value["name"]}",\n'
                f'        ModbusRegisterType::{value.get("register_type", spec["register_type"])},\n'
                f'        {start_address},\n'
                f'        ModbusValueType::{value.get("value_type", "None")},\n'
                f'        {"true" if value.get("drop_sign", False) else "false"},\n'
                f'        {value.get("offset", 0.0)}f,\n'
                f'        {value.get("scale_factor", 1.0)}f,\n'
                '    },'
            )

            if value['value_id'] == 'VALUE_ID_META':
                value_index.append('    VALUE_INDEX_META,')
            elif value['value_id'] == 'VALUE_ID_DEBUG':
                value_index.append('    VALUE_INDEX_DEBUG,')
            else:
                if value['value_id'] in value_ids_raw:
                    print(f'Error: Value {spec_name.space} / {value["name"]} has duplicate ID {value["value_id"]}')
                    sys.exit(1)

                value_ids_raw.append(value['value_id'])
                value_ids.append(f'    MeterValueID::{value["value_id"]},')
                value_index.append(f'    {current_index},')
                current_index += 1

        spec_values.append(f'enum class {spec_name.camel}Address : uint16_t {{\n' + '\n'.join(value_addresses) + '\n};')
        spec_values.append(f'static const MeterModbusTCP::ValueSpec {spec_name.under}_specs[] = {{\n' + '\n'.join(value_specs) + '\n};')

        if len(value_ids) > 0:
            spec_values.append(f'static const MeterValueID {spec_name.under}_ids[] = {{\n' + '\n'.join(value_ids) + '\n};')

        spec_values.append(f'static const uint32_t {spec_name.under}_index[] = {{\n' + '\n'.join(value_index) + '\n};')
        spec_values.append(f'static const MeterModbusTCP::ValueTable {spec_name.under}_table = {{\n'
                           f'    {spec_name.under}_specs,\n'
                           f'    ARRAY_SIZE({spec_name.under}_specs),\r')

        if len(value_ids) > 0:
            spec_values.append(f'    {spec_name.under}_ids,\n'
                               f'    ARRAY_SIZE({spec_name.under}_ids),\r')
        else:
            spec_values.append('    nullptr,\n'
                               '    0,\r')

        spec_values.append(f'    {spec_name.under}_index,\n'
                           f'    {f32_negative_max_as_nan},\n'
                           '};')

with open('meters_modbus_tcp_defs.inc', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#define VALUE_INDEX_META  0xFFFFFFFEu\n')
    f.write('#define VALUE_INDEX_DEBUG 0xFFFFFFFDu\n\n')
    f.write('#define START_ADDRESS_VIRTUAL 0xFFFFFFFEu\n\n')
    f.write('\n\n'.join(spec_values).replace('\r\n', '') + '\n')
