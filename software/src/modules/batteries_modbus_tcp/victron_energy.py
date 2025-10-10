# https://www.victronenergy.com/live/ess:ess_mode_2_and_3

table_prototypes = [
    ('Victron Energy GX', [
        'device_address',
        {
            'action': 'Permit Grid Charge',
            'name': 'grid_draw_setpoint_charge',  # positive = draw, negative = feed
            'type': 'Int32',
            'default': 1000,  # W
        },
        {
            'action': 'Revoke Grid Charge Override',
            'name': 'grid_draw_setpoint_default',  # positive = draw, negative = feed
            'type': 'Int32',
            'default': 50,  # W
        },
    ]),
]

default_device_addresses = [
    ('Victron Energy GX', 100),
]

specs = [
    {
        'group': 'Victron Energy GX',
        'action': 'Permit Grid Charge',
        'repeat_interval': 60,
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
        ],
    },
    {
        'group': 'Victron Energy GX',
        'action': 'Revoke Grid Charge Override',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'AC grid setpoint override [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2716,  # S32BE
                'values': [
                    None,
                    None,
                ],
                'mapping': 'values[0] = static_cast<uint16_t>(static_cast<uint32_t>(grid_draw_setpoint_default) >> 16);\n'
                           'values[1] = static_cast<uint16_t>(static_cast<uint32_t>(grid_draw_setpoint_default) & 0xFFFF);',
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'action': 'Forbid Discharge',
        'repeat_interval': 60,
        'register_blocks': [
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
        'action': 'Revoke Discharge Override',
        'repeat_interval': 60,
        'register_blocks': [
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
        'action': 'Forbid Charge',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'DVCC system max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2705,  # S16
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Victron Energy GX',
        'action': 'Revoke Charge Override',
        'repeat_interval': 60,
        'register_blocks': [
            {
                'description': 'DVCC system max charge current [A]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 2705,  # S16
                'values': [
                    65535,  # -1 = no limit
                ],
            },
        ],
    },
]
