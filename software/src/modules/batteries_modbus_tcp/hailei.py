# This is the same register table as Alpha ESS

table_prototypes = [
    ('Hailei Hybrid Inverter', [
        'device_address',
        {
            'action': 'Permit Grid Charge',
            'name': 'max_soc',
            'type': 'Uint8',  # FIXME: add range limit to [0..100]
            'default': 100,  # %
        },
        {
            'action': 'Revoke Discharge Override',
            'name': 'min_soc',
            'type': 'Uint8',  # FIXME: add range limit to [0..100]
            'default': 10,  # %
        },
    ]),
]

default_device_addresses = [
    ('Hailei Hybrid Inverter', 85),
]

specs = [
    {
        'group': 'Hailei Hybrid Inverter',
        'action': 'Permit Grid Charge',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Time charge start hours',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0856,  # U16
                'values': [
                    0,
                    23,
                    23,
                    23,
                ],
            },
            {
                'description': 'Time charge start minutes',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x085E,  # U16
                'values': [
                    0,
                    59,
                    59,
                    59,
                ],
            },
            {
                'description': 'Time period control flag',
                'function_code': 'MaskWriteRegister',
                'start_address': 0x084F,  # U16
                'values': [
                    0b10, 0b01  # leave bit 1 unchanged, set bit 0 to enable charge time period control
                ],
            },
            {
                'description': 'Charge cut SOC [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0855,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = max_soc;',
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'action': 'Revoke Grid Charge Override',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Time period control flag',
                'function_code': 'MaskWriteRegister',
                'start_address': 0x084F,  # U16
                'values': [
                    0b10, 0b00  # leave bit 1 unchanged, clear bit 0 to disable charge time period control
                ],
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'action': 'Forbid Discharge',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Time charge start hours',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0856,  # U16
                'values': [
                    0,
                    23,
                    23,
                    23,
                ],
            },
            {
                'description': 'Time charge start minutes',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x085E,  # U16
                'values': [
                    0,
                    59,
                    59,
                    59,
                ],
            },
            {
                'description': 'Time period control flag',
                'function_code': 'MaskWriteRegister',
                'start_address': 0x084F,  # U16
                'values': [
                    0b01, 0b10  # leave bit 0 unchanged, set bit 1 to enable discharge time period control
                ],
            },
            {
                'description': 'UPS reserve enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0862,  # U16
                'values': [
                    1,  # enable
                ],
            },
            {
                'description': 'UPS reserve SOC [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0850,  # U16
                'values': [
                    100,
                ],
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'action': 'Revoke Discharge Override',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'Time period control flag',
                'function_code': 'MaskWriteRegister',
                'start_address': 0x084F,  # U16
                'values': [
                    0b01, 0b00  # leave bit 0 unchanged, clear bit 1 to disable discharge time period control
                ],
            },
            {
                'description': 'UPS reserve enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0862,  # U16
                'values': [
                    0,  # disable
                ],
            },
            {
                'description': 'UPS reserve SOC [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0850,  # U16
                'values': [
                    None,
                ],
                'mapping': 'values[0] = min_soc;',
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'action': 'Forbid Charge',
        'repeat_interval': 60,
        'register_blocks': [
            # FIXME
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'action': 'Revoke Charge Override',
        'repeat_interval': 60,
        'register_blocks': [
            # FIXME
        ],
    },
]
