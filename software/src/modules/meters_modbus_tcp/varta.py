default_device_addresses = [
    ('VARTA Element', 255),
    ('VARTA Flex', 255),
]

specs = [
    {
        'name': 'VARTA Element Grid',  # for models element, one, pulse and link
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Grid Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 1078,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
        ],
    },
    {
        'name': 'VARTA Element Battery',  # for models element, one, pulse and link
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Active Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 1066,
                'value_type': 'S16',
            },
            {
                'name': 'Apparent Power [VA]',
                'value_id': 'PowerApparentLSumImExDiff',
                'start_address': 1067,
                'value_type': 'S16',
            },
            {
                'name': 'SOC [%]',
                'value_id': 'StateOfCharge',
                'start_address': 1068,
                'value_type': 'U16',
            },
            {
                'name': 'Total Charge Energy [Wh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 1069,
                'value_type': 'U32LE',
                'scale_factor': 0.001,
            },
        ],
    },
    {
        'name': 'VARTA Flex Grid',  # for models pulse neo and flex storage
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Grid Power [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 1078,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
            {
                'name': 'Grid Frequency [0.1 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 1082,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Power SF',
                'value_id': 'VALUE_ID_META',
                'start_address': 2078,
                'value_type': 'S16',
            },
            {
                'name': 'Scaled Grid Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
        ],
    },
    {
        'name': 'VARTA Flex Battery',  # for models pulse neo and flex storage
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Active Power',
                'value_id': 'VALUE_ID_META',
                'start_address': 1066,
                'value_type': 'S16',
            },
            {
                'name': 'Apparent Power',
                'value_id': 'VALUE_ID_META',
                'start_address': 1067,
                'value_type': 'S16',
            },
            {
                'name': 'SOC [%]',
                'value_id': 'StateOfCharge',
                'start_address': 1068,
                'value_type': 'U16',
            },
            {
                'name': 'Total Charge Energy',
                'value_id': 'VALUE_ID_META',
                'start_address': 1069,
                'value_type': 'U32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Active Power SF',
                'value_id': 'VALUE_ID_META',
                'start_address': 2066,
                'value_type': 'S16',
            },
            {
                'name': 'Apparent Power SF',
                'value_id': 'VALUE_ID_META',
                'start_address': 2067,
                'value_type': 'S16',
            },
            {
                'name': 'Total Charge Energy SF',
                'value_id': 'VALUE_ID_META',
                'start_address': 2069,
                'value_type': 'S16',
            },
            {
                'name': 'Scaled Active Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Scaled Apparent Power [W]',
                'value_id': 'PowerApparentLSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Scaled Total Charge Energy [kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
        ],
    },
]
