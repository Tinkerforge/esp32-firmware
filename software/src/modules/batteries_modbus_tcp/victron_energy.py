# https://www.victronenergy.com/live/ess:ess_mode_2_and_3

def s32be(name):
    return f'static_cast<uint16_t>(static_cast<uint32_t>({name}) >> 16)', f'static_cast<uint16_t>(static_cast<uint32_t>({name}) & 0xFFFF)'


display_names = [
    ('Victron Energy GX', {
        'en': 'Victron Energy GX',
        'de': 'Victron Energy GX',
    }),
]

table_prototypes = [
    ('Victron Energy GX', [
        'device_address',
        {
            'name': 'grid_draw_setpoint_normal',  # positive = draw, negative = feed
            'type': 'Int32',
            'default': 50,  # W
        },
        {
            'name': 'grid_draw_setpoint_force_charge',  # positive = draw
            'type': 'Int32',  # FIXME: add range limit to [0..2147483647]
            'default': 1000,  # W
        },
        {
            'name': 'grid_draw_setpoint_force_discharge',  # negative = feed
            'type': 'Int32',  # FIXME: add range limit to [-2147483648..0]
            'default': -1000,  # W
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
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    0, 0,
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
                'description': 'ESS max discharge power [10 W]',
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
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    *s32be('grid_draw_setpoint_normal'),
                ],
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
                'description': 'ESS max discharge power [10 W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2704,  # S16
                'values': [
                    32767,  # maximum = no limit
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'mode': 'Block Discharge',
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    *s32be('grid_draw_setpoint_normal'),
                ],
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
                'description': 'ESS max discharge power [10 W]',
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
        'mode': 'Force Charge',
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    *s32be('grid_draw_setpoint_force_charge'),
                ],
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
                'description': 'ESS max discharge power [10 W]',
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
        'mode': 'Block Charge',
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    *s32be('grid_draw_setpoint_normal'),
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
                'description': 'ESS max discharge power [10 W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2704,  # S16
                'values': [
                    32767,  # maximum = no limit
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'mode': 'Force Discharge',
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    *s32be('grid_draw_setpoint_force_discharge'),
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
                'description': 'ESS max discharge power [10 W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2704,  # S16
                'values': [
                    32767,  # maximum = no limit
                ],
            },
        ],
    },
]
