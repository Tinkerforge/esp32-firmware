# This is the same register table as Hailei

display_names = [
    ('Alpha ESS Hybrid Inverter', {
        'en': 'Alpha ESS hybrid inverter',
        'de': 'Alpha ESS Hybrid-Wechselrichter',
    }),
]

table_prototypes = [
    ('Alpha ESS Hybrid Inverter', [
        'device_address',
        {
            'name': 'force_charge_power',
            'type': 'Uint16',  # FIXME: add range limit to [0..32000]
            'default': 2000,  # W
        },
        {
            'name': 'force_discharge_power',
            'type': 'Uint16',  # FIXME: add range limit to [0..33535]
            'default': 2000,  # W
        },
    ]),
]

default_device_addresses = [
    ('Alpha ESS Hybrid Inverter', 85),
]

repeat_intervals = [
    ('Alpha ESS Hybrid Inverter', 60),
]

specs = [
    {
        'group': 'Alpha ESS Hybrid Inverter',
        'mode': 'Block',
        'register_blocks': [
            {
                'description': 'Dispatch',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0880,
                'values': [
                    1,         # start dispatch, U16
                    0, 32000,  # active power [W], U32BE
                    0, 32000,  # reactive power [var], U32BE, unused
                    2,         # state of charge control, U16
                    125,       # state of charge [0.4 %], U16, unused
                    0, 90,     # duration [s], U32BE
                ],
            },
        ],
    },
    {
        'group': 'Alpha ESS Hybrid Inverter',
        'mode': 'Normal',
        'register_blocks': [
            {
                'description': 'Dispatch',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0880,  # U16
                'values': [
                    0,  # stop dispatch
                ],
            },
        ],
    },
    {
        'group': 'Alpha ESS Hybrid Inverter',
        'mode': 'Block Discharge',
        'register_blocks': [
            {
                'description': 'Dispatch',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0880,
                'values': [
                    1,         # start dispatch, U16
                    0, 0,      # active power [W], U32BE
                    0, 32000,  # reactive power [var], U32BE, unused
                    1,         # battery only charges from PV, U16
                    250,       # state of charge [0.4 %], U16, unused
                    0, 90,     # duration [s], U32BE
                ],
            },
        ],
    },
    {
        'group': 'Alpha ESS Hybrid Inverter',
        'mode': 'Force Charge',
        'register_blocks': [
            {
                'description': 'Dispatch',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0880,
                'values': [
                    1,                                # start dispatch, U16
                    0, '32000 - force_charge_power',  # active power [W], U32BE
                    0, 32000,                         # reactive power [var], U32BE, unused
                    2,                                # state of charge control, U16
                    250,                              # state of charge [0.4 %], U16, 100 %
                    0, 90,                            # duration [s], U32BE
                ],
            },
        ],
    },
    {
        'group': 'Alpha ESS Hybrid Inverter',
        'mode': 'Block Charge',
        'register_blocks': [
            {
                'description': 'Dispatch',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0880,
                'values': [
                    1,         # start dispatch, U16
                    0, 32000,  # active power [W], U32BE, unused
                    0, 32000,  # reactive power [var], U32BE, unused
                    19,        # no battery charge, U16
                    0,         # state of charge [0.4 %], U16, unused
                    0, 90,     # duration [s], U32BE
                ],
            },
        ],
    },
    {
        'group': 'Alpha ESS Hybrid Inverter',
        'mode': 'Force Discharge',
        'register_blocks': [
            {
                'description': 'Dispatch',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0880,
                'values': [
                    1,                                   # start dispatch, U16
                    0, '32000 + force_discharge_power',  # active power [W], U32BE
                    0, 32000,                            # reactive power [var], U32BE, unused
                    2,                                   # state of charge control, U16
                    25,                                  # state of charge [0.4 %], U16, 10 %
                    0, 90,                               # duration [s], U32BE
                ],
            },
        ],
    },
]
