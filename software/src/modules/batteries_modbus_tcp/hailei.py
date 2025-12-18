# This is the same register table as Alpha ESS

table_prototypes = [
    ('Hailei Hybrid Inverter', [
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
    ('Hailei Hybrid Inverter', 85),
]

repeat_intervals = [
    ('Hailei Hybrid Inverter', 60),
]

specs = [
    {
        'group': 'Hailei Hybrid Inverter',
        'mode': 'Block',
        'actions': ('Block', 'Block'),
        'register_blocks': [
            {
                'description': 'Dispatch',  # block charge and discharge
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
        'group': 'Hailei Hybrid Inverter',
        'mode': 'Normal',
        'actions': ('Normal', 'Normal'),
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
        'group': 'Hailei Hybrid Inverter',
        'mode': 'Charge From Excess',
        'actions': ('Normal', 'Block'),
        'register_blocks': [
            {
                'description': 'Dispatch',  # block discharge
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
        'group': 'Hailei Hybrid Inverter',
        'mode': 'Charge From Grid',
        'actions': ('Force', 'Block'),
        'register_blocks': [
            {
                'description': 'Dispatch',  # force charge
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0880,
                'values': [
                    1,         # start dispatch, U16
                    0, None,   # active power [W], U32BE
                    0, 32000,  # reactive power [var], U32BE, unused
                    2,         # state of charge control, U16
                    250,       # state of charge [0.4 %], U16, 100 %
                    0, 90,     # duration [s], U32BE
                ],
                'mapping': 'values[2] = 32000 - force_charge_power;',
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'mode': 'Discharge To Load',
        'actions': ('Block', 'Normal'),
        'register_blocks': [
            {
                'description': 'Dispatch',  # block charge
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
        'group': 'Hailei Hybrid Inverter',
        'mode': 'Discharge To Grid',
        'actions': ('Block', 'Force'),
        'register_blocks': [
            {
                'description': 'Dispatch',  # force discharge
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0880,
                'values': [
                    1,         # start dispatch, U16
                    0, None,   # active power [W], U32BE
                    0, 32000,  # reactive power [var], U32BE, unused
                    2,         # state of charge control, U16
                    25,        # state of charge [0.4 %], U16, 10 %
                    0, 90,     # duration [s], U32BE
                ],
                'mapping': 'values[2] = 32000 + force_discharge_power;',
            },
        ],
    },
]
