table_prototypes = [
    ('Sungrow Hybrid Inverter', [
        'device_address',
        {
            'name': 'grid_charge_power',
            'type': 'Uint16',  # FIXME: add range limit to [0..5000]
            'default': 1000,  # W
        },
        {
            'name': 'grid_discharge_power',
            'type': 'Uint16',  # FIXME: add range limit to [0..5000]
            'default': 1000,  # W
        },
        {
            'name': 'max_charge_power',
            'type': 'Uint16',  # FIXME: add range limit to [1..65535]
            'default': 3000,  # 0.01 kW
        },
        {
            'name': 'max_discharge_power',
            'type': 'Uint16',  # FIXME: add range limit to [1..65535]
            'default': 1500,  # 0.01 kW
        },
    ]),
]

default_device_addresses = [
    ('Sungrow Hybrid Inverter', 1),
]

repeat_intervals = [
    ('Sungrow Hybrid Inverter', 60),
]

specs = [
    {
        'group': 'Sungrow Hybrid Inverter',
        'mode': 'Block',
        'actions': ('Block', 'Block'),
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
                    0xCC,  # stop
                ],
            },
            {
                'description': 'Charge/discharge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13052,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Maximum charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33047,  # U16
                'values': [
                    1,  # minimum
                ],
            },
            {
                'description': 'Maximum discharge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33048,
                'values': [
                    1,  # minimum
                ],
            },
        ],
    },
    {
        'group': 'Sungrow Hybrid Inverter',
        'mode': 'Normal',
        'actions': ('Normal', 'Normal'),
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
            {
                'description': 'Charge/discharge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13052,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Maximum charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33047,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_charge_power;',
            },
            {
                'description': 'Maximum discharge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33048,
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_discharge_power;',
            },
        ],
    },
    {
        'group': 'Sungrow Hybrid Inverter',
        'mode': 'Charge From Excess',
        'actions': ('Normal', 'Block'),
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
            {
                'description': 'Charge/discharge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13052,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Maximum charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33047,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_charge_power;',
            },
            {
                'description': 'Maximum discharge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33048,
                'values': [
                    1,  # minimum
                ],
            },
        ],
    },
    {
        'group': 'Sungrow Hybrid Inverter',
        'mode': 'Charge From Grid',
        'actions': ('Force', 'Block'),
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
            {
                'description': 'Maximum charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33047,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_charge_power;',
            },
            {
                'description': 'Maximum discharge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33048,
                'values': [
                    1,  # minimum
                ],
            },
        ],
    },
    {
        'group': 'Sungrow Hybrid Inverter',
        'mode': 'Discharge To Load',
        'actions': ('Block', 'Normal'),
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
            {
                'description': 'Charge/discharge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13052,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Maximum charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33047,  # U16
                'values': [
                    1,  # minimum
                ],
            },
            {
                'description': 'Maximum discharge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33048,
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_discharge_power;',
            },
        ],
    },
    {
        'group': 'Sungrow Hybrid Inverter',
        'mode': 'Discharge To Grid',
        'actions': ('Block', 'Force'),
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
                    0xBB,  # discharge
                ],
            },
            {
                'description': 'Charge/discharge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 13052,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = grid_discharge_power;',
            },
            {
                'description': 'Maximum charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33047,  # U16
                'values': [
                    1,  # minimum
                ],
            },
            {
                'description': 'Maximum discharge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_number': 33048,
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_discharge_power;',
            },
        ],
    },
]
