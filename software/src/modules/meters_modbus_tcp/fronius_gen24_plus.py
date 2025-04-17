# Fronius GEN24 Plus abuses MPPT 3 and 4 of SunSpec model 160 to report battery information

from copy import deepcopy

battery_integer_values = [
    {
        'name': 'DCA_SF: Current Scale Factor | DCA_SF',
        'value_id': 'VALUE_ID_META',
        'start_address': 40256,
        'value_type': 'S16',  # SunSpec: sunssf
    },
    {
        'name': 'DCV_SF: Voltage Scale Factor | DCV_SF',
        'value_id': 'VALUE_ID_META',
        'start_address': 40257,
        'value_type': 'S16',  # SunSpec: sunssf
    },
    {
        'name': 'DCW_SF: Power Scale Factor | DCW_SF',
        'value_id': 'VALUE_ID_META',
        'start_address': 40258,
        'value_type': 'S16',  # SunSpec: sunssf
    },
    {
        'name': 'DCWH_SF: Energy Scale Factor | DCWH_SF',
        'value_id': 'VALUE_ID_META',
        'start_address': 40259,
        'value_type': 'S16',  # SunSpec: sunssf
    },
    {
        'name': 'module/3/DCA: DC Charge Current [A] | Charge DCA',
        'value_id': 'VALUE_ID_META',
        'start_address': 40313,
        'value_type': 'U16',  # SunSpec: uint16
    },
    {
        'name': 'module/3/DCV: DC Charge Voltage [V] | Charge DCV',
        'value_id': 'VALUE_ID_META',
        'start_address': 40314,
        'value_type': 'U16',  # SunSpec: uint16
    },
    {
        'name': 'module/3/DCW: DC Charge Power [W] | Charge DCW',
        'value_id': 'VALUE_ID_META',
        'start_address': 40315,
        'value_type': 'U16',  # SunSpec: uint16
    },
    {
        'name': 'module/3/DCWH: Lifetime Charge Energy [Wh] | Charge DCWH',
        'value_id': 'VALUE_ID_META',
        'start_address': 40316,
        'value_type': 'U32BE',  # SunSpec: acc32
    },
    {
        'name': 'module/4/DCA: DC Discharge Current [A] | Discharge DCA',
        'value_id': 'VALUE_ID_META',
        'start_address': 40333,
        'value_type': 'U16',  # SunSpec: uint16
    },
    {
        'name': 'module/4/DCV: DC Discharge Voltage [V] | Discharge DCV',
        'value_id': 'VALUE_ID_META',
        'start_address': 40334,
        'value_type': 'U16',  # SunSpec: uint16
    },
    {
        'name': 'module/4/DCW: DC Discharge Power [W] | Discharge DCW',
        'value_id': 'VALUE_ID_META',
        'start_address': 40335,
        'value_type': 'U16',  # SunSpec: uint16
    },
    {
        'name': 'module/4/DCWH: Lifetime Discharge Energy [Wh] | Discharge DCWH',
        'value_id': 'VALUE_ID_META',
        'start_address': 40336,
        'value_type': 'U32BE',  # SunSpec: acc32
    },
    {
        'name': 'ChaState: Currently available energy as a percent of the capacity rating [%] | ChaState',
        'value_id': 'VALUE_ID_META',
        'start_address': 40352,
        'value_type': 'U16',  # SunSpec: uint16
    },
    {
        'name': 'ChaState_SF: Scale factor for available energy percent | ChaState_SF',
        'value_id': 'VALUE_ID_META',
        'start_address': 40366,
        'value_type': 'S16',  # SunSpec: sunssf
    },
    {
        'name': 'Battery Current [A]',
        'value_id': 'CurrentDCChaDisDiff',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'Battery Voltage [V]',
        'value_id': 'VoltageDC',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'Battery Power [W]',
        'value_id': 'PowerDCChaDisDiff',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'Battery State Of Charge [%]',
        'value_id': 'StateOfCharge',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'Battery Charge Energy [kWh]',
        'value_id': 'EnergyDCCharge',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'Battery Discharge Energy [kWh]',
        'value_id': 'EnergyDCDischarge',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
]

def make_battery_values(start_address_offset):
    battery_values = deepcopy(battery_integer_values)

    for battery_value in battery_values:
        if battery_value['start_address'] != 'START_ADDRESS_VIRTUAL':
            battery_value['start_address'] += start_address_offset

    return battery_values

specs = [
    {
        'name': 'Fronius GEN24 Plus Battery Type',
        'register_type': 'HoldingRegister',
        'start_address_offset': 1,
        'values': [
            {
                'name': 'module/1/ID: Input ID or ID: SunSpec Model ID | Input ID or Model ID',
                'value_id': 'VALUE_ID_META',
                'start_address': 40264,
                'value_type': 'U16',
            },
        ],
    },
    {
        'name': 'Fronius GEN24 Plus Battery Integer',
        'register_type': 'HoldingRegister',
        'start_address_offset': 1,
        'values': make_battery_values(0),
    },
    {
        'name': 'Fronius GEN24 Plus Battery Float',
        'register_type': 'HoldingRegister',
        'start_address_offset': 1,
        'values': make_battery_values(10),
    },
]
