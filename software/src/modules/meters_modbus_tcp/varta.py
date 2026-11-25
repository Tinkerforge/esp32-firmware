display_names = [
    ('VARTA Element', {
        'en': 'VARTA element / one / pulse / link',
        'de': 'VARTA element / one / pulse / link',
    }),
    ('VARTA Flex', {
        'en': 'VARTA pulse neo / flex storage',
        'de': 'VARTA pulse neo / flex storage',
    }),
]

table_prototypes = [
    ('VARTA Element', ['device_address', 'virtual_meter']),
    ('VARTA Flex', ['device_address', 'virtual_meter']),
]

default_device_addresses = [
    ('VARTA Element', 255),
    ('VARTA Flex', 255),
]

specs = [
    {
        'name': 'VARTA Element Grid',  # for models element, one, pulse and link
        'table_id': 'VARTA Element',
        'selection': 'primary',
        'virtual_meter': 'Grid',
        'fixed_location': 'Grid',
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
        'table_id': 'VARTA Element',
        'selection': 'primary',
        'virtual_meter': 'Battery',
        'fixed_location': 'Battery',
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
                'name': 'Total Charge Energy [0.001 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 1069,
                'value_type': 'U32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Installed Capacity [0.01 kWh]',
                'value_id': 'Capacity',
                'start_address': 1071,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
        ],
    },
    {
        'name': 'VARTA Flex Grid',  # for models pulse neo and flex storage
        'table_id': 'VARTA Flex',
        'selection': 'primary',
        'virtual_meter': 'Grid',
        'fixed_location': 'Grid',
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
                'name': 'Undocumented 1',  # this value is undocumented, but testing showed that it can be read without error
                'value_id': 'VALUE_ID_GAP_FILLER',
                'start_address': 1079,
                'value_type': 'U16',
            },
            {
                'name': 'Undocumented 2',  # this value is undocumented, but testing showed that it can be read without error
                'value_id': 'VALUE_ID_GAP_FILLER',
                'start_address': 1080,
                'value_type': 'U16',
            },
            {
                'name': 'Undocumented 3',  # this value is undocumented, but testing showed that it can be read without error
                'value_id': 'VALUE_ID_GAP_FILLER',
                'start_address': 1081,
                'value_type': 'U16',
            },
            {
                'name': 'Grid Frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 1082,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Grid Power SF',
                'value_id': 'VALUE_ID_META',
                'start_address': 2078,
                'value_type': 'S16',
            },
            {
                'name': 'Grid Power Scaled [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
        ],
    },
    {
        'name': 'VARTA Flex Battery',  # for models pulse neo and flex storage
        'table_id': 'VARTA Flex',
        'selection': 'primary',
        'virtual_meter': 'Battery',
        'fixed_location': 'Battery',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Active Power [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 1066,
                'value_type': 'S16',
            },
            {
                'name': 'Apparent Power [VA]',
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
                'name': 'Total Charge Energy [0.001 kWh]',
                'value_id': 'VALUE_ID_META',
                'start_address': 1069,
                'value_type': 'U32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Installed Capacity [0.01 kWh]',
                'value_id': 'VALUE_ID_META',
                'start_address': 1071,
                'value_type': 'U16',
                'scale_factor': 0.01,
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
                'name': 'Undocumented 1',  # this value is undocumented, but testing showed that it can be read without error, if it is read in between documented values
                'value_id': 'VALUE_ID_GAP_FILLER',
                'start_address': 2068,
                'value_type': 'U16',
            },
            {
                'name': 'Total Charge Energy SF',
                'value_id': 'VALUE_ID_META',
                'start_address': 2069,
                'value_type': 'S16',
            },
            {
                'name': 'Undocumented 2',  # this value is undocumented, but testing showed that it can be read without error, if it is read in between documented values
                'value_id': 'VALUE_ID_GAP_FILLER',
                'start_address': 2070,
                'value_type': 'U16',
            },
            {
                'name': 'Installed Capacity SF',
                'value_id': 'VALUE_ID_META',
                'start_address': 2071,
                'value_type': 'S16',
            },
            {
                'name': 'Active Power Scaled [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Apparent Power Scaled [W]',
                'value_id': 'PowerApparentLSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Total Charge Energy Scaled [kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Installed Capacity Scaled [kWh]',
                'value_id': 'Capacity',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
        ],
    },
]
