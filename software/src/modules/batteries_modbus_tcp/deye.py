specs = [
    {
        'group': 'Deye Hybrid Inverter',
        'action': 'Permit Grid Charge',
        'table_prototype': ['device_address'],
        'default_device_address': 1,
        'register_blocks': [
            {
                'description': 'Time Points',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 148,
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
                'description': 'Target SoC (Full)',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 166,
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
                'description': 'Time Points Charge Enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 172,
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
        'table_prototype': ['device_address'],
        'default_device_address': 1,
        'register_blocks': [
            {
                'description': 'Target SoC (Minimum)',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 166,
                'values': [
                    10,
                    10,
                    10,
                    10,
                    10,
                    10,
                ],
            },
        ],
    },
    {
        'group': 'Deye Hybrid Inverter',
        'action': 'Forbid Discharge',
        'table_prototype': ['device_address'],
        'default_device_address': 1,
        'register_blocks': [
            {
                'description': 'Max A discharge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 109,
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Deye Hybrid Inverter',
        'action': 'Revoke Discharge Override',
        'table_prototype': ['device_address'],
        'default_device_address': 1,
        'register_blocks': [
            {
                'description': 'Max A discharge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 109,
                'values': [
                    50,
                ],
            },
        ],
    },
    {
        'group': 'Deye Hybrid Inverter',
        'action': 'Forbid Charge',
        'table_prototype': ['device_address'],
        'default_device_address': 1,
        'register_blocks': [
            {
                'description': 'Max A charge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 108,
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Deye Hybrid Inverter',
        'action': 'Revoke Charge Override',
        'table_prototype': ['device_address'],
        'default_device_address': 1,
        'register_blocks': [
            {
                'description': 'Max A charge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 108,
                'values': [
                    50,
                ],
            },
        ],
    },
]
