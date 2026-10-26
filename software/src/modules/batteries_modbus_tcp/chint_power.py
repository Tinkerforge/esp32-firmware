display_names = [
    ('Chint Power Hybrid Inverter', {
        'en': 'Chint Power hybrid inverter',
        'de': 'Chint Power Hybrid-Wechselrichter',
    }),
]

table_prototypes = [
    ('Chint Power Hybrid Inverter', [
        'device_address',
        {
            'name': 'force_charge_power',
            'type': 'Uint16',  # FIXME: add range limit to [0..200]
            'default': 20,  # 0.1 kW
        },
        {
            'name': 'force_discharge_power',
            'type': 'Uint16',  # FIXME: add range limit to [0..1500]
            'default': 20,  # 0.1 kW
        },
    ]),
]

default_device_addresses = [
    ('Chint Power Hybrid Inverter', 1),
]

repeat_intervals = [
    ('Chint Power Hybrid Inverter', 60),  # Chint Power has no watchdog, but the repeat is for the robustness of the battery control
]

variants = []

specs = [
    {
        'group': 'Chint Power Hybrid Inverter',
        'mode': 'Block',
        'register_blocks': [
            {
                'description': 'Battery charge current limit [A]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x40E3,  # S16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Battery discharge current limit [A]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x40E4,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'System working mode',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x41D0,  # U16
                'values': [
                    1,  # self-consumption
                ],
            },
        ],
    },
    {
        'group': 'Chint Power Hybrid Inverter',
        'mode': 'Normal',
        'register_blocks': [
            {
                'description': 'Battery charge current limit [A]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x40E3,  # S16
                'values': [
                    64986,  # == -550, maximum
                ],
            },
            {
                'description': 'Battery discharge current limit [A]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x40E4,  # U16
                'values': [
                    550,  # maximum
                ],
            },
            {
                'description': 'System working mode',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x41D0,  # U16
                'values': [
                    1,  # self-consumption
                ],
            },
        ],
    },
    {
        'group': 'Chint Power Hybrid Inverter',
        'mode': 'Block Discharge',
        'register_blocks': [
            {
                'description': 'Battery charge current limit [A]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x40E3,  # S16
                'values': [
                    64986,  # == -550, maximum
                ],
            },
            {
                'description': 'Battery discharge current limit [A]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x40E4,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'System working mode',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x41D0,  # U16
                'values': [
                    1,  # self-consumption
                ],
            },
        ],
    },
    {
        'group': 'Chint Power Hybrid Inverter',
        'mode': 'Force Charge',
        'register_blocks': [
            {
                'description': 'Battery charge current limit [A]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x40E3,  # S16
                'values': [
                    64986,  # == -550, maximum
                ],
            },
            {
                'description': 'Battery discharge current limit [A]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x40E4,  # U16
                'values': [
                    0,
                ],
            },
            {
                'description': 'System working mode',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x41D0,  # U16
                'values': [
                    3,  # emergency charging
                ],
            },
            {
                'description': 'Grid power limit function',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x41D1,  # U16
                'values': [
                    0,  # disabled
                ],
            },
            {
                'description': 'From grid charging power limit [0.1 kW]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x41D4,  # S16
                'values': [
                    'force_charge_power',
                ],
            },
            {
                'description': 'From grid charging function',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x41DB,  # U16
                'values': [
                    1,  # enabled
                ],
            },
        ],
    },
    {
        'group': 'Chint Power Hybrid Inverter',
        'mode': 'Block Charge',
        'register_blocks': [
            {
                'description': 'Battery charge current limit [A]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x40E3,  # S16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Battery discharge current limit [A]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x40E4,  # U16
                'values': [
                    550,  # maximum
                ],
            },
            {
                'description': 'System working mode',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x41D0,  # U16
                'values': [
                    1,  # self-consumption
                ],
            },
        ],
    },
    {
        'group': 'Chint Power Hybrid Inverter',
        'mode': 'Force Discharge',
        'register_blocks': [
            {
                'description': 'Battery charge current limit [A]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x40E3,  # S16
                'values': [
                    0,
                ],
            },
            {
                'description': 'Battery discharge current limit [A]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x40E4,  # U16
                'values': [
                    550,  # maximum
                ],
            },
            {
                'description': 'System working mode',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x41D0,  # U16
                'values': [
                    2,  # fully feed
                ],
            },
            {
                'description': 'Grid power limit function',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x41D1,  # U16
                'values': [
                    0,  # disabled
                ],
            },
            {
                'description': 'Feed grid power limit [0.1 kW]',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0x41D2,  # S16
                'values': [
                    'force_discharge_power',
                ],
            },
        ],
    },
]
