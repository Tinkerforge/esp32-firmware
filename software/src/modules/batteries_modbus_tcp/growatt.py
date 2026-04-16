display_names = [
    ('Growatt Hybrid Inverter', {
        'en': 'Growatt hybrid inverter (TL-X and TL-XH series)',
        'de': 'Growatt Hybrid-Wechselrichter (TL-X- und TL-XH-Serie)',
    }),
]

table_prototypes = [
    ('Growatt Hybrid Inverter', [
        'device_address',
        {
            'name': 'force_charge_rate',
            'type': 'Uint8',  # FIXME: add range limit to [1..100]
            'default': 100,  # %
        },
        {
            'name': 'force_discharge_rate',
            'type': 'Uint8',  # FIXME: add range limit to [1..100]
            'default': 100,  # %
        },
    ]),
]

default_device_addresses = [
    ('Growatt Hybrid Inverter', 1),
]

repeat_intervals = [
    ('Growatt Hybrid Inverter', 60),
]

specs = [
    {
        'group': 'Growatt Hybrid Inverter',
        'mode': 'Block',
        'register_blocks': [
            {
                'description': 'Discharge Power Rate [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3036,
                'values': [
                    1,  # minimum
                ],
            },
            {
                'description': 'Charge/Discharge Mode, Start/End Time',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3038,
                'values': [
                    32768, 5947,  # load first
                ],
            },
            {
                'description': 'Charge Power Rate [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3047,
                'values': [
                    1,  # minimum
                ],
            },
            {
                'description': 'AC charge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3049,
                'values': [
                    0,  # disable
                ],
            },
        ],
    },
    {
        'group': 'Growatt Hybrid Inverter',
        'mode': 'Normal',
        'register_blocks': [
            {
                'description': 'Discharge Power Rate [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3036,
                'values': [
                    100,  # maximum
                ],
            },
            {
                'description': 'Charge/Discharge Mode, Start/End Time',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3038,
                'values': [
                    32768, 5947,  # load first
                ],
            },
            {
                'description': 'Charge Power Rate [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3047,
                'values': [
                    100,  # maximum
                ],
            },
            {
                'description': 'AC Charge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3049,
                'values': [
                    0,  # disable
                ],
            },
        ],
    },
    {
        'group': 'Growatt Hybrid Inverter',
        'mode': 'Block Discharge',
        'register_blocks': [
            {
                'description': 'Discharge Power Rate [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3036,
                'values': [
                    1,  # minimum
                ],
            },
            {
                'description': 'Charge/Discharge Mode, Start/End Time',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3038,
                'values': [
                    32768, 5947,  # load first
                ],
            },
            {
                'description': 'Charge Power Rate [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3047,
                'values': [
                    100,  # maximum
                ],
            },
            {
                'description': 'AC Charge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3049,
                'values': [
                    0,  # disable
                ],
            },
        ],
    },
    {
        'group': 'Growatt Hybrid Inverter',
        'mode': 'Force Charge',
        'register_blocks': [
            {
                'description': 'Discharge Power Rate [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3036,
                'values': [
                    1,  # minimum
                ],
            },
            {
                'description': 'Charge/Discharge Mode, Start/End Time',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3038,
                'values': [
                    40960, 5947,  # battery first
                ],
            },
            {
                'description': 'Charge Power Rate [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3047,
                'values': [
                    'force_charge_rate',
                ],
            },
            {
                'description': 'AC Charge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3049,
                'values': [
                    1,  # enable
                ],
            },
        ],
    },
    {
        'group': 'Growatt Hybrid Inverter',
        'mode': 'Block Charge',
        'register_blocks': [
            {
                'description': 'Discharge Power Rate [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3036,
                'values': [
                    100,  # maximum
                ],
            },
            {
                'description': 'Charge/Discharge Mode, Start/End Time',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3038,
                'values': [
                    32768, 5947,  # load first
                ],
            },
            {
                'description': 'Charge Power Rate [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3047,
                'values': [
                    1,  # minimum
                ],
            },
            {
                'description': 'AC Charge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3049,
                'values': [
                    0,  # disable
                ],
            },
        ],
    },
    {
        'group': 'Growatt Hybrid Inverter',
        'mode': 'Force Discharge',
        'register_blocks': [
            {
                'description': 'Discharge Power Rate [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3036,
                'values': [
                    'force_discharge_rate',
                ],
            },
            {
                'description': 'Charge/Discharge Mode, Start/End Time',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3038,
                'values': [
                    49152, 5947,  # grid first
                ],
            },
            {
                'description': 'Charge Power Rate [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3047,
                'values': [
                    1,  # minimum
                ],
            },
            {
                'description': 'AC Charge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 3049,
                'values': [
                    0,  # disable
                ],
            },
        ],
    },
]
