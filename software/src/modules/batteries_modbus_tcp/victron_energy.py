# https://www.victronenergy.com/live/ess:ess_mode_2_and_3

table_prototypes = [
    ('Victron Energy GX', [
        'device_address',
        {
            'name': 'grid_draw_setpoint_normal',  # positive = draw, negative = feed
            'type': 'Int32',
            'default': 50,  # W
        },
        {
            'name': 'grid_draw_setpoint_charge',  # positive = draw, negative = feed
            'type': 'Int32',
            'default': 1000,  # W
        },
        {
            'name': 'grid_draw_setpoint_discharge',  # positive = draw, negative = feed
            'type': 'Int32',
            'default': 1000,  # W
        },
    ]),
]

default_device_addresses = [
    ('Victron Energy GX', 100),
]

repeat_intervals = [
    ('Victron Energy GX', 60),
]

specs = [
    {
        'group': 'Victron Energy GX',
        'mode': 'Block',
        'actions': ('Block', 'Block'),
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    0,
                    0,
                ],
            },
            {
                'description': 'DVCC system max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2705,  # S16
                'values': [
                    0,
                ],
            },
            {
                'description': 'ESS max discharge power [0.1 W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2704,  # S16
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'mode': 'Normal',
        'actions': ('Normal', 'Normal'),
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    None,
                    None,
                ],
                'mapping': 'values[0] = static_cast<uint16_t>(static_cast<uint32_t>(grid_draw_setpoint_normal) >> 16);\n'
                           'values[1] = static_cast<uint16_t>(static_cast<uint32_t>(grid_draw_setpoint_normal) & 0xFFFF);',
            },
            {
                'description': 'DVCC system max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2705,  # S16
                'values': [
                    65535,  # -1 = no limit
                ],
            },
            {
                'description': 'ESS max discharge power [0.1 W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2704,  # S16
                'values': [
                    65535,  # -1 = no limit
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'mode': 'Charge From Excess',
        'actions': ('Normal', 'Block'),
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    None,
                    None,
                ],
                'mapping': 'values[0] = static_cast<uint16_t>(static_cast<uint32_t>(grid_draw_setpoint_normal) >> 16);\n'
                           'values[1] = static_cast<uint16_t>(static_cast<uint32_t>(grid_draw_setpoint_normal) & 0xFFFF);',
            },
            {
                'description': 'DVCC system max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2705,  # S16
                'values': [
                    65535,  # -1 = no limit
                ],
            },
            {
                'description': 'ESS max discharge power [0.1 W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2704,  # S16
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'mode': 'Charge From Grid',
        'actions': ('Force', 'Block'),
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    None,
                    None,
                ],
                'mapping': 'values[0] = static_cast<uint16_t>(static_cast<uint32_t>(grid_draw_setpoint_charge) >> 16);\n'
                           'values[1] = static_cast<uint16_t>(static_cast<uint32_t>(grid_draw_setpoint_charge) & 0xFFFF);',
            },
            {
                'description': 'DVCC system max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2705,  # S16
                'values': [
                    65535,  # -1 = no limit
                ],
            },
            {
                'description': 'ESS max discharge power [0.1 W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2704,  # S16
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'mode': 'Discharge To Load',
        'actions': ('Block', 'Normal'),
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    None,
                    None,
                ],
                'mapping': 'values[0] = static_cast<uint16_t>(static_cast<uint32_t>(grid_draw_setpoint_normal) >> 16);\n'
                           'values[1] = static_cast<uint16_t>(static_cast<uint32_t>(grid_draw_setpoint_normal) & 0xFFFF);',
            },
            {
                'description': 'DVCC system max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2705,  # S16
                'values': [
                    0,
                ],
            },
            {
                'description': 'ESS max discharge power [0.1 W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2704,  # S16
                'values': [
                    65535,  # -1 = no limit
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'mode': 'Discharge To Grid',
        'actions': ('Block', 'Force'),
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    None,
                    None,
                ],
                'mapping': 'values[0] = static_cast<uint16_t>(static_cast<uint32_t>(grid_draw_setpoint_discharge) >> 16);\n'
                           'values[1] = static_cast<uint16_t>(static_cast<uint32_t>(grid_draw_setpoint_discharge) & 0xFFFF);',
            },
            {
                'description': 'DVCC system max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2705,  # S16
                'values': [
                    0,
                ],
            },
            {
                'description': 'ESS max discharge power [0.1 W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2704,  # S16
                'values': [
                    65535,  # -1 = no limit
                ],
            },
        ],
    },
]
