# This is the same register table as Alpha ESS

specs = [
    {
        'group': 'Hailei Hybrid Inverter',
        'action': 'Permit Grid Charge',
        'table_prototype': ['device_address'],
        'default_device_address': 85,
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Time period control flag',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x084F,
                'values': [
                    1,
                ],
            },
            {
                'description': 'Charge Cut Soc',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0855,
                'values': [
                    100,
                ],
            },
            {
                'description': 'Time Charge Hours',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0856,
                'values': [
                    0,
                    23,
                    23,
                    23,
                ],
            },
            {
                'description': 'Time Charge Minutes',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x085E,
                'values': [
                    0,
                    59,
                    59,
                    59,
                ],
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'action': 'Revoke Grid Charge Override',
        'table_prototype': ['device_address'],
        'default_device_address': 85,
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Time period control flag',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x084F,
                'values': [
                    0,
                ],
            },
            {
                'description': 'Charge Cut Soc',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0855,
                'values': [
                    10,
                ],
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'action': 'Forbid Discharge',
        'table_prototype': ['device_address'],
        'default_device_address': 85,
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Time period control flag',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x084F,
                'values': [
                    2,
                ],
            },
            {
                'description': 'Time Discharge Hours',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0851,
                'values': [
                    0,
                    23,
                    23,
                    23,
                ],
            },
            {
                'description': 'Time Discharge Minutes',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x085A,
                'values': [
                    0,
                    59,
                    59,
                    59,
                ],
            },
            {
                'description': 'UPS reserve enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0862,
                'values': [
                    1,
                ],
            },
            {
                'description': 'UPS Reserve Soc',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0850,
                'values': [
                    100,
                ],
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'action': 'Revoke Discharge Override',
        'table_prototype': ['device_address'],
        'default_device_address': 85,
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'UPS reserve enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0862,
                'values': [
                    0,
                ],
            },
            {
                'description': 'UPS Reserve Soc',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0850,
                'values': [
                    10,
                ],
            },
            {
                'description': 'Time period control flag',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x084F,
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'action': 'Forbid Charge',
        'table_prototype': ['device_address'],
        'default_device_address': 85,
        'repeat_interval': 60,
        'register_blocks': [
            # FIXME
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'action': 'Revoke Charge Override',
        'table_prototype': ['device_address'],
        'default_device_address': 85,
        'repeat_interval': 60,
        'register_blocks': [
            # FIXME
        ],
    },
]
