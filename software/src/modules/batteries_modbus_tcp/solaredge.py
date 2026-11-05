def f32le(value):
    return [f'static_cast<uint16_t>(std::bit_cast<uint32_t>(static_cast<float>({value})) & 0xFFFF)', f'static_cast<uint16_t>(std::bit_cast<uint32_t>(static_cast<float>({value})) >> 16)']


display_names = [
    ('Solaredge Hybrid Inverter', {
        'en': 'SolarEdge hybrid inverter',
        'de': 'SolarEdge Hybrid-Wechselrichter',
    }),
]

table_prototypes = [
    ('Solaredge Hybrid Inverter', [
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
    ('Solaredge Hybrid Inverter', 1),
]

repeat_intervals = [
    ('Solaredge Hybrid Inverter', 60),  # remote control commands use a timeout of 90 seconds
]

variants = []

specs = [
    {
        'group': 'Solaredge Hybrid Inverter',
        'mode': 'Block',
        'register_blocks': [
            {
                'description': 'Storage Control Mode',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE004,  # U16
                'values': [
                    4,  # Remote control
                ],
            },
            {
                'description': 'Storage AC Charge Policy',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE005,  # U16
                'values': [
                    1,  # Always allowed
                ],
            },
            {
                'description': 'Remote Control Command Timeout',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE00B,  # U32LE
                'values': [
                    90, 0,  # 90 seconds
                ],
            },
            {
                'description': 'Remote Control Command Mode',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE00D,  # U16
                'values': [
                    0,  # Off
                ],
            },
            {
                'description': ' Remote Control Command Charge Limit [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE00E,  # F32LE
                'f32le_values': [
                    0,
                ],
            },
            {
                'description': ' Remote Control Command Discharge Limit [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE010,  # F32LE
                'f32le_values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Solaredge Hybrid Inverter',
        'mode': 'Normal',
        'register_blocks': [
            {
                'description': 'Storage Control Mode',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE004,  # U16
                'values': [
                    1,  # Maximize self-consumption
                ],
            },
            {
                'description': 'Storage AC Charge Policy',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE005,  # U16
                'values': [
                    1,  # Always allowed
                ],
            },
            {
                'description': 'Remote Control Command Timeout',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE00B,  # U32LE
                'values': [
                    90, 0,  # 90 seconds
                ],
            },
            {
                'description': 'Remote Control Command Mode',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE00D,  # U16
                'values': [
                    7,  # Maximize self-consumption
                ],
            },
            {
                'description': ' Remote Control Command Charge Limit [W]',
                'function_code': 'CopyMultipleHoldingRegisters',
                'start_address': 0xE00E,  # F32LE
                'values': [
                    0xE144, 2,
                ],
            },
            {
                'description': ' Remote Control Command Discharge Limit [W]',
                'function_code': 'CopyMultipleHoldingRegisters',
                'start_address': 0xE010,  # F32LE
                'values': [
                    0xE146, 2,
                ],
            },
        ],
    },
    {
        'group': 'Solaredge Hybrid Inverter',
        'mode': 'Block Discharge',
        'register_blocks': [
            {
                'description': 'Storage Control Mode',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE004,  # U16
                'values': [
                    4,  # Remote control
                ],
            },
            {
                'description': 'Storage AC Charge Policy',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE005,  # U16
                'values': [
                    1,  # Always allowed
                ],
            },
            {
                'description': 'Remote Control Command Timeout',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE00B,  # U32LE
                'values': [
                    90, 0,  # 90 seconds
                ],
            },
            {
                'description': 'Remote Control Command Mode',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE00D,  # U16
                'values': [
                    7,  # Maximize self-consumption
                ],
            },
            {
                'description': 'Remote Control Command Charge Limit [W]',
                'function_code': 'CopyMultipleHoldingRegisters',
                'start_address': 0xE00E,  # F32LE
                'values': [
                    0xE144, 2,
                ],
            },
            {
                'description': 'Remote Control Command Discharge Limit [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE010,  # F32LE
                'f32le_values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Solaredge Hybrid Inverter',
        'mode': 'Force Charge',
        'register_blocks': [
            {
                'description': 'Storage Control Mode',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE004,  # U16
                'values': [
                    4,  # Remote control
                ],
            },
            {
                'description': 'Storage AC Charge Policy',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE005,  # U16
                'values': [
                    1,  # Always allowed
                ],
            },
            {
                'description': 'Remote Control Command Timeout',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE00B,  # U32LE
                'values': [
                    90, 0,  # 90 seconds
                ],
            },
            {
                'description': 'Remote Control Command Mode',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE00D,  # U16
                'values': [
                    3,  # Charge from PV+AC according to the max battery power
                ],
            },
            {
                'description': 'Remote Control Command Charge Limit [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE00E,  # F32LE
                'f32le_values': [
                    'force_charge_power',
                ],
            },
            {
                'description': 'Remote Control Command Discharge Limit [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE010,  # F32LE
                'f32le_values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'Solaredge Hybrid Inverter',
        'mode': 'Block Charge',
        'register_blocks': [
            {
                'description': 'Storage Control Mode',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE004,  # U16
                'values': [
                    4,  # Remote control
                ],
            },
            {
                'description': 'Storage AC Charge Policy',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE005,  # U16
                'values': [
                    1,  # Always allowed
                ],
            },
            {
                'description': 'Remote Control Command Timeout',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE00B,  # U32LE
                'values': [
                    90, 0,  # 90 seconds
                ],
            },
            {
                'description': 'Remote Control Command Mode',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE00D,  # U16
                'values': [
                    7,  # Maximize self-consumption
                ],
            },
            {
                'description': 'Remote Control Command Charge Limit [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE00E,  # F32LE
                'f32le_values': [
                    0,
                ],
            },
            {
                'description': 'Remote Control Command Discharge Limit [W]',
                'function_code': 'CopyMultipleHoldingRegisters',
                'start_address': 0xE010,  # F32LE
                'values': [
                    0xE146, 2,
                ],
            },
        ],
    },
    {
        'group': 'Solaredge Hybrid Inverter',
        'mode': 'Force Discharge',
        'register_blocks': [
            {
                'description': 'Storage Control Mode',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE004,  # U16
                'values': [
                    4,  # Remote control
                ],
            },
            {
                'description': 'Storage AC Charge Policy',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE005,  # U16
                'values': [
                    1,  # Always allowed
                ],
            },
            {
                'description': 'Remote Control Command Timeout',
                'function_code': 'IfDifferentWriteMultipleRegisters',
                'start_address': 0xE00B,  # U32LE
                'values': [
                    90, 0,  # 90 seconds
                ],
            },
            {
                'description': 'Remote Control Command Mode',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE00D,  # U16
                'values': [
                    4,  # Maximize export – discharge battery to meet max inverter AC limit
                ],
            },
            {
                'description': 'Remote Control Command Charge Limit [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE00E,  # F32LE
                'f32le_values': [
                    0,
                ],
            },
            {
                'description': 'Remote Control Command Discharge Limit [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 0xE010,  # F32LE
                'f32le_values': [
                    'force_discharge_power',
                ],
            },
        ],
    },
]


for spec in specs:
    for register_block in spec['register_blocks']:
        if 'values' not in register_block:
            register_block['values'] = []

            for f32le_value in register_block['f32le_values']:
                register_block['values'] += f32le(f32le_value)

            del register_block['f32le_values']
