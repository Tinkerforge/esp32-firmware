table_prototypes = [
    ('Deye Hybrid Inverter', [
        'device_address',
        {
            'name': 'max_charge_current',
            'type': 'Uint8',  # FIXME: add range limit to [1..185]
            'default': 50,  # A
        },
        {
            'name': 'max_discharge_current',
            'type': 'Uint8',  # FIXME: add range limit to [1..185]
            'default': 50,  # A
        },
        {
            'name': 'min_soc',
            'type': 'Uint8',  # FIXME: add range limit to [0..100]
            'default': 10,  # %
        },
        {
            'name': 'max_soc',
            'type': 'Uint8',  # FIXME: add range limit to [0..100]
            'default': 100,  # %
        },
    ]),
]

default_device_addresses = [
    ('Deye Hybrid Inverter', 1),
]

repeat_intervals = [
    ('Deye Hybrid Inverter', 60),
]

specs = [
    {
        'group': 'Deye Hybrid Inverter',
        'mode': 'Block',
        'actions': ('Block', 'Block'),
        'register_blocks': [
            {
                'description': 'Max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 108,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Max discharge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 109,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Target SOC [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 166,  # U16
                'values': [
                    None,
                    None,
                    None,
                    None,
                    None,
                    None,
                ],
                'mapping': 'values[0] = min_soc;\n'
                           'values[1] = min_soc;\n'
                           'values[2] = min_soc;\n'
                           'values[3] = min_soc;\n'
                           'values[4] = min_soc;\n'
                           'values[5] = min_soc;\n',
            },
            {
                'description': 'Time points charge enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 172,  # U16
                'values': [
                    0,  # disable time points
                    0,
                    0,
                    0,
                    0,
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Deye Hybrid Inverter',
        'mode': 'Normal',
        'actions': ('Normal', 'Normal'),
        'register_blocks': [
            {
                'description': 'Max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 108,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_charge_current;',
            },
            {
                'description': 'Max discharge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 109,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_discharge_current;',
            },
            {
                'description': 'Target SOC [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 166,  # U16
                'values': [
                    None,
                    None,
                    None,
                    None,
                    None,
                    None,
                ],
                'mapping': 'values[0] = min_soc;\n'
                           'values[1] = min_soc;\n'
                           'values[2] = min_soc;\n'
                           'values[3] = min_soc;\n'
                           'values[4] = min_soc;\n'
                           'values[5] = min_soc;\n',
            },
            {
                'description': 'Time points charge enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 172,  # U16
                'values': [
                    0,  # disable time points
                    0,
                    0,
                    0,
                    0,
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Deye Hybrid Inverter',
        'mode': 'Charge From Excess',
        'actions': ('Normal', 'Block'),
        'register_blocks': [
            {
                'description': 'Max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 108,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_charge_current;',
            },
            {
                'description': 'Max discharge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 109,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Target SOC [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 166,  # U16
                'values': [
                    None,
                    None,
                    None,
                    None,
                    None,
                    None,
                ],
                'mapping': 'values[0] = max_soc;\n'
                           'values[1] = max_soc;\n'
                           'values[2] = max_soc;\n'
                           'values[3] = max_soc;\n'
                           'values[4] = max_soc;\n'
                           'values[5] = max_soc;\n',
            },
            {
                'description': 'Time points charge enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 172,  # U16
                'values': [
                    0,  # disable time points
                    0,
                    0,
                    0,
                    0,
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Deye Hybrid Inverter',
        'mode': 'Charge From Grid',
        'actions': ('Force', 'Block'),
        'register_blocks': [
            {
                'description': 'Max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 108,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_charge_current;',
            },
            {
                'description': 'Max discharge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 109,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Target SOC [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 166,  # U16
                'values': [
                    None,
                    None,
                    None,
                    None,
                    None,
                    None,
                ],
                'mapping': 'values[0] = max_soc;\n'
                           'values[1] = max_soc;\n'
                           'values[2] = max_soc;\n'
                           'values[3] = max_soc;\n'
                           'values[4] = max_soc;\n'
                           'values[5] = max_soc;\n',
            },
            {
                'description': 'Time points',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 148,  # U16
                'values': [
                    0,
                    2355,
                    2355,
                    2355,
                    2355,
                    2355,
                ],
            },
            {
                'description': 'Time points charge enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 172,  # U16
                'values': [
                    1,  # enable time points, grid buy
                    1,
                    1,
                    1,
                    1,
                    1,
                ],
            },
        ],
    },
    {
        'group': 'Deye Hybrid Inverter',
        'mode': 'Discharge To Load',
        'actions': ('Block', 'Normal'),
        'register_blocks': [
            {
                'description': 'Max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 108,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Max discharge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 109,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_discharge_current;',
            },
            {
                'description': 'Target SOC [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 166,  # U16
                'values': [
                    None,
                    None,
                    None,
                    None,
                    None,
                    None,
                ],
                'mapping': 'values[0] = min_soc;\n'
                           'values[1] = min_soc;\n'
                           'values[2] = min_soc;\n'
                           'values[3] = min_soc;\n'
                           'values[4] = min_soc;\n'
                           'values[5] = min_soc;\n',
            },
            {
                'description': 'Time points charge enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 172,  # U16
                'values': [
                    0,  # disable time points
                    0,
                    0,
                    0,
                    0,
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Deye Hybrid Inverter',
        'mode': 'Discharge To Grid',
        'actions': ('Block', 'Force'),
        'register_blocks': [
            {
                'description': 'Max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 108,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Max discharge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 109,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_discharge_current;',
            },
            {
                'description': 'Target SOC [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 166,  # U16
                'values': [
                    None,
                    None,
                    None,
                    None,
                    None,
                    None,
                ],
                'mapping': 'values[0] = min_soc;\n'
                           'values[1] = min_soc;\n'
                           'values[2] = min_soc;\n'
                           'values[3] = min_soc;\n'
                           'values[4] = min_soc;\n'
                           'values[5] = min_soc;\n',
            },
            {
                'description': 'Time points',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 148,  # U16
                'values': [
                    0,
                    2355,
                    2355,
                    2355,
                    2355,
                    2355,
                ],
            },
            {
                'description': 'Time points charge enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 172,  # U16
                'values': [
                    32,  # enable time points, grid sell
                    32,
                    32,
                    32,
                    32,
                    32,
                ],
            },
        ],
    },
]
