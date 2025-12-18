table_prototypes = [
    ('SMA Hybrid Inverter', [
        'device_address',
        {
            'name': 'max_normal_charge_power',
            'type': 'Uint32',
            'default': 2000,  # W
        },
        {
            'name': 'max_normal_discharge_power',
            'type': 'Uint32',
            'default': 2000,  # W
        },
        {
            'name': 'force_charge_power',
            'type': 'Uint32',
            'default': 2000,  # W
        },
        {
            'name': 'force_discharge_power',
            'type': 'Uint32',
            'default': 2000,  # W
        },
    ]),
]

default_device_addresses = [
    ('SMA Hybrid Inverter', 3),
]

repeat_intervals = [
    ('SMA Hybrid Inverter', 60),
]

specs = [
    {
        'group': 'SMA Hybrid Inverter',
        'mode': 'Block',
        'actions': ('Block', 'Block'),
        'register_blocks': [
            {
                'description': 'CmpBMS.OpMod - Operating Mode of the battery management system',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40236,
                'values': [
                    0, 2424,  # Default setting, U32BE
                ],
            },
            {
                'description': 'CmpBMS.XYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40793,
                'values': [
                    0, 0,  # CmpBMS.BatChaMinW - Minimum battery charging power [W], U32BE
                    0, 0,  # CmpBMS.BatChaMaxW - Maximum battery charging power [W], U32BE
                    0, 0,  # CmpBMS.BatChaMinW - Minimum battery discharging power [W], U32BE
                    0, 0,  # CmpBMS.BatChaMaxW - Maximum battery discharging power [W], U32BE
                    0, 0,  # CmpBMS.GridWSpt - Gird transfer power setpoint [W], U32BE
                ],
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter',
        'mode': 'Normal',
        'actions': ('Normal', 'Normal'),
        'register_blocks': [
            {
                'description': 'CmpBMS.OpMod - Operating Mode of the battery management system',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40236,
                'values': [
                    0, 2424,  # Default setting, U32BE
                ],
            },
            {
                'description': 'CmpBMS.XYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40793,
                'values': [
                    0, 0,        # CmpBMS.BatChaMinW - Minimum battery charging power [W], U32BE
                    None, None,  # CmpBMS.BatChaMaxW - Maximum battery charging power [W], U32BE
                    0, 0,        # CmpBMS.BatChaMinW - Minimum battery discharging power [W], U32BE
                    None, None,  # CmpBMS.BatChaMaxW - Maximum battery discharging power [W], U32BE
                    0, 0,        # CmpBMS.GridWSpt - Gird transfer power setpoint [W], U32BE
                ],
                'mapping': 'values[2] = static_cast<uint16_t>(max_normal_charge_power >> 16);\n'
                           'values[3] = static_cast<uint16_t>(max_normal_charge_power & 0xFFFF);\n'
                           'values[6] = static_cast<uint16_t>(max_normal_discharge_power >> 16);\n'
                           'values[7] = static_cast<uint16_t>(max_normal_discharge_power & 0xFFFF);',
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter',
        'mode': 'Charge From Excess',
        'actions': ('Normal', 'Block'),
        'register_blocks': [
            {
                'description': 'CmpBMS.OpMod - Operating Mode of the battery management system',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40236,
                'values': [
                    0, 2424,  # Default setting, U32BE
                ],
            },
            {
                'description': 'CmpBMS.XYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40793,
                'values': [
                    0, 0,        # CmpBMS.BatChaMinW - Minimum battery charging power [W], U32BE
                    None, None,  # CmpBMS.BatChaMaxW - Maximum battery charging power [W], U32BE
                    0, 0,        # CmpBMS.BatChaMinW - Minimum battery discharging power [W], U32BE
                    0, 0,        # CmpBMS.BatChaMaxW - Maximum battery discharging power [W], U32BE
                    0, 0,        # CmpBMS.GridWSpt - Gird transfer power setpoint [W], U32BE
                ],
                'mapping': 'values[2] = static_cast<uint16_t>(max_normal_charge_power >> 16);\n'
                           'values[3] = static_cast<uint16_t>(max_normal_charge_power & 0xFFFF);',
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter',
        'mode': 'Charge From Grid',
        'actions': ('Force', 'Block'),
        'register_blocks': [
            {
                'description': 'CmpBMS.OpMod - Operating Mode of the battery management system',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40236,
                'values': [
                    0, 2289,  # Battery charging, U32BE
                ],
            },
            {
                'description': 'CmpBMS.XYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40793,
                'values': [
                    None, None,  # CmpBMS.BatChaMinW - Minimum battery charging power [W], U32BE
                    None, None,  # CmpBMS.BatChaMaxW - Maximum battery charging power [W], U32BE
                    0, 0,        # CmpBMS.BatChaMinW - Minimum battery discharging power [W], U32BE
                    0, 0,        # CmpBMS.BatChaMaxW - Maximum battery discharging power [W], U32BE
                    0, 0,        # CmpBMS.GridWSpt - Gird transfer power setpoint [W], U32BE
                ],
                'mapping': 'values[0] = static_cast<uint16_t>(force_charge_power >> 16);\n'
                           'values[1] = static_cast<uint16_t>(force_charge_power & 0xFFFF);'
                           'values[2] = static_cast<uint16_t>(force_charge_power >> 16);\n'
                           'values[3] = static_cast<uint16_t>(force_charge_power & 0xFFFF);',
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter',
        'mode': 'Discharge To Load',
        'actions': ('Block', 'Normal'),
        'register_blocks': [
            {
                'description': 'CmpBMS.OpMod - Operating Mode of the battery management system',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40236,
                'values': [
                    0, 2424,  # Default setting, U32BE
                ],
            },
            {
                'description': 'CmpBMS.XYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40793,
                'values': [
                    0, 0,        # CmpBMS.BatChaMinW - Minimum battery charging power [W], U32BE
                    0, 0,        # CmpBMS.BatChaMaxW - Maximum battery charging power [W], U32BE
                    0, 0,        # CmpBMS.BatChaMinW - Minimum battery discharging power [W], U32BE
                    None, None,  # CmpBMS.BatChaMaxW - Maximum battery discharging power [W], U32BE
                    0, 0,        # CmpBMS.GridWSpt - Gird transfer power setpoint [W], U32BE
                ],
                'mapping': 'values[6] = static_cast<uint16_t>(max_normal_discharge_power >> 16);\n'
                           'values[6] = static_cast<uint16_t>(max_normal_discharge_power & 0xFFFF);',
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter',
        'mode': 'Discharge To Grid',
        'actions': ('Block', 'Force'),
        'register_blocks': [
            {
                'description': 'CmpBMS.OpMod - Operating Mode of the battery management system',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40236,
                'values': [
                    0, 2290,  # Battery discharging, U32BE
                ],
            },
            {
                'description': 'CmpBMS.XYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40793,
                'values': [
                    0, 0,        # CmpBMS.BatChaMinW - Minimum battery charging power [W], U32BE
                    0, 0,        # CmpBMS.BatChaMaxW - Maximum battery charging power [W], U32BE
                    None, None,  # CmpBMS.BatChaMinW - Minimum battery discharging power [W], U32BE
                    None, None,  # CmpBMS.BatChaMaxW - Maximum battery discharging power [W], U32BE
                    0, 0,        # CmpBMS.GridWSpt - Gird transfer power setpoint [W], U32BE
                ],
                'mapping': 'values[4] = static_cast<uint16_t>(force_discharge_power >> 16);\n'
                           'values[5] = static_cast<uint16_t>(force_discharge_power & 0xFFFF);'
                           'values[6] = static_cast<uint16_t>(force_discharge_power >> 16);\n'
                           'values[7] = static_cast<uint16_t>(force_discharge_power & 0xFFFF);',
            },
        ],
    },
]
