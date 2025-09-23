table_prototypes = [
    ('Sungrow Hybrid Inverter', ['device_address']),
]

default_device_addresses = [
    ('Sungrow Hybrid Inverter', 1),
]

specs = [
    {
        'group': 'Sungrow Hybrid Inverter',
        'action': 'Permit Grid Charge',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'EMS Mode Selection = Forced',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13050,
                'values': [
                    2,
                ],
            },
            {
                'description': 'Charge/Discharge Command = Charge',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13051,
                'values': [
                    0xAA,
                ],
            },
            {
                'description': 'Charge/Discharge Power = 1000W',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13052,
                'values': [
                    1000,
                ],
            },
        ],
    },
    {
        'group': 'Sungrow Hybrid Inverter',
        'action': 'Revoke Grid Charge Override',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'EMS Mode Selection = Self',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13050,
                'values': [
                    0,
                ],
            },
            {
                'description': 'Charge/Discharge Command = Stop',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13051,
                'values': [
                    0xCC,
                ],
            },
            {
                'description': 'Charge/Discharge Power = 0W',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13052,
                'values': [
                    0,  # FIXME: should this really be set to 0W?
                ],
            },
        ],
    },
    {
        'group': 'Sungrow Hybrid Inverter',
        'action': 'Forbid Discharge',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Max discharge power = 1',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33048,
                'values': [
                    1,
                ],
            },
        ],
    },
    {
        'group': 'Sungrow Hybrid Inverter',
        'action': 'Revoke Discharge Override',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Max discharge power = 1500',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33048,
                'values': [
                    1500,
                ],
            },
        ],
    },
    {
        'group': 'Sungrow Hybrid Inverter',
        'action': 'Forbid Charge',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Max charge power = 1',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33047,
                'values': [
                    1,
                ],
            },
        ],
    },
    {
        'group': 'Sungrow Hybrid Inverter',
        'action': 'Revoke Charge Override',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Max charge power = 3000',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33047,
                'values': [
                    3000,
                ],
            },
        ],
    },
]
