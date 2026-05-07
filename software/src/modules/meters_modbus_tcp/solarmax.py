display_names = [
    ('Solarmax Max Storage', {
        'en': 'Solarmax Max.Storage',
        'de': 'Solarmax Max.Storage',
    }),
]

table_prototypes = [
    ('Solarmax Max Storage', ['device_address', 'virtual_meter']),
]

default_device_addresses = [
    ('Solarmax Max Storage', 1),
]

specs = [
    {
        'name': 'Solarmax Max Storage Inverter',
        'virtual_meter': ('Solarmax Max Storage', 'Inverter'),
        'fixed_location': 'Inverter',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Inverter DC Power [W]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 110,
                'value_type': 'U32LE',
            },
            {
                'name': 'Total DC Power [W]',
                'value_id': 'PowerDCExport',
                'start_address': 112,
                'value_type': 'U32LE',
            },
            {
                'name': 'Direct Power Usage [W]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 116,
                'value_type': 'U32LE',
            },
            {
                'name': 'Active Power [W]',
                'value_id': 'PowerActiveLSumExport',
                'start_address': 120,
                'value_type': 'U32LE',
            },
        ],
    },
    {
        'name': 'Solarmax Max Storage Grid',
        'virtual_meter': ('Solarmax Max Storage', 'Grid'),
        'fixed_location': 'Grid',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Export Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 118,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
            },
        ],
    },
    {
        'name': 'Solarmax Max Storage Battery',
        'virtual_meter': ('Solarmax Max Storage', 'Battery'),
        'fixed_location': 'Battery',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 114,
                'value_type': 'S32LE',
            },
            {
                'name': 'Battery SoC [%]',
                'value_id': 'StateOfCharge',
                'start_address': 122,
                'value_type': 'U16',
            },
        ],
    },
]
