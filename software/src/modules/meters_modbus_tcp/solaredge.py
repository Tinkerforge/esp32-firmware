display_names = [
    ('Solaredge Inverter', {
        'en': 'SolarEdge inverter',
        'de': 'SolarEdge Wechselrichter',
    }),
]

table_prototypes = [
    ('Solaredge Inverter', ['device_address', 'virtual_meter']),
]

default_device_addresses = [
    ('Solaredge Inverter', 1),
]

specs = [
    {
        'name': 'Solaredge Inverter Battery',
        'virtual_meter': ('Solaredge Inverter', 'Battery'),
        'fixed_location': 'Battery',
        'register_type': 'HoldingRegister',
        'f32_negative_max_as_nan': True,
        'values': [
            {
                'name': 'Battery 1 Average Temperature [°C]',
                'value_id': 'VALUE_ID_META',
                'start_address': 0xE16C,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery 1 Instantaneous Voltage [V]',
                'value_id': 'VALUE_ID_META',
                'start_address': 0xE170,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery 1 Instantaneous Current [A]',
                'value_id': 'VALUE_ID_META',
                'start_address': 0xE172,
                'value_type': 'F32LE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Battery 1 Instantaneous Power [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 0xE174,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery 1 Lifetime Export Energy Counter [0.001 kWh]',
                'value_id': 'VALUE_ID_META',
                'start_address': 0xE176,
                'value_type': 'U64LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Battery 1 Lifetime Import Energy Counter [0.001 kWh]',
                'value_id': 'VALUE_ID_META',
                'start_address': 0xE17A,
                'value_type': 'U64LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Battery 1 State of Energy [%]',
                'value_id': 'VALUE_ID_META',
                'start_address': 0xE184,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery 2 Average Temperature [°C]',
                'value_id': 'Temperature',
                'start_address': 0xE26C,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery 2 Instantaneous Voltage [V]',
                'value_id': 'VoltageDC',
                'start_address': 0xE270,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery 2 Instantaneous Current [A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 0xE272,
                'value_type': 'F32LE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Battery 2 Instantaneous Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 0xE274,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery 2 Lifetime Export Energy Counter [0.001 kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 0xE276,
                'value_type': 'U64LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Battery 2 Lifetime Import Energy Counter [0.001 kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 0xE27A,
                'value_type': 'U64LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Battery 2 State of Energy [%]',
                'value_id': 'StateOfCharge',
                'start_address': 0xE284,
                'value_type': 'F32LE',
            },
        ],
    },
    {
        'name': 'Solaredge Inverter PV',
        'virtual_meter': ('Solaredge Inverter', 'PV'),
        'fixed_location': 'PV',
        'register_type': 'HoldingRegister',
        'f32_negative_max_as_nan': True,
        'values': [
            {
                'name': 'Battery 1 Instantaneous Power [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 0xE174,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery 2 Instantaneous Power [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 0xE274,
                'value_type': 'F32LE',
            },
            {
                'name': 'DC Power [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 40100,
                'value_type': 'S16',
            },
            {
                'name': 'DC Power_SF',
                'value_id': 'VALUE_ID_META',
                'start_address': 40101,
                'value_type': 'S16',
            },
            {
                'name': 'PV Power [W]',
                'value_id': 'PowerPVSumExport',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
        ],
    },
]
