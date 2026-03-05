# The SAX Power Modbus/TCP interface comes in there user configurable modes:
#
# 1: basic mode: smart meter
# 2: basic mode: power setpoint
# 3: extended mode
#
# In mode 1 the smart meter is used to automatically set the charge/discharge
# power to result in 0 W of grid power. In this mode register 43 and 44 can be used
# to limit the maximum charge/discharge power. These registers are only available
# in this mode and have to be unlocked by the user first. The charge/discharge
# power cannot be forced, but only limited.
#
# In mode 2 the smart meter is not used at all. Fully manual control of the
# charge/discharge power is required using register 41, that is only available
# in this mode.
#
# In mode 3 the smart meter is used to automatically set the charge/discharge
# power to result in 0 W grid power. This mode offers no registers to limit the
# maximum charge/discharge power. The manual mentions register 348 that could be
# used to force the charge/discharge power in percent with fallback after 5 minutes
# to automatically letting the smart meter be used to set the charge/discharge
# power to result in 0 W of grid power. But the manual says that this register is
# not supported yet as of March 2025.
#
# Overall (assuming register 348 would be available) all the parts necessary for
# full battery control are available. But due to the different exclusive modes
# it is not possible to realize full battery control. Therefore, the best that can
# be done right now it normal and block and to degrade force to normal.

table_prototypes = [
    ('SAX Power Home Basic Mode', [
        'device_address',
    ]),
]

default_device_addresses = [
    ('SAX Power Home Basic Mode', 64),
]

repeat_intervals = [
    ('SAX Power Home Basic Mode', 60),
]

specs = [
    {
        'group': 'SAX Power Home Basic Mode',
        'mode': 'Block',
        'actions': ('Block', 'Block'),
        'register_blocks': [
            {
                'description': 'Leistungsgrenzwert für Ladung [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 44,
                'values': [
                    0,
                ],
            },
            {
                'description': 'Leistungsgrenzwert für Entladung [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43,
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'SAX Power Home Basic Mode',
        'mode': 'Normal',
        'actions': ('Normal', 'Normal'),
        'register_blocks': [
            {
                'description': 'Leistungsgrenzwert für Ladung [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 44,
                'values': [
                    4600,  # maximum
                ],
            },
            {
                'description': 'Leistungsgrenzwert für Entladung [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43,
                'values': [
                    4600,  # maximum
                ],
            },
        ],
    },
    {
        'group': 'SAX Power Home Basic Mode',
        'mode': 'Charge From Excess',
        'actions': ('Normal', 'Block'),
        'register_blocks': [
            {
                'description': 'Leistungsgrenzwert für Ladung [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 44,
                'values': [
                    4600,  # maximum
                ],
            },
            {
                'description': 'Leistungsgrenzwert für Entladung [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43,
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'SAX Power Home Basic Mode',
        'mode': 'Charge From Grid',
        'actions': ('Force', 'Block'),  # FIXME: force degraded to normal
        'register_blocks': [
            {
                'description': 'Leistungsgrenzwert für Ladung [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 44,
                'values': [
                    4600,  # maximum
                ],
            },
            {
                'description': 'Leistungsgrenzwert für Entladung [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43,
                'values': [
                    0,
                ],
            },
        ],
    },
    {
        'group': 'SAX Power Home Basic Mode',
        'mode': 'Discharge To Load',
        'actions': ('Block', 'Normal'),
        'register_blocks': [
            {
                'description': 'Leistungsgrenzwert für Ladung [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 44,
                'values': [
                    0,
                ],
            },
            {
                'description': 'Leistungsgrenzwert für Entladung [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43,
                'values': [
                    4600,  # maximum
                ],
            },
        ],
    },
    {
        'group': 'SAX Power Home Basic Mode',
        'mode': 'Discharge To Grid',
        'actions': ('Block', 'Force'),  # FIXME: force degraded to normal
        'register_blocks': [
            {
                'description': 'Leistungsgrenzwert für Ladung [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 44,
                'values': [
                    0,
                ],
            },
            {
                'description': 'Leistungsgrenzwert für Entladung [W]',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 43,
                'values': [
                    4600,  # maximum
                ],
            },
        ],
    },
]
