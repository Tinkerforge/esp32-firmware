# FIXME: it would be better to use 43130/43131 instead of 43117/43118 to block
# charge/discharge because they have a dedicated "no limit" value so that the
# user would not need to specify the max_normal_dis/charge_current values. but
# during testing 43130/43131 did not have an effect.

display_names = [
    ('Solis Hybrid Inverter', {
        'en': 'Solis hybrid inverter',
        'de': 'Solis Hybrid-Wechselrichter',
    })
]

table_prototypes = [
    ('Solis Hybrid Inverter', [
        'device_address',
        {
            'name': 'max_normal_charge_current',
            'type': 'Uint16',
            'default': 500,  # 0.1 A
        },
        {
            'name': 'max_normal_discharge_current',
            'type': 'Uint16',
            'default': 500,  # 0.1 A
        },
        {
            'name': 'force_charge_power',
            'type': 'Uint16',
            'default': 500,  # 0.01 kW
        },
        {
            'name': 'force_discharge_power',
            'type': 'Uint16',
            'default': 500,  # 0.01 kW
        },
    ]),
]

default_device_addresses = [
    ('Solis Hybrid Inverter', 1),
]

repeat_intervals = [
    ('Solis Hybrid Inverter', 60),  # Solis watchdog duration is 5 minutes
]

specs = [
    {
        'group': 'Solis Hybrid Inverter',
        'mode': 'Block',
        'actions': ('Block', 'Block'),
        'register_blocks': [
            {
                'description': 'Battery max charge current [0.1 A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43117,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Battery max discharge current [0.1 A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43118,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Remote control force battery charge/discharge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43135,  # U16
                'values': [
                    0,  # off
                ],
            },
            {
                'description': 'Remote control force battery charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43136,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Remote control force battery discharge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43129,  # U16
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Solis Hybrid Inverter',
        'mode': 'Normal',
        'actions': ('Normal', 'Normal'),
        'register_blocks': [
            {
                'description': 'Battery max charge current [0.1 A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43117,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_normal_charge_current;',
            },
            {
                'description': 'Battery max discharge current [0.1 A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43118,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_normal_discharge_current;',
            },
            {
                'description': 'Remote control force battery charge/discharge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43135,  # U16
                'values': [
                    0,  # off
                ],
            },
            {
                'description': 'Remote control force battery charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43136,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Remote control force battery discharge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43129,  # U16
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Solis Hybrid Inverter',
        'mode': 'Charge From Excess',
        'actions': ('Normal', 'Block'),
        'register_blocks': [
            {
                'description': 'Battery max charge current [0.1 A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43117,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_normal_charge_current;',
            },
            {
                'description': 'Battery max discharge current [0.1 A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43118,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Remote control force battery charge/discharge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43135,  # U16
                'values': [
                    0,  # off
                ],
            },
            {
                'description': 'Remote control force battery charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43136,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Remote control force battery discharge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43129,  # U16
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Solis Hybrid Inverter',
        'mode': 'Charge From Grid',
        'actions': ('Force', 'Block'),
        'register_blocks': [
            {
                'description': 'Battery max charge current [0.1 A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43117,  # U16
                'values': [
                    65535,  # don't limit force charge
                ],
            },
            {
                'description': 'Battery max discharge current [0.1 A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43118,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Remote control force battery charge/discharge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43135,  # U16
                'values': [
                    1,  # force charge
                ],
            },
            {
                'description': 'Remote control force battery charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43136,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = force_charge_power;',
            },
            {
                'description': 'Remote control force battery discharge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43129,  # U16
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Solis Hybrid Inverter',
        'mode': 'Discharge To Load',
        'actions': ('Block', 'Normal'),
        'register_blocks': [
            {
                'description': 'Battery max charge current [0.1 A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43117,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Battery max discharge current [0.1 A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43118,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_normal_discharge_current;',
            },
            {
                'description': 'Remote control force battery charge/discharge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43135,  # U16
                'values': [
                    0,  # off
                ],
            },
            {
                'description': 'Remote control force battery charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43136,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Remote control force battery discharge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43129,  # U16
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Solis Hybrid Inverter',
        'mode': 'Discharge To Grid',
        'actions': ('Block', 'Force'),
        'register_blocks': [
            {
                'description': 'Battery max charge current [0.1 A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43117,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Battery max discharge current [0.1 A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43118,  # U16
                'values': [
                    65535,  # don't limit force discharge
                ],
            },
            {
                'description': 'Remote control force battery charge/discharge',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43135,  # U16
                'values': [
                    2,  # force discharge
                ],
            },
            {
                'description': 'Remote control force battery charge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43136,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Remote control force battery discharge power [0.01 kW]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43129,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = force_discharge_power;',
            },
        ],
    },
]
