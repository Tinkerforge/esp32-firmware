table_prototypes = [
    ('SMA Hybrid Inverter', [
        'device_address',
        {
            'name': 'max_charge_power',
            'type': 'Uint32',
            'default': 2000,  # W
        },
        {
            'name': 'max_discharge_power',
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
                'description': 'CmpBMS.OpMod - Betriebsart des BMS',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40236,
                'values': [
                    0, 2424,  # Voreinstellung, U32BE
                ],
            },
            {
                'description': 'CmpBMS.XYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40793,
                'values': [
                    0, 0,  # CmpBMS.BatChaMinW - Minimale Batterieladeleistung [W], U32BE
                    0, 0,  # CmpBMS.BatChaMaxW - Maximale Batterieladeleistung [W], U32BE
                    0, 0,  # CmpBMS.BatChaMinW - Minimale Batterieentladeleistung [W], U32BE
                    0, 0,  # CmpBMS.BatChaMaxW - Maximale Batterieentladeleistung [W], U32BE
                    0, 0,  # CmpBMS.GridWSpt - Sollwert der Netzaustauschleistung [W], U32BE
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
                'description': 'CmpBMS.OpMod - Betriebsart des BMS',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40236,
                'values': [
                    0, 2424,  # Voreinstellung, U32BE
                ],
            },
            {
                'description': 'CmpBMS.XYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40793,
                'values': [
                    0, 0,        # CmpBMS.BatChaMinW - Minimale Batterieladeleistung [W], U32BE
                    None, None,  # CmpBMS.BatChaMaxW - Maximale Batterieladeleistung [W], U32BE
                    0, 0,        # CmpBMS.BatChaMinW - Minimale Batterieentladeleistung [W], U32BE
                    None, None,  # CmpBMS.BatChaMaxW - Maximale Batterieentladeleistung [W], U32BE
                    0, 0,        # CmpBMS.GridWSpt - Sollwert der Netzaustauschleistung [W], U32BE
                ],
                'mapping': 'values[2] = static_cast<uint16_t>(max_charge_power >> 16);\n'
                           'values[3] = static_cast<uint16_t>(max_charge_power & 0xFFFF);\n'
                           'values[6] = static_cast<uint16_t>(max_discharge_power >> 16);\n'
                           'values[7] = static_cast<uint16_t>(max_discharge_power & 0xFFFF);',
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter',
        'mode': 'Charge From Excess',
        'actions': ('Normal', 'Block'),
        'register_blocks': [
            {
                'description': 'CmpBMS.OpMod - Betriebsart des BMS',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40236,
                'values': [
                    0, 2424,  # Voreinstellung, U32BE
                ],
            },
            {
                'description': 'CmpBMS.XYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40793,
                'values': [
                    0, 0,        # CmpBMS.BatChaMinW - Minimale Batterieladeleistung [W], U32BE
                    None, None,  # CmpBMS.BatChaMaxW - Maximale Batterieladeleistung [W], U32BE
                    0, 0,        # CmpBMS.BatChaMinW - Minimale Batterieentladeleistung [W], U32BE
                    0, 0,        # CmpBMS.BatChaMaxW - Maximale Batterieentladeleistung [W], U32BE
                    0, 0,        # CmpBMS.GridWSpt - Sollwert der Netzaustauschleistung [W], U32BE
                ],
                'mapping': 'values[2] = static_cast<uint16_t>(max_charge_power >> 16);\n'
                           'values[3] = static_cast<uint16_t>(max_charge_power & 0xFFFF);',
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter',
        'mode': 'Charge From Grid',
        'actions': ('Force', 'Block'),
        'register_blocks': [
            {
                'description': 'CmpBMS.OpMod - Betriebsart des BMS',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40236,
                'values': [
                    0, 2289,  # Batterie laden, U32BE
                ],
            },
            {
                'description': 'CmpBMS.XYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40793,
                'values': [
                    None, None,  # CmpBMS.BatChaMinW - Minimale Batterieladeleistung [W], U32BE
                    None, None,  # CmpBMS.BatChaMaxW - Maximale Batterieladeleistung [W], U32BE
                    0, 0,        # CmpBMS.BatChaMinW - Minimale Batterieentladeleistung [W], U32BE
                    0, 0,        # CmpBMS.BatChaMaxW - Maximale Batterieentladeleistung [W], U32BE
                    0, 0,        # CmpBMS.GridWSpt - Sollwert der Netzaustauschleistung [W], U32BE
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
                'description': 'CmpBMS.OpMod - Betriebsart des BMS',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40236,
                'values': [
                    0, 2424,  # Voreinstellung, U32BE
                ],
            },
            {
                'description': 'CmpBMS.XYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40793,
                'values': [
                    0, 0,        # CmpBMS.BatChaMinW - Minimale Batterieladeleistung [W], U32BE
                    0, 0,        # CmpBMS.BatChaMaxW - Maximale Batterieladeleistung [W], U32BE
                    0, 0,        # CmpBMS.BatChaMinW - Minimale Batterieentladeleistung [W], U32BE
                    None, None,  # CmpBMS.BatChaMaxW - Maximale Batterieentladeleistung [W], U32BE
                    0, 0,        # CmpBMS.GridWSpt - Sollwert der Netzaustauschleistung [W], U32BE
                ],
                'mapping': 'values[6] = static_cast<uint16_t>(max_discharge_power >> 16);\n'
                           'values[6] = static_cast<uint16_t>(max_discharge_power & 0xFFFF);',
            },
        ],
    },
    {
        'group': 'SMA Hybrid Inverter',
        'mode': 'Discharge To Grid',
        'actions': ('Block', 'Force'),
        'register_blocks': [
            {
                'description': 'CmpBMS.OpMod - Betriebsart des BMS',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40236,
                'values': [
                    0, 2290,  # Batterie entladen, U32BE
                ],
            },
            {
                'description': 'CmpBMS.XYZ',
                'function_code': 'WriteMultipleRegisters',
                'start_address': 40793,
                'values': [
                    0, 0,        # CmpBMS.BatChaMinW - Minimale Batterieladeleistung [W], U32BE
                    0, 0,        # CmpBMS.BatChaMaxW - Maximale Batterieladeleistung [W], U32BE
                    None, None,  # CmpBMS.BatChaMinW - Minimale Batterieentladeleistung [W], U32BE
                    None, None,  # CmpBMS.BatChaMaxW - Maximale Batterieentladeleistung [W], U32BE
                    0, 0,        # CmpBMS.GridWSpt - Sollwert der Netzaustauschleistung [W], U32BE
                ],
                'mapping': 'values[4] = static_cast<uint16_t>(force_discharge_power >> 16);\n'
                           'values[5] = static_cast<uint16_t>(force_discharge_power & 0xFFFF);'
                           'values[6] = static_cast<uint16_t>(force_discharge_power >> 16);\n'
                           'values[7] = static_cast<uint16_t>(force_discharge_power & 0xFFFF);',
            },
        ],
    },
]
