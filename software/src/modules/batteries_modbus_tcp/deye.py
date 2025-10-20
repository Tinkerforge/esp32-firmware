table_prototypes = [
    ('Deye Hybrid Inverter', [
        'device_address',
        {
            'action': 'Permit Grid Charge',
            'name': 'max_soc',
            'type': 'Uint8',  # FIXME: add range limit to [0..100]
            'default': 100,  # %
        },
        {
            'action': 'Revoke Grid Charge Override',
            'name': 'min_soc',
            'type': 'Uint8',  # FIXME: add range limit to [0..100]
            'default': 10,  # %
        },
        {
            'action': 'Revoke Discharge Override',
            'name': 'max_discharge_current',
            'type': 'Uint8',  # FIXME: add range limit to [1..185]
            'default': 50,  # A
        },
        {
            'action': 'Revoke Charge Override',
            'name': 'max_charge_current',
            'type': 'Uint8',  # FIXME: add range limit to [1..185]
            'default': 50,  # A
        },
    ]),
]

default_device_addresses = [
    ('Deye Hybrid Inverter', 1),
]

specs = [
    {
        'group': 'Deye Hybrid Inverter',
        'action': 'Permit Grid Charge',
        'repeat_interval': 60,
        'register_blocks': [
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
                    2359,
                    2359,
                    2359,
                    2359,
                    2359,
                ],
            },
            {
                'description': 'Time points charge enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 172,  # U16
                'values': [
                    1,
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
        'action': 'Revoke Grid Charge Override',
        'repeat_interval': 60,
        'register_blocks': [
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
                    0,
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
        'action': 'Forbid Discharge',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Max discharge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 109,  # U16
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Deye Hybrid Inverter',
        'action': 'Revoke Discharge Override',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Max discharge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 109,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_discharge_current;',
            },
        ],
    },
    {
        'group': 'Deye Hybrid Inverter',
        'action': 'Forbid Charge',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 108,  # U16
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Deye Hybrid Inverter',
        'action': 'Revoke Charge Override',
        'repeat_interval': 60,
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
        ],
    },
]
