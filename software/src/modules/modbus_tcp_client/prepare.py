import sys
import tinkerforge_util as tfutil

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
