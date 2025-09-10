specs = [
    {
        'group': 'Victron Energy GX',
        'action': 'Permit Grid Charge',
        'default_device_address': 100,
        'register_blocks': [
            {
                'description': 'Set AC grid setpoint override to 65 kW',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,
                'values': [
                    0,
                    65534,
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'action': 'Revoke Grid Charge Override',
        'default_device_address': 100,
        'register_blocks': [
            {
                'description': 'Set AC grid setpoint override to 0 W',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,
                'values': [
                    0,
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'action': 'Forbid Discharge',
        'default_device_address': 100,
        'register_blocks': [
            {
                'description': 'Set ESS max discharge current to 0 A',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2704,
                'values': [
                    32767
                ],
            },
            {
                'description': 'Set ESS max discharge current to 0 %',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2702,
                'values': [
                    0
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'action': 'Revoke Discharge Override',
        'default_device_address': 100,
        'register_blocks': [
            {
                'description': 'Set ESS max discharge current to -1 A',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2704,
                'values': [
                    65535
                ],
            },
            {
                'description': 'Set ESS max discharge current to 100 %',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2702,
                'values': [
                    100
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'action': 'Forbid Charge',
        'default_device_address': 100,
        'register_blocks': [
            {
                'description': 'Set ESS max charge current to 0 %',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2701,
                'values': [
                    0
                ],
            },
            {
                'description': 'Set ESS max charge current to 0 A',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2705,
                'values': [
                    0
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'action': 'Revoke Charge Override',
        'default_device_address': 100,
        'register_blocks': [
            {
                'description': 'Set ESS max charge current to 100 %',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2701,
                'values': [
                    100
                ],
            },
            {
                'description': 'Set ESS max charge current to 155 A',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2705,
                'values': [
                    155
                ],
            },
        ],
    },
]
