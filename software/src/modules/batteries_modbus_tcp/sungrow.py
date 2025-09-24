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
                'description': 'EMS mode selection',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13050,
                'values': [
                    2,  # forced
                ],
            },
            {
                'description': 'Charge/discharge command',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13051,
                'values': [
                    0xAA,  # charge
                ],
            },
            {
                'description': 'Charge/discharge power [W]',
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
                'description': 'EMS mode selection',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13050,
                'values': [
                    0,  # self
                ],
            },
            {
                'description': 'Charge/discharge command',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13051,
                'values': [
                    0xCC,  # stop
                ],
            },
            {
                'description': 'Charge/discharge power [W]',
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
                'description': 'Maximum discharge power [0.01 kW]',
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
                'description': 'Maximum discharge power [0.01 kW]',
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
                'description': 'Maximum charge power [0.01 kW]',
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
                'description': 'Maximum charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33047,
                'values': [
                    3000,
                ],
            },
        ],
    },
]
