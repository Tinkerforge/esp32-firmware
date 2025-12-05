table_prototypes = [
    ('E3DC Hauskraftwerk', ['device_address', 'virtual_meter']),
]

default_device_addresses = [
    ('E3DC Hauskraftwerk', 1),
]

specs = [
    {
        'name': 'E3DC Hauskraftwerk Grid',
        'virtual_meter': ('E3DC Hauskraftwerk', 'Grid'),
        'default_location': 'Grid',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Leistung am Netzübergabepunkt [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_number': 74,
                'value_type': 'S32LE',
            },
        ],
    },
    {
        'name': 'E3DC Hauskraftwerk Battery',
        'virtual_meter': ('E3DC Hauskraftwerk', 'Battery'),
        'default_location': 'Battery',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Batterie-Leistung [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_number': 70,
                'value_type': 'S32LE',
            },
            {
                'name': 'Batterie-SOC [%]',
                'value_id': 'StateOfCharge',
                'start_number': 83,
                'value_type': 'U16',
            },
        ],
    },
    {
        'name': 'E3DC Hauskraftwerk Load',
        'virtual_meter': ('E3DC Hauskraftwerk', 'Load'),
        'default_location': 'Load',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Hausverbrauchs-Leistung [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_number': 72,
                'value_type': 'S32LE',
            },
        ],
    },
    {
        'name': 'E3DC Hauskraftwerk PV',
        'virtual_meter': ('E3DC Hauskraftwerk', 'PV'),
        'default_location': 'PV',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'DC String 1 Voltage [V]',
                'value_id': 'VoltagePV1',
                'start_number': 96,
                'value_type': 'U16',
            },
            {
                'name': 'DC String 2 Voltage [V]',
                'value_id': 'VoltagePV2',
                'start_number': 97,
                'value_type': 'U16',
            },
            {
                'name': 'DC String 3 Voltage [V]',
                'value_id': 'VoltagePV3',
                'start_number': 98,
                'value_type': 'U16',
            },
            {
                'name': 'DC String 1 Current [0.01 A]',
                'value_id': 'CurrentPV1Export',
                'start_number': 99,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'DC String 2 Current [0.01 A]',
                'value_id': 'CurrentPV2Export',
                'start_number': 100,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'DC String 3 Current [0.01 A]',
                'value_id': 'CurrentPV3Export',
                'start_number': 101,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'DC String 1 Power [W]',
                'value_id': 'PowerPV1Export',
                'start_number': 102,
                'value_type': 'U16',
            },
            {
                'name': 'DC String 2 Power [W]',
                'value_id': 'PowerPV2Export',
                'start_number': 103,
                'value_type': 'U16',
            },
            {
                'name': 'DC String 3 Power [W]',
                'value_id': 'PowerPV3Export',
                'start_number': 104,
                'value_type': 'U16',
            },
            {
                'name': 'Photovoltaik-Leistung [W]',
                'value_id': 'PowerPVSumExport',
                'start_number': 68,
                'value_type': 'S32LE',
            },
        ],
    },
    {
        'name': 'E3DC Hauskraftwerk Additional Generation',
        'virtual_meter': ('E3DC Hauskraftwerk', 'Additional Generation'),
        'default_location': 'PV',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Leistung aller zusätzlichen Einspeiser [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_number': 76,
                'value_type': 'S32LE',
            },
        ],
    },
]
