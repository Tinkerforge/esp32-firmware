# This is the same register table as Alpha ESS

table_prototypes = [
    ('Hailei Hybrid Inverter', [
        'device_address',
        {
            'name': 'min_soc',
            'type': 'Uint8',  # FIXME: add range limit to [0..100]
            'default': 10,  # %
        },
        {
            'name': 'max_soc',
            'type': 'Uint8',  # FIXME: add range limit to [0..100]
            'default': 100,  # %
        },
    ]),
]

default_device_addresses = [
    ('Hailei Hybrid Inverter', 85),
]

repeat_intervals = [
    ('Hailei Hybrid Inverter', 60),
]

specs = [
    {
        'group': 'Hailei Hybrid Inverter',
        'mode': 'Block',
        #            Block
        'actions': ('Normal', 'Block'),  # FIXME: cannot fully block charge
        'register_blocks': [
            {
                'description': 'Time discharge start hours',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0851,  # U16
                'values': [
                    0,
                    23,
                    23,
                    23,
                ],
            },
            {
                'description': 'Time discharge start minutes',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x085A,  # U16
                'values': [
                    0,
                    59,
                    59,
                    59,
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
            {
                'description': 'UPS reserve enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0862,  # U16
                'values': [
                    1,  # enable UPS reserve
                ],
            },
            {
                'description': 'Time period control flag',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x084F,  # U16
                'values': [
                    2,  # enable discharge time period control
                ],
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'mode': 'Normal',
        'actions': ('Normal', 'Normal'),
        'register_blocks': [
            {
                'description': 'Time period control flag',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x084F,  # U16
                'values': [
                    0,  # disable time period control
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
            {
                'description': 'UPS reserve enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0862,  # U16
                'values': [
                    0,  # disable UPS reserve
                ],
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'mode': 'Charge From Excess',
        'actions': ('Normal', 'Block'),
        'register_blocks': [
            {
                'description': 'Time discharge start hours',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0851,  # U16
                'values': [
                    0,
                    23,
                    23,
                    23,
                ],
            },
            {
                'description': 'Time discharge start minutes',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x085A,  # U16
                'values': [
                    0,
                    59,
                    59,
                    59,
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
            {
                'description': 'UPS reserve enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0862,  # U16
                'values': [
                    1,  # enable UPS reserve
                ],
            },
            {
                'description': 'Time period control flag',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x084F,  # U16
                'values': [
                    2,  # enable discharge time period control
                ],
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'mode': 'Charge From Grid',
        'actions': ('Force', 'Block'),
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
                'description': 'Time discharge start hours',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0851,  # U16
                'values': [
                    0,
                    23,
                    23,
                    23,
                ],
            },
            {
                'description': 'Time discharge start minutes',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x085A,  # U16
                'values': [
                    0,
                    59,
                    59,
                    59,
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
            {
                'description': 'UPS reserve SOC [%]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0850,  # U16
                'values': [
                    100,
                ],
            },
            {
                'description': 'UPS reserve enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0862,  # U16
                'values': [
                    1,  # enable UPS reserve
                ],
            },
            {
                'description': 'Time period control flag',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x084F,  # U16
                'values': [
                    3,  # enable charge and discharge time period control
                ],
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'mode': 'Discharge To Load',
        #            Block
        'actions': ('Normal', 'Normal'),  # FIXME: cannot fully block charge
        'register_blocks': [
            {
                'description': 'Time period control flag',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x084F,  # U16
                'values': [
                    0,  # disable time period control
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
            {
                'description': 'UPS reserve enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0862,  # U16
                'values': [
                    0,  # disable UPS reserve
                ],
            },
        ],
    },
    {
        'group': 'Hailei Hybrid Inverter',
        'mode': 'Discharge To Grid',
        #            Block     Force
        'actions': ('Normal', 'Normal'),  # FIXME: cannot fully block charge and cannot force discharge
        'register_blocks': [
            {
                'description': 'Time period control flag',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x084F,  # U16
                'values': [
                    0,  # disable time period control
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
            {
                'description': 'UPS reserve enable',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0x0862,  # U16
                'values': [
                    0,  # disable UPS reserve
                ],
            },
        ],
    },
]
