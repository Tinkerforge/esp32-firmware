table_prototypes = [
    ('SMA Hybrid Inverter 40793', [
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
    ('SMA Hybrid Inverter 41467', [
        'device_address',
        {
            'name': 'max_normal_charge_power',
            'type': 'Uint32',  # FIXME: add range limit to [0..2147483647]
            'default': 2000,  # W
        },
        {
            'name': 'max_normal_discharge_power',
            'type': 'Uint32',  # FIXME: add range limit to [0..2147483647]
            'default': 2000,  # W
        },
        {
            'name': 'force_charge_power',
            'type': 'Uint32',  # FIXME: add range limit to [0..2147483647]
            'default': 2000,  # W
        },
        {
            'name': 'force_discharge_power',
            'type': 'Uint32',  # FIXME: add range limit to [0..2147483647]
            'default': 2000,  # W
        },
    ]),
]

default_device_addresses = [
    ('SMA Hybrid Inverter 40793', 3),
    ('SMA Hybrid Inverter 41467', 3),
]

repeat_intervals = [
    ('SMA Hybrid Inverter 40793', 60),  # SMA watchdog duration is 5 minutes
    ('SMA Hybrid Inverter 41467', 60),  # SMA watchdog duration is 5 minutes
]

specs = [
    {
        'group': 'SMA Hybrid Inverter 40793',
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
        'group': 'SMA Hybrid Inverter 40793',
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
        'group': 'SMA Hybrid Inverter 40793',
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
        'group': 'SMA Hybrid Inverter 40793',
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
        'group': 'SMA Hybrid Inverter 40793',
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
        'group': 'SMA Hybrid Inverter 40793',
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
    {
        'group': 'SMA Hybrid Inverter 41467',
        'mode': 'Block',
        'actions': ('Block', 'Block'),
        'register_blocks': [
            {
                'description': 'Bat.WCtlCom.WSptXYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 41467,
                'values': [
                    0, 0,  # Bat.WCtlCom.WSptMax - Maximum setpoint value for active power of storage (positive = discharge, negative = charge) [W], S32BE
                    0, 0,  # Bat.WCtlCom.WSptMin - Minimum setpoint value for active power of storage (positive = discharge, negative = charge) [W], S32BE
                ],
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter 41467',
        'mode': 'Normal',
        'actions': ('Normal', 'Normal'),
        'register_blocks': [
            {
                'description': 'Bat.WCtlCom.WSptXYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 41467,
                'values': [
                    None, None,  # Bat.WCtlCom.WSptMax - Maximum setpoint value for active power of storage (positive = discharge, negative = charge) [W], S32BE
                    None, None,  # Bat.WCtlCom.WSptMin - Minimum setpoint value for active power of storage (positive = discharge, negative = charge) [W], S32BE
                ],
                'mapping': 'values[0] = static_cast<uint16_t>(max_normal_discharge_power >> 16);\n'
                           'values[1] = static_cast<uint16_t>(max_normal_discharge_power & 0xFFFF);\n'
                           'values[2] = static_cast<uint16_t>(static_cast<uint32_t>(-static_cast<int32_t>(max_normal_charge_power)) >> 16);\n'
                           'values[3] = static_cast<uint16_t>(static_cast<uint32_t>(-static_cast<int32_t>(max_normal_charge_power)) & 0xFFFF);',
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter 41467',
        'mode': 'Charge From Excess',
        'actions': ('Normal', 'Block'),
        'register_blocks': [
            {
                'description': 'Bat.WCtlCom.WSptXYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 41467,
                'values': [
                    0, 0,        # Bat.WCtlCom.WSptMax - Maximum setpoint value for active power of storage (positive = discharge, negative = charge) [W], S32BE
                    None, None,  # Bat.WCtlCom.WSptMin - Minimum setpoint value for active power of storage (positive = discharge, negative = charge) [W], S32BE
                ],
                'mapping': 'values[2] = static_cast<uint16_t>(static_cast<uint32_t>(-static_cast<int32_t>(max_normal_charge_power)) >> 16);\n'
                           'values[3] = static_cast<uint16_t>(static_cast<uint32_t>(-static_cast<int32_t>(max_normal_charge_power)) & 0xFFFF);',
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter 41467',
        'mode': 'Charge From Grid',
        'actions': ('Force', 'Block'),
        'register_blocks': [
            {
                'description': 'Bat.WCtlCom.WSptXYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 41467,
                'values': [
                    None, None,  # Bat.WCtlCom.WSptMax - Maximum setpoint value for active power of storage (positive = discharge, negative = charge) [W], S32BE
                    None, None,  # Bat.WCtlCom.WSptMin - Minimum setpoint value for active power of storage (positive = discharge, negative = charge) [W], S32BE
                ],
                'mapping': 'values[0] = static_cast<uint16_t>(static_cast<uint32_t>(-static_cast<int32_t>(force_charge_power)) >> 16);\n'
                           'values[1] = static_cast<uint16_t>(static_cast<uint32_t>(-static_cast<int32_t>(force_charge_power)) & 0xFFFF);\n'
                           'values[2] = static_cast<uint16_t>(static_cast<uint32_t>(-static_cast<int32_t>(force_charge_power)) >> 16);\n'
                           'values[3] = static_cast<uint16_t>(static_cast<uint32_t>(-static_cast<int32_t>(force_charge_power)) & 0xFFFF);',
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter 41467',
        'mode': 'Discharge To Load',
        'actions': ('Block', 'Normal'),
        'register_blocks': [
            {
                'description': 'Bat.WCtlCom.WSptXYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 41467,
                'values': [
                    None, None,  # Bat.WCtlCom.WSptMax - Maximum setpoint value for active power of storage (positive = discharge, negative = charge) [W], S32BE
                    0, 0,        # Bat.WCtlCom.WSptMin - Minimum setpoint value for active power of storage (positive = discharge, negative = charge) [W], S32BE
                ],
                'mapping': 'values[0] = static_cast<uint16_t>(max_normal_discharge_power >> 16);\n'
                           'values[1] = static_cast<uint16_t>(max_normal_discharge_power & 0xFFFF);',
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter 41467',
        'mode': 'Discharge To Grid',
        'actions': ('Block', 'Force'),
        'register_blocks': [
            {
                'description': 'Bat.WCtlCom.WSptXYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 41467,
                'values': [
                    None, None,  # Bat.WCtlCom.WSptMax - Maximum setpoint value for active power of storage (positive = discharge, negative = charge) [W], S32BE
                    None, None,  # Bat.WCtlCom.WSptMin - Minimum setpoint value for active power of storage (positive = discharge, negative = charge) [W], S32BE
                ],
                'mapping': 'values[0] = static_cast<uint16_t>(force_discharge_power >> 16);\n'
                           'values[1] = static_cast<uint16_t>(force_discharge_power & 0xFFFF);\n'
                           'values[2] = static_cast<uint16_t>(force_discharge_power >> 16);\n'
                           'values[3] = static_cast<uint16_t>(force_discharge_power & 0xFFFF);',
            },
        ],
    },
]
