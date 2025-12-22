table_prototypes = [
    ('Deye Hybrid Inverter', [
        'device_address',
        {
            'name': 'max_normal_charge_current',
            'type': 'Uint8',  # FIXME: add range limit to [1..185]
            'default': 50,  # A
        },
        {
            'name': 'max_normal_discharge_current',
            'type': 'Uint8',  # FIXME: add range limit to [1..185]
            'default': 50,  # A
        },
        {
            'name': 'force_charge_current',
            'type': 'Uint8',  # FIXME: add range limit to [1..185]
            'default': 25,  # A
        },
        {
            'name': 'force_discharge_current',
            'type': 'Uint8',  # FIXME: add range limit to [1..185]
            'default': 25,  # A
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
                    10,
                    10,
                    10,
                    10,
                    10,
                    10,
                ],
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
                'mapping': 'values[0] = max_normal_charge_current;',
            },
            {
                'description': 'Max discharge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 109,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_normal_discharge_current;',
            },
            {
                'description': 'Target SOC [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 166,  # U16
                'values': [
                    10,
                    10,
                    10,
                    10,
                    10,
                    10,
                ],
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
                'mapping': 'values[0] = max_normal_charge_current;',
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
                    10,
                    10,
                    10,
                    10,
                    10,
                    10,
                ],
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
                'mapping': 'values[0] = force_charge_current;',
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
                    100,
                    100,
                    100,
                    100,
                    100,
                    100,
                ],
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
                'mapping': 'values[0] = max_normal_discharge_current;',
            },
            {
                'description': 'Target SOC [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 166,  # U16
                'values': [
                    10,
                    10,
                    10,
                    10,
                    10,
                    10,
                ],
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
                'mapping': 'values[0] = force_discharge_current;',
            },
            {
                'description': 'Target SOC [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 166,  # U16
                'values': [
                    10,
                    10,
                    10,
                    10,
                    10,
                    10,
                ],
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
