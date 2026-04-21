from copy import deepcopy


def f32be(value):
    return [f'static_cast<uint16_t>(std::bit_cast<uint32_t>(static_cast<float>({value})) >> 16)', f'static_cast<uint16_t>(std::bit_cast<uint32_t>(static_cast<float>({value})) & 0xFFFF)']


def f32le(value):
    return [f'static_cast<uint16_t>(std::bit_cast<uint32_t>(static_cast<float>({value})) & 0xFFFF)', f'static_cast<uint16_t>(std::bit_cast<uint32_t>(static_cast<float>({value})) >> 16)']


display_names = [
    ('Kostal Plenticore Plus G2 Big Endian', {
        'en': 'KOSTAL PLENTICORE plus G2 (big-endian)',
        'de': 'KOSTAL PLENTICORE plus G2 (Big-Endian)',
    }),
    ('Kostal Plenticore Plus G2 Little Endian', {
        'en': 'KOSTAL PLENTICORE plus G2 (little-endian)',
        'de': 'KOSTAL PLENTICORE plus G2 (Little-Endian)',
    }),
    ('Kostal Plenticore G3 Big Endian', {
        'en': 'KOSTAL PLENTICORE G3 (big-endian)',
        'de': 'KOSTAL PLENTICORE G3 (Big-Endian)',
    }),
    ('Kostal Plenticore G3 Little Endian', {
        'en': 'KOSTAL PLENTICORE G3 (little-endian)',
        'de': 'KOSTAL PLENTICORE G3 (Little-Endian)',
    }),
]

table_prototypes = [
    ('Kostal Plenticore Plus G2 Big Endian', [
        'device_address',
        {
            'name': 'force_charge_power',
            'type': 'Int32',  # FIXME: add range limit to [0..2147483647]
            'default': 2000,  # W
        },
        {
            'name': 'force_discharge_power',
            'type': 'Int32',  # FIXME: add range limit to [0..2147483647]
            'default': 2000,  # W
        },
    ]),
    ('Kostal Plenticore Plus G2 Little Endian', [
        'device_address',
        {
            'name': 'force_charge_power',
            'type': 'Int32',  # FIXME: add range limit to [0..2147483647]
            'default': 2000,  # W
        },
        {
            'name': 'force_discharge_power',
            'type': 'Int32',  # FIXME: add range limit to [0..2147483647]
            'default': 2000,  # W
        },
    ]),
    ('Kostal Plenticore G3 Big Endian', [
        'device_address',
        {
            'name': 'force_charge_power',
            'type': 'Int32',  # FIXME: add range limit to [0..2147483647]
            'default': 2000,  # W
        },
        {
            'name': 'force_discharge_power',
            'type': 'Int32',  # FIXME: add range limit to [0..2147483647]
            'default': 2000,  # W
        },
    ]),
    ('Kostal Plenticore G3 Little Endian', [
        'device_address',
        {
            'name': 'force_charge_power',
            'type': 'Int32',  # FIXME: add range limit to [0..2147483647]
            'default': 2000,  # W
        },
        {
            'name': 'force_discharge_power',
            'type': 'Int32',  # FIXME: add range limit to [0..2147483647]
            'default': 2000,  # W
        },
    ]),
]

default_device_addresses = [
    ('Kostal Plenticore Plus G2 Big Endian', 71),
    ('Kostal Plenticore Plus G2 Little Endian', 71),
    ('Kostal Plenticore G3 Big Endian', 71),
    ('Kostal Plenticore G3 Little Endian', 71),
]

repeat_intervals = [
    ('Kostal Plenticore Plus G2 Big Endian', 20),  # Kostal watchdog duration minimum is 30 seconds
    ('Kostal Plenticore Plus G2 Little Endian', 20),  # Kostal watchdog duration minimum is 30 seconds
    ('Kostal Plenticore G3 Big Endian', 20),  # Kostal watchdog duration minimum is 30 seconds
    ('Kostal Plenticore G3 Little Endian', 20),  # Kostal watchdog duration minimum is 30 seconds
]

# FIXME: register 1034 cannot actively be cleared. it can only be cleared passively by the
#        watchdog, but any write to the register in the 1026 to 1044 range resets the watchdog,
#        keeping register 1034 at its last written value, even if we stop writing it

plenticore_plus_g2_specs = [
    {
        'group': 'Kostal Plenticore Plus G2',
        'mode': 'Block',
        'register_blocks': [
            {
                'description': 'Battery charge power DC setpoint absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1034,
                'f32_values': [
                    0,
                ],
            },
            {
                'description': 'Battery maximum charge power limit absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1038,
                'f32_values': [
                    0,
                ],
            },
            {
                'description': 'Battery maximum discharge power limit absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1040,
                'f32_values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Kostal Plenticore Plus G2',
        'mode': 'Normal',
        'register_blocks': [
            # don't write register 1034, 1038, 1040 to let the watchdog clear any previous force/block charge/discharge
        ],
    },
    {
        'group': 'Kostal Plenticore Plus G2',
        'mode': 'Block Discharge',
        'effective_mode': 'Block',  # it's not possible to actively clear a previous force charge/discharge to gain normal charge again, so block charge too
        'register_blocks': [
            {
                'description': 'Battery charge power DC setpoint absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1034,
                'f32_values': [
                    0,
                ],
            },
            {
                'description': 'Battery maximum charge power limit absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1038,
                'f32_values': [
                    2147483647,
                ],
            },
            {
                'description': 'Battery maximum discharge power limit absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1040,
                'f32_values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Kostal Plenticore Plus G2',
        'mode': 'Force Charge',
        'register_blocks': [
            {
                'description': 'Battery charge power DC setpoint absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1034,
                'f32_values': [
                    '-force_charge_power',  # positive = discharge, negative = charge
                ],
            },
            {
                'description': 'Battery maximum charge power limit absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1038,
                'f32_values': [
                    2147483647,
                ],
            },
            {
                'description': 'Battery maximum discharge power limit absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1040,
                'f32_values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Kostal Plenticore Plus G2',
        'mode': 'Block Charge',
        'effective_mode': 'Block',  # it's not possible to actively clear a previous force charge/discharge to gain normal discharge again, so block discharge too
        'register_blocks': [
            {
                'description': 'Battery charge power DC setpoint absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1034,
                'f32_values': [
                    0,
                ],
            },
            {
                'description': 'Battery maximum charge power limit absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1038,
                'f32_values': [
                    0,
                ],
            },
            {
                'description': 'Battery maximum discharge power limit absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1040,
                'f32_values': [
                    2147483647,
                ],
            },
        ],
    },
    {
        'group': 'Kostal Plenticore Plus G2',
        'mode': 'Force Discharge',
        'register_blocks': [
            {
                'description': 'Battery charge power DC setpoint absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1034,
                'f32_values': [
                    'force_discharge_power',  # positive = discharge, negative = charge
                ],
            },
            {
                'description': 'Battery maximum charge power limit absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1038,
                'f32_values': [
                    0,
                ],
            },
            {
                'description': 'Battery maximum discharge power limit absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1040,
                'f32_values': [
                    2147483647,
                ],
            },
        ],
    },
]

plenticore_g3_specs = [
    {
        'group': 'Kostal Plenticore G3',
        'mode': 'Block',
        'register_blocks': [
            # don't write register 1034 to let the watchdog clear any previous force charge/discharge to gain normal charge/discharge
            {
                'description': 'Max battery charge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1280,
                'f32_values': [
                    0,
                ],
            },
            {
                'description': 'Max battery discharge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1282,
                'f32_values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Kostal Plenticore G3',
        'mode': 'Normal',
        'register_blocks': [
            # don't write register 1034 to let the watchdog clear any previous force charge/discharge to gain normal charge/discharge
            {
                'description': 'Max battery charge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1280,
                'f32_values': [
                    2147483647,
                ],
            },
            {
                'description': 'Max battery discharge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1282,
                'f32_values': [
                    2147483647,
                ],
            },
        ],
    },
    {
        'group': 'Kostal Plenticore G3',
        'mode': 'Block Discharge',
        'register_blocks': [
            # don't write register 1034 to let the watchdog clear any previous force charge/discharge to gain normal charge
            {
                'description': 'Max battery charge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1280,
                'f32_values': [
                    2147483647,
                ],
            },
            {
                'description': 'Max battery discharge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1282,
                'f32_values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Kostal Plenticore G3',
        'mode': 'Force Charge',
        'register_blocks': [
            {
                'description': 'Battery charge power DC setpoint absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1034,
                'f32_values': [
                    '-force_charge_power',  # positive = discharge, negative = charge
                ],
            },
            {
                'description': 'Max battery charge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1280,
                'f32_values': [
                    2147483647,
                ],
            },
            {
                'description': 'Max battery discharge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1282,
                'f32_values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Kostal Plenticore G3',
        'mode': 'Block Charge',
        'register_blocks': [
            # don't write register 1034 to let the watchdog clear any previous force charge/discharge to gain normal discharge
            {
                'description': 'Max battery charge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1280,
                'f32_values': [
                    0,
                ],
            },
            {
                'description': 'Max battery discharge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1282,
                'f32_values': [
                    2147483647,
                ],
            },
        ],
    },
    {
        'group': 'Kostal Plenticore G3',
        'mode': 'Force Discharge',
        'register_blocks': [
            {
                'description': 'Battery charge power DC setpoint absolute [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1034,
                'f32_values': [
                    'force_discharge_power',  # positive = discharge, negative = charge
                ],
            },
            {
                'description': 'Max battery charge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1280,
                'f32_values': [
                    0,
                ],
            },
            {
                'description': 'Max battery discharge power [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 1282,
                'f32_values': [
                    2147483647,
                ],
            },
        ],
    },
]


def make_specs(specs, group_suffix, f32_convert):
    specs = deepcopy(specs)

    for spec in specs:
        spec['group'] += group_suffix

        for register_block in spec['register_blocks']:
            register_block['values'] = []

            for f32_value in register_block['f32_values']:
                register_block['values'] += f32_convert(f32_value)

            del register_block['f32_values']

    return specs


specs = make_specs(plenticore_plus_g2_specs, ' Big Endian', f32be) \
      + make_specs(plenticore_plus_g2_specs, ' Little Endian', f32le) \
      + make_specs(plenticore_g3_specs, ' Big Endian', f32be) \
      + make_specs(plenticore_g3_specs, ' Little Endian', f32le)
