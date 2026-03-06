import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util


def make_modbus_value_type(register_count, is_signed, is_float, register_order_is_le):
    assert 1 <= register_count <= 4, register_count

    #      bit [0..2]              bit [3]                          bit [4]                         bit [5]
    return register_count | ((1 if is_signed else 0) << 3) | ((1 if is_float else 0) << 4) | ((1 if register_order_is_le else 0) << 5)


modbus_value_types = [
    util.EnumValue(util.FlavoredName('None').get(),  0),
    util.EnumValue(util.FlavoredName('U16').get(),   make_modbus_value_type(1, False, False, False)),
    util.EnumValue(util.FlavoredName('S16').get(),   make_modbus_value_type(1, True,  False, False)),
    util.EnumValue(util.FlavoredName('U32BE').get(), make_modbus_value_type(2, False, False, False)),
    util.EnumValue(util.FlavoredName('U32LE').get(), make_modbus_value_type(2, False, False, True)),
    util.EnumValue(util.FlavoredName('S32BE').get(), make_modbus_value_type(2, True,  False, False)),
    util.EnumValue(util.FlavoredName('S32LE').get(), make_modbus_value_type(2, True,  False, True)),
    util.EnumValue(util.FlavoredName('F32BE').get(), make_modbus_value_type(2, True,  True,  False)),
    util.EnumValue(util.FlavoredName('F32LE').get(), make_modbus_value_type(2, True,  True,  True)),
    util.EnumValue(util.FlavoredName('U64BE').get(), make_modbus_value_type(4, False, False, False)),
    util.EnumValue(util.FlavoredName('U64LE').get(), make_modbus_value_type(4, False, False, True)),
    util.EnumValue(util.FlavoredName('S64BE').get(), make_modbus_value_type(4, True,  False, False)),
    util.EnumValue(util.FlavoredName('S64LE').get(), make_modbus_value_type(4, True,  False, True)),
    util.EnumValue(util.FlavoredName('F64BE').get(), make_modbus_value_type(4, True,  True,  False)),
    util.EnumValue(util.FlavoredName('F64LE').get(), make_modbus_value_type(4, True,  True,  True)),
]

util.generate_enum('Modbus Value Type.uint8.enum', util.FlavoredName('Modbus TCP Client').get(), util.FlavoredName('Modbus Value Type').get(), 'uint8_t', modbus_value_types)
