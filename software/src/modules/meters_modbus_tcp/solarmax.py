specs = [
    {
        'name': 'Solarmax Max Storage Inverter',
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
