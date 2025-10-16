table_prototypes = [
    ('Sungrow Hybrid Inverter', [
        'device_address',
        {
            'action': 'Permit Grid Charge',
            'name': 'grid_charge_power',
            'type': 'Uint16',
            'default': 1000,  # W
        },
        {
            'action': 'Revoke Discharge Override',
            'name': 'max_discharge_power',
            'type': 'Uint16',
            'default': 1500,  # 0.01 kW
        },
        {
            'action': 'Revoke Charge Override',
            'name': 'max_charge_power',
            'type': 'Uint16',
            'default': 3000,  # 0.01 kW
        },
    ]),
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
                'start_number': 13050,  # U16
                'values': [
                    2,  # forced
                ],
            },
            {
                'description': 'Charge/discharge command',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13051,  # U16
                'values': [
                    0xAA,  # charge
                ],
            },
            {
                'description': 'Charge/discharge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13052,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = grid_charge_power;',
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
                'start_number': 13050,  # U16
                'values': [
                    0,  # self
                ],
            },
            {
                'description': 'Charge/discharge command',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13051,  # U16
                'values': [
                    0xCC,  # stop
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
                'start_number': 33048,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_discharge_power;',
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
                'start_number': 33047,  # U16
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
                'start_number': 33047,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_charge_power;',
            },
        ],
    },
]
