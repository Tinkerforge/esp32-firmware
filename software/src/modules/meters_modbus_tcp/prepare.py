import tinkerforge_util as tfutil
import sungrow
import solarmax
import victron_energy
import deye
import alpha_ess
import shelly
import goodwe

tfutil.create_parent_module(__file__, 'software')

from software import util

def make_modbus_value_type(register_count, is_signed, is_float, register_order_is_le):
    assert 1 <= register_count <= 4, register_count

    #      bit [0..2]              bit [3]                          bit [4]                         bit [5]
    return register_count | ((1 if is_signed else 0) << 3) | ((1 if is_float else 0) << 4) | ((1 if register_order_is_le else 0) << 5)


modbus_value_types = [
    ('None',  0),
    ('U16',   make_modbus_value_type(1, False, False, False)),
    ('S16',   make_modbus_value_type(1, True,  False, False)),
    ('U32BE', make_modbus_value_type(2, False, False, False)),
    ('U32LE', make_modbus_value_type(2, False, False, True)),
    ('S32BE', make_modbus_value_type(2, True,  False, False)),
    ('S32LE', make_modbus_value_type(2, True,  False, True)),
    ('F32BE', make_modbus_value_type(2, True,  True,  False)),
    ('F32LE', make_modbus_value_type(2, True,  True,  True)),
    ('U64BE', make_modbus_value_type(4, False, False, False)),
    ('U64LE', make_modbus_value_type(4, False, False, True)),
    ('S64BE', make_modbus_value_type(4, True,  False, False)),
    ('S64LE', make_modbus_value_type(4, True,  False, True)),
    ('F64BE', make_modbus_value_type(4, True,  True,  False)),
    ('F64LE', make_modbus_value_type(4, True,  True,  True)),
]

with open('Modbus Value Type.uint8.enum', 'w', encoding='utf-8') as f:
    f.write('# WARNING: This file is generated\n')

    for item in modbus_value_types:
        f.write(f'{item[0]} = {item[1]}\n')

specs = sungrow.specs + solarmax.specs + victron_energy.specs + deye.specs + alpha_ess.specs + shelly.specs + goodwe.specs
spec_values = []

for spec in specs:
    for variant_spec in spec.get('variants', [None]):
        value_specs = []
        value_ids = []
        value_index = []
        current_index = 0

        for value in spec['values']:
            variant_value = value.get('variant')

            if variant_value != None and variant_value not in variant_spec:
                continue

            start_address_offset = value.get('start_address_offset', spec.get('start_address_offset', 0))

            value_specs.append(
                '    {\n'
                f'        "{value["name"]}",\n'
                f'        ModbusRegisterType::{value.get("register_type", spec["register_type"])},\n'
                f'        {value["start_address"] - start_address_offset if value["start_address"] != "START_ADDRESS_VIRTUAL" else "START_ADDRESS_VIRTUAL"},\n'
                f'        ModbusValueType::{value.get("value_type", "None")},\n'
                f'        {value.get("offset", 0.0)}f,\n'
                f'        {value.get("scale_factor", 1.0)}f,\n'
                '    },'
            )

            if value['value_id'] == 'VALUE_ID_META':
                value_index.append('    VALUE_INDEX_META,')
            elif value['value_id'] == 'VALUE_ID_DEBUG':
                value_index.append('    VALUE_INDEX_DEBUG,')
            else:
                value_ids.append(f'    MeterValueID::{value["value_id"]},')
                value_index.append(f'    {current_index},')
                current_index += 1

        name = util.FlavoredName(spec['name'].format(variant=variant_spec)).get()

        spec_values.append(f'static const MeterModbusTCP::ValueSpec {name.under}_specs[] = {{\n' + '\n'.join(value_specs) + '\n};')

        if len(value_ids) > 0:
            spec_values.append(f'static const MeterValueID {name.under}_ids[] = {{\n' + '\n'.join(value_ids) + '\n};')

        spec_values.append(f'static const uint32_t {name.under}_index[] = {{\n' + '\n'.join(value_index) + '\n};')
        spec_values.append(f'static const MeterModbusTCP::ValueTable {name.under}_table = {{\n'
                           f'    {name.under}_specs,\n'
                           f'    ARRAY_SIZE({name.under}_specs),\r')

        if len(value_ids) > 0:
            spec_values.append(f'    {name.under}_ids,\n'
                               f'    ARRAY_SIZE({name.under}_ids),\r')
        else:
            spec_values.append('    nullptr,\n'
                               '    0,\r')

        spec_values.append(f'    {name.under}_index,\n'
                           '};')

with open('meters_modbus_tcp_defs.inc', 'w', encoding='utf-8') as f:
    f.write('// WARNING: This file is generated.\n\n')
    f.write('#define VALUE_INDEX_META  0xFFFFFFFEu\n')
    f.write('#define VALUE_INDEX_DEBUG 0xFFFFFFFDu\n\n')
    f.write('#define START_ADDRESS_VIRTUAL 0xFFFFFFFEu\n\n')
    f.write('\n\n'.join(spec_values).replace('\r\n', '') + '\n')
